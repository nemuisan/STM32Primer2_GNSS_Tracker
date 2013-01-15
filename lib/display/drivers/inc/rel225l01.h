/********************************************************************************/
/*!
	@file			rel225l01.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.12.23
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					and Based on y_notsu's QVGA-TFT demo thanks!				@n
					http://mbed.org/users/y_notsu/programs/QVGA_TFT_test/lldy66 @n
						Available TFT-LCM are listed below.						@n
					 -REL225L01						(REL225L01)	8/16bit mode.	@n
					 -FPC192GC00					(REL225L01) 8/16bit mode.   @n

    @section HISTORY
		2011.09.30	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __REL225L01_H
#define __REL225L01_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* REL225L01 unique value */
/* mst be need for REL225L01 */
#define OFS_COL				0x0C
#define OFS_RAW				0x08
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define REL225L01_RES_SET()	DISPLAY_RES_SET()
#define REL225L01_RES_CLR()	DISPLAY_RES_CLR()
#define REL225L01_CS_SET()	DISPLAY_CS_SET()
#define REL225L01_CS_CLR()	DISPLAY_CS_CLR()
#define REL225L01_DC_SET()	DISPLAY_DC_SET()
#define REL225L01_DC_CLR()	DISPLAY_DC_CLR()
#define REL225L01_WR_SET()	DISPLAY_WR_SET()
#define REL225L01_WR_CLR()	DISPLAY_WR_CLR()
#define REL225L01_RD_SET()	DISPLAY_RD_SET()
#define REL225L01_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define REL225L01_WR()   	REL225L01_WR_CLR(); \
							REL225L01_WR_SET();
#else
 #define REL225L01_WR()
#endif

#define	REL225L01_DATA		DISPLAY_DATAPORT
#define REL225L01_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void REL225L01_reset(void);
extern void REL225L01_init(void);
extern void REL225L01_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void REL225L01_wr_cmd(uint8_t cmd);
extern void REL225L01_wr_dat(uint8_t dat);
extern void REL225L01_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void REL225L01_clear(void);
extern int  REL225L01_draw_bmp(const uint8_t* ptr);
extern uint16_t REL225L01_rd_cmd(uint8_t cmd);
extern void REL225L01_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			REL225L01_init
#define Display_rect_if 		REL225L01_rect
#define Display_wr_dat_if		REL225L01_wr_gram
#define Display_wr_cmd_if		REL225L01_wr_cmd
#define Display_wr_block_if		REL225L01_wr_block
#define Display_clear_if 		REL225L01_clear
#define Display_draw_bmp_if		REL225L01_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __REL225L01_H */
