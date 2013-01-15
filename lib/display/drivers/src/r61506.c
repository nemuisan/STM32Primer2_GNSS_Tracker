/********************************************************************************/
/*!
	@file			r61506.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive T-51991GD015-MLW-AIN	TFT module(8bit bus).	@n

    @section HISTORY
		2011.11.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "r61506.h"
/* check header file version for fool proof */
#if __R61506_H != 0x0100
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
//#define SET_ILI9320M

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
inline void R61506_reset(void)
{
	R61506_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	R61506_RD_SET();
	R61506_WR_SET();
	_delay_ms(50);								/* wait 1ms     			*/

	R61506_RES_CLR();							/* RES=L, CS=L   			*/
	R61506_CS_CLR();
	_delay_ms(50);								/* wait 50ms     			*/
	
	R61506_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void R61506_wr_cmd(uint8_t cmd)
{
	R61506_DC_CLR();							/* DC=L							*/

	R61506_CMD =  cmd;							/* command(8bit_Low or 16bit)	*/
	R61506_WR();								/* WR=L->H						*/

	R61506_DC_SET();							/* DC=H							*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void R61506_wr_dat(uint16_t dat)
{
	R61506_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	R61506_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void R61506_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	R61506_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	R61506_WR();								/* WR=L->H					*/
#endif

	R61506_DATA = gram;							/* data(8bit_Low or 16bit)	*/
	R61506_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void R61506_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		R61506_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		R61506_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void R61506_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
#if defined(SET_ILI9320M)
	R61506_wr_cmd(0x0050);				/* Horizontal RAM Start ADDR */
	R61506_wr_dat(OFS_COL + x);
	R61506_wr_cmd(0x0051);				/* Horizontal RAM End ADDR */
	R61506_wr_dat(OFS_COL + width);
	R61506_wr_cmd(0x0052);				/* Vertical RAM Start ADDR */
	R61506_wr_dat(OFS_RAW + y);
	R61506_wr_cmd(0x0053);				/* Vertical End ADDR */
	R61506_wr_dat(OFS_RAW + height);

	R61506_wr_cmd(0x0020);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	R61506_wr_dat(OFS_RAW + y);
	R61506_wr_cmd(0x0021);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	R61506_wr_dat(OFS_COL + x);

	R61506_wr_cmd(0x0022);			/* Write Data to GRAM */
#else
	R61506_wr_cmd(0x2A);				/* GRAM Horizontal ADDR Set */
	R61506_wr_dat(0);
	R61506_wr_dat(OFS_COL + x);
	R61506_wr_dat(0);
	R61506_wr_dat(OFS_COL + width);

	R61506_wr_cmd(0x2B);				/* GRAM Vertical ADDR Set */
	R61506_wr_dat(0);
	R61506_wr_dat(OFS_RAW + y); 
	R61506_wr_dat(0);
	R61506_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	R61506_wr_cmd(0x2C);
#endif

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void R61506_clear(void)
{
	volatile uint32_t n;

	R61506_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61506_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t R61506_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	R61506_wr_cmd(cmd);
	R61506_WR_SET();

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
void R61506_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	R61506_reset();

	/* Check Device Code */
	devicetype = R61506_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	/* Initialize R61506 */
#if defined(SET_ILI9320M)
R61506_wr_cmd(0x00a4);R61506_wr_dat(0x0001);
_delay_ms(100);
R61506_wr_cmd(0x0001);R61506_wr_dat(0x0100);
R61506_wr_cmd(0x0002);R61506_wr_dat(0x0300);
R61506_wr_cmd(0x0003);R61506_wr_dat(0x1230);
R61506_wr_cmd(0x0008);R61506_wr_dat(0x0404);
R61506_wr_cmd(0x0008);R61506_wr_dat(0x0404);
R61506_wr_cmd(0x000e);R61506_wr_dat(0x0010);
R61506_wr_cmd(0x0070);R61506_wr_dat(0x1000);
R61506_wr_cmd(0x0071);R61506_wr_dat(0x0001);
R61506_wr_cmd(0x0030);R61506_wr_dat(0x0002);
R61506_wr_cmd(0x0031);R61506_wr_dat(0x0400);
R61506_wr_cmd(0x0032);R61506_wr_dat(0x0007);
R61506_wr_cmd(0x0033);R61506_wr_dat(0x0500);
R61506_wr_cmd(0x0034);R61506_wr_dat(0x0007);
R61506_wr_cmd(0x0035);R61506_wr_dat(0x0703);
R61506_wr_cmd(0x0036);R61506_wr_dat(0x0507);
R61506_wr_cmd(0x0037);R61506_wr_dat(0x0005);
R61506_wr_cmd(0x0038);R61506_wr_dat(0x0407);
R61506_wr_cmd(0x0039);R61506_wr_dat(0x000e);
R61506_wr_cmd(0x0040);R61506_wr_dat(0x0202);
R61506_wr_cmd(0x0041);R61506_wr_dat(0x0003);
R61506_wr_cmd(0x0042);R61506_wr_dat(0x0000);
R61506_wr_cmd(0x0043);R61506_wr_dat(0x0200);
R61506_wr_cmd(0x0044);R61506_wr_dat(0x0707);
R61506_wr_cmd(0x0045);R61506_wr_dat(0x0407);
R61506_wr_cmd(0x0046);R61506_wr_dat(0x0505);
R61506_wr_cmd(0x0047);R61506_wr_dat(0x0002);
R61506_wr_cmd(0x0048);R61506_wr_dat(0x0004);
R61506_wr_cmd(0x0049);R61506_wr_dat(0x0004);
R61506_wr_cmd(0x0060);R61506_wr_dat(0x0202);
R61506_wr_cmd(0x0061);R61506_wr_dat(0x0003);
R61506_wr_cmd(0x0062);R61506_wr_dat(0x0000);
R61506_wr_cmd(0x0063);R61506_wr_dat(0x0200);
R61506_wr_cmd(0x0064);R61506_wr_dat(0x0707);
R61506_wr_cmd(0x0065);R61506_wr_dat(0x0407);
R61506_wr_cmd(0x0066);R61506_wr_dat(0x0505);
R61506_wr_cmd(0x0068);R61506_wr_dat(0x0004);
R61506_wr_cmd(0x0069);R61506_wr_dat(0x0004);
R61506_wr_cmd(0x0007);R61506_wr_dat(0x0001);
R61506_wr_cmd(0x0018);R61506_wr_dat(0x0001);
R61506_wr_cmd(0x0010);R61506_wr_dat(0x1690);
R61506_wr_cmd(0x0011);R61506_wr_dat(0x0100);
R61506_wr_cmd(0x0012);R61506_wr_dat(0x0117);
R61506_wr_cmd(0x0013);R61506_wr_dat(0x0f80);
R61506_wr_cmd(0x0012);R61506_wr_dat(0x0137);
R61506_wr_cmd(0x0020);R61506_wr_dat(0x0000);
R61506_wr_cmd(0x0021);R61506_wr_dat(0x0000);
R61506_wr_cmd(0x0050);R61506_wr_dat(0x0000);
R61506_wr_cmd(0x0051);R61506_wr_dat(0x00af);
R61506_wr_cmd(0x0052);R61506_wr_dat(0x0000);
R61506_wr_cmd(0x0053);R61506_wr_dat(0x0083);
R61506_wr_cmd(0x0090);R61506_wr_dat(0x0003);
R61506_wr_cmd(0x0091);R61506_wr_dat(0x0000);
R61506_wr_cmd(0x0092);R61506_wr_dat(0x0101);
R61506_wr_cmd(0x0098);R61506_wr_dat(0x0400);
R61506_wr_cmd(0x0099);R61506_wr_dat(0x1302);
R61506_wr_cmd(0x009a);R61506_wr_dat(0x0202);
R61506_wr_cmd(0x009b);R61506_wr_dat(0x0200);
_delay_ms(100);
R61506_wr_cmd(0x0007);R61506_wr_dat(0x0021);
R61506_wr_cmd(0x0012);R61506_wr_dat(0x0137);
_delay_ms(100);
R61506_wr_cmd(0x0007);R61506_wr_dat(0x0021);
R61506_wr_cmd(0x0012);R61506_wr_dat(0x1137);
_delay_ms(100);
R61506_wr_cmd(0x0007);R61506_wr_dat(0x0233);

#else
    R61506_wr_cmd(0x01);
    R61506_wr_dat(0x00);

    _delay_ms(10);

    R61506_wr_cmd(0xB1);
    R61506_wr_dat(0x16);
    R61506_wr_dat(0x03);

    R61506_wr_cmd(0xB2);
    R61506_wr_dat(0x17);
    R61506_wr_dat(0x03);

    R61506_wr_cmd(0xB4);
    R61506_wr_dat(0x00);

    R61506_wr_cmd(0xB6);
    R61506_wr_dat(0x01);

 //   R61506_wr_cmd(0xB7);
 //   R61506_wr_dat(0x00);
 //   R61506_wr_dat(0x00);
 //   R61506_wr_dat(0x02);
 //   R61506_wr_dat(0x00);
 //   R61506_wr_dat(0x06);
 //   R61506_wr_dat(0x26);
 //   R61506_wr_dat(0x2D);
 //   R61506_wr_dat(0x27);
 //   R61506_wr_dat(0x55);
 //   R61506_wr_dat(0x27);

    R61506_wr_cmd(0xB8);
    R61506_wr_dat(0x10);

    R61506_wr_cmd(0xB9);
    R61506_wr_dat(0x52);
    R61506_wr_dat(0x12);
    R61506_wr_dat(0x03);

    R61506_wr_cmd(0xC0);
    R61506_wr_dat(0x0A);
    R61506_wr_dat(0x10);
    R61506_wr_dat(0x10);

    R61506_wr_cmd(0xC2);
    R61506_wr_dat(0x14);
    R61506_wr_dat(0x23);

    R61506_wr_cmd(0xC3);
    R61506_wr_dat(0x12);
    R61506_wr_dat(0x23);

    R61506_wr_cmd(0xC6);
    R61506_wr_dat(0x48);

    R61506_wr_cmd(0xE0);
    R61506_wr_dat(0x20);
    R61506_wr_dat(0x71);
    R61506_wr_dat(0x17);
    R61506_wr_dat(0x09);
    R61506_wr_dat(0x70);
    R61506_wr_dat(0x0C);
    R61506_wr_dat(0x13);
    R61506_wr_dat(0x25);

    R61506_wr_cmd(0xE1);
    R61506_wr_dat(0x37);
    R61506_wr_dat(0x00);
    R61506_wr_dat(0x63);
    R61506_wr_dat(0x11);
    R61506_wr_dat(0xD9);
    R61506_wr_dat(0x00);
    R61506_wr_dat(0x12);
    R61506_wr_dat(0x01);

    R61506_wr_cmd(0xE2);
    R61506_wr_dat(0x42);
    R61506_wr_dat(0x42);
    R61506_wr_dat(0x60);
    R61506_wr_dat(0x08);
    R61506_wr_dat(0xB4);
    R61506_wr_dat(0x07);
    R61506_wr_dat(0x0E);
    R61506_wr_dat(0x90);

    R61506_wr_cmd(0xE3);
    R61506_wr_dat(0x47);
    R61506_wr_dat(0x60);
    R61506_wr_dat(0x66);
    R61506_wr_dat(0x09);
    R61506_wr_dat(0x6A);
    R61506_wr_dat(0x02);
    R61506_wr_dat(0x0E);
    R61506_wr_dat(0x09);

    R61506_wr_cmd(0xE4);
    R61506_wr_dat(0x11);
    R61506_wr_dat(0x40);
    R61506_wr_dat(0x03);
    R61506_wr_dat(0x0A);
    R61506_wr_dat(0xC1);
    R61506_wr_dat(0x0D);
    R61506_wr_dat(0x17);
    R61506_wr_dat(0x30);

    R61506_wr_cmd(0xE5);
    R61506_wr_dat(0x00);
    R61506_wr_dat(0x30);
    R61506_wr_dat(0x77);
    R61506_wr_dat(0x1C);
    R61506_wr_dat(0xFB);
    R61506_wr_dat(0x00);
    R61506_wr_dat(0x13);
    R61506_wr_dat(0x07);

    R61506_wr_cmd(0xE6);
    R61506_wr_dat(0x01);

    R61506_wr_cmd(0x35);
    R61506_wr_dat(0x00);

    R61506_wr_cmd(0x36);
    R61506_wr_dat(0x00);

    R61506_wr_cmd(0xF2);
    R61506_wr_dat(0x40);

    R61506_wr_cmd(0xF3);
    R61506_wr_dat(0x50);

    R61506_wr_cmd(0xFB);
    R61506_wr_dat(0x01);

    R61506_wr_cmd(0x11);
    R61506_wr_dat(0x00);

    _delay_ms(200);

    R61506_wr_cmd(0x3A);
    R61506_wr_dat(0x65);

    R61506_wr_cmd(0x29);
    R61506_wr_dat(0x00);

#endif

	/*else { for(;;);} *//* Invalid Device Code!! */

	R61506_clear();

#if 1	/* test code RED */
	volatile uint32_t n;

	R61506_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61506_wr_gram(COL_RED);
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
int R61506_draw_bmp(const uint8_t* ptr){

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
	R61506_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		R61506_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			R61506_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
