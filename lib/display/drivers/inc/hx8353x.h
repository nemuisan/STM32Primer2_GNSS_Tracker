/********************************************************************************/
/*!
	@file			hx8353x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -NHD-1.8-128160TF-CTXI#	(HX8353D)		8bit mode only.

    @section HISTORY
		2012.11.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX8353X_H
#define __HX8353X_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8353x Unique Value		*/
#define OFS_COL			0
#define OFS_RAW			0
#define MAX_X			128
#define MAX_Y			160
#define HX8353xSPI_3WIREMODE


#ifndef USE_HX8353x_TFT

#ifdef HX8353xSPI_3WIREMODE
#if defined(USE_HARDWARE_SPI)
 #if defined(SUPPORT_HARDWARE_9BIT_SPI)
  #define DNC_CMD()	
  #define DNC_DAT()	
 #else
  #error "9bit-SPI Does not Support on Hardware 8bit-SPI Handling !!"
 #endif
#elif defined(USE_SOFTWARE_SPI)
 #define DNC_CMD()			DISPLAY_SDI_CLR();	\
							CLK_OUT();
 #define DNC_DAT()			DISPLAY_SDI_SET();	\
							CLK_OUT();
#else
  #error "NOT Defined SPI Handling !"
#endif

#else /* 4WIRE-8BitMODE */
 #if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
  #error "4-WireMode Supports 8bit-SPI Handling ONLY !!"
 #endif
 #define DNC_CMD()	
 #define DNC_DAT()	
#endif

#endif


/* Display Contol Macros */
#define HX8353x_RES_SET()		DISPLAY_RES_SET()
#define HX8353x_RES_CLR()		DISPLAY_RES_CLR()
#define HX8353x_CS_SET()		DISPLAY_CS_SET()
#define HX8353x_CS_CLR()		DISPLAY_CS_CLR()
#if 	defined(HX8353xSPI_3WIREMODE)
 #define HX8353x_DC_SET()
 #define HX8353x_DC_CLR()
#elif 	defined(HX8353xSPI_4WIREMODE)
 #define HX8353x_DC_SET()		DISPLAY_DC_SET()
 #define HX8353x_DC_CLR()		DISPLAY_DC_CLR()
#else
 #error "U MUST Select HX8353x SPI Mode!!"
#endif
#define HX8353x_WR_SET()		DISPLAY_WR_SET()
#define HX8353x_WR_CLR()		DISPLAY_WR_CLR()
#define HX8353x_RD_SET()		DISPLAY_RD_SET()
#define HX8353x_RD_CLR()		DISPLAY_RD_CLR()
#define HX8353x_SDATA_SET()		DISPLAY_SDI_SET()
#define HX8353x_SDATA_CLR()		DISPLAY_SDI_CLR()
#define HX8353x_SCLK_SET()		DISPLAY_SCK_SET()
#define HX8353x_SCLK_CLR()		DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8353x_WR()   		HX8353x_WR_CLR(); \
								HX8353x_WR_SET();
#else
 #define HX8353x_WR()
#endif

#define	HX8353x_DATA			DISPLAY_DATAPORT
#define HX8353x_CMD				DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8353x_reset(void);
extern void HX8353x_init(void);
extern void HX8353x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8353x_wr_cmd(uint8_t cmd);
extern void HX8353x_wr_dat(uint8_t dat);
extern void HX8353x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8353x_clear(void);
extern int  HX8353x_draw_bmp(const uint8_t* ptr);
extern void HX8353x_wr_gram(uint16_t gram);
extern uint16_t HX8353x_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8353x_init
#define Display_rect_if 		HX8353x_rect
#define Display_wr_dat_if		HX8353x_wr_gram
#define Display_wr_cmd_if		HX8353x_wr_cmd
#define Display_wr_block_if		HX8353x_wr_block
#define Display_clear_if 		HX8353x_clear
#define Display_draw_bmp_if		HX8353x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX8353X_H */
