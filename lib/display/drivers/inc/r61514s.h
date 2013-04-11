/********************************************************************************/
/*!
	@file			r61514s.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive DST5012B-NH		TFT module(8bit mode only).

    @section HISTORY
		2010.12.31	V1.00	Stable Release.
		2011.03.10	V2.00	C++ Ready.
		2011.10.25	V3.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __R61514S_H
#define __R61514S_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* R61514S unique value */
/* mst be need for R61514S */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				128
#define MAX_Y				160

/* Display Contol Macros */
#define R61514S_RES_SET()	DISPLAY_RES_SET()
#define R61514S_RES_CLR()	DISPLAY_RES_CLR()
#define R61514S_CS_SET()	DISPLAY_CS_SET()
#define R61514S_CS_CLR()	DISPLAY_CS_CLR()
#define R61514S_DC_SET()	DISPLAY_DC_SET()
#define R61514S_DC_CLR()	DISPLAY_DC_CLR()
#define R61514S_WR_SET()	DISPLAY_WR_SET()
#define R61514S_WR_CLR()	DISPLAY_WR_CLR()
#define R61514S_RD_SET()	DISPLAY_RD_SET()
#define R61514S_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define R61514S_WR()   	R61514S_WR_CLR(); \
							R61514S_WR_SET();
#else
 #define R61514S_WR()
#endif

#define	R61514S_DATA		DISPLAY_DATAPORT
#define R61514S_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void R61514S_reset(void);
extern void R61514S_init(void);
extern void R61514S_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void R61514S_wr_cmd(uint16_t cmd);
extern void R61514S_wr_dat(uint16_t dat);
extern void R61514S_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void R61514S_clear(void);
extern int  R61514S_draw_bmp(const uint8_t* ptr);
extern uint16_t R61514S_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			R61514S_init
#define Display_rect_if 		R61514S_rect
#define Display_wr_dat_if		R61514S_wr_dat
#define Display_wr_cmd_if		R61514S_wr_cmd
#define Display_wr_block_if		R61514S_wr_block
#define Display_clear_if 		R61514S_clear
#define Display_draw_bmp_if		R61514S_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __R61514S_H */
