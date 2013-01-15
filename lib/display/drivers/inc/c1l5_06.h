/********************************************************************************/
/*!
	@file			c1l5_06.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.21
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CS1802		TFT module(8bit bus only).			@n

    @section HISTORY
		2012.01.21	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __C1L5_06_H
#define __C1L5_06_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* C1L5_06 unique value */
/* mst be need for C1L5_06 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				128
#define MAX_Y				160

/* Display Contol Macros */
#define C1L5_06_RES_SET()	DISPLAY_RES_SET()
#define C1L5_06_RES_CLR()	DISPLAY_RES_CLR()
#define C1L5_06_CS_SET()	DISPLAY_CS_SET()
#define C1L5_06_CS_CLR()	DISPLAY_CS_CLR()
#define C1L5_06_DC_SET()	DISPLAY_DC_SET()
#define C1L5_06_DC_CLR()	DISPLAY_DC_CLR()
#define C1L5_06_WR_SET()	DISPLAY_WR_SET()
#define C1L5_06_WR_CLR()	DISPLAY_WR_CLR()
#define C1L5_06_RD_SET()	DISPLAY_RD_SET()
#define C1L5_06_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define C1L5_06_WR()   	C1L5_06_WR_CLR(); \
							C1L5_06_WR_SET();
#else
 #define C1L5_06_WR()
#endif

#define	C1L5_06_DATA		DISPLAY_DATAPORT
#define C1L5_06_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void C1L5_06_reset(void);
extern void C1L5_06_init(void);
extern void C1L5_06_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void C1L5_06_wr_cmd(uint8_t cmd);
extern void C1L5_06_wr_dat(uint8_t dat);
extern void C1L5_06_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void C1L5_06_clear(void);
extern int  C1L5_06_draw_bmp(const uint8_t* ptr);
extern uint16_t C1L5_06_rd_cmd(uint8_t cmd);
extern void C1L5_06_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			C1L5_06_init
#define Display_rect_if 		C1L5_06_rect
#define Display_wr_dat_if		C1L5_06_wr_gram
#define Display_wr_cmd_if		C1L5_06_wr_cmd
#define Display_wr_block_if		C1L5_06_wr_block
#define Display_clear_if 		C1L5_06_clear
#define Display_draw_bmp_if		C1L5_06_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __C1L5_06_H */
