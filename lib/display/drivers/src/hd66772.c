/********************************************************************************/
/*!
	@file			hd66772.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.10.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TX05D99VM1AAA TFT module.

    @section HISTORY
		2011.10.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "HD66772.h"
/* check header file version for fool proof */
#if __HD66772_H != 0x0100
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
inline void HD66772_reset(void)
{
	HD66772_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HD66772_RD_SET();
	HD66772_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	HD66772_RES_CLR();							/* RES=L, CS=L   			*/
	HD66772_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	HD66772_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HD66772_wr_cmd(uint8_t cmd)
{
	HD66772_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HD66772_CMD = 0;
	HD66772_WR();
#endif

	HD66772_CMD = cmd;							/* cmd(8bit)				*/
	HD66772_WR();								/* WR=L->H					*/
	
	HD66772_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HD66772_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HD66772_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	HD66772_WR();								/* WR=L->H					*/
	HD66772_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	HD66772_DATA = dat;							/* 16bit data 				*/
#endif
	HD66772_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HD66772_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HD66772_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HD66772_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HD66772_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HD66772_wr_cmd(0x44);				/* Vertical Start,End ADDR */
	HD66772_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	HD66772_wr_cmd(0x45);				/* Horizontal Start,End ADDR */
	HD66772_wr_dat(((OFS_RAW+ height)<<8)|(OFS_RAW + y));

	HD66772_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	HD66772_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	HD66772_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HD66772_clear(void)
{
	volatile uint32_t n;

	HD66772_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HD66772_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HD66772_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	HD66772_wr_cmd(cmd);
	HD66772_WR_SET();

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
void HD66772_init (void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HD66772_reset();

	/* Check Device Code */
	devicetype = HD66772_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0772)
	{
		HD66772_wr_cmd(0x00);
		HD66772_wr_dat(0x0001);					/* Start_osc */
		_delay_ms(1);
	
		HD66772_wr_cmd(0x03);
		HD66772_wr_dat(0x0030);
		/*HD66772_wr_cmd(0x03);
		HD66772_wr_dat(0x0230);*/
		HD66772_wr_cmd(0x01);
		HD66772_wr_dat(0x011D);
		_delay_ms(1);
	
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0106); 
		_delay_ms(1);


		HD66772_wr_cmd(0x08);
		HD66772_wr_dat(0x0707);
		HD66772_wr_cmd(0x0c);
		HD66772_wr_dat(0x01);
		HD66772_wr_cmd(0x02);
		HD66772_wr_dat(0x0400);
		_delay_ms(1);
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0107); 
		_delay_ms(1);


		HD66772_wr_cmd(0x12);
		HD66772_wr_dat(0x0001);					/* VciOUT = 3V ` 0.83 = 2.49V */
		/* only 100(=VCI), 000(.92 VCI),001(.83),010(.73VCI) is valid. other is inhibited */
		_delay_ms(1);
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0101); 
		_delay_ms(1);
	   
		/*HD66772_wr_cmd(0x13);
		HD66772_wr_dat(0x404);*/
		/*_delay_ms(1);
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0101);
		_delay_ms(1);*/

		HD66772_wr_cmd(0x11);
		HD66772_wr_dat(0x0505);					 /* CAD = 0; */
		/*_delay_ms(1);
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0101);
		_delay_ms(1);*/



		HD66772_wr_cmd(0x14);
		HD66772_wr_dat(0x2c0f); 				/* last 4 bit must be 1!!! vcomg = 1, */
		/* VDV4-0 == 09-0e,10-12  other is inhibited.!!! */
		_delay_ms(1);
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0102); 
		_delay_ms(1);

		HD66772_wr_cmd(0x10);
		HD66772_wr_dat(0x1a10);					/* if Vci > 3.0V, use 0x1a10. */
		_delay_ms(1);
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0100); 
		_delay_ms(1);

		/* !!!!!!!!!!!!! VERY IMPORTANT!!! */
		HD66772_wr_cmd(0x13);
		HD66772_wr_dat(0x0616);
		/* set VRH3-0 to fit REG1out= 3 to DDVDH-0.5 */
		_delay_ms(1);
	
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0101); 
		_delay_ms(1);

		HD66772_wr_cmd(0x30);
		HD66772_wr_dat(0x0003);
		HD66772_wr_cmd(0x31);
		HD66772_wr_dat(0x0404);
		HD66772_wr_cmd(0x32);
		HD66772_wr_dat(0x0303);
		HD66772_wr_cmd(0x33);
		HD66772_wr_dat(0x0406);
		HD66772_wr_cmd(0x34);
		HD66772_wr_dat(0x0404);
		HD66772_wr_cmd(0x35);
		HD66772_wr_dat(0x0303);
		HD66772_wr_cmd(0x36);
		HD66772_wr_dat(0x0407);
		HD66772_wr_cmd(0x37);
		HD66772_wr_dat(0x0604);
		HD66772_wr_cmd(0x07);
		HD66772_wr_dat(0x0037);
		_delay_ms(1);

		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0100);
		_delay_ms(1); 
		
		/************ SAP, AP set*****************/
		HD66772_wr_cmd(0x10);
		HD66772_wr_dat(0x1a10);					/* BT2-0 Ok,and... */
		_delay_ms(1);

		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0100); 
		_delay_ms(1);	

		/*********** PON  set********************/
		HD66772_wr_cmd(0x13);
		HD66772_wr_dat(0x0616);
		_delay_ms(1);
	
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0101); 
		_delay_ms(1);
		
		/***********Display ON set*****************/
		HD66772_wr_cmd(0x07);
		HD66772_wr_dat(0x0037);
		_delay_ms(1);
		HD66772_wr_cmd(0x0a);
		HD66772_wr_dat(0x0100); 
		_delay_ms(1);   

	}

	else { for(;;);} /* Invalid Device Code!! */

	HD66772_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HD66772_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		HD66772_wr_dat(COL_RED);
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
int HD66772_draw_bmp(const uint8_t* ptr){

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
	HD66772_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HD66772_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HD66772_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
