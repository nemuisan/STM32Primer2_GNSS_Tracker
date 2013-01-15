/********************************************************************************/
/*!
	@file			ssd1289.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2013.01.02
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KFM597A01-1A		  		(SSD1289)	16bit mode only.	@n
					 -FP-SQV28002A00		  	(SSD1297)	8bit mode only.		@n
					 -FPC-Y80216		  		(SSD1298)	16bit mode only.

    @section HISTORY
		2010.10.01	V1.00	Stable Release.
		2010.12.31	V2.00	Cleanup SourceCode.
		2011.03.10	V3.00	C++ Ready.
		2011.10.25	V4.00	Added DMA TransactionSupport.
		2011.12.23	V5.00	Optimize Some Codes.
		2013.01.02	V6.00	Added SSD1297,SSD1298 Devices.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SSD1289_H
#define __SSD1289_H	0x0600

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SSD1289 unique value */
/* mst be need for SSD1289 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define SSD1289_RES_SET()	DISPLAY_RES_SET()
#define SSD1289_RES_CLR()	DISPLAY_RES_CLR()
#define SSD1289_CS_SET()	DISPLAY_CS_SET()
#define SSD1289_CS_CLR()	DISPLAY_CS_CLR()
#define SSD1289_DC_SET()	DISPLAY_DC_SET()
#define SSD1289_DC_CLR()	DISPLAY_DC_CLR()
#define SSD1289_WR_SET()	DISPLAY_WR_SET()
#define SSD1289_WR_CLR()	DISPLAY_WR_CLR()
#define SSD1289_RD_SET()	DISPLAY_RD_SET()
#define SSD1289_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SSD1289_WR()   	SSD1289_WR_CLR(); \
							SSD1289_WR_SET();
#else
 #define SSD1289_WR()
#endif

#define	SSD1289_DATA		DISPLAY_DATAPORT
#define SSD1289_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void SSD1289_reset(void);
extern void SSD1289_init(void);
extern void SSD1289_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SSD1289_wr_cmd(uint8_t cmd);
extern void SSD1289_wr_dat(uint16_t dat);
extern void SSD1289_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SSD1289_clear(void);
extern int  SSD1289_draw_bmp(const uint8_t* ptr);
extern uint16_t SSD1289_rd_cmd(uint8_t cmd);
extern void (*SSD1289_wr_gram)(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SSD1289_init
#define Display_rect_if 		SSD1289_rect
#define Display_wr_dat_if		SSD1289_wr_gram
#define Display_wr_cmd_if		SSD1289_wr_cmd
#define Display_wr_block_if		SSD1289_wr_block
#define Display_clear_if 		SSD1289_clear
#define Display_draw_bmp_if		SSD1289_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SSD1289_H */
