/********************************************************************************/
/*!
	@file			usb_msc_memory.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        8.00
    @date           2025.04.09
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

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_msc_memory.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint32_t Block_Read_count = 0;
__IO uint32_t Block_offset;
__IO uint32_t Counter = 0;
__IO uint8_t TransferState = TXFR_IDLE;
/* USB/DMA Data buffer MUST BE 4byte-alignment */
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
  }
  
  if (TransferState == TXFR_ONGOING )
  {
    if(Block_Read_count==0)
    {
	  if(R_Length > DATA_PACKET_SIZE)	R_Residu = DATA_PACKET_SIZE;
	  else 								R_Residu = R_Length;
	  
      MAL_Read(lun ,
               R_Offset,
               (uint32_t*)Data_Buffer,
			   R_Residu);
			   
      USB_SIL_DBLWrite(ENDP1, Data_Buffer, BULK_MAX_PACKET_SIZE);
	  
      Block_Read_count = R_Residu - BULK_MAX_PACKET_SIZE;
      Block_offset = BULK_MAX_PACKET_SIZE;
    }
    else
    {
      USB_SIL_DBLWrite(ENDP1, Data_Buffer + Block_offset, BULK_MAX_PACKET_SIZE);
	  
      Block_Read_count -= BULK_MAX_PACKET_SIZE;
      Block_offset += BULK_MAX_PACKET_SIZE;
    }

    R_Offset += BULK_MAX_PACKET_SIZE;
    R_Length -= BULK_MAX_PACKET_SIZE;

    CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;
    Led_RW_ON();
  }
  
  if (R_Length == 0)
  {
    Block_Read_count = 0;
    Block_offset = 0;
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

  uint32_t temp =  Counter + BULK_MAX_PACKET_SIZE;

  if (TransferState == TXFR_IDLE )
  {
    W_Offset = (uint64_t)Block_Addr * Mass_Block_Size[lun];
    W_Length = BlockCount * Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
	
	if(BlockCount >= 4) /* Doing multisector write above "4"counts */
	{
		if(W_Length > DATA_PACKET_SIZE)	W_Residu = DATA_PACKET_SIZE;
		else 							W_Residu = Mass_Block_Size[lun];
	}
	else
	{
		W_Residu = Mass_Block_Size[lun];
	}
  }
  
  if (TransferState == TXFR_ONGOING )
  {

    for (int idx = 0 ; Counter < temp; Counter++)
    {
      *(Data_Buffer + Counter) = Bulk_Data_Buff[idx++];
    }

    W_Offset += Data_Len;
    W_Length -= Data_Len;
	
	/* Doing Multi/Single block write */
    if (!(W_Length % W_Residu))
    {
      Counter = 0;
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
    Counter = 0;
	W_Residu = 0;
    Set_CSW (CSW_CMD_PASSED);
    TransferState = TXFR_IDLE;
    Led_RW_OFF();
  }
}

/* End Of File ---------------------------------------------------------------*/
