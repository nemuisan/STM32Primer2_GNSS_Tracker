/********************************************************************************/
/*!
	@file			spfd54124.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2012.05.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -H179IT01-V1			(3-wire & 9-bit serial only!)		@n
					 -Nokia Modules			(3-wire & 9-bit serial only!)		@n
					  C1-01,166x,1610,1800,5030									@n
					  610x,5200,6060,6070,6080,6125,7360

    @section HISTORY
		2011.12.23	V1.00	Stable Release.
		2012.04.22  V2.00	Added Nokia LCD-Module Supports.
		2012.05.25  V3.00	Added Hardware 9-bitSerial Handling.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SPFD54124_H
#define __SPFD54124_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* Module Driver Configure */
/* U MUST select one from those modules */
//#define USE_NOKIA_C101_166x_1610_1800_5030
#define USE_NOKIA_160x_5200_6060_6080_6125_7360


/* SPFD54124 Unique Value		*/
#if defined(USE_NOKIA_C101_166x_1610_1800_5030)
#define OFS_COL			2
#define OFS_RAW			1
#define MAX_X			128
#define MAX_Y			160
#define USEMODULE		NOKIA_C101_166x_1610_1800_5030
#define SPFD54124SPI_3WIREMODE

#elif  defined(USE_NOKIA_160x_5200_6060_6080_6125_7360)
#define OFS_COL			0
#define OFS_RAW			0
#define MAX_X			132
#define MAX_Y			162
#define USEMODULE		NOKIA_160x_5200_6060_6080_6125_7360
#define SPFD54124SPI_3WIREMODE

#endif


#ifndef USE_SPFD54124_TFT

#ifdef SPFD54124SPI_3WIREMODE
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
#endif

#else /* 4WIRE-8BitMODE */
 #if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
  #error "4-WireMode Supports 8bit-SPI Handling ONLY !!"
 #endif
 #define DNC_CMD()	
 #define DNC_DAT()	
#endif


/* Display Contol Macros */
#define SPFD54124_RES_SET()		DISPLAY_RES_SET()
#define SPFD54124_RES_CLR()		DISPLAY_RES_CLR()
#define SPFD54124_CS_SET()		DISPLAY_CS_SET()
#define SPFD54124_CS_CLR()		DISPLAY_CS_CLR()
#if 	defined(SPFD54124SPI_3WIREMODE)
 #define SPFD54124_DC_SET()
 #define SPFD54124_DC_CLR()
#elif 	defined(SPFD54124SPI_4WIREMODE)
 #define SPFD54124_DC_SET()		DISPLAY_DC_SET()
 #define SPFD54124_DC_CLR()		DISPLAY_DC_CLR()
#else
 #error "U MUST Select SPFD54124 SPI Mode!!"
#endif
#define SPFD54124_WR_SET()		DISPLAY_WR_SET()
#define SPFD54124_WR_CLR()		DISPLAY_WR_CLR()
#define SPFD54124_RD_SET()		DISPLAY_RD_SET()
#define SPFD54124_RD_CLR()		DISPLAY_RD_CLR()
#define SPFD54124_SDATA_SET()	DISPLAY_SDI_SET()
#define SPFD54124_SDATA_CLR()	DISPLAY_SDI_CLR()
#define SPFD54124_SCLK_SET()	DISPLAY_SCK_SET()
#define SPFD54124_SCLK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SPFD54124_WR()   	SPFD54124_WR_CLR(); \
							SPFD54124_WR_SET();
#else
 #define SPFD54124_WR()
#endif

#define	SPFD54124_DATA		DISPLAY_DATAPORT
#define SPFD54124_CMD		DISPLAY_CMDPORT


/* SPFD54124 Instruction */
#define NO_OP		(0x00)
#define SWRESET		(0x01)
#define RDDID		(0x04)
#define RDDST		(0x09)
#define RDDPM		(0x0A)
#define RDDMADCTL	(0x0B)
#define RDDCOLMOD	(0x0C)
#define RDDIM		(0x0D)
#define RDDSM		(0x0E)
#define SLPIN		(0x10)
#define SLPOUT		(0x11)
#define PTLON		(0x12)
#define NORON		(0x13)
#define INVOFF		(0x20)
#define INVON		(0x21)
#define GAMSET		(0x26)
#define DISPOFF		(0x28)
#define DISPON		(0x29)
#define CASET		(0x2A)
#define RASET		(0x2B)
#define RAMWR		(0x2C)
#define RAMRD		(0x2E)
#define PTLAR		(0x30)
#define TEOFF		(0x34)
#define TEON		(0x35)
#define MADCTL		(0x36)
#define IDMOFF		(0x38)
#define IDMON		(0x39)
#define COLMOD		(0x3A)
#define RDID1		(0xDA)
#define RDID2		(0xDB)
#define RDID3		(0xDC)
#define RDID4		(0xD3)

/* SPFD54124 Panel Function Command List */
#define FRMCTR1		(0xB1)
#define FRMCTR2		(0xB2)
#define FRMCTR3		(0xB3)
#define INVCTR		(0xB4)
#define DISSET5		(0xB6)
#define PWCTR1		(0xC0)
#define PWCTR2		(0xC1)
#define PWCTR3		(0xC2)
#define PWCTR4		(0xC3)
#define PWCTR5		(0xC4)
#define VMCTR1		(0xC5)
#define VMCTR2		(0xC6)
#define VMOFCTR		(0xC7)
#define WRID2		(0xD1)
#define WRID3		(0xD2)
#define PWCTR6		(0xFC)
#define NVFCTR1		(0xD9)
#define NVFCTR2		(0xDE)
#define NVFCTR3		(0xDF)
#define GAMCTRP1	(0xE0)
#define GAMCTRN1	(0xE1)
#define EXTCTRL		(0xF0)
#define VCOM4L		(0xFF)


/* Display Control Functions Prototype */
extern void SPFD54124_reset(void);
extern void SPFD54124_init(void);
extern void SPFD54124_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SPFD54124_wr_cmd(uint8_t cmd);
extern void SPFD54124_wr_dat(uint8_t dat);
extern void SPFD54124_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SPFD54124_clear(void);
extern int  SPFD54124_draw_bmp(const uint8_t* ptr);
extern void SPFD54124_wr_gram(uint16_t gram);
extern uint16_t SPFD54124_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SPFD54124_init
#define Display_rect_if 		SPFD54124_rect
#define Display_wr_dat_if		SPFD54124_wr_gram
#define Display_wr_cmd_if		SPFD54124_wr_cmd
#define Display_wr_block_if		SPFD54124_wr_block
#define Display_clear_if 		SPFD54124_clear
#define Display_draw_bmp_if		SPFD54124_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SPFD54124_H */
