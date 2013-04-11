/********************************************************************************/
/*!
	@file			ili932x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        10.00
    @date           2013.04.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -YHY024006A				(ILI9325)	8/16bit mode.		@n
					 -EGO028Q02-F05				(ILI9325)	8/16bit mode.		@n
					 -WBX280V009				(ILI9325)	8/16bit mode.		@n
					 -V320P243282WSI7TB   		(ILI9320)	16bit mode.			@n
					 -PH240320T-030-XP1Q  		(R61505U)	8/16bit mode.		@n
					 -TFT-GG1N4198UTSW-W-TP-E  	(LGDP4531)	8/16bit,spi mode.	@n
					 -MR028-9325-51P-TP-A  		(ILI9328)	8/16bit,spi mode.	@n
					 -MR024-9325-51P-TP-B  		(ILI9325C)	8/16bit,spi mode.	@n
					 -OPFPCT0634-V0				(ILI9320)	8/16bit mode.		@n
					 -TS8026Y					(ST7781)	8bit mode only.		@n
					 -TFT1P3204-E				(R61505W)	8/16bit mode.		@n
					 -KD032C-2-TP 				(ILI9325C)	8/16bit,spi mode.   @n
					 -TFT1P2477-E				(R61505V)	8bit mode only.		@n
					 -VS3026A					(RM68050)	8/16bit mode.		@n
					 -KFM529B21-1A				(SPFD5408B)	8/16bit mode.		@n
					 -CH24Q304-LCM-A			(LGDP4535)  8/16bit mode.		@n
					 -KXM280H-T09				(ILI9331)   8/16bit mode.		@n
					 -DST2401PH					(R61580)   	8/16bit mode.		@n
					 -JLX320-002				(RM68090)   8bit mode only.		@n
					 -AT32Q02					(FT1505C)   8/16bit mode.

    @section HISTORY
		2010.03.01	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added ILI9328 device.
							Added 3-Wire Serial Support.
		2011.03.10	V4.00	C++ Ready.
		2011.10.25	V5.00	Added OPFPCT0634-V0 Consideration.
							Added DMA Transaction Support.
		2012.01.18	V6.00	Added ST7781 device.
		2012.04.20	V7.00	Added R61505W & R61505V device.
		2012.09.30	V8.00	Added SPDF5408A/B,RM68050 device.
						    Added LGDP4535 device.
		2013.01.03	V9.00   Added ILI9331,R61580,RM68090 device.
		2013.04.01 V10.00   Added FT1505C device.
							Fixed GGRAM Addressing.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ILI932X_H
#define __ILI932X_H	0x1000

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ILI932x unique value */
/* mst be need for ILI932x */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320
/* MUST be need SPI access */
#define ILI932x_ID			(0x70)	/* 01110*** */
#define ID_IM0				(0<<2)
#define RS_CMD				(0<<1)
#define RS_DATA				(1<<1)
#define RW_WRITE			(0<<0)
#define RW_READ				(1<<0)
#define SET_INDEX			(RS_CMD  | RW_WRITE)
#define WRITE_DATA			(RS_DATA | RW_WRITE)
#define READ_STATUS			(RS_CMD  | RW_READ)
#define READ_DATA			(RS_DATA | RW_READ)
#define START_WR_CMD		(ILI932x_ID | ID_IM0 | SET_INDEX)
#define START_WR_DATA		(ILI932x_ID | ID_IM0 | WRITE_DATA)
#define START_RD_STATUS		(ILI932x_ID | ID_IM0 | READ_STATUS)
#define START_RD_DATA		(ILI932x_ID | ID_IM0 | READ_DATA)

/* Display Contol Macros */
#define ILI932x_RES_SET()	DISPLAY_RES_SET()
#define ILI932x_RES_CLR()	DISPLAY_RES_CLR()
#define ILI932x_CS_SET()	DISPLAY_CS_SET()
#define ILI932x_CS_CLR()	DISPLAY_CS_CLR()
#define ILI932x_DC_SET()	DISPLAY_DC_SET()
#define ILI932x_DC_CLR()	DISPLAY_DC_CLR()
#define ILI932x_WR_SET()	DISPLAY_WR_SET()
#define ILI932x_WR_CLR()	DISPLAY_WR_CLR()
#define ILI932x_RD_SET()	DISPLAY_RD_SET()
#define ILI932x_RD_CLR()	DISPLAY_RD_CLR()
#define ILI932x_SCK_SET()	DISPLAY_SCK_SET()
#define ILI932x_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI932x_WR()   	ILI932x_WR_CLR(); \
							ILI932x_WR_SET();
#else
 #define ILI932x_WR()
#endif

#define	ILI932x_DATA		DISPLAY_DATAPORT
#define ILI932x_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI932x_reset(void);
extern void ILI932x_init(void);
extern void ILI932x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI932x_wr_cmd(uint8_t cmd);
extern void ILI932x_wr_dat(uint16_t dat);
extern void ILI932x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI932x_clear(void);
extern int  ILI932x_draw_bmp(const uint8_t* ptr);
extern uint16_t ILI932x_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */
#define Display_init_if			ILI932x_init
#define Display_rect_if 		ILI932x_rect
#define Display_wr_dat_if		ILI932x_wr_dat
#define Display_wr_cmd_if		ILI932x_wr_cmd
#define Display_wr_block_if		ILI932x_wr_block
#define Display_clear_if 		ILI932x_clear
#define Display_draw_bmp_if		ILI932x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ILI932X_H */
