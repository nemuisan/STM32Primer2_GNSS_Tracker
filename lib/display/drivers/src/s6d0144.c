/********************************************************************************/
/*!
	@file			s6d0144.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2012.01.21
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -T18DES10					(S6D0144)	8/16bit mode.		@n
					 -TFT2P0327-E               (S6D0151)   8/16bit,spi mode.

    @section HISTORY
		2011.12.23	V1.00	Stable Release.
		2012.01.21	V2.00	Added S6D0151 Device & SPI Support.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d0144.h"
/* check header file version for fool proof */
#if __S6D0144_H != 0x0200
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#if   defined(S6D0144SPI_3WIREMODE)
 #warning "USE S6D144/S6D151 as 3-Wire SPIMode!"
#elif defined(S6D0144SPI_4WIREMODE)
 #warning "USE S6D144/S6D151 as 4-Wire SPIMode!"
#endif

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
inline void S6D0144_reset(void)
{
#ifdef USE_S6D0144_TFT
	S6D0144_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D0144_RD_SET();
	S6D0144_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S6D0144_RES_CLR();							/* RES=L, CS=L   			*/
	S6D0144_CS_CLR();

#elif  USE_S6D0144_SPI_TFT
	S6D0144_RES_SET();							/* RES=H, CS=H				*/
	S6D0144_CS_SET();
	S6D0144_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	S6D0144_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(60);								/* wait 60ms     			*/
	S6D0144_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_S6D0144_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D0144_wr_cmd(uint16_t cmd)
{
	S6D0144_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D0144_CMD = 0;
	S6D0144_WR();
#endif

	S6D0144_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	S6D0144_WR();								/* WR=L->H					*/

	S6D0144_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6D0144_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D0144_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	S6D0144_WR();								/* WR=L->H					*/
#endif

	S6D0144_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	S6D0144_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D0144_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6D0144_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		S6D0144_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D0144_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	S6D0144_wr_cmd(cmd);
	S6D0144_WR_SET();

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


#elif USE_S6D0144_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D0144_wr_cmd(uint16_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(START_WR_CMD);
	SendSPI16(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void S6D0144_wr_dat(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(START_WR_DATA);
	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D0144_wr_block(uint8_t *p,unsigned int cnt)
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
inline  uint16_t S6D0144_rd_cmd(uint16_t cmd)
{
#if defined(S6D0144SPI_3WIREMODE)
	uint16_t val;
	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI16(cmd);
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	

	DISPLAY_ASSART_CS();						/* CS=L		     */
	SendSPI(START_RD_DATA);
	temp = RecvSPI();							/* Dummy Read */
	temp = RecvSPI();							/* Upper Read */
	val  = RecvSPI();							/* Lower Read */

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
#else 
 #warning "4-Wire SPIMode Does NOT Implemented Read Function..."
	return 0x0151;								/* TFT2P0327-E Module does NOT have SDO Pin...S**K!! */
#endif
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6D0144_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S6D0144_wr_cmd(0x44);	/* Horizontal Start,End ADDR */
	S6D0144_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	S6D0144_wr_cmd(0x45);	/* Vertical Start,End ADDR */
	S6D0144_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	S6D0144_wr_cmd(0x21);	/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	S6D0144_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	S6D0144_wr_cmd(0x22);	/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D0144_clear(void)
{
	volatile uint32_t n;

	S6D0144_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D0144_wr_dat(COL_BLACK);
	} while (--n);

}



/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6D0144_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	S6D0144_reset();

	/* Check Device Code */
	devicetype = S6D0144_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0144)
	{
		/* Initialize S6D0144 */
		_delay_ms(1);   
		S6D0144_wr_cmd(0xB4);
		S6D0144_wr_dat(0x0010);   
		S6D0144_wr_cmd(0x07);
		S6D0144_wr_dat(0x0000);   
		S6D0144_wr_cmd(0x13);
		S6D0144_wr_dat(0x0000);   
		_delay_ms(1);   
		S6D0144_wr_cmd(0x12);
		S6D0144_wr_dat(0x0071);   
		S6D0144_wr_cmd(0x13);
		S6D0144_wr_dat(0x080b);   
		S6D0144_wr_cmd(0x14);
		S6D0144_wr_dat(0x78dd);   
		S6D0144_wr_cmd(0x10);
		S6D0144_wr_dat(0x1910);   

		S6D0144_wr_cmd(0x01);
		S6D0144_wr_dat(0x0114);   
		S6D0144_wr_cmd(0x02);
		S6D0144_wr_dat(0x0100);   
		S6D0144_wr_cmd(0x03);
		S6D0144_wr_dat(0x0030);   
		S6D0144_wr_cmd(0x08);
		S6D0144_wr_dat(0x0101);   
		S6D0144_wr_cmd(0x0b);
		S6D0144_wr_dat(0x0005);   
		S6D0144_wr_cmd(0x0C);
		S6D0144_wr_dat(0x0002);   
		_delay_ms(10);   


		S6D0144_wr_cmd(0x30);			/* GAMMA Control */   
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x31);
		S6D0144_wr_dat(0x0506);   
		S6D0144_wr_cmd(0x32);
		S6D0144_wr_dat(0x0403);   
		S6D0144_wr_cmd(0x33);
		S6D0144_wr_dat(0x0200);   
		S6D0144_wr_cmd(0x34);
		S6D0144_wr_dat(0x0303);   
		S6D0144_wr_cmd(0x35);
		S6D0144_wr_dat(0x0002);   
		S6D0144_wr_cmd(0x36);
		S6D0144_wr_dat(0x0707);   
		S6D0144_wr_cmd(0x37);
		S6D0144_wr_dat(0x0200);   
		S6D0144_wr_cmd(0x38);
		S6D0144_wr_dat(0x0900);   
		S6D0144_wr_cmd(0x11);
		S6D0144_wr_dat(0x000a);   

		S6D0144_wr_cmd(0x40);			/* Coordination Control setting */  
		S6D0144_wr_dat(0x0000);		
		S6D0144_wr_cmd(0x41);
		S6D0144_wr_dat(0x00E5);   
		S6D0144_wr_cmd(0x42);
		S6D0144_wr_dat(0x9f00);   
		S6D0144_wr_cmd(0x43);
		S6D0144_wr_dat(0x9f00);   
		S6D0144_wr_cmd(0x44);
		S6D0144_wr_dat(0x7F00);   
		S6D0144_wr_cmd(0x45);
		S6D0144_wr_dat(0x9f00);   

		S6D0144_wr_cmd(0x13);
		S6D0144_wr_dat(0x081b);   
		_delay_ms(50);   
		S6D0144_wr_cmd(0x07);
		S6D0144_wr_dat(0x0037);			/* DTE =1 */ 
		_delay_ms(20);   
	}

	else if(devicetype == 0x0151)
	{
		/* Initialize S6D0151 */
		S6D0144_wr_cmd(0x07);
		S6D0144_wr_dat(0x0020);
		S6D0144_wr_cmd(0xb6);			/* Module Vendor */
		S6D0144_wr_dat(0x013f);
		S6D0144_wr_cmd(0xB4);			/* MTP Control */
		S6D0144_wr_dat(0x0010);

		/* power control setting */     
		S6D0144_wr_cmd(0x12);			/* Power control 2 */
		S6D0144_wr_dat(0x00b1);
		_delay_ms(10);                  
		S6D0144_wr_cmd(0x13);			/*  Power control 3 (R13h) */
		S6D0144_wr_dat(0x080e);
		_delay_ms(10);                  
		S6D0144_wr_cmd(0x14);			/* Power control 4 (R14h) */
		S6D0144_wr_dat(0x5BC9); 		
		_delay_ms(10);                  
		S6D0144_wr_cmd(0x61);			/* Oscillator control */
		S6D0144_wr_dat(0x0018);
		S6D0144_wr_cmd(0x10);			/* Power Control 1 */
		S6D0144_wr_dat(0x190C);
		_delay_ms(80);                  
		S6D0144_wr_cmd(0x13);         
		S6D0144_wr_dat(0x081e);         
		_delay_ms(50);

		S6D0144_wr_cmd(0x01);			/* Driver Output Control */
		S6D0144_wr_dat((1<<12)|(0<<9)|(0<<8)|(1<<4)|(1<<2)); 		
		S6D0144_wr_cmd(0x02);			/* LCD Inversion Control */
		S6D0144_wr_dat(0x0100);
		S6D0144_wr_cmd(0x03);			/* Entry Mode */
		S6D0144_wr_dat((1<<5)|(1<<4));	/*S6D0144_wr_dat(0x0030);*/
		S6D0144_wr_cmd(0x08);			/* Blank period control 1 */
		S6D0144_wr_dat(0x0202);
		S6D0144_wr_cmd(0x0b);			/* Frame cycle control */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x0C);			/* External Display Interface Control */
		S6D0144_wr_dat(0x0000);
  
		S6D0144_wr_cmd(0x61);			/* Oscillator control */
		S6D0144_wr_dat(0x0018);
		S6D0144_wr_cmd(0x69);			/* DC/DC convert low power mode */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x70);			/* Source Driver pre-driving period setting */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x71);			/* Gate Output Period Control */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x11);			/* Gamma control 1 */
		S6D0144_wr_dat(0x0000);
   
		/* GAMMA CONTROL */             
		S6D0144_wr_cmd(0x30);			/* Gamma control 2 (R30h to R37h) */
		S6D0144_wr_dat(0x0303);
		S6D0144_wr_cmd(0x31);         
		S6D0144_wr_dat(0x0303);         
		S6D0144_wr_cmd(0x32);         
		S6D0144_wr_dat(0x0303);         
		S6D0144_wr_cmd(0x33);         
		S6D0144_wr_dat(0x0000);         
		S6D0144_wr_cmd(0x34);         
		S6D0144_wr_dat(0x0404);         
		S6D0144_wr_cmd(0x35);         
		S6D0144_wr_dat(0x0404);         
		S6D0144_wr_cmd(0x36);         
		S6D0144_wr_dat(0x0404);         
		S6D0144_wr_cmd(0x37);         
		S6D0144_wr_dat(0x0000);         
		S6D0144_wr_cmd(0x38);         
		S6D0144_wr_dat(0x0707);         
		_delay_ms(5);

		/* Position setting */          
		S6D0144_wr_cmd(0x40);			/* Gate scan position */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x42);			/* Screen driving position */
		S6D0144_wr_dat(0x9F00);
		S6D0144_wr_cmd(0x43);			/* Screen driving position */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x44);			/* Horizontal  address position */
		S6D0144_wr_dat(0x7F00);
		S6D0144_wr_cmd(0x45);			/* Vertical RAM address position */
		S6D0144_wr_dat(0x9F00);
		S6D0144_wr_cmd(0x21);			/* GRAM address set */
		S6D0144_wr_dat(0x0000);
		_delay_ms(20);

		S6D0144_wr_cmd(0x69);			/* DC/DC convert low power mode */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x70);			/* Source Driver pre-driving period setting */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x71);			/* Gate Output Period Control */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0x73);			/* Test_Key */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0xB3);			/* DC/DC converter Clock Source Selection */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0xBD);			/* MTP Data Read */
		S6D0144_wr_dat(0x0000);
		S6D0144_wr_cmd(0xBE);			/* Interface Mode Selection */
		S6D0144_wr_dat(0x0000);

		S6D0144_wr_cmd(0x07);			/* Display control */
		S6D0144_wr_dat(0x0020);
		_delay_ms(5);                   
		S6D0144_wr_cmd(0x07);         
		S6D0144_wr_dat(0x0021);         
		S6D0144_wr_cmd(0x07);         
		S6D0144_wr_dat(0x0027);         
		_delay_ms(50);                  
		S6D0144_wr_cmd(0x07);
		S6D0144_wr_dat(0x0037);
		_delay_ms(10);
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6D0144_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6D0144_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		S6D0144_wr_dat(COL_RED);
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
int S6D0144_draw_bmp(const uint8_t* ptr){

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
	S6D0144_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		S6D0144_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			/* 262k colour access */
			S6D0144_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
