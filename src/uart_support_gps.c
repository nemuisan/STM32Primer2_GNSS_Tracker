/********************************************************************************/
/*!
	@file			uart_support_gps.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        8.00
    @date           2025.04.21
	@brief          For STM32 Primer2(USART2).

    @section HISTORY
		2012.01.31	V1.00	Start Here.
		2013.02.20	V2.00	Added RX/TX Buffer Consideration.
		2014.04.20	V3.00	Fixed Suitable Interruption level.
		2015.01.11	V4.00	Added buffered UART information.
		2015.08.25	V5.00	Fixed wrong expression.
		2022.10.10	V6.00	Fixed more robustness.
		2023.03.07	V7.00	Fixed cosmetic bugfixes.
		2025.04.21	V8.00	Fixed UART Rx-Pin to pullup.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "uart_support_gps.h"
/* check header file version for fool proof */
#if UART_SUPPORT_GPS_H!= 0x0800
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure ={0};
static USART_TypeDef* UART;
static USART_Buffer_t* pUSART_Buf;
USART_Buffer_t USARTx_Buf;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
void (*xUART_IRQ)(void);

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Initialize UART.
*/
/**************************************************************************/
/* Initialize serial console */
void conio_init(uint32_t port, uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure = {0};
  
	/* Turn on USART*/
	switch (port)
	{
		case 1 : /* NOT Supported */
		break;
		
		case 2 :
			UART = (USART_TypeDef *) USART2_BASE;
			
#if defined (USE_STM3210B_EVAL) || defined (USE_STM3210C_EVAL) || \
	defined (USE_GOLDBULL)      || defined (USE_CQ_STARM_COMP) || defined (USE_ETHERPOD)
			/* Turn on peripheral clocks */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_AFIO, ENABLE);
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			
			/* Enable the USART2 Pins Software Remapping */
			GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);
			
			/* Configure USART2 TX as alternate function push-pull */
			GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_5;
			GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF_PP;
			GPIO_Init(GPIOD, &GPIO_InitStructure);
			
			/* Configure USART2 RX as alternate function in pullup */
			GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6;
			GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPU;
			GPIO_Init(GPIOD, &GPIO_InitStructure);
			
#else	/* defined (USE_STM32PRIMER2) */
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
#endif
			/* USART Settings */
			USART_StructInit(&USART_InitStructure);
			USART_InitStructure.USART_BaudRate = baudrate;
			USART_Init(UART, &USART_InitStructure);
			
#if defined(UART_INTERRUPT_MODE)
			/* Enable USART Interrupt */
			NVIC_SetPriority(USART2_IRQn,2);
			NVIC_EnableIRQ(USART2_IRQn);
			
			/* Init Ring Buffer */
			pUSART_Buf = &USARTx_Buf;
			USARTx_Buf.RX_Tail = 0;
			USARTx_Buf.RX_Head = 0;
			USARTx_Buf.TX_Tail = 0;
			USARTx_Buf.TX_Head = 0;
			
			/* Enable USART Receive interrupts */
			USART_ITConfig(UART, USART_IT_RXNE, ENABLE);
#endif
			/* Enable USART */
			USART_Cmd(UART, ENABLE);
		break;
		
		case 3 : /* NOT Supported */
		break;
		
	}
}

/**************************************************************************/
/*! 
    Check UART TX Buffer Empty.
*/
/**************************************************************************/
bool USART_TXBuffer_FreeSpace(USART_Buffer_t* USART_buf)
{
	/* Make copies to make sure that volatile access is specified. */
	unsigned int tempHead = (USART_buf->TX_Head + 1) & (UART_BUFSIZE-1);
	unsigned int tempTail = USART_buf->TX_Tail;

	/* There are data left in the buffer unless Head and Tail are equal. */
	return (tempHead != tempTail);
}

/**************************************************************************/
/*! 
    Put Bytedata with Buffering.
*/
/**************************************************************************/
bool USART_TXBuffer_PutByte(USART_Buffer_t* USART_buf, uint8_t data)
{

	unsigned int tempTX_Head;
	bool TXBuffer_FreeSpace;

	TXBuffer_FreeSpace = USART_TXBuffer_FreeSpace(USART_buf);


	if(TXBuffer_FreeSpace)
	{
	  	tempTX_Head = USART_buf->TX_Head;
		
		__disable_irq();
	  	USART_buf->TX[tempTX_Head]= data;
		/* Advance buffer head. */
		USART_buf->TX_Head = (tempTX_Head + 1) & (UART_BUFSIZE-1);
		__enable_irq();
		
		/* Enable TXE interrupt. */
		USART_ITConfig(UART, USART_IT_TXE, ENABLE);
	}
	return TXBuffer_FreeSpace;
}

/**************************************************************************/
/*! 
    Check UART RX Buffer Empty.
*/
/**************************************************************************/
bool USART_RXBufferData_Available(USART_Buffer_t* USART_buf)
{
	/* Make copies to make sure that volatile access is specified. */
	unsigned int tempHead = USART_buf->RX_Head;
	unsigned int tempTail = USART_buf->RX_Tail;

	/* There are data left in the buffer unless Head and Tail are equal. */
	return (tempHead != tempTail);
}

