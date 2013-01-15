/********************************************************************************/
/*!
	@file			uart_support_gps.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.31
	@brief          For STM32 Primer2(USART2).

    @section HISTORY
		2012.01.31	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __UART_SUPPORT_GPS_H
#define __UART_SUPPORT_GPS_H

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
#define UART_BUFSIZE		256
#define UART_BAUDLATE		230400UL
/*#define UART_HANDLING		UART_POLLING_MODE*/
#define UART_HANDLING		UART_INTERRUPT_MODE

#define UART_DEFAULT_NUM	2


/* General Definition */
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Funcion Prototypes */
extern void conio_init(uint32_t port, uint32_t baudrate);
extern void putch(uint8_t c);
extern uint8_t getch(void);
extern uint8_t keypressed(void);
extern void cputs(char *s);
extern void cgets(char *s, int bufsize);
extern void conio_IRQ(void);
extern void (*xUART_IRQ)(void);

/* Structs of UART(This is From AVRX uC Sample!!!) */
/* @brief USART transmit and receive ring buffer. */
typedef struct USART_Buffer
{
	/* @brief Receive buffer. */
	volatile uint8_t RX[UART_BUFSIZE];
	/* @brief Transmit buffer. */
	volatile uint8_t TX[UART_BUFSIZE];
	/* @brief Receive buffer head. */
	volatile uint16_t RX_Head;
	/* @brief Receive buffer tail. */
	volatile uint16_t RX_Tail;
	/* @brief Transmit buffer head. */
	volatile uint16_t TX_Head;
	/* @brief Transmit buffer tail. */
	volatile uint16_t TX_Tail;
} USART_Buffer_t;

/* Externs */
extern USART_InitTypeDef USART_InitStructure;
extern USART_Buffer_t USART1_Buf;
extern USART_Buffer_t USART2_Buf;


#ifdef __cplusplus
}
#endif

#endif	/* __UART_SUPPORT_GPS_H */
