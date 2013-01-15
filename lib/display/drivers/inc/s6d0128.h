/********************************************************************************/
/*!
	@file			s6d0128.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.12.23
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive AS021350D TFT module(8/16bit).

    @section HISTORY
		2011.04.30	V1.00	Stable Release
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __S6D0128_H
#define __S6D0128_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S6D0128 unique value */
/* mst be need for S6D0128 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				176
#define MAX_Y				220

/* Display Contol Macros */
#define S6D0128_RES_SET()	DISPLAY_RES_SET()
#define S6D0128_RES_CLR()	DISPLAY_RES_CLR()
#define S6D0128_CS_SET()	DISPLAY_CS_SET()
#define S6D0128_CS_CLR()	DISPLAY_CS_CLR()
#define S6D0128_DC_SET()	DISPLAY_DC_SET()
#define S6D0128_DC_CLR()	DISPLAY_DC_CLR()
#define S6D0128_WR_SET()	DISPLAY_WR_SET()
#define S6D0128_WR_CLR()	DISPLAY_WR_CLR()
#define S6D0128_RD_SET()	DISPLAY_RD_SET()
#define S6D0128_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S6D0128_WR()   	S6D0128_WR_CLR(); \
							S6D0128_WR_SET();
#else
 #define S6D0128_WR()
#endif

#define	S6D0128_DATA		DISPLAY_DATAPORT
#define S6D0128_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S6D0128_reset(void);
extern void S6D0128_init(void);
extern void S6D0128_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S6D0128_wr_cmd(uint8_t cmd);
extern void S6D0128_wr_dat(uint16_t dat);
extern void S6D0128_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S6D0128_clear(void);
extern int  S6D0128_draw_bmp(const uint8_t* ptr);
extern uint16_t S6D0128_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			S6D0128_init
#define Display_rect_if 		S6D0128_rect
#define Display_wr_dat_if		S6D0128_wr_dat
#define Display_wr_cmd_if		S6D0128_wr_cmd
#define Display_wr_block_if		S6D0128_wr_block
#define Display_clear_if 		S6D0128_clear
#define Display_draw_bmp_if		S6D0128_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __S6D0128_H */
