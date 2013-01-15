/********************************************************************************/
/*!
	@file			seps525.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG-6028GDEAF01 OLED module(8bit/4wire-spi).

    @section HISTORY
		2011.04.01	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SEPS525_H
#define __SEPS525_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SEPS525 Unique Value		*/
/* MUST be need for SEPS525	*/
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				160
#define MAX_Y				128

/* Display Contol Macros */
#define SEPS525_RES_SET()	DISPLAY_RES_SET()
#define SEPS525_RES_CLR()	DISPLAY_RES_CLR()
#define SEPS525_CS_SET()	DISPLAY_CS_SET()
#define SEPS525_CS_CLR()	DISPLAY_CS_CLR()
#define SEPS525_DC_SET()	DISPLAY_DC_SET()
#define SEPS525_DC_CLR()	DISPLAY_DC_CLR()
#define SEPS525_WR_SET()	DISPLAY_WR_SET()
#define SEPS525_WR_CLR()	DISPLAY_WR_CLR()
#define SEPS525_RD_SET()	DISPLAY_RD_SET()
#define SEPS525_RD_CLR()	DISPLAY_RD_CLR()
#define SEPS525_SDATA_SET()	DISPLAY_SDI_SET()
#define SEPS525_SDATA_CLR()	DISPLAY_SDI_CLR()
#define SEPS525_SCLK_SET()	DISPLAY_SCK_SET()
#define SEPS525_SCLK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SEPS525_WR()   	SEPS525_WR_CLR(); \
							SEPS525_WR_SET();
#else
 #define SEPS525_WR()
#endif

#define	SEPS525_DATA		DISPLAY_DATAPORT
#define SEPS525_CMD			DISPLAY_CMDPORT



/* Display Control Functions Prototype */
extern void SEPS525_reset(void);
extern void SEPS525_init(void);
extern void SEPS525_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SEPS525_wr_cmd(uint8_t cmd);
extern void SEPS525_wr_dat(uint8_t dat);
extern void SEPS525_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SEPS525_clear(void);
extern int  SEPS525_draw_bmp(const uint8_t* ptr);
extern void SEPS525_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SEPS525_init
#define Display_rect_if 		SEPS525_rect
#define Display_wr_dat_if		SEPS525_wr_gram
#define Display_wr_cmd_if		SEPS525_wr_cmd
#define Display_wr_block_if		SEPS525_wr_block
#define Display_clear_if 		SEPS525_clear
#define Display_draw_bmp_if		SEPS525_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SEPS525_H */
