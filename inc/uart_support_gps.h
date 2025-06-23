/********************************************************************************/
/*!
	@file			uart_support_gps.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2025.06.18
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
		2025.06.18	V9.00	Fixed implicit cast warnings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef UART_SUPPORT_GPS_H
#define UART_SUPPORT_GPS_H	0x0900

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

/* uC Specific Inclusion */
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "stm32f10x_conf.h"

/* USART Definition */
#define UART_BUFSIZE		512		/* Buffer size MUST Takes power of 2(64,128,256,512...) */
#define UART_BAUDRATE		230400UL
#define UART_INTERRUPT_MODE			/* If u want polling mode, uncomment this */

#define UART_DEFAULT_NUM	2
#define USARTx_Buf			USART2_Buf
#define USARTx_IRQHandler	USART2_IRQHandler


/* General Definition */
#define countof(a)			(sizeof(a) / sizeof(*(a)))

/* Funcion Prototypes */
extern void conio_init(uint32_t port, uint32_t baudrate);
extern void putch(uint8_t c);
extern uint8_t getch(void);
extern uint8_t keypressed(void);
extern void cputs(char *s);
extern void cgets(char *s, int bufsize);
extern void conio_IRQ(void);
extern void (*xUART_IRQ)(void);
extern void Flush_RXBuffer(void);
extern uint8_t WaitTxBuffer(void);

/* Structs of UART(This is Based on AVRX uC Sample!!!) */
/* @brief USART transmit and receive ring buffer. */
typedef struct USART_Buffer
{
	/* @brief Receive buffer. */
	volatile uint8_t RX[UART_BUFSIZE];
	/* @brief Transmit buffer. */
	volatile uint8_t TX[UART_BUFSIZE];
	/* @brief Receive buffer head. */
	volatile unsigned int RX_Head;
	/* @brief Receive buffer tail. */
	volatile unsigned int RX_Tail;
	/* @brief Transmit buffer head. */
	volatile unsigned int TX_Head;
	/* @brief Transmit buffer tail. */
	volatile unsigned int TX_Tail;
} USART_Buffer_t;

/* Externs */
extern USART_InitTypeDef USART_InitStructure;
extern USART_Buffer_t USARTx_Buf;


#ifdef __cplusplus
}
#endif

#endif	/* UART_SUPPORT_GPS_H */
