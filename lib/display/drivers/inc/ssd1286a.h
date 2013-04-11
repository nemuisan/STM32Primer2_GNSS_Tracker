/********************************************************************************/
/*!
	@file			ssd1286a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CM06834 TFT module(8bit mode only).

    @section HISTORY
		2011.06.10	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SSD1286A_H
#define __SSD1286A_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SSD1286A unique value */
/* mst be need for SSD1286A */
#define OFS_RAW				0
#define OFS_COL				2
#define MAX_X				128
#define MAX_Y				160

/* Display Contol Macros */
#define SSD1286A_RES_SET()	DISPLAY_RES_SET()
#define SSD1286A_RES_CLR()	DISPLAY_RES_CLR()
#define SSD1286A_CS_SET()	DISPLAY_CS_SET()
#define SSD1286A_CS_CLR()	DISPLAY_CS_CLR()
#define SSD1286A_DC_SET()	DISPLAY_DC_SET()
#define SSD1286A_DC_CLR()	DISPLAY_DC_CLR()
#define SSD1286A_WR_SET()	DISPLAY_WR_SET()
#define SSD1286A_WR_CLR()	DISPLAY_WR_CLR()
#define SSD1286A_RD_SET()	DISPLAY_RD_SET()
#define SSD1286A_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SSD1286A_WR()   	SSD1286A_WR_CLR(); \
							SSD1286A_WR_SET();
#else
 #define SSD1286A_WR()
#endif

#define	SSD1286A_DATA			DISPLAY_DATAPORT
#define SSD1286A_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void SSD1286A_reset(void);
extern void SSD1286A_init(void);
extern void SSD1286A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SSD1286A_wr_cmd(uint8_t cmd);
extern void SSD1286A_wr_dat(uint16_t dat);
extern void SSD1286A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SSD1286A_clear(void);
extern int  SSD1286A_draw_bmp(const uint8_t* ptr);
extern uint16_t SSD1286A_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SSD1286A_init
#define Display_rect_if 		SSD1286A_rect
#define Display_wr_dat_if		SSD1286A_wr_dat
#define Display_wr_cmd_if		SSD1286A_wr_cmd
#define Display_wr_block_if		SSD1286A_wr_block
#define Display_clear_if 		SSD1286A_clear
#define Display_draw_bmp_if		SSD1286A_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SSD1286A_H */
