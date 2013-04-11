/********************************************************************************/
/*!
	@file			ssd1351.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.06.03
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG-2828GDEDF11 OLED module(8bit,spi mode).

    @section HISTORY
		2012.06.03	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SSD1351_H
#define __SSD1351_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SSD1351 Unique Value		*/
/* MUST be need for SSD1351 */
#define OFS_RAW				0
#define OFS_COL				0
#define MAX_X				128
#define MAX_Y				128

/* Display Contol Macros */
#define SSD1351_RES_SET()	DISPLAY_RES_SET()
#define SSD1351_RES_CLR()	DISPLAY_RES_CLR()
#define SSD1351_CS_SET()	DISPLAY_CS_SET()
#define SSD1351_CS_CLR()	DISPLAY_CS_CLR()
#define SSD1351_DC_SET()	DISPLAY_DC_SET()
#define SSD1351_DC_CLR()	DISPLAY_DC_CLR()
#define SSD1351_WR_SET()	DISPLAY_WR_SET()
#define SSD1351_WR_CLR()	DISPLAY_WR_CLR()
#define SSD1351_RD_SET()	DISPLAY_RD_SET()
#define SSD1351_RD_CLR()	DISPLAY_RD_CLR()
#define SSD1351_SCK_SET()	DISPLAY_SCK_SET()
#define SSD1351_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SSD1351_WR()   	SSD1351_WR_CLR(); \
							SSD1351_WR_SET();
#else
 #define SSD1351_WR()
#endif

#define	SSD1351_DATA		DISPLAY_DATAPORT
#define SSD1351_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void SSD1351_reset(void);
extern void SSD1351_init(void);
extern void SSD1351_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SSD1351_wr_cmd(uint8_t cmd);
extern void SSD1351_wr_dat(uint8_t dat);
extern void SSD1351_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SSD1351_clear(void);
extern int  SSD1351_draw_bmp(const uint8_t* ptr);
extern void SSD1351_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SSD1351_init
#define Display_rect_if 		SSD1351_rect
#define Display_wr_dat_if		SSD1351_wr_gram
#define Display_wr_cmd_if		SSD1351_wr_cmd
#define Display_wr_block_if		SSD1351_wr_block
#define Display_clear_if 		SSD1351_clear
#define Display_draw_bmp_if		SSD1351_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SSD1351_H */
