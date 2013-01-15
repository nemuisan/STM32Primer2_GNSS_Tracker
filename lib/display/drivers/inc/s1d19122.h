/********************************************************************************/
/*!
	@file			s1d19122.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2012.01.15
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive STM025QVT-001 TFT module(8/16bit).

    @section HISTORY
		2010.10.01	V1.00	Stable Release
		2010.12.31	V2.00	Added GRAM write function.
		2011.03.10	V3.00	C++ Ready.
		2011.05.27	V4.00	Fixed S1D19122_clear() in 8-bit access.
		2011.10.25	V5.00	Added DMA TransactionSupport.
		2011.12.23	V6.00	Optimize Some Codes.
		2012.01.16	V7.00	Fixed Startup Failure.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __S1D19122_H
#define __S1D19122_H	0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S1D19122 unique value */
/* mst be need for S1D19122 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				320
#define MAX_Y				240

/* Display Contol Macros */
#define S1D19122_RES_SET()	DISPLAY_RES_SET()
#define S1D19122_RES_CLR()	DISPLAY_RES_CLR()
#define S1D19122_CS_SET()	DISPLAY_CS_SET()
#define S1D19122_CS_CLR()	DISPLAY_CS_CLR()
#define S1D19122_DC_SET()	DISPLAY_DC_SET()
#define S1D19122_DC_CLR()	DISPLAY_DC_CLR()
#define S1D19122_WR_SET()	DISPLAY_WR_SET()
#define S1D19122_WR_CLR()	DISPLAY_WR_CLR()
#define S1D19122_RD_SET()	DISPLAY_RD_SET()
#define S1D19122_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S1D19122_WR()   	S1D19122_WR_CLR(); \
							S1D19122_WR_SET();
#else
 #define S1D19122_WR()
#endif

#define	S1D19122_DATA		DISPLAY_DATAPORT
#define S1D19122_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S1D19122_reset(void);
extern void S1D19122_init(void);
extern void S1D19122_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S1D19122_wr_cmd(uint8_t cmd);
extern void S1D19122_wr_dat(uint8_t dat);
extern void S1D19122_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S1D19122_clear(void);
extern int  S1D19122_draw_bmp(const uint8_t* ptr);
extern uint16_t S1D19122_rd_cmd(uint8_t cmd);
extern void S1D19122_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			S1D19122_init
#define Display_rect_if 		S1D19122_rect
#define Display_wr_dat_if		S1D19122_wr_gram
#define Display_wr_cmd_if		S1D19122_wr_cmd
#define Display_wr_block_if		S1D19122_wr_block
#define Display_clear_if 		S1D19122_clear
#define Display_draw_bmp_if		S1D19122_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __S1D19122_H */
