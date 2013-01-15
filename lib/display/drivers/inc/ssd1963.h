/********************************************************************************/
/*!
	@file			ssd1963.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2012.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available "Raw" TFT-Modules are listed below.				@n
					This driver is intened for 565-16bit Interface Only!		@m
 					 -AT043TN24 V.1												@n
 					 -HSD043-BOO

    @section HISTORY
		2011.05.22	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		2012.08.01	V4.00	Add HSD043-B00 Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SSD1963_H
#define __SSD1963_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* SSD1963 unique value */
/* U MUST SELSECT EACH ONE */
//#define USE_AT043TN24_V1
#define USE_HSD043_BOO

#if defined(USE_AT043TN24_V1)
 /* For AT043TN24 V.1 Settings */
 #define OFS_COL			0
 #define OFS_RAW			0
 #define MAX_X				480
 #define MAX_Y				272
 
 #define MODULETYPE			0x18	
 /* Holizontal */
 #define HDP				(MAX_X-1)
 #define HT					531
 #define HPS				43
 #define LPS				8
 #define HPW				10
 /* Vertical */
 #define VDP				(MAX_Y-1)
 #define VT					288
 #define VPS				12
 #define FPS				4
 #define VPW				10

#elif defined(USE_HSD043_BOO)
 /* For USE_HSD043-BOO Settings */
 #define OFS_COL			0
 #define OFS_RAW			0
 #define MAX_X				480
 #define MAX_Y				272
 
 #define MODULETYPE			0x20	
 /* Holizontal */
 #define HDP				(MAX_X-1)
 #define HT					531
 #define HPS				43
 #define LPS				8
 #define HPW				10
 /* Vertical */
 #define VDP				(MAX_Y-1)
 #define VT					288
 #define VPS				12
 #define FPS				4
 #define VPW				10
#endif

/* Display Contol Macros */
#define SSD1963_RES_SET()	DISPLAY_RES_SET()
#define SSD1963_RES_CLR()	DISPLAY_RES_CLR()
#define SSD1963_CS_SET()	DISPLAY_CS_SET()
#define SSD1963_CS_CLR()	DISPLAY_CS_CLR()
#define SSD1963_DC_SET()	DISPLAY_DC_SET()
#define SSD1963_DC_CLR()	DISPLAY_DC_CLR()
#define SSD1963_WR_SET()	DISPLAY_WR_SET()
#define SSD1963_WR_CLR()	DISPLAY_WR_CLR()
#define SSD1963_RD_SET()	DISPLAY_RD_SET()
#define SSD1963_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define SSD1963_WR()   	SSD1963_WR_CLR(); \
							SSD1963_WR_SET();
#else
 #define SSD1963_WR()
#endif

#define	SSD1963_DATA		DISPLAY_DATAPORT
#define SSD1963_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void SSD1963_reset(void);
extern void SSD1963_init(void);
extern void SSD1963_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void SSD1963_wr_cmd(uint8_t cmd);
extern void SSD1963_wr_dat(uint16_t dat);
extern void SSD1963_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void SSD1963_clear(void);
extern int  SSD1963_draw_bmp(const uint8_t* ptr);
extern uint16_t SSD1963_rd_cmd(uint8_t cmd);
extern void SSD1963_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			SSD1963_init
#define Display_rect_if 		SSD1963_rect
#define Display_wr_dat_if		SSD1963_wr_gram
#define Display_wr_cmd_if		SSD1963_wr_cmd
#define Display_wr_block_if		SSD1963_wr_block
#define Display_clear_if 		SSD1963_clear
#define Display_draw_bmp_if		SSD1963_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __SSD1963_H */
