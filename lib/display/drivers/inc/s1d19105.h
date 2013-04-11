/********************************************************************************/
/*!
	@file			s1d19105.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive FPC-SH9516-11A TFT module(8bit mode only).

    @section HISTORY
		2011.06.08	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __S1D19105_H
#define __S1D19105_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S1D19105 unique value */
/* mst be need for S1D19105 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				176
#define MAX_Y				220

/* Display Contol Macros */
#define S1D19105_RES_SET	DISPLAY_RES_SET
#define S1D19105_RES_CLR	DISPLAY_RES_CLR
#define S1D19105_CS_SET		DISPLAY_CS_SET
#define S1D19105_CS_CLR		DISPLAY_CS_CLR
#define S1D19105_DC_SET		DISPLAY_DC_SET
#define S1D19105_DC_CLR		DISPLAY_DC_CLR
#define S1D19105_WR_SET		DISPLAY_WR_SET
#define S1D19105_WR_CLR		DISPLAY_WR_CLR
#define S1D19105_RD_SET		DISPLAY_RD_SET
#define S1D19105_RD_CLR		DISPLAY_RD_CLR


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S1D19105_WR()   	S1D19105_WR_CLR(); \
							S1D19105_WR_SET();
#else
 #define S1D19105_WR()
#endif

#define	S1D19105_DATA		DISPLAY_DATAPORT
#define S1D19105_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S1D19105_reset(void);
extern void S1D19105_init(void);
extern void S1D19105_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S1D19105_wr_cmd(uint8_t cmd);
extern void S1D19105_wr_dat(uint8_t dat);
extern void S1D19105_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S1D19105_clear(void);
extern int  S1D19105_draw_bmp(const uint8_t* ptr);
extern void S1D19105_wr_gram(uint16_t gram);
extern uint8_t S1D19105_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			S1D19105_init
#define Display_rect_if 		S1D19105_rect
#define Display_wr_dat_if		S1D19105_wr_gram
#define Display_wr_cmd_if		S1D19105_wr_cmd
#define Display_wr_block_if		S1D19105_wr_block
#define Display_clear_if 		S1D19105_clear
#define Display_draw_bmp_if		S1D19105_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __S1D19105_H */
