/********************************************************************************/
/*!
	@file			upd161704a.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.07.15
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive LTM022A69B TFT module.							@n
					 8/16bit-i8080,4wire8bit Serial Support!

    @section HISTORY
		2012.07.15	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __UPD161704A_H
#define __UPD161704A_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* UPD161704A unique value */
/* mst be need for UPD161704A */
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				240
#define MAX_Y				320

/* Display Contol Macros */
#define UPD161704A_RES_SET()	DISPLAY_RES_SET()
#define UPD161704A_RES_CLR()	DISPLAY_RES_CLR()
#define UPD161704A_CS_SET()		DISPLAY_CS_SET()
#define UPD161704A_CS_CLR()		DISPLAY_CS_CLR()
#define UPD161704A_DC_SET()		DISPLAY_DC_SET()
#define UPD161704A_DC_CLR()		DISPLAY_DC_CLR()
#define UPD161704A_WR_SET()		DISPLAY_WR_SET()
#define UPD161704A_WR_CLR()		DISPLAY_WR_CLR()
#define UPD161704A_RD_SET()		DISPLAY_RD_SET()
#define UPD161704A_RD_CLR()		DISPLAY_RD_CLR()
#define UPD161704A_SCK_SET()	DISPLAY_SCK_SET()
#define UPD161704A_SCK_CLR()	DISPLAY_SCK_CLR()

#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define UPD161704A_WR()   		UPD161704A_WR_CLR(); \
								UPD161704A_WR_SET();
#else
 #define UPD161704A_WR()
#endif

#define	UPD161704A_DATA			DISPLAY_DATAPORT
#define UPD161704A_CMD			DISPLAY_CMDPORT

/* Display Control Functions Prototype */
extern void UPD161704A_reset(void);
extern void UPD161704A_init(void);
extern void UPD161704A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void UPD161704A_wr_cmd(uint16_t cmd);
extern void UPD161704A_wr_dat(uint16_t dat);
extern void UPD161704A_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void UPD161704A_clear(void);
extern int  UPD161704A_draw_bmp(const uint8_t* ptr);
extern uint8_t UPD161704A_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			UPD161704A_init
#define Display_rect_if 		UPD161704A_rect
#define Display_wr_dat_if		UPD161704A_wr_dat
#define Display_wr_cmd_if		UPD161704A_wr_cmd
#define Display_wr_block_if		UPD161704A_wr_block
#define Display_clear_if 		UPD161704A_clear
#define Display_draw_bmp_if		UPD161704A_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __UPD161704A_H */
