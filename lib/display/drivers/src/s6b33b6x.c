/********************************************************************************/
/*!
	@file			s6b33b6x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.02.10
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG12D245A CSTN module.							@n
					8/16bit-i8080,4wire8bit/3wire9bit Serial Support!

    @section HISTORY
		2012.02.10	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6b33b6x.h"
/* check header file version for fool proof */
#if __S6B33B6X_H != 0x0100
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#warning "Due to CSTN-Driver IC, You might need slow access (Under 20MHz)"

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
inline void S6B33B6x_reset(void)
{
#ifdef USE_S6B33B6x_CSTN
	S6B33B6x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6B33B6x_RD_SET();
	S6B33B6x_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	S6B33B6x_RES_CLR();							/* RES=L, CS=L   			*/
	S6B33B6x_CS_CLR();

#elif  USE_S6B33B6x_SPI_CSTN
	S6B33B6x_RES_SET();							/* RES=H, CS=H				*/
	S6B33B6x_CS_SET();
	S6B33B6x_SCLK_SET();						/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	S6B33B6x_RES_CLR();							/* RES=L, CS=L   			*/
	S6B33B6x_CS_CLR();

#endif

	_delay_ms(2);								/* wait 2ms     			*/
	S6B33B6x_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait at least 4 ms     */
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_S6B33B6x_CSTN
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6B33B6x_wr_cmd(uint8_t cmd)
{
	S6B33B6x_DC_CLR();							/* DC=L		     */
	
	S6B33B6x_CMD = cmd;							/* D7..D0=cmd    */
	S6B33B6x_WR();								/* WR=L->H       */
	
	S6B33B6x_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void S6B33B6x_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6B33B6x_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	S6B33B6x_WR();								/* WR=L->H					*/
	S6B33B6x_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	S6B33B6x_DATA = gram;						/* 16bit data 				*/
#endif
	S6B33B6x_WR();								/* WR=L->H					*/
}


/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6B33B6x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6B33B6x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		S6B33B6x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

#else /* USE_S6B33B6x_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6B33B6x_wr_cmd(uint8_t cmd)
{
	S6B33B6x_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	DNC_CMD();
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	S6B33B6x_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6B33B6x_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	DNC_DAT();
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}
static inline void S6B33B6x_wr_sdat(uint8_t dat)
{	
	DNC_DAT();
	SendSPI(dat);
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void S6B33B6x_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */

#if defined(S6B33B6xSPI_4WIREMODE)
	SendSPI16(gram);
#else
	DNC_DAT();
	SendSPI((uint8_t)(gram>>8));
	DNC_DAT();
	SendSPI((uint8_t)gram);
#endif
	DISPLAY_NEGATE_CS();						/* CS=H		     */
}


/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6B33B6x_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;
	
	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		S6B33B6x_wr_sdat(*p++);
		S6B33B6x_wr_sdat(*p++);
		S6B33B6x_wr_sdat(*p++);
		S6B33B6x_wr_sdat(*p++);
	}
	while (n--) {
		S6B33B6x_wr_sdat(*p++);
	}
#endif

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}
#endif



/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6B33B6x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	S6B33B6x_wr_cmd(0x43);
	S6B33B6x_wr_cmd(OFS_COL + x);
	S6B33B6x_wr_cmd(OFS_COL + width);

	/* Set RAS Address */
	S6B33B6x_wr_cmd(0x42);
	S6B33B6x_wr_cmd(OFS_RAW + y);
	S6B33B6x_wr_cmd(OFS_RAW + height); 

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6B33B6x_clear(void)
{
	volatile uint32_t n;

	S6B33B6x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		S6B33B6x_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6B33B6x_init(void)
{

	Display_IoInit_If();

	S6B33B6x_reset();

	/* Initialize S6B33B6x */
	S6B33B6x_wr_cmd(0x2c);
	_delay_ms(50);

	S6B33B6x_wr_cmd(0x02);
	S6B33B6x_wr_cmd(0x01);

	S6B33B6x_wr_cmd(0x20);			/* DC-DC selset */
	S6B33B6x_wr_cmd(0x0f);			/* X2.0 */

	S6B33B6x_wr_cmd(0x26);
	S6B33B6x_wr_cmd(0x0f);
	_delay_ms(50);

	S6B33B6x_wr_cmd(0x28);
	S6B33B6x_wr_cmd(0x01);

	S6B33B6x_wr_cmd(0x2e);

	S6B33B6x_wr_cmd(0x45);			/* RAM Skip AREA set */
	S6B33B6x_wr_cmd(0x00);

	S6B33B6x_wr_cmd(0x53);
	S6B33B6x_wr_cmd(0x00);

	S6B33B6x_wr_cmd(0x10);			/* Drive output mode set */
	S6B33B6x_wr_cmd(0x03);			/* 0,0,DLN,MY,MX,SWP,CDIR */

	S6B33B6x_wr_cmd(0x24);
	S6B33B6x_wr_cmd(0x22);

	S6B33B6x_wr_cmd(0x30);	 		/* 4k he 65k */
	S6B33B6x_wr_cmd(0x1d);

	S6B33B6x_wr_cmd(0x32);
	S6B33B6x_wr_cmd(0x0e);

	S6B33B6x_wr_cmd(0x40);			/* Interface Select */
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6B33B6x_wr_cmd(0x80);			/* mode 8-bit  */
#else
	S6B33B6x_wr_cmd(0x00);			/* mode 16-bit */
#endif		

	S6B33B6x_wr_cmd(0x42);			/* Y ADDRESS */
	S6B33B6x_wr_cmd(0x00);
	S6B33B6x_wr_cmd(0x83);

	S6B33B6x_wr_cmd(0x43);			/* X ADDRESS */
	S6B33B6x_wr_cmd(0x00);
	S6B33B6x_wr_cmd(0x83);

	S6B33B6x_wr_cmd(0x34);
	S6B33B6x_wr_cmd(0x8d);

	S6B33B6x_wr_cmd(0x2a);
	S6B33B6x_wr_cmd(0x45);

	S6B33B6x_wr_cmd(0x2b);
	S6B33B6x_wr_cmd(0x69);

	S6B33B6x_wr_cmd(0x22);
	S6B33B6x_wr_cmd(0x11);

	S6B33B6x_wr_cmd(0x55);
	S6B33B6x_wr_cmd(0x00);
	S6B33B6x_wr_cmd(0x56);
	S6B33B6x_wr_cmd(0x00);
	S6B33B6x_wr_cmd(0x57);
	S6B33B6x_wr_cmd(0x9f);

	S6B33B6x_wr_cmd(0x59);
	S6B33B6x_wr_cmd(0x00);
	S6B33B6x_wr_cmd(0x00);
	S6B33B6x_wr_cmd(0x9f);
	S6B33B6x_wr_cmd(0x00);

	S6B33B6x_wr_cmd(0x51);			/* Display ON */

	S6B33B6x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6B33B6x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		S6B33B6x_wr_gram(COL_RED);
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
int S6B33B6x_draw_bmp(const uint8_t* ptr){

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
	S6B33B6x_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		S6B33B6x_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			S6B33B6x_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
