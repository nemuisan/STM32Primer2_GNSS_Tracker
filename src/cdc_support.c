/********************************************************************************/
/*!
	@file			cdc_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        10.00
    @date           2025.04.21
	@brief          Interface of USB-CommunicationDeviceClass.

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.04.20	V2.00	Fixed Suitable Interruption level.
		2014.07.16	V3.00	Reset Systick to Suitable Frequency.
		2019.09.20	V4.00	Fixed redundant declaration.
		2020.05.30	V5.00	Display system version string.
		2022.10.10	V6.00	Purge UART buffer on connect.
		2023.03.22	V7.00	Enable UART Rx interrupt on connect.
		2023.12.19  V8.00	Improved watchdog handlings.
		2025.04.08	V9.00	Changed minor function name.
		2025.04.21 V10.00	Fixed UART Rx-Pin to pullup.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "cdc_support.h"
/* check header file version for fool proof */
#if CDC_SUPPORT_H!= 0x1000
#error "header file version is not correspond!"
#endif

#include "display_if.h"
#include "display_if_support.h"
#include "font_if.h"

/* Defines -------------------------------------------------------------------*/
/* Set USB-CDC RX-Buffer Size */
#define USART_RX_DATA_SIZE   	2048

/* Variables -----------------------------------------------------------------*/
extern LINE_CODING linecoding;

uint8_t  USART_Rx_Buffer [USART_RX_DATA_SIZE];
uint32_t USART_Rx_ptr_in  = 0;
uint32_t USART_Rx_ptr_out = 0;
uint32_t USART_Rx_length  = 0;
uint8_t  USB_Tx_State     = 0;
uint8_t  USB_xMutex       = 0;

extern __IO uint8_t cdc_zpf; /* Zero-length packet flag */

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**************************************************************************/
/*!
    @brief	Configure of UART Specific CDC.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USART_EnableRXInt(void)
{
	/* Enable the USART Receive interrupt */
	USART_ITConfig(CDC_UART, USART_IT_RXNE, ENABLE);
}
/**************************************************************************/
/*!
    @brief	Configure of UART Specific CDC.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USART_DisableRXInt(void)
{
	/* Enable the USART Receive interrupt */
	USART_ITConfig(CDC_UART, USART_IT_RXNE, DISABLE);
}
	
/**************************************************************************/
/*!
    @brief	Configure of UART Specific CDC.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USART_Config_Default(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Default configuration
		- BaudRate = 9600 baud
		- Word Length = 8 Bits
		- One Stop Bit
		- Parity None
		- Hardware flow control disabled
		- Receive and transmit enabled
	*/
	USART_InitStructure.USART_BaudRate 				= 9600;
	USART_InitStructure.USART_WordLength 			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits 				= USART_StopBits_1;
	USART_InitStructure.USART_Parity 				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode 					= USART_Mode_Rx | USART_Mode_Tx;

	/* Turn on peripheral clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Configure USART2 TX as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART2 RX as alternate function in pullup */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Init USART */
	USART_Init(CDC_UART, &USART_InitStructure);

	/* Enable USART */
	USART_Cmd(CDC_UART, ENABLE);
}


