/********************************************************************************/
/*!
	@file			lgdp4511.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive KTFT4605FPC-B3-E TFT module(16bit bus).

    @section HISTORY
		2011.06.04	V1.00	Stable Release
		2011.10.25	V2.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lgdp4511.h"
/* check header file version for fool proof */
#if __LGDP4511_H != 0x0200
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
inline void LGDP4511_reset(void)
{
	LGDP4511_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	LGDP4511_RD_SET();
	LGDP4511_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	LGDP4511_RES_CLR();							/* RES=L, CS=L   			*/
	LGDP4511_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	LGDP4511_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void LGDP4511_wr_cmd(uint8_t cmd)
{
	LGDP4511_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	LGDP4511_CMD = 0;
	LGDP4511_WR();
#endif

	LGDP4511_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	LGDP4511_WR();								/* WR=L->H					*/

	LGDP4511_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void LGDP4511_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	LGDP4511_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	LGDP4511_WR();								/* WR=L->H					*/
	LGDP4511_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	LGDP4511_DATA = dat;						/* 16bit data 				*/
#endif
	LGDP4511_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void LGDP4511_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		LGDP4511_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		LGDP4511_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void LGDP4511_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	LGDP4511_wr_cmd(0x44);				/* Horizontal Start,End ADDR */
	LGDP4511_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	LGDP4511_wr_cmd(0x45);				/* Vertical Start,End ADDR */
	LGDP4511_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	LGDP4511_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	LGDP4511_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	LGDP4511_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void LGDP4511_clear(void)
{
	volatile uint32_t n;

	LGDP4511_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		LGDP4511_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t LGDP4511_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	LGDP4511_wr_cmd(cmd);
	LGDP4511_WR_SET();

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
void LGDP4511_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	LGDP4511_reset();

	/* Check Device Code */
	devicetype = LGDP4511_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x4511)
	{
		/* Initialize LGDP4511 */
		LGDP4511_wr_cmd(0x00);
		LGDP4511_wr_dat(0x0001);
		LGDP4511_wr_cmd(0x07);
		LGDP4511_wr_dat(0x0000);
		LGDP4511_wr_cmd(0x11);
		LGDP4511_wr_dat(0x0001);
		LGDP4511_wr_cmd(0x12);
		LGDP4511_wr_dat(0x000C);
		LGDP4511_wr_cmd(0x13);
		LGDP4511_wr_dat(0x0A0D);
		_delay_ms(20);

		LGDP4511_wr_cmd(0x10);
		LGDP4511_wr_dat(0x0004);
		LGDP4511_wr_cmd(0x10);
		LGDP4511_wr_dat(0x0014);
		LGDP4511_wr_cmd(0x11);
		LGDP4511_wr_dat(0x0112);
		LGDP4511_wr_cmd(0x12);
		LGDP4511_wr_dat(0x001C);
		LGDP4511_wr_cmd(0x13);
		LGDP4511_wr_dat(0x2A0D);
		LGDP4511_wr_cmd(0x10);
		LGDP4511_wr_dat(0x0710);
		_delay_ms(20);

		LGDP4511_wr_cmd(0x01);
		LGDP4511_wr_dat(0x0113);
		LGDP4511_wr_cmd(0x02);
		LGDP4511_wr_dat(0x0300);
		LGDP4511_wr_cmd(0x03);
		LGDP4511_wr_dat(0x0030);
		_delay_ms(20);

		LGDP4511_wr_cmd(0x08);
		LGDP4511_wr_dat(0x0808);
		LGDP4511_wr_cmd(0x09);
		LGDP4511_wr_dat(0x0000);
		LGDP4511_wr_cmd(0x0B);
		LGDP4511_wr_dat(0x0006);
		_delay_ms(20);
	
		LGDP4511_wr_cmd(0x30);
		LGDP4511_wr_dat(0x0707);
		LGDP4511_wr_cmd(0x31);
		LGDP4511_wr_dat(0x0407);
		LGDP4511_wr_cmd(0x32);
		LGDP4511_wr_dat(0x0000);
		LGDP4511_wr_cmd(0x33);
		LGDP4511_wr_dat(0x0506);
		LGDP4511_wr_cmd(0x34);
		LGDP4511_wr_dat(0x0707);
		LGDP4511_wr_cmd(0x35);
		LGDP4511_wr_dat(0x0007);
		LGDP4511_wr_cmd(0x36);
		LGDP4511_wr_dat(0x0000);
		LGDP4511_wr_cmd(0x37);
		LGDP4511_wr_dat(0x0305);
		LGDP4511_wr_cmd(0x38);
		LGDP4511_wr_dat(0x1100);
		LGDP4511_wr_cmd(0x39);
		LGDP4511_wr_dat(0x1106);
		_delay_ms(20);

		LGDP4511_wr_cmd(0x40);
		LGDP4511_wr_dat(0x0000);
		LGDP4511_wr_cmd(0x41);
		LGDP4511_wr_dat(0x0000);
		LGDP4511_wr_cmd(0x42);
		LGDP4511_wr_dat(0x9F00);
		LGDP4511_wr_cmd(0x43);
		LGDP4511_wr_dat(0x5C00);
		LGDP4511_wr_cmd(0x44);
		LGDP4511_wr_dat(0x7F00);
		LGDP4511_wr_cmd(0x45);
		LGDP4511_wr_dat(0x9F00);

		LGDP4511_wr_cmd(0x10);
		LGDP4511_wr_dat(0x3710);
		LGDP4511_wr_cmd(0x51);
		LGDP4511_wr_dat(0x0000);
	
		LGDP4511_wr_cmd(0x07);
		LGDP4511_wr_dat(0x0005);
		_delay_ms(10);
		LGDP4511_wr_cmd(0x07);
		LGDP4511_wr_dat(0x0025);
		_delay_ms(10);
		LGDP4511_wr_cmd(0x07);
		LGDP4511_wr_dat(0x0027);
		_delay_ms(10);
		LGDP4511_wr_cmd(0x07);
		LGDP4511_wr_dat(0x0037);
		_delay_ms(10);
	}

	else { for(;;);} /* Invalid Device Code!! */

	LGDP4511_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	LGDP4511_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		LGDP4511_wr_dat(COL_RED);
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
int LGDP4511_draw_bmp(const uint8_t* ptr){

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
	LGDP4511_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		LGDP4511_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			/* 262k colour access */
			//d2 = *p << 14;	  		/* Blue  */
			//d  = (*p++ >> 4);			/* Blue  */
			//d |= ((*p++>>2)<<4);		/* Green */
			//d |= ((*p++>>2)<<10);		/* Red   */
			/* 262k +16M dither colour access */
			//d2 = *p++ << 8;			/* Blue  */
			//d  = *p++;				/* Green */
			//d |= *p++ << 8;			/* Red   */
			LGDP4511_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
