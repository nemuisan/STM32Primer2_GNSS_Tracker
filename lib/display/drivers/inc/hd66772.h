/********************************************************************************/
/*!
	@file			hd66772.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.10.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TX05D99VM1AAA TFT module.

    @section HISTORY
		2011.10.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HD66772_H
#define __HD66772_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HD66772 unique value */
/* mst be need for HD66772 */
#define USE_FIRST_DISPLAY
/*#define USE_SECOND_DISPLAY*/

#ifdef USE_FIRST_DISPLAY
 #define OFS_COL			0
 #define OFS_RAW			80
 #define MAX_X				128
 #define MAX_Y				160
#else
 #define OFS_COL			0
 #define OFS_RAW			16
 #define MAX_X				96
 #define MAX_Y				64
#endif

/* Display Contol Macros */
#define HD66772_RES_SET()	DISPLAY_RES_SET()
#define HD66772_RES_CLR()	DISPLAY_RES_CLR()
#define HD66772_CS_SET()	DISPLAY_CS_SET()
#define HD66772_CS_CLR()	DISPLAY_CS_CLR()
#define HD66772_DC_SET()	DISPLAY_DC_SET()
#define HD66772_DC_CLR()	DISPLAY_DC_CLR()
#define HD66772_WR_SET()	DISPLAY_WR_SET()
#define HD66772_WR_CLR()	DISPLAY_WR_CLR()
#define HD66772_RD_SET()	DISPLAY_RD_SET()
#define HD66772_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HD66772_WR()   	HD66772_WR_CLR(); \
							HD66772_WR_SET();
#else
 #define HD66772_WR()
#endif

#define	HD66772_DATA		DISPLAY_DATAPORT
#define HD66772_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HD66772_reset(void);
extern void HD66772_init(void);
extern void HD66772_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HD66772_wr_cmd(uint8_t cmd);
extern void HD66772_wr_dat(uint16_t dat);
extern void HD66772_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HD66772_clear(void);
extern int  HD66772_draw_bmp(const uint8_t* ptr);
extern uint16_t HD66772_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HD66772_init
#define Display_rect_if 		HD66772_rect
#define Display_wr_dat_if		HD66772_wr_dat
#define Display_wr_cmd_if		HD66772_wr_cmd
#define Display_wr_block_if		HD66772_wr_block
#define Display_clear_if 		HD66772_clear
#define Display_draw_bmp_if		HD66772_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HD66772_H */
