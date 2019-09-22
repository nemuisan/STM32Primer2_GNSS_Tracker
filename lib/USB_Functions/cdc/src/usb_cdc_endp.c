/********************************************************************************/
/*!
	@file			usb_cdc_endp.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2019.09.20
	@brief          Endpoint routines.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2019.09.20	V2.00	Fixed shadowed variable.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_cdc_desc.h"
#include "usb_cdc_conf.h"
#include "usb_pwr.h"
#include "usb_istr.h"

/* Defines -------------------------------------------------------------------*/
/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5

/* Variables -----------------------------------------------------------------*/
uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];
extern  uint8_t USART_Rx_Buffer[];
extern uint32_t USART_Rx_ptr_out;
extern uint32_t USART_Rx_length;
extern uint8_t  USB_Tx_State;
extern uint8_t  USB_xMutex;
extern uint32_t USART_Rx_ptr_in;


/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  CDC_EP1_IN_Callback.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void CDC_EP1_IN_Callback(void)
{
  uint16_t USB_Tx_ptr;
  uint16_t USB_Tx_length;
  
  if (USB_Tx_State == 1)
  {
    if (USART_Rx_length == 0) 
    {
      USB_Tx_State = 0;
	  USB_xMutex =1;
    }
    else 
    {

		  if (USART_Rx_length > VIRTUAL_COM_PORT_DATA_SIZE){
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = VIRTUAL_COM_PORT_DATA_SIZE;
			
			USART_Rx_ptr_out += VIRTUAL_COM_PORT_DATA_SIZE;
			USART_Rx_length -= VIRTUAL_COM_PORT_DATA_SIZE;    
		  }
		  else 
		  {
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = USART_Rx_length;
			
			USART_Rx_ptr_out += USART_Rx_length;
			USART_Rx_length = 0;
		  }

      UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], CDC_ENDP1_TXADDR, USB_Tx_length);
      SetEPTxCount(ENDP1, USB_Tx_length);
      SetEPTxValid(ENDP1); 
    }
	
	if((USART_Rx_ptr_in != 0) && (USB_xMutex ==1)){ /* USB_Tx_State =0,xMutex=1,and UARTRX data reached */

		if (USART_Rx_length > VIRTUAL_COM_PORT_DATA_SIZE){
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = VIRTUAL_COM_PORT_DATA_SIZE;
			
			USART_Rx_ptr_out += VIRTUAL_COM_PORT_DATA_SIZE;
			USART_Rx_length -= VIRTUAL_COM_PORT_DATA_SIZE;    
		}
		else 
		{
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = USART_Rx_length;
			
			USART_Rx_ptr_out += USART_Rx_length;
			USART_Rx_length = 0;
		}
		
      UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], CDC_ENDP1_TXADDR, USB_Tx_length);
      SetEPTxCount(ENDP1, USB_Tx_length);
      SetEPTxValid(ENDP1); 
		}

  }
}

/**************************************************************************/
/*! 
    @brief  CDC_EP3_OUT_Callback.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void CDC_EP3_OUT_Callback(void)
{
  uint16_t USB_Rx_Cnt;
  
  /* Get the received data buffer and update the counter */
  /* Nemui Added DoubleBuffer */
  if (GetENDPOINT(ENDP3) & EP_DTOG_TX)
  {
	FreeUserBuffer(ENDP3, EP_DBUF_OUT); 
	USB_Rx_Cnt = GetEPDblBuf0Count(ENDP3);
	PMAToUserBufferCopy(USB_Rx_Buffer, CDC_ENDP3_BUF0Addr, USB_Rx_Cnt);
  }
  else
  {
	FreeUserBuffer(ENDP3, EP_DBUF_OUT); 
	USB_Rx_Cnt = GetEPDblBuf1Count(ENDP3);
	PMAToUserBufferCopy(USB_Rx_Buffer, CDC_ENDP3_BUF1Addr, USB_Rx_Cnt);
  } 

  /* USB data will be immediately processed, this allow next USB traffic beeing 
  NAKed till the end of the USART Xfet */
  USB_To_USART_Send_Data(USB_Rx_Buffer, USB_Rx_Cnt);

  /* Enable the receive of data on EP3 */
  SetEPRxValid(ENDP3);
 
}

/**************************************************************************/
/*! 
    @brief  CDC_SOF_Callback / INTR_SOFINTR_Callback.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void CDC_SOF_Callback(void)
{
  static uint32_t FrameCount = 0;
  
  if(bDeviceState == CONFIGURED)
  {
    if (FrameCount++ == VCOMPORT_IN_FRAME_INTERVAL)
    {
      /* Reset the frame counter */
      FrameCount = 0;
      
      /* Check the data to be sent through IN pipe */
      Handle_USBAsynchXfer();
    }
  }  
}

/* End Of File ---------------------------------------------------------------*/
