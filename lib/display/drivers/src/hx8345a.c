/********************************************************************************/
/*!
	@file			hx8345a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.03.31
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CM2111 TFT module(16bit-bus only).

    @section HISTORY
		2012.03.31	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8345a.h"
/* check header file version for fool proof */
#if __HX8345A_H != 0x0100
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
inline void HX8345A_reset(void)
{
#ifdef USE_HX8345A_TFT
	HX8345A_RES_SET();								/* RES=H, RD=H, WR=H   		*/
	HX8345A_RD_SET();
	HX8345A_WR_SET();
	_delay_ms(5);									/* wait 5ms     			*/

	HX8345A_RES_CLR();								/* RES=L, CS=L   			*/
	HX8345A_CS_CLR();

#elif  USE_HX8345A_SPI_TFT
	HX8345A_RES_SET();								/* RES=H, CS=H				*/
	HX8345A_CS_SET();
	HX8345A_SCK_SET();								/* SPI MODE3     			*/
	_delay_ms(5);									/* wait 5ms     			*/

	HX8345A_RES_CLR();								/* RES=L		   			*/

#endif

	_delay_ms(10);									/* wait 10ms     			*/
	HX8345A_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    				/* wait at least 120ms     	*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_HX8345A_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8345A_wr_cmd(uint8_t cmd)
{
	HX8345A_DC_CLR();								/* DC=L						*/
	
	HX8345A_CMD = cmd;								/* cmd(8bit_Low or 16bit)	*/
	HX8345A_WR();									/* WR=L->H					*/
	
	HX8345A_DC_SET();								/* DC=H 					*/
}

/**************************************************************************/
/*! 
    Write LCD DAT & GRAM.
*/
/**************************************************************************/
inline void HX8345A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8345A_DATA = (uint8_t)(dat>>8);				/* upper 8bit data			*/
	HX8345A_WR();									/* WR=L->H					*/
	HX8345A_DATA = (uint8_t)dat;					/* lower 8bit data			*/
#else
	HX8345A_DATA = dat;								/* 16bit data 				*/
#endif
	HX8345A_WR();									/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8345A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
			/* avoid -Wsequence-point's warning */
		HX8345A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HX8345A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX8345A_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	HX8345A_wr_cmd(cmd);
	HX8345A_WR_SET();

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


