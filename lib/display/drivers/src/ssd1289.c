/********************************************************************************/
/*!
	@file			ssd1289.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2013.01.02
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KFM597A01-1A		  		(SSD1289)	16bit mode only.	@n
					 -FP-SQV28002A00		  	(SSD1297)	8bit mode only.		@n
					 -FPC-Y80216		  		(SSD1298)	16bit mode only.

    @section HISTORY
		2010.10.01	V1.00	Stable Release.
		2010.12.31	V2.00	Cleanup SourceCode.
		2011.03.10	V3.00	C++ Ready.
		2011.10.25	V4.00	Added DMA TransactionSupport.
		2011.12.23	V5.00	Optimize Some Codes.
		2013.01.02	V6.00	Added SSD1297,SSD1298 Devices.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "SSD1289.h"
/* check header file version for fool proof */
#if __SSD1289_H != 0x0600
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
/* Pointer to the Write GGRAM Function */
void (*SSD1289_wr_gram)(uint16_t gram);

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
inline void SSD1289_reset(void)
{
	SSD1289_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	SSD1289_RD_SET();
	SSD1289_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	SSD1289_RES_CLR();							/* RES=L, CS=L   			*/
	SSD1289_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	SSD1289_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD1289_wr_cmd(uint8_t cmd)
{
	SSD1289_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SSD1289_CMD = 0;
	SSD1289_WR();
#endif

	SSD1289_CMD = cmd;							/* cmd						*/
	SSD1289_WR();								/* WR=L->H					*/
	
	SSD1289_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
void SSD1289_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SSD1289_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	SSD1289_WR();								/* WR=L->H					*/
	SSD1289_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	SSD1289_DATA = dat;							/* 16bit data 				*/
#endif
	SSD1289_WR();								/* WR=L->H					*/
}
/* SSD1297 8-Bit Interface WorkAround */
void SSD1297_wr_gram(uint16_t gram)
{
	SSD1289_DATA = ((gram>>11)<<3);				/* B */
	SSD1289_WR();
	SSD1289_DATA = ((gram>>5)<<2);				/* G */
	SSD1289_WR();	
	SSD1289_DATA = (gram<<3);					/* R */
	SSD1289_WR();
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SSD1289_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SSD1289_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		SSD1289_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1289_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	SSD1289_wr_cmd(0x45);				/* Horizontal RAM Start ADDR */
	SSD1289_wr_dat(OFS_RAW + y);
	SSD1289_wr_cmd(0x46);				/* Horizontal RAM End ADDR */
	SSD1289_wr_dat(OFS_RAW + height);

	SSD1289_wr_cmd(0x44);				/* Vertical Start,End ADDR */
	SSD1289_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	SSD1289_wr_cmd(0x4F);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	SSD1289_wr_dat(OFS_RAW + y);
	SSD1289_wr_cmd(0x4E);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	SSD1289_wr_dat(OFS_COL + x);

	SSD1289_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1289_clear(void)
{
	volatile uint32_t n;

	SSD1289_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1289_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SSD1289_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	SSD1289_wr_cmd(cmd);
	SSD1289_WR_SET();

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
void SSD1289_init (void)
{
	uint16_t devicetype;
	/* Set WriteGGRAM Function */
	SSD1289_wr_gram = SSD1289_wr_dat;
	
	Display_IoInit_If();

	SSD1289_reset();

	/* Check Device Code */
	devicetype = SSD1289_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8989)
	{
		/* Initialize SSD1289 */
		SSD1289_wr_cmd(0x00);
		SSD1289_wr_dat(0x0001);   /* oscillator on */
		_delay_ms(10); 

		SSD1289_wr_cmd(0x03);
		SSD1289_wr_dat(0xaeae);  /* //0xA2A0//0xeeee */
		SSD1289_wr_cmd(0x0c);
		SSD1289_wr_dat(0x0000);  /* 0x0002 */
		SSD1289_wr_cmd(0x0d);
		SSD1289_wr_dat(0x000F);   
		SSD1289_wr_cmd(0x0e);
		SSD1289_wr_dat(0x3200);  /* 0x3600 */
		_delay_ms(10); 

		SSD1289_wr_cmd(0x1e);
		SSD1289_wr_dat(0x00Ad);  /* 0x00A5 */
		SSD1289_wr_cmd(0x01);
		SSD1289_wr_dat(0x2B3F); 
		SSD1289_wr_cmd(0x02);
		SSD1289_wr_dat(0x0600); 
		SSD1289_wr_cmd(0x10);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x11);
		SSD1289_wr_dat(0x6830);

		/* add 20070705 */
		SSD1289_wr_cmd(0x05);
		SSD1289_wr_dat(0x0000);      
		SSD1289_wr_cmd(0x06);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x07);
		SSD1289_wr_dat(0x0233); 
		/* add 20070705 */

		SSD1289_wr_cmd(0x0b);
		SSD1289_wr_dat(0x0030);  /* 0x5300 */
		SSD1289_wr_cmd(0x0f);
		SSD1289_wr_dat(0x0000); 
		_delay_ms(100);
	
		/* add 20070705 */
		SSD1289_wr_cmd(0x41);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x42);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x44);
		SSD1289_wr_dat(0xef00); 
		SSD1289_wr_cmd(0x45);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x46);
		SSD1289_wr_dat(0x013f); 
		SSD1289_wr_cmd(0x48);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x49);
		SSD1289_wr_dat(0x013f); 
		SSD1289_wr_cmd(0x4a);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x4b);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x4e);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x4f);
		SSD1289_wr_dat(0x0000); 

		/* optimize for video display */
		SSD1289_wr_cmd(0x28);
		SSD1289_wr_dat(0x0006); 
		SSD1289_wr_cmd(0x2f);
		SSD1289_wr_dat(0x12be);	/*0x12ae */
		SSD1289_wr_cmd(0x12);
		SSD1289_wr_dat(0x6ceb);	/*optimize for video */
		_delay_ms(10); 
		SSD1289_wr_cmd(0x25);
		SSD1289_wr_dat(0xe000);	 
		/* add 20070705 */

		SSD1289_wr_cmd(0x30);
		SSD1289_wr_dat(0x0707);
		SSD1289_wr_cmd(0x31);
		SSD1289_wr_dat(0x0204);
		SSD1289_wr_cmd(0x32);
		SSD1289_wr_dat(0x0204);
		SSD1289_wr_cmd(0x33);
		SSD1289_wr_dat(0x0502);
		SSD1289_wr_cmd(0x34);
		SSD1289_wr_dat(0x0507);
		SSD1289_wr_cmd(0x35);
		SSD1289_wr_dat(0x0204);
		SSD1289_wr_cmd(0x36);
		SSD1289_wr_dat(0x0204);
		SSD1289_wr_cmd(0x37);
		SSD1289_wr_dat(0x0502);
		SSD1289_wr_cmd(0x3A);
		SSD1289_wr_dat(0x0302);
		SSD1289_wr_cmd(0x3B);
		SSD1289_wr_dat(0x0504);
		_delay_ms(20); 
