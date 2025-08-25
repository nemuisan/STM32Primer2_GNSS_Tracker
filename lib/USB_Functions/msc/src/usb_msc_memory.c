/********************************************************************************/
/*!
	@file			usb_msc_memory.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2025.08.19
	@brief          Memory management layer.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2016.01.15	V3.00	Changed definition compatibility.
		2016.12.28	V4.00	Fixed address calculation above 4GB.
		2017.03.29	V5.00	Removed retired variables.
		2019.09.20	V6.00	Fixed shadowed variable.
		2023.03.23	V7.00	Fixed data_buffer capacity.
		2025.04.09	V8.00	Changed to MultiBlock R/W.
		2025.08.19	V9.00	Fixed data corruption bug in write mode.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_msc_memory.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint32_t Byte_Read_count = 0;
__IO uint32_t Byte_Read_offset = 0;
__IO uint32_t Byte_Write_count = 0;
__IO uint8_t TransferState = TXFR_IDLE;
/* SDIO-DMA Data buffer MUST BE 4byte-alignment */
/* Multiple of 64byte and SD Sector size 512byte */
#define DATA_PACKET_SIZE (1024*16)
uint8_t Data_Buffer[DATA_PACKET_SIZE] __attribute__ ((aligned (4)));

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief	Handle the Read operation from the microSD card.
*/
/**************************************************************************/
void Read_Memory(uint8_t lun, uint32_t Block_Addr, uint32_t BlockCount)
{
  static uint64_t R_Offset;
  static uint32_t R_Length;
  static uint32_t R_Residu;

  if (TransferState == TXFR_IDLE )
  {
    R_Offset = (uint64_t)Block_Addr * Mass_Block_Size[lun];
    R_Length = BlockCount * Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
	Byte_Read_count = 0;
  }
  
  if (TransferState == TXFR_ONGOING )
  {
    if(Byte_Read_count==0)
    {
	  if(R_Length > DATA_PACKET_SIZE)	R_Residu = DATA_PACKET_SIZE;
	  else 								R_Residu = R_Length;
	  
	  /* Doing Multi/Single block read */
      MAL_Read(lun ,
               R_Offset,
               (uint32_t*)Data_Buffer,
			   R_Residu);
			   
      USB_SIL_DBLWrite(ENDP1, Data_Buffer, BULK_MAX_PACKET_SIZE);
	  
      Byte_Read_count = R_Residu - BULK_MAX_PACKET_SIZE;
      Byte_Read_offset = BULK_MAX_PACKET_SIZE;
    }
    else
    {
      USB_SIL_DBLWrite(ENDP1, Data_Buffer + Byte_Read_offset, BULK_MAX_PACKET_SIZE);
	  
      Byte_Read_count -= BULK_MAX_PACKET_SIZE;
      Byte_Read_offset += BULK_MAX_PACKET_SIZE;
    }

    R_Offset += BULK_MAX_PACKET_SIZE;
    R_Length -= BULK_MAX_PACKET_SIZE;

    CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;
    Led_RW_ON();
  }
  
  if (R_Length == 0)
  {
    Byte_Read_count = 0;
    Byte_Read_offset = 0;
    R_Offset = 0;
	R_Residu = 0;
    Bot_State = BOT_DATA_IN_LAST;
    TransferState = TXFR_IDLE;
    Led_RW_OFF();
  }
}

/**************************************************************************/
/*! 
    @brief	Handle the Write operation to the microSD card.
*/
/**************************************************************************/
void Write_Memory (uint8_t lun, uint32_t Block_Addr, uint32_t BlockCount)
{
  static uint64_t W_Offset;
  static uint32_t W_Length;
  static uint32_t W_Residu;

  uint32_t temp =  Byte_Write_count + Data_Len;

  if (TransferState == TXFR_IDLE )
  {
    W_Offset = (uint64_t)Block_Addr * Mass_Block_Size[lun];
    W_Length = BlockCount * Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
	Byte_Write_count = 0;
  }
  
  if (TransferState == TXFR_ONGOING )
  {
    if(Byte_Write_count==0) /* Byte_Write_count can take Mass_Block_Size[lun], W_Length or DATA_PACKET_SIZE */
    {
		if(W_Length > DATA_PACKET_SIZE)	W_Residu = DATA_PACKET_SIZE; /* Multiply of 512byte */
		else 							W_Residu = W_Length;
	}

    for (int idx = 0 ; Byte_Write_count < temp; Byte_Write_count++)
    {
      *(Data_Buffer + Byte_Write_count) = Bulk_Data_Buff[idx++];
    }

    W_Offset += Data_Len;
    W_Length -= Data_Len;
	
	/* Doing Multi/Single block write */
    if (Byte_Write_count >= W_Residu)
    {
      Byte_Write_count = 0;
      MAL_Write(lun ,
                W_Offset - W_Residu,
                (uint32_t*)Data_Buffer,
                W_Residu);
    }

    CSW.dDataResidue -= Data_Len;
    Led_RW_ON();
  }

  if ((W_Length == 0) || (Bot_State == BOT_CSW_Send))
  {
    Byte_Write_count = 0;
	W_Residu = 0;
    Set_CSW (CSW_CMD_PASSED);
    TransferState = TXFR_IDLE;
    Led_RW_OFF();
  }
}

/* End Of File ---------------------------------------------------------------*/
