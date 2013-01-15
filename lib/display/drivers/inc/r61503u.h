/********************************************************************************/
/*!
	@file			r61503u.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.09.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive EP-T022QCW1R1-T03	TFT module(8/16bit).		@n

    @section HISTORY
		2012.09.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __R61503U_H
#define __R61503U_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* R61503U unique value */
/* mst be need for R61503U */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				176
#define MAX_Y				220

/* Display Contol Macros */
#define R61503U_RES_SET()	DISPLAY_RES_SET()
#define R61503U_RES_CLR()	DISPLAY_RES_CLR()
#define R61503U_CS_SET()	DISPLAY_CS_SET()
#define R61503U_CS_CLR()	DISPLAY_CS_CLR()
#define R61503U_DC_SET()	DISPLAY_DC_SET()
#define R61503U_DC_CLR()	DISPLAY_DC_CLR()
#define R61503U_WR_SET()	DISPLAY_WR_SET()
#define R61503U_WR_CLR()	DISPLAY_WR_CLR()
#define R61503U_RD_SET()	DISPLAY_RD_SET()
#define R61503U_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define R61503U_WR()   	R61503U_WR_CLR(); \
							R61503U_WR_SET();
#else
 #define R61503U_WR()
#endif

#define	R61503U_DATA		DISPLAY_DATAPORT
#define R61503U_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void R61503U_reset(void);
extern void R61503U_init(void);
extern void R61503U_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void R61503U_wr_cmd(uint16_t cmd);
extern void R61503U_wr_dat(uint16_t dat);
extern void R61503U_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void R61503U_clear(void);
extern int  R61503U_draw_bmp(const uint8_t* ptr);
extern uint16_t R61503U_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			R61503U_init
#define Display_rect_if 		R61503U_rect
#define Display_wr_dat_if		R61503U_wr_dat
#define Display_wr_cmd_if		R61503U_wr_cmd
#define Display_wr_block_if		R61503U_wr_block
#define Display_clear_if 		R61503U_clear
#define Display_draw_bmp_if		R61503U_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __R61503U_H */
