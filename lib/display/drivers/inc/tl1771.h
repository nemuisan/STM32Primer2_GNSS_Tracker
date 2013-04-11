/********************************************************************************/
/*!
	@file			tl1771.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.12.23
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive LTS182QQ-F0 TFT module(8/16bit mode).

    @section HISTORY
		2011.09.14	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __TL1771_H
#define __TL1771_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* TL1771 unique value */
/* mst be need for TL1771 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				128
#define MAX_Y				160

/* Display Contol Macros */
#define TL1771_RES_SET()	DISPLAY_RES_SET()
#define TL1771_RES_CLR()	DISPLAY_RES_CLR()
#define TL1771_CS_SET()		DISPLAY_CS_SET()
#define TL1771_CS_CLR()		DISPLAY_CS_CLR()
#define TL1771_DC_SET()		DISPLAY_DC_SET()
#define TL1771_DC_CLR()		DISPLAY_DC_CLR()
#define TL1771_WR_SET()		DISPLAY_WR_SET()
#define TL1771_WR_CLR()		DISPLAY_WR_CLR()
#define TL1771_RD_SET()		DISPLAY_RD_SET()
#define TL1771_RD_CLR()		DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define TL1771_WR()   		TL1771_WR_CLR(); \
							TL1771_WR_SET();
#else
 #define TL1771_WR()
#endif

#define	TL1771_DATA			DISPLAY_DATAPORT
#define TL1771_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void TL1771_reset(void);
extern void TL1771_init(void);
extern void TL1771_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void TL1771_wr_cmd(uint8_t cmd);
extern void TL1771_wr_dat(uint16_t dat);
extern void TL1771_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void TL1771_clear(void);
extern int  TL1771_draw_bmp(const uint8_t* ptr);
extern uint16_t TL1771_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			TL1771_init
#define Display_rect_if 		TL1771_rect
#define Display_wr_dat_if		TL1771_wr_dat
#define Display_wr_cmd_if		TL1771_wr_cmd
#define Display_wr_block_if		TL1771_wr_block
#define Display_clear_if 		TL1771_clear
#define Display_draw_bmp_if		TL1771_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __TL1771_H */
