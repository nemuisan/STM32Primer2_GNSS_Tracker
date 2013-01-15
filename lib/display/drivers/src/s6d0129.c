/********************************************************************************/
/*!
	@file			s6d0129.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2012.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95591-AAA				(S6D0128)	8/16bit mode.		@n
					 -TFT1P0913-E				(S6D1121)	8/16bit,Serial mode.

    @section HISTORY
		2012.01.02	V1.00	Stable Release.
		2012.11.30	V5.00	Added TFT1P0913-E(S6D1121) support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d0129.h"
/* check header file version for fool proof */
#if __S6D0129_H != 0x0200
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
inline void S6D0129_reset(void)
{
#ifdef USE_S6D0129_TFT
	S6D0129_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D0129_RD_SET();
	S6D0129_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S6D0129_RES_CLR();							/* RES=L, CS=L   			*/
	S6D0129_CS_CLR();

#elif  USE_S6D0129_SPI_TFT
	S6D0129_RES_SET();							/* RES=H, CS=H				*/
	S6D0129_CS_SET();
	S6D0129_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	S6D0129_RES_CLR();							/* RES=L		   			*/

#endif
	_delay_ms(50);								/* wait 60ms     			*/
	S6D0129_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_S6D0129_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D0129_wr_cmd(uint8_t cmd)
{
	S6D0129_DC_CLR();							/* DC=L						*/

	S6D0129_CMD = cmd;							/* cmd						*/
	S6D0129_WR();								/* WR=L->H					*/

	S6D0129_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6D0129_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D0129_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	S6D0129_WR();								/* WR=L->H					*/
	S6D0129_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	S6D0129_DATA = dat;							/* 16bit data 				*/
#endif
	S6D0129_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D0129_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6D0129_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		S6D0129_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D0129_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	S6D0129_wr_cmd(cmd);
	S6D0129_WR_SET();

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


#elif USE_S6D0129_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D0129_wr_cmd(uint8_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI(0);
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void S6D0129_wr_dat(uint16_t dat)
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
inline void S6D0129_wr_block(uint8_t *p,unsigned int cnt)
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
inline  uint16_t S6D0129_rd_cmd(uint8_t cmd)
{
#if 0 /* TFT1P0913-E does not have SDO Pin... */
	uint16_t val;
	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI16(cmd);
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_RD_DATA);
	temp = RecvSPI();							/* Dummy Read */
	temp = RecvSPI();							/* Upper Read */
	val  = RecvSPI();							/* Lower Read */

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
#else
	return 0x1121;
#endif

}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6D0129_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S6D0129_wr_cmd(0x46);				/* Horizontal Start,End ADDR */
	S6D0129_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	S6D0129_wr_cmd(0x48);				/* Vertical Start ADDR */
	S6D0129_wr_dat(OFS_RAW + y);
	S6D0129_wr_cmd(0x47);				/* Vertical End ADDR */
	S6D0129_wr_dat(OFS_RAW + height);

	S6D0129_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	S6D0129_wr_dat(OFS_COL + x);
	S6D0129_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	S6D0129_wr_dat(OFS_RAW + y);
	
	S6D0129_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D0129_clear(void)
{
	volatile uint32_t n;

	S6D0129_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D0129_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6D0129_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	S6D0129_reset();

	/* Check Device Code */
	devicetype = S6D0129_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0129)
	{
		/* Initialize S6D0129 Nemuisan Special  */
		/* Enable Oscillator */
		S6D0129_wr_cmd(0x00);
		S6D0129_wr_dat(0x0001);

		/* Set MCU Interface */
		S6D0129_wr_cmd(0x0B);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x0C);
		S6D0129_wr_dat(0x0000);
	
		/* Power Setting */
		S6D0129_wr_cmd(0x11);
		S6D0129_wr_dat(0x2E00);
		S6D0129_wr_cmd(0x14);
		S6D0129_wr_dat(0x1119);
		S6D0129_wr_cmd(0x10);
		S6D0129_wr_dat(0x1040);
		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0x0040);
		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0x0060);
		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0x0070);

		/* Display Draw Settings */
		S6D0129_wr_cmd(0x01);
		S6D0129_wr_dat(0x0927);
		S6D0129_wr_cmd(0x02);
		S6D0129_wr_dat(0x0700);

		S6D0129_wr_cmd(0x03);
		S6D0129_wr_dat(0x1030);
		S6D0129_wr_cmd(0x07);
		S6D0129_wr_dat(0x0004);
		S6D0129_wr_cmd(0x08);
		S6D0129_wr_dat(0x0505);
		S6D0129_wr_cmd(0x09);
		S6D0129_wr_dat(0x0000);

		/* GAMMA Settings */
		S6D0129_wr_cmd(0x30);
		S6D0129_wr_dat(0x0404);
		S6D0129_wr_cmd(0x31);
		S6D0129_wr_dat(0x0006);
		S6D0129_wr_cmd(0x32);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x33);
		S6D0129_wr_dat(0x0202);
		S6D0129_wr_cmd(0x34);
		S6D0129_wr_dat(0x0707);
		S6D0129_wr_cmd(0x35);
		S6D0129_wr_dat(0x0107);
		S6D0129_wr_cmd(0x36);
		S6D0129_wr_dat(0x0303);
		S6D0129_wr_cmd(0x37);
		S6D0129_wr_dat(0x0202);
		S6D0129_wr_cmd(0x38);
		S6D0129_wr_dat(0x1100);
		S6D0129_wr_cmd(0x39);
		S6D0129_wr_dat(0x1100);

		/* Coordination Control setting */  
		S6D0129_wr_cmd(0x40);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x41);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x42);
		S6D0129_wr_dat(0x013f);
		S6D0129_wr_cmd(0x43);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x44);
		S6D0129_wr_dat(0x00ef);
		S6D0129_wr_cmd(0x45);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x46);
		S6D0129_wr_dat(0xef00);
		S6D0129_wr_cmd(0x47);
		S6D0129_wr_dat(0x013f);
		S6D0129_wr_cmd(0x48);
		S6D0129_wr_dat(0x0000);

		S6D0129_wr_cmd(0x20);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x21);
		S6D0129_wr_dat(0x0000);

		/* Display On */
		S6D0129_wr_cmd(0x07);
		S6D0129_wr_dat(0x0015);
		S6D0129_wr_cmd(0x07);
		S6D0129_wr_dat(0x0017);
	}

	else if(devicetype == 0x1121)
	{
		/* Initialize S6D1121 */
		S6D0129_wr_cmd(0x11);
		S6D0129_wr_dat(0x2004);
		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0xCC00);
		S6D0129_wr_cmd(0x15);
		S6D0129_wr_dat(0x2600);	
		S6D0129_wr_cmd(0x14);
		S6D0129_wr_dat(0x252A);	
		S6D0129_wr_cmd(0x12);
		S6D0129_wr_dat(0x0033);
		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0xCC04);
		_delay_ms(1);
 
		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0xCC06);
		_delay_ms(1);
 
		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0xCC4F);
		_delay_ms(1);

		S6D0129_wr_cmd(0x13);
		S6D0129_wr_dat(0x674F);
		S6D0129_wr_cmd(0x11);
		S6D0129_wr_dat(0x2003);
		_delay_ms(1);

		S6D0129_wr_cmd(0x30);
		S6D0129_wr_dat(0x2609);
		S6D0129_wr_cmd(0x31);
		S6D0129_wr_dat(0x242C);
		S6D0129_wr_cmd(0x32);
		S6D0129_wr_dat(0x1F23);
		S6D0129_wr_cmd(0x33);
		S6D0129_wr_dat(0x2425);
		S6D0129_wr_cmd(0x34);
		S6D0129_wr_dat(0x2226);
		S6D0129_wr_cmd(0x35);
		S6D0129_wr_dat(0x2523);
		S6D0129_wr_cmd(0x36);
		S6D0129_wr_dat(0x1C1A);
		S6D0129_wr_cmd(0x37);
		S6D0129_wr_dat(0x131D);
		S6D0129_wr_cmd(0x38);
		S6D0129_wr_dat(0x0B11);
		S6D0129_wr_cmd(0x39);
		S6D0129_wr_dat(0x1210);
		S6D0129_wr_cmd(0x3A);
		S6D0129_wr_dat(0x1315);
		S6D0129_wr_cmd(0x3B);
		S6D0129_wr_dat(0x3619);
		S6D0129_wr_cmd(0x3C);
		S6D0129_wr_dat(0x0D00);
		S6D0129_wr_cmd(0x3D);
		S6D0129_wr_dat(0x000D);
		S6D0129_wr_cmd(0x16);
		S6D0129_wr_dat(0x0007);
		S6D0129_wr_cmd(0x02);
		S6D0129_wr_dat(0x0013);
		S6D0129_wr_cmd(0x03);
		S6D0129_wr_dat(0x0003);
		S6D0129_wr_cmd(0x01);
		S6D0129_wr_dat(0x0127);
		_delay_ms(1); 
		S6D0129_wr_cmd(0x08);
		S6D0129_wr_dat(0x0303);
		S6D0129_wr_cmd(0x0A);
		S6D0129_wr_dat(0x000B);
		S6D0129_wr_cmd(0x0B);
		S6D0129_wr_dat(0x0003);
		S6D0129_wr_cmd(0x0C);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x41);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x50);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x60);
		S6D0129_wr_dat(0x0005);
		S6D0129_wr_cmd(0x70);
		S6D0129_wr_dat(0x000B);
		S6D0129_wr_cmd(0x71);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x78);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x7A);
		S6D0129_wr_dat(0x0000);
		S6D0129_wr_cmd(0x79);
		S6D0129_wr_dat(0x0007);
		S6D0129_wr_cmd(0x07);
		S6D0129_wr_dat(0x0051);
		_delay_ms(1);
 	
		S6D0129_wr_cmd(0x07);
		S6D0129_wr_dat(0x0053);
		S6D0129_wr_cmd(0x79);
		S6D0129_wr_dat(0x0000);
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6D0129_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6D0129_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		S6D0129_wr_dat(COL_RED);
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
int S6D0129_draw_bmp(const uint8_t* ptr){

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
	S6D0129_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		S6D0129_rect(xs, xe, m, m);
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
			S6D0129_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