/*
		SSD1289_wr_cmd(0x23);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x24);
		SSD1289_wr_dat(0x0000); 
		 _delay_ms(10); 
		SSD1289_wr_cmd(0x07);
		SSD1289_wr_dat(0x0233); 
		_delay_ms(10); 
*/
	}
	
	else if((devicetype == 0x9999) || (devicetype == 0x9797))
	{
		/* Initialize SSD1297 */
		SSD1289_wr_cmd(0xE5);
		SSD1289_wr_dat(0x8000);   
		SSD1289_wr_cmd(0x28);
		SSD1289_wr_dat(0x0006);		/* enable test command */  
		SSD1289_wr_cmd(0x07);
		SSD1289_wr_dat(0x0021);  
		SSD1289_wr_cmd(0x00);
		SSD1289_wr_dat(0x0001);		/* Start internal OSC */   
		_delay_ms(5);  
  
		SSD1289_wr_cmd(0x07);
		SSD1289_wr_dat(0x0023);  
		SSD1289_wr_cmd(0x10);
		SSD1289_wr_dat(0x0000);		/* Sleep mode off */ 
		_delay_ms(5);  
  
		SSD1289_wr_cmd(0x07);
		SSD1289_wr_dat(0x0033);		/* Display on */   
		SSD1289_wr_cmd(0x01);
		SSD1289_wr_dat(0x2b3f);		/* Driver output control */       
  
		SSD1289_wr_cmd(0x02);
		SSD1289_wr_dat(0x0600);		/* Set to line inversion */    
  
		SSD1289_wr_cmd(0x11);
	#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
		SSD1289_wr_dat(0x4030);		/* Entry mode setup */ 
	#else
		SSD1289_wr_dat(0x6030);		/* Entry mode setup */ 
	#endif
	
		SSD1289_wr_cmd(0x03);
		SSD1289_wr_dat(0xa6a8);		/* Step-up factor/cycle setting */   
		SSD1289_wr_cmd(0x0f);
		SSD1289_wr_dat(0x0000);		/* Gate scan position start at G0 */    
  
		SSD1289_wr_cmd(0x0e);
		SSD1289_wr_dat(0x3100);		/* Set alternating amplitude of VCOM */    
		SSD1289_wr_cmd(0x1e);
		SSD1289_wr_dat(0x009f);		/* Set VcomH voltage */      
  
		SSD1289_wr_cmd(0x0C);
		SSD1289_wr_dat(0x0005);		/* Adjust VCIX2 output voltage */   
  
		SSD1289_wr_cmd(0x0D);
		SSD1289_wr_dat(0x0005);		/* Set amplitude magnification of VLCD63 */   
  
		SSD1289_wr_cmd(0x0b);
		SSD1289_wr_dat(0x5308);		/* Frame cycle control */   
  
		/* Special command */   
		SSD1289_wr_cmd(0x25);
		SSD1289_wr_dat(0xe000);		/* Frame freq control */    
		SSD1289_wr_cmd(0x3f);
		SSD1289_wr_dat(0xbb84);		/* System setting */    
		SSD1289_wr_cmd(0x27);
		SSD1289_wr_dat(0x0567);		/* Internal Vcomh/Vcoml timing */      
		SSD1289_wr_cmd(0x20);
		SSD1289_wr_dat(0x316c);		/* Internal VCOM strength */      
  
		/* Adjust the Gamma Curve */   
		SSD1289_wr_cmd(0x3a);
		SSD1289_wr_dat(0x1000);   
		SSD1289_wr_cmd(0x3b);
		SSD1289_wr_dat(0x0008);   
		SSD1289_wr_cmd(0x30);
		SSD1289_wr_dat(0x0007);   
		SSD1289_wr_cmd(0x31);
		SSD1289_wr_dat(0x0003);  
		SSD1289_wr_cmd(0x32);
		SSD1289_wr_dat(0x0300);   
		SSD1289_wr_cmd(0x33);
		SSD1289_wr_dat(0x0101);   
		SSD1289_wr_cmd(0x34);
		SSD1289_wr_dat(0x0707);   
		SSD1289_wr_cmd(0x35);
		SSD1289_wr_dat(0x0002);   
		SSD1289_wr_cmd(0x36);
		SSD1289_wr_dat(0x0503);   
		SSD1289_wr_cmd(0x37);
		SSD1289_wr_dat(0x0202);   
		SSD1289_wr_cmd(0x3a);
		SSD1289_wr_dat(0x1000);   
		SSD1289_wr_cmd(0x3b);
		SSD1289_wr_dat(0x0008);   
   
		SSD1289_wr_cmd(0x48);
		SSD1289_wr_dat(0x0000);  
		SSD1289_wr_cmd(0x49);
		SSD1289_wr_dat(0x013f);  
		SSD1289_wr_cmd(0x4a);
		SSD1289_wr_dat(0xef00);  
		SSD1289_wr_cmd(0x4b);
		SSD1289_wr_dat(0x013f);

	#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
		/* Set WriteGGRAM Function */
		SSD1289_wr_gram = SSD1297_wr_gram;
	#endif
	}

	else if(devicetype == 0x8999)
	{
		/* Initialize SSD1298 */
		/* Start Initial Sequence */
		SSD1289_wr_cmd(0x00);
		SSD1289_wr_dat(0x0001);			/* Start internal OSC. */
		SSD1289_wr_cmd(0x01);
		SSD1289_wr_dat(0x3B3F);			/* Driver output control, RL=0;REV=1;GD=1;BGR=1;SM=0;TB=1 */ 
		SSD1289_wr_cmd(0x02);
		SSD1289_wr_dat(0x0600);			/* set 1 line inversion */ 

		/* Power control setup */
		SSD1289_wr_cmd(0x0C);
		SSD1289_wr_dat(0x0007);			/* Adjust VCIX2 output voltage */ 
		SSD1289_wr_cmd(0x0D);
		SSD1289_wr_dat(0x0006);			/* Set amplitude magnification of VLCD63 */ 
		SSD1289_wr_cmd(0x0E);
		SSD1289_wr_dat(0x3200);			/* Set alternating amplitude of VCOM */ 
		SSD1289_wr_cmd(0x1E);
		SSD1289_wr_dat(0x00BB);			/* Set VcomH voltage */ 
		SSD1289_wr_cmd(0x03);
		SSD1289_wr_dat(0x6A64);			/* Step-up factor/cycle setting */ 

		/* Turn On display */
		SSD1289_wr_cmd(0x10);
		SSD1289_wr_dat(0x0000);			/* Sleep mode off. */ 
		_delay_ms(30);					/* Wait 30mS */  
		SSD1289_wr_cmd(0x11);
		SSD1289_wr_dat(0x6030);			/* Entry mode setup */ 
		SSD1289_wr_cmd(0x07);
		SSD1289_wr_dat(0x0033);			/* Display ON */

		/* LCD driver AC setting */
		SSD1289_wr_cmd(0x25);
		SSD1289_wr_dat(0xE000);			/* Frame freq control, 65Hz */ 
		SSD1289_wr_cmd(0x0B);
		SSD1289_wr_dat(0x5308);			/* Frame cycle control, POR setting */
 
		/* RAM position control */
		SSD1289_wr_cmd(0x0F);
		SSD1289_wr_dat(0x0000);			/* Gate scan position start at G0. */ 
		SSD1289_wr_cmd(0x44);
		SSD1289_wr_dat(0xEF00);			/* Horizontal RAM address position */ 
		SSD1289_wr_cmd(0x45);
		SSD1289_wr_dat(0x0000);			/* Vertical RAM address start position */ 
		SSD1289_wr_cmd(0x46);
		SSD1289_wr_dat(0x013F);			/* Vertical RAM address end position */ 

		/* Adjust the Gamma Curve */
		SSD1289_wr_cmd(0x30);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x31);
		SSD1289_wr_dat(0x0706); 
		SSD1289_wr_cmd(0x32);
		SSD1289_wr_dat(0x0206); 
		SSD1289_wr_cmd(0x33);
		SSD1289_wr_dat(0x0300); 
		SSD1289_wr_cmd(0x34);
		SSD1289_wr_dat(0x0002); 
		SSD1289_wr_cmd(0x35);
		SSD1289_wr_dat(0x0000); 
		SSD1289_wr_cmd(0x36);
		SSD1289_wr_dat(0x0707); 
		SSD1289_wr_cmd(0x37);
		SSD1289_wr_dat(0x0200); 
		SSD1289_wr_cmd(0x3A);
		SSD1289_wr_dat(0x0908); 
		SSD1289_wr_cmd(0x3B);
		SSD1289_wr_dat(0x0F0D); 
	
		/* Special command */
		SSD1289_wr_cmd(0x28);
		SSD1289_wr_dat(0x0006);			/* Enable test command */ 
		SSD1289_wr_cmd(0x2F);
		SSD1289_wr_dat(0x12EB);			/* RAM speed tuning */ 
		SSD1289_wr_cmd(0x26);
		SSD1289_wr_dat(0x7000);			/* Internal Bandgap strength */ 
		SSD1289_wr_cmd(0x20);
		SSD1289_wr_dat(0xB0E3);			/* Internal Vcom strength */ 
		SSD1289_wr_cmd(0x27);
		SSD1289_wr_dat(0x0044);			/* Internal Vcomh/VcomL timing */ 
		SSD1289_wr_cmd(0x2E);
		SSD1289_wr_dat(0x7E45);			/* VCOM charge sharing time */

	}

	else { for(;;);} /* Invalid Device Code!! */

	SSD1289_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1289_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		SSD1289_wr_gram(COL_RED);
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
int SSD1289_draw_bmp(const uint8_t* ptr){

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
	SSD1289_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SSD1289_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SSD1289_wr_gram(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
