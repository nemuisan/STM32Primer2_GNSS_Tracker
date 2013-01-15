/********************************************************************************/
/*!
	@file			r61526.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.04.20
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KD024B-3-TP 				(R61526)	8/16bit mode.		@n
					 -Nokia6300-LCD				(MC2PA8201)	8bit mode only.

    @section HISTORY
		2012.04.20	V1.00	First Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "r61526.h"
/* check header file version for fool proof */
#if __R61526_H != 0x0100
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
inline void R61526_reset(void)
{
	R61526_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	R61526_RD_SET();
	R61526_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	R61526_RES_CLR();							/* RES=L, CS=L   			*/
	R61526_CS_CLR();

	_delay_ms(10);								/* wait 10ms     			*/
	R61526_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void R61526_wr_cmd(uint8_t cmd)
{
	R61526_DC_CLR();							/* DC=L						*/

	R61526_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	R61526_WR();								/* WR=L->H					*/

	R61526_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void R61526_wr_dat(uint8_t dat)
{
	R61526_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	R61526_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void R61526_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	R61526_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	R61526_WR();								/* WR=L->H					*/
#endif

	R61526_DATA = gram;							/* data(8bit_Low or 16bit)	*/
	R61526_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void R61526_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		R61526_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		R61526_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t R61526_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	R61526_wr_cmd(cmd);
	R61526_WR_SET();

    ReadLCDData(temp);		/* through */
    ReadLCDData(temp);		/* through */
    ReadLCDData(temp);		/* through */
    ReadLCDData(temp);
    ReadLCDData(val);

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void R61526_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	R61526_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	R61526_wr_dat((OFS_COL + x)>>8);
	R61526_wr_dat(OFS_COL + x);
	R61526_wr_dat((OFS_COL + width)>>8);
	R61526_wr_dat(OFS_COL + width);

	R61526_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	R61526_wr_dat((OFS_RAW + y)>>8);
	R61526_wr_dat(OFS_RAW + y);
	R61526_wr_dat((OFS_RAW + height)>>8);
	R61526_wr_dat(OFS_RAW + height);

	R61526_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void R61526_clear(void)
{
	volatile uint32_t n;

	R61526_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61526_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void R61526_init(void)
{
	uint16_t devicetype,nokia6300;
	
	Display_IoInit_If();

	R61526_reset();

	/* Check Device Code */
	R61526_wr_cmd(0xB0);					/* Enable Manufacturer Commands */
	R61526_wr_dat(0x3F);					/* Nemuisan Inserted */
	R61526_wr_dat(0x3F);
	_delay_ms(5);
	
	devicetype = R61526_rd_cmd(0xBF);  		/* Confirm Vaild LCD Controller */
	nokia6300  = R61526_rd_cmd(0x04);  		/* Confirm Nokia6300 LCD Controller */

	if(devicetype == 0x1526)
	{
		/* Initialize R61526 */
		R61526_wr_cmd(0xB0);
		R61526_wr_dat(0x3F);
		R61526_wr_dat(0x3F);
		_delay_ms(5);
		
		R61526_wr_cmd(0xFE);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x21);
		R61526_wr_dat(0xB4);

		R61526_wr_cmd(0xB3);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x10);

		R61526_wr_cmd(0xE0);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x40);
		_delay_ms(5);

		R61526_wr_cmd(0xB3);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0xFE);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x21);
		R61526_wr_dat(0x30);

		R61526_wr_cmd(0xB0);
		R61526_wr_dat(0x3F);
		R61526_wr_dat(0x3F);

		R61526_wr_cmd(0xB3);
		R61526_wr_dat(0x02);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0xB4);
		R61526_wr_dat(0x00);    

		R61526_wr_cmd(0xC0);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x4F);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x10);
		R61526_wr_dat(0xA2);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x01);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0xC1);
		R61526_wr_dat(0x01);
		R61526_wr_dat(0x01);
		R61526_wr_dat(0x19);
		R61526_wr_dat(0x08);
		R61526_wr_dat(0x08);
		_delay_ms(25);

		R61526_wr_cmd(0xC3);
		R61526_wr_dat(0x01);  
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x28);
		R61526_wr_dat(0x08);
		R61526_wr_dat(0x08);
		_delay_ms(5);

		R61526_wr_cmd(0xC4);
		R61526_wr_dat(0x11);
		R61526_wr_dat(0x01);  
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0xC8);
		R61526_wr_dat(0x0C);
		R61526_wr_dat(0x0C);
		R61526_wr_dat(0x0D);
		R61526_wr_dat(0x14);
		R61526_wr_dat(0x18);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x09);
		R61526_wr_dat(0x09);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x05);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x08);
		R61526_wr_dat(0x07);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x15);
		R61526_wr_dat(0x12);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0xC9);
		R61526_wr_dat(0x0C);
		R61526_wr_dat(0x0C);
		R61526_wr_dat(0x0D);
		R61526_wr_dat(0x14);
		R61526_wr_dat(0x18);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x09);
		R61526_wr_dat(0x09);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x05);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x08);
		R61526_wr_dat(0x07);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x15);
		R61526_wr_dat(0x12);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0xCA);
		R61526_wr_dat(0x0C);
		R61526_wr_dat(0x0C);
		R61526_wr_dat(0x0D);
		R61526_wr_dat(0x14);
		R61526_wr_dat(0x18);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x09);
		R61526_wr_dat(0x09);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x05);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x08);
		R61526_wr_dat(0x07);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x15);
		R61526_wr_dat(0x12);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x0E);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x0A);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0xD0);
		R61526_wr_dat(0x63);
		R61526_wr_dat(0x53);
		R61526_wr_dat(0x82);
		R61526_wr_dat(0x3F);

		R61526_wr_cmd(0xD1);
		R61526_wr_dat(0x6A);
		R61526_wr_dat(0x64);

		R61526_wr_cmd(0xD2);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x24);

		R61526_wr_cmd(0xD4);
		R61526_wr_dat(0x03);
		R61526_wr_dat(0x24);

		R61526_wr_cmd(0xE2);
		R61526_wr_dat(0x3F);

		R61526_wr_cmd(0x35);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0x36);
		R61526_wr_dat(0x00);

		R61526_wr_cmd(0x3A);
		R61526_wr_dat(0x55);			/* 66=18BIT;55=16BIT */

		R61526_wr_cmd(0x2A);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0xEF);
		R61526_wr_cmd(0x2B);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x00);
		R61526_wr_dat(0x01);
		R61526_wr_dat(0x3f);
		R61526_wr_cmd(0x11);
		_delay_ms(40);
		R61526_wr_cmd(0x29);
		R61526_wr_cmd(0x2c);
	}
	
	else if(nokia6300 == 0x2700)
	{
		/* Initialize nokia6300 */
		R61526_wr_cmd(0x36);				/* MADCTRL */
		R61526_wr_dat((1<<7)|(1<<6));

		R61526_wr_cmd(0x3A);				/* RGB bits set */
		R61526_wr_dat(0x55);				/* 66=18BIT;55=16BIT */

		R61526_wr_cmd(0xB1);				/* Reflesh rate set to 60Hz(nemuisan said so) */
		R61526_wr_dat(0x17);
		R61526_wr_dat(0x10);
		R61526_wr_dat(0x06);
		R61526_wr_dat(0x1C);
		R61526_wr_dat(0x10);
		R61526_wr_dat(0x06);

		R61526_wr_cmd(0x11);
		_delay_ms(40);
		R61526_wr_cmd(0x29);
	}

	else { for(;;);} /* Invalid Device Code!! */

	R61526_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	R61526_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61526_wr_gram(COL_RED);
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
int R61526_draw_bmp(const uint8_t* ptr){

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
	R61526_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		R61526_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
		#if 0
			/* 262k colour access */
			d2 = *p << 14;	  		/* Blue  */
			d  = (*p++ >> 4);			/* Blue  */
			d |= ((*p++>>2)<<4);		/* Green */
			d |= ((*p++>>2)<<10);		/* Red   */
			/* 262k +16M dither colour access */
			d2 = *p++ << 8;			/* Blue  */
			d  = *p++;				/* Green */
			d |= *p++ << 8;			/* Red   */
		#endif
			R61526_wr_gram(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
