/********************************************************************************/
/*!
	@file			ssd1286a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CM06834 TFT module.

    @section HISTORY
		2011.06.10	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1286a.h"
/* check header file version for fool proof */
#if __SSD1286A_H != 0x0200
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
inline void SSD1286A_reset(void)
{
	SSD1286A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	SSD1286A_RD_SET();
	SSD1286A_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	SSD1286A_RES_CLR();							/* RES=L, CS=L   			*/
	SSD1286A_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	SSD1286A_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD1286A_wr_cmd(uint8_t cmd)
{
	SSD1286A_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SSD1286A_CMD = 0;
	SSD1286A_WR();
#endif

	SSD1286A_CMD = cmd;							/* cmd 8bit data			*/
	SSD1286A_WR();								/* WR=L->H					*/
	
	SSD1286A_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void SSD1286A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SSD1286A_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	SSD1286A_WR();								/* WR=L->H					*/
	SSD1286A_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	SSD1286A_DATA = dat;						/* 16bit data 				*/
#endif
	SSD1286A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SSD1286A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SSD1286A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		SSD1286A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1286A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	SSD1286A_wr_cmd(0x44);				/* Horizontal Start,End ADDR */
	SSD1286A_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	SSD1286A_wr_cmd(0x45);				/* Vertical Start,End ADDR */
	SSD1286A_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	SSD1286A_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD16) */
	SSD1286A_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	SSD1286A_wr_cmd(0x22);				/* Write Data to GRAM */
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1286A_clear(void)
{
	volatile uint32_t n;

	SSD1286A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1286A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SSD1286A_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	SSD1286A_wr_cmd(cmd);
	SSD1286A_WR_SET();
	
	ReadLCDData(temp);				/* Dummy Read */

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif

    ReadLCDData(val);

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	val &= 0x00FF;
	val |= temp<<8;
#endif
	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SSD1286A_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	SSD1286A_reset();

	/* Check Device Code */
	devicetype = SSD1286A_rd_cmd(0x0000); 			/* Confirm Vaild LCD Controller */

	if((devicetype == 0x1286) || (devicetype == 0x8383))
	{
		SSD1286A_wr_cmd(0x00);
		SSD1286A_wr_dat(0x0001);
		
		SSD1286A_wr_cmd(0x07);
		SSD1286A_wr_dat(0x0233);

		SSD1286A_wr_cmd(0x0B);
		SSD1286A_wr_dat(0x5C08);

		SSD1286A_wr_cmd(0x10);
		SSD1286A_wr_dat(0x11D2);

		SSD1286A_wr_cmd(0x11);
		SSD1286A_wr_dat(0x001C);

		SSD1286A_wr_cmd(0x12);
		SSD1286A_wr_dat(0x0509);

		SSD1286A_wr_cmd(0x13);
		SSD1286A_wr_dat(0x2A00);

		SSD1286A_wr_cmd(0x1E);
		SSD1286A_wr_dat(0x0131);
		
		SSD1286A_wr_cmd(0x01);
		SSD1286A_wr_dat(0x2A9F);

		SSD1286A_wr_cmd(0x02);
		SSD1286A_wr_dat(0x0700);

		SSD1286A_wr_cmd(0x03);
		SSD1286A_wr_dat((1<<14)|(1<<13)|(1<<5)|(1<<4)|(0<<3));
		
		SSD1286A_wr_cmd(0x04);
		SSD1286A_wr_dat(0x0000);

		SSD1286A_wr_cmd(0x05);
		SSD1286A_wr_dat(0x0000);

		SSD1286A_wr_cmd(0x17);
		SSD1286A_wr_dat(0x0101);

		SSD1286A_wr_cmd(0x07);
		SSD1286A_wr_dat(0x0733);

		SSD1286A_wr_cmd(0x40);
		SSD1286A_wr_dat(0x0001);

		SSD1286A_wr_cmd(0x41);
		SSD1286A_wr_dat(0x0000);

		SSD1286A_wr_cmd(0x42);
		SSD1286A_wr_dat(0x9F00);

		SSD1286A_wr_cmd(0x43);
		SSD1286A_wr_dat(0x5000);
/*
		SSD1286A_wr_cmd(0x44);
		SSD1286A_wr_dat(0x8102);

		SSD1286A_wr_cmd(0x45);
		SSD1286A_wr_dat(0x9F00);
*/
		SSD1286A_wr_cmd(0x2C);
		SSD1286A_wr_dat(0x3000);

		SSD1286A_wr_cmd(0x30);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x31);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x32);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x33);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x34);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x35);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x36);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x37);
		SSD1286A_wr_dat(0x0000);

		SSD1286A_wr_cmd(0x3A);
		SSD1286A_wr_dat(0x0000);
		SSD1286A_wr_cmd(0x3B);
		SSD1286A_wr_dat(0x0000);

	}

	else { for(;;);} /* Invalid Device Code!! */

	SSD1286A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1286A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		SSD1286A_wr_dat(COL_RED);
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
int SSD1286A_draw_bmp(const uint8_t* ptr){

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
		xs = 0;xe = MAX_X-1;
	} else {
		xs = (MAX_X - bw) / 2;
		xe = (MAX_X - bw) / 2 + bw - 1;
	}
	if (bh > MAX_Y) {
		ys = 0;ye = MAX_Y-1;
	} else {
		ys = (MAX_Y - bh) / 2;
		ye = (MAX_Y - bh) / 2 + bh - 1;
	}

	/* Clear Display */
	SSD1286A_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SSD1286A_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SSD1286A_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
