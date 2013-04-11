/********************************************************************************/
/*!
	@file			ssd1339.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.10.31
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG-2828GFEFF01 OLED module(8bit,spi mode).

    @section HISTORY
		2011.10.31	V1.00	Renewal from SED1339.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1339.h"
/* check header file version for fool proof */
#if __SSD1339_H != 0x0100
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Abstract Layer Delay Settings.
*/
/**************************************************************************/
#ifndef __SYSTICK_H
volatile uint32_t ticktime;
static inline void _delay_ms(uint32_t ms)
{
	ms += ticktime;
	while (ticktime < ms);
}
#endif

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void SSD1339_reset(void)
{
#ifdef USE_SSD1339_OLED
	SSD1339_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	SSD1339_RD_SET();
	SSD1339_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	SSD1339_RES_CLR();							/* RES=L, CS=L   			*/
	SSD1339_CS_CLR();

#elif  USE_SSD1339_SPI_OLED
	SSD1339_RES_SET();							/* RES=H, CS=H				*/
	SSD1339_CS_SET();
	SSD1339_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	SSD1339_RES_CLR();							/* RES=L		   			*/
#endif

	_delay_ms(10);								/* wait 10ms     			*/
	
	SSD1339_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_SSD1339_OLED
/**************************************************************************/
/*! 
    Write OLED Command.
*/
/**************************************************************************/
inline void SSD1339_wr_cmd(uint8_t cmd)
{
	SSD1339_DC_CLR();							/* DC=L		     */
	
	SSD1339_CMD = cmd;							/* D7..D0=cmd    */
	SSD1339_WR();								/* WR=L->H       */
	
	SSD1339_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write OLED Data.
*/
/**************************************************************************/
inline void SSD1339_wr_dat(uint8_t dat)
{
	SSD1339_DATA = dat;							/* D7..D0=dat    */
	SSD1339_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write OLED GRAM.
*/
/**************************************************************************/
inline void SSD1339_wr_gram(uint16_t gram)
{
	SSD1339_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	SSD1339_WR();								/* WR=L->H					*/

	SSD1339_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	SSD1339_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write OLED Block Data.
*/
/**************************************************************************/
inline void SSD1339_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		SSD1339_wr_dat (*p++);
		SSD1339_wr_dat (*p++);
		SSD1339_wr_dat (*p++);
		SSD1339_wr_dat (*p++);
	}
	while (n--) {
		SSD1339_wr_dat (*p++);
	}
#endif

}

#elif USE_SSD1339_SPI_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD1339_wr_cmd(uint8_t cmd)
{
	SSD1339_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD1339_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void SSD1339_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void SSD1339_wr_gram(uint16_t gram)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SSD1339_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt );
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
	}
#endif

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}
#endif

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1339_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SSD1339_wr_cmd(0x15);
	SSD1339_wr_dat(OFS_COL + x);
	SSD1339_wr_dat(OFS_COL + width);

	/* Set RAS Address */
	SSD1339_wr_cmd(0x75);
	SSD1339_wr_dat(OFS_RAW + y);
	SSD1339_wr_dat(OFS_RAW + height); 

	/* Write RAM */
	SSD1339_wr_cmd(0x5C);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1339_clear(void)
{
	volatile uint32_t n;

	SSD1339_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
	    /* 16Bit Colour Access */
		SSD1339_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    OLED Module Initialize.
*/
/**************************************************************************/
void SSD1339_init(void)
{
	Display_IoInit_If();
	
	SSD1339_reset();
	
	/* SSD1339 */
	SSD1339_wr_cmd(0xA0);	/* Set Re-map / Color Depth */
	SSD1339_wr_dat(0x74);	/* 64K color, COM split, COM remap, 8bitBus, Color remap, 
								Non column address remap, Hotizontal increment */
	SSD1339_wr_cmd(0xA1);	/* Set display start line */
	SSD1339_wr_dat(0x00);	/* 00h start */

	SSD1339_wr_cmd(0xA2);	/* Set display offset */
	SSD1339_wr_dat(0x80);	/* 80h start */

	SSD1339_wr_cmd(0xA6);	/* Normal display */

	SSD1339_wr_cmd(0xAD);	/* Set Master Configuration */
	SSD1339_wr_dat(0x8E);	/* DC-DC off & external VcomH voltage & external pre-charge voltage */

	SSD1339_wr_cmd(0xB0);	/* Power saving mode */
	SSD1339_wr_dat(0x05);

	SSD1339_wr_cmd(0xB1);	/* Set pre & dis_charge */
	SSD1339_wr_dat(0x11);	/* pre=1h dis=1h */

	SSD1339_wr_cmd(0xB3);	/* clock & frequency */
 	SSD1339_wr_dat(0x61);	/* clock=Divser+2 frequency=6 */

	SSD1339_wr_cmd(0xBB);	/* Set pre-charge voltage of color A B C */
	SSD1339_wr_dat(0x1C);	/* color A */
	SSD1339_wr_dat(0x1C);	/* color B */
	SSD1339_wr_dat(0x1C);	/* color C */

	SSD1339_wr_cmd(0xBE);	/* Set VcomH */
	SSD1339_wr_dat(0x1F);	

	SSD1339_wr_cmd(0xc1);	/* Set contrast current for A B C */
	SSD1339_wr_dat(0xaa);	/* Color A */
	SSD1339_wr_dat(0xb4);	/* Color B */ 
	SSD1339_wr_dat(0xc8);	/* Color C */

	SSD1339_wr_cmd(0xc7);	/* Set master contrast */
	SSD1339_wr_dat(0x0f);	/* no change */

	SSD1339_wr_cmd(0xCA);	/* Duty */ 
	SSD1339_wr_dat(0x7F);	/* 128 */

	SSD1339_clear();

	SSD1339_wr_cmd(0xAF);	/* Display on */
	_delay_ms(10);

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1339_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1339_wr_gram(COL_RED);
	} while (--n);

	_delay_ms(500);
	for(;;);

#endif

}


