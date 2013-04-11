/********************************************************************************/
/*!
	@file			ST7787.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2013.04.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95417				(ST7787)	8/16bit mode.			@n
					 -AR240320A7NFWUG2		(ST7785)	8/16bit mode.			

    @section HISTORY
		2012.08.27	V1.00	Stable Release.
		2013.04.01  V2.00   Added ST7785 Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ST7787_H
#define __ST7787_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ST7787 unique value */
/* mst be need for ST7787 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define ST7787_RES_SET()	DISPLAY_RES_SET()
#define ST7787_RES_CLR()	DISPLAY_RES_CLR()
#define ST7787_CS_SET()		DISPLAY_CS_SET()
#define ST7787_CS_CLR()		DISPLAY_CS_CLR()
#define ST7787_DC_SET()		DISPLAY_DC_SET()
#define ST7787_DC_CLR()		DISPLAY_DC_CLR()
#define ST7787_WR_SET()		DISPLAY_WR_SET()
#define ST7787_WR_CLR()		DISPLAY_WR_CLR()
#define ST7787_RD_SET()		DISPLAY_RD_SET()
#define ST7787_RD_CLR()		DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ST7787_WR()   		ST7787_WR_CLR(); \
							ST7787_WR_SET();
#else
 #define ST7787_WR()
#endif

#define	ST7787_DATA			DISPLAY_DATAPORT
#define ST7787_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ST7787_reset(void);
extern void ST7787_init(void);
extern void ST7787_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ST7787_wr_cmd(uint8_t cmd);
extern void ST7787_wr_dat(uint8_t dat);
extern void ST7787_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ST7787_clear(void);
extern int  ST7787_draw_bmp(const uint8_t* ptr);
extern uint16_t ST7787_rd_cmd(uint8_t cmd);
extern void ST7787_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			ST7787_init
#define Display_rect_if 		ST7787_rect
#define Display_wr_dat_if		ST7787_wr_gram
#define Display_wr_cmd_if		ST7787_wr_cmd
#define Display_wr_block_if		ST7787_wr_block
#define Display_clear_if 		ST7787_clear
#define Display_draw_bmp_if		ST7787_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ST7787_H */
