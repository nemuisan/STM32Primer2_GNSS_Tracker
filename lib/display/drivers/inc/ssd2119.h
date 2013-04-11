/********************************************************************************/
/*!
	@file			ssd2119.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CFAF320240F-T-TS TFT module(8/16bit,spi mode).

    @section HISTORY
		2010.07.10	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Cleanup SourceCode.
		2011.03.10	V4.00	C++ Ready.
		2011.10.25	V5.00	Added DMA TransactionSupport.
							Added 4-Wire SPI Transfer Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SSD2119_H
#define __SSD2119_H	0x0500

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SSD2119 unique value */
/* mst be need for SSD2119 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				320
#define MAX_Y				240

/* Display Contol Macros */
#define SSD2119_RES_SET()	DISPLAY_RES_SET()
#define SSD2119_RES_CLR()	DISPLAY_RES_CLR()
#define SSD2119_CS_SET()	DISPLAY_CS_SET()
#define SSD2119_CS_CLR()	DISPLAY_CS_CLR()
#define SSD2119_DC_SET()	DISPLAY_DC_SET()
#define SSD2119_DC_CLR()	DISPLAY_DC_CLR()
#define SSD2119_WR_SET()	DISPLAY_WR_SET()
#define SSD2119_WR_CLR()	DISPLAY_WR_CLR()
#define SSD2119_RD_SET()	DISPLAY_RD_SET()
#define SSD2119_RD_CLR()	DISPLAY_RD_CLR()
#define SSD2119_SCK_SET()	DISPLAY_SCK_SET()
#define SSD2119_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SSD2119_WR()   	SSD2119_WR_CLR(); \
							SSD2119_WR_SET();
#else
 #define SSD2119_WR()
#endif

#define	SSD2119_DATA		DISPLAY_DATAPORT
#define SSD2119_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void SSD2119_reset(void);
extern void SSD2119_init(void);
extern void SSD2119_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SSD2119_wr_cmd(uint8_t cmd);
extern void SSD2119_wr_dat(uint16_t dat);
extern void SSD2119_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SSD2119_clear(void);
extern int  SSD2119_draw_bmp(const uint8_t* ptr);
extern uint16_t SSD2119_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SSD2119_init
#define Display_rect_if 		SSD2119_rect
#define Display_wr_dat_if		SSD2119_wr_dat
#define Display_wr_cmd_if		SSD2119_wr_cmd
#define Display_wr_block_if		SSD2119_wr_block
#define Display_clear_if 		SSD2119_clear
#define Display_draw_bmp_if		SSD2119_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SSD2119_H */
