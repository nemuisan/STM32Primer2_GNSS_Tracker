/********************************************************************************/
/*!
	@file			r61509x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.04.29
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KIC30108-A				(R61509V)	8/16bit mode.		@n
					 -TFT8K3162                 (R61509V)   8/16bit mode.       @n
					 -BLC280-07D-1				(SPFD5420A)	8/16bit mode.		@n
					 -BLC300-02B-2				(SPFD5420A)	8/16bit mode.		@n
					 -FL260WQC01-A1T(KD156)		(R61509)	8/16bit&SPI mode. 	@n
					 -S95361					(ILI9326)   8/16bit mode.

    @section HISTORY
		2012.04.29	V1.00	Revised from r61509v.c

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __R61509X_H
#define __R61509X_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* R61509x unique value */
/* mst be need for R61509x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				400
/* MUST be need SPI access */
#define R61509x_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(R61509x_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(R61509x_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(R61509x_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(R61509x_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define R61509x_RES_SET()	DISPLAY_RES_SET()
#define R61509x_RES_CLR()	DISPLAY_RES_CLR()
#define R61509x_CS_SET()	DISPLAY_CS_SET()
#define R61509x_CS_CLR()	DISPLAY_CS_CLR()
#define R61509x_DC_SET()	DISPLAY_DC_SET()
#define R61509x_DC_CLR()	DISPLAY_DC_CLR()
#define R61509x_WR_SET()	DISPLAY_WR_SET()
#define R61509x_WR_CLR()	DISPLAY_WR_CLR()
#define R61509x_RD_SET()	DISPLAY_RD_SET()
#define R61509x_RD_CLR()	DISPLAY_RD_CLR()
#define R61509x_SCK_SET()	DISPLAY_SCK_SET()
#define R61509x_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define R61509x_WR()   	R61509x_WR_CLR(); \
							R61509x_WR_SET();
#else
 #define R61509x_WR()
#endif

#define	R61509x_DATA		DISPLAY_DATAPORT
#define R61509x_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void R61509x_reset(void);
extern void R61509x_init(void);
extern void R61509x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void R61509x_wr_cmd(uint16_t cmd);
extern void R61509x_wr_dat(uint16_t dat);
extern void R61509x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void R61509x_clear(void);
extern int  R61509x_draw_bmp(const uint8_t* ptr);
extern uint16_t R61509x_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			R61509x_init
#define Display_rect_if 		R61509x_rect
#define Display_wr_dat_if		R61509x_wr_dat
#define Display_wr_cmd_if		R61509x_wr_cmd
#define Display_wr_block_if		R61509x_wr_block
#define Display_clear_if 		R61509x_clear
#define Display_draw_bmp_if		R61509x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __R61509X_H */
