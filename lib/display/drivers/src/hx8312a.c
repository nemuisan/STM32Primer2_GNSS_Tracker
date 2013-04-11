/********************************************************************************/
/*!
	@file			hx8312a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive WSTTN03-S220A TFT module(16bit mode only).

    @section HISTORY
		2010.12.31	V1.00	Stable Release.
		2011.03.10	V2.00	C++ Ready.
		2011.10.25	V3.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8312a.h"
/* check header file version for fool proof */
#if __HX8312A_H != 0x0300
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
inline void HX8312A_reset(void)
{
	HX8312A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8312A_RD_SET();
	HX8312A_WR_SET();
	_delay_ms(5);								/* wait 5ms     			*/

	HX8312A_RES_CLR();							/* RES=L, CS=L   			*/
	HX8312A_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	HX8312A_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait at least 10ms     	*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8312A_wr_cmd(uint8_t cmd,uint8_t cmdata)
{
	HX8312A_DC_CLR();								/* DC=L						*/
	
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8312A_CMD = (uint8_t)(cmd);					/* upper 8bit data			*/
	HX8312A_WR();									/* WR=L->H					*/
	HX8312A_CMD = (uint8_t)cmdata;					/* lower 8bit data			*/
#else
	HX8312A_CMD = (uint16_t)((cmd<<8)|(cmdata));	/* cmd(8bit_Low or 16bit)	*/
#endif
	HX8312A_WR();									/* WR=L->H					*/

	HX8312A_DC_SET();								/* DC=H 					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8312A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8312A_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	HX8312A_WR();								/* WR=L->H					*/
	HX8312A_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	HX8312A_DATA = dat;							/* 16bit data 				*/
#endif
	HX8312A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8312A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8312A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HX8312A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8312A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8312A_wr_cmd(0x45,(OFS_COL + x));
	HX8312A_wr_cmd(0x46,(OFS_COL + width));
	HX8312A_wr_cmd(0x47,((OFS_RAW + y)>>8));
	HX8312A_wr_cmd(0x48,(OFS_RAW + y));
	HX8312A_wr_cmd(0x49,((OFS_RAW + height)>>8));
	HX8312A_wr_cmd(0x4A,(OFS_RAW + height));

	HX8312A_wr_cmd(0x42,(OFS_COL + x));
	HX8312A_wr_cmd(0x43,((OFS_RAW + y)>>8));
	HX8312A_wr_cmd(0x44,(OFS_RAW + y));

	HX8312A_wr_cmd(0x00,0x20);   

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8312A_clear(void)
{
	volatile uint32_t n;

	HX8312A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8312A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX8312A_rd_cmd(uint8_t cmd,uint8_t cmdata)
{
	uint16_t val;

	HX8312A_DC_CLR();								/* DC=L						*/
	
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8312A_CMD = (uint8_t)(cmd);					/* upper 8bit data			*/
	HX8312A_WR();									/* WR=L->H					*/
	HX8312A_CMD = (uint8_t)cmdata;					/* lower 8bit data			*/
#else
	HX8312A_CMD = (uint16_t)((cmd<<8)|(cmdata));	/* cmd(8bit_Low or 16bit)	*/
#endif
	HX8312A_WR();									/* WR=L->H					*/

    ReadLCDData(val);

	HX8312A_DC_SET();								/* DC=H 					*/

	val &= 0x00FF;
	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8312A_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HX8312A_reset();

	/* Check Device Code */
	devicetype = HX8312A_rd_cmd(0x32,0x00);
	
	if(devicetype == 0x0C)
	{
		/* Initialize HX8312A */
		/* reset device */
		HX8312A_wr_cmd(0x01,0x01);	/* start osc */   
		HX8312A_wr_cmd(0x00,0x00);	/* cancel the standby mode */
		HX8312A_wr_cmd(0x03,0x01);	/* soft reset */
		_delay_ms(20);   
		HX8312A_wr_cmd(0x03,0x00);	/* normal operation */  
		HX8312A_wr_cmd(0x2b,0x0e);	/* internal oscillator frequency */
		HX8312A_wr_cmd(0x28,0x0b);   
		HX8312A_wr_cmd(0x1a,0x05);   
		HX8312A_wr_cmd(0x25,0x00);   
		HX8312A_wr_cmd(0x1c,0x73);   
		HX8312A_wr_cmd(0x19,0x0c);   
		
		/* set power */
		HX8312A_wr_cmd(0x18,0xc1);   
		_delay_ms(20);   
		HX8312A_wr_cmd(0x1e,0x01);   
		HX8312A_wr_cmd(0x18,0xc5);   
		HX8312A_wr_cmd(0x18,0xe5);   
		_delay_ms(40);   
		_delay_ms(40);   
		HX8312A_wr_cmd(0x18,0xf5);   
		_delay_ms(80);   
		HX8312A_wr_cmd(0x1b,0x09);   
		_delay_ms(10);   
		HX8312A_wr_cmd(0x20,0x17);   
		HX8312A_wr_cmd(0x1f,0x16);   
		HX8312A_wr_cmd(0x1e,0x81);   
		_delay_ms(10);   
		HX8312A_wr_cmd(0x9d,0x00);
		HX8312A_wr_cmd(0x05,(1<<4)|(0<<2));
		HX8312A_wr_cmd(0x06,(0<<0));
		HX8312A_wr_cmd(0xc0,0x00);   
		HX8312A_wr_cmd(0xc1,0x00);  
		HX8312A_wr_cmd(0x0d,0x00);  
	 
		HX8312A_wr_cmd(0x0e,0x00);   
		HX8312A_wr_cmd(0x0f,0x00);   
		HX8312A_wr_cmd(0x10,0x00);   
		HX8312A_wr_cmd(0x11,0x00);   
		HX8312A_wr_cmd(0x12,0x00);   
		HX8312A_wr_cmd(0x13,0x00);   
		HX8312A_wr_cmd(0x14,0x00);   
		HX8312A_wr_cmd(0x15,0x00);   
		HX8312A_wr_cmd(0x16,0x00);   
		HX8312A_wr_cmd(0x17,0x00);   
		HX8312A_wr_cmd(0x34,0x01);   

		/* set ggram */  
		HX8312A_wr_cmd(0x35,0x00);   
		HX8312A_wr_cmd(0x3c,0x00);   
		HX8312A_wr_cmd(0x3d,0x00);   
		HX8312A_wr_cmd(0x3e,0x01);   
		HX8312A_wr_cmd(0x3f,0x3f);   
		HX8312A_wr_cmd(0x40,0x02);   
		HX8312A_wr_cmd(0x41,0x02);   
		HX8312A_wr_cmd(0x42,0x00);   
		HX8312A_wr_cmd(0x43,0x00);   
		HX8312A_wr_cmd(0x44,0x00);   
		HX8312A_wr_cmd(0x45,0x00);   

		/* set rectangle */ 
		/*
		HX8312A_wr_cmd(0x46,0xef);   
		HX8312A_wr_cmd(0x47,0x00);   
		HX8312A_wr_cmd(0x48,0x00);   
		HX8312A_wr_cmd(0x49,0x01);   
		HX8312A_wr_cmd(0x4a,0x3f);
		*/
		HX8312A_wr_cmd(0x1d,0x08);   
		HX8312A_wr_cmd(0x86,0x00);   
		HX8312A_wr_cmd(0x87,0x2f);   
		HX8312A_wr_cmd(0x88,0x02);   
		HX8312A_wr_cmd(0x89,0x05);   
		HX8312A_wr_cmd(0x8d,0x01);   
		HX8312A_wr_cmd(0x8b,0x36);   
		HX8312A_wr_cmd(0x33,0x01);   
		HX8312A_wr_cmd(0x37,0x01);   
		HX8312A_wr_cmd(0x76,0x00);   

		/* gamma set */ 
		HX8312A_wr_cmd(0x8f,0x00);   
		HX8312A_wr_cmd(0x90,0x77);   
		HX8312A_wr_cmd(0x91,0x07);   
		HX8312A_wr_cmd(0x92,0x54);   
		HX8312A_wr_cmd(0x93,0x07);   
		HX8312A_wr_cmd(0x94,0x00);   
		HX8312A_wr_cmd(0x95,0x77);   
		HX8312A_wr_cmd(0x96,0x45);   
		HX8312A_wr_cmd(0x97,0x00);   
		HX8312A_wr_cmd(0x98,0x06);   
		HX8312A_wr_cmd(0x99,0x03);   
		HX8312A_wr_cmd(0x9a,0x00);   
		
		/* display on */  
		HX8312A_wr_cmd(0x3b,0x01);   
		_delay_ms(40);
	}
	
	else { for(;;);} /* Invalid Device Code!! */

	HX8312A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8312A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8312A_wr_dat(COL_RED);
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
int HX8312A_draw_bmp(const uint8_t* ptr){

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
	HX8312A_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX8312A_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX8312A_wr_dat(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