/**************************************************************************/
/*! 
    Draw Windows 24bitBMP File.
*/
/**************************************************************************/
int SSD1339_draw_bmp(const uint8_t* ptr){

	uint32_t n, m, biofs, bw, iw, bh, w;
	uint32_t xs, xe, ys, ye, i;
	uint8_t *p;
	uint16_t d;

	/* Load BitStream Address Offset  */
	biofs = LD_UINT32(ptr+10);
	/* Check Plane Count "1" */
	if (LD_UINT16(ptr+26) != 1)  return 0;
	/* Check BMP bit_counts "24(windows bitmap standard)" */
	if (LD_UINT16(ptr+28) != 24) return 0;
	/* Check BMP Compression "BI_RGB(no compresstion)"*/
	if (LD_UINT32(ptr+30) != 0)  return 0;
	/* Load BMP width */
	bw = LD_UINT32(ptr+18);
	/* Load BMP height */
	bh = LD_UINT32(ptr+22);
	/* Check BMP width under 1280px */
	if (!bw || bw > 1280 || !bh) return 0;
	
	/* Calculate Data byte count per holizontal line */
	iw = ((bw * 3) + 3) & ~3;

	/* Centering */
	if (bw > MAX_X) {
		xs = 0; xe = MAX_X-1;
	} else {
		xs = (MAX_X - bw) / 2;
		xe = (MAX_X - bw) / 2 + bw - 1;
	}
	if (bh > MAX_Y) {
		ys = 0; ye = MAX_Y-1;
	} else {
		ys = (MAX_Y - bh) / 2;
		ye = (MAX_Y - bh) / 2 + bh - 1;
	}

	/* Clear Display */
	SSD1339_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SSD1339_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SSD1339_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
