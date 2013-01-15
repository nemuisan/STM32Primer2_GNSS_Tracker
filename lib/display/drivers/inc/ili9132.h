/********************************************************************************/
/*!
	@file			ili9132.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CMI-35 TFT module.

    @section HISTORY
		2011.09.14	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ILI9132_H
#define __ILI9132_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* ILI9132 unique value */
/* mst be need for ILI9132 */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				128
#define MAX_Y				128

/* Display Contol Macros */
#define ILI9132_RES_SET()	DISPLAY_RES_SET()
#define ILI9132_RES_CLR()	DISPLAY_RES_CLR()
#define ILI9132_CS_SET()	DISPLAY_CS_SET()
#define ILI9132_CS_CLR()	DISPLAY_CS_CLR()
#define ILI9132_DC_SET()	DISPLAY_DC_SET()
#define ILI9132_DC_CLR()	DISPLAY_DC_CLR()
#define ILI9132_WR_SET()	DISPLAY_WR_SET()
#define ILI9132_WR_CLR()	DISPLAY_WR_CLR()
#define ILI9132_RD_SET()	DISPLAY_RD_SET()
#define ILI9132_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9132_WR()   	ILI9132_WR_CLR(); \
							ILI9132_WR_SET();
#else
 #define ILI9132_WR()
#endif

#define	ILI9132_DATA		DISPLAY_DATAPORT
#define ILI9132_CMD			DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void ILI9132_reset(void);
extern void ILI9132_init(void);
extern void ILI9132_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9132_wr_cmd(uint16_t cmd);
extern void ILI9132_wr_dat(uint16_t dat);
extern void ILI9132_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9132_clear(void);
extern int  ILI9132_draw_bmp(const uint8_t* ptr);
extern uint16_t ILI9132_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			ILI9132_init
#define Display_rect_if 		ILI9132_rect
#define Display_wr_dat_if		ILI9132_wr_dat
#define Display_wr_cmd_if		ILI9132_wr_cmd
#define Display_wr_block_if		ILI9132_wr_block
#define Display_clear_if 		ILI9132_clear
#define Display_draw_bmp_if		ILI9132_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ILI9132_H */
