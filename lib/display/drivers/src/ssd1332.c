/********************************************************************************/
/*!
	@file			ssd1332.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2012.02.15
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive ALO-095BWNN-J9 OLED module via SPI.

    @section HISTORY
		2010.06.01	V1.00	Unstable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added GRAM write function.
		2011.03.10	V4.00	C++ Ready.
		2012.02.15	V5.00	Add Foolest WorkAround In Write GGRAM Access.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1332.h"
/* check header file version for fool proof */
#if __SSD1332_H != 0x0500
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
    Abstract Layer Delay Settings.
*/
/**************************************************************************/
void SSD1332_reset(void)
{
	SSD1332_RES_SET();							/* RES=H, CS=H				*/
	SSD1332_CS_SET();
	_delay_ms(5);								/* wait 5ms     			*/

	SSD1332_RES_CLR();							/* RES=L		   			*/
	_delay_ms(80);								/* wait 80ms     			*/
	
	SSD1332_RES_SET();						  	/* RES=H		   			*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}

/**************************************************************************/
/*! 
    Write OLED Command.
*/
/**************************************************************************/
inline void SSD1332_wr_cmd(uint8_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	SSD1332_DC_CLR();							/* DC=L		     */
	
	SendSPI(cmd);

	SSD1332_DC_SET();							/* DC=H		     */
	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write OLED Data.
*/
/**************************************************************************/
inline void SSD1332_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write OLED GRAM.
*/
/**************************************************************************/
inline void SSD1332_wr_gram(uint16_t gram)
{	
	DISPLAY_ASSART_CS();						/* CS=L		    		*/
	
	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     		*/
	SSD1332_PUSH_CLK();							/* Foolest Workaround   */
}

/**************************************************************************/
/*! 
    Write OLED Block Data.
*/
/**************************************************************************/
inline void SSD1332_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		SSD1332_wr_dat (*p++);
		SSD1332_wr_dat (*p++);
		SSD1332_wr_dat (*p++);
		SSD1332_wr_dat (*p++);
	}
	while (n--) {
		SSD1332_wr_dat (*p++);
	}
#endif

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD1332_PUSH_CLK();							/* Foolest Workaround   */
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1332_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SSD1332_wr_cmd(0x15);
	SSD1332_wr_cmd(OFS_COL + x);
	SSD1332_wr_cmd(OFS_COL + width);

	/* Set RAS Address */
	SSD1332_wr_cmd(0x75);
	SSD1332_wr_cmd(OFS_RAW + y);
	SSD1332_wr_cmd(OFS_RAW + height); 

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1332_clear(void)
{
	volatile uint32_t n;

	SSD1332_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1332_wr_gram(COL_BLACK);
	} while (--n);
	SSD1332_PUSH_CLK();							/* Foolest Workaround   */
}


/**************************************************************************/
/*! 
    OLED Module Initialize.
*/
/**************************************************************************/
void SSD1332_init(void)
{
	Display_IoInit_If();
	
	SSD1332_reset();
	
	/* SSD1332 */
	SSD1332_wr_cmd(0xAE);			/* Display OFF */
	
	SSD1332_wr_cmd(0x75);			/* Set Row Address */
	SSD1332_wr_cmd(0x00);			/* Start :0  */
	SSD1332_wr_cmd(0x3F);			/* End   :63 */
	
	SSD1332_wr_cmd(0x15);			/* Set Column Address */
	SSD1332_wr_cmd(0x00);			/* Start :0  */
	SSD1332_wr_cmd(0x5F);			/* End   :96 */
	
	/* Contrast */
	SSD1332_wr_cmd(0xA0); 			/* Set remap&data format 0111 0000 */
	SSD1332_wr_cmd(0x74);
	SSD1332_wr_cmd(0xA1); 			/* Set display star row RAM */
	SSD1332_wr_cmd(0x00);
	SSD1332_wr_cmd(0xA2);			/* Set display offset */
	SSD1332_wr_cmd(0x00);
	SSD1332_wr_cmd(0xA4); 			/* Set Display Mode */
	SSD1332_wr_cmd(0xA8); 			/* Set Multiplex Ratio */
	SSD1332_wr_cmd(0x3F);
	SSD1332_wr_cmd(0xAD); 			/* Set Master Configuration */
	SSD1332_wr_cmd(0x8F); 			/* (External VCC Supply Selected) */
	SSD1332_wr_cmd(0xB0); 			/* Set Power Saving Mode */
	SSD1332_wr_cmd(0x1A);
	SSD1332_wr_cmd(0xB1); 			/* Set Phase 1 & 2 Period Adjustment */
	SSD1332_wr_cmd(0x74);
	SSD1332_wr_cmd(0xB3); 			/* Set Display Clock Divide Ratio / Oscillator */
	
	/* Frequency */
	SSD1332_wr_cmd(0xD0);
	SSD1332_wr_cmd(0x8A); 			/* Set Second Pre-charge Speed of Color A */
	SSD1332_wr_cmd(0x81);
	SSD1332_wr_cmd(0x8B); 			/* Set Second Pre-charge Speed of Color B */
	SSD1332_wr_cmd(0x82);
	SSD1332_wr_cmd(0x8C); 			/* Set Second Pre-charge Speed of Color C */
	SSD1332_wr_cmd(0x83);
	SSD1332_wr_cmd(0xBB); 			/* Set Pre-charge Level */
	SSD1332_wr_cmd(0x3E);
	SSD1332_wr_cmd(0xBE); 			/* Set VCOMH */
	SSD1332_wr_cmd(0x3E);
	SSD1332_wr_cmd(0x87); 			/* Set Master Current Control */
	SSD1332_wr_cmd(0x0F);
	SSD1332_wr_cmd(0x81); 			/* Set Contrast Control for Color gAh*/
	SSD1332_wr_cmd(0x90);
	SSD1332_wr_cmd(0x82); 			/* Set Contrast Control for Color gBh*/
	SSD1332_wr_cmd(0x80);
	SSD1332_wr_cmd(0x83); 			/* Set Contrast Control for Color gCh*/
	SSD1332_wr_cmd(0x80);
	
	SSD1332_clear();

	SSD1332_wr_cmd(0xAF); 			/* Display ON */


#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1332_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1332_wr_gram(COL_RED);
	} while (--n);
	SSD1332_PUSH_CLK();							/* Foolest Workaround   */
	_delay_ms(500);
	for(;;);

#endif

}


/**************************************************************************/
/*! 
    Draw Windows 24bitBMP File.
*/
/**************************************************************************/
int SSD1332_draw_bmp(const uint8_t* ptr){

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
	SSD1332_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SSD1332_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access SSD1332 specific (R->G->Bonly)*/
			d = (*p++ >> 3) << 11;
			d |= (*p++ >> 2) << 5;
			d |= *p++ >> 3;
			SSD1332_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
