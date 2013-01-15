/********************************************************************************/
/*!
	@file			st7787.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.08.27
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive S95417	TFT module.

    @section HISTORY
		2012.08.27	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "st7787.h"
/* check header file version for fool proof */
#if __ST7787_H != 0x0100
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
inline void ST7787_reset(void)
{
	ST7787_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ST7787_RD_SET();
	ST7787_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	ST7787_RES_CLR();							/* RES=L, CS=L   			*/
	ST7787_CS_CLR();

	_delay_ms(10);								/* wait 10ms     			*/
	ST7787_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7787_wr_cmd(uint8_t cmd)
{
	ST7787_DC_CLR();							/* DC=L						*/

	ST7787_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	ST7787_WR();								/* WR=L->H					*/

	ST7787_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7787_wr_dat(uint8_t dat)
{
	ST7787_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	ST7787_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7787_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ST7787_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	ST7787_WR();								/* WR=L->H					*/
#endif

	ST7787_DATA = gram;							/* data(8bit_Low or 16bit)	*/
	ST7787_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7787_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ST7787_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ST7787_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ST7787_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	ST7787_wr_cmd(cmd);
	ST7787_WR_SET();

    ReadLCDData(temp);							/* Dummy Read(Invalid Data) */
    ReadLCDData(temp);							/* Dummy Read				*/
    ReadLCDData(temp);							/* Upper Read				*/
    ReadLCDData(val);							/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ST7787_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ST7787_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ST7787_wr_dat((OFS_COL + x)>>8);
	ST7787_wr_dat(OFS_COL + x);
	ST7787_wr_dat((OFS_COL + width)>>8);
	ST7787_wr_dat(OFS_COL + width);

	ST7787_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	ST7787_wr_dat((OFS_RAW + y)>>8);
	ST7787_wr_dat(OFS_RAW + y);
	ST7787_wr_dat((OFS_RAW + height)>>8);
	ST7787_wr_dat(OFS_RAW + height);

	ST7787_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ST7787_clear(void)
{
	volatile uint32_t n;

	ST7787_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ST7787_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ST7787_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	ST7787_reset();

	/* Check Device Code */
	/* ST7787 doesn't have DeviceID */
	/* But we can read "Display Read Status(09h)" as "0x6100" */
	devicetype = ST7787_rd_cmd(0x09);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x6100)
	{
		/* Initialize ST7787 */
		ST7787_wr_cmd(0x11);   
		_delay_ms(125);   

		ST7787_wr_cmd(0xB1);   
		ST7787_wr_dat(0x3C); 
		ST7787_wr_dat(0x02);   
		ST7787_wr_dat(0x02);   

		ST7787_wr_cmd(0xBC);   

		ST7787_wr_cmd(0xC2);
		ST7787_wr_dat(0x04);
		ST7787_wr_dat(0xE6);
		ST7787_wr_dat(0x86);
		ST7787_wr_dat(0x33);
		ST7787_wr_dat(0x03);   

		ST7787_wr_cmd(0xC3);   
		ST7787_wr_dat(0x03);   
		ST7787_wr_dat(0x33);   
		ST7787_wr_dat(0x03);   
		ST7787_wr_dat(0x00);   
		ST7787_wr_dat(0x00);   

		ST7787_wr_cmd(0xF4);   
		ST7787_wr_dat(0xFF);   
		ST7787_wr_dat(0x3F);   

		ST7787_wr_cmd(0xF5);   
		ST7787_wr_dat(0x10);   

		ST7787_wr_cmd(0xFB);   
		ST7787_wr_dat(0x7F);   

		ST7787_wr_cmd(0xC5);   
		ST7787_wr_dat(0xC9);
		ST7787_wr_dat(0x1A); 

		ST7787_wr_cmd(0xC6);   
		ST7787_wr_dat(0x24);
		ST7787_wr_dat(0x00);

		ST7787_wr_cmd(0xC0);   
		ST7787_wr_dat(0x00);   

		ST7787_wr_cmd(0xB6);   
		ST7787_wr_dat(0x02);   
		ST7787_wr_dat(0x04);        

		ST7787_wr_cmd(0x36);   
		ST7787_wr_dat((0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2));   

		ST7787_wr_cmd(0x3A);   
		ST7787_wr_dat(0x05); 

		ST7787_wr_cmd(0x29);

	}

	else { for(;;);} /* Invalid Device Code!! */

	ST7787_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ST7787_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ST7787_wr_gram(COL_RED);
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
int ST7787_draw_bmp(const uint8_t* ptr){

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
	ST7787_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		ST7787_rect(xs, xe, m, m);
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
			ST7787_wr_gram(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
