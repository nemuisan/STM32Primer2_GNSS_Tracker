/********************************************************************************/
/*!
	@file			hx8352x.c
    @version        1.00
    @date           2012.08.27
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -QD-ET3207ABT				(HX8352A)	16bit mode.			@n
					 -S95461C					(HX8352B)	16bit mode.

    @section HISTORY
		2012.08.27	V1.00	Revised From hx8352a.c

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8352x.h"
/* check header file version for fool proof */
#if __HX8352X_H != 0x0100
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
/* Pointer to the Rectangles */
void (*HX8352x_rect)(uint32_t x, uint32_t width, uint32_t y, uint32_t height);

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
inline void HX8352x_reset(void)
{
	HX8352x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8352x_RD_SET();
	HX8352x_WR_SET();
	_delay_ms(40);								/* wait 40ms     			*/

	HX8352x_RES_CLR();							/* RES=L, CS=L   			*/
	HX8352x_CS_CLR();
	_delay_ms(60);								/* wait 60ms     			*/
	
	HX8352x_RES_SET();						  	/* RES=H					*/
	_delay_ms(60);				    			/* wait 60ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8352x_wr_cmd(uint8_t cmd)
{
	HX8352x_DC_CLR();							/* DC=L						*/

	HX8352x_CMD = cmd;							/* cmd						*/
	HX8352x_WR();								/* WR=L->H					*/

	HX8352x_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8352x_wr_dat(uint8_t dat)
{
	HX8352x_DATA = dat;							/* data						*/
	HX8352x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8352x_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8352x_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	HX8352x_WR();								/* WR=L->H					*/
	HX8352x_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	HX8352x_DATA = gram;						/* 16bit data 				*/
#endif
	HX8352x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8352x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8352x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8352x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle HX8352A.
*/
/**************************************************************************/
void HX8352A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	HX8352x_wr_cmd(0x02);				/* Horizontal RAM Start ADDR2 */
	HX8352x_wr_dat((OFS_COL + x)>>8);
	HX8352x_wr_cmd(0x03);				/* Horizontal RAM Start ADDR1 */
	HX8352x_wr_dat(OFS_COL + x);
	HX8352x_wr_cmd(0x04);				/* Horizontal RAM End ADDR2 */
	HX8352x_wr_dat((OFS_COL + width)>>8);
	HX8352x_wr_cmd(0x05);				/* Horizontal RAM End ADDR1 */
	HX8352x_wr_dat(OFS_COL + width);
	HX8352x_wr_cmd(0x06);				/* Vertical RAM Start ADDR2 */
	HX8352x_wr_dat((OFS_RAW + y)>>8);
	HX8352x_wr_cmd(0x07);				/* Vertical RAM Start ADDR1 */
	HX8352x_wr_dat(OFS_RAW + y);
	HX8352x_wr_cmd(0x08);				/* Vertical RAM End ADDR2 */
	HX8352x_wr_dat((OFS_RAW + height)>>8);
	HX8352x_wr_cmd(0x09);				/* Vertical RAM End ADDR1 */
	HX8352x_wr_dat(OFS_RAW + height);

	HX8352x_wr_cmd(0x22);				/* Write Data to GRAM */
}

/**************************************************************************/
/*! 
    Set Rectangle HX8352B.
*/
/**************************************************************************/
void HX8352B_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	HX8352x_wr_cmd(0x02);				/* Horizontal RAM Start ADDR2 */
	HX8352x_wr_dat((OFS_COL + x)>>8);
	HX8352x_wr_cmd(0x03);				/* Horizontal RAM Start ADDR1 */
	HX8352x_wr_dat(OFS_COL + x);
	HX8352x_wr_cmd(0x04);				/* Horizontal RAM End ADDR2 */
	HX8352x_wr_dat((OFS_COL + width)>>8);
	HX8352x_wr_cmd(0x05);				/* Horizontal RAM End ADDR1 */
	HX8352x_wr_dat(OFS_COL + width);
	HX8352x_wr_cmd(0x06);				/* Vertical RAM Start ADDR2 */
	HX8352x_wr_dat((OFS_RAW + y)>>8);
	HX8352x_wr_cmd(0x07);				/* Vertical RAM Start ADDR1 */
	HX8352x_wr_dat(OFS_RAW + y);
	HX8352x_wr_cmd(0x08);				/* Vertical RAM End ADDR2 */
	HX8352x_wr_dat((OFS_RAW + height)>>8);
	HX8352x_wr_cmd(0x09);				/* Vertical RAM End ADDR1 */
	HX8352x_wr_dat(OFS_RAW + height);

	HX8352x_wr_cmd(0x80);				/* Horizontal RAM Start ADDR2 */
	HX8352x_wr_dat((OFS_COL + x)>>8);
	HX8352x_wr_cmd(0x81);				/* Horizontal RAM Start ADDR1 */
	HX8352x_wr_dat(OFS_COL + x);

	HX8352x_wr_cmd(0x82);				/* Vertical RAM Start ADDR2 */
	HX8352x_wr_dat((OFS_RAW + y)>>8);
	HX8352x_wr_cmd(0x83);				/* Vertical RAM Start ADDR1 */
	HX8352x_wr_dat(OFS_RAW + y);

	HX8352x_wr_cmd(0x22);				/* Write Data to GRAM */
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8352x_clear(void)
{
	volatile uint32_t n;

	HX8352x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8352x_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX8352x_rd_cmd(uint8_t cmd)
{
	uint16_t val;

	HX8352x_wr_cmd(cmd);

	HX8352x_WR_SET();

    ReadLCDData(val);		/* Dummy Read (MUST need for HX8352B) */
    ReadLCDData(val);		/* Read ID */

	val &= 0x00FF;
	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8352x_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HX8352x_reset();
	
	HX8352x_wr_cmd(0xFF);	/* Set Page0 */
	HX8352x_wr_dat(0x00); 
	
	/* Check Device Code */
	devicetype = HX8352x_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x52)
	{
		/* Initialize HX8352A */
		HX8352x_wr_cmd(0x83);
		HX8352x_wr_dat(0x02); 
		HX8352x_wr_cmd(0x85);
		HX8352x_wr_dat(0x03); 
		HX8352x_wr_cmd(0x8B);
		HX8352x_wr_dat(0x00); 
		HX8352x_wr_cmd(0x8C);
		HX8352x_wr_dat(0x13); 
		HX8352x_wr_cmd(0x91);
		HX8352x_wr_dat(0x01); 
		HX8352x_wr_cmd(0x83);
		HX8352x_wr_dat(0x00); 
		_delay_ms(5);

		/* ---------------- Gamma Setting  ---------------------------- */
		HX8352x_wr_cmd(0x3E);
		HX8352x_wr_dat(0xC4);
		HX8352x_wr_cmd(0x3F);
		HX8352x_wr_dat(0x44);
		HX8352x_wr_cmd(0x40);
		HX8352x_wr_dat(0x22);
		HX8352x_wr_cmd(0x41);
		HX8352x_wr_dat(0x57);
		HX8352x_wr_cmd(0x42);
		HX8352x_wr_dat(0x03);
		HX8352x_wr_cmd(0x43);
		HX8352x_wr_dat(0x47);
		HX8352x_wr_cmd(0x44);
		HX8352x_wr_dat(0x02);
		HX8352x_wr_cmd(0x45);
		HX8352x_wr_dat(0x55);
		HX8352x_wr_cmd(0x46);
		HX8352x_wr_dat(0x06);
		HX8352x_wr_cmd(0x47);
		HX8352x_wr_dat(0x4C);
		HX8352x_wr_cmd(0x48);
		HX8352x_wr_dat(0x06);
		HX8352x_wr_cmd(0x49);
		HX8352x_wr_dat(0x8C); 

		/* ----------  Power Supply Setting  -------------------------- */   
		HX8352x_wr_cmd(0x17);
		HX8352x_wr_dat(0x91); 
		HX8352x_wr_cmd(0x23);
		HX8352x_wr_dat(0x01);
		HX8352x_wr_cmd(0x2B);
		HX8352x_wr_dat(0xF9); 
		_delay_ms(20);       
		HX8352x_wr_cmd(0x18);
		HX8352x_wr_dat(0x3A); 

		HX8352x_wr_cmd(0x1B);
		HX8352x_wr_dat(0x11); 
		HX8352x_wr_cmd(0x83);
		HX8352x_wr_dat(0x02);
		HX8352x_wr_cmd(0x8A);
		HX8352x_wr_dat(0x06);
		HX8352x_wr_cmd(0x83);
		HX8352x_wr_dat(0x00);
		HX8352x_wr_cmd(0x1A);
		HX8352x_wr_dat(0x11); 

		HX8352x_wr_cmd(0x1C);
		HX8352x_wr_dat(0x0A); 
		HX8352x_wr_cmd(0x1F);
		HX8352x_wr_dat(0x58); 
		_delay_ms(30);
		HX8352x_wr_cmd(0x19);
		HX8352x_wr_dat(0x0A);  
		HX8352x_wr_cmd(0x19);
		HX8352x_wr_dat(0x1A); 
		_delay_ms(50);
		HX8352x_wr_cmd(0x19);
		HX8352x_wr_dat(0x12); 
		_delay_ms(50);
		HX8352x_wr_cmd(0x1E);
		HX8352x_wr_dat(0x2E); 
		_delay_ms(100);

		/* ----------------  DUG FUNCTION ON ---------------------------- */
		HX8352x_wr_cmd(0x5A);
		HX8352x_wr_dat(0x01);

		/* ------------------- RGB LUT ---------------------------------- */ 
		HX8352x_wr_cmd(0x5C);
		for(int i=0;i<3;i++)
		{
			HX8352x_wr_dat(0x00);/* -----------0 */
			HX8352x_wr_dat(0x03);
			HX8352x_wr_dat(0x0A);
			HX8352x_wr_dat(0x0F);
			HX8352x_wr_dat(0x13);
			HX8352x_wr_dat(0x16);/* -----------5 */
			HX8352x_wr_dat(0x19);
			HX8352x_wr_dat(0x1C);
			HX8352x_wr_dat(0x1E);
			HX8352x_wr_dat(0x1F);
			HX8352x_wr_dat(0x25);/* -----------10 */
			HX8352x_wr_dat(0x2A);
			HX8352x_wr_dat(0x30);
			HX8352x_wr_dat(0x35);
			HX8352x_wr_dat(0x39);
			HX8352x_wr_dat(0x3D);/* -----------15 */
			HX8352x_wr_dat(0x41);
			HX8352x_wr_dat(0x45);
			HX8352x_wr_dat(0x48);
			HX8352x_wr_dat(0x4C);
			HX8352x_wr_dat(0x4F);/* -----------20 */
			HX8352x_wr_dat(0x53);
			HX8352x_wr_dat(0x58);
			HX8352x_wr_dat(0x5D);
			HX8352x_wr_dat(0x61);
			HX8352x_wr_dat(0x66);/* -----------25 */
			HX8352x_wr_dat(0x6A);
			HX8352x_wr_dat(0x6E);
			HX8352x_wr_dat(0x72);
			HX8352x_wr_dat(0x76);
			HX8352x_wr_dat(0x7A);/* -----------30 */
			HX8352x_wr_dat(0x7E);
			HX8352x_wr_dat(0x82);
			HX8352x_wr_dat(0x85);
			HX8352x_wr_dat(0x89);
			HX8352x_wr_dat(0x8D);/* -----------35 */
			HX8352x_wr_dat(0x90);
			HX8352x_wr_dat(0x94);
			HX8352x_wr_dat(0x97);
			HX8352x_wr_dat(0x9A);
			HX8352x_wr_dat(0x9D);/* -----------40 */
			HX8352x_wr_dat(0xA2);
			HX8352x_wr_dat(0xA5);
			HX8352x_wr_dat(0xA9);
			HX8352x_wr_dat(0xAC);
			HX8352x_wr_dat(0xB0);/* -----------45 */
			HX8352x_wr_dat(0xB4);
			HX8352x_wr_dat(0xB8);
			HX8352x_wr_dat(0xBC);
			HX8352x_wr_dat(0xC0);
			HX8352x_wr_dat(0xC3);/* -----------50 */
			HX8352x_wr_dat(0xC8);
			HX8352x_wr_dat(0xCC);
			HX8352x_wr_dat(0xD2);
			HX8352x_wr_dat(0xD6);
			HX8352x_wr_dat(0xDC);/* -----------55 */	
			HX8352x_wr_dat(0xDF);
			HX8352x_wr_dat(0xE2);
			HX8352x_wr_dat(0xE5);
			HX8352x_wr_dat(0xE8);
			HX8352x_wr_dat(0xEC);/* -----------60 */
			HX8352x_wr_dat(0xEF);
			HX8352x_wr_dat(0xF4);
			HX8352x_wr_dat(0xFF);/* -----------63 */
		}

		/* -------------  Display ON Setting  -------------------------- */
		HX8352x_wr_cmd(0x3C);
		HX8352x_wr_dat(0xC0); 
		HX8352x_wr_cmd(0x3D);
		HX8352x_wr_dat(0x1C); 
		HX8352x_wr_cmd(0x34);
		HX8352x_wr_dat(0x38);
		HX8352x_wr_cmd(0x35);
		HX8352x_wr_dat(0x38);
		HX8352x_wr_cmd(0x24);
		HX8352x_wr_dat(0x38);
		_delay_ms(50);
		HX8352x_wr_cmd(0x24);
		HX8352x_wr_dat(0x3C);
		HX8352x_wr_cmd(0x16);
		HX8352x_wr_dat((1<<4)|(1<<3)|(1<<2)); 

		HX8352x_wr_cmd(0x01);
		HX8352x_wr_dat(0x06); 
		HX8352x_wr_cmd(0x55);
		HX8352x_wr_dat(0x00);
		
		HX8352x_wr_cmd(0x02);
		HX8352x_wr_dat(0x00); 
		HX8352x_wr_cmd(0x03);
		HX8352x_wr_dat(0x00);
		HX8352x_wr_cmd(0x04);
		HX8352x_wr_dat(((MAX_X)>>8));
		HX8352x_wr_cmd(0x05);
		HX8352x_wr_dat((MAX_X & 0xFF));
		
		HX8352x_wr_cmd(0x06);
		HX8352x_wr_dat(0x00); 
		HX8352x_wr_cmd(0x07);
		HX8352x_wr_dat(0x00);
		HX8352x_wr_cmd(0x08);
		HX8352x_wr_dat(((MAX_Y)>>8));
		HX8352x_wr_cmd(0x09);
		HX8352x_wr_dat((MAX_Y&0xFF));
	
		/* Set Rectangle Function */
		HX8352x_rect = HX8352A_rect;
	}

	else if(devicetype == 0x65)
	{
		/* Initialize HX8352B */
		HX8352x_wr_cmd(0xE2);	/* VREFsetting */
		HX8352x_wr_dat(0x15); 
		HX8352x_wr_cmd(0xE5);
		HX8352x_wr_dat(0x18);
		HX8352x_wr_cmd(0xE7);
		HX8352x_wr_dat(0x18);
		HX8352x_wr_cmd(0xE8);
		HX8352x_wr_dat(0x40);
		HX8352x_wr_cmd(0xEC);
		HX8352x_wr_dat(0x09);
		HX8352x_wr_cmd(0xED);
		HX8352x_wr_dat(0x06);
		HX8352x_wr_cmd(0xEE);
		HX8352x_wr_dat(0x20);
		HX8352x_wr_cmd(0xEF);
		HX8352x_wr_dat(0x50);
		HX8352x_wr_cmd(0x29);
		HX8352x_wr_dat(0x01);
		HX8352x_wr_cmd(0x2B);
		HX8352x_wr_dat(0x03);
		HX8352x_wr_cmd(0x2E);
		HX8352x_wr_dat(0x85);
		/* Power on Setting */
		HX8352x_wr_cmd(0x23);
		HX8352x_wr_dat(0x76);
		HX8352x_wr_cmd(0x24);
		HX8352x_wr_dat(0x57);
		HX8352x_wr_cmd(0x25);
		HX8352x_wr_dat(0x71);
		HX8352x_wr_cmd(0x1B);
		HX8352x_wr_dat(0x1E);		/* VREG1 = 4.5V */
		HX8352x_wr_cmd(0x01);
		HX8352x_wr_dat(0x00);
		HX8352x_wr_cmd(0x1C);
		HX8352x_wr_dat(0x04);
		/* Power on sequence */
		HX8352x_wr_cmd(0x19);
		HX8352x_wr_dat(0x01);
		_delay_ms(5);
		HX8352x_wr_cmd(0x1F);
		HX8352x_wr_dat(0x8C);
		HX8352x_wr_cmd(0x1F);
		HX8352x_wr_dat(0x84);
		_delay_ms(10);
		HX8352x_wr_cmd(0x1F);
		HX8352x_wr_dat(0x94);
		_delay_ms(10);
		HX8352x_wr_cmd(0x1F);
		HX8352x_wr_dat(0xD4);
		_delay_ms(5);
		/* Gamma Setting */
		HX8352x_wr_cmd(0x40);
		HX8352x_wr_dat(0x08);
		HX8352x_wr_cmd(0x41);
		HX8352x_wr_dat(0x31);
		HX8352x_wr_cmd(0x42);
		HX8352x_wr_dat(0x2F);
		HX8352x_wr_cmd(0x43);
		HX8352x_wr_dat(0x3E);
		HX8352x_wr_cmd(0x44);
		HX8352x_wr_dat(0x3D);
		HX8352x_wr_cmd(0x45);
		HX8352x_wr_dat(0x3F);
		HX8352x_wr_cmd(0x46);
		HX8352x_wr_dat(0x2F);
		HX8352x_wr_cmd(0x47);
		HX8352x_wr_dat(0x79);
		HX8352x_wr_cmd(0x48);
		HX8352x_wr_dat(0x08);
		HX8352x_wr_cmd(0x49);
		HX8352x_wr_dat(0x06);
		HX8352x_wr_cmd(0x4A);
		HX8352x_wr_dat(0x08);
		HX8352x_wr_cmd(0x4B);
		HX8352x_wr_dat(0x0E);
		HX8352x_wr_cmd(0x4C);
		HX8352x_wr_dat(0x17);

		HX8352x_wr_cmd(0x50);
		HX8352x_wr_dat(0x00);
		HX8352x_wr_cmd(0x51);
		HX8352x_wr_dat(0x02);
		HX8352x_wr_cmd(0x52);
		HX8352x_wr_dat(0x01);
		HX8352x_wr_cmd(0x53);
		HX8352x_wr_dat(0x10);
		HX8352x_wr_cmd(0x54);
		HX8352x_wr_dat(0x0E);
		HX8352x_wr_cmd(0x55);
		HX8352x_wr_dat(0x37);
		HX8352x_wr_cmd(0x56);
		HX8352x_wr_dat(0x06);
		HX8352x_wr_cmd(0x57);
		HX8352x_wr_dat(0x50);
		HX8352x_wr_cmd(0x58);
		HX8352x_wr_dat(0x08);
		HX8352x_wr_cmd(0x59);
		HX8352x_wr_dat(0x11);
		HX8352x_wr_cmd(0x5A);
		HX8352x_wr_dat(0x17);
		HX8352x_wr_cmd(0x5B);
		HX8352x_wr_dat(0x19);
		HX8352x_wr_cmd(0x5C);
		HX8352x_wr_dat(0x17);
		HX8352x_wr_cmd(0x5D);
		HX8352x_wr_dat(0xFF);
		/* Display ON Setting */
		HX8352x_wr_cmd(0x17);
		HX8352x_wr_dat(0x55);
		HX8352x_wr_cmd(0x16);
		HX8352x_wr_dat((0<<7)|(1<<6)|(0<<5)|(1<<3)|(0<<2)|(0<<1)|(1<<0));/* HX8352x_wr_dat(0x49); */
		HX8352x_wr_cmd(0x28);
		HX8352x_wr_dat(0x20);
		_delay_ms(40);
		HX8352x_wr_cmd(0x28);
		HX8352x_wr_dat(0x38);
		/* Waiting 2 frames at least */
		_delay_ms(40);
		HX8352x_wr_cmd(0x28);
		HX8352x_wr_dat(0x3C);
		
		/* Set Rectangle Function */
		HX8352x_rect = HX8352B_rect;
	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8352x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8352x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8352x_wr_gram(COL_RED);
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
int HX8352x_draw_bmp(const uint8_t* ptr){

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
	HX8352x_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX8352x_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX8352x_wr_gram(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
