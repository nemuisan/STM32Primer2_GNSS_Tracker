/********************************************************************************/
/*!
	@file			s6d0154.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2013.01.02
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P2181-E TFT module(8/16bit mode).

    @section HISTORY
		2013.01.02	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __S6D0154_H
#define __S6D0154_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S6D0154 unique value */
/* mst be need for S6D0154 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define S6D0154_RES_SET()	DISPLAY_RES_SET()
#define S6D0154_RES_CLR()	DISPLAY_RES_CLR()
#define S6D0154_CS_SET()	DISPLAY_CS_SET()
#define S6D0154_CS_CLR()	DISPLAY_CS_CLR()
#define S6D0154_DC_SET()	DISPLAY_DC_SET()
#define S6D0154_DC_CLR()	DISPLAY_DC_CLR()
#define S6D0154_WR_SET()	DISPLAY_WR_SET()
#define S6D0154_WR_CLR()	DISPLAY_WR_CLR()
#define S6D0154_RD_SET()	DISPLAY_RD_SET()
#define S6D0154_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S6D0154_WR()   	S6D0154_WR_CLR(); \
							S6D0154_WR_SET();
#else
 #define S6D0154_WR()
#endif

#define	S6D0154_DATA		DISPLAY_DATAPORT
#define S6D0154_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S6D0154_reset(void);
extern void S6D0154_init(void);
extern void S6D0154_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S6D0154_wr_cmd(uint8_t cmd);
extern void S6D0154_wr_dat(uint16_t dat);
extern void S6D0154_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S6D0154_clear(void);
extern int  S6D0154_draw_bmp(const uint8_t* ptr);
extern uint16_t S6D0154_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			S6D0154_init
#define Display_rect_if 		S6D0154_rect
#define Display_wr_dat_if		S6D0154_wr_dat
#define Display_wr_cmd_if		S6D0154_wr_cmd
#define Display_wr_block_if		S6D0154_wr_block
#define Display_clear_if 		S6D0154_clear
#define Display_draw_bmp_if		S6D0154_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __S6D0154_H */