/**************************************************************************/
/*! 
    Get Bytedata with Buffering.
*/
/**************************************************************************/
uint8_t USART_RXBuffer_GetByte(USART_Buffer_t* USART_buf)
{
	uint8_t ans;

	__disable_irq();
	ans = (USART_buf->RX[USART_buf->RX_Tail]);

	/* Advance buffer tail. */
	USART_buf->RX_Tail = (USART_buf->RX_Tail + 1) & (UART_BUFSIZE-1);
	
	__enable_irq();

	return ans;
}

/**************************************************************************/
/*! 
    High Level function.
*/
/**************************************************************************/
/* Send 1 character */
inline void putch(uint8_t data)
{
#if defined(UART_INTERRUPT_MODE)
	/* Interrupt Version */
	while(!USART_TXBuffer_FreeSpace(pUSART_Buf));
	USART_TXBuffer_PutByte(pUSART_Buf,data);
#else 
	/* Polling version */
	while (!(UART->SR & USART_FLAG_TXE));
	UART->DR = data;
#endif
}

/**************************************************************************/
/*! 
    High Level function.
*/
/**************************************************************************/
/* Receive 1 character */
uint8_t getch(void)
{
#if defined(UART_INTERRUPT_MODE)
	if (USART_RXBufferData_Available(pUSART_Buf))  return USART_RXBuffer_GetByte(pUSART_Buf);
	else										   return false;
#else
	/* Polling version */
	while (!(UART->SR & USART_FLAG_RXNE));
	return (uint8_t)(UART->DR);
#endif
}

/**************************************************************************/
/*! 
    High Level function.
*/
/**************************************************************************/
/* Return 1 if key pressed */
uint8_t keypressed(void)
{
#if defined(UART_INTERRUPT_MODE)
	return (USART_RXBufferData_Available(pUSART_Buf));
#else
	return (UART->SR & USART_FLAG_RXNE);
#endif
}

/**************************************************************************/
/*! 
    High Level function.
*/
/**************************************************************************/
/* Send a string */
void cputs(char *s)
{
	while (*s)
		putch(*s++);
}

/**************************************************************************/
/*! 
    High Level function.
*/
/**************************************************************************/
/* Receive a string, with rudimentary line editing */
void cgets(char *s, int bufsize)
{
	char *p;
	int c;

	memset(s, 0, bufsize);

	p = s;

	for (p = s; p < s + bufsize-1;)
	{
		/* 20090521Nemui */
		do{		
			c = getch();
		}while(c == false);
		/* 20090521Nemui */
		switch (c)
		{
		  case '\r' :
		  case '\n' :
			putch('\r');
			putch('\n');
			*p = '\n';
			return;
			
		  case '\b' :
			if (p > s)
			{
			  *p-- = 0;
			  putch('\b');
			  putch(' ');
			  putch('\b');
			}
			break;
			
		  default :
			putch(c);
			*p++ = c;
			break;
		}
	}

}


#if defined(UART_INTERRUPT_MODE)
/**************************************************************************/
/*! 
    Interrupt handlers.
*/
/**************************************************************************/
void conio_IRQ(void)
{
	if(USART_GetITStatus(UART, USART_IT_RXNE) != RESET)
	{
		/* Advance buffer head. */
		unsigned int tempRX_Head = ((&USARTx_Buf)->RX_Head + 1) & (UART_BUFSIZE-1);
		
		/* Check for overflow. */
		unsigned int tempRX_Tail = (&USARTx_Buf)->RX_Tail;
		uint8_t data =  USART_ReceiveData(UART);
		
		if (tempRX_Head == tempRX_Tail) {
			/* Disable the UART Receive interrupt */
			USART_ITConfig(UART, USART_IT_RXNE, DISABLE);
		}else{
			(&USARTx_Buf)->RX[(&USARTx_Buf)->RX_Head] = data;
			(&USARTx_Buf)->RX_Head = tempRX_Head;
		}
	}

	if(USART_GetITStatus(UART, USART_IT_TXE) != RESET)
	{   
		/* Check if all data is transmitted. */
		unsigned int tempTX_Tail = (&USARTx_Buf)->TX_Tail;
		if ((&USARTx_Buf)->TX_Head == tempTX_Tail){
			/* Overflow MAX size Situation */
			/* Disable the UART Transmit interrupt */
			USART_ITConfig(UART, USART_IT_TXE, DISABLE);
		}else{
			/* Start transmitting. */
			uint8_t data = (&USARTx_Buf)->TX[(&USARTx_Buf)->TX_Tail];
			UART->DR = data;
			
			/* Advance buffer tail. */
			(&USARTx_Buf)->TX_Tail = ((&USARTx_Buf)->TX_Tail + 1) & (UART_BUFSIZE-1);
		}
	}
}

/**************************************************************************/
/*! 
    @brief	Handles USARTx global interrupt wrapper.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USARTx_IRQHandler(void)
{
	xUART_IRQ();
}

/**************************************************************************/
/*! 
    @brief	Flush USART RX Buffers.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Flush_RXBuffer(void)
{
	/* Init Ring Buffer */
	pUSART_Buf->RX_Tail = 0;
	pUSART_Buf->RX_Head = 0;

	/* Re-Enable UART Receive interrupts */
	USART_ITConfig(UART, USART_IT_RXNE, ENABLE);
}

/**************************************************************************/
/*! 
    @brief	Wait to Empty USART TX Buffers.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
uint8_t WaitTxBuffer(void)
{
	/* Return 1 If All Character send */
	unsigned int tempTX_Tail = pUSART_Buf->TX_Tail;
	return (pUSART_Buf->TX_Head == tempTX_Tail);
}
#endif
/* End Of File ---------------------------------------------------------------*/
