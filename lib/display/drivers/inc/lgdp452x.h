/********************************************************************************/
/*!
	@file			lgdp452x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2012.08.27
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -LSG020-117NA0-FPC 		(LGDP4524)	8/16bit mode.		@n
					 -LH220Q01                  (LGDP4522)  8bit mode.       	@n
					 -S95311                  	(HX8340A)   8bit mode.       	@n

    @section HISTORY
		2011.11.30	V1.00	Renewed From LGDP452x driver.
		2012.08.27  V2.00	Added HX8340A driver.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __LGDP452X_H
#define __LGDP452X_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* LGDP452x unique value */
/* mst be need for LGDP452x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				176
#define MAX_Y				220

/* Display Contol Macros */
#define LGDP452x_RES_SET()	DISPLAY_RES_SET()
#define LGDP452x_RES_CLR()	DISPLAY_RES_CLR()
#define LGDP452x_CS_SET()	DISPLAY_CS_SET()
#define LGDP452x_CS_CLR()	DISPLAY_CS_CLR()
#define LGDP452x_DC_SET()	DISPLAY_DC_SET()
#define LGDP452x_DC_CLR()	DISPLAY_DC_CLR()
#define LGDP452x_WR_SET()	DISPLAY_WR_SET()
#define LGDP452x_WR_CLR()	DISPLAY_WR_CLR()
#define LGDP452x_RD_SET()	DISPLAY_RD_SET()
#define LGDP452x_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define LGDP452x_WR()   	LGDP452x_WR_CLR(); \
							LGDP452x_WR_SET();
#else
 #define LGDP452x_WR()
#endif

#define	LGDP452x_DATA		DISPLAY_DATAPORT
#define LGDP452x_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void LGDP452x_reset(void);
extern void LGDP452x_init(void);
extern void LGDP452x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void LGDP452x_wr_cmd(uint16_t cmd);
extern void LGDP452x_wr_dat(uint16_t dat);
extern void LGDP452x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void LGDP452x_clear(void);
extern int  LGDP452x_draw_bmp(const uint8_t* ptr);
extern uint16_t LGDP452x_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			LGDP452x_init
#define Display_rect_if 		LGDP452x_rect
#define Display_wr_dat_if		LGDP452x_wr_dat
#define Display_wr_cmd_if		LGDP452x_wr_cmd
#define Display_wr_block_if		LGDP452x_wr_block
#define Display_clear_if 		LGDP452x_clear
#define Display_draw_bmp_if		LGDP452x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __LGDP452x_H */
