/********************************************************************************/
/*!
	@file			st7735.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -JD-T18003-T01				(4-wire serial)					@n
					 -ZY-FGD1442701V1			(8bit mode)						@n
					 -HD177P2501				(8bit mode)						@n

    @section HISTORY
		2010.03.01	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added GRAM write function.
		2011.01.20  V4.00   Added Confirmation RDDID1
		                    Added 4-Wire Serial Support.
		2011.03.10	V5.00	C++ Ready.
		2011.06.10	V6.00	HD177P2501 support.
		2011.10.25	V7.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ST7735_H
#define __ST7735_H	0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ST7735 Unique Value		*/
/* MUST be need for ST7735	*/
#if defined(USE_ST7735_SPI_TFT)		/* For JD-T18003-T01   module */
	#define OFS_COL			2
	#define OFS_RAW			1
	#define MAX_X			128
	#define MAX_Y			160
	#define MADVAL 			0xC8
#elif defined(USE_TOUCH_CTRL)	/* For HD177P2501 (with TouchPanel model) module */
	#define OFS_COL			0
	#define OFS_RAW			0
	#define MAX_X			128
	#define MAX_Y			160
	#define MADVAL 			0xC5
#else 								/* For ZY-FGD1442701V1 module */
	#define OFS_COL			2
	#define OFS_RAW			3
	#define MAX_X			128
	#define MAX_Y			128
	#define MADVAL 			0xC8
#endif

/* Display Contol Macros */
#define ST7735_RES_SET()	DISPLAY_RES_SET()
#define ST7735_RES_CLR()	DISPLAY_RES_CLR()
#define ST7735_CS_SET()		DISPLAY_CS_SET()
#define ST7735_CS_CLR()		DISPLAY_CS_CLR()
#define ST7735_DC_SET()		DISPLAY_DC_SET()
#define ST7735_DC_CLR()		DISPLAY_DC_CLR()
#define ST7735_WR_SET()		DISPLAY_WR_SET()
#define ST7735_WR_CLR()		DISPLAY_WR_CLR()
#define ST7735_RD_SET()		DISPLAY_RD_SET()
#define ST7735_RD_CLR()		DISPLAY_RD_CLR()
#define ST7735_SDATA_SET()	DISPLAY_SDI_SET()
#define ST7735_SDATA_CLR()	DISPLAY_SDI_CLR()
#define ST7735_SCLK_SET()	DISPLAY_SCK_SET()
#define ST7735_SCLK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ST7735_WR()   		ST7735_WR_CLR(); \
							ST7735_WR_SET();
#else
 #define ST7735_WR()
#endif

#define	ST7735_DATA			DISPLAY_DATAPORT
#define ST7735_CMD			DISPLAY_CMDPORT


/* ST7735 Instruction */
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

/* ST7735 Panel Function Command List */
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
extern void ST7735_reset(void);
extern void ST7735_init(void);
extern void ST7735_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ST7735_wr_cmd(uint8_t cmd);
extern void ST7735_wr_dat(uint8_t dat);
extern void ST7735_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ST7735_clear(void);
extern int  ST7735_draw_bmp(const uint8_t* ptr);
extern void ST7735_wr_gram(uint16_t gram);
extern uint16_t ST7735_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			ST7735_init
#define Display_rect_if 		ST7735_rect
#define Display_wr_dat_if		ST7735_wr_gram
#define Display_wr_cmd_if		ST7735_wr_cmd
#define Display_wr_block_if		ST7735_wr_block
#define Display_clear_if 		ST7735_clear
#define Display_draw_bmp_if		ST7735_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ST7735_H */
