/********************************************************************************/
/*!
	@file			ili9342.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2013.01.02
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive XYL62291B-2B TFT module(8/16bit mode).

    @section HISTORY
		2013.01.02	V1.00	Stable Release

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ILI9342_H
#define __ILI9342_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ILI9342 unique value */
/* mst be need for ILI9342 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				320
#define MAX_Y				240

/* Display Contol Macros */
#define ILI9342_RES_SET()	DISPLAY_RES_SET()
#define ILI9342_RES_CLR()	DISPLAY_RES_CLR()
#define ILI9342_CS_SET()	DISPLAY_CS_SET()
#define ILI9342_CS_CLR()	DISPLAY_CS_CLR()
#define ILI9342_DC_SET()	DISPLAY_DC_SET()
#define ILI9342_DC_CLR()	DISPLAY_DC_CLR()
#define ILI9342_WR_SET()	DISPLAY_WR_SET()
#define ILI9342_WR_CLR()	DISPLAY_WR_CLR()
#define ILI9342_RD_SET()	DISPLAY_RD_SET()
#define ILI9342_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9342_WR()   	ILI9342_WR_CLR(); \
							ILI9342_WR_SET();
#else
 #define ILI9342_WR()
#endif

#define	ILI9342_DATA		DISPLAY_DATAPORT
#define ILI9342_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI9342_reset(void);
extern void ILI9342_init(void);
extern void ILI9342_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9342_wr_cmd(uint8_t cmd);
extern void ILI9342_wr_dat(uint8_t dat);
extern void ILI9342_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9342_clear(void);
extern int  ILI9342_draw_bmp(const uint8_t* ptr);
extern uint16_t ILI9342_rd_cmd(uint8_t cmd);
extern void ILI9342_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			ILI9342_init
#define Display_rect_if 		ILI9342_rect
#define Display_wr_dat_if		ILI9342_wr_gram
#define Display_wr_cmd_if		ILI9342_wr_cmd
#define Display_wr_block_if		ILI9342_wr_block
#define Display_clear_if 		ILI9342_clear
#define Display_draw_bmp_if		ILI9342_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ILI9342_H */
