/********************************************************************************/
/*!
	@file			hx8310a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive MT240TMA7B-04 TFT module(16bit mode only).	

    @section HISTORY
		2012.11.30	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8310a.h"
/* check header file version for fool proof */
#if __HX8310A_H != 0x0100
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
inline void HX8310A_reset(void)
{
	HX8310A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8310A_RD_SET();
	HX8310A_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	HX8310A_RES_CLR();							/* RES=L, CS=L   			*/
	HX8310A_CS_CLR();

	_delay_ms(20);								/* wait 20ms     			*/
	HX8310A_RES_SET();						  	/* RES=H					*/
	_delay_ms(30);				    			/* wait 30ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8310A_wr_cmd(uint8_t cmd)
{
	HX8310A_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8310A_CMD = 0;
	HX8310A_WR();
#endif

	HX8310A_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	HX8310A_WR();								/* WR=L->H					*/

	HX8310A_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX8310A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8310A_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	HX8310A_WR();								/* WR=L->H					*/
	HX8310A_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	HX8310A_DATA = dat;							/* 16bit data 				*/
#endif
	HX8310A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8310A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8310A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HX8310A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX8310A_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	HX8310A_wr_cmd(cmd);
	HX8310A_WR_SET();

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
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8310A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8310A_wr_cmd(0x16);				/* GRAM Horizontal ADDR Set */
	HX8310A_wr_dat(((OFS_COL + width) <<8) | (OFS_COL + x));
	HX8310A_wr_cmd(0x17);				/* GRAM Vertical ADDR Set */
	HX8310A_wr_dat(((OFS_RAW + height)<<8) | (OFS_RAW + y));
	HX8310A_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	HX8310A_wr_dat(((OFS_RAW + y)<<8) | (OFS_COL + x));

	HX8310A_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8310A_clear(void)
{
	volatile uint32_t n;

	HX8310A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8310A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8310A_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HX8310A_reset();

	/* Check Device Code */
	devicetype = HX8310A_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8310)
	{
		/* Initialize HX8310A */
		HX8310A_wr_cmd(0x00);
		HX8310A_wr_dat(0x0001);

		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0004);	

		HX8310A_wr_cmd(0x01);			/* Driver Output Direction */
		HX8310A_wr_dat((1<<8)|0x0013);			/* Up   --> Down;Left --> Right */

		HX8310A_wr_cmd(0x02);			/* LCD AC Signal */	
		HX8310A_wr_dat(0x0700);	

		HX8310A_wr_cmd(0x05);			/* entry mode */
		HX8310A_wr_dat((1<<12)|(1<<5)|(1<<4)|(0<<3));

		HX8310A_wr_cmd(0x06);			/* Compare Register */	
    	HX8310A_wr_dat(0x0000);	

		HX8310A_wr_cmd(0x0B);			/* Frame Contro */
		HX8310A_wr_dat(0x0000); 	
		_delay_ms(20);

		HX8310A_wr_cmd(0x0C);			/* Power 3 */
		HX8310A_wr_dat(0x0002); 	
		_delay_ms(20);              

		HX8310A_wr_cmd(0x03);			/* Power 1 */
		HX8310A_wr_dat(0x0410); 	
		_delay_ms(20);

		HX8310A_wr_cmd(0x04);			/* Power 2 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x0E);			/* Power 5 */
		HX8310A_wr_dat(0x351E);
		HX8310A_wr_cmd(0x0D);			/* Power 4 */
		HX8310A_wr_dat(0x0A1B);
		HX8310A_wr_cmd(0x30);			/* Gamma 1 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x31);			/* Gamma 2 */
		HX8310A_wr_dat(0x0000);	
		HX8310A_wr_cmd(0x32);			/* Gamma 3 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x33);			/* Gamma 4 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x34);			/* Gamma 5 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x35);			/* Gamma 6 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x36);			/* Gamma 7 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x37);			/* Gamma 8 */
		HX8310A_wr_dat(0x0000);	
		HX8310A_wr_cmd(0x3A);			/* Gamma 9 */
		HX8310A_wr_dat(0x0000);	
		HX8310A_wr_cmd(0x3B);			/* Gamma 10 */
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x0F);			/* COM Scan Position */
		HX8310A_wr_dat(0x0000);			/* 02 */

		HX8310A_wr_cmd(0x14);			/* 1'Screen Address */
		HX8310A_wr_dat(0x9F00);
	
		HX8310A_wr_cmd(0x16);			/* Window Horizonta Segm */
		HX8310A_wr_dat(0x7F00);
		HX8310A_wr_cmd(0x17);			/* Window Vertical Com */
		HX8310A_wr_dat(0x9F00);

		HX8310A_wr_cmd(0x07);			/* Diaplay Control */
		HX8310A_wr_dat(0x0005);

		HX8310A_wr_cmd(0x07);   
		HX8310A_wr_dat(0x0025);
		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0027);
		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0037);
	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8310A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8310A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8310A_wr_dat(COL_RED);
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
int HX8310A_draw_bmp(const uint8_t* ptr){

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
	HX8310A_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX8310A_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX8310A_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
