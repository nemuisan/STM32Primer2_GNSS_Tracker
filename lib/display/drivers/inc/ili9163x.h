/********************************************************************************/
/*!
	@file			ili9163x.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2012.08.31
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -SGP18T-00		(ILI9163B)	4-Wire,8-bitSerial.				@n
					 -SDT018ATFT	(ILI9163C)	8/16bit & 3-Wire,9bitSerial.	@n
					 -S93235Z		(ILI9163B)	4-Wire,8-bitSerial.				@n
					 -S93160		(ILI9163B)	8bit mode only.

    @section HISTORY
		2011.12.23	V1.00	Revised From ILI9163B Driver.
		2012.01.18	V2.00	Added S93235Z Module Support.
		2012.05.25  V3.00	Added Hardware 9-bitSerial Handling.
		2012.08.31  V4.00	Added S93160 Module Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ILI9163X_H
#define __ILI9163X_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>

/* display includes */
#include "display_if_basis.h"

/* Module Driver Configure */
/* U MUST select one from those modules */
#define USE_S93235Z
//#define USE_SGP18T_00
//#define USE_SDT018ATFT
//#define USE_S93160

/* ILI9163x Unique Value */
#if defined(USE_S93235Z)
#define OFS_COL			0
#define OFS_RAW			0
#define MAX_X			128
#define MAX_Y			128
#define PWCTR1VAL		0x0A
#define VMCTR1VALH		0x4F
#define VMCTR1VALL		0x5A
#define VMOFCTRVAL		0x40
#define MADVAL			0xC8
#define USEMODULE		S95325Z-1
#define ILI9163xSPI_4WIREMODE

#elif  defined(USE_SGP18T_00)
#define OFS_COL			0
#define OFS_RAW			0
#define MAX_X			128
#define MAX_Y			160
#define PWCTR1VAL		0x0A
#define VMCTR1VALH		0x4F
#define VMCTR1VALL		0x5A
#define VMOFCTRVAL		0x40
#define MADVAL			0xC0
#define USEMODULE		SGP18T-00
#define ILI9163xSPI_4WIREMODE

#elif  defined(USE_SDT018ATFT)
/* MUST be need Software-SPI access some MCUs (ONLY for ILI9163C!) */
#define OFS_COL			0
#define OFS_RAW			0
#define MAX_X			128
#define MAX_Y			160
#define PWCTR1VAL		0x0A
#define VMCTR1VALH		0x4F
#define VMCTR1VALL		0x5A
#define VMOFCTRVAL		0x40
#define MADVAL			0xC8
#define USEMODULE		SDT018ATFT
#define ILI9163xSPI_3WIREMODE
#endif

#if defined(USE_S93160)
#define OFS_COL			0
#define OFS_RAW			0
#define MAX_X			128
#define MAX_Y			128
#define PWCTR1VAL		0x0A
#define VMCTR1VALH		0x4F
#define VMCTR1VALL		0x5A
#define VMOFCTRVAL		0x40
#define MADVAL			0xC8
#define USEMODULE		S93160
#define ILI9163xSPI_4WIREMODE
#endif

#ifdef ILI9163xSPI_3WIREMODE
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

/* Display Contol Macros */
#define ILI9163x_RES_SET()		DISPLAY_RES_SET()
#define ILI9163x_RES_CLR()		DISPLAY_RES_CLR()
#define ILI9163x_CS_SET()		DISPLAY_CS_SET()
#define ILI9163x_CS_CLR()		DISPLAY_CS_CLR()
#if 	defined(ILI9163xSPI_3WIREMODE)
 #define ILI9163x_DC_SET()
 #define ILI9163x_DC_CLR()
#elif	defined(ILI9163xSPI_4WIREMODE)
 #define ILI9163x_DC_SET()		DISPLAY_DC_SET()
 #define ILI9163x_DC_CLR()		DISPLAY_DC_CLR()
#else
 #error "U MUST Select ILI9163x SPI Mode!!"
