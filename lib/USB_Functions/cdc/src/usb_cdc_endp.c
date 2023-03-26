/********************************************************************************/
/*!
	@file			usb_cdc_endp.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.03.21
	@brief          Endpoint routines.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2019.09.20	V2.00	Fixed shadowed variable.
		2023.03.21	V3.00	Fixed USB Rx buffer size.
		
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
uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE*2]  __attribute__ ((aligned (4)));
__IO uint8_t cdc_zpf = 0; /* Zero-length packet flag */
extern  uint8_t USART_Rx_Buffer[]; /* Assure 2048Bytes */
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

	if(USB_Tx_State == 1)
	{/* Device to HOST_PC Transfer state */
		if (USART_Rx_length == 0) 
		{/* USB_Tx_State = 1,xMutex = any,and No UART reached */
			USB_Tx_State = 0;
			USB_xMutex = 1;
			
			/* Send zero-length packet */
			if(cdc_zpf ==1){
				cdc_zpf = 0;
				USB_Tx_length = 0;
				USB_Tx_ptr = 0;
				UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], CDC_ENDP1_TXADDR, USB_Tx_length);
				SetEPTxCount(ENDP1, USB_Tx_length);
				SetEPTxValid(ENDP1);
			}
		}
		else 
		{/* USB_Tx_State = 1,xMutex = any,and UART datas reached */
			if (USART_Rx_length > VIRTUAL_COM_PORT_DATA_SIZE)
			{
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
				
				/* Make zero-length packet flag if needed */
				if(USB_Tx_length == VIRTUAL_COM_PORT_DATA_SIZE) cdc_zpf =1;
			}

			UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], CDC_ENDP1_TXADDR, USB_Tx_length);
			SetEPTxCount(ENDP1, USB_Tx_length);
			SetEPTxValid(ENDP1); 
		}
	}
	else if((USART_Rx_ptr_in > 0) && (USB_xMutex ==1))
	{	/* USB_Tx_State = 0,xMutex=1,and UARTRX data reached */
		/* Data HUN-DUMARI taisaku */
		if (USART_Rx_length > VIRTUAL_COM_PORT_DATA_SIZE)
		{
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
			
			/* Make zero-length packet flag if needed */
			if(USB_Tx_length == VIRTUAL_COM_PORT_DATA_SIZE) cdc_zpf =1;
		}
		
 		UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], CDC_ENDP1_TXADDR, USB_Tx_length);
		SetEPTxCount(ENDP1, USB_Tx_length);
		SetEPTxValid(ENDP1);
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
	uint8_t RxBufOFs;

	/* Double Buffered RX */
	if (GetENDPOINT(ENDP3) & EP_DTOG_TX)
	{
		USB_Rx_Cnt = GetEPDblBuf0Count(ENDP3);
		if(USB_Rx_Cnt == 0) return; /* Detect zero length packet */
		FreeUserBuffer(ENDP3, EP_DBUF_OUT);	/* Toggles EP_DTOG_TX / SW_BUF soon */
		RxBufOFs = 0;
		PMAToUserBufferCopy(USB_Rx_Buffer, CDC_ENDP3_BUF0Addr, USB_Rx_Cnt);
	}
	else
	{
		USB_Rx_Cnt = GetEPDblBuf1Count(ENDP3);
		if(USB_Rx_Cnt == 0) return; /* Detect zero length packet */
		FreeUserBuffer(ENDP3, EP_DBUF_OUT);	/* Toggles EP_DTOG_TX / SW_BUF soon */
		RxBufOFs = VIRTUAL_COM_PORT_DATA_SIZE;
		PMAToUserBufferCopy(USB_Rx_Buffer+RxBufOFs, CDC_ENDP3_BUF1Addr, USB_Rx_Cnt);
	} 

	/* USB data will be immediately processed, this allow next USB traffic beeing 
	   NAKed till the end of the USART Xfer */
	/* Assure MAX 64Bytes at USB-FullSpeed */
	USB_To_USART_Send_Data(USB_Rx_Buffer+RxBufOFs, USB_Rx_Cnt);

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

			/* Check the data to be sent through IN pipe,see cdc_support.c */
			Handle_USBAsynchXfer();
		}
	}
}

/* End Of File ---------------------------------------------------------------*/
