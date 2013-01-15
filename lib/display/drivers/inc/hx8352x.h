/********************************************************************************/
/*!
	@file			hx8352x.h
    @version        1.00
    @date           2012.08.27
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -QD-ET3207ABT				(HX8352A)	16bit mode.			@n
					 -S95461C					(HX8352B)	16bit mode.

    @section HISTORY
		2012.08.27	V1.00	Revised From hx8352a.c

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX8352X_H
#define __HX8352X_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8352x unique value */
/* mst be need for HX8352x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				400

/* Display Contol Macros */
#define HX8352x_RES_SET()	DISPLAY_RES_SET()
#define HX8352x_RES_CLR()	DISPLAY_RES_CLR()
#define HX8352x_CS_SET()	DISPLAY_CS_SET()
#define HX8352x_CS_CLR()	DISPLAY_CS_CLR()
#define HX8352x_DC_SET()	DISPLAY_DC_SET()
#define HX8352x_DC_CLR()	DISPLAY_DC_CLR()
#define HX8352x_WR_SET()	DISPLAY_WR_SET()
#define HX8352x_WR_CLR()	DISPLAY_WR_CLR()
#define HX8352x_RD_SET()	DISPLAY_RD_SET()
#define HX8352x_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8352x_WR()   	HX8352x_WR_CLR(); \
							HX8352x_WR_SET();
#else
 #define HX8352x_WR()
#endif

#define	HX8352x_DATA		DISPLAY_DATAPORT
#define HX8352x_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8352x_reset(void);
extern void HX8352x_init(void);
extern void (*HX8352x_rect)(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8352x_wr_cmd(uint8_t cmd);
extern void HX8352x_wr_dat(uint8_t dat);
extern void HX8352x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8352x_clear(void);
extern int  HX8352x_draw_bmp(const uint8_t* ptr);
extern uint16_t HX8352x_rd_cmd(uint8_t cmd);
extern void HX8352x_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8352x_init
#define Display_rect_if 		HX8352x_rect
#define Display_wr_dat_if		HX8352x_wr_gram
#define Display_wr_cmd_if		HX8352x_wr_cmd
#define Display_wr_block_if		HX8352x_wr_block
#define Display_clear_if 		HX8352x_clear
#define Display_draw_bmp_if		HX8352x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX8352X_H */
