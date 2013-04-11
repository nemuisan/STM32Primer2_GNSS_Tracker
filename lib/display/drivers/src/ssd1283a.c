/********************************************************************************/
/*!
	@file			ssd1283a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2011.03.10
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive H016IT01 TFT module(4-wire,8-bit serial only)

    @section HISTORY
		2010.03.01	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Changed Some Description (16bit Write as GRAM write).
		2011.03.10	V4.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1283a.h"
/* check header file version for fool proof */
#if __SSD1283A_H != 0x0400
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
static void _delay_ms(uint32_t ms)
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
void SSD1283A_reset(void)
{
	SSD1283A_RES_SET();							/* RES=H, CS=H ,SCLK=L  	*/
	SSD1283A_CS_SET();
	SSD1283A_SCLK_CLR();
	_delay_ms(100);				    			/* wait 100	ms     			*/

	SSD1283A_RES_CLR();							/* RES=L, CS=L   			*/
	SSD1283A_CS_CLR();
	_delay_ms(5);								/* wait 5ms     			*/
	
	SSD1283A_RES_SET();						  	/* RES=H		 			*/
	_delay_ms(100);				    			/* wait 100	ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD1283A_wr_cmd(uint8_t cmd)
{
	SSD1283A_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD1283A_DC_SET();							/* DC=H		     */
}

