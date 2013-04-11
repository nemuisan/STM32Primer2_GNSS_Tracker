/********************************************************************************/
/*!
	@file			s6e63d6.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.12.23
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available OLED Modules are listed below.					@n
					 -BL024ACRNB$				(S6E63D6)	8/16bit,spi mode.	@n
					 -C0283QGLH-T				(S6E63D6)	8/16bit,spi mode.	@n

    @section HISTORY
		2011.05.22	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __S6E63D6_H
#define __S6E63D6_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S6E63D6 unique value */
/* mst be need for S6E63D6 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320
/* MUST be need SPI access */
#define S6E63D6_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(S6E63D6_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(S6E63D6_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(S6E63D6_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(S6E63D6_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define S6E63D6_RES_SET()	DISPLAY_RES_SET()
#define S6E63D6_RES_CLR()	DISPLAY_RES_CLR()
#define S6E63D6_CS_SET()	DISPLAY_CS_SET()
#define S6E63D6_CS_CLR()	DISPLAY_CS_CLR()
#define S6E63D6_DC_SET()	DISPLAY_DC_SET()
#define S6E63D6_DC_CLR()	DISPLAY_DC_CLR()
#define S6E63D6_WR_SET()	DISPLAY_WR_SET()
#define S6E63D6_WR_CLR()	DISPLAY_WR_CLR()
#define S6E63D6_RD_SET()	DISPLAY_RD_SET()
#define S6E63D6_RD_CLR()	DISPLAY_RD_CLR()
#define S6E63D6_SCK_SET()	DISPLAY_SCK_SET()
#define S6E63D6_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S6E63D6_WR()   	S6E63D6_WR_CLR(); \
							S6E63D6_WR_SET();
#else
 #define S6E63D6_WR()
#endif

#define	S6E63D6_DATA		DISPLAY_DATAPORT
#define S6E63D6_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S6E63D6_reset(void);
extern void S6E63D6_init(void);
extern void S6E63D6_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S6E63D6_wr_cmd(uint8_t cmd);
extern void S6E63D6_wr_dat(uint16_t dat);
extern void S6E63D6_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S6E63D6_clear(void);
extern int  S6E63D6_draw_bmp(const uint8_t* ptr);
extern uint16_t S6E63D6_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			S6E63D6_init
#define Display_rect_if 		S6E63D6_rect
#define Display_wr_dat_if		S6E63D6_wr_dat
#define Display_wr_cmd_if		S6E63D6_wr_cmd
#define Display_wr_block_if		S6E63D6_wr_block
#define Display_clear_if 		S6E63D6_clear
#define Display_draw_bmp_if		S6E63D6_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __S6E63D6_H */
