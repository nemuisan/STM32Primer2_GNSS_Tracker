/********************************************************************************/
/*!
	@file			ssd1351.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.06.03
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG-2828GDEDF11 OLED module.

    @section HISTORY
		2012.06.03	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1351.h"
/* check header file version for fool proof */
#if __SSD1351_H != 0x0100
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
inline void SSD1351_reset(void)
{
#ifdef USE_SSD1351_OLED
	SSD1351_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	SSD1351_RD_SET();
	SSD1351_WR_SET();
	_delay_ms(20);								/* wait 20ms     			*/

	SSD1351_RES_CLR();							/* RES=L, CS=L   			*/
	SSD1351_CS_CLR();

#elif  USE_SSD1351_SPI_OLED
	SSD1351_RES_SET();							/* RES=H, CS=H				*/
	SSD1351_CS_SET();
	SSD1351_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(20);								/* wait 20ms     			*/

	SSD1351_RES_CLR();							/* RES=L		   			*/
#endif

	_delay_ms(20);								/* wait 20ms     			*/
	
	SSD1351_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_SSD1351_OLED
/**************************************************************************/
/*! 
    Write OLED Command.
*/
/**************************************************************************/
inline void SSD1351_wr_cmd(uint8_t cmd)
{
	SSD1351_DC_CLR();							/* DC=L		     */
	
	SSD1351_CMD = cmd;							/* D7..D0=cmd    */
	SSD1351_WR();								/* WR=L->H       */
	
	SSD1351_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write OLED Data.
*/
/**************************************************************************/
inline void SSD1351_wr_dat(uint8_t dat)
{
	SSD1351_DATA = dat;							/* D7..D0=dat    */
	SSD1351_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write OLED GRAM.
*/
/**************************************************************************/
inline void SSD1351_wr_gram(uint16_t gram)
{
	SSD1351_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	SSD1351_WR();								/* WR=L->H					*/

	SSD1351_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	SSD1351_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write OLED Block Data.
*/
/**************************************************************************/
inline void SSD1351_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		SSD1351_wr_dat (*p++);
		SSD1351_wr_dat (*p++);
		SSD1351_wr_dat (*p++);
		SSD1351_wr_dat (*p++);
	}
	while (n--) {
		SSD1351_wr_dat (*p++);
	}
#endif

}

#elif USE_SSD1351_SPI_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD1351_wr_cmd(uint8_t cmd)
{
	SSD1351_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD1351_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void SSD1351_wr_dat(uint8_t dat)
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
inline void SSD1351_wr_gram(uint16_t gram)
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
inline void SSD1351_wr_block(uint8_t *p,unsigned int cnt)
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
inline void SSD1351_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SSD1351_wr_cmd(0x15);
	SSD1351_wr_dat(OFS_COL + x);
	SSD1351_wr_dat(OFS_COL + width);

	/* Set RAS Address */
	SSD1351_wr_cmd(0x75);
	SSD1351_wr_dat(OFS_RAW + y);
	SSD1351_wr_dat(OFS_RAW + height); 

	/* Write RAM */
	SSD1351_wr_cmd(0x5C);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1351_clear(void)
{
	volatile uint32_t n;

	SSD1351_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
	    /* 16Bit Colour Access */
		SSD1351_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    OLED Module Initialize.
*/
/**************************************************************************/
void SSD1351_init(void)
{
	Display_IoInit_If();
	
	SSD1351_reset();
	
	/* SSD1351 */
	SSD1351_wr_cmd(0xFD);	/* Unlock */
	SSD1351_wr_dat(0x12);
	
	SSD1351_wr_cmd(0xFD);	/* Unlock */
	SSD1351_wr_dat(0xB1);
	
	SSD1351_wr_cmd(0xAE);	/* Display off */
	
	SSD1351_wr_cmd(0xB3);	/* clock & frequency */
 	SSD1351_wr_dat(0xF1);	/* clock=Divser+2 frequency=6 */

	SSD1351_wr_cmd(0xA2);	/* Set display offset */
	SSD1351_wr_dat(0x00);	/* 80h start */

	SSD1351_wr_cmd(0xA1);	/* Set display start line */
	SSD1351_wr_dat(0x00);	/* 00h start */
	
	SSD1351_wr_cmd(0xA0);	/* Set Re-map / Color Depth */
	SSD1351_wr_dat(0x74);	/* 64K color, COM split, COM remap, 8bitBus, Color remap, 
								Non column address remap, Hotizontal increment */

	SSD1351_wr_cmd(0xB5);	/* Set GPIO */
	SSD1351_wr_dat(0x00);

	SSD1351_wr_cmd(0xAB);	/* Function Selection */
	SSD1351_wr_dat(0x01);

	SSD1351_wr_cmd(0xB4);	/* Set Segment Low Voltage */
 	SSD1351_wr_dat(0xA0);
	SSD1351_wr_dat(0xB5);
	SSD1351_wr_dat(0x55);

	SSD1351_wr_cmd(0xc1);	/* Set contrast current for A B C */
	SSD1351_wr_dat(0xc8);	/* Color A */
	SSD1351_wr_dat(0x80);	/* Color B */ 
	SSD1351_wr_dat(0x8a);	/* Color C */

	SSD1351_wr_cmd(0xc7);	/* Set master contrast */
	SSD1351_wr_cmd(0x0f);	/* no change */

	SSD1351_wr_cmd(0xB8);   /* Gamma Look up Table */
	SSD1351_wr_dat(0x02);
	SSD1351_wr_dat(0x03);
	SSD1351_wr_dat(0x04);
	SSD1351_wr_dat(0x05);
	SSD1351_wr_dat(0x06);
	SSD1351_wr_dat(0x07);
	SSD1351_wr_dat(0x08);
	SSD1351_wr_dat(0x09);
	SSD1351_wr_dat(0x0A);
	SSD1351_wr_dat(0x0B);
	SSD1351_wr_dat(0x0C);
	SSD1351_wr_dat(0x0D);
	SSD1351_wr_dat(0x0E);
	SSD1351_wr_dat(0x0F);
	SSD1351_wr_dat(0x10);
	SSD1351_wr_dat(0x11);
	SSD1351_wr_dat(0x12);
	SSD1351_wr_dat(0x13);
	SSD1351_wr_dat(0x15);
	SSD1351_wr_dat(0x17);
	SSD1351_wr_dat(0x19);
	SSD1351_wr_dat(0x1B);
	SSD1351_wr_dat(0x1D);
	SSD1351_wr_dat(0x1F);
	SSD1351_wr_dat(0x21);
	SSD1351_wr_dat(0x23);
	SSD1351_wr_dat(0x25);
	SSD1351_wr_dat(0x27);
	SSD1351_wr_dat(0x2A);
	SSD1351_wr_dat(0x2D);
	SSD1351_wr_dat(0x30);
	SSD1351_wr_dat(0x33);
	SSD1351_wr_dat(0x36);
	SSD1351_wr_dat(0x39);
	SSD1351_wr_dat(0x3C);
	SSD1351_wr_dat(0x3F);
	SSD1351_wr_dat(0x42);
	SSD1351_wr_dat(0x45);
	SSD1351_wr_dat(0x48);
	SSD1351_wr_dat(0x4C);
	SSD1351_wr_dat(0x50);
	SSD1351_wr_dat(0x54);
	SSD1351_wr_dat(0x58);
	SSD1351_wr_dat(0x5C);
	SSD1351_wr_dat(0x60);
	SSD1351_wr_dat(0x64);
	SSD1351_wr_dat(0x68);
	SSD1351_wr_dat(0x6C);
	SSD1351_wr_dat(0x70);
	SSD1351_wr_dat(0x74);
	SSD1351_wr_dat(0x78);
	SSD1351_wr_dat(0x7D);
	SSD1351_wr_dat(0x82);
	SSD1351_wr_dat(0x87);
	SSD1351_wr_dat(0x8C);
	SSD1351_wr_dat(0x91);
	SSD1351_wr_dat(0x96);
	SSD1351_wr_dat(0x9B);
	SSD1351_wr_dat(0xA0);
	SSD1351_wr_dat(0xA5);
	SSD1351_wr_dat(0xAA);
	SSD1351_wr_dat(0xAF);
	SSD1351_wr_dat(0xB4);

	SSD1351_wr_cmd(0xB1);	/* Set Phase Length */
	SSD1351_wr_dat(0x32);	/* pre=1h dis=1h */

	SSD1351_wr_cmd(0xB2);	/* Enhance Driving Scheme Capability */
	SSD1351_wr_cmd(0xA4);
	SSD1351_wr_cmd(0x00);
	SSD1351_wr_cmd(0x00);

	SSD1351_wr_cmd(0xBB);	/* Set Pre-Charge Voltage */
	SSD1351_wr_dat(0x17);

	SSD1351_wr_cmd(0xB6);	/* Set Second Pre-Charge Period */
 	SSD1351_wr_dat(0x01);

	SSD1351_wr_cmd(0xBE);	/* Set VCOMH Voltage */
	SSD1351_wr_dat(0x05);

	SSD1351_wr_cmd(0xA6);	/* Normal display */

	SSD1351_clear();

	SSD1351_wr_cmd(0xAF);	/* Display on */
	_delay_ms(10);

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1351_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1351_wr_gram(COL_RED);
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
int SSD1351_draw_bmp(const uint8_t* ptr){

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
	SSD1351_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SSD1351_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SSD1351_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
