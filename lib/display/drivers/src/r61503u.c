/********************************************************************************/
/*!
	@file			r61503u.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.09.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive EP-T022QCW1R1-T03	TFT module(8/16bit mode).

    @section HISTORY
		2012.09.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "r61503u.h"
/* check header file version for fool proof */
#if __R61503U_H != 0x0100
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
inline void R61503U_reset(void)
{
	R61503U_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	R61503U_RD_SET();
	R61503U_WR_SET();
	_delay_ms(50);								/* wait 1ms     			*/

	R61503U_RES_CLR();							/* RES=L, CS=L   			*/
	R61503U_CS_CLR();
	_delay_ms(50);								/* wait 50ms     			*/
	
	R61503U_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void R61503U_wr_cmd(uint16_t cmd)
{
	R61503U_DC_CLR();							/* DC=L							*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	R61503U_CMD = 0;
	R61503U_WR();
#endif
	R61503U_CMD = (uint8_t)cmd;					/* command(8bit_Low or 16bit)	*/
	R61503U_WR();								/* WR=L->H						*/

	R61503U_DC_SET();							/* DC=H							*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void R61503U_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	R61503U_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	R61503U_WR();								/* WR=L->H					*/
#endif
	R61503U_DATA =  dat;						/* lower 8bit data			*/
	R61503U_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void R61503U_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		R61503U_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		R61503U_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void R61503U_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	R61503U_wr_cmd(0x50);				/* Horizontal RAM Start ADDR */
	R61503U_wr_dat(OFS_COL + x);
	R61503U_wr_cmd(0x51);				/* Horizontal RAM End ADDR */
	R61503U_wr_dat(OFS_COL + width);
	R61503U_wr_cmd(0x52);				/* Vertical RAM Start ADDR */
	R61503U_wr_dat(OFS_RAW + y);
	R61503U_wr_cmd(0x53);				/* Vertical End ADDR */
	R61503U_wr_dat(OFS_RAW + height);

	R61503U_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	R61503U_wr_dat(OFS_COL + x);
	R61503U_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	R61503U_wr_dat(OFS_RAW + y);

	R61503U_wr_cmd(0x22);				/* Write Data to GRAM */
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void R61503U_clear(void)
{
	volatile uint32_t n;

	R61503U_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61503U_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t R61503U_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	R61503U_wr_cmd(cmd);
	R61503U_WR_SET();

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
void R61503U_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	R61503U_reset();

	R61503U_wr_dat(0x0000);
	R61503U_wr_dat(0x0000);
	R61503U_wr_dat(0x0000);
	R61503U_wr_dat(0x0000);

	/* Check Device Code */
	devicetype = R61503U_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x1503)
	{
		/* Initialize R61503U */
		R61503U_wr_cmd(0xA4);
		R61503U_wr_dat(0x0001);
		_delay_ms(50);

		/* Power On Sequanece */
		R61503U_wr_cmd(0x07);
		R61503U_wr_dat(0x0001);
		R61503U_wr_cmd(0x18);
		R61503U_wr_dat(0x0001);
		R61503U_wr_cmd(0x10);
		R61503U_wr_dat(0x11b0);
		R61503U_wr_cmd(0x11);
		R61503U_wr_dat(0x0110);
		R61503U_wr_cmd(0x12);
		R61503U_wr_dat(0x0036);
		R61503U_wr_cmd(0x13);
		R61503U_wr_dat(0x8c18);
		R61503U_wr_cmd(0x12);
		R61503U_wr_dat(0x0036);
		R61503U_wr_cmd(0x14);
		R61503U_wr_dat(0x8000);
		R61503U_wr_cmd(0x01);
		R61503U_wr_dat(0x0100);
		R61503U_wr_cmd(0x02);
		R61503U_wr_dat(0x0700);
		R61503U_wr_cmd(0x03);
		R61503U_wr_dat((1<<12)|(0<<9)|(1<<5)|(1<<4)|(0<<3));

		R61503U_wr_cmd(0x04);
		R61503U_wr_dat(0x0000);
		R61503U_wr_cmd(0x08);
		R61503U_wr_dat(0x0808);
		R61503U_wr_cmd(0x09);
		R61503U_wr_dat(0x0000);
		R61503U_wr_cmd(0x20);
		R61503U_wr_dat(0x0000);
		R61503U_wr_cmd(0x21);
		R61503U_wr_dat(0x0000);

		/* Gamma */
		R61503U_wr_cmd(0x30);
		R61503U_wr_dat(0x0400);
		R61503U_wr_cmd(0x31);
		R61503U_wr_dat(0x0506);
		R61503U_wr_cmd(0x32);
		R61503U_wr_dat(0x0003);
		R61503U_wr_cmd(0x33);
		R61503U_wr_dat(0x0202);
		R61503U_wr_cmd(0x34);
		R61503U_wr_dat(0x0104);
		R61503U_wr_cmd(0x35);
		R61503U_wr_dat(0x0004);
		R61503U_wr_cmd(0x36);
		R61503U_wr_dat(0x0707);
		R61503U_wr_cmd(0x37);
		R61503U_wr_dat(0x0303);
		R61503U_wr_cmd(0x38);
		R61503U_wr_dat(0x0005);
		R61503U_wr_cmd(0x39);
		R61503U_wr_dat(0x0500);
		R61503U_wr_cmd(0x3A);
		R61503U_wr_dat(0x0E00);

		/* Other */
		R61503U_wr_cmd(0x70);
		R61503U_wr_dat(0x1b00);
		_delay_ms(30);
		R61503U_wr_cmd(0x71);
		R61503U_wr_dat(0x0001);
		_delay_ms(30);		
		R61503U_wr_cmd(0x7a);
		R61503U_wr_dat(0x0000);
		_delay_ms(30);		
		R61503U_wr_cmd(0x85);
		R61503U_wr_dat(0x0000);
		R61503U_wr_cmd(0x90);
		R61503U_wr_dat(0x0008);
		R61503U_wr_cmd(0x91);
		R61503U_wr_dat(0x0100);
		R61503U_wr_cmd(0x92);
		R61503U_wr_dat(0x0001);
		
		/* Display On Sequance */
		R61503U_wr_cmd(0x07);
		R61503U_wr_dat(0x0001);
		R61503U_wr_cmd(0x07);
		R61503U_wr_dat(0x0021);
		R61503U_wr_cmd(0x12);
		R61503U_wr_dat(0x1138);
		R61503U_wr_cmd(0x07);
		R61503U_wr_dat(0x0233);
	}

	else { for(;;);} /* Invalid Device Code!! */

	R61503U_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	R61503U_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61503U_wr_dat(COL_RED);
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
int R61503U_draw_bmp(const uint8_t* ptr){

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
	R61503U_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		R61503U_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			R61503U_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