/**************************************************************************/
/*!
    @brief	Configure of UART Specific CDC.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
bool USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* set the Stop bit */
	switch (linecoding.format)
	{
		case 0:
			USART_InitStructure.USART_StopBits = USART_StopBits_1;
			break;
		case 1:
			USART_InitStructure.USART_StopBits = USART_StopBits_1_5;
			break;
		case 2:
			USART_InitStructure.USART_StopBits = USART_StopBits_2;
			break;
		default :
			{
				USART_Config_Default();
				return (FALSE);
			}
	}

	/* set the parity bit */
	switch (linecoding.paritytype)
	{
		case 0:
			USART_InitStructure.USART_Parity = USART_Parity_No;
			break;
		case 1:
			USART_InitStructure.USART_Parity = USART_Parity_Even;
			break;
		case 2:
			USART_InitStructure.USART_Parity = USART_Parity_Odd;
			break;
		default :
			{
				USART_Config_Default();
				return (false);
			}
	}

	/*set the data type : only 8bits and 9bits is supported */
	switch (linecoding.datatype)
	{
		case 0x07:
			/* With this configuration a parity (Even or Odd) should be set */
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;
			break;
		case 0x08:
			if (USART_InitStructure.USART_Parity == USART_Parity_No)
			{
				USART_InitStructure.USART_WordLength = USART_WordLength_8b;
			}
			else
			{
				USART_InitStructure.USART_WordLength = USART_WordLength_9b;
			}
			break;
			
		default :
			{
				USART_Config_Default();
				return (false);
			}
	}

	USART_InitStructure.USART_BaudRate = linecoding.bitrate;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure and enable the USART */
	/* Turn on peripheral clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* Configure USART2 TX as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART2 RX as alternate function in pullup */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Purge UART Buffer */
	USART_ITConfig(CDC_UART, USART_IT_RXNE, DISABLE);
	USART_Rx_ptr_in  = 0;
	USART_Rx_ptr_out = 0;
	USART_Rx_length  = 0;
	USB_Tx_State     = 0;
	USB_xMutex       = 0;

	/* Init USART */
	USART_Init(CDC_UART, &USART_InitStructure);

	/* Enable USART */
	USART_Cmd(CDC_UART, ENABLE);

	return (true);
}


/**************************************************************************/
/*!
    @brief	Send the received data from USB to the UART.
	@param	data_buffer : data address.
            Nb_bytes    : number of bytes to send.
    @retval	None.
*/
/**************************************************************************/
void USB_To_USART_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes)
{
	uint32_t i;

	for (i = 0; i < Nb_bytes; i++)
	{
		USART_SendData(CDC_UART, *(data_buffer + i));
		while(USART_GetFlagStatus(CDC_UART, USART_FLAG_TXE) == RESET);
	}

}


/**************************************************************************/
/*!
    @brief	Send data to USB.Executed in USBInterrupt Handler(SOF Callback).
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Handle_USBAsynchXfer (void)
{
	uint16_t USB_Tx_ptr;
	uint16_t USB_Tx_length;

	/* Release Mutex */
	USB_xMutex = 0;

	if(USB_Tx_State != 1)
	{
		/* Overflow */
		if (USART_Rx_ptr_out >= USART_RX_DATA_SIZE)
		{
			USART_Rx_ptr_out = 0;
		}
		
		/* If Rx buffer is empty */
		if(USART_Rx_ptr_out == USART_Rx_ptr_in)
		{
			USB_Tx_State = 0;
			
			if(cdc_zpf == 1){
				/* Send zero-length packet */
				cdc_zpf = 0;
				USB_Tx_length = 0;
				USB_Tx_ptr = 0;
				UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], CDC_ENDP1_TXADDR, USB_Tx_length);
				SetEPTxCount(ENDP1, USB_Tx_length);
				SetEPTxValid(ENDP1);
			}
			return;
		}
		
		/* Pointer rollback */
		if(USART_Rx_ptr_out > USART_Rx_ptr_in) 
		{
			USART_Rx_length = USART_RX_DATA_SIZE - USART_Rx_ptr_out;
		}
		else
		{
			USART_Rx_length = USART_Rx_ptr_in - USART_Rx_ptr_out;
		}
		
		/* Larger than Max PacketSize */
		if(USART_Rx_length > VIRTUAL_COM_PORT_DATA_SIZE)
		{
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = VIRTUAL_COM_PORT_DATA_SIZE;
			
			USART_Rx_ptr_out += VIRTUAL_COM_PORT_DATA_SIZE;
			USART_Rx_length -= VIRTUAL_COM_PORT_DATA_SIZE;
		}
		else /* Smaller than Max PacketSize or equal */
		{
			USB_Tx_ptr = USART_Rx_ptr_out;
			USB_Tx_length = USART_Rx_length;
			
			USART_Rx_ptr_out += USART_Rx_length;
			USART_Rx_length = 0;
			
			/* Make zero-length packet flag if needed */
			if(USB_Tx_length == VIRTUAL_COM_PORT_DATA_SIZE) cdc_zpf =1;
		}
		
		/* Set USB Transfer State */
		USB_Tx_State = 1;
		
		UserToPMABufferCopy(&USART_Rx_Buffer[USB_Tx_ptr], CDC_ENDP1_TXADDR, USB_Tx_length);
		SetEPTxCount(ENDP1, USB_Tx_length);
		SetEPTxValid(ENDP1);
	}
}

