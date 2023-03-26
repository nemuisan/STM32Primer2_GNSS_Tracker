/********************************************************************************/
/*!
	@file			usb_msc_memory.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2023.03.23
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
/* Data buffer MUST BE 4byte-alignment */
/* Multiple of 64byte and SD Sector size 512byte */
uint8_t Data_Buffer[512] __attribute__ ((aligned (4)));
__IO uint8_t TransferState = TXFR_IDLE;


/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief	Handle the Read operation from the microSD card.
*/
/**************************************************************************/
void Read_Memory(uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length)
{
  static uint64_t R_Offset;
  static uint32_t R_Length;

  if (TransferState == TXFR_IDLE )
  {
    R_Offset = (uint64_t)Memory_Offset * Mass_Block_Size[lun];
    R_Length = Transfer_Length * Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
  }
  
  if (TransferState == TXFR_ONGOING )
  {
    if(Block_Read_count==0)
    {
      MAL_Read(lun ,
               R_Offset ,
               (uint32_t*)Data_Buffer,
               Mass_Block_Size[lun]);

      USB_SIL_Write(EP1_IN, Data_Buffer, BULK_MAX_PACKET_SIZE);

      Block_Read_count = Mass_Block_Size[lun] - BULK_MAX_PACKET_SIZE;
      Block_offset = BULK_MAX_PACKET_SIZE;
    }
    else
    {
      USB_SIL_Write(EP1_IN, Data_Buffer + Block_offset, BULK_MAX_PACKET_SIZE);

      Block_Read_count -= BULK_MAX_PACKET_SIZE;
      Block_offset += BULK_MAX_PACKET_SIZE;
    }

    SetEPTxCount(ENDP1, BULK_MAX_PACKET_SIZE);
    SetEPTxStatus(ENDP1, EP_TX_VALID);

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
    Bot_State = BOT_DATA_IN_LAST;
    TransferState = TXFR_IDLE;
    Led_RW_OFF();
  }
}

/**************************************************************************/
/*! 
    @brief	Handle the Write operation to the microSD card..
*/
/**************************************************************************/
void Write_Memory (uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length)
{

  static uint64_t W_Offset;
  static uint32_t W_Length;

  uint32_t temp =  Counter + 64;

  if (TransferState == TXFR_IDLE )
  {
    W_Offset = (uint64_t)Memory_Offset * Mass_Block_Size[lun];
    W_Length = Transfer_Length * Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
  }
  
  if (TransferState == TXFR_ONGOING )
  {

    for (int idx = 0 ; Counter < temp; Counter++)
    {
      *(Data_Buffer + Counter) = Bulk_Data_Buff[idx++];
    }

    W_Offset += Data_Len;
    W_Length -= Data_Len;
	
	/* Doing Single block write */
    if (!(W_Length % Mass_Block_Size[lun]))
    {
      Counter = 0;
      MAL_Write(lun ,
                W_Offset - Mass_Block_Size[lun],
                (uint32_t*)Data_Buffer,
                Mass_Block_Size[lun]);
    }

    CSW.dDataResidue -= Data_Len;
    SetEPRxStatus(ENDP2, EP_RX_VALID); /* enable the next transaction */

    Led_RW_ON();
  }

  if ((W_Length == 0) || (Bot_State == BOT_CSW_Send))
  {
    Counter = 0;
    Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
    TransferState = TXFR_IDLE;
    Led_RW_OFF();
  }
}

/* End Of File ---------------------------------------------------------------*/
