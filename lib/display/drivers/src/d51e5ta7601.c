/********************************************************************************/
/*!
	@file			d51e5ta7601.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.07.28
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P4705-E TFT module.(8/16bit)

    @section HISTORY
		2012.07.28	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "d51e5ta7601.h"
/* check header file version for fool proof */
#if __D51E5TA7601_H != 0x0100
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#warning "S**kly,This controller CANNOT SET odd X-Axis start address! \
So You CANNOT use several GFX sub Functions....! "
 
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
inline void D51E5TA7601_reset(void)
{
	D51E5TA7601_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	D51E5TA7601_RD_SET();
	D51E5TA7601_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	D51E5TA7601_RES_CLR();						/* RES=L, CS=L   			*/
	D51E5TA7601_CS_CLR();

	_delay_ms(50);								/* wait 50ms     			*/
	D51E5TA7601_RES_SET();						/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void D51E5TA7601_wr_cmd(uint16_t cmd)
{
	D51E5TA7601_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	D51E5TA7601_CMD = 0;
	D51E5TA7601_WR();
#endif

	D51E5TA7601_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	D51E5TA7601_WR();								/* WR=L->H					*/

	D51E5TA7601_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void D51E5TA7601_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	D51E5TA7601_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	D51E5TA7601_WR();								/* WR=L->H					*/
#endif

	D51E5TA7601_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	D51E5TA7601_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void D51E5TA7601_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		D51E5TA7601_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		D51E5TA7601_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t D51E5TA7601_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	D51E5TA7601_wr_cmd(cmd);
	D51E5TA7601_WR_SET();

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
inline void D51E5TA7601_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* NemuisanSaid...*/
	/* This Controller CANNOT SET odd START ADDRESS!! Too S**K!! */
	
#if defined(FLIP_SCREEN_TFT1P4705_E)
	D51E5TA7601_wr_cmd(0x0046);				/* Vertical End ADDR */
	D51E5TA7601_wr_dat(OFS_RAW + height);
	D51E5TA7601_wr_cmd(0x0047);				/* Vertical RAM Start ADDR */
	D51E5TA7601_wr_dat(OFS_RAW + y);

	D51E5TA7601_wr_cmd(0x0044);				/* Horizontal RAM End ADDR */
	D51E5TA7601_wr_dat(MAX_X-1 - x);
	D51E5TA7601_wr_cmd(0x0045);				/* Horizontal RAM Start ADDR */
	D51E5TA7601_wr_dat(MAX_X-1 - width);

	D51E5TA7601_wr_cmd(0x0020);				/* GRAM Vertical/Horizontal ADDR Set(AD9~AD17) */
	D51E5TA7601_wr_dat(OFS_RAW + y);
	D51E5TA7601_wr_cmd(0x0021);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD8) */
	D51E5TA7601_wr_dat(MAX_X-1 - x);
#else
	D51E5TA7601_wr_cmd(0x0044);				/* Horizontal RAM End ADDR */
	D51E5TA7601_wr_dat(OFS_COL + width);
	D51E5TA7601_wr_cmd(0x0045);				/* Horizontal RAM Start ADDR */
	D51E5TA7601_wr_dat(OFS_COL + x);
	D51E5TA7601_wr_cmd(0x0046);				/* Vertical End ADDR */
	D51E5TA7601_wr_dat(OFS_RAW + height);
	D51E5TA7601_wr_cmd(0x0047);				/* Vertical RAM Start ADDR */
	D51E5TA7601_wr_dat(OFS_RAW + y);

	D51E5TA7601_wr_cmd(0x0020);				/* GRAM Vertical/Horizontal ADDR Set(AD9~AD17) */
	D51E5TA7601_wr_dat(OFS_RAW + y);
	D51E5TA7601_wr_cmd(0x0021);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD8) */
	D51E5TA7601_wr_dat(OFS_COL + x);
