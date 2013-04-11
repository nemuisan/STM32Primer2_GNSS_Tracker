/********************************************************************************/
/*!
	@file			d51e5ta7601.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.07.28
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P4705-E TFT module(8/16bit mode).

    @section HISTORY
		2012.07.28	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __D51E5TA7601_H
#define __D51E5TA7601_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* D51E5TA7601 unique value */
#define FLIP_SCREEN_TFT1P4705_E

/* mst be need for D51E5TA7601 */
#if defined(FLIP_SCREEN_TFT1P4705_E)
 #define OFS_COL			0
 #define OFS_RAW			0
#else
 #define OFS_COL			0
 #define OFS_RAW			0
#endif
#define MAX_X				320
#define MAX_Y				480

/* Display Contol Macros */
#define D51E5TA7601_RES_SET()	DISPLAY_RES_SET()
#define D51E5TA7601_RES_CLR()	DISPLAY_RES_CLR()
#define D51E5TA7601_CS_SET()	DISPLAY_CS_SET()
#define D51E5TA7601_CS_CLR()	DISPLAY_CS_CLR()
#define D51E5TA7601_DC_SET()	DISPLAY_DC_SET()
#define D51E5TA7601_DC_CLR()	DISPLAY_DC_CLR()
#define D51E5TA7601_WR_SET()	DISPLAY_WR_SET()
#define D51E5TA7601_WR_CLR()	DISPLAY_WR_CLR()
#define D51E5TA7601_RD_SET()	DISPLAY_RD_SET()
#define D51E5TA7601_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define D51E5TA7601_WR()   	D51E5TA7601_WR_CLR(); \
								D51E5TA7601_WR_SET();
#else
 #define D51E5TA7601_WR()
#endif

#define	D51E5TA7601_DATA		DISPLAY_DATAPORT
#define D51E5TA7601_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void D51E5TA7601_reset(void);
extern void D51E5TA7601_init(void);
extern void D51E5TA7601_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void D51E5TA7601_wr_cmd(uint16_t cmd);
extern void D51E5TA7601_wr_dat(uint16_t dat);
extern void D51E5TA7601_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void D51E5TA7601_clear(void);
extern int  D51E5TA7601_draw_bmp(const uint8_t* ptr);
extern uint16_t D51E5TA7601_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			D51E5TA7601_init
#define Display_rect_if 		D51E5TA7601_rect
#define Display_wr_dat_if		D51E5TA7601_wr_dat
#define Display_wr_cmd_if		D51E5TA7601_wr_cmd
#define Display_wr_block_if		D51E5TA7601_wr_block
#define Display_clear_if 		D51E5TA7601_clear
#define Display_draw_bmp_if		D51E5TA7601_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __D51E5TA7601_H */
