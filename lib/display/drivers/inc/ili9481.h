/********************************************************************************/
/*!
	@file			ili9481.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2013.04.06
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95517-AAA				(ILI9481)	16bit mode.			@n
					 -S95517-A035				(ILI9481)	8/16bit mode.		@n
					 -TFT1P2797-E				(ILI9481)	8/16bit mode.		@n
					 -XC-T0353701				(HX8357B)	8/16bit mode.		@n
					 -FTN35P02N-01				(ILI9481)	8/16bit mode &		@n
					                                        3,4Wire-SPI mode.   @n
					 -CNKT0350T37-11001A		(R61581B0)	8/16bit mode.		@n
					 -WK35039V0					(RM68042)	8/16bit mode.		@n
					 -TM035PDZ48				(ILI9486L)	8/16bit mode

    @section HISTORY
		2010.10.01	V1.00	Stable Release.
		2010.12.31	V2.00	Cleanup SourceCode.
		2011.03.10	V3.00	C++ Ready.
		2011.10.25	V4.00	Added DMA TransactionSupport.
		2012.01.02	V5.00	Added TFT1P2797-E Support.
		2012.07.20  V6.00	Added HX8357B Support.
		2012.11.30  V7.00	Added Serial Handling.
		2013.01.02  V8.00	Added R61581/B0 Devices Support.
							Added RM68042 Devices Support.
		2013.04.06  V9.00	Added ILI9486L Devices Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ILI9481_H
#define __ILI9481_H	0x0900

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ILI9481 unique value */
/* mst be need for ILI9481 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				320
#define MAX_Y				480

/* For TFT1P2797-E (with TouchPanel model ILI9481) module Force  */
/*#define USE_TFT1P2797_E*/
/* For TFT1P7134-E (with TouchPanel model R61581) module */
/*#define USE_TFT1P7134_E*/
#if !defined(USE_ILI9481_SPI_TFT)
 #if	defined(USE_TOUCH_CTRL)
	#define USE_TFT1P2797_E				/* For TFT1P2797 (with TouchPanel model) module */
 #else 									/* For S95517 ( NO TouchPanel & Normally Black)module */
 #endif
#endif

/* Serect MIPI-DBI TypeC Handlings */
#define ILI9481SPI_4WIREMODE
/*#define ILI9481SPI_3WIREMODE*/

/* Don't Touch This!! */
#if defined(USE_ILI9481_SPI_TFT)
#ifdef ILI9481SPI_3WIREMODE
#if defined(USE_HARDWARE_SPI)
 #if defined(SUPPORT_HARDWARE_9BIT_SPI)
  #define DNC_CMD()	
  #define DNC_DAT()	
 #else
  #error "9bit-SPI Does not Support on Hardware 8bit-SPI Handling !!"
 #endif
#elif defined(USE_SOFTWARE_SPI)
 #define DNC_CMD()			DISPLAY_SDI_CLR();	\
							CLK_OUT();
 #define DNC_DAT()			DISPLAY_SDI_SET();	\
							CLK_OUT();
#else
  #error "NOT Defined SPI Handling !"
#endif

#else /* 4WIRE-8BitMODE */
 #if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
  #error "4-WireMode Supports 8bit-SPI Handling ONLY !!"
 #endif
 #define DNC_CMD()	
 #define DNC_DAT()	
#endif
#endif

/* Display Contol Macros */
#define ILI9481_RES_SET()	DISPLAY_RES_SET()
#define ILI9481_RES_CLR()	DISPLAY_RES_CLR()
#define ILI9481_CS_SET()	DISPLAY_CS_SET()
#define ILI9481_CS_CLR()	DISPLAY_CS_CLR()
#define ILI9481_DC_SET()	DISPLAY_DC_SET()
#define ILI9481_DC_CLR()	DISPLAY_DC_CLR()
#define ILI9481_WR_SET()	DISPLAY_WR_SET()
#define ILI9481_WR_CLR()	DISPLAY_WR_CLR()
#define ILI9481_RD_SET()	DISPLAY_RD_SET()
#define ILI9481_RD_CLR()	DISPLAY_RD_CLR()
#define ILI9481_SCK_SET()	DISPLAY_SCK_SET()
#define ILI9481_SCK_CLR()	DISPLAY_SCK_CLR()
#define ILI9481_SDI_SET()	DISPLAY_SDI_SET()
#define ILI9481_SDI_CLR()	DISPLAY_SDI_CLR()
#define ILI9481_SDO_SET()	DISPLAY_SDO_SET()
#define ILI9481_SDO_CLR()	DISPLAY_SDO_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9481_WR()   	ILI9481_WR_CLR();ILI9481_WR_CLR(); \
							ILI9481_WR_SET();
#else
 #define ILI9481_WR()
#endif

#define	ILI9481_DATA		DISPLAY_DATAPORT
#define ILI9481_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI9481_reset(void);
extern void ILI9481_init(void);
extern void ILI9481_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9481_wr_cmd(uint8_t cmd);
extern void ILI9481_wr_dat(uint8_t dat);
extern void ILI9481_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9481_clear(void);
extern int  ILI9481_draw_bmp(const uint8_t* ptr);
extern uint16_t ILI9481_rd_cmd(uint8_t cmd);
extern void ILI9481_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			ILI9481_init
#define Display_rect_if 		ILI9481_rect
#define Display_wr_dat_if		ILI9481_wr_gram
#define Display_wr_cmd_if		ILI9481_wr_cmd
#define Display_wr_block_if		ILI9481_wr_block
#define Display_clear_if 		ILI9481_clear
#define Display_draw_bmp_if		ILI9481_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ILI9481_H */