#endif

	D51E5TA7601_wr_cmd(0x0022);				/* Write Data to GRAM */
	
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void D51E5TA7601_clear(void)
{
	volatile uint32_t n;

	D51E5TA7601_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		D51E5TA7601_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void D51E5TA7601_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	D51E5TA7601_reset();

	/* Check Device Code */
	devicetype = D51E5TA7601_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x7601)
	{
		/* Initialize D51E5TA7601 */
		D51E5TA7601_wr_cmd(0x0001);
#if defined(FLIP_SCREEN_TFT1P4705_E)
		D51E5TA7601_wr_dat(0x003C|(0<<10)|(0<<9));
#else
		D51E5TA7601_wr_dat(0x003C|(0<<10)|(1<<9));
#endif
		D51E5TA7601_wr_cmd(0x0002);
		D51E5TA7601_wr_dat(0x0100);
		D51E5TA7601_wr_cmd(0x0003);
#if defined(FLIP_SCREEN_TFT1P4705_E)
		D51E5TA7601_wr_dat(0x1020);
#else
		D51E5TA7601_wr_dat(0x1030);
#endif
		D51E5TA7601_wr_cmd(0x0008);
		D51E5TA7601_wr_dat(0x0808);
		D51E5TA7601_wr_cmd(0x000A);
		D51E5TA7601_wr_dat(0x0500);
		D51E5TA7601_wr_cmd(0x000B);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x000C);
		D51E5TA7601_wr_dat(0x0770);
		D51E5TA7601_wr_cmd(0x000D);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x000E);
		D51E5TA7601_wr_dat(0x0001);
		D51E5TA7601_wr_cmd(0x0011);
		D51E5TA7601_wr_dat(0x0406);
		D51E5TA7601_wr_cmd(0x0012);
		D51E5TA7601_wr_dat(0x000E);
		D51E5TA7601_wr_cmd(0x0013);
		D51E5TA7601_wr_dat(0x0222);
		D51E5TA7601_wr_cmd(0x0014);
		D51E5TA7601_wr_dat(0x0015);
		D51E5TA7601_wr_cmd(0x0015);
		D51E5TA7601_wr_dat(0x4277);
		D51E5TA7601_wr_cmd(0x0016);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x0030);
		D51E5TA7601_wr_dat(0x6A50);
		D51E5TA7601_wr_cmd(0x0031);
		D51E5TA7601_wr_dat(0x00C9);
		D51E5TA7601_wr_cmd(0x0032);
		D51E5TA7601_wr_dat(0xC7BE);
		D51E5TA7601_wr_cmd(0x0033);
		D51E5TA7601_wr_dat(0x0003);
		D51E5TA7601_wr_cmd(0x0036);
		D51E5TA7601_wr_dat(0x3443);
		D51E5TA7601_wr_cmd(0x003B);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x003C);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x002C);
		D51E5TA7601_wr_dat(0x6A50);
		D51E5TA7601_wr_cmd(0x002D);
		D51E5TA7601_wr_dat(0x00C9);
		D51E5TA7601_wr_cmd(0x002E);
		D51E5TA7601_wr_dat(0xC7BE);
		D51E5TA7601_wr_cmd(0x002F);
		D51E5TA7601_wr_dat(0x0003);
		D51E5TA7601_wr_cmd(0x0035);
		D51E5TA7601_wr_dat(0x3443);
		D51E5TA7601_wr_cmd(0x0039);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x003A);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x0028);
		D51E5TA7601_wr_dat(0x6A50);
		D51E5TA7601_wr_cmd(0x0029);
		D51E5TA7601_wr_dat(0x00C9);
		D51E5TA7601_wr_cmd(0x002A);
		D51E5TA7601_wr_dat(0xC7BE);
		D51E5TA7601_wr_cmd(0x002B);
		D51E5TA7601_wr_dat(0x0003);
		D51E5TA7601_wr_cmd(0x0034);
		D51E5TA7601_wr_dat(0x3443);
		D51E5TA7601_wr_cmd(0x0037);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x0038);
		D51E5TA7601_wr_dat(0x0000);
		_delay_ms(20);
		D51E5TA7601_wr_cmd(0x0012);
		D51E5TA7601_wr_dat(0x200E);
		_delay_ms(160);
		D51E5TA7601_wr_cmd(0x0012);
		D51E5TA7601_wr_dat(0x2003);
		_delay_ms(40);
		D51E5TA7601_wr_cmd(0x0044);
		D51E5TA7601_wr_dat(0x013F);
		D51E5TA7601_wr_cmd(0x0045);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x0046);
		D51E5TA7601_wr_dat(0x01DF);
		D51E5TA7601_wr_cmd(0x0047);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x0020);
		D51E5TA7601_wr_dat(0x0000);
		D51E5TA7601_wr_cmd(0x0021);
		D51E5TA7601_wr_dat(0x013F);
		D51E5TA7601_wr_cmd(0x0022);
		/* Display still image, choose the code below */
		D51E5TA7601_wr_cmd(0x0067);
		D51E5TA7601_wr_dat(0x0200);
		D51E5TA7601_wr_cmd(0x0004);
		D51E5TA7601_wr_dat(0x7000);
		D51E5TA7601_wr_cmd(0x0005);
		D51E5TA7601_wr_dat(0x0002);
		D51E5TA7601_wr_cmd(0x0048);
		D51E5TA7601_wr_dat(0x4B90);
		D51E5TA7601_wr_cmd(0x0049);
		D51E5TA7601_wr_dat(0x95A0);
		D51E5TA7601_wr_cmd(0x004A);
		D51E5TA7601_wr_dat(0xA0AC);
		D51E5TA7601_wr_cmd(0x004B);
		D51E5TA7601_wr_dat(0xB5CE);
		
		D51E5TA7601_wr_cmd(0x0007);
		D51E5TA7601_wr_dat(0x0012);
		_delay_ms(40);
		D51E5TA7601_wr_cmd(0x0007);
		D51E5TA7601_wr_dat(0x0017);

	}

	else { for(;;);} /* Invalid Device Code!! */

	D51E5TA7601_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	D51E5TA7601_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		D51E5TA7601_wr_dat(COL_RED);
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
int D51E5TA7601_draw_bmp(const uint8_t* ptr){

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
	D51E5TA7601_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		D51E5TA7601_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
		#if 0
			/* 262k colour access */
			d2 = *p << 14;	  		/* Blue  */
			d  = (*p++ >> 4);		/* Blue  */
			d |= ((*p++>>2)<<4);	/* Green */
			d |= ((*p++>>2)<<10);	/* Red   */
			/* 262k +16M dither colour access */
			d2 = *p++ << 8;			/* Blue  */
			d  = *p++;				/* Green */
			d |= *p++ << 8;			/* Red   */
		#endif
			D51E5TA7601_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
