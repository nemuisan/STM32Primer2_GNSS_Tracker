/********************************************************************************/
/*!
	@file			display_if_basis.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.06.01
	@brief          Interface of Display Device Basics for STM32 Primer2.	@n
					"MCU Depend Layer"

    @section HISTORY
		2010.07.02	V1.00	Restart Here.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Changed Some.
		2011.03.10	V4.00	C++ Ready.
		2023.04.21	V5.00	Fixed cosmetic bugfix.
		2023.04.21	V6.00	Removed unused delay macro.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef DISPLAY_IF_BASIS_H
#define DISPLAY_IF_BASIS_H 0x0600

#ifdef __cplusplus
 extern "C" {
#endif

/* Device or MCU Depend Includes */
#include "stm32f10x.h"
#include "systick.h"


/* ST7732 TYPE PORT SETTINGS */
#if defined(USE_ST7732_TFT)
/* STM32F103VET6(STM32 Primer2) FSMC Bus BANK3 Definition */
/* PORT :  15| 14| 13| 12| 11| 10|  9|  8|  7|  6|  5|  4|  3|  2|  1|  0 */ 
/* GPIOE: ---|D7 |D6 |D5 |D4 |D3 |D2 |D1 |D0 |---|---|---|---|---|---|--- */
/* GPIOD: ---|---|---|---|DC |---|---|---|CS |RES|WR |RD |---|---|---|--- */

/*CTRL-Definitions*/
#define CTRL_RD					GPIO_Pin_4
#define CTRL_WR					GPIO_Pin_5
#define CTRL_CS					GPIO_Pin_7
#define CTRL_DC					GPIO_Pin_11
#define CTRL_RES				GPIO_Pin_6

#define DISPLAY_PORT_RES		GPIOD
#define DISPLAY_PORT_CS			GPIOD
#define DISPLAY_PORT_DC			GPIOD
#define DISPLAY_PORT_WR			GPIOD
#define DISPLAY_PORT_RD			GPIOD
#define DISPLAY_CLK_RES			RCC_APB2Periph_GPIOD
#define DISPLAY_CLK_CS			RCC_APB2Periph_GPIOD
#define DISPLAY_CLK_DC			RCC_APB2Periph_GPIOD
#define DISPLAY_CLK_WR			RCC_APB2Periph_GPIOD
#define DISPLAY_CLK_RD			RCC_APB2Periph_GPIOD

#define DISPLAY_RES_SET()		(DISPLAY_PORT_RES->BSRR = CTRL_RES)
#define DISPLAY_RES_CLR()		(DISPLAY_PORT_RES->BRR  = CTRL_RES)
#define DISPLAY_CS_SET()		(DISPLAY_PORT_CS->BSRR  = CTRL_CS)
#define DISPLAY_CS_CLR()        (DISPLAY_PORT_CS->BRR   = CTRL_CS)
#define DISPLAY_DC_SET()		(DISPLAY_PORT_DC->BSRR  = CTRL_DC)
#define DISPLAY_DC_CLR()        (DISPLAY_PORT_DC->BRR   = CTRL_DC)
#define DISPLAY_WR_SET()		(DISPLAY_PORT_WR->BSRR  = CTRL_WR)
#define DISPLAY_WR_CLR()        (DISPLAY_PORT_WR->BRR   = CTRL_WR)
#define DISPLAY_RD_SET()		(DISPLAY_PORT_RD->BSRR  = CTRL_RD)
#define DISPLAY_RD_CLR()        (DISPLAY_PORT_RD->BRR   = CTRL_RD)

/*DATA-Definitions*/
#define DISPLAY_PORT_DATA		GPIOE
#define DISPLAY_CLK_DATA		RCC_APB2Periph_GPIOE

/* LCD addresses as seen by the FSMC*/
#define LCD_DATA_MODE_ADDR   	((uint32_t)0x68020000)
#define LCD_CMD_MODE_ADDR   	((uint32_t)0x68000000)

/* LCD is connected to the FSMC_Bank1_NOR/SRAM3 and NE3 is used as ship select signal */
#define DISPLAY_DATAPORT		(*(volatile uint16_t*)LCD_DATA_MODE_ADDR)
#define DISPLAY_CMDPORT			(*(volatile uint16_t*)LCD_CMD_MODE_ADDR)

/* Define Acess Procedure */
#define BUS_ACCESS_8BIT
#define LCD_D0          		GPIO_Pin_7		/* FSMC_D4 */
#define LCD_D1         			GPIO_Pin_8		/* FSMC_D5 */
#define LCD_D2          		GPIO_Pin_9		/* FSMC_D6 */
#define LCD_D3          		GPIO_Pin_10		/* FSMC_D7 */
#define LCD_D4          		GPIO_Pin_11		/* FSMC_D8 */
#define LCD_D5          		GPIO_Pin_12		/* FSMC_D9 */
#define LCD_D6          		GPIO_Pin_13		/* FSMC_D10 */
#define LCD_D7          		GPIO_Pin_14		/* FSMC_D11 */
#define DATA_PINS   			( LCD_D0 | LCD_D1 | LCD_D2 | LCD_D3 | LCD_D4 | LCD_D5 | LCD_D6 | LCD_D7 )

#define ReadLCDData(x)					\
	x = DISPLAY_DATAPORT;

#endif


/* RGB565 Colour Structure */
/* e.g RED-PINK        | Blue     | Green           | Red               */
/* #define FrontG      ((50 >> 3) |((50 >> 2) << 5) |((255 >> 3) << 11))*/
#if !defined(USE_SSD1332_SPI_OLED)
	#define RGB(r,g,b)	(uint16_t)((b >> 3) |((g >> 2) << 5) |((r >> 3) << 11))
#else
	#define RGB(r,g,b)	(uint16_t)((r >> 3) |((g >> 2) << 5) |((b >> 3) << 11))
#endif


#define FrontG		COL_REDPINK
#define BackG   	COL_BLACK

#define COL_BLACK	RGB(0,0,0)
#define COL_WHITE	RGB(255,255,255)
#define COL_RED		RGB(255,0,0)
#define COL_BLUE	RGB(0,0,255)
#define COL_GREEN	RGB(0,255,0)

#define COL_YELLOW	RGB(255,255,0)
#define COL_MAGENTA	RGB(255,0,255)
#define COL_AQUA	RGB(0,255,255)

#define COL_PURPLE	RGB(160,32,240)
#define COL_REDPINK RGB(255,50,50)
#define COL_ORANGE  RGB(255,165,0)

/* Potiner Access Macro(Little Endian) */
#define	LD_UINT16(ptr)		(uint16_t)(((uint16_t)*(uint8_t*)((ptr)+1)<<8)|(uint16_t)*(uint8_t*)(ptr))
#define	LD_UINT32(ptr)		(uint32_t)(((uint32_t)*(uint8_t*)((ptr)+3)<<24)|((uint32_t)*(uint8_t*)((ptr)+2)<<16)|((uint16_t)*(uint8_t*)((ptr)+1)<<8)|*(uint8_t*)(ptr))

/* Function Prototypes */
extern void Display_IoInit_If(void);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_IF_BASIS_H */

