/********************************************************************************/
/*!
	@file			seps525.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG-6028GDEAF01 OLED module(8bit,spi mode).

    @section HISTORY
		2011.04.01	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "seps525.h"
/* check header file version for fool proof */
#if __SEPS525_H != 0x0200
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
inline void SEPS525_reset(void)
{
#ifdef USE_SEPS525_OLED
	SEPS525_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	SEPS525_RD_SET();
	SEPS525_WR_SET();
	SEPS525_CS_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	SEPS525_RES_CLR();							/* RES=L, CS=L   			*/
	SEPS525_CS_CLR();

#elif  USE_SEPS525_SPI_OLED
	SEPS525_RES_SET();							/* RES=H, CS=H				*/
	SEPS525_CS_SET();
	SEPS525_SCLK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	SEPS525_RES_CLR();							/* RES=L, CS=L   			*/
	SEPS525_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     			*/
	SEPS525_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50 ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_SEPS525_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SEPS525_wr_cmd(uint8_t cmd)
{
	SEPS525_DC_CLR();							/* DC=L		     */
	
	SEPS525_CMD = cmd;							/* D7..D0=cmd    */
	SEPS525_WR();								/* WR=L->H       */
	
	SEPS525_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void SEPS525_wr_dat(uint8_t dat)
{
	SEPS525_DATA = dat;							/* D7..D0=dat    */
	SEPS525_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void SEPS525_wr_gram(uint16_t gram)
{
	SEPS525_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	SEPS525_WR();								/* WR=L->H					*/

	SEPS525_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	SEPS525_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SEPS525_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		SEPS525_wr_dat(*p++);
		SEPS525_wr_dat(*p++);
		SEPS525_wr_dat(*p++);
		SEPS525_wr_dat(*p++);
	}
	while (n--) {
		SEPS525_wr_dat(*p++);
	}
#endif

}

#else /* USE_SEPS525_SPI_OLED */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SEPS525_wr_cmd(uint8_t cmd)
{
	SEPS525_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SEPS525_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void SEPS525_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void SEPS525_wr_gram(uint16_t gram)
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
inline void SEPS525_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
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
inline void SEPS525_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SEPS525_wr_cmd(0x17);
	SEPS525_wr_dat(OFS_COL + x);
	SEPS525_wr_cmd(0x18);
	SEPS525_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	SEPS525_wr_cmd(0x19);
	SEPS525_wr_dat(OFS_RAW + y);
	SEPS525_wr_cmd(0x1A);
	SEPS525_wr_dat(OFS_RAW + height); 
	
	/* Set Column DDRAM Address */
	SEPS525_wr_cmd(0x20);
	SEPS525_wr_dat(OFS_COL + x);

	/* Set Row DDRAM Address */
	SEPS525_wr_cmd(0x21);
	SEPS525_wr_dat(OFS_RAW + y);

	/* Write RAM */
	SEPS525_wr_cmd(0x22);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SEPS525_clear(void)
{
	volatile uint32_t n;

	SEPS525_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		SEPS525_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SEPS525_init(void)
{
	Display_IoInit_If();

	SEPS525_reset();

	/* Initialize SEPS525 */
	SEPS525_wr_cmd(0x04); 
	SEPS525_wr_dat(0x01);
	_delay_ms(1);
	SEPS525_wr_cmd(0x04); 
	SEPS525_wr_dat(0x00);
	_delay_ms(1);


	SEPS525_wr_cmd(0x02); 
	SEPS525_wr_dat(0x41);		/* Set OSC_CTL (internal OSC on) need to check with 0x01 changed from 0x41 to 0x01 */

	SEPS525_wr_cmd(0x03); 
	SEPS525_wr_dat(0x30);		/* Set CLOCK_DIV (OSC clock 1)  changed 0x30 to 0x33 for osc clock 1/3 */

	SEPS525_wr_cmd(0x08); 
	SEPS525_wr_dat(0x01);		/* Set PRECHARGE_TIME_R (1 CLK) */
	SEPS525_wr_cmd(0x09); 
	SEPS525_wr_dat(0x01);		/* Set PRECHARGE_TIME_G (1 CLK) */
	SEPS525_wr_cmd(0x0A); 
	SEPS525_wr_dat(0x01);		/* Set PRECHARGE_TIME_B (1 CLK) */

	SEPS525_wr_cmd(0x0B); 
	SEPS525_wr_dat(0x0A);		/* Set PRECHARGE_CURRENT_R */
	SEPS525_wr_cmd(0x0C); 
	SEPS525_wr_dat(0x0A);		/* Set PRECHARGE_CURRENT_G */
	SEPS525_wr_cmd(0x0D); 
	SEPS525_wr_dat(0x0A);		/* Set PRECHARGE_CURRENT_B */

	SEPS525_wr_cmd(0x10); 
	SEPS525_wr_dat(0x46);		/*Set DRIVING_CURRENT_R */
	SEPS525_wr_cmd(0x11); 
	SEPS525_wr_dat(0x38);		/*Set DRIVING_CURRENT_G */
	SEPS525_wr_cmd(0x12); 
	SEPS525_wr_dat(0x3A);		/*Set DRIVING_CURRENT_B */

	SEPS525_wr_cmd(0x13); 
	SEPS525_wr_dat(0x00);		/*Set DISPLAY_MODE_SET  */

	SEPS525_wr_cmd(0x14);
    SEPS525_wr_dat(0x21);		/*Set RGB_IF (MPU mode) */

	SEPS525_wr_cmd(0x15); 
	SEPS525_wr_dat(0x00);		/*Set RGB_POL */

	SEPS525_wr_cmd(0x16); 
	SEPS525_wr_dat((1<<6)|(1<<5)|(1<<2)|(1<<1)); /* Set 65k,Double */
	
	SEPS525_wr_cmd(0x28); 
	SEPS525_wr_dat(0x7F);		/*Set DUTY */

	SEPS525_wr_cmd(0x29); 
	SEPS525_wr_dat(0x00);		/*Set DSL */


	SEPS525_wr_cmd(0x80); 
	SEPS525_wr_dat(0x00);		/*Set IREF (controlled by external resistor) */

	SEPS525_wr_cmd(0x3B); 
	SEPS525_wr_dat(0x0D);		/*Screen Saver ON change this to 0D */

	SEPS525_wr_cmd(0x3C); 
	SEPS525_wr_dat(0x05);		/*screen saver sleeps after 10*64 frames */

	SEPS525_wr_cmd(0x3D); 
	SEPS525_wr_dat(0x00);		/* Screen save mode */


	SEPS525_wr_cmd(0x06); 
	SEPS525_wr_dat(0x01);
	
	SEPS525_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SEPS525_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		SEPS525_wr_gram(COL_RED);
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
int SEPS525_draw_bmp(const uint8_t* ptr){

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
	SEPS525_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SEPS525_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SEPS525_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