/**************************************************************************/
/*! 
    Write LCD DATA and GRAM.
*/
/**************************************************************************/
inline void SSD1283A_wr_dat(uint16_t dat)
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
inline void SSD1283A_wr_block(uint8_t *p,unsigned int cnt)
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

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1283A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SSD1283A_wr_cmd(0x44);
	SSD1283A_wr_dat((uint16_t)((OFS_COL + width)<<8) | (OFS_COL + x));
	
	/* Set RAS Address */
	SSD1283A_wr_cmd(0x45);
	SSD1283A_wr_dat((uint16_t)((OFS_RAW + height)<<8) | (OFS_RAW + y));

	/* Set RAM Address */
	SSD1283A_wr_cmd(0x21);
	SSD1283A_wr_dat((uint16_t)((OFS_RAW + y)<<8) | (OFS_COL + x));
	
	/* Write RAM */
	SSD1283A_wr_cmd(0x22);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1283A_clear(void)
{
	volatile uint32_t n;

	SSD1283A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		SSD1283A_wr_dat(COL_BLACK);
	} while (--n);

}                                    


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SSD1283A_init(void)
{
	Display_IoInit_If();
	
	SSD1283A_reset();
	
	/* SSD1283A */
	SSD1283A_wr_cmd(0x10);			/* Power Control (1) */
	SSD1283A_wr_dat(0x2f8e);		/* Onchip DCDC Clock,Fine*2 StartCycle,VGH unregurated */
									/* StepUpCycle fosc,Internal OPAMP Current reserved,Sleep Mode OFF */

	SSD1283A_wr_cmd(0x11);			/* Power Control (2) */
	SSD1283A_wr_dat(0x000c);		/* VGH/VCI ratio x4 */

	SSD1283A_wr_cmd(0x07);			/* Display Control */
	SSD1283A_wr_dat(0x0021);		/* Nomalize source out zero, Vartical Scroll not pafomed */
									/* 1 Divison Display,Source output GND, Gate output GVOFFL */

	SSD1283A_wr_cmd(0x28);			/* VCOM-OTP1 */
	SSD1283A_wr_dat(0x0006);

	SSD1283A_wr_cmd(0x28);			/* VCOM-OTP1 */
	SSD1283A_wr_dat(0x0005);

	SSD1283A_wr_cmd(0x27);			/* VCOM-OTP */
	SSD1283A_wr_dat(0x057f);

	SSD1283A_wr_cmd(0x29);			/* VCOM-OTP2 */
	SSD1283A_wr_dat(0x89a1);

	SSD1283A_wr_cmd(0x00);			/* Oscillator On */
	SSD1283A_wr_dat(0x0001);
	_delay_ms(150);

	SSD1283A_wr_cmd(0x29);			/* VCOM-OTP2 */
	SSD1283A_wr_dat(0x80b0);
	_delay_ms(50);
	
	SSD1283A_wr_cmd(0x29);			/* VCOM-OTP2 */
	SSD1283A_wr_dat(0xfffe);

	SSD1283A_wr_cmd(0x07);			/* Display Control */
	SSD1283A_wr_dat(0x0023);	
	_delay_ms(50);
	
	SSD1283A_wr_cmd(0x07);			/* Display Control (Confirm) */
	SSD1283A_wr_dat(0x0023);	
	_delay_ms(50);

	SSD1283A_wr_cmd(0x07);			/* Display Control */
	SSD1283A_wr_dat(0x0033);		/* Nomalize source out zero, Vartical Scroll not pafomed */
									/* 1 Divison Display,,Source output GND (OFF) */
									/* Gate output Selected VGH, others GVOFFL */

	SSD1283A_wr_cmd(0x01);			/* Driver Output Control */
	SSD1283A_wr_dat(0x2183);		/* Reversal ON ,Cs On Common,BGR,Scanning Mounting Method */
									/* Output Shift Direction of Gate Driver G131->G0, */
									/* Output Shift Direction of Source Driver S0->S395, */
									/* Number of Gate Driver 131 */
									
	SSD1283A_wr_cmd(0x2f);			/* ??? */
	SSD1283A_wr_dat(0xffff);

	SSD1283A_wr_cmd(0x2c);			/* Oscillator frequency */
	SSD1283A_wr_dat(0x8000);		/* 520kHz */

	SSD1283A_wr_cmd(0x03);			/* Entry mode */
	SSD1283A_wr_dat(0x6830);		/* Colour mode 65k,OE defines Display Window,D-mode from internal ram */
									/* Horizontal increment & Vertical increment & AM 0 Horizontal */

	SSD1283A_wr_cmd(0x40);			/* Set Offset */
	SSD1283A_wr_dat(0x0002);	

	SSD1283A_wr_cmd(0x27);			/* Further bias current setting */
	SSD1283A_wr_dat(0x0570);		/* Maximum */

	SSD1283A_wr_cmd(0x02);			/* LCD-Driving-Waveform Control */
	SSD1283A_wr_dat(0x0300);		/* the odd/even frame-select signals and the N-line inversion */
									/* signals are EORed for alternating drive */

	SSD1283A_wr_cmd(0x0b);			/* Frame Cycle Control */
	SSD1283A_wr_dat(0x580c);		/* Amount of Overlap 4 clock cycle */
									/* Delay amount of the source output 2 clock cycle */
									/* EQ period 2 clock cycle, Division Ratio 2 */

	SSD1283A_wr_cmd(0x12);			/* Power Control 3 */
	SSD1283A_wr_dat(0x0609);		/* Set amplitude magnification of VLCD63 = Vref x 2.175 */

	SSD1283A_wr_cmd(0x13);			/* Power Control 4 */
	SSD1283A_wr_dat(0x3100);		/* Set output voltage of VcomL = VLCD63 x 1.02 */

	SSD1283A_wr_cmd(0x2a);			/* Test Commands ? */
	SSD1283A_wr_dat(0x1dd0);

	SSD1283A_wr_cmd(0x2b);			/* Test Commands ? */
	SSD1283A_wr_dat(0x0a90);

	SSD1283A_wr_cmd(0x2d);			/* Test Commands ? */
	SSD1283A_wr_dat(0x310f);
	_delay_ms(100);

	SSD1283A_wr_cmd(0x44);			/* Set CAS Address */
	SSD1283A_wr_dat(0x8300);

	SSD1283A_wr_cmd(0x45);			/* Set RAS Address */
	SSD1283A_wr_dat(0x8300);
	
	SSD1283A_wr_cmd(0x21);			/* Set RAM Address */
	SSD1283A_wr_dat(0x0000);

	SSD1283A_wr_cmd(0x1e);			/* Power Control 5 */ 
	SSD1283A_wr_dat(0x00bf);  		/* VcomH = VLCD63 x 0.99 */
	_delay_ms(1);
	
	SSD1283A_wr_cmd(0x1e);			/* Power Control 5 */ 
	SSD1283A_wr_dat(0x0000);		/* VcomH =VLCD63 x 0.36 */
	_delay_ms(100);

	SSD1283A_clear ();

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1283A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1283A_wr_dat(COL_RED);
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
int SSD1283A_draw_bmp(const uint8_t* ptr){

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
	SSD1283A_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		SSD1283A_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			SSD1283A_wr_dat(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
