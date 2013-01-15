/********************************************************************************/
/*!
	@file			hx8340bn.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2012.05.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive BTL221722-276L TFT module.(9-bit serial only).

    @section HISTORY
		2011.12.01	V1.00	Stable Release.
		2012.05.25  V2.00	Added Hardware 9-bitSerial Handling.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HX8340BN_H
#define __HX8340BN_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* HX8340BN Unique Value	*/
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				176
#define MAX_Y				220

/* MUST be need Software-SPI access (ONLY for HX8340BN(N)!) */
#ifdef USE_HX8340BN_SPI_TFT
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

#endif

/* Display Contol Macros */
#define HX8340BN_RES_SET()	DISPLAY_RES_SET()
#define HX8340BN_RES_CLR()	DISPLAY_RES_CLR()
#define HX8340BN_CS_SET()	DISPLAY_CS_SET()
#define HX8340BN_CS_CLR()	DISPLAY_CS_CLR()
#define HX8340BN_DC_SET()	DISPLAY_DC_SET()
#define HX8340BN_DC_CLR()	DISPLAY_DC_CLR()
#define HX8340BN_WR_SET()	DISPLAY_WR_SET()
#define HX8340BN_WR_CLR()	DISPLAY_WR_CLR()
#define HX8340BN_RD_SET()	DISPLAY_RD_SET()
#define HX8340BN_RD_CLR()	DISPLAY_RD_CLR()
#define HX8340BN_SCK_SET()	DISPLAY_SCK_SET()
#define HX8340BN_SCK_CLR()	DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define HX8340BN_WR()   	HX8340BN_WR_CLR(); \
							HX8340BN_WR_SET();
#else
 #define HX8340BN_WR()
#endif

#define	HX8340BN_DATA		DISPLAY_DATAPORT
#define HX8340BN_CMD		DISPLAY_CMDPORT


/* Display Control Functions Prototype */
extern void HX8340BN_reset(void);
extern void HX8340BN_init(void);
extern void HX8340BN_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void HX8340BN_wr_cmd(uint8_t cmd);
extern void HX8340BN_wr_dat(uint8_t dat);
extern void HX8340BN_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void HX8340BN_clear(void);
extern int  HX8340BN_draw_bmp(const uint8_t* ptr);
extern void HX8340BN_wr_gram(uint16_t gram);
extern uint8_t HX8340BN_rd_cmd(uint8_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			HX8340BN_init
#define Display_rect_if 		HX8340BN_rect
#define Display_wr_dat_if		HX8340BN_wr_gram
#define Display_wr_cmd_if		HX8340BN_wr_cmd
#define Display_wr_block_if		HX8340BN_wr_block
#define Display_clear_if 		HX8340BN_clear
#define Display_draw_bmp_if		HX8340BN_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __HX8340BN_H */
