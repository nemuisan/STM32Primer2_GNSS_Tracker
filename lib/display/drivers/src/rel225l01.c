/********************************************************************************/
/*!
	@file			rel225l01.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.12.23
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					and Based on y_notsu's QVGA-TFT demo thanks!				@n
					http://mbed.org/users/y_notsu/programs/QVGA_TFT_test/lldy66 @n
						Available TFT-LCM are listed below.						@n
					 -REL225L01						(REL225L01)	8/16bit mode.	@n
					 -FPC192GC00					(REL225L01) 8/16bit mode.   @n

    @section HISTORY
		2011.09.30	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "rel225l01.h"
/* check header file version for fool proof */
#if __REL225L01_H != 0x0300
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Abstract Layer _delay_ms Settings.
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
inline void REL225L01_reset(void)
{
	REL225L01_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	REL225L01_RD_SET();
	REL225L01_WR_SET();
	_delay_ms(20);								/* wait 20ms     			*/

	REL225L01_RES_CLR();						/* RES=L, CS=L   			*/
	REL225L01_CS_CLR();
	_delay_ms(50);								/* wait 50ms     			*/
	
	REL225L01_RES_SET();						/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void REL225L01_wr_cmd(uint8_t cmd)
{
	REL225L01_DC_CLR();							/* DC=L						*/

	REL225L01_CMD = cmd;						/* cmd(8bit_Low or 16bit)	*/
	REL225L01_WR();								/* WR=L->H					*/

	REL225L01_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void REL225L01_wr_dat(uint8_t dat)
{
	REL225L01_DATA = dat;						/* data						*/
	REL225L01_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void REL225L01_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	REL225L01_DATA = (uint8_t)(gram>>8);		/* upper 8bit data			*/
	REL225L01_WR();								/* WR=L->H					*/
	REL225L01_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	REL225L01_DATA = gram;						/* 16bit data 				*/
#endif
	REL225L01_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void REL225L01_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		REL225L01_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		REL225L01_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void REL225L01_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	REL225L01_wr_cmd(0x2A);					/* Horizontal RAM Start ADDR */
	REL225L01_wr_dat((OFS_COL + x)>>8);
	REL225L01_wr_dat(OFS_COL + x);
	REL225L01_wr_dat((OFS_COL + width)>>8);
	REL225L01_wr_dat(OFS_COL + width);

	REL225L01_wr_cmd(0x2B);					/* Horizontal RAM Start ADDR */
	REL225L01_wr_dat((OFS_RAW + y)>>8);
	REL225L01_wr_dat(OFS_RAW + y);
	REL225L01_wr_dat((OFS_RAW + height)>>8);
	REL225L01_wr_dat(OFS_RAW + height);

	REL225L01_wr_cmd(0x2C);					/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void REL225L01_clear(void)
{
	volatile uint32_t n;

	REL225L01_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		REL225L01_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t REL225L01_rd_cmd(uint8_t cmd)
{
#if 0
	uint16_t temp,i;
	uint16_t val;


	REL225L01_wr_cmd(cmd);
	REL225L01_WR_SET();

	for(i=0;i<4;i++){
		ReadLCDData(temp);
	}

    ReadLCDData(val);
	val &= 0x00FF;
	val |= temp<<8;
#endif
	return 0xFFFF;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void REL225L01_init(void)
{
	volatile int i;

	Display_IoInit_If();

	REL225L01_reset();

	/* Initialize REL225L01 */
    REL225L01_wr_cmd(0xB0);					/* ??? */
    REL225L01_wr_dat(0xCA);
    REL225L01_wr_dat(0x90);
    REL225L01_wr_dat(0x37);
    REL225L01_wr_dat(0x01);
    REL225L01_wr_dat(0x48);
    REL225L01_wr_dat(0x01);
    REL225L01_wr_dat(0x40);
    REL225L01_wr_dat(0x04);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);

    REL225L01_wr_cmd(0x26);					/* ??? */
    REL225L01_wr_dat(0x01);
    
    REL225L01_wr_cmd(0xB1);					/* ??? */
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x08);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x04);
    REL225L01_wr_dat(0x10);
    REL225L01_wr_dat(0x48);
    REL225L01_wr_dat(0x82);
    REL225L01_wr_dat(0x20);
    REL225L01_wr_dat(0x84);
    REL225L01_wr_dat(0x92);
    REL225L01_wr_dat(0x2A);
    REL225L01_wr_dat(0x52);
    REL225L01_wr_dat(0xAA);
    REL225L01_wr_dat(0x5A);
    REL225L01_wr_dat(0x55);
    REL225L01_wr_dat(0x2A);
    REL225L01_wr_dat(0x55);
    REL225L01_wr_dat(0xAE);
    REL225L01_wr_dat(0xFF);
    REL225L01_wr_dat(0xFE);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x04);

    REL225L01_wr_cmd(0xB2);					/* ??? */
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x04);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x02);
    REL225L01_wr_dat(0x08);
    REL225L01_wr_dat(0x48);
    REL225L01_wr_dat(0x82);
    REL225L01_wr_dat(0x20);
    REL225L01_wr_dat(0x84);
    REL225L01_wr_dat(0x42);
    REL225L01_wr_dat(0x48);
    REL225L01_wr_dat(0x92);
    REL225L01_wr_dat(0x2A);
    REL225L01_wr_dat(0x52);
    REL225L01_wr_dat(0xAA);
    REL225L01_wr_dat(0x5A);
    REL225L01_wr_dat(0x55);
    REL225L01_wr_dat(0x2A);
    REL225L01_wr_dat(0x55);
    REL225L01_wr_dat(0xAE);
    REL225L01_wr_dat(0xFF);
    REL225L01_wr_dat(0xFE);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x00);
    REL225L01_wr_dat(0x04);


    REL225L01_wr_cmd(0x36);				    /* Entry Mode */
	REL225L01_wr_dat((1<<6)|(0<<5)|(1<<3)|(1<<0));

    REL225L01_wr_cmd(0x3A);					/* Set Colour */
    REL225L01_wr_dat(0x05);				/* Colour 65k */

    REL225L01_wr_cmd(0xC2);					/* databus width */
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	REL225L01_wr_dat(0x00);				/* mode 8-bit */
#else
	REL225L01_wr_dat(0x03);				/* mode 16-bit */
#endif

    REL225L01_wr_cmd(0xBA);
	for(i=0;i<47;i++)
	{ 
		REL225L01_wr_dat(0x55);
	}

    REL225L01_wr_cmd(0x11);
	_delay_ms(1);
	REL225L01_clear();
	_delay_ms(80);
    REL225L01_wr_cmd(0x29);
	REL225L01_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	REL225L01_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		REL225L01_wr_gram(COL_RED);
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
int REL225L01_draw_bmp(const uint8_t* ptr){

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
	REL225L01_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		REL225L01_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			REL225L01_wr_gram(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
