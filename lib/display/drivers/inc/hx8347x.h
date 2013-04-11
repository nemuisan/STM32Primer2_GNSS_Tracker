/********************************************************************************/
/*!
	@file			hx8347x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2013.02.28
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95300					(HX8347A)	8/16bit mode.		@n
					 -STM032QVT-003				(HX8347A)	8/16bit mode.		@n
					 -DST6007					(HX8347D)	8/16bit mode.		@n
					 -MI0283QT-2	   			(HX8347D)	8/16bit&SPI mode. 	@n
					 -CD028THT22V2			  	(HX8347G(T))8/16bit&SPI mode. 	@n
					 -TFT8K0943FPC-A1-E		  	(HX8346A)	16bit mode.

    @section HISTORY
		2011.12.23	V1.00	Renewed From HX8347A driver.
		2012.03.31	V2.00	Added HX8347G(T) driver.
		2012.09.30	V3.00	Revised HX8346A driver.
		2013.02.28  V4.00	Optimized Some Codes.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX8347X_H
#define __HX8347X_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8347x unique value */
/* mst be need for HX8347x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320
/* MUST be need SPI access */
/*#define HX8347xSPI_4WIREMODE*/
#define HX8347xSPI_3WIREMODE

/* HX8347D&G SPI 8Bit 3-Wire mode Settings */
#define HX8347x_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(HX8347x_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(HX8347x_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(HX8347x_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(HX8347x_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define HX8347x_RES_SET()	DISPLAY_RES_SET()
#define HX8347x_RES_CLR()	DISPLAY_RES_CLR()
#define HX8347x_CS_SET()	DISPLAY_CS_SET()
#define HX8347x_CS_CLR()	DISPLAY_CS_CLR()
#if 	defined(HX8347xSPI_3WIREMODE)
 #define HX8347x_DC_SET()
 #define HX8347x_DC_CLR()
#elif 	defined(HX8347xSPI_4WIREMODE)
 #define HX8347x_DC_SET()	DISPLAY_DC_SET()
 #define HX8347x_DC_CLR()	DISPLAY_DC_CLR()
#else
 #error "U MUST Select HX8347x SPI Mode!!"
#endif
#define HX8347x_WR_SET()	DISPLAY_WR_SET()
#define HX8347x_WR_CLR()	DISPLAY_WR_CLR()
#define HX8347x_RD_SET()	DISPLAY_RD_SET()
#define HX8347x_RD_CLR()	DISPLAY_RD_CLR()
#define HX8347x_SCK_SET()	DISPLAY_SCK_SET()
#define HX8347x_SCK_CLR()	DISPLAY_SCK_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8347x_WR()   	HX8347x_WR_CLR(); \
							HX8347x_WR_SET();
#else
 #define HX8347x_WR()
#endif

#define	HX8347x_DATA		DISPLAY_DATAPORT
#define HX8347x_CMD			DISPLAY_CMDPORT

/* Display Control Functions Prototype */
extern void HX8347x_reset(void);
extern void HX8347x_init(void);
extern void HX8347x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8347x_wr_cmd(uint8_t cmd);
extern void HX8347x_wr_dat(uint8_t dat);
extern void HX8347x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8347x_clear(void);
extern int  HX8347x_draw_bmp(const uint8_t* ptr);
extern uint8_t HX8347x_rd_cmd(uint8_t cmd);
extern void HX8347x_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8347x_init
#define Display_rect_if 		HX8347x_rect
#define Display_wr_dat_if		HX8347x_wr_gram
#define Display_wr_cmd_if		HX8347x_wr_cmd
#define Display_wr_block_if		HX8347x_wr_block
#define Display_clear_if 		HX8347x_clear
#define Display_draw_bmp_if		HX8347x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX8347X_H */
