/********************************************************************************/
/*!
	@file			lgdp4511.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive KTFT4605FPC-B3-E TFT module(16bit bus).

    @section HISTORY
		2011.06.04	V1.00	Stable Release
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __LGDP4511_H
#define __LGDP4511_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* LGDP4511 unique value */
/* mst be need for LGDP4511 */
#define OFS_COL				4
#define OFS_RAW				0
#define MAX_X				128
#define MAX_Y				160

/* Display Contol Macros */
#define LGDP4511_RES_SET()	DISPLAY_RES_SET()
#define LGDP4511_RES_CLR()	DISPLAY_RES_CLR()
#define LGDP4511_CS_SET()	DISPLAY_CS_SET()
#define LGDP4511_CS_CLR()	DISPLAY_CS_CLR()
#define LGDP4511_DC_SET()	DISPLAY_DC_SET()
#define LGDP4511_DC_CLR()	DISPLAY_DC_CLR()
#define LGDP4511_WR_SET()	DISPLAY_WR_SET()
#define LGDP4511_WR_CLR()	DISPLAY_WR_CLR()
#define LGDP4511_RD_SET()	DISPLAY_RD_SET()
#define LGDP4511_RD_CLR()	DISPLAY_RD_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define LGDP4511_WR()   	LGDP4511_WR_CLR(); \
							LGDP4511_WR_SET();
#else
 #define LGDP4511_WR()
#endif

#define	LGDP4511_DATA		DISPLAY_DATAPORT
#define LGDP4511_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void LGDP4511_reset(void);
extern void LGDP4511_init(void);
extern void LGDP4511_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void LGDP4511_wr_cmd(uint8_t cmd);
extern void LGDP4511_wr_dat(uint16_t dat);
extern void LGDP4511_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void LGDP4511_clear(void);
extern int  LGDP4511_draw_bmp(const uint8_t* ptr);
extern uint16_t LGDP4511_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			LGDP4511_init
#define Display_rect_if 		LGDP4511_rect
#define Display_wr_dat_if		LGDP4511_wr_dat
#define Display_wr_cmd_if		LGDP4511_wr_cmd
#define Display_wr_block_if		LGDP4511_wr_block
#define Display_clear_if 		LGDP4511_clear
#define Display_draw_bmp_if		LGDP4511_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __LGDP4511_H */
