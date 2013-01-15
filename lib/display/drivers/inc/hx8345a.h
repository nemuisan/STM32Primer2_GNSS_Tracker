/********************************************************************************/
/*!
	@file			hx8345a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.03.31
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CM2111 TFT module(16bit-bus only).

    @section HISTORY
		2012.03.31	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX8345A_H
#define __HX8345A_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8345A Unique Value	*/
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				128
#define MAX_Y				160
/* MUST be need SPI access */
#define HX8345A_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(HX8345A_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(HX8345A_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(HX8345A_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(HX8345A_ID | ID_IM0 | READ_DATA)


/* Display Contol Macros */
#define HX8345A_RES_SET()	DISPLAY_RES_SET()
#define HX8345A_RES_CLR()	DISPLAY_RES_CLR()
#define HX8345A_CS_SET()	DISPLAY_CS_SET()
#define HX8345A_CS_CLR()	DISPLAY_CS_CLR()
#define HX8345A_DC_SET()	DISPLAY_DC_SET()
#define HX8345A_DC_CLR()	DISPLAY_DC_CLR()
#define HX8345A_WR_SET()	DISPLAY_WR_SET()
#define HX8345A_WR_CLR()	DISPLAY_WR_CLR()
#define HX8345A_RD_SET()	DISPLAY_RD_SET()
#define HX8345A_RD_CLR()	DISPLAY_RD_CLR()
#define HX8345A_SCK_SET()	DISPLAY_SCK_SET()
#define HX8345A_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8345A_WR()   	HX8345A_WR_CLR(); \
							HX8345A_WR_SET();
#else
 #define HX8345A_WR()
#endif

#define	HX8345A_DATA		DISPLAY_DATAPORT
#define HX8345A_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8345A_reset(void);
extern void HX8345A_init(void);
extern void HX8345A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8345A_wr_cmd(uint8_t cmd);
extern void HX8345A_wr_dat(uint16_t dat);
extern void HX8345A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8345A_clear(void);
extern int  HX8345A_draw_bmp(const uint8_t* ptr);
extern uint16_t HX8345A_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8345A_init
#define Display_rect_if 		HX8345A_rect
#define Display_wr_dat_if		HX8345A_wr_dat
#define Display_wr_cmd_if		HX8345A_wr_cmd
#define Display_wr_block_if		HX8345A_wr_block
#define Display_clear_if 		HX8345A_clear
#define Display_draw_bmp_if		HX8345A_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX8345A_H */