#endif
#define ILI9163x_WR_SET()		DISPLAY_WR_SET()
#define ILI9163x_WR_CLR()		DISPLAY_WR_CLR()
#define ILI9163x_RD_SET()		DISPLAY_RD_SET()
#define ILI9163x_RD_CLR()		DISPLAY_RD_CLR()
#define ILI9163x_SDATA_SET()	DISPLAY_SDI_SET()
#define ILI9163x_SDATA_CLR()	DISPLAY_SDI_CLR()
#define ILI9163x_SCLK_SET()		DISPLAY_SCK_SET()
#define ILI9163x_SCLK_CLR()		DISPLAY_SCK_CLR()


#if defined(GPIO_ACCESS_8BIT) | defined(GPIO_ACCESS_16BIT)
 #define ILI9163x_WR()   	ILI9163x_WR_CLR(); \
							ILI9163x_WR_SET();
#else
 #define ILI9163x_WR()
#endif

#define	ILI9163x_DATA		DISPLAY_DATAPORT
#define ILI9163x_CMD		DISPLAY_CMDPORT


/* ILI9163x Instruction */
#define NO_OP		(0x00)
#define SWRESET		(0x01)
#define RDDID		(0x04)
#define RDDST		(0x09)
#define RDDPM		(0x0A)
#define RDDMADCTL	(0x0B)
#define RDDCOLMOD	(0x0C)
#define RDDIM		(0x0D)
#define RDDSM		(0x0E)
#define SLPIN		(0x10)
#define SLPOUT		(0x11)
#define PTLON		(0x12)
#define NORON		(0x13)
#define INVOFF		(0x20)
#define INVON		(0x21)
#define GAMSET		(0x26)
#define DISPOFF		(0x28)
#define DISPON		(0x29)
#define CASET		(0x2A)
#define RASET		(0x2B)
#define RAMWR		(0x2C)
#define RAMRD		(0x2E)
#define PTLAR		(0x30)
#define TEOFF		(0x34)
#define TEON		(0x35)
#define MADCTL		(0x36)
#define IDMOFF		(0x38)
#define IDMON		(0x39)
#define COLMOD		(0x3A)
#define RDID1		(0xDA)
#define RDID2		(0xDB)
#define RDID3		(0xDC)

/* ILI9163x Panel Function Command List */
#define FRMCTR1		(0xB1)
#define FRMCTR2		(0xB2)
#define FRMCTR3		(0xB3)
#define INVCTR		(0xB4)
#define DISSET5		(0xB6)
#define SRCINVCTR	(0xB7)
#define PWCTR1		(0xC0)
#define PWCTR2		(0xC1)
#define PWCTR3		(0xC2)
#define PWCTR4		(0xC3)
#define PWCTR5		(0xC4)
#define VMCTR1		(0xC5)
#define VMOFCTR		(0xC7)
#define WRID2		(0xD1)
#define WRID3		(0xD2)
#define PWCTR6		(0xFC)
#define NVFCTR1		(0xD9)
#define NVFCTR2		(0xDE)
#define NVFCTR3		(0xDF)
#define GAMCTRP0	(0xE0)
#define GAMCTRN0	(0xE1)
#define EXTCTRL		(0xF0)
#define GAM_R_SEL   (0xF2)
#define VCOM4L		(0xFF)


/* Display Control Functions Prototype */
extern void ILI9163x_reset(void);
extern void ILI9163x_init(void);
extern void ILI9163x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height);
extern void ILI9163x_wr_cmd(uint8_t cmd);
extern void ILI9163x_wr_dat(uint8_t dat);
extern void ILI9163x_wr_block(uint8_t* blockdata,unsigned int datacount);
extern void ILI9163x_clear(void);
extern int  ILI9163x_draw_bmp(const uint8_t* ptr);
extern void ILI9163x_wr_gram(uint16_t gram);
extern uint16_t ILI9163x_rd_cmd(uint16_t cmd);

/* For Display Module's Delay Routine */
#define Display_timerproc_if()	ticktime++
extern volatile uint32_t ticktime;

/* Macros From Application Layer */ 
#define Display_init_if			ILI9163x_init
#define Display_rect_if 		ILI9163x_rect
#define Display_wr_dat_if		ILI9163x_wr_gram
#define Display_wr_cmd_if		ILI9163x_wr_cmd
#define Display_wr_block_if		ILI9163x_wr_block
#define Display_clear_if 		ILI9163x_clear
#define Display_draw_bmp_if		ILI9163x_draw_bmp

#ifdef __cplusplus
}
#endif

#endif /* __ILI9163X_H */
