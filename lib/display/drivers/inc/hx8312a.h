/********************************************************************************/
/*!
	@file			hx8312a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive WSTTN03-S220A TFT module(16bit mode only).

    @section HISTORY
		2010.12.31	V1.00	Stable Release.
		2011.03.10	V2.00	C++ Ready.
		2011.10.25	V3.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX8312A_H
#define __HX8312A_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8312A Unique Value	*/
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define HX8312A_RES_SET()	DISPLAY_RES_SET()
#define HX8312A_RES_CLR()	DISPLAY_RES_CLR()
#define HX8312A_CS_SET()	DISPLAY_CS_SET()
#define HX8312A_CS_CLR()	DISPLAY_CS_CLR()
#define HX8312A_DC_SET()	DISPLAY_DC_SET()
#define HX8312A_DC_CLR()	DISPLAY_DC_CLR()
#define HX8312A_WR_SET()	DISPLAY_WR_SET()
#define HX8312A_WR_CLR()	DISPLAY_WR_CLR()
#define HX8312A_RD_SET()	DISPLAY_RD_SET()
#define HX8312A_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8312A_WR()   	HX8312A_WR_CLR(); HX8312A_WR_CLR(); \
							HX8312A_WR_SET(); HX8312A_WR_SET(); 
#else
 #define HX8312A_WR()
#endif

#define	HX8312A_DATA		DISPLAY_DATAPORT
#define HX8312A_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8312A_reset(void);
extern void HX8312A_init(void);
extern void HX8312A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8312A_wr_cmd(uint8_t cmd,uint8_t cmdata);
extern void HX8312A_wr_dat(uint16_t dat);
extern void HX8312A_wr_block(uint8_t* blockdata,unsigned int datacount);

extern void HX8312A_clear(void);
extern int  HX8312A_draw_bmp(const uint8_t* ptr);
extern uint16_t HX8312A_rd_cmd(uint8_t cmd,uint8_t cmdata);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8312A_init
#define Display_rect_if 		HX8312A_rect
#define Display_wr_dat_if		HX8312A_wr_dat
#define Display_wr_cmd_if		HX8312A_wr_cmd
#define Display_wr_block_if		HX8312A_wr_block
#define Display_clear_if 		HX8312A_clear
#define Display_draw_bmp_if		HX8312A_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX8312A_H */
