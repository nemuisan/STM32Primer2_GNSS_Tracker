/********************************************************************************/
/*!
	@file			spfd54126.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.09.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive MF_LCM2.0_V3 TFT module(8/16bit).

    @section HISTORY
		2012.09.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "spfd54126.h"
/* check header file version for fool proof */
#if __SPFD54126_H != 0x0100
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
inline void SPFD54126_reset(void)
{
	SPFD54126_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	SPFD54126_RD_SET();
	SPFD54126_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	SPFD54126_RES_CLR();						/* RES=L, CS=L   			*/
	SPFD54126_CS_CLR();

	_delay_ms(20);								/* wait 20ms     			*/
	SPFD54126_RES_SET();						/* RES=H					*/
	_delay_ms(150);				    			/* wait over 120ms     		*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SPFD54126_wr_cmd(uint8_t cmd)
{
	SPFD54126_DC_CLR();							/* DC=L		     */
	
	SPFD54126_CMD = cmd;						/* D7..D0=cmd    */
	SPFD54126_WR();								/* WR=L->H       */
	
	SPFD54126_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void SPFD54126_wr_dat(uint8_t dat)
{
	SPFD54126_DATA = dat;						/* D7..D0=dat    */
	SPFD54126_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void SPFD54126_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SPFD54126_DATA = (uint8_t)(gram>>8);		/* upper 8bit data			*/
	SPFD54126_WR();								/* WR=L->H					*/
#endif

	SPFD54126_DATA = gram;						/* data(8bit_Low or 16bit)	*/
	SPFD54126_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SPFD54126_rd_cmd(uint8_t cmd)
{
	uint16_t val,temp;

	SPFD54126_wr_cmd(cmd);
	SPFD54126_WR_SET();

    ReadLCDData(temp);				/* Dummy Read				*/
    ReadLCDData(temp);				/* Upper Read				*/
    ReadLCDData(val);				/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SPFD54126_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SPFD54126_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		SPFD54126_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SPFD54126_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SPFD54126_wr_cmd(CASET); 
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_COL + x);
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	SPFD54126_wr_cmd(RASET);
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_RAW + y); 
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	SPFD54126_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SPFD54126_clear(void)
{
	volatile uint32_t n;

	SPFD54126_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		SPFD54126_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SPFD54126_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	SPFD54126_reset();

	/* Check Device Code */
	devicetype = SPFD54126_rd_cmd(RDID4);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0616)
	{
		/* Initialize SPFD54126 */
		SPFD54126_wr_cmd(SLPOUT);

		SPFD54126_wr_cmd(VMCTR1);
		SPFD54126_wr_dat(0xc5);

		SPFD54126_wr_cmd(0xf0);
		SPFD54126_wr_dat(0x5a);

		SPFD54126_wr_cmd(0xf2);
		SPFD54126_wr_dat(0x01);

		SPFD54126_wr_cmd(GAMCTRP1);
		SPFD54126_wr_dat(0x00);
		SPFD54126_wr_dat(0x01);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x2e);
		SPFD54126_wr_dat(0x2b);
		SPFD54126_wr_dat(0x0b);
		SPFD54126_wr_dat(0x1a);
		SPFD54126_wr_dat(0x02);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x0c);
		SPFD54126_wr_dat(0x0d);
		SPFD54126_wr_dat(0x00);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x02);
		SPFD54126_wr_dat(0x05);

		SPFD54126_wr_cmd(GAMCTRN1);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x23);
		SPFD54126_wr_dat(0x25);
		SPFD54126_wr_dat(0x0f);
		SPFD54126_wr_dat(0x0a);
		SPFD54126_wr_dat(0x04);
		SPFD54126_wr_dat(0x02);
		SPFD54126_wr_dat(0x1a);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x03);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x01);
		SPFD54126_wr_dat(0x0c);
		SPFD54126_wr_dat(0x0b);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x05);

		SPFD54126_wr_cmd(COLMOD);
		SPFD54126_wr_dat(0x65);

		SPFD54126_wr_cmd(DISPON);

	}

	else { for(;;);} /* Invalid Device Code!! */

	SPFD54126_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SPFD54126_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		SPFD54126_wr_gram(COL_RED);
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
int SPFD54126_draw_bmp(const uint8_t* ptr){

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
	SPFD54126_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SPFD54126_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SPFD54126_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
