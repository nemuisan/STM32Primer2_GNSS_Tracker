/********************************************************************************/
/*!
	@file			tl1771.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.12.23
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive LTS182QQ-F0 TFT module.

    @section HISTORY
		2011.09.14	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "tl1771.h"
/* check header file version for fool proof */
#if __TL1771_H != 0x0300
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
inline void TL1771_reset(void)
{
	TL1771_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	TL1771_RD_SET();
	TL1771_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	TL1771_RES_CLR();							/* RES=L, CS=L   			*/
	TL1771_CS_CLR();

	_delay_ms(5);								/* wait 5ms     			*/
	TL1771_RES_SET();						  	/* RES=H					*/
	_delay_ms(5);				    			/* wait 5ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void TL1771_wr_cmd(uint8_t cmd)
{
	TL1771_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	TL1771_CMD = 0;
	TL1771_WR();
#endif

	TL1771_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	TL1771_WR();								/* WR=L->H					*/

	TL1771_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void TL1771_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	TL1771_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	TL1771_WR();								/* WR=L->H					*/
	TL1771_DATA = (uint8_t)dat;					/* lower 8bit data			*/
#else
	TL1771_DATA = dat;							/* 16bit data 				*/
#endif
	TL1771_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void TL1771_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		TL1771_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		TL1771_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t TL1771_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	TL1771_wr_cmd(cmd);
	TL1771_WR_SET();

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
inline void TL1771_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	TL1771_wr_cmd(0x0044);				/* GRAM Horizontal ADDR Set */
	TL1771_wr_dat(((OFS_COL + width) <<8) | (OFS_COL + x));
	TL1771_wr_cmd(0x0045);				/* GRAM Vertical ADDR Set */
	TL1771_wr_dat(((OFS_RAW + height)<<8) | (OFS_RAW + y));
	TL1771_wr_cmd(0x0021);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	TL1771_wr_dat(((OFS_RAW + y)<<8) | (OFS_COL + x));

	TL1771_wr_cmd(0x0022);				/* Write Data to GRAM */
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void TL1771_clear(void)
{
	volatile uint32_t n;

	TL1771_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		TL1771_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void TL1771_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	TL1771_reset();

	/* Check Device Code */
	devicetype = TL1771_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x1771)
	{
		TL1771_wr_cmd(0x0000);					/* start power */
		TL1771_wr_dat(0x0001);
		_delay_ms(15);
		TL1771_wr_cmd(0x0011);					/* power control 2 */
		TL1771_wr_dat(0x1f1e);
		_delay_ms(15);
		TL1771_wr_cmd(0x0012);					/* power control 3 */
		TL1771_wr_dat(0x0003);
		_delay_ms(15);
		TL1771_wr_cmd(0x0013);					/* power control 4 */
		TL1771_wr_dat(0x080c);
		_delay_ms(15);
		TL1771_wr_cmd(0x0014);					/* power control 5 */
		TL1771_wr_dat(0x3019);
		_delay_ms(15);
		TL1771_wr_cmd(0x0010);					/* power control 1 */
		TL1771_wr_dat(0x1a10);
		_delay_ms(50);
		TL1771_wr_cmd(0x0013);					/* power control 4 */
		TL1771_wr_dat(0x081c);
		_delay_ms(200);

		TL1771_wr_cmd(0x0001);					/* driver output control */
		TL1771_wr_dat((0<<9)|(1<<8)|(1<<3)|(1<<2));
		//TL1771_wr_dat(0x000c);

		TL1771_wr_cmd(0x0002);					/* lcd driving_waveform */
		TL1771_wr_dat(0x0200);  //0200

		TL1771_wr_cmd(0x0003);					/* entry mode */
		TL1771_wr_dat((1<<12)|(0<<6)|(1<<5)|(1<<4));
		//TL1771_wr_dat(0x1030);

		TL1771_wr_cmd(0x0007);					/* display control 1 */
		TL1771_wr_dat(0x0005);

		TL1771_wr_cmd(0x0008);					/* display control 2 */
		TL1771_wr_dat(0x0603);

		TL1771_wr_cmd(0x000b);					/* frame cycle control */
		TL1771_wr_dat(0x0006);

		TL1771_wr_cmd(0x000c);					/* external display interface control */
		TL1771_wr_dat(0x0000);

		TL1771_wr_cmd(0x000e);					/* equalize control */
		TL1771_wr_dat(0x0002);

		TL1771_wr_cmd(0x00a0);					/* power control 7 */
		TL1771_wr_dat(0x0100);

		TL1771_wr_cmd(0x00a7);					/* power control 9 */
		TL1771_wr_dat(0x0014);


		TL1771_wr_cmd(0x0030);					/* gamma control 1 */
		TL1771_wr_dat(0x0201);

		TL1771_wr_cmd(0x0031);					/* gamma control 2 */
		TL1771_wr_dat(0x0603);

		TL1771_wr_cmd(0x0032);					/* gamma control 3 */
		TL1771_wr_dat(0x0606);

		TL1771_wr_cmd(0x0033);					/* gamma control 4 */
		TL1771_wr_dat(0x0002);

		TL1771_wr_cmd(0x0034);					/* gamma control 5 */
		TL1771_wr_dat(0x0503);

		TL1771_wr_cmd(0x0035);					/* gamma control 6 */
		TL1771_wr_dat(0x0305);

		TL1771_wr_cmd(0x0036);					/* gamma control 7 */
		TL1771_wr_dat(0x0606);

		TL1771_wr_cmd(0x0037);					/* gamma control 8 */
		TL1771_wr_dat(0x0100);



		TL1771_wr_cmd(0x0038);					/* power control 6 */
		TL1771_wr_dat(0x0d0f);


		TL1771_wr_cmd(0x0040);					/* gate scan position register */
		TL1771_wr_dat(0x0000);

		TL1771_wr_cmd(0x0042);					/* 1st screen driving position */
		TL1771_wr_dat(0x9f00);

		TL1771_wr_cmd(0x0043);					/* 2st screen driving position */
		TL1771_wr_dat(0x9f00);

		TL1771_wr_cmd(0x0044);					/* horizontal wiondow address */
		TL1771_wr_dat(0x7f00);

		TL1771_wr_cmd(0x0045);					/* vertical wiondow address */
		TL1771_wr_dat(0x9f00);


		TL1771_wr_cmd(0x0007);
		TL1771_wr_dat(0x0021);

		TL1771_wr_cmd(0x0007);
		TL1771_wr_dat(0x0023);

		TL1771_wr_cmd(0x0007);
		TL1771_wr_dat(0x0037);
	}

	else { for(;;);} /* Invalid Device Code!! */

	TL1771_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	TL1771_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		TL1771_wr_dat(COL_RED);
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
int TL1771_draw_bmp(const uint8_t* ptr){

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
	TL1771_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		TL1771_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			TL1771_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