#elif USE_HX8345A_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8345A_wr_cmd(uint8_t cmd)
{
	DISPLAY_ASSART_CS();							/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();							/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8345A_wr_dat(uint16_t dat)
{	
	DISPLAY_ASSART_CS();							/* CS=L		     */
	
	SendSPI(START_WR_DATA);
	SendSPI16(dat);

	DISPLAY_NEGATE_CS();							/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8345A_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();							/* CS=L		     */
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

	DISPLAY_NEGATE_CS();							/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX8345A_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint8_t temp;

	DISPLAY_ASSART_CS();							/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI16(cmd);
	
	DISPLAY_NEGATE_CS();							/* CS=H		     */
	

	DISPLAY_ASSART_CS();							/* CS=L		     */
	
	SendSPI(START_RD_DATA);
	temp = RecvSPI();								/* Dummy Read */
	temp = RecvSPI();								/* Upper Read */
	val  = RecvSPI();								/* Lower Read */

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;
	
	DISPLAY_NEGATE_CS();							/* CS=H		     */

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8345A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8345A_wr_cmd(0x16);					/* Horizontal Start,End ADDR */
	HX8345A_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	HX8345A_wr_cmd(0x17);					/* Vertical Start,End ADDR */
	HX8345A_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	HX8345A_wr_cmd(0x21);					/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	HX8345A_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));
	
	HX8345A_wr_cmd(0x22);					/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8345A_clear(void)
{
	volatile uint32_t n;

	HX8345A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8345A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8345A_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	HX8345A_reset();

	/* Check Device Code */
	devicetype = HX8345A_rd_cmd(0x00);  				/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8345)
	{
		/* Initialize HX8345A */
		/* display control */    
		HX8345A_wr_cmd(0x00);
		HX8345A_wr_dat(0x0001);			/* OSC_EN=1 */
		_delay_ms(40);
		/* power on */
		HX8345A_wr_cmd(0x07);
		HX8345A_wr_dat(0x0000);			/* PT=2'h0,VLE=2'h0,SPT=0,GON=0,DTE=0,CL=0,REV=0,D=2'h0(S384~S1=GND) */
		HX8345A_wr_cmd(0x0D);
		HX8345A_wr_dat(0x0000);			/* PON=0,VRH=4'h0000(Vreg1out=REGP*1.33) */
		HX8345A_wr_cmd(0x0E);
		HX8345A_wr_dat(0x0000);			/* VCOMG=0(VCOML=GND,VDV),VDV=5'h0(VCOM=0.6*Vreg1out),VCM=5'h0(VCOMH=0.4*Vreg1out) */
		_delay_ms(20);
		HX8345A_wr_cmd(0x0C);
		HX8345A_wr_dat(0x0001);			/* VC=3'h0(Vci1,Vreg1outVREGP=0.92*Vci) */
		HX8345A_wr_cmd(0x0D);
		HX8345A_wr_dat(0x0003);			/* PON=0,VRH=4'h3(Vreg1out=REGP*1.65) */
		HX8345A_wr_cmd(0x0E);
		HX8345A_wr_dat(0x1017);			/* VCOMG=1(VCOML,VDV),VDV=5'h0(VCOM=0.6*Vreg1out),VCM=5'h17(VCOMH=0.76*Vreg1out) */
		HX8345A_wr_cmd(0x09);
		HX8345A_wr_dat(0x0008);			/* DCM=0,DC12~10=000(Fdcdc2=fosc/8),DK=1,SAP=3'h0 */
		HX8345A_wr_cmd(0x03);
		HX8345A_wr_dat(0x0010);			/* BT=3'h0(VLCD=2*Vci1,VCL=-Vci1,VGH=6*Vci1,VGL=-5*Vci1),DC0=3'h0(Fdcdc1=fosc/4),AP=3'h4,SLP=0,STB=0 */
		HX8345A_wr_cmd(0x09);
		HX8345A_wr_dat(0x0000);			/* DCM=0,DC12~10=000(Fdcdc2=fosc/8),DK=0,SAP=3'h0 */
		HX8345A_wr_cmd(0x0D);
		HX8345A_wr_dat(0x0013);			/* PON=1(),VRH=4'h3(Vreg1out=REGP*1.65) */
		_delay_ms(20);             
		HX8345A_wr_cmd(0x03);
		HX8345A_wr_dat(0x0110);			/* BT=3'h0(VLCD=2*Vci1,VCL=-Vci1,VGH=6*Vci1,VGL=-4*Vci1),DC0=3'h0(Fdcdc1=fosc/4),AP=3'h4,SLP=0,STB=0 */
		HX8345A_wr_cmd(0x09);
		HX8345A_wr_dat(0x0004);			/* DCM=0,DC12~10=000(Fdcdc2=fosc/8),DK=0,SAP=3'h4 */
		HX8345A_wr_cmd(0x0E);
		HX8345A_wr_dat(0x361c);			/* VCOMG=1(VCOML,VDV),VDV=5'h10(VCOM=1.23*Vreg1out),VCM=5'h1B(VCOMH=0.94*Vreg1out)  */
		/* display control */ 
		HX8345A_wr_cmd(0x01);
		HX8345A_wr_dat(0x0113);			/* SM=0,GS=0(G1~G160),SS=1(S384~S1),NL=5'h13 */
		HX8345A_wr_cmd(0x02);
		HX8345A_wr_dat(0x0700);			/* FLD=2'h1,B/C=1,EOR=1,NW=5'h0 */
		HX8345A_wr_cmd(0x05);
		HX8345A_wr_dat(0x0030);			/* BGR=0,ID=2'h3,AM=0,LG=3'h0 */
		HX8345A_wr_cmd(0x08);
		HX8345A_wr_dat(0x0404);			/* VSPL=0,HSPL=0,DPL=0,EPL=0,FP=4'h4,ISC=4'h0,BP=4'h6 */
		HX8345A_wr_cmd(0x0A);
		HX8345A_wr_dat(0x0000);			/* TRI=0,DFM=2'h0,PTG=2'h0,RM=0,DM=2'h0,RIM=2'h0 */
		HX8345A_wr_cmd(0x10);
		HX8345A_wr_dat(0x0000);			/* F=0(CP/WM=0,16) */
		/* panel interface control */
		HX8345A_wr_cmd(0x0B);
		HX8345A_wr_dat(0x1800);			/* GD=2'h0,SDT=2'h1,CE=2'h2,DIVI=2'h0(Fosc/1.0),RTNI=4'h0(1H=16*OSC) */
		/* BASE IMAGE DISPLAY control */   
		HX8345A_wr_cmd(0x0F);
		HX8345A_wr_dat(0x0000);			/* SCN=4'h0 */
		/* WINDOW ADDRESS control */   
		HX8345A_wr_cmd(0x16);
		HX8345A_wr_dat(0x7F00);			/* 00h<HSA(7-0)<HEA(7-0)<7Fh */
		HX8345A_wr_cmd(0x17);
		HX8345A_wr_dat(0x9F00);			/* 00h<VSA(7-0)<VEA(7-0)<9Fh */
		/* RAM control */
		HX8345A_wr_cmd(0x21);
		HX8345A_wr_dat(0x0000);			/* (GRAM=0000---G1) */
		/* power on */
		_delay_ms(20);
		HX8345A_wr_cmd(0x07);
		HX8345A_wr_dat(0x0001);			/* PT=2'h0,VLE=2'h0,SPT=0,GON=0,DTE=0,CL=0(262K),REV=0,D=2'h1(S384~S1=GND) */
		_delay_ms(20);
		HX8345A_wr_cmd(0x07);
		HX8345A_wr_dat(0x0025);			/* PT=2'h0,VLE=2'h0,SPT=0,GON=1,DTE=0,CL=0,REV=0,D=2'h1(S384~S1=GND) */
		HX8345A_wr_cmd(0x07);
		HX8345A_wr_dat(0x0027);			/* PT=2'h0,VLE=2'h0,SPT=0,GON=1,DTE=0,CL=0(262K),REV=0,D=2'h3(S384~S1) */
		_delay_ms(20);             
		HX8345A_wr_cmd(0x07);
		HX8345A_wr_dat(0x0037);			/* PT=2'h0,VLE=2'h0,SPT=0,GON=1,DTE=1,CL=0(262K),REV=1,D=2'h3(S384~S1) */

	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8345A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8345A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8345A_wr_dat(COL_RED);
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
int HX8345A_draw_bmp(const uint8_t* ptr){

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
	HX8345A_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX8345A_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX8345A_wr_dat(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
