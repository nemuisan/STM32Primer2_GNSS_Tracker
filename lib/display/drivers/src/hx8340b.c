/********************************************************************************/
/*!
	@file			hx8340b.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2011.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95215A-1				(HX8340B)		8bit mode only.		@n
					 -S95328-AAA			(HX8340B)		8/16bit mode.

    @section HISTORY
		2010.09.03	V1.00	Stable Release.
		2010.12.31	V2.00	Added GRAM write function.
							Fixed bug in 16bit access.
		2011.03.10	V3.00	C++ Ready.
		2011.11.10	V4.00	Added DMA TransactionSupport.
							Added Device ID Read.
		2011.11.30	V5.00	Added 3-Wire Serial Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8340b.h"
/* check header file version for fool proof */
#if __HX8340B_H != 0x0500
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
inline void HX8340B_reset(void)
{
#ifdef USE_HX8340B_TFT
	HX8340B_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8340B_RD_SET();
	HX8340B_WR_SET();
	_delay_ms(5);								/* wait 5ms     			*/

	HX8340B_RES_CLR();							/* RES=L, CS=L   			*/
	HX8340B_CS_CLR();

#elif  USE_HX8340B_SPI_TFT
	HX8340B_RES_SET();							/* RES=H, CS=H				*/
	HX8340B_CS_SET();
	HX8340B_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(5);								/* wait 5ms     			*/

	HX8340B_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(10);								/* wait 10ms     			*/
	HX8340B_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait at least 120ms     	*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_HX8340B_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8340B_wr_cmd(uint8_t cmd)
{
	HX8340B_DC_CLR();							/* DC=L						*/
	
	HX8340B_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	HX8340B_WR();								/* WR=L->H					*/
	
	HX8340B_DC_SET();							/* DC=H 					*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8340B_wr_dat(uint8_t dat)
{
	HX8340B_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	HX8340B_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8340B_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8340B_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	HX8340B_WR();								/* WR=L->H					*/
	HX8340B_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	HX8340B_DATA = gram;						/* 16bit data 				*/
#endif
	HX8340B_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8340B_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8340B_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8340B_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8340B_rd_cmd(uint8_t cmd)
{
	uint16_t val;

	HX8340B_wr_cmd(cmd);
	HX8340B_WR_SET();

    ReadLCDData(val);

	val &= 0x00FF;

	return (uint8_t)val;
}


#elif USE_HX8340B_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8340B_wr_cmd(uint8_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8340B_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_DATA);
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8340B_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_DATA);
	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8340B_wr_block(uint8_t *p,unsigned int cnt)
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
inline  uint8_t HX8340B_rd_cmd(uint8_t cmd)
{
	uint8_t val;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI(cmd);
	
	SendSPI(START_RD_DATA);
	val = RecvSPI();

	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8340B_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8340B_wr_cmd(0x02);				/* Horizontal RAM Start ADDR2 */
	HX8340B_wr_dat((OFS_COL + x)>>8);
	HX8340B_wr_cmd(0x03);				/* Horizontal RAM Start ADDR1 */
	HX8340B_wr_dat(OFS_COL + x);
	HX8340B_wr_cmd(0x04);				/* Horizontal RAM End ADDR2 */
	HX8340B_wr_dat((OFS_COL + width)>>8);
	HX8340B_wr_cmd(0x05);				/* Horizontal RAM End ADDR1 */
	HX8340B_wr_dat(OFS_COL + width);
	HX8340B_wr_cmd(0x06);				/* Vertical RAM Start ADDR2 */
	HX8340B_wr_dat((OFS_RAW + y)>>8);
	HX8340B_wr_cmd(0x07);				/* Vertical RAM Start ADDR1 */
	HX8340B_wr_dat(OFS_RAW + y);
	HX8340B_wr_cmd(0x08);				/* Vertical RAM End ADDR2 */
	HX8340B_wr_dat((OFS_RAW + height)>>8);
	HX8340B_wr_cmd(0x09);				/* Vertical RAM End ADDR1 */
	HX8340B_wr_dat(OFS_RAW + height);

	HX8340B_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8340B_clear(void)
{
	volatile uint32_t n;

	HX8340B_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8340B_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8340B_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	HX8340B_reset();

	/* Check Device Code */
	devicetype = HX8340B_rd_cmd(0x93);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x49)
	{
		/* Initialize HX8340B */
		/* Driving ability Setting */
		HX8340B_wr_cmd(0x60);
		HX8340B_wr_dat(0x00);
		HX8340B_wr_cmd(0x61);
		HX8340B_wr_dat(0x06);
		HX8340B_wr_cmd(0x62);
		HX8340B_wr_dat(0x00);
		HX8340B_wr_cmd(0x63);
		HX8340B_wr_dat(0xD9);
		_delay_ms(20);
		
		/* Gamma Setting */
		HX8340B_wr_cmd(0x73);
		HX8340B_wr_dat(0x70);
		HX8340B_wr_cmd(0x40);
		HX8340B_wr_dat(0x00);
		HX8340B_wr_cmd(0x41);
		HX8340B_wr_dat(0x40);
		HX8340B_wr_cmd(0x42);
		HX8340B_wr_dat(0x45);
		HX8340B_wr_cmd(0x43);
		HX8340B_wr_dat(0x01);
		HX8340B_wr_cmd(0x44);
		HX8340B_wr_dat(0x60);
		HX8340B_wr_cmd(0x45);
		HX8340B_wr_dat(0x05);
		HX8340B_wr_cmd(0x46);
		HX8340B_wr_dat(0x0C);
		HX8340B_wr_cmd(0x47);
		HX8340B_wr_dat(0xD1);
		HX8340B_wr_cmd(0x48);
		HX8340B_wr_dat(0x05);
		
		HX8340B_wr_cmd(0x50);
		HX8340B_wr_dat(0x75);
		HX8340B_wr_cmd(0x51);
		HX8340B_wr_dat(0x01);
		HX8340B_wr_cmd(0x52);
		HX8340B_wr_dat(0x67);
		HX8340B_wr_cmd(0x53);
		HX8340B_wr_dat(0x14);
		HX8340B_wr_cmd(0x54);
		HX8340B_wr_dat(0xF2);
		HX8340B_wr_cmd(0x55);
		HX8340B_wr_dat(0x07);
		HX8340B_wr_cmd(0x56);
		HX8340B_wr_dat(0x03);
		HX8340B_wr_cmd(0x57);
		HX8340B_wr_dat(0x49);
		_delay_ms(20);

		/*	Power Voltage Setting */
		HX8340B_wr_cmd(0x1F);	/* VRH=4.65V     VREG1EGAMMAE 00~1E*/
		HX8340B_wr_dat(0x03); 
		HX8340B_wr_cmd(0x20);   /* BT (VGH~15V,VGL~-12V,DDVDH~5V) */
		HX8340B_wr_dat(0x00);
		HX8340B_wr_cmd(0x24);	/* VCOMH(VCOM High voltage3.2V) */
		HX8340B_wr_dat(0x2f);   /* 1Ch */
		HX8340B_wr_cmd(0x25);	/* VCOML(VCOM Low voltage -1.2V) */
		HX8340B_wr_dat(0x48);	/* 34h */

		/* VCOM offset */ 
		HX8340B_wr_cmd(0x23);   /* VMF(no offset) */
		HX8340B_wr_dat(0x2F);
		_delay_ms(20);
		
		/* Power Supply Setting */
		HX8340B_wr_cmd(0x18);	/* I/P_RADJ,N/P_RADJ Noraml mode 60Hz */
		HX8340B_wr_dat(0x44);
		HX8340B_wr_cmd(0x1B);
		HX8340B_wr_dat(0x44);
		HX8340B_wr_cmd(0x21);	/* OSC_EN='1' start osc */
		HX8340B_wr_dat(0x01);
		HX8340B_wr_cmd(0x01);	/* SLP='0' out sleep */
		HX8340B_wr_dat(0x00);
		HX8340B_wr_cmd(0x1C);	/* AP=011 */
		HX8340B_wr_dat(0x03);
		HX8340B_wr_cmd(0x19);	/* VOMG=1,PON=1, DK=0, */
		HX8340B_wr_dat(0x06);
		_delay_ms(20);

		/* 262k/65k color selection */
		HX8340B_wr_cmd(0x17);	/* 65k Colour */
		HX8340B_wr_dat(0x05);	
		HX8340B_wr_cmd(0x26);	/* PT=10,GON=0, DTE=0, D=0100 */
		HX8340B_wr_dat(0x84);
		_delay_ms(40);
		HX8340B_wr_cmd(0x26);	/* PT=10,GON=1, DTE=1, D=1000 */
		HX8340B_wr_dat(0xB8);	
		_delay_ms(40);
		HX8340B_wr_cmd(0x26);	/* PT=10,GON=1, DTE=1, D=1100 */
		HX8340B_wr_dat(0xBC);
		_delay_ms(20);

	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8340B_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8340B_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8340B_wr_gram(COL_RED);
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
int HX8340B_draw_bmp(const uint8_t* ptr){

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
	HX8340B_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX8340B_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX8340B_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
