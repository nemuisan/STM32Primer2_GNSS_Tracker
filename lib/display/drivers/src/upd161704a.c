/********************************************************************************/
/*!
	@file			upd161704a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.07.15
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive LTM022A69B TFT module.							@n
					 8/16bit-i8080,4wire8bit Serial Support!

    @section HISTORY
		2012.07.15	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "upd161704a.h"
/* check header file version for fool proof */
#if __UPD161704A_H != 0x0100
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
    Stupid Delay Routine for Fastest Bus (like STM32F4xx FSMC).
*/
/**************************************************************************/
static inline void _delay_nop(volatile uint32_t ms)
{
	while (ms--);
}

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void UPD161704A_reset(void)
{
#ifdef USE_UPD161704A_TFT
	UPD161704A_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	UPD161704A_RD_SET();
	UPD161704A_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/
	
	UPD161704A_RES_CLR();						/* RES=L, CS=L   			*/
	UPD161704A_CS_CLR();

#elif  USE_UPD161704A_SPI_TFT
	UPD161704A_RES_SET();						/* RES=H, CS=H				*/
	UPD161704A_CS_SET();
	UPD161704A_SCK_SET();						/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	UPD161704A_RES_CLR();						/* RES=L		   			*/
#endif

	_delay_ms(10);								/* wait 10ms     			*/
	
	UPD161704A_RES_SET();						/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_UPD161704A_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void UPD161704A_wr_cmd(uint16_t cmd)
{
	UPD161704A_DC_CLR();						/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	UPD161704A_DATA = (uint8_t)(cmd>>8);		/* upper 8bit data			*/
	UPD161704A_WR();							/* WR=L->H					*/
	UPD161704A_DATA = (uint8_t)cmd;				/* lower 8bit data			*/
#else
	UPD161704A_DATA = cmd;						/* 16bit data 				*/
#endif

	UPD161704A_DC_SET();						/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM/DATA.
*/
/**************************************************************************/
inline void UPD161704A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	UPD161704A_DATA = (uint8_t)(dat>>8);		/* upper 8bit data			*/
	UPD161704A_WR();							/* WR=L->H					*/
	UPD161704A_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	UPD161704A_DATA = dat;						/* 16bit data 				*/
#endif
	UPD161704A_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void UPD161704A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		UPD161704A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		UPD161704A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t UPD161704A_rd_cmd(uint16_t cmd)
{
	uint8_t val;

	UPD161704A_wr_cmd(cmd);
	UPD161704A_WR_SET();

	val &= 0x00FF;
	return val;
}


#elif USE_UPD161704A_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void UPD161704A_wr_cmd(uint16_t cmd)
{
	UPD161704A_DC_CLR();						/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	UPD161704A_DC_SET();						/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data/GRAM.
*/
/**************************************************************************/
inline void UPD161704A_wr_dat(uint16_t dat)
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
inline void UPD161704A_wr_block(uint8_t *p,unsigned int cnt)
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
inline uint8_t UPD161704A_rd_cmd(uint16_t cmd)
{
	uint8_t val;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	SendSPI16(cmd);
	DISPLAY_NEGATE_CS();						/* CS=H		     */

	UPD161704A_DC_CLR();						/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	val  = RecvSPI();
	val &= 0x00FF;

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	UPD161704A_DC_SET();						/* DC=L		     */
	
	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void UPD161704A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	UPD161704A_wr_cmd(0x0008);				/* Horizontal RAM Start ADDR */
	UPD161704A_wr_dat(OFS_COL + x);
	UPD161704A_wr_cmd(0x0009);				/* Horizontal RAM End ADDR */
	UPD161704A_wr_dat(OFS_COL + width);
	UPD161704A_wr_cmd(0x000A);				/* Vertical RAM Start ADDR */
	UPD161704A_wr_dat(OFS_RAW + y);
	UPD161704A_wr_cmd(0x000B);				/* Vertical RAM End ADDR */
	UPD161704A_wr_dat(OFS_RAW + height);

	UPD161704A_wr_cmd(0x0006);				/* Horizontal RAM Start ADDR */
	UPD161704A_wr_dat(OFS_COL + x);
	UPD161704A_wr_cmd(0x0007);				/* Vertical RAM Start ADDR */
	UPD161704A_wr_dat(OFS_RAW + y);

	UPD161704A_wr_cmd(0x000E);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void UPD161704A_clear(void)
{
	volatile uint32_t n;

	UPD161704A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		UPD161704A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void UPD161704A_init(void)
{
	Display_IoInit_If();

	UPD161704A_reset();

	/* Initialize uPD161704A */
	/* register reset */
	UPD161704A_wr_cmd(0x0003);
	UPD161704A_wr_dat(0x0001);		/* Soft reset (reset pulse occurs by writing in "1") */

	/* oscillator start */
	UPD161704A_wr_cmd(0x003A);
	UPD161704A_wr_dat(0x0001);		/* Oscillator control (0:oscillator stop, 1: oscillator operation) */
	_delay_ms(1);

	/* y-setting */
	UPD161704A_wr_cmd(0x0024);
	UPD161704A_wr_dat(0x007B);		/* amplitude setting */
	_delay_ms(1);
	UPD161704A_wr_cmd(0x0025);
	UPD161704A_wr_dat(0x003B);		/* amplitude setting */
	UPD161704A_wr_cmd(0x0026);
	UPD161704A_wr_dat(0x0034);		/* amplitude setting */
	_delay_ms(1);
	UPD161704A_wr_cmd(0x0027);
	UPD161704A_wr_dat(0x0004);		/* amplitude setting */	
	UPD161704A_wr_cmd(0x0052);
	UPD161704A_wr_dat(0x0025);		/* circuit setting 1 */
	_delay_ms(1);
	UPD161704A_wr_cmd(0x0053);
	UPD161704A_wr_dat(0x0033);		/* circuit setting 2 */	
	UPD161704A_wr_cmd(0x0061);
	UPD161704A_wr_dat(0x001C);		/* adjustment V10 positive polarity */
	_delay_ms(1);
	UPD161704A_wr_cmd(0x0062);
	UPD161704A_wr_dat(0x002C);		/* adjustment V9 negative polarity */
	UPD161704A_wr_cmd(0x0063);
	UPD161704A_wr_dat(0x0022);		/* adjustment V34 positive polarity */
	_delay_ms(1);
	UPD161704A_wr_cmd(0x0064);
	UPD161704A_wr_dat(0x0027);		/* adjustment V31 negative polarity */
	_delay_ms(1);
	UPD161704A_wr_cmd(0x0065);
	UPD161704A_wr_dat(0x0014);		/* adjustment V61 negative polarity */
	_delay_ms(1);
	UPD161704A_wr_cmd(0x0066);
	UPD161704A_wr_dat(0x0010);		/* adjustment V61 negative polarity */
	
	/* Basical clock for 1 line (BASECOUNT[7:0]) number specified */
	UPD161704A_wr_cmd(0x002E);
	UPD161704A_wr_dat(0x002D);
	
	/* Power supply setting */
	UPD161704A_wr_cmd(0x0019);
	UPD161704A_wr_dat(0x0000);		/* DC/DC output setting */
	_delay_ms(10);
	UPD161704A_wr_cmd(0x001A);
	UPD161704A_wr_dat(0x1000);		/* DC/DC frequency setting */ 
	UPD161704A_wr_cmd(0x001B);
	UPD161704A_wr_dat(0x0023);		/* DC/DC rising setting */
	UPD161704A_wr_cmd(0x001C);
	UPD161704A_wr_dat(0x0C01);		/* Regulator voltage setting */
	UPD161704A_wr_cmd(0x001D);
	UPD161704A_wr_dat(0x0000);		/* Regulator current setting */
	UPD161704A_wr_cmd(0x001E);
	UPD161704A_wr_dat(0x0009);		/* VCOM output setting */
	UPD161704A_wr_cmd(0x001F);
	UPD161704A_wr_dat(0x0035);		/* VCOM amplitude setting */	
	UPD161704A_wr_cmd(0x0020);
	UPD161704A_wr_dat(0x0015);		/* VCOMM cencter setting */	
	UPD161704A_wr_cmd(0x0018);
	UPD161704A_wr_dat(0x1E7B);		/* DC/DC operation setting */

	/* windows setting */
	UPD161704A_wr_cmd(0x0008);
	UPD161704A_wr_dat(0x0000);		/* Minimum X address in window access mode */
	UPD161704A_wr_cmd(0x0009);
	UPD161704A_wr_dat(0x00EF);		/* Maximum X address in window access mode */
	UPD161704A_wr_cmd(0x000a);
	UPD161704A_wr_dat(0x0000);		/* Minimum Y address in window access mode */
	UPD161704A_wr_cmd(0x000b);
	UPD161704A_wr_dat(0x013F);		/* Maximum Y address in window access mode */

	/* LCD display area setting */
	UPD161704A_wr_cmd(0x0029);
	UPD161704A_wr_dat(0x0000);		/* [LCDSIZE]  X MIN. size set */
	UPD161704A_wr_cmd(0x002A);
	UPD161704A_wr_dat(0x0000);		/* [LCDSIZE]  Y MIN. size set */
	UPD161704A_wr_cmd(0x002B);
	UPD161704A_wr_dat(0x00EF);		/* [LCDSIZE]  X MAX. size set */
	UPD161704A_wr_cmd(0x002C);
	UPD161704A_wr_dat(0x013F);		/* [LCDSIZE]  Y MAX. size set */

	/* Gate scan setting */
	UPD161704A_wr_cmd(0x0032);
	UPD161704A_wr_dat(0x0002);
	
	/* n line inversion line number */
	UPD161704A_wr_cmd(0x0033);
	UPD161704A_wr_dat(0x0000);

	/* Line inversion/frame inversion/interlace setting */
	UPD161704A_wr_cmd(0x0037);
	UPD161704A_wr_dat(0x0000);
	
	/* Gate scan operation setting register */
	UPD161704A_wr_cmd(0x003B);
	UPD161704A_wr_dat(0x0001);
	
	/* Color mode */
	UPD161704A_wr_cmd(0x0004);
	UPD161704A_wr_dat(0x0000);		/* GS = 0: 260-k color (64 gray scale), GS = 1: 8 color (2 gray scale) */

	/* RAM control register */
	UPD161704A_wr_cmd(0x0005);
	UPD161704A_wr_dat(0x0010);		/* Window access control (0: Normal access, 1: Window access) */
	
	/* Display setting register 2 */
	UPD161704A_wr_cmd(0x0001);
	UPD161704A_wr_dat(0x0000);

	/* display setting	*/
	UPD161704A_wr_cmd(0x0000);
	UPD161704A_wr_dat(0x0000);		/* display on */
		
	UPD161704A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	UPD161704A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		UPD161704A_wr_dat(COL_RED);
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
int UPD161704A_draw_bmp(const uint8_t* ptr){

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
	UPD161704A_clear();

   	/* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

   	/* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		UPD161704A_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			UPD161704A_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
