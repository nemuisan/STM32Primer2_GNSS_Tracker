/********************************************************************************/
/*!
	@file			lgdp452x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2012.08.27
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -LSG020-117NA0-FPC 		(LGDP4524)	8/16bit mode.		@n
					 -LH220Q01                  (LGDP4522)  8bit mode.       	@n
					 -S95311                  	(HX8340A)   8bit mode.       	@n

    @section HISTORY
		2011.11.30	V1.00	Renewed From LGDP452x driver.
		2012.08.27  V2.00	Added HX8340A driver.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lgdp452x.h"
/* check header file version for fool proof */
#if __LGDP452X_H != 0x0200
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Abstract Layer _delay_ms Settings.
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
inline void LGDP452x_reset(void)
{
	LGDP452x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	LGDP452x_RD_SET();
	LGDP452x_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	LGDP452x_RES_CLR();							/* RES=L, CS=L   			*/
	LGDP452x_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	LGDP452x_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void LGDP452x_wr_cmd(uint16_t cmd)
{
	LGDP452x_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	LGDP452x_CMD = 0;
	LGDP452x_WR();
#endif

	LGDP452x_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	LGDP452x_WR();								/* WR=L->H					*/

	LGDP452x_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void LGDP452x_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	LGDP452x_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	LGDP452x_WR();								/* WR=L->H					*/
	LGDP452x_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	LGDP452x_DATA = dat;						/* 16bit data 				*/
#endif
	LGDP452x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void LGDP452x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		LGDP452x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		LGDP452x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void LGDP452x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	LGDP452x_wr_cmd(0x44);				/* Horizontal Start,End ADDR */
	LGDP452x_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	LGDP452x_wr_cmd(0x45);				/* Vertical Start,End ADDR */
	LGDP452x_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	LGDP452x_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	LGDP452x_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	LGDP452x_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void LGDP452x_clear(void)
{
	volatile uint32_t n;

	LGDP452x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		LGDP452x_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t LGDP452x_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	LGDP452x_wr_cmd(cmd);
	LGDP452x_WR_SET();
	
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
void LGDP452x_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	LGDP452x_reset();

	/* Check Device Code */
	devicetype = LGDP452x_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x4522)
	{
		/* Initialize LGDP4522 */
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x12);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x13);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x14);
		LGDP452x_wr_dat(0x5331);
		LGDP452x_wr_cmd(0x11);
		LGDP452x_wr_dat(0x0101);
		LGDP452x_wr_cmd(0x12);
		LGDP452x_wr_dat(0x000a);
		LGDP452x_wr_cmd(0x13);
		LGDP452x_wr_dat(0x0045);
		LGDP452x_wr_cmd(0x10);
		LGDP452x_wr_dat(0x0164);
		LGDP452x_wr_cmd(0x12);
		LGDP452x_wr_dat(0x001a);
		LGDP452x_wr_cmd(0x13);
		LGDP452x_wr_dat(0x2d45);
		LGDP452x_wr_cmd(0x10);
		LGDP452x_wr_dat(0x0160);
		LGDP452x_wr_cmd(0x01);
		LGDP452x_wr_dat(0x011b);
		LGDP452x_wr_cmd(0x02);
		LGDP452x_wr_dat(0x0700);
		LGDP452x_wr_cmd(0x03);
		LGDP452x_wr_dat(0x1030);

		LGDP452x_wr_cmd(0x08);
		LGDP452x_wr_dat(0x0504);
		LGDP452x_wr_cmd(0x09);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x0b);
		LGDP452x_wr_dat(0x0c30);

		LGDP452x_wr_cmd(0x21);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x30);
		LGDP452x_wr_dat(0x0606);
		LGDP452x_wr_cmd(0x31);
		LGDP452x_wr_dat(0x0606);
		LGDP452x_wr_cmd(0x32);
		LGDP452x_wr_dat(0x0606);
		LGDP452x_wr_cmd(0x33);
		LGDP452x_wr_dat(0x0803);
		LGDP452x_wr_cmd(0x34);
		LGDP452x_wr_dat(0x0202);
		LGDP452x_wr_cmd(0x35);
		LGDP452x_wr_dat(0x0404);
		LGDP452x_wr_cmd(0x36);
		LGDP452x_wr_dat(0x0404);
		LGDP452x_wr_cmd(0x37);
		LGDP452x_wr_dat(0x0404);
		LGDP452x_wr_cmd(0x38);
		LGDP452x_wr_dat(0x0402);
		LGDP452x_wr_cmd(0x39);
		LGDP452x_wr_dat(0x100c);
		LGDP452x_wr_cmd(0x44);
		LGDP452x_wr_dat(0xaf00);
		LGDP452x_wr_cmd(0x45);
		LGDP452x_wr_dat(0xdb00);
		LGDP452x_wr_cmd(0x71);
		LGDP452x_wr_dat(0x0040);
		LGDP452x_wr_cmd(0x10);
		LGDP452x_wr_dat(0x6060);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0005);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0025);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0027);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0037);
		LGDP452x_wr_cmd(0x21);
		LGDP452x_wr_dat(0x0000);

	}

	else if(devicetype == 0x4524)
	{
		/* Initialize LGDP4524 */
		LGDP452x_wr_cmd(0x07);		 	/* display control:GON=DTE=D1-0=0 */
		LGDP452x_wr_dat(0x0000);
				  
		LGDP452x_wr_cmd(0x12); 			/* power control 3:pon=0 */
		LGDP452x_wr_dat(0x0000); 		 		 
		
		LGDP452x_wr_cmd(0x13); 			/* power control 4:VCOMG=0 */
		LGDP452x_wr_dat(0x0000); 
		_delay_ms(60);

		LGDP452x_wr_cmd(0x00);		 	/* oscillation start */
		LGDP452x_wr_dat(0x0001);
		_delay_ms(60);

	   /* power on */
		LGDP452x_wr_cmd(0x14); 			/* power control 5:VCOMG=0 */
		LGDP452x_wr_dat(0x0331);
		_delay_ms(60);	 		   		/* delay 30 ms */
		LGDP452x_wr_cmd(0x12); 			/* power control 3 */
		LGDP452x_wr_dat(0x000A); 
		LGDP452x_wr_cmd(0x11); 			/* power control 2 */
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x10); 			/* power control 1 */
		LGDP452x_wr_dat(0x0004);
		LGDP452x_wr_cmd(0x13); 			/* power control 4 */
		LGDP452x_wr_dat(0x0c2a);
		_delay_ms(60);		 

		LGDP452x_wr_cmd(0x10); 			/* power control 1 */
		LGDP452x_wr_dat(0x0254);		/* set bt */
		LGDP452x_wr_cmd(0x11); 			/* power control 2 */
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x12); 			/* power control 3 */
		LGDP452x_wr_dat(0x0019);		/* set vrh   */
		_delay_ms(60);					/* delay 30 ms */
		
		LGDP452x_wr_cmd(0x13); 			/* power control 4 */
		LGDP452x_wr_dat(0x2C37);		/* set vdv,vcm281f2b222d253027 2D2F 2C2F 2C38 */
		LGDP452x_wr_cmd(0x10); 			/* power control 1 */
		LGDP452x_wr_dat(0x0250);  		
		_delay_ms(120); 				/* delay 30 ms */ 
	    /* end power on */  

		LGDP452x_wr_cmd(0x60); 	 
		LGDP452x_wr_dat(0x2000);
		LGDP452x_wr_cmd(0x60); 	 
		LGDP452x_wr_dat(0x0000); 
		LGDP452x_wr_cmd(0x61); 	 
		LGDP452x_wr_dat(0x0002); 

		LGDP452x_wr_cmd(0x01); 			/* driver output control */ 
		LGDP452x_wr_dat(0x011B); 

		LGDP452x_wr_cmd(0x02); 			/* lcd drive AC control */
		LGDP452x_wr_dat(0x0700);
						 
		LGDP452x_wr_cmd(0x03); 			/* entry mode */
		LGDP452x_wr_dat(0x1030);  
		
	 
		LGDP452x_wr_cmd(0x09); 			/* display control */
		LGDP452x_wr_dat(0x0000);       
		
		LGDP452x_wr_cmd(0x0B); 			/* frame cycle adjustment */
		LGDP452x_wr_dat(0x5d2c);

		LGDP452x_wr_cmd(0x21); 			/* RAM address set */   
		LGDP452x_wr_dat(0x0000);

		LGDP452x_wr_cmd(0x30); 			/* Gamma control (1) */
		LGDP452x_wr_dat(0x0101);
		LGDP452x_wr_cmd(0x31); 			/* Gamma control (2) */
		LGDP452x_wr_dat(0x0704);
		LGDP452x_wr_cmd(0x32);			/* Gamma control (3) */
		LGDP452x_wr_dat(0x0707);
		LGDP452x_wr_cmd(0x33);  		/* Gamma control (4) */
		LGDP452x_wr_dat(0x0403);
		LGDP452x_wr_cmd(0x34); 			/* Gamma control (5) */
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x35); 			/* Gamma control (6) */
		LGDP452x_wr_dat(0x0400);
		LGDP452x_wr_cmd(0x36); 			/* Gamma control (7) */
		LGDP452x_wr_dat(0x0606);
		LGDP452x_wr_cmd(0x37); 			/* Gamma control (8) */
		LGDP452x_wr_dat(0x0304);
		LGDP452x_wr_cmd(0x38); 			/* Gamma control (9) */
		LGDP452x_wr_dat(0x070E);
		LGDP452x_wr_cmd(0x39); 			/* Gamma control (10) */
		LGDP452x_wr_dat(0x070E);
		LGDP452x_wr_cmd(0x3A); 			/* Gamma control (10) */
		LGDP452x_wr_dat(0x0101);
		LGDP452x_wr_cmd(0x3B); 			/* Gamma control (10) */
		LGDP452x_wr_dat(0x0101);
		LGDP452x_wr_cmd(0x3C); 			/* Gamma control (10) */
		LGDP452x_wr_dat(0x0101);
		LGDP452x_wr_cmd(0x3D); 			/* Gamma control (10) */
		LGDP452x_wr_dat(0x0101);
		LGDP452x_wr_cmd(0x3E); 			/* Gamma control (10) */
		LGDP452x_wr_dat(0x0001);
		LGDP452x_wr_cmd(0x3F); 			/* Gamma control (10) */
		LGDP452x_wr_dat(0x0001);
	
		LGDP452x_wr_cmd(0x42); 			/* power control 4:VCOMG=0 */
		LGDP452x_wr_dat(0xdb00);
		LGDP452x_wr_cmd(0x44); 			/* horizontal RAM address position */
		LGDP452x_wr_dat(0xAF00);
		LGDP452x_wr_cmd(0x45); 			/* vertical RAM address position */
		LGDP452x_wr_dat(0xDB00);
		LGDP452x_wr_cmd(0x71); 			/* test register */
		LGDP452x_wr_dat(0x0040);
		LGDP452x_wr_cmd(0x72); 			/* test register */
		LGDP452x_wr_dat(0x0002);
		_delay_ms(20); 					/* delay 20 ms */
	 
		/* display on */    
		LGDP452x_wr_cmd(0x10); 			/* power control 1 */
		LGDP452x_wr_dat(0x3250);		/* set sap */
		_delay_ms(60);
		LGDP452x_wr_cmd(0x07); 			/* Display control on */
		LGDP452x_wr_dat(0x0001);
		_delay_ms(30);					/* delay 30 ms */
		LGDP452x_wr_cmd(0x07); 			/* Display control on */
		LGDP452x_wr_dat(0x0021);
		_delay_ms(30);
		LGDP452x_wr_cmd(0x07); 			/* 	Display control on */
		LGDP452x_wr_dat(0x0023);
		_delay_ms(30);					/* delay 30 ms */
		LGDP452x_wr_cmd(0x07); 			/* 	Display control on */
		LGDP452x_wr_dat(0x0037);
		_delay_ms(60);					/* delay 30 ms */

	}
	
	else if(devicetype == 0x8340)
	{
		/* Initialize HX8340A */
		/* Fix waving issue */
		LGDP452x_wr_cmd(0x47);
		LGDP452x_wr_dat(0x0001);		/* enter test mode */
		LGDP452x_wr_cmd(0x67);
		LGDP452x_wr_dat(0x0001);		/* set RADJ[2:0], the default value is 001 */
		LGDP452x_wr_cmd(0x47);
		LGDP452x_wr_dat(0x0000);		/* leave test mode */
		
		/* Initialize */
		LGDP452x_wr_cmd(0x01);
		LGDP452x_wr_dat(0x011B);
		LGDP452x_wr_cmd(0x02);
		LGDP452x_wr_dat(0x0700);
		LGDP452x_wr_cmd(0x03);
		LGDP452x_wr_dat((1<<14)|(1<<13)|(1<<12)|(1<<5)|(1<<4)|(0<<3));/* LGDP452x_wr_dat(0x6020); */
		LGDP452x_wr_cmd(0x04);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x05);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0004);
		LGDP452x_wr_cmd(0x08);
		LGDP452x_wr_dat(0x0202);
		LGDP452x_wr_cmd(0x09);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x0B);
		LGDP452x_wr_dat(0x0004);
		LGDP452x_wr_cmd(0x0C);
		LGDP452x_wr_dat(0x0003);
		LGDP452x_wr_cmd(0x40);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x41);
		LGDP452x_wr_dat(0x00EF);
		LGDP452x_wr_cmd(0x42);
		LGDP452x_wr_dat(0xDB00);
		LGDP452x_wr_cmd(0x43);
		LGDP452x_wr_dat(0xDB00);
		LGDP452x_wr_cmd(0x44);
		LGDP452x_wr_dat(0xAF00);
		LGDP452x_wr_cmd(0x45);
		LGDP452x_wr_dat(0xDB00);
		_delay_ms(100);
		/* Power_Set */
		LGDP452x_wr_cmd(0x00);
		LGDP452x_wr_dat(0x0001);
		_delay_ms(10);
		LGDP452x_wr_cmd(0x10);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x11);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x12);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x13);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x11);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x13);
		LGDP452x_wr_dat(0x121D);
		LGDP452x_wr_cmd(0x12);
		LGDP452x_wr_dat(0x0008);
		LGDP452x_wr_cmd(0x10);
		LGDP452x_wr_dat(0x4040);
		_delay_ms(10);
		LGDP452x_wr_cmd(0x11);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x10);
		LGDP452x_wr_dat(0x0054);
		LGDP452x_wr_cmd(0x12);
		LGDP452x_wr_dat(0x0013);
		LGDP452x_wr_cmd(0x13);
		LGDP452x_wr_dat(0x320D);
		_delay_ms(100);
		/* Gamma_Set */
		LGDP452x_wr_cmd(0x30);
		LGDP452x_wr_dat(0x0100);
		LGDP452x_wr_cmd(0x31);
		LGDP452x_wr_dat(0x0503);
		LGDP452x_wr_cmd(0x32);
		LGDP452x_wr_dat(0x0104);
		LGDP452x_wr_cmd(0x33);
		LGDP452x_wr_dat(0x0301);
		LGDP452x_wr_cmd(0x34);
		LGDP452x_wr_dat(0x0307);
		LGDP452x_wr_cmd(0x35);
		LGDP452x_wr_dat(0x0202);
		LGDP452x_wr_cmd(0x36);
		LGDP452x_wr_dat(0x0706);
		LGDP452x_wr_cmd(0x37);
		LGDP452x_wr_dat(0x0304);
		LGDP452x_wr_cmd(0x38);
		LGDP452x_wr_dat(0x0000);
		LGDP452x_wr_cmd(0x39);
		LGDP452x_wr_dat(0x0000);
		/* Display_ON */
		LGDP452x_wr_cmd(0x10);
		LGDP452x_wr_dat(0x4040);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0005);
		_delay_ms(40);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0025);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0027);
		_delay_ms(40);
		LGDP452x_wr_cmd(0x07);
		LGDP452x_wr_dat(0x0037);

	}
	
	else { for(;;);} /* Invalid Device Code!! */

	LGDP452x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	LGDP452x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		LGDP452x_wr_dat(COL_RED);
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
int LGDP452x_draw_bmp(const uint8_t* ptr){

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
	LGDP452x_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		LGDP452x_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			/* 262k colour access */
			/* d2 = *p << 14;	  		 Blue  */
			/* d  = (*p++ >> 4);		 Blue  */
			/* d |= ((*p++>>2)<<4);		 Green */
			/* d |= ((*p++>>2)<<10);	 Red   */
			/* 262k +16M dither colour access */
			/* d2 = *p++ << 8;			 Blue  */
			/* d  = *p++;				 Green */
			/* d |= *p++ << 8;			 Red   */
			LGDP452x_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