/**************************************************************************/
/*!
    @brief	Send the received data from UART to USB.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USART_To_USB_Send_Data(void)
{

	if (linecoding.datatype == 7)
	{
		USART_Rx_Buffer[USART_Rx_ptr_in] = USART_ReceiveData(USART2) & 0x7F;
	}
	else if (linecoding.datatype == 8)
	{
		USART_Rx_Buffer[USART_Rx_ptr_in] = USART_ReceiveData(USART2);
	}
	else /* assure linecoding.datatype == 8 */
	{
		USART_Rx_Buffer[USART_Rx_ptr_in] = USART_ReceiveData(USART2);
	}

	USART_Rx_ptr_in++;

	/* To avoid buffer overflow */
	if(USART_Rx_ptr_in >= USART_RX_DATA_SIZE)
	{
		USART_Rx_ptr_in = 0;
	}
}


/**************************************************************************/
/*!
    @brief	Configures the USB interrupts.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
static void USB_Interrupts_Config(void)
{
	/* Enable USB_LP Interrupt */
	NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn,3);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	
	/* Enable USART Interrupt */
	NVIC_SetPriority(CDC_UART_IRQ,2);
	NVIC_EnableIRQ(CDC_UART_IRQ);
}


/**************************************************************************/
/*!
   UART Interrupt Handlers.
*/
/**************************************************************************/
void CDC_IRQ(void)
{
	if (USART_GetITStatus(CDC_UART, USART_IT_RXNE) != RESET)
	{
		/* Send the received data to the PC Host */
		USART_To_USB_Send_Data();
	}

	/* If overrun condition occurs, clear the ORE flag and recover communication */
	if (USART_GetFlagStatus(CDC_UART, USART_FLAG_ORE) != RESET)
	{
		(void)USART_ReceiveData(CDC_UART);
	}
 }


/**************************************************************************/
/*!
    Main CommunicationDeviceClass Task Routine.
*/
/**************************************************************************/
void cdc_task(void)
{
	/* Set Available SystemClock to 72MHz for USB Functions */
	SetSysClock72();
	
	/* Retrieve SystemClock Frequency and reset SysTick */
	SysTickInit(INTERVAL);

	/* Init Display Driver and FONTX Driver */
	Display_init_if();
	InitFont_Ank(&ANKFONT,font_table_ank);
	ChangeCurrentAnk(&ANKFONT);

	/* Diaplay CDC mode message */
	Display_clear_if();
	Display_Puts_If(0,0,(uint8_t*)"Start Virtual COM",TRANSPARENT);
	Display_Puts_If(0,1*CurrentAnkDat->Y_Size,(uint8_t*)("System Version:"APP_VERSION),TRANSPARENT);

	/* USB-CDC Configurations */
	USB_Disconnect_Config();
	USB_Cable_Config(DISABLE); /* fool ploof */

	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
	USB_Cable_Config(ENABLE);
	
	/* Wait CONFIGURED state */
	while (bDeviceState != CONFIGURED)
	{
		WDT_Reset();
	}

	/* Main loop */
	while (1){
		__WFI();
		WDT_Reset();
	}

}

/* End Of File ---------------------------------------------------------------*/
