/********************************************************************************/
/*!
	@file			s6b33b6x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.02.10
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG12D245A CSTN module.							@n
					8/16bit-i8080,4wire8bit/3wire9bit Serial Support!

    @section HISTORY
		2012.02.10	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __S6B33B6X_H
#define __S6B33B6X_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* S6B33B6x Unique Value */
/* mst be need for S6B33B6x */
#define OFS_COL			2
#define OFS_RAW			2
#define MAX_X			128
#define MAX_Y			128

#define S6B33B6xSPI_4WIREMODE
/*#define S6B33B6xSPI_3WIREMODE*/

#ifdef S6B33B6xSPI_3WIREMODE
#ifndef USE_SOFTWARE_SPI
 #error "9bit-SPI Does not Support on Hardware 8bit-SPI Handling !!"
#else
 #define DNC_CMD()			DISPLAY_SDI_CLR();	\
							CLK_OUT();
 #define DNC_DAT()			DISPLAY_SDI_SET();	\
							CLK_OUT();
#endif

#else
 #define DNC_CMD()	
 #define DNC_DAT()	
#endif

/* Display Contol Macros */
#define S6B33B6x_RES_SET()		DISPLAY_RES_SET()
#define S6B33B6x_RES_CLR()		DISPLAY_RES_CLR()
#define S6B33B6x_CS_SET()		DISPLAY_CS_SET()
#define S6B33B6x_CS_CLR()		DISPLAY_CS_CLR()
#if 	defined(S6B33B6xSPI_3WIREMODE)
 #define S6B33B6x_DC_SET()
 #define S6B33B6x_DC_CLR()
#elif	defined(S6B33B6xSPI_4WIREMODE)
 #define S6B33B6x_DC_SET()		DISPLAY_DC_SET()
 #define S6B33B6x_DC_CLR()		DISPLAY_DC_CLR()
#else
 #error "U MUST Select S6B33B6x SPI Mode!!"
#endif
#define S6B33B6x_WR_SET()		DISPLAY_WR_SET()
#define S6B33B6x_WR_CLR()		DISPLAY_WR_CLR()
#define S6B33B6x_RD_SET()		DISPLAY_RD_SET()
#define S6B33B6x_RD_CLR()		DISPLAY_RD_CLR()
#define S6B33B6x_SDATA_SET()	DISPLAY_SDI_SET()
#define S6B33B6x_SDATA_CLR()	DISPLAY_SDI_CLR()
#define S6B33B6x_SCLK_SET()		DISPLAY_SCK_SET()
#define S6B33B6x_SCLK_CLR()		DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define S6B33B6x_WR()   	S6B33B6x_WR_CLR(); \
							S6B33B6x_WR_SET();
#else
 #define S6B33B6x_WR()
#endif

#define	S6B33B6x_DATA		DISPLAY_DATAPORT
#define S6B33B6x_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void S6B33B6x_reset(void);
extern void S6B33B6x_init(void);
extern void S6B33B6x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void S6B33B6x_wr_cmd(uint8_t cmd);
extern void S6B33B6x_wr_dat(uint8_t dat);
extern void S6B33B6x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void S6B33B6x_clear(void);
extern int  S6B33B6x_draw_bmp(const uint8_t* ptr);
extern void S6B33B6x_wr_gram(uint16_t gram);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			S6B33B6x_init
#define Display_rect_if 		S6B33B6x_rect
#define Display_wr_dat_if		S6B33B6x_wr_gram
#define Display_wr_cmd_if		S6B33B6x_wr_cmd
#define Display_wr_block_if		S6B33B6x_wr_block
#define Display_clear_if 		S6B33B6x_clear
#define Display_draw_bmp_if		S6B33B6x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __S6B33B6X_H */
