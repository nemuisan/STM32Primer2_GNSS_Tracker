/********************************************************************************/
/*!
	@file			r61514s.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive DST5012B-NH		TFT module(8bit bus).		@n

    @section HISTORY
		2010.12.31	V1.00	Stable Release.
		2011.03.10	V2.00	C++ Ready.
		2011.10.25	V3.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "r61514s.h"
/* check header file version for fool proof */
#if __R61514S_H != 0x0300
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
inline void R61514S_reset(void)
{
	R61514S_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	R61514S_RD_SET();
	R61514S_WR_SET();
	_delay_ms(50);								/* wait 1ms     			*/

	R61514S_RES_CLR();							/* RES=L, CS=L   			*/
	R61514S_CS_CLR();
	_delay_ms(50);								/* wait 50ms     			*/
	
	R61514S_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void R61514S_wr_cmd(uint16_t cmd)
{
	R61514S_DC_CLR();							/* DC=L							*/

	R61514S_CMD = (uint8_t)(cmd>>8);
	R61514S_WR();

	R61514S_CMD = (uint8_t)cmd;					/* command(8bit_Low or 16bit)	*/
	R61514S_WR();								/* WR=L->H						*/

	R61514S_DC_SET();							/* DC=H							*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void R61514S_wr_dat(uint16_t dat)
{
	R61514S_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	R61514S_WR();								/* WR=L->H					*/
	
	R61514S_DATA = (uint8_t)dat;				/* lower 8bit data			*/
	R61514S_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void R61514S_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		R61514S_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		R61514S_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void R61514S_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	R61514S_wr_cmd(0x0044);				/* GRAM Horizontal ADDR Set */
	R61514S_wr_dat(((OFS_COL + width) <<8) | (OFS_COL + x));
	R61514S_wr_cmd(0x0045);				/* GRAM Vertical ADDR Set */
	R61514S_wr_dat(((OFS_RAW + height)<<8) | (OFS_RAW + y));
	R61514S_wr_cmd(0x0021);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	R61514S_wr_dat(((OFS_RAW + y)<<8) | (OFS_COL + x));

	R61514S_wr_cmd(0x0022);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void R61514S_clear(void)
{
	volatile uint32_t n;

	R61514S_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61514S_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t R61514S_rd_cmd(uint16_t cmd)
{
	uint16_t val;
	uint16_t temp;

	R61514S_wr_cmd(cmd);
	R61514S_WR_SET();

    ReadLCDData(temp);
    ReadLCDData(val);

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void R61514S_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	R61514S_reset();

	/* Check Device Code */
	devicetype = R61514S_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x1514)
	{
		/* Initialize R61514S */
		R61514S_wr_cmd(0x0000);
		R61514S_wr_dat(0x0001);
		R61514S_wr_cmd(0x0007);
		R61514S_wr_dat(0x0000);
		_delay_ms(40);
		R61514S_wr_cmd(0x0017);
		R61514S_wr_dat(0x0001);
		R61514S_wr_cmd(0x0010);
		R61514S_wr_dat(0x4410);
		R61514S_wr_cmd(0x0011);
		R61514S_wr_dat(0x0000);
		R61514S_wr_cmd(0x0012);
		R61514S_wr_dat(0x0118);
		R61514S_wr_cmd(0x0013);
		R61514S_wr_dat(0x0d23);
		_delay_ms(40);
		R61514S_wr_cmd(0x0010);
		R61514S_wr_dat(0x4490);
		R61514S_wr_cmd(0x0011);
		R61514S_wr_dat(0x0000);
		R61514S_wr_cmd(0x0012);
		R61514S_wr_dat(0x013d);
		_delay_ms(40);
		R61514S_wr_cmd(0x0001);
		R61514S_wr_dat((0<<10)|(0<<9)|(1<<8)|(1<<4)|(0<<2)|(1<<1)|(1<<0));		/* R61514S_wr_dat(0x0213); */
		R61514S_wr_cmd(0x0003);
		R61514S_wr_dat((1<<12)|(1<<9)|(1<<5)|(1<<4)|(1<<3));
		R61514S_wr_cmd(0x0007);
		R61514S_wr_dat(0x0005);
		R61514S_wr_cmd(0x0008);
		R61514S_wr_dat(0x0603);
		R61514S_wr_cmd(0x0009);
		R61514S_wr_dat(0x002f);
		R61514S_wr_cmd(0x000b);
		R61514S_wr_dat(0x8000);

		R61514S_wr_cmd(0x0040);
		R61514S_wr_dat(0x0000);
		R61514S_wr_cmd(0x0041);
		R61514S_wr_dat(0x0000);
		R61514S_wr_cmd(0x0042);
		R61514S_wr_dat(0x9f00);
		R61514S_wr_cmd(0x0043);
		R61514S_wr_dat(0x9f00);
		R61514S_wr_cmd(0x0044);
		R61514S_wr_dat(0x7f00);
		R61514S_wr_cmd(0x0045);
		R61514S_wr_dat(0x9f00);
		R61514S_wr_cmd(0x0021);
		R61514S_wr_dat(0x0000);

		R61514S_wr_cmd(0x0030);
		R61514S_wr_dat(0x0103);
		R61514S_wr_cmd(0x0031);
		R61514S_wr_dat(0x0001);
		R61514S_wr_cmd(0x0032);
		R61514S_wr_dat(0x0304);
		R61514S_wr_cmd(0x0033);
		R61514S_wr_dat(0x0204);
		R61514S_wr_cmd(0x0034);
		R61514S_wr_dat(0x0406);
		R61514S_wr_cmd(0x0035);
		R61514S_wr_dat(0x0707);
		R61514S_wr_cmd(0x0036);
		R61514S_wr_dat(0x0407);
		R61514S_wr_cmd(0x0037);
		R61514S_wr_dat(0x0303);
		R61514S_wr_cmd(0x0038);
		R61514S_wr_dat(0x0403);
		R61514S_wr_cmd(0x0039);
		R61514S_wr_dat(0x0102);
		_delay_ms(100);

		/* Display On */
		R61514S_wr_cmd(0x0002);
		R61514S_wr_dat(0x0600);
		R61514S_wr_cmd(0x0007);
		R61514S_wr_dat(0x0015);
		_delay_ms(100);
		R61514S_wr_cmd(0x0007);
		R61514S_wr_dat(0x0055);
		_delay_ms(100);
		R61514S_wr_cmd(0x0002);
		R61514S_wr_dat(0x0700);
		R61514S_wr_cmd(0x0007);
		R61514S_wr_dat((1<<6)|(1<<5)|(1<<4)|(1<<2)|(1<<1)|(1<<0));
		_delay_ms(100);

	}

	else { for(;;);} /* Invalid Device Code!! */

	R61514S_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	R61514S_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61514S_wr_dat(COL_RED);
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
int R61514S_draw_bmp(const uint8_t* ptr){

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
	R61514S_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		R61514S_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			R61514S_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
