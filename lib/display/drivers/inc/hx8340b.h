/********************************************************************************/
/*!
	@file			hx8340b.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2011.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95215A-1				(HX8340B)		8bit mode only.		@n
					 -S95328-AAA			(HX8340B)		8/16bit mode.

    @section HISTORY
		2010.09.03	V1.00	Stable Release.
		2010.12.31	V2.00	Added GRAM write function.
							Fixed bug in 16bit access.
		2011.03.10	V3.00	C++ Ready.
		2011.11.10	V4.00	Added DMA TransactionSupport.
							Added Device ID Read.
		2011.11.30	V5.00	Added 3-Wire Serial Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX8340B_H
#define __HX8340B_H	0x0500

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8340B Unique Value	*/
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				176
#define MAX_Y				220
/* MUST be need SPI access */
#define HX8340B_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(HX8340B_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(HX8340B_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(HX8340B_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(HX8340B_ID | ID_IM0 | READ_DATA)


/* Display Contol Macros */
#define HX8340B_RES_SET()	DISPLAY_RES_SET()
#define HX8340B_RES_CLR()	DISPLAY_RES_CLR()
#define HX8340B_CS_SET()	DISPLAY_CS_SET()
#define HX8340B_CS_CLR()	DISPLAY_CS_CLR()
#define HX8340B_DC_SET()	DISPLAY_DC_SET()
#define HX8340B_DC_CLR()	DISPLAY_DC_CLR()
#define HX8340B_WR_SET()	DISPLAY_WR_SET()
#define HX8340B_WR_CLR()	DISPLAY_WR_CLR()
#define HX8340B_RD_SET()	DISPLAY_RD_SET()
#define HX8340B_RD_CLR()	DISPLAY_RD_CLR()
#define HX8340B_SCK_SET()	DISPLAY_SCK_SET()
#define HX8340B_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8340B_WR()   	HX8340B_WR_CLR(); \
							HX8340B_WR_SET();
#else
 #define HX8340B_WR()
#endif

#define	HX8340B_DATA		DISPLAY_DATAPORT
#define HX8340B_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8340B_reset(void);
extern void HX8340B_init(void);
extern void HX8340B_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8340B_wr_cmd(uint8_t cmd);
extern void HX8340B_wr_dat(uint8_t dat);
extern void HX8340B_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8340B_clear(void);
extern int  HX8340B_draw_bmp(const uint8_t* ptr);
extern void HX8340B_wr_gram(uint16_t gram);
extern uint8_t HX8340B_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8340B_init
#define Display_rect_if 		HX8340B_rect
#define Display_wr_dat_if		HX8340B_wr_gram
#define Display_wr_cmd_if		HX8340B_wr_cmd
#define Display_wr_block_if		HX8340B_wr_block
#define Display_clear_if 		HX8340B_clear
#define Display_draw_bmp_if		HX8340B_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX8340B_H */
