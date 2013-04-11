/********************************************************************************/
/*!
	@file			ili9132.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CMI-35 TFT module(16bit mode only).

    @section HISTORY
		2011.09.14	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9132.h"
/* check header file version for fool proof */
#if __ILI9132_H != 0x0200
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
inline void ILI9132_reset(void)
{
	ILI9132_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9132_RD_SET();
	ILI9132_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9132_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9132_CS_CLR();

	_delay_ms(30);								/* wait 30ms     			*/
	ILI9132_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9132_wr_cmd(uint16_t cmd)
{
	ILI9132_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9132_CMD = 0;
	ILI9132_WR();
#endif

	ILI9132_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	ILI9132_WR();								/* WR=L->H					*/

	ILI9132_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI9132_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9132_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	ILI9132_WR();								/* WR=L->H					*/
	ILI9132_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	ILI9132_DATA = dat;							/* 16bit data 				*/
#endif
	ILI9132_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9132_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9132_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9132_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9132_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	ILI9132_wr_cmd(cmd);
	ILI9132_WR_SET();

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
inline void ILI9132_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9132_wr_cmd(0x0016);				/* GRAM Horizontal ADDR Set */
	ILI9132_wr_dat(((OFS_COL + width) <<8) | (OFS_COL + x));
	ILI9132_wr_cmd(0x0017);				/* GRAM Vertical ADDR Set */
	ILI9132_wr_dat(((OFS_RAW + height)<<8) | (OFS_RAW + y));
	ILI9132_wr_cmd(0x0021);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	ILI9132_wr_dat(((OFS_RAW + y)<<8) | (OFS_COL + x));

	ILI9132_wr_cmd(0x0022);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9132_clear(void)
{
	volatile uint32_t n;

	ILI9132_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9132_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9132_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	ILI9132_reset();

	/* Check Device Code */
	devicetype = ILI9132_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x9132)
	{
		/* Start intial Sequence */  	
		ILI9132_wr_cmd(0x0003);
		ILI9132_wr_dat(0x0210);
		_delay_ms(50);
		ILI9132_wr_cmd(0x0009);
		ILI9132_wr_dat(0x0004);
		_delay_ms(50);
		ILI9132_wr_cmd(0x000d);
		ILI9132_wr_dat(0x0014);
		_delay_ms(50);
		ILI9132_wr_cmd(0x000e);
		ILI9132_wr_dat(0x311d);
		ILI9132_wr_cmd(0x0000);
		ILI9132_wr_dat(0x0001);
		ILI9132_wr_cmd(0x0001);
		ILI9132_wr_dat(0x010f);
		ILI9132_wr_cmd(0x0002);
		ILI9132_wr_dat(0x0700);
		ILI9132_wr_cmd(0x0005);
		ILI9132_wr_dat(0x1030);
		ILI9132_wr_cmd(0x0008);
		ILI9132_wr_dat(0x0305);
		ILI9132_wr_cmd(0x000a);
		ILI9132_wr_dat(0x0000);	
		ILI9132_wr_cmd(0x000b);
		ILI9132_wr_dat(0xfc00);
		ILI9132_wr_cmd(0x000c);
		ILI9132_wr_dat(0x0000);
		ILI9132_wr_cmd(0x000f);
		ILI9132_wr_dat(0x0000);
		
		/* Power On */
		ILI9132_wr_cmd(0x0010);
		ILI9132_wr_dat(0x0000);
		ILI9132_wr_cmd(0x0011);
		ILI9132_wr_dat(0x0000);
		ILI9132_wr_cmd(0x0014);
		ILI9132_wr_dat(0x7f00);
		ILI9132_wr_cmd(0x0015);
		ILI9132_wr_dat(0x7f00);
		ILI9132_wr_cmd(0x0016);
		ILI9132_wr_dat(0x7f00);
		ILI9132_wr_cmd(0x0017);
		ILI9132_wr_dat(0x7f00);

		ILI9132_wr_cmd(0x0021);
		ILI9132_wr_dat(0x0000);

		/* Set gamma */
		ILI9132_wr_cmd(0x0030);
		ILI9132_wr_dat(0x0000);
		ILI9132_wr_cmd(0x0031);
		ILI9132_wr_dat(0x0606);
		ILI9132_wr_cmd(0x0032);
		ILI9132_wr_dat(0x0405);
		ILI9132_wr_cmd(0x0033);
		ILI9132_wr_dat(0x0007);
		ILI9132_wr_cmd(0x0034);
		ILI9132_wr_dat(0x0003);
		ILI9132_wr_cmd(0x0035);
		ILI9132_wr_dat(0x0000);
		ILI9132_wr_cmd(0x0036);
		ILI9132_wr_dat(0x0002);
		ILI9132_wr_cmd(0x0037);
		ILI9132_wr_dat(0x0500);
		ILI9132_wr_cmd(0x003a);
		ILI9132_wr_dat(0x1b07);
		ILI9132_wr_cmd(0x003b);
		ILI9132_wr_dat(0x180b);

		ILI9132_wr_cmd(0x0007);
		ILI9132_wr_dat(0x0037);
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9132_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9132_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9132_wr_dat(COL_RED);
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
int ILI9132_draw_bmp(const uint8_t* ptr){

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
	ILI9132_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		ILI9132_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			ILI9132_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
