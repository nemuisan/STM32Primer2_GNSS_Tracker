/********************************************************************************/
/*!
	@file			ili9327.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -DST9901A-NH				(ILI9327)	8/16bit mode.		@n
					 -S95361A					(ILI9327)   8/16bit mode.       @n

    @section HISTORY
		2010.10.01	V1.00	Stable Release
		2010.12.31	V2.00	Cleanup SourceCode.
		2011.03.10	V3.00	C++ Ready.
		2011.10.25	V4.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ILI9327_H
#define __ILI9327_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ILI9327 unique value */
/* mst be need for ILI9327 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				400

/* Display Contol Macros */
#define ILI9327_RES_SET()	DISPLAY_RES_SET()
#define ILI9327_RES_CLR()	DISPLAY_RES_CLR()
#define ILI9327_CS_SET()	DISPLAY_CS_SET()
#define ILI9327_CS_CLR()	DISPLAY_CS_CLR()
#define ILI9327_DC_SET()	DISPLAY_DC_SET()
#define ILI9327_DC_CLR()	DISPLAY_DC_CLR()
#define ILI9327_WR_SET()	DISPLAY_WR_SET()
#define ILI9327_WR_CLR()	DISPLAY_WR_CLR()
#define ILI9327_RD_SET()	DISPLAY_RD_SET()
#define ILI9327_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9327_WR()   	ILI9327_WR_CLR(); \
							ILI9327_WR_SET();
#else
 #define ILI9327_WR()
#endif

#define	ILI9327_DATA		DISPLAY_DATAPORT
#define ILI9327_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI9327_reset(void);
extern void ILI9327_init(void);
extern void ILI9327_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9327_wr_cmd(uint8_t cmd);
extern void ILI9327_wr_dat(uint8_t dat);
extern void ILI9327_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9327_clear(void);
extern int  ILI9327_draw_bmp(const uint8_t* ptr);
extern uint16_t ILI9327_rd_cmd(uint16_t cmd);
extern void ILI9327_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			ILI9327_init
#define Display_rect_if 		ILI9327_rect
#define Display_wr_dat_if		ILI9327_wr_gram
#define Display_wr_cmd_if		ILI9327_wr_cmd
#define Display_wr_block_if		ILI9327_wr_block
#define Display_clear_if 		ILI9327_clear
#define Display_draw_bmp_if		ILI9327_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ILI9327_H */
