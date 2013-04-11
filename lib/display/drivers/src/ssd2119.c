/********************************************************************************/
/*!
	@file			ssd2119.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CFAF320240F-T-TS TFT module(8/16bit,spi mode).

    @section HISTORY
		2010.07.10	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Cleanup SourceCode.
		2011.03.10	V4.00	C++ Ready.
		2011.10.25	V5.00	Added DMA TransactionSupport.
							Added 4-Wire SPI Transfer Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd2119.h"
/* check header file version for fool proof */
#if __SSD2119_H != 0x0500
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
inline void SSD2119_reset(void)
{
#ifdef USE_SSD2119_TFT
	SSD2119_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	SSD2119_RD_SET();
	SSD2119_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	SSD2119_RES_CLR();							/* RES=L, CS=L   			*/
	SSD2119_CS_CLR();

#elif  USE_SSD2119_SPI_TFT
	SSD2119_RES_SET();							/* RES=H, CS=H				*/
	SSD2119_CS_SET();
	SSD2119_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	SSD2119_RES_CLR();							/* RES=L		   			*/
#endif

	_delay_ms(10);								/* wait 10ms     			*/
	
	SSD2119_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_SSD2119_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD2119_wr_cmd(uint8_t cmd)
{
	SSD2119_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SSD2119_CMD = 0;
	SSD2119_WR();
#endif

	SSD2119_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	SSD2119_WR();								/* WR=L->H					*/
	
	SSD2119_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void SSD2119_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SSD2119_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	SSD2119_WR();								/* WR=L->H					*/
	SSD2119_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	SSD2119_DATA = dat;							/* 16bit data 				*/
#endif
	SSD2119_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SSD2119_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SSD2119_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		SSD2119_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SSD2119_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	SSD2119_wr_cmd(cmd);
	SSD2119_WR_SET();

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

#elif USE_SSD2119_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD2119_wr_cmd(uint8_t cmd)
{
	SSD2119_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD2119_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void SSD2119_wr_dat(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SSD2119_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */

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
inline  uint16_t SSD2119_rd_cmd(uint16_t cmd)
{
	uint16_t val;
	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(cmd);
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	
	SSD2119_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	temp = RecvSPI();							/* Dummy Read */
	temp = RecvSPI();							/* Upper Read */
	val  = RecvSPI();							/* Lower Read */

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD2119_DC_SET();							/* DC=L		     */
	
	return 0x9919;								/* Read Function was NOT implemented in SPI-MODE! */
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD2119_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	SSD2119_wr_cmd(0x45);				/* Horizontal RAM Start ADDR */
	SSD2119_wr_dat(OFS_COL + x);
	SSD2119_wr_cmd(0x46);				/* Horizontal RAM End ADDR */
	SSD2119_wr_dat(OFS_COL + width);

	SSD2119_wr_cmd(0x44);				/* Vertical Start,End ADDR */
	SSD2119_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	SSD2119_wr_cmd(0x4f);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	SSD2119_wr_dat(OFS_RAW + y);
	SSD2119_wr_cmd(0x4e);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	SSD2119_wr_dat(OFS_COL + x);

	SSD2119_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD2119_clear(void)
{
	volatile uint32_t n;

	SSD2119_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD2119_wr_dat(COL_BLACK);
	} while (--n);

}



/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SSD2119_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	SSD2119_reset();

	/* Check Device Code */
	devicetype = SSD2119_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if((devicetype == 0x9919) || (devicetype == 0x99))
	{
		/* Initialize SSD2119 */
		/* ----------- POWER ON & RESET DISPLAY OFF---------- */
		SSD2119_wr_cmd(0x28);
		SSD2119_wr_dat(0x0006);

		SSD2119_wr_cmd(0x00);
		SSD2119_wr_dat(0x0001);

		SSD2119_wr_cmd(0x10);
		SSD2119_wr_dat(0x0000);

		SSD2119_wr_cmd(0x01);
		SSD2119_wr_dat(0x72ef);

		SSD2119_wr_cmd(0x02);
		SSD2119_wr_dat(0x0600);

		SSD2119_wr_cmd(0x03);
		SSD2119_wr_dat(0x6a38);
		
		SSD2119_wr_cmd(0x11);
		//SSD2119_wr_dat((1<<13)|(1<<12)|(1<<11)|(1<<6)|(1<<5)|(1<<4)|(0<<2));
		SSD2119_wr_dat(0x6834); /*SSD2119_wr_dat(0x6874); */
 
		SSD2119_wr_cmd(0x0f);						/* Gate Scan Position */ 
		SSD2119_wr_dat(0x0000); 

		SSD2119_wr_cmd(0x0b);						/* Frame cycle control */
		SSD2119_wr_dat(0x5308); 

		SSD2119_wr_cmd(0x0c);
		SSD2119_wr_dat(0x0003);

		SSD2119_wr_cmd(0x0d);
		SSD2119_wr_dat(0x000a);

		SSD2119_wr_cmd(0x0e);
		SSD2119_wr_dat(0x2e00);

		SSD2119_wr_cmd(0x1e);
		SSD2119_wr_dat(0x00be);

		SSD2119_wr_cmd(0x25);
		SSD2119_wr_dat(0x8000);

		SSD2119_wr_cmd(0x26);
		SSD2119_wr_dat(0x7800);

		SSD2119_wr_cmd(0x27);
		SSD2119_wr_dat(0x0078);

		SSD2119_wr_cmd(0x4e);
		SSD2119_wr_dat(0x0000);

		SSD2119_wr_cmd(0x4f);
		SSD2119_wr_dat(0x0000);

		SSD2119_wr_cmd(0x12);
		SSD2119_wr_dat(0x08d9);

		/* ----------- Adjust the Gamma Curve ---------- */
		SSD2119_wr_cmd(0x30);
		SSD2119_wr_dat(0x0000);		/*0007*/

		SSD2119_wr_cmd(0x31);
		SSD2119_wr_dat(0x0104);		/*0203*/

		SSD2119_wr_cmd(0x32);
		SSD2119_wr_dat(0x0100);		/*0001*/

		SSD2119_wr_cmd(0x33);
		SSD2119_wr_dat(0x0305);	  	/*0007*/

		SSD2119_wr_cmd(0x34);
		SSD2119_wr_dat(0x0505);	  	/*0007*/
		
		SSD2119_wr_cmd(0x35);
		SSD2119_wr_dat(0x0305);		/*0407*/

		SSD2119_wr_cmd(0x36);
		SSD2119_wr_dat(0x0707);		/*0407*/

		SSD2119_wr_cmd(0x37);
		SSD2119_wr_dat(0x0300);		/*0607*/

		SSD2119_wr_cmd(0x3a);
		SSD2119_wr_dat(0x1200);		/*0106*/

		SSD2119_wr_cmd(0x3b);
		SSD2119_wr_dat(0x0800);		 

		SSD2119_wr_cmd(0x07);
		SSD2119_wr_dat(0x0033);

	}

	else { for(;;);} /* Invalid Device Code!! */

	SSD2119_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD2119_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD2119_wr_dat(COL_RED);
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
int SSD2119_draw_bmp(const uint8_t* ptr){

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
	SSD2119_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SSD2119_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SSD2119_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
