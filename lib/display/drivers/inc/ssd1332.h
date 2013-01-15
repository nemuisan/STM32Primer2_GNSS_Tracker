/********************************************************************************/
/*!
	@file			ssd1332.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2012.02.15
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive ALO-095BWNN-J9 OLED module via SPI.

    @section HISTORY
		2010.06.01	V1.00	Unstable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added GRAM write function.
		2011.03.10	V4.00	C++ Ready.
		2012.02.15	V5.00	Add Foolest WorkAround In Write GGRAM Access.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SSD1332_H
#define __SSD1332_H	0x0500

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SSD1332 Unique Value	 */
/* MUST be need for SSD1332 */
#define OFS_RAW				0
#define OFS_COL				0
#define MAX_X				96
#define MAX_Y				64

/* Display Contol Macros */
#define SSD1332_RES_SET()	DISPLAY_RES_SET()
#define SSD1332_RES_CLR()	DISPLAY_RES_CLR()
#define SSD1332_CS_SET()	DISPLAY_CS_SET()
#define SSD1332_CS_CLR()	DISPLAY_CS_CLR()
#define SSD1332_DC_SET()	DISPLAY_DC_SET()
#define SSD1332_DC_CLR()	DISPLAY_DC_CLR()
#define SSD1332_SDATA_SET()	DISPLAY_SDI_SET()
#define SSD1332_SDATA_CLR()	DISPLAY_SDI_CLR()
#define SSD1332_SCLK_SET()	DISPLAY_SCK_SET()
#define SSD1332_SCLK_CLR()	DISPLAY_SCK_CLR()


/* Display Control Functions Prototype */
extern void SSD1332_reset(void);
extern void SSD1332_init(void);
extern void SSD1332_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SSD1332_wr_cmd(uint8_t cmd);
extern void SSD1332_wr_dat(uint8_t dat);
extern void SSD1332_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SSD1332_clear(void);
extern int  SSD1332_draw_bmp(const uint8_t* ptr);
extern void SSD1332_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SSD1332_init
#define Display_rect_if 		SSD1332_rect
#define Display_wr_dat_if		SSD1332_wr_gram
#define Display_wr_cmd_if		SSD1332_wr_cmd
#define Display_wr_block_if		SSD1332_wr_block
#define Display_clear_if 		SSD1332_clear
#define Display_draw_bmp_if		SSD1332_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SSD1332_H */
