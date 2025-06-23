/********************************************************************************/
/*!
	@file			st7732.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2025.06.19
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -STM32 Primer2				(ST7732)	(Shifted 8bit mode.)

    @section HISTORY
		2010.03.24	V1.00	Stable Release.
		2010.09.06	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added GRAM write function.
		2011.03.10	V4.00	C++ Ready.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.
		2025.06.19	V7.00	Fixed implicit cast warnings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ST7732_H
#define ST7732_H 0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* Basic Includes */
#include <string.h>
#include <inttypes.h>

/* Display Includes */
#include "display_if_basis.h"

/* ST7732 Unique Value		*/
/* MUST be need for ST7732	*/
#define OFS_COL				0
#define OFS_RAW				0
#define MAX_X				128
#define MAX_Y				160

/* Display Contol Macros */
#define ST7732_RES_SET()	DISPLAY_RES_SET()
#define ST7732_RES_CLR()	DISPLAY_RES_CLR()
#define ST7732_CS_SET()		DISPLAY_CS_SET()
#define ST7732_CS_CLR()		DISPLAY_CS_CLR()
#define ST7732_DC_SET()		DISPLAY_DC_SET()
#define ST7732_DC_CLR()		DISPLAY_DC_CLR()
#define ST7732_WR_SET()		DISPLAY_WR_SET()
#define ST7732_WR_CLR()		DISPLAY_WR_CLR()
#define ST7732_RD_SET()		DISPLAY_RD_SET()
#define ST7732_RD_CLR()		DISPLAY_RD_CLR()

#define	ST7732_DATA			DISPLAY_DATAPORT
#define ST7732_CMD			DISPLAY_CMDPORT


/* ST7732 Instruction */
#define NOP				(0x00)
#define SWRESET			(0x01)
#define RDDID			(0x04)
#define RDDST			(0x09)
#define RDDPM			(0x0A)
#define RDDMADCTR		(0x0B)
#define RDDCOLMOD		(0x0C)
#define RDDIM			(0x0D)
#define RDDSM			(0x0E)
#define RDDSDR			(0x0F)

#define SLPIN 			(0x10)
#define SLPOUT			(0x11)
#define PTLON			(0x12)
#define NORON			(0x13)

#define INVOFF			(0x20)
#define INVON			(0x21)
#define GAMSET			(0x26)
#define DISPOFF			(0x28)
#define DISPON			(0x29)
#define CASET 			(0x2A)
#define RASET			(0x2B)
#define RAMWR			(0x2C)
#define RGBSET			(0x2D)
#define RAMRD			(0x2E)

#define PTLAR			(0x30)
#define SCRLAR			(0x33)
#define TEOFF			(0x34)
#define TEON			(0x35)
#define MADCTL 			(0x36)
#define VSCSAD			(0x37)
#define IDMOFF			(0x38)
#define IDMON			(0x39)
#define COLMOD			(0x3A)

#define RDID1			(0xDA)
#define RDID2			(0xDB)
#define RDID3			(0xDC)

#define RGBCTR			(0xB0)
#define FRMCTR1			(0xB1)
#define FRMCTR2			(0xB2)
#define FRMCTR3			(0xB3)
#define INVCTR			(0xB4)
#define RGBBPCTR		(0xB5)
#define DISSET5			(0xB6)

#define PWCTR1			(0xC0)
#define PWCTR2			(0xC1)
#define PWCTR3			(0xC2)
#define PWCTR4			(0xC3)
#define PWCTR5			(0xC4)
#define VMCTR1			(0xC5)
#define VMOFCTR			(0xC7)

#define WRID2			(0xD1)
#define WRID3			(0xD2)
#define RDID4			(0xD3)
#define NVCTR1			(0xD9)
#define NVCTR2			(0xDE)
#define NVCTR3			(0xDF)

#define GAMCTRP1		(0xE0)
#define GAMCTRN1		(0xE1)

#define AUTOCTRL		(0xF1)
#define OSCADJ			(0xF2)
#define DISPCTRL		(0xF5)
#define DEFADJ			(0xF6)


/* Display Control Functions Prototype */
extern void ST7732_reset(void);
extern void ST7732_init(void);
extern void ST7732_clear(void);
extern void ST7732_rect(uint16_t x, uint16_t width, uint16_t y, uint16_t height);
extern void ST7732_wr_cmd(uint8_t cmd);
extern void ST7732_wr_dat(uint8_t dat);
extern void ST7732_wr_block(uint8_t *p,unsigned int cnt);
extern uint16_t ST7732_rd_cmd(uint8_t cmd);
extern void ST7732_wr_gram(uint16_t gram);


/* Macros From Application Layer */ 
#define Display_init_if			ST7732_init
#define Display_rect_if			ST7732_rect
#define Display_wr_dat_if		ST7732_wr_gram
#define Display_wr_cmd_if		ST7732_wr_cmd
#define Display_wr_block_if		ST7732_wr_block
#define Display_clear_if		ST7732_clear

#ifdef __cplusplus
}
#endif

#endif /* ST7732_H */
