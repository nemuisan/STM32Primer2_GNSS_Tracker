/********************************************************************************/
/*!
	@file			hx5051.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive C0200QILC-C OLED module.

    @section HISTORY
		2011.11.30	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx5051.h"
/* check header file version for fool proof */
#if __HX5051_H != 0x0100
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
inline void HX5051_reset(void)
{
#ifdef USE_HX5051_OLED
	HX5051_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX5051_RD_SET();
	HX5051_WR_SET();
	_delay_ms(100);								/* wait 100ms     			*/

	HX5051_RES_CLR();							/* RES=L, CS=L   			*/
	HX5051_CS_CLR();

#elif  USE_HX5051_SPI_OLED
	HX5051_RES_SET();							/* RES=H, CS=H				*/
	HX5051_CS_SET();
	HX5051_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(100);								/* wait 100ms     			*/

	HX5051_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(10);								/* wait 10ms     			*/
	HX5051_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_HX5051_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX5051_wr_cmd(uint16_t cmd)
{
	HX5051_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX5051_CMD = 0;
	HX5051_WR();
#endif

	HX5051_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	HX5051_WR();								/* WR=L->H					*/

	HX5051_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX5051_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX5051_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	HX5051_WR();								/* WR=L->H					*/
#endif

	HX5051_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	HX5051_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX5051_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX5051_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HX5051_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX5051_rd_cmd(uint16_t cmd)
{
#if 0
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	HX5051_wr_cmd(cmd);
	HX5051_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif

    ReadLCDData(val);

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	val &= 0x00FF;
	val |= temp<<8;
#endif
#endif
	/* C0200QILC-C OLED module DON'T HAVE RD Pin!! ,So CANNOT USE read function.*/
	return  0x8319;
}


#elif USE_HX5051_SPI_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX5051_wr_cmd(uint16_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI16(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX5051_wr_dat(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_DATA);
	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX5051_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */
	SendSPI(START_WR_DATA);

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt );
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
	}
#endif

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline  uint16_t HX5051_rd_cmd(uint16_t cmd)
{
	uint16_t val;
	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI16(cmd);
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_RD_DATA);
	temp = RecvSPI();							/* Upper Read */
	val  = RecvSPI();							/* Lower Read */

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX5051_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
#if defined(FLIP_SCREEN_C0200QILC)
	HX5051_wr_cmd(0x23);				/* Horizontal Start,End ADDR */
	HX5051_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	HX5051_wr_cmd(0x24);				/* Vertical Start,End ADDR */
	HX5051_wr_dat(((OFS_RAW - y)<<8)|(OFS_RAW - height));

	HX5051_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	HX5051_wr_dat(((OFS_RAW - y)<<8)|(OFS_COL + x));

#else
	HX5051_wr_cmd(0x23);				/* Horizontal Start,End ADDR */
	HX5051_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	HX5051_wr_cmd(0x24);				/* Vertical Start,End ADDR */
	HX5051_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	HX5051_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	HX5051_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));
#endif

	HX5051_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX5051_clear(void)
{
	volatile uint32_t n;

	HX5051_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX5051_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX5051_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HX5051_reset();

	/* Check Device Code */
	devicetype = HX5051_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8319)
	{
		/* Initialize HX5051 */
		HX5051_wr_cmd(0x07);
	#if defined(FLIP_SCREEN_C0200QILC)
		HX5051_wr_dat((1<<11)|(1<<9)|(0<<8)|(1<<2)|(0<<1));
	#else
		HX5051_wr_dat((1<<11)|(0<<9)|(0<<8)|(1<<2)|(0<<1));
	#endif
		/*HX5051_wr_dat(0x0804);*/
	
		HX5051_wr_cmd(0x03);
		HX5051_wr_dat(0x0015);
	
		HX5051_wr_cmd(0x01);
		HX5051_wr_dat(0x4740);

		HX5051_wr_cmd(0x21);
		HX5051_wr_dat(0x00AF);
		
		HX5051_wr_cmd(0x05);
	#if defined(FLIP_SCREEN_C0200QILC)
		HX5051_wr_dat((0<<5)|(1<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0));
	#else                                      
		HX5051_wr_dat((1<<5)|(1<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0));
	#endif

		HX5051_wr_cmd(0x1C);
		HX5051_wr_dat((1<<15));

		HX5051_wr_cmd(0x02);
		HX5051_wr_dat(0x0305);

		HX5051_wr_cmd(0x09);
		HX5051_wr_dat((1<<12)|(1<<11)|0x0003);

	}

	else { for(;;);} /* Invalid Device Code!! */

	HX5051_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX5051_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX5051_wr_dat(COL_BLUE);
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
int HX5051_draw_bmp(const uint8_t* ptr){

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
	HX5051_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX5051_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX5051_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
