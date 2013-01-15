/********************************************************************************/
/*!
	@file			hx8309a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2012.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95068				(HX8309A)		8/16bit mode.		
					 -FPC071-2A				(ILI9222)		8bit mode only.	

    @section HISTORY
		2012.05.20	V1.00	Start Here.
		2012.11.20	V2.00	Added ILI9222 Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8309a.h"
/* check header file version for fool proof */
#if __HX8309A_H != 0x0200
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
inline void HX8309A_reset(void)
{
	HX8309A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8309A_RD_SET();
	HX8309A_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	HX8309A_RES_CLR();							/* RES=L, CS=L   			*/
	HX8309A_CS_CLR();

	_delay_ms(20);								/* wait 20ms     			*/
	HX8309A_RES_SET();						  	/* RES=H					*/
	_delay_ms(30);				    			/* wait 30ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8309A_wr_cmd(uint8_t cmd)
{
	HX8309A_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8309A_CMD = 0;
	HX8309A_WR();
#endif

	HX8309A_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	HX8309A_WR();								/* WR=L->H					*/

	HX8309A_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX8309A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8309A_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	HX8309A_WR();								/* WR=L->H					*/
	HX8309A_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	HX8309A_DATA = dat;							/* 16bit data 				*/
#endif
	HX8309A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8309A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8309A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HX8309A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX8309A_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	HX8309A_wr_cmd(cmd);
	HX8309A_WR_SET();

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
inline void HX8309A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8309A_wr_cmd(0x44);				/* GRAM Horizontal ADDR Set */
	HX8309A_wr_dat(((OFS_COL + width) <<8) | (OFS_COL + x));
	HX8309A_wr_cmd(0x45);				/* GRAM Vertical ADDR Set */
	HX8309A_wr_dat(((OFS_RAW + height)<<8) | (OFS_RAW + y));
	HX8309A_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	HX8309A_wr_dat(((OFS_RAW + y)<<8) | (OFS_COL + x));

	HX8309A_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8309A_clear(void)
{
	volatile uint32_t n;

	HX8309A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8309A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8309A_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HX8309A_reset();

	/* Check Device Code */
	devicetype = HX8309A_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if((devicetype == 0x9220) || (devicetype == 0x8309))
	{
		/* Initialize ILI9220 & HX8309A */
		HX8309A_wr_cmd(0x01);
		HX8309A_wr_dat(0x011B);
		HX8309A_wr_cmd(0x01);
		HX8309A_wr_dat(0x011B);
		HX8309A_wr_cmd(0x02);
		HX8309A_wr_dat(0x0700);
		HX8309A_wr_cmd(0x03);
		HX8309A_wr_dat(0x7030);
		HX8309A_wr_cmd(0x04);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x05);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x08);
		HX8309A_wr_dat(0x0202);
		HX8309A_wr_cmd(0x09);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x0B);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x0C);
		HX8309A_wr_dat(0x0003);
		HX8309A_wr_cmd(0x40);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x41);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x42);
		HX8309A_wr_dat(0xDB00);
		HX8309A_wr_cmd(0x43);
		HX8309A_wr_dat(0xDB00);
		HX8309A_wr_cmd(0x44);
		HX8309A_wr_dat(0xAF00);
		HX8309A_wr_cmd(0x45);
		HX8309A_wr_dat(0xDB00);
		_delay_ms(50);

		HX8309A_wr_cmd(0x00);
		HX8309A_wr_dat(0x0001);
	
		_delay_ms(10);
		HX8309A_wr_cmd(0x10);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x11);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x12);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x13);
		HX8309A_wr_dat(0x0000);
	
		HX8309A_wr_cmd(0x11);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x13);
		HX8309A_wr_dat(0x1518);
		HX8309A_wr_cmd(0x12);
		HX8309A_wr_dat(0x0008);
		HX8309A_wr_cmd(0x10);
		HX8309A_wr_dat(0x4040);
		_delay_ms(10);
		HX8309A_wr_cmd(0x11);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x10);
		HX8309A_wr_dat(0x4140);
		HX8309A_wr_cmd(0x12);
		HX8309A_wr_dat(0x0011);
		HX8309A_wr_cmd(0x13);
		HX8309A_wr_dat(0x3308);
		_delay_ms(100);
	
		HX8309A_wr_cmd(0x30);
		HX8309A_wr_dat(0x0000);
		HX8309A_wr_cmd(0x31);
		HX8309A_wr_dat(0x0202);
		HX8309A_wr_cmd(0x32);
		HX8309A_wr_dat(0x0700);
		HX8309A_wr_cmd(0x33);
		HX8309A_wr_dat(0x0007);
		HX8309A_wr_cmd(0x34);
		HX8309A_wr_dat(0x0004);
		HX8309A_wr_cmd(0x35);
		HX8309A_wr_dat(0x0101);
		HX8309A_wr_cmd(0x36);
		HX8309A_wr_dat(0x0305);
		HX8309A_wr_cmd(0x37);
		HX8309A_wr_dat(0x0302);
		HX8309A_wr_cmd(0x38);
		HX8309A_wr_dat(0x1000);
		HX8309A_wr_cmd(0x39);
		HX8309A_wr_dat(0x030D);
	
		HX8309A_wr_cmd(0x10);
		HX8309A_wr_dat(0x4140);
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0005);
		_delay_ms(40);
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0025);
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0027);
		_delay_ms(40);
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0037);
		_delay_ms(10);
	}
	
	else if(devicetype == 0x9222)
	{
		/* Initialize ILI9222 */
		HX8309A_wr_cmd(0x00);
		HX8309A_wr_dat(0x0001);		/* Start internal OSC. */ 
		HX8309A_wr_cmd(0x01);
		HX8309A_wr_dat(0x011B);		/* set the display line number and display direction */ 
		HX8309A_wr_cmd(0x02);
		HX8309A_wr_dat(0x0700);		/* set 1 line inversion */ 
		HX8309A_wr_cmd(0x03);
		HX8309A_wr_dat(0x1030);		/* set GRAM write direction and BGR=1. */ 
		HX8309A_wr_cmd(0x04);
		HX8309A_wr_dat(0x0000);		/* Set mask bit */ 
		HX8309A_wr_cmd(0x05);
		HX8309A_wr_dat(0x0000);		/* Set mask bit */ 
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0000);		/* Display off */ 
		HX8309A_wr_cmd(0x08);
		HX8309A_wr_dat(0x0202);		/* set the back porch and front porch */ 
		HX8309A_wr_cmd(0x09);
		HX8309A_wr_dat(0x0000);		/* Display Control 3  */
		HX8309A_wr_cmd(0x4f);
		HX8309A_wr_dat(0x0012);
		HX8309A_wr_cmd(0x0B);
		HX8309A_wr_dat(0x0001);		/* set the clocks number per line */ 
		HX8309A_wr_cmd(0x0C);
		HX8309A_wr_dat(0x0000);		/* CPU interface */ 
		HX8309A_wr_cmd(0x10);
		HX8309A_wr_dat(0x0000);		/* Set VGH/VGL voltage, turn off the charge-pump circuit */ 
		_delay_ms(10);				/* Delay 10 ms */ 
		HX8309A_wr_cmd(0x11);
		HX8309A_wr_dat(0x0000);		/* Set the frequency of charge-pump circuit and reference voltage */ 
		HX8309A_wr_cmd(0x12);
		HX8309A_wr_dat(0x0000);		/* Set the reference voltage of gamma circuit */ 
		_delay_ms(40);				/* Delay 40 ms */
		HX8309A_wr_cmd(0x13);
		HX8309A_wr_dat(0x0000);		/* Set VCOMH/VCOML voltage */ 
		_delay_ms(40);				/* Delay 40 ms */ 
		HX8309A_wr_cmd(0x56);
		HX8309A_wr_dat(0x080F);		/* Please add this line */ 
		/* Please follow this power on sequence */
		HX8309A_wr_cmd(0x10);
		HX8309A_wr_dat(0x4140);		/* Set VGH/VGL voltage, turn on the charge-pump circuit */ 
		_delay_ms(10);				/* Delay 10 ms */ 
		HX8309A_wr_cmd(0x11);
		HX8309A_wr_dat(0x0000);		/* Set the frequency of charge-pump circuit and reference voltage */ 
		HX8309A_wr_cmd(0x12);
		HX8309A_wr_dat(0x0014);		/* Set the reference voltage of gamma circuit */ 
 
		_delay_ms(40);				/* Delay 40 ms */ 
		HX8309A_wr_cmd(0x13);
		HX8309A_wr_dat(0x131D);		/* Set VCOMH/VCOML voltage */ 
		_delay_ms(40);				/* Delay 40 ms */ 
		HX8309A_wr_cmd(0x13);
		HX8309A_wr_dat(0x331D);		/* Set VCOMH/VCOML voltage */
		
		/* Adjust the Gamma 2.2 Curve */
		HX8309A_wr_cmd(0x30);
		HX8309A_wr_dat(0x0300); 
		HX8309A_wr_cmd(0x31);
		HX8309A_wr_dat(0x0503); 
		HX8309A_wr_cmd(0x32);
		HX8309A_wr_dat(0x0304); 
		HX8309A_wr_cmd(0x33);
		HX8309A_wr_dat(0x0400); 
		HX8309A_wr_cmd(0x34);
		HX8309A_wr_dat(0x0006); 
		HX8309A_wr_cmd(0x35);
		HX8309A_wr_dat(0x0704); 
		HX8309A_wr_cmd(0x36);
		HX8309A_wr_dat(0x0707); 
		HX8309A_wr_cmd(0x37);
		HX8309A_wr_dat(0x0000); 
		HX8309A_wr_cmd(0x38);
		HX8309A_wr_dat(0x0304); 
		HX8309A_wr_cmd(0x39);
		HX8309A_wr_dat(0x0404);
		
		/* Set GRAM area */
		HX8309A_wr_cmd(0x21);
		HX8309A_wr_dat(0x0000);		/* set the writing start address of AC counter */ 
		HX8309A_wr_cmd(0x40);
		HX8309A_wr_dat(0x0000);		/* Gate Scan Control */ 
		HX8309A_wr_cmd(0x41);
		HX8309A_wr_dat(0x0000);		/* Vertical Scroll Control */ 
		HX8309A_wr_cmd(0x42);
		HX8309A_wr_dat(0xDB00);		/* 1st Screen Drive Position */ 
		HX8309A_wr_cmd(0x43);
		HX8309A_wr_dat(0xDB00);		/* 2nd Screen Drive Position */ 
		HX8309A_wr_cmd(0x44);
		HX8309A_wr_dat(0xAF00);		/* Horizontal RAM Address Position */ 
		HX8309A_wr_cmd(0x45);
		HX8309A_wr_dat(0xDB00);		/* Vertical RAM Address Position */ 
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0001); 
		_delay_ms(20);  
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0021); 
		_delay_ms(20); 
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0023); 
		_delay_ms(20); 
		HX8309A_wr_cmd(0x07);
		HX8309A_wr_dat(0x0037); 
		_delay_ms(40); 
	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8309A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8309A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8309A_wr_dat(COL_RED);
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
int HX8309A_draw_bmp(const uint8_t* ptr){

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
	HX8309A_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX8309A_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX8309A_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
