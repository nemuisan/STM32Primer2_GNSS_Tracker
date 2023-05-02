/********************************************************************************/
/*!
	@file			st7732.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.05.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Display Device Driver for STM32 Primer2

    @section HISTORY
		2010.03.24	V1.00	Stable Release.
		2010.09.06	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added GRAM write function.
		2011.03.10	V4.00	C++ Ready.
		2023.05.01	V5.00	Removed unused delay function.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "st7732.h"
/* check header file version for fool proof */
#if ST7732_H != 0x0500
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
void ST7732_reset(void)
{
	ST7732_RES_SET();							/* RES=H			   		*/
	_delay_ms(100);								/* wait 1ms     			*/

	ST7732_RES_CLR();							/* RES=L, CS=L   			*/
	ST7732_CS_CLR();
	_delay_ms(100);								/* wait 100ms     			*/

	ST7732_RES_SET();						  	/* RES=H		 			*/
	_delay_ms(100);				    			/* wait 100 ms    			 */
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7732_wr_cmd(uint8_t cmd)
{
	/* Transfer command to the FSMC */
	ST7732_CMD = (cmd<<4);
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ST7732_wr_dat(uint8_t dat)
{
	/* Transfer data to the FSMC */
	ST7732_DATA = (dat<<4);
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7732_wr_gram(uint16_t gram)
{
	/* Transfer data to the FSMC */
	ST7732_DATA = (gram>>4);
	ST7732_DATA = (gram<<4);
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7732_wr_block(uint8_t *p,unsigned int cnt)
{
	int n;

	n = cnt % 4;
	cnt /= 4;
	while (cnt--) {
		ST7732_wr_dat(*p++);
		ST7732_wr_dat(*p++);
		ST7732_wr_dat(*p++);
		ST7732_wr_dat(*p++);
	}
	while (n--) {
		ST7732_wr_dat(*p++);
	}
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ST7732_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	/* Set CAS Address */
	ST7732_wr_cmd(CASET); 
	ST7732_wr_dat(0);
	ST7732_wr_dat(OFS_COL+x);
	ST7732_wr_dat(0);
	ST7732_wr_dat(OFS_COL+width); 
	
	/* Set RAS Address */
	ST7732_wr_cmd(RASET);
	ST7732_wr_dat(0);
	ST7732_wr_dat(OFS_RAW+y);
	ST7732_wr_dat(0);
	ST7732_wr_dat(OFS_RAW+height);
	
	/* Write RAM */
	ST7732_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ST7732_clear(void)
{
	volatile uint32_t n;

	ST7732_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);
	
	do {
		ST7732_wr_gram(COL_WHITE);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ST7732_init(void)
{
	Display_IoInit_If();

	ST7732_reset();
	
	/* ST7732 */
	ST7732_wr_cmd(SWRESET);		/* Software Reset */
	_delay_ms(180);				/* above 150mSec */
	
	ST7732_wr_cmd(SLPIN);		/* Sleep In */
	_delay_ms(100);
	ST7732_wr_cmd(SLPOUT);		/* Sleep Out */
	_delay_ms(150);
	
	ST7732_wr_cmd(FRMCTR1);     /* Frame Rate Control (In normal mode/ Full colors) */
	ST7732_wr_dat(0x06);		/* Set RTNA2 */
	ST7732_wr_dat(0x03);		/* Set FPA2 */
	ST7732_wr_dat(0x02);		/* Set BPA2 */

	ST7732_wr_cmd(INVCTR);		/* Set Display Inversion Control */
	ST7732_wr_dat(0x03);		/* Set to Flame inversion */

	ST7732_wr_cmd(DISSET5);		/* Display Function set 5 */
	ST7732_wr_dat(0x02);		/* Amount of non-overlap of the gate output 						:1 clock cycle */
								/* delay amount from gate signal falling edge of the source output  :1 clock cycle */
								/* Set the Equalizing period										:4 clock cycle */
	ST7732_wr_dat(0x0E);		/* Normal scan,Source output on non-display area AGND,VCOM output on non-display area AGND */

	ST7732_wr_cmd(DISPCTRL);	/* OTP and OSC function control */
	ST7732_wr_dat(0x1A);		/* PDM:VCOML,FLM:Disable the monochrom image detection function
								   C8ON:Enable the monochrom line detection function
								   EQPW:VDD(+frame), GND(-frame), ps. EQVDD=f1f
								   EQVDD:VDD(+frame), GND(-frame) mode, ps. EQPW=f10f
								   ENGS:Gate ScanDirection G1~G162 */

	ST7732_wr_cmd(PWCTR1);		/* Power Control 1*/
	ST7732_wr_dat(0x02);		/* GVDD=4.70V */
	ST7732_wr_dat(0x00);		/* VCI1=2.75V */

	ST7732_wr_cmd(PWCTR2);		/* Power Control 2 */
	ST7732_wr_dat(0x05);		/* AVDD= 5.49V,VCL= -2.74, VGHH= 14.70, VGLL= -9.80 */

	ST7732_wr_cmd(PWCTR3);		/* Power Control 3 */
	ST7732_wr_dat(0x02);		/* Amount of Current in Operational Amplifier :Medium Low */
	ST7732_wr_dat(0x02);		/* Set the Booster circuit Step-up cycle in Normal mode/ full colors. */
								/* circuit 1 :BCLK / 1, circuit 2,3 :BCLK / 4 */

	ST7732_wr_cmd(PWCTR4);		/* Power Control 4 */
	ST7732_wr_dat(0x01);		/* Amount of Current in Operational Amplifier :Small */
	ST7732_wr_dat(0x02);		/* Set the Booster circuit Step-up cycle in Idle mode/8 colors. */
								/* circuit 1 :BCLK / 1, circuit 2,3 :BCLK / 4 */

	ST7732_wr_cmd(PWCTR5);		/* Power Control 5 */
	ST7732_wr_dat(0x01);		/* Amount of Current in Operational Amplifier :Small */
	ST7732_wr_dat(0x02);		/* Set the Booster circuit Step-up cycle in Partial mode/ full-colors. */
								/* circuit 1 :BCLK / 1, circuit 2,3 :BCLK / 4 */

	ST7732_wr_cmd(VMCTR1);		/* VCOM Control 1 */
	ST7732_wr_dat(0x47);		/* VCOMH=+4.275V */
	ST7732_wr_dat(0x3A);		/* VCOML=-1.050V */

	ST7732_wr_cmd(OSCADJ);		/* Internal OSC frequency control */
	ST7732_wr_dat(0x02);		/* 80Hz */

	ST7732_wr_cmd(DEFADJ);		/* Default mode setting */
	ST7732_wr_dat(0x4C);		/* Disable the output mode of TESEL pin */

	ST7732_wr_cmd(0xF8);		/* SLEEP OUT LOAD DEFAULT*/
	ST7732_wr_dat(0x06);		/* Enable */

	ST7732_wr_cmd(GAMCTRP1);    /* Set Gamma correction Polarity + */	          
	ST7732_wr_dat(0x06);
	ST7732_wr_dat(0x1c);
	ST7732_wr_dat(0x1f);
	ST7732_wr_dat(0x1f);
	ST7732_wr_dat(0x18);
	ST7732_wr_dat(0x13);
	ST7732_wr_dat(0x06);
	ST7732_wr_dat(0x03);
	ST7732_wr_dat(0x03);
	ST7732_wr_dat(0x04);
	ST7732_wr_dat(0x07);
	ST7732_wr_dat(0x07);
	ST7732_wr_dat(0x00);

	ST7732_wr_cmd(GAMCTRN1);    /* Set Gamma correction Polarity - */	          
	ST7732_wr_dat(0x0a);
	ST7732_wr_dat(0x14);
	ST7732_wr_dat(0x1b);
	ST7732_wr_dat(0x18);
	ST7732_wr_dat(0x12);
	ST7732_wr_dat(0x0e);
	ST7732_wr_dat(0x02);
	ST7732_wr_dat(0x01);
	ST7732_wr_dat(0x00);
	ST7732_wr_dat(0x01);
	ST7732_wr_dat(0x08);
	ST7732_wr_dat(0x07);
	ST7732_wr_dat(0x00);

	ST7732_wr_cmd(MADCTL);		/* Memory Data Access Control */
								/* -This command defines read/ write scanning direction of frame memory. */
	ST7732_wr_dat(0xC0); 
								/* MY=1,MX=1,MV=0,ML=0(LCD vertical refresh Top to Bottom) */
								/* Color selector switch control :RGB color filter panel */

	ST7732_wr_cmd(COLMOD);		/* Interface Pixel Format */
	ST7732_wr_dat(0x55);		/* 16-bit/pixel R5G6B5 */

	ST7732_wr_cmd(TEON);		/* Tearing Effect Line ON */
	ST7732_wr_dat(0x00);		/* OFF */
	
	ST7732_clear();
	
	ST7732_wr_cmd(DISPON);		/* Display On */
	_delay_ms(20);


#if 0	/* test code RED */
	volatile uint32_t n;
	
	ST7732_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ST7732_wr_gram(COL_RED);
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
int ST7732_draw_bmp(const uint8_t* ptr){

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
	ST7732_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		ST7732_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			ST7732_wr_gram(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
