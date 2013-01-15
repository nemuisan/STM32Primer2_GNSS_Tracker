/********************************************************************************/
/*!
	@file			st7735.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2011.10.25
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -JD-T18003-T01				(4-wire serial)					@n
					 -ZY-FGD1442701V1			(8bit mode)						@n
					 -HD177P2501				(8bit mode)						@n

    @section HISTORY
		2010.03.01	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added GRAM write function.
		2011.01.20  V4.00   Added Confirmation RDDID1
		                    Added 4-Wire Serial Support.
		2011.03.10	V5.00	C++ Ready.
		2011.06.10	V6.00	HD177P2501 support.
		2011.10.25	V7.00	Added DMA TransactionSupport.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "st7735.h"
/* check header file version for fool proof */
#if __ST7735_H != 0x0700
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
inline void ST7735_reset(void)
{
#ifdef USE_ST7735_TFT
	ST7735_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ST7735_RD_SET();
	ST7735_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	ST7735_RES_CLR();							/* RES=L, CS=L   			*/
	ST7735_CS_CLR();

#elif  USE_ST7735_SPI_TFT
	ST7735_RES_SET();							/* RES=H, CS=H				*/
	ST7735_CS_SET();
	ST7735_SCLK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	ST7735_RES_CLR();							/* RES=L, CS=L   			*/
	ST7735_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     			*/
	ST7735_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait at least 120 ms     */
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ST7735_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7735_wr_cmd(uint8_t cmd)
{
	ST7735_DC_CLR();							/* DC=L		     */
	
	ST7735_CMD = cmd;							/* D7..D0=cmd    */
	ST7735_WR();								/* WR=L->H       */
	
	ST7735_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7735_wr_dat(uint8_t dat)
{
	ST7735_DATA = dat;							/* D7..D0=dat    */
	ST7735_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7735_wr_gram(uint16_t gram)
{
	ST7735_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	ST7735_WR();								/* WR=L->H					*/

	ST7735_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	ST7735_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ST7735_rd_cmd(uint16_t cmd)
{
	uint8_t val;

	ST7735_wr_cmd(cmd);
	ST7735_WR_SET();

    ReadLCDData(val);	/* Dummy */
    ReadLCDData(val);

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7735_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		ST7735_wr_dat(*p++);
		ST7735_wr_dat(*p++);
		ST7735_wr_dat(*p++);
		ST7735_wr_dat(*p++);
	}
	while (n--) {
		ST7735_wr_dat(*p++);
	}
#endif

}

#else /* USE_ST7735_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7735_wr_cmd(uint8_t cmd)
{
	ST7735_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ST7735_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7735_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7735_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ST7735_rd_cmd(uint16_t cmd)
{
	/* Read Function was NOT implemented in SPI-MODE! */
	return 0x5C;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7735_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
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
#endif



/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ST7735_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	ST7735_wr_cmd(CASET); 
	ST7735_wr_dat(0);
	ST7735_wr_dat(OFS_COL + x);
	ST7735_wr_dat(0);
	ST7735_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	ST7735_wr_cmd(RASET);
	ST7735_wr_dat(0);
	ST7735_wr_dat(OFS_RAW + y); 
	ST7735_wr_dat(0);
	ST7735_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	ST7735_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ST7735_clear(void)
{
	volatile uint32_t n;

	ST7735_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		ST7735_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ST7735_init(void)
{
	uint8_t devicetype;
	
	Display_IoInit_If();

	ST7735_reset();

	/* Check Device Code */
	devicetype = ST7735_rd_cmd(RDID1);  			/* Confirm Vaild LCD Controller */

	if((devicetype == 0x5C) || (devicetype == 0xE3))
	{
		/* Initialize ST7735 */
		ST7735_wr_cmd(SWRESET);		/* Software Reset */
		_delay_ms(50);
		
		ST7735_wr_cmd(SLPOUT);		/* Sleep Out */
		_delay_ms(200);
		
		ST7735_wr_cmd(VCOM4L);		/* Vcom 4 Level control */
		ST7735_wr_dat(0x40);
		ST7735_wr_dat(0x03);
		ST7735_wr_dat(0x1A);		/* Write must 0x1A */
		
		ST7735_wr_cmd(NVFCTR1);     /* EEPROM Control Status */
		ST7735_wr_dat(0x60);		/* VMF_EN Command 0xC7(VMOFCTR)enable */
		ST7735_wr_cmd(VMOFCTR);		/* Set VCOM offset control */	
		ST7735_wr_dat(0x90);
		_delay_ms(200);

		ST7735_wr_cmd(FRMCTR1);		/* Set the frame frequency of the full colors normal mode. */
									/* Frame rate=fosc/((RTNA + 20) x (LINE + FPA + BPA)) */
		ST7735_wr_dat(0x04);		/* Set RTNA */
		ST7735_wr_dat(0x25);		/* Set FPA */
		ST7735_wr_dat(0x18);		/* Set BPA */

		ST7735_wr_cmd(FRMCTR2);		/* Set the Frame Rate Control (In Idle mode/ 8-colors) */
									/* Frame rate=fosc/((RTNB + 20) x (LINE + FPB + BPB)) */
		ST7735_wr_dat(0x04);		/* Set RTNB */
		ST7735_wr_dat(0x25);		/* Set FPB */
		ST7735_wr_dat(0x18);		/* Set BPB */

		ST7735_wr_cmd(FRMCTR3);		/* Set the Frame Rate Control (In Partial mode/ full colors) */
									/* Frame rate=fosc/((RTNC + 20) x (LINE + FPC + BPC)) */
		ST7735_wr_dat(0x04);		/* Set RTNC */
		ST7735_wr_dat(0x25);		/* Set FPC */
		ST7735_wr_dat(0x18);		/* Set BPC */
		ST7735_wr_dat(0x04);		/* Set RTND -line inversion mode-*/
		ST7735_wr_dat(0x25);		/* Set FPC */
		ST7735_wr_dat(0x18);		/* Set BPD */

		ST7735_wr_cmd(INVCTR);		/* Set Display Inversion Control */
		ST7735_wr_dat(0x03);		/* Set to Flame inversion */

		ST7735_wr_cmd(DISSET5);		/* Display Function set 5 */
		ST7735_wr_dat(0x15);		/* Amount of non-overlap of the gate output 						:2 clock cycle */
									/* delay amount from gate signal rising edge of the source output   :1 clock cycle */
									/* Set the Equalizing period										:3 clock cycle */
		ST7735_wr_dat(0x02);		/* Normal scan,Source output on non-display area AGND,VCOM output on non-display area AGND */

		ST7735_wr_cmd(PWCTR1);		/* Power Control 1*/
		ST7735_wr_dat(0x02);		/* GVDD=4.70V */
		ST7735_wr_dat(0x70);		/* AVDD=1.0uA */

		ST7735_wr_cmd(PWCTR2);		/* Power Control 2 */
		ST7735_wr_dat(0x07);		/* VGHH=14.7V,VGLL=-12.25V */

		ST7735_wr_cmd(PWCTR3);		/* Power Control 3 */
		ST7735_wr_dat(0x01);		/* Amount of Current in Operational Amplifier :Small */
		ST7735_wr_dat(0x01);		/* Set the Booster circuit Step-up cycle in Normal mode/ full colors. */
									/* circuit 1 :BCLK / 1, circuit 2,4 :BCLK / 2 */

		ST7735_wr_cmd(PWCTR4);		/* Power Control 4 */
		ST7735_wr_dat(0x02);		/* Amount of Current in Operational Amplifier :Medium Low */
		ST7735_wr_dat(0x07);		/* Set the Booster circuit Step-up cycle in Idle mode/8 colors. */
									/* circuit 1 :BCLK / 4, circuit 2,4 :BCLK / 16 */

		ST7735_wr_cmd(PWCTR5);		/* Power Control 5 */
		ST7735_wr_dat(0x02);		/* Amount of Current in Operational Amplifier :Medium Low */
		ST7735_wr_dat(0x04);		/* Set the Booster circuit Step-up cycle in Partial mode/ full-colors. */
									/* circuit 1 :BCLK / 2, circuit 2,4 :BCLK / 4 */


		ST7735_wr_cmd(PWCTR6);		/* Power Control 6 (Gamma control adjust)*/
		ST7735_wr_dat(0x11);
		ST7735_wr_dat(0x17);

		ST7735_wr_cmd(VMCTR1);		/* VCOM Control 1 */
		ST7735_wr_dat(0x4f);		/* VCOMH=+4.475V */ /* original +4.000V(0x3C) */
		ST7735_wr_dat(0x3C);		/* VCOML=-1.000V */ /* original -0.525V(0x4F) */

		ST7735_wr_cmd(MADCTL);		/* Memory Data Access Control */
									/* -This command defines read/ write scanning direction of frame memory. */
		ST7735_wr_dat(MADVAL); 		/* MY=1,MX=1,MV=0,ML=0(LCD vertical refresh Top to Bottom) */
									/* Color selector switch control :BGR color filter panel */
									/* LCD horizontal refresh Left to right */

		ST7735_wr_cmd(COLMOD);		/* Interface Pixel Format */
		ST7735_wr_dat(0x05);		/* 16-bit/pixel R5G6B5 */

		ST7735_wr_cmd(GAMCTRP1);	/* Gamma e+fpolarity Correction Characteristics Setting */
		ST7735_wr_dat(0x06);
		ST7735_wr_dat(0x0E);
		ST7735_wr_dat(0x05);
		ST7735_wr_dat(0x20);
		ST7735_wr_dat(0x27);
		ST7735_wr_dat(0x23);
		ST7735_wr_dat(0x1C);
		ST7735_wr_dat(0x21);
		ST7735_wr_dat(0x20);
		ST7735_wr_dat(0x1C);
		ST7735_wr_dat(0x26);
		ST7735_wr_dat(0x2F);
		ST7735_wr_dat(0x00);
		ST7735_wr_dat(0x03);
		ST7735_wr_dat(0x00);
		ST7735_wr_dat(0x24);

		ST7735_wr_cmd(GAMCTRN1);	/* Gamma e-fpolarity Correction Characteristics Setting */
		ST7735_wr_dat(0x06);
		ST7735_wr_dat(0x10);
		ST7735_wr_dat(0x05);
		ST7735_wr_dat(0x21);
		ST7735_wr_dat(0x27);
		ST7735_wr_dat(0x22);
		ST7735_wr_dat(0x1C);
		ST7735_wr_dat(0x21);
		ST7735_wr_dat(0x1F);
		ST7735_wr_dat(0x1D);
		ST7735_wr_dat(0x27);
		ST7735_wr_dat(0x2F);
		ST7735_wr_dat(0x05);
		ST7735_wr_dat(0x03);
		ST7735_wr_dat(0x00);
		ST7735_wr_dat(0x3F);

		ST7735_wr_cmd(DISPON);		/* Display On */
		_delay_ms(100);

	}

	else { for(;;);} /* Invalid Device Code!! */

	ST7735_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ST7735_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		ST7735_wr_gram(COL_RED);
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
int ST7735_draw_bmp(const uint8_t* ptr){

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
	ST7735_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		ST7735_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			ST7735_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
