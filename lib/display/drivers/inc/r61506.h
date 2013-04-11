/********************************************************************************/
/*!
	@file			r61506.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive T-51991GD015-MLW-AIN	TFT module(8bit mode).

    @section HISTORY
		2011.11.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __R61506_H
#define __R61506_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* R61506 unique value */
/* mst be need for R61506 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				173
#define MAX_Y				132

/* Display Contol Macros */
#define R61506_RES_SET()	DISPLAY_RES_SET()
#define R61506_RES_CLR()	DISPLAY_RES_CLR()
#define R61506_CS_SET()		DISPLAY_CS_SET()
#define R61506_CS_CLR()		DISPLAY_CS_CLR()
#define R61506_DC_SET()		DISPLAY_DC_SET()
#define R61506_DC_CLR()		DISPLAY_DC_CLR()
#define R61506_WR_SET()		DISPLAY_WR_SET()
#define R61506_WR_CLR()		DISPLAY_WR_CLR()
#define R61506_RD_SET()		DISPLAY_RD_SET()
#define R61506_RD_CLR()		DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define R61506_WR()   		R61506_WR_CLR(); \
							R61506_WR_SET();
#else
 #define R61506_WR()
#endif

#define	R61506_DATA			DISPLAY_DATAPORT
#define R61506_CMD			DISPLAY_CMDPORT

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

/* Display Control Functions Prototype */
extern void R61506_reset(void);
extern void R61506_init(void);
extern void R61506_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void R61506_wr_cmd(uint8_t cmd);
extern void R61506_wr_dat(uint16_t dat);
extern void R61506_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void R61506_clear(void);
extern int  R61506_draw_bmp(const uint8_t* ptr);
extern uint16_t R61506_rd_cmd(uint8_t cmd);
extern void R61506_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			R61506_init
#define Display_rect_if 		R61506_rect
#define Display_wr_dat_if		R61506_wr_gram
#define Display_wr_cmd_if		R61506_wr_cmd
#define Display_wr_block_if		R61506_wr_block
#define Display_clear_if 		R61506_clear
#define Display_draw_bmp_if		R61506_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __R61506_H */
