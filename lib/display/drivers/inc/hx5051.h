/********************************************************************************/
/*!
	@file			hx5051.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive C0200QILC-C OLED module.

    @section HISTORY
		2011.11.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX5051_H
#define __HX5051_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX5051 unique value */
#define FLIP_SCREEN_C0200QILC

/* mst be need for HX5051 */
#if defined(FLIP_SCREEN_C0200QILC)
 #define OFS_COL			0
 #define OFS_RAW			220
#else
 #define OFS_COL			0
 #define OFS_RAW			0
#endif
#define MAX_X				176
#define MAX_Y				220
/* MUST be need SPI access */
#define HX5051_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(HX5051_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(HX5051_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(HX5051_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(HX5051_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define HX5051_RES_SET()	DISPLAY_RES_SET()
#define HX5051_RES_CLR()	DISPLAY_RES_CLR()
#define HX5051_CS_SET()		DISPLAY_CS_SET()
#define HX5051_CS_CLR()		DISPLAY_CS_CLR()
#define HX5051_DC_SET()		DISPLAY_DC_SET()
#define HX5051_DC_CLR()		DISPLAY_DC_CLR()
#define HX5051_WR_SET()		DISPLAY_WR_SET()
#define HX5051_WR_CLR()		DISPLAY_WR_CLR()
#define HX5051_RD_SET()		DISPLAY_RD_SET()
#define HX5051_RD_CLR()		DISPLAY_RD_CLR()
#define HX5051_SCK_SET()	DISPLAY_SCK_SET()
#define HX5051_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX5051_WR()   		HX5051_WR_CLR(); \
							HX5051_WR_SET();
#else
 #define HX5051_WR()
#endif

#define	HX5051_DATA			DISPLAY_DATAPORT
#define HX5051_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX5051_reset(void);
extern void HX5051_init(void);
extern void HX5051_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX5051_wr_cmd(uint16_t cmd);
extern void HX5051_wr_dat(uint16_t dat);
extern void HX5051_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX5051_clear(void);
extern int  HX5051_draw_bmp(const uint8_t* ptr);
extern uint16_t HX5051_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			HX5051_init
#define Display_rect_if 		HX5051_rect
#define Display_wr_dat_if		HX5051_wr_dat
#define Display_wr_cmd_if		HX5051_wr_cmd
#define Display_wr_block_if		HX5051_wr_block
#define Display_clear_if 		HX5051_clear
#define Display_draw_bmp_if		HX5051_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX5051_H */
