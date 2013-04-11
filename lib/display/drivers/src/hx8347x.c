 /********************************************************************************/
/*!
	@file			hx8347x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2013.02.28
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95300					(HX8347A)	8/16bit mode.		@n
					 -STM032QVT-003				(HX8347A)	8/16bit mode.		@n
					 -DST6007					(HX8347D)	8/16bit mode.		@n
					 -MI0283QT-2	   			(HX8347D)	8/16bit&SPI mode. 	@n
					 -CD028THT22V2			  	(HX8347G(T))8/16bit&SPI mode. 	@n
					 -TFT8K0943FPC-A1-E		  	(HX8346A)	16bit mode.

    @section HISTORY
		2011.12.23	V1.00	Renewed From HX8347A driver.
		2012.03.31	V2.00	Added HX8347G(T) driver.
		2012.09.30	V3.00	Revised HX8346A driver.
		2013.02.28  V4.00	Optimized Some Codes.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8347x.h"
/* check header file version for fool proof */
#if __HX8347X_H != 0x0400
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#if defined(LCD_FASTESTBUS_WORKAROUND) && !defined(USE_HX8347x_SPI_TFT)
 #warning "U Might Need for FASTESTBUS_WORKAROUND in use of STM32F4xx-FSMC for HX8347A ,but HX8347D/G does NOT needed... "
 /* You should insert stupid delay and can change delay value */
 #define DELAY_WHEEL	27
#endif

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
#if defined(LCD_FASTESTBUS_WORKAROUND) && !defined(USE_HX8347x_SPI_TFT)
 /* Pointer to the Bus Sleep */
 void(*Bus_Sleep)(volatile int ms);
#endif

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
inline void HX8347x_reset(void)
{
#ifdef USE_HX8347x_TFT
	HX8347x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8347x_RD_SET();
	HX8347x_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/
	
	HX8347x_RES_CLR();							/* RES=L, CS=L   			*/
	HX8347x_CS_CLR();

#elif  USE_HX8347x_SPI_TFT
	HX8347x_RES_SET();							/* RES=H, CS=H				*/
	HX8347x_CS_SET();
	HX8347x_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	HX8347x_RES_CLR();							/* RES=L		   			*/
#endif

	_delay_ms(10);								/* wait 10ms     			*/
	
	HX8347x_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait 125ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_HX8347x_TFT
#ifdef LCD_FASTESTBUS_WORKAROUND
/**************************************************************************/
/*! 
    Stupid Delay Routine for Fastest Bus (like STM32F4xx FSMC).
*/
/**************************************************************************/
void Bus_Sleep_hx8347a(volatile int ms)
{
	while (ms--);
}
void Bus_Sleep_hx8347dgi(volatile int ms)
{
}
#endif

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8347x_wr_cmd(uint8_t cmd)
{
	HX8347x_DC_CLR();							/* DC=L						*/

	HX8347x_CMD = cmd;							/* cmd						*/
	HX8347x_WR();								/* WR=L->H					*/
	
#ifdef LCD_FASTESTBUS_WORKAROUND
	Bus_Sleep(DELAY_WHEEL);						/* Need for HX8347A			*/
#endif

	HX8347x_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8347x_wr_dat(uint8_t dat)
{
	HX8347x_DATA = dat;							/* data						*/
	HX8347x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8347x_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8347x_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	HX8347x_WR();								/* WR=L->H					*/
	HX8347x_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	HX8347x_DATA = gram;						/* 16bit data 				*/
#endif
	HX8347x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8347x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8347x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8347x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8347x_rd_cmd(uint8_t cmd)
{
	uint8_t val;

	HX8347x_wr_cmd(cmd);
	HX8347x_WR_SET();

	_delay_ms(1);		/* Some Wait Must be Need on HX8347A, Nemui-San Said So... */
    ReadLCDData(val);

	val &= 0x00FF;
	return val;
}


#elif USE_HX8347x_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8347x_wr_cmd(uint8_t cmd)
{
	HX8347x_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef HX8347xSPI_3WIREMODE
	SendSPI(START_WR_CMD);
#endif
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	HX8347x_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8347x_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef HX8347xSPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8347x_wr_gram(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef HX8347xSPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif
	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8347x_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */
#ifdef HX8347xSPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif

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
inline uint8_t HX8347x_rd_cmd(uint8_t cmd)
{
#ifdef HX8347xSPI_4WIREMODE
 #warning "SPI4Wire Mode Supports ONLY HX8347D/G Chips!"
 #warning "HX8347D USES SDA(Input&Output Multiplexed) Line!"
#endif

	uint8_t val;

	DISPLAY_ASSART_CS();						/* CS=L		     */
#ifdef HX8347xSPI_3WIREMODE
	SendSPI(START_WR_CMD);
#endif
	SendSPI16(cmd);
	DISPLAY_NEGATE_CS();						/* CS=H		     */


	HX8347x_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
#ifdef HX8347xSPI_3WIREMODE
	SendSPI(START_RD_DATA);
#endif
	val  = RecvSPI();
	val &= 0x00FF;

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	HX8347x_DC_SET();							/* DC=L		     */
	
	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8347x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8347x_wr_cmd(0x02);				/* Horizontal RAM Start ADDR2 */
	HX8347x_wr_dat((OFS_COL + x)>>8);
	HX8347x_wr_cmd(0x03);				/* Horizontal RAM Start ADDR1 */
	HX8347x_wr_dat(OFS_COL + x);
	HX8347x_wr_cmd(0x04);				/* Horizontal RAM End ADDR2 */
	HX8347x_wr_dat((OFS_COL + width)>>8);
	HX8347x_wr_cmd(0x05);				/* Horizontal RAM End ADDR1 */
	HX8347x_wr_dat(OFS_COL + width);
	HX8347x_wr_cmd(0x06);				/* Vertical RAM Start ADDR2 */
	HX8347x_wr_dat((OFS_RAW + y)>>8);
	HX8347x_wr_cmd(0x07);				/* Vertical RAM Start ADDR1 */
	HX8347x_wr_dat(OFS_RAW + y);
	HX8347x_wr_cmd(0x08);				/* Vertical RAM End ADDR2 */
	HX8347x_wr_dat((OFS_RAW + height)>>8);
	HX8347x_wr_cmd(0x09);				/* Vertical RAM End ADDR1 */
	HX8347x_wr_dat(OFS_RAW + height);

	HX8347x_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8347x_clear(void)
{
	volatile uint32_t n;

	HX8347x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8347x_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8347x_init(void)
{
	uint8_t id8347d,id834xA;
	
	#ifdef LCD_FASTESTBUS_WORKAROUND
	 Bus_Sleep = Bus_Sleep_hx8347dgi;
	#endif

	Display_IoInit_If();

	HX8347x_reset();

	/* Check Device Code */
	id834xA = HX8347x_rd_cmd(0x67);  			/* Confirm Vaild LCD Controller */
	id8347d = HX8347x_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(id8347d == 0x47)
	{
		/* Initialize HX8347D */
		/* Driving ability Setting */
		HX8347x_wr_cmd(0xEA);
		HX8347x_wr_dat(0x00); 			/* PTBA[15:8] */
		HX8347x_wr_cmd(0xEB);
		HX8347x_wr_dat(0x20); 			/* PTBA[7:0] */
		HX8347x_wr_cmd(0xEC);
		HX8347x_wr_dat(0x0C); 			/* STBA[15:8] */
		HX8347x_wr_cmd(0xED);
		HX8347x_wr_dat(0xC4); 			/* STBA[7:0] */
		HX8347x_wr_cmd(0xE8);
		HX8347x_wr_dat(0x40); 			/* OPON[7:0] */
		HX8347x_wr_cmd(0xE9);
		HX8347x_wr_dat(0x38); 			/* OPON1[7:0] */
		HX8347x_wr_cmd(0xF1);
		HX8347x_wr_dat(0x01); 			/* OTPS1B */
		HX8347x_wr_cmd(0xF2);
		HX8347x_wr_dat(0x10); 			/* GEN */
		HX8347x_wr_cmd(0x27);
		HX8347x_wr_dat(0xA3);  
	 
	   	/* Gamma 2.2 Setting */   
		HX8347x_wr_cmd(0x40);
		HX8347x_wr_dat(0x01);
		HX8347x_wr_cmd(0x41);
		HX8347x_wr_dat(0x00);
		HX8347x_wr_cmd(0x42);
		HX8347x_wr_dat(0x00);
		HX8347x_wr_cmd(0x43);
		HX8347x_wr_dat(0x10);
		HX8347x_wr_cmd(0x44);
		HX8347x_wr_dat(0x0E);
		HX8347x_wr_cmd(0x45);
		HX8347x_wr_dat(0x24);
		HX8347x_wr_cmd(0x46);
		HX8347x_wr_dat(0x04);
		HX8347x_wr_cmd(0x47);
		HX8347x_wr_dat(0x50);
		HX8347x_wr_cmd(0x48);
		HX8347x_wr_dat(0x02);
		HX8347x_wr_cmd(0x49);
		HX8347x_wr_dat(0x13);
		HX8347x_wr_cmd(0x4A);
		HX8347x_wr_dat(0x19);
		HX8347x_wr_cmd(0x4B);
		HX8347x_wr_dat(0x19);
		HX8347x_wr_cmd(0x4C);
		HX8347x_wr_dat(0x16);
	 
		HX8347x_wr_cmd(0x50);
		HX8347x_wr_dat(0x1B);
		HX8347x_wr_cmd(0x51);
		HX8347x_wr_dat(0x31);
		HX8347x_wr_cmd(0x52);
		HX8347x_wr_dat(0x2F);
		HX8347x_wr_cmd(0x53);
		HX8347x_wr_dat(0x3F);
		HX8347x_wr_cmd(0x54);
		HX8347x_wr_dat(0x3F);
		HX8347x_wr_cmd(0x55);
		HX8347x_wr_dat(0x3E);
		HX8347x_wr_cmd(0x56);
		HX8347x_wr_dat(0x2F);
		HX8347x_wr_cmd(0x57);
		HX8347x_wr_dat(0x7B);
		HX8347x_wr_cmd(0x58);
		HX8347x_wr_dat(0x09);
		HX8347x_wr_cmd(0x59);
		HX8347x_wr_dat(0x06);
		HX8347x_wr_cmd(0x5A);
		HX8347x_wr_dat(0x06);
		HX8347x_wr_cmd(0x5B);
		HX8347x_wr_dat(0x0C);
		HX8347x_wr_cmd(0x5C);
		HX8347x_wr_dat(0x1D);
		HX8347x_wr_cmd(0x5D);
		HX8347x_wr_dat(0xCC);

		/* Power Voltage Setting */
		HX8347x_wr_cmd(0x1B);
		HX8347x_wr_dat(0x1B); 			/* VRH=4.65V */
		HX8347x_wr_cmd(0x1A);
		HX8347x_wr_dat(0x01); 			/* BT (VGH~15V,VGL~-10V,DDVDH~5V)  */
		HX8347x_wr_cmd(0x24);
		HX8347x_wr_dat(0x2F); 			/* VMH(VCOM High voltage ~3.2V) */
		HX8347x_wr_cmd(0x25);
		HX8347x_wr_dat(0x57); 			/* VML(VCOM Low voltage -1.2V) */

		/* VCOM offset*/
		HX8347x_wr_cmd(0x23);
		HX8347x_wr_dat(0x88); 			/* for Flicker adjust can reload from OTP */
	 
		/* Power on Setting */
		HX8347x_wr_cmd(0x18);
		HX8347x_wr_dat(0x36); 			/* I/P_RADJ,N/P_RADJ, Normal mode 75Hz */ 
		HX8347x_wr_cmd(0x19);
		HX8347x_wr_dat(0x01); 			/* OSC_EN='1', start Osc */
		HX8347x_wr_cmd(0x01);
		HX8347x_wr_dat(0x00); 			/* DP_STB='0', out deep sleep */ 
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0x88); 			/*  GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0 */
		_delay_ms(5); 
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0x80); 			/*  GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0 */
		_delay_ms(5); 
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0x90); 			/*  GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0 */
		_delay_ms(5); 
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0xD0); 			/*  GAS=1, VOMG=10, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0 */
		_delay_ms(5); 
	 
		/* 262k/65k color selection */
		HX8347x_wr_cmd(0x17);
		HX8347x_wr_dat(0x05); 			/* default 0x06 262k color 0x05 65k color */
	 
		/* SET PANEL */
		HX8347x_wr_cmd(0x36);
		HX8347x_wr_dat(0x00); 			/* SS_P, GS_P,REV_P,BGR_P */
		
		/* Display ON Setting */
		HX8347x_wr_cmd(0x28);
		HX8347x_wr_dat(0x38); 			/* GON=1, DTE=1, D=1000 */
		_delay_ms(40); 
		HX8347x_wr_cmd(0x28);
		HX8347x_wr_dat(0x3C); 			/* GON=1, DTE=1, D=1100 */

	}
	
	else if(id8347d == 0x75)
	{
		/* Initialize HX8347G(T) */
		/* Driving ability Setting */
		HX8347x_wr_cmd(0xEA);
		HX8347x_wr_dat(0x00);			/* PTBA[15:8] */
		HX8347x_wr_cmd(0xEB);
		HX8347x_wr_dat(0x20);			/* PTBA[7:0] */
		HX8347x_wr_cmd(0xEC);
		HX8347x_wr_dat(0x3C);			/* STBA[15:8] */
		HX8347x_wr_cmd(0xED);
		HX8347x_wr_dat(0xC4);			/* STBA[7:0] */
		HX8347x_wr_cmd(0xE8);
		HX8347x_wr_dat(0x48);			/* OPON[7:0] */
		HX8347x_wr_cmd(0xE9);
		HX8347x_wr_dat(0x38);			/* OPON1[7:0] */
		HX8347x_wr_cmd(0xF1);
		HX8347x_wr_dat(0x01);			/* OTPS1B */
		HX8347x_wr_cmd(0xF2);
		HX8347x_wr_dat(0x08);			/* GEN */

		/* Gamma 2.2 Setting */
		HX8347x_wr_cmd(0x40);
		HX8347x_wr_dat(0x01);
		HX8347x_wr_cmd(0x41);
		HX8347x_wr_dat(0x07);
		HX8347x_wr_cmd(0x42);
		HX8347x_wr_dat(0x09);
		HX8347x_wr_cmd(0x43);
		HX8347x_wr_dat(0x19);
		HX8347x_wr_cmd(0x44);
		HX8347x_wr_dat(0x14);
		HX8347x_wr_cmd(0x45);
		HX8347x_wr_dat(0x0B);
		HX8347x_wr_cmd(0x46);
		HX8347x_wr_dat(0x18);
		HX8347x_wr_cmd(0x47);
		HX8347x_wr_dat(0x61);
		HX8347x_wr_cmd(0x48);
		HX8347x_wr_dat(0x00);
		HX8347x_wr_cmd(0x49);
		HX8347x_wr_dat(0x10);
		HX8347x_wr_cmd(0x4A);
		HX8347x_wr_dat(0x15);
		HX8347x_wr_cmd(0x4B);
		HX8347x_wr_dat(0x15);
		HX8347x_wr_cmd(0x4C);
		HX8347x_wr_dat(0x10);

		HX8347x_wr_cmd(0x50);
		HX8347x_wr_dat(0x19);
		HX8347x_wr_cmd(0x51);
		HX8347x_wr_dat(0x24);
		HX8347x_wr_cmd(0x52);
		HX8347x_wr_dat(0x21);
		HX8347x_wr_cmd(0x53);
		HX8347x_wr_dat(0x29);
		HX8347x_wr_cmd(0x54);
		HX8347x_wr_dat(0x38);
		HX8347x_wr_cmd(0x55);
		HX8347x_wr_dat(0x3E);
		HX8347x_wr_cmd(0x56);
		HX8347x_wr_dat(0x10);
		HX8347x_wr_cmd(0x57);
		HX8347x_wr_dat(0x5A);
		HX8347x_wr_cmd(0x58);
		HX8347x_wr_dat(0x09);
		HX8347x_wr_cmd(0x59);
		HX8347x_wr_dat(0x04);
		HX8347x_wr_cmd(0x5A);
		HX8347x_wr_dat(0x02);
		HX8347x_wr_cmd(0x5B);
		HX8347x_wr_dat(0x04);
		HX8347x_wr_cmd(0x5C);
		HX8347x_wr_dat(0x1D);
		HX8347x_wr_cmd(0x5D);
		HX8347x_wr_dat(0xCC);
		HX8347x_wr_cmd(0x1B);
		HX8347x_wr_dat(0x16);			/* VRH=4.65V */
		HX8347x_wr_cmd(0x1A);
		HX8347x_wr_dat(0x01);			/* BT (VGH~15V) VGL~-10V) DDVDH~5V) */
		HX8347x_wr_cmd(0x23);
		HX8347x_wr_dat(0x70);			/* VMH(VCOM High voltage ~4.2V) */
		HX8347x_wr_cmd(0x24);
		HX8347x_wr_dat(0x93);			/* VML(VCOM Low voltage -1.2V) */

		/* VCOM offset */
		HX8347x_wr_cmd(0x23);
		HX8347x_wr_dat(0x59);			/* for Flicker adjust */			/* can reload from OTP */

		/* Power on Setting */
		HX8347x_wr_cmd(0x18);
		HX8347x_wr_dat(0x36); 			/* I/P_RADJ,N/P_RADJ, Normal mode 75Hz */ 
		HX8347x_wr_cmd(0x19);
		HX8347x_wr_dat(0x01);			/* OSC_EN='1') start Osc */
		HX8347x_wr_cmd(0x01);
		HX8347x_wr_dat(0x00);			/* DP_STB='0') out deep sleep */
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0x8A);			/* GAS=1) VOMG=00) PON=0) DK=1) XDK=0) DVDH_TRI=1) STB=0 */
		_delay_ms(100);
		
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0x82);			/* GAS=1) VOMG=00) PON=0) DK=0) XDK=0) DVDH_TRI=1) STB=0 */
		_delay_ms(100);
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0x92);			/* GAS=1) VOMG=00) PON=1) DK=0) XDK=0) DVDH_TRI=1) STB=0 */
		_delay_ms(100);
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0xD2);			/* GAS=1) VOMG=10) PON=1) DK=0) XDK=0) DDVDH_TRI=1) STB=0 */
		_delay_ms(100);

		/* 262k/65k color selection */
		HX8347x_wr_cmd(0x17);
		HX8347x_wr_dat(0x05);			/* default 0x06 262k color */			/* 0x05 65k color */

		/* SET PANEL */
		HX8347x_wr_cmd(0x36);
		HX8347x_wr_dat(0x09);			/* SS_P) GS_P) REV_P) BGR_P */

		/* Display ON Setting */
		HX8347x_wr_cmd(0x28);
		HX8347x_wr_dat(0x38);			/* GON=1) DTE=1) D=1000 */
		_delay_ms(100);
	
		HX8347x_wr_cmd(0x28);
		HX8347x_wr_dat(0x3F);			/* GON=1) DTE=1) D=1100 */

	}

	else if(id834xA == 0x47)
	{
		/* Initialize HX8347A*/
	#ifdef LCD_FASTESTBUS_WORKAROUND
		Bus_Sleep = Bus_Sleep_hx8347a;
	#endif
		/* Gamma for CMO 3.2ÅP */
		HX8347x_wr_cmd(0x46);
		HX8347x_wr_dat(0x91);
		HX8347x_wr_cmd(0x47);
		HX8347x_wr_dat(0x11);
		HX8347x_wr_cmd(0x48);
		HX8347x_wr_dat(0x00);
		HX8347x_wr_cmd(0x49);
		HX8347x_wr_dat(0x66);
		HX8347x_wr_cmd(0x4a);
		HX8347x_wr_dat(0x37);
		HX8347x_wr_cmd(0x4b);
		HX8347x_wr_dat(0x04);
		HX8347x_wr_cmd(0x4c);
		HX8347x_wr_dat(0x11);
		HX8347x_wr_cmd(0x4d);
		HX8347x_wr_dat(0x77);
		HX8347x_wr_cmd(0x4e);
		HX8347x_wr_dat(0x00);
		HX8347x_wr_cmd(0x4f);
		HX8347x_wr_dat(0x1F);
		HX8347x_wr_cmd(0x50);
		HX8347x_wr_dat(0x0F);
		HX8347x_wr_cmd(0x51);
		HX8347x_wr_dat(0x00);

		/* Display Setting */
		HX8347x_wr_cmd(0x01);
		HX8347x_wr_dat(0x06);			/* IDMON=0, INVON=1, NORON=1, PTLON=0 */

		HX8347x_wr_cmd(0x16);
		HX8347x_wr_dat(0xc8);			/* MY=0, MX=0, MV=0, ML=1, BGR=0, TEON=0 */
		HX8347x_wr_cmd(0x23);
		HX8347x_wr_dat(0x95);			/* N_DC=1001 0101 */

		HX8347x_wr_cmd(0x24);
		HX8347x_wr_dat(0x95);			/* PI_DC=1001 0101 */
		HX8347x_wr_cmd(0x25);
		HX8347x_wr_dat(0xFF);			/* I_DC=1111 1111 */
		HX8347x_wr_cmd(0x27);
		HX8347x_wr_dat(0x02);			/* N_BP=0000 0010 */
		HX8347x_wr_cmd(0x28);
		HX8347x_wr_dat(0x02);			/* N_FP=0000 0010 */
		HX8347x_wr_cmd(0x29);
		HX8347x_wr_dat(0x02);			/* PI_BP=0000 0010 */
		HX8347x_wr_cmd(0x2a);
		HX8347x_wr_dat(0x02);			/* PI_FP=0000 0010 */
		HX8347x_wr_cmd(0x2C);
		HX8347x_wr_dat(0x02);			/* I_BP=0000 0010 */
		HX8347x_wr_cmd(0x2d);
		HX8347x_wr_dat(0x02);			/* I_FP=0000 0010 */
		HX8347x_wr_cmd(0x3a);
		HX8347x_wr_dat(0x01);			/* N_RTN=0000, N_NW=001    0001 */
		HX8347x_wr_cmd(0x3b);
		HX8347x_wr_dat(0x01);			/* P_RTN=0000, P_NW=001 */
		HX8347x_wr_cmd(0x3c);
		HX8347x_wr_dat(0xf0);			/* I_RTN=1111, I_NW=000 */
		HX8347x_wr_cmd(0x3d);
		HX8347x_wr_dat(0x00);			/* DIV=00 */
		_delay_ms(2);
		HX8347x_wr_cmd(0x35);
		HX8347x_wr_dat(0x38);			/* EQS=38h */
		HX8347x_wr_cmd(0x36);
		HX8347x_wr_dat(0x78);			/* EQP=78h */
		HX8347x_wr_cmd(0x3E);
		HX8347x_wr_dat(0x38);			/* SON=38h */
		HX8347x_wr_cmd(0x40);
		HX8347x_wr_dat(0x0F);			/* GDON=0Fh */
		HX8347x_wr_cmd(0x41);
		HX8347x_wr_dat(0xF0);			/* GDOFF */
		
		/* Power Supply Setting */
		HX8347x_wr_cmd(0x19);
		HX8347x_wr_dat(0x49);			/* CADJ=0100, CUADJ=100, OSD_EN=1 ,60Hz */
		HX8347x_wr_cmd(0x93);
		HX8347x_wr_dat(0x0F);			/* RADJ=1111, 100% */
		_delay_ms(10);
		HX8347x_wr_cmd(0x20);
		HX8347x_wr_dat(0x40);			/* BT=0100 */
		HX8347x_wr_cmd(0x1D);
		HX8347x_wr_dat(0x07);			/* VC1=111 */
		HX8347x_wr_cmd(0x1E);
		HX8347x_wr_dat(0x00);			/* VC3=000 */
		HX8347x_wr_cmd(0x1F);
		HX8347x_wr_dat(0x03);			/* VRH=0011 */
		HX8347x_wr_cmd(0x44);
		HX8347x_wr_dat(0x50);			/* VCM=101 0000 */
		HX8347x_wr_cmd(0x45);
		HX8347x_wr_dat(0x11);			/* VDV=1 0001 */
		_delay_ms(10);
		HX8347x_wr_cmd(0x1C);
		HX8347x_wr_dat(0x04);			/* AP=100 */
		_delay_ms(20);
		HX8347x_wr_cmd(0x43);
		HX8347x_wr_dat(0x80);			/* set VCOMG=1 */
		_delay_ms(5);
		HX8347x_wr_cmd(0x1B);
		HX8347x_wr_dat(0x08);			/* GASENB=0, PON=0, DK=1, XDK=0, VLCD_TRI=0, STB=0 */
		_delay_ms(40);
		HX8347x_wr_cmd(0x1B);
		HX8347x_wr_dat(0x10);			/* GASENB=0, PON=1, DK=0, XDK=0, VLCD_TRI=0, STB=0 */
		_delay_ms(40);
		
		/* Display ON Setting */
		HX8347x_wr_cmd(0x90);
		HX8347x_wr_dat(0x7F);			/*  SAP=0111 1111 */
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x04);			/* GON=0, DTE=0, D=01 */
		_delay_ms(40);
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x24);			/* GON=1, DTE=0, D=01 */
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x2C);			/* GON=1, DTE=0, D=11 */
		_delay_ms(40);
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x3C);			/* GON=1, DTE=1, D=11 */
		
		/* Fixed the read data issue */
		HX8347x_wr_cmd(0x57);
		HX8347x_wr_dat(0x02);			/* TEST_Mode=1: into TEST mode */
		HX8347x_wr_cmd(0x56);
		HX8347x_wr_dat(0x02);			/* tune the memory timing */
		HX8347x_wr_cmd(0x57);
		HX8347x_wr_dat(0x00);			/* TEST_Mode=0: exit TEST mode */
		HX8347x_wr_cmd(0x21);
		HX8347x_wr_dat(0x00);
	}

	else if(id834xA == 0x46)
	{
		/* Initialize HX8346A */
		/*** Gamma ***/   
		HX8347x_wr_cmd(0x46);
		HX8347x_wr_dat(0x45);
		HX8347x_wr_cmd(0x47);
		HX8347x_wr_dat(0x54);
		HX8347x_wr_cmd(0x48);
		HX8347x_wr_dat(0x00);
		HX8347x_wr_cmd(0x49);
		HX8347x_wr_dat(0x67);
		HX8347x_wr_cmd(0x4a);
		HX8347x_wr_dat(0x07);
		HX8347x_wr_cmd(0x4b);
		HX8347x_wr_dat(0x07);
		HX8347x_wr_cmd(0x4c);
		HX8347x_wr_dat(0x01);
		HX8347x_wr_cmd(0x4d);
		HX8347x_wr_dat(0x77);
		HX8347x_wr_cmd(0x4e);
		HX8347x_wr_dat(0x00);
		HX8347x_wr_cmd(0x4f);
		HX8347x_wr_dat(0x29);
		HX8347x_wr_cmd(0x50);
		HX8347x_wr_dat(0x04);
		HX8347x_wr_cmd(0x51);
		HX8347x_wr_dat(0x40);   
    
		/*** Display Setting ***/   
		HX8347x_wr_cmd(0x01);
		HX8347x_wr_dat(0x06);			/* IDMON=0, INVON=1, NORON=1, PTLON=0 */  

		HX8347x_wr_cmd(0x16);
		HX8347x_wr_dat((1<<7)|(1<<6)|(0<<5)|(0<<4)|(1<<3));		/* MY=1, MX=1, MV=0, ML=0, BGR=1, TEON=0 */

		HX8347x_wr_cmd(0x23);
		HX8347x_wr_dat(0x95);			/* N_DC=1001 0101 */  

		HX8347x_wr_cmd(0x24);
		HX8347x_wr_dat(0x95);			/* P_DC=1001 0101 */   

		HX8347x_wr_cmd(0x25);
		HX8347x_wr_dat(0xff);			/* I_DC=1111 1111 */   

		HX8347x_wr_cmd(0x28);
		HX8347x_wr_dat(0x02);			/* N_BP=0000 0010 */  
		HX8347x_wr_cmd(0x29);
		HX8347x_wr_dat(0x02);			/* N_FP=0000 0010 */   
		HX8347x_wr_cmd(0x2a);
		HX8347x_wr_dat(0x02);			/* P_BP=0000 0010 */   
		HX8347x_wr_cmd(0x2b);
		HX8347x_wr_dat(0x02);			/* P_FP=0000 0010 */   
		HX8347x_wr_cmd(0x2c);
		HX8347x_wr_dat(0x02);			/* I_BP=0000 0010 */   
		HX8347x_wr_cmd(0x2d);
		HX8347x_wr_dat(0x02);			/* I_FP=0000 0010 */   
		HX8347x_wr_cmd(0x3a);
		HX8347x_wr_dat(0x01);			/* N_RTN=0000, N_NW=001 */   
		HX8347x_wr_cmd(0x3b);
		HX8347x_wr_dat(0x01);			/* P_RTN=0000, P_NW=001 */   
		HX8347x_wr_cmd(0x3c);
		HX8347x_wr_dat(0xf0);			/* I_RTN=1111, I_NW=000 */   
		HX8347x_wr_cmd(0x3d);
		HX8347x_wr_dat(0x00);			/* DIV=00 */   
		_delay_ms(2);   
    
		/*** Power Supply Setting ***/
		HX8347x_wr_cmd(0x19);
		HX8347x_wr_dat(0x41);			/* OSCADJ=010000, OSD_EN=1 */   
    
		_delay_ms(1);
		/*** for the setting before power supply startup ***/   
		HX8347x_wr_cmd(0x20);
		HX8347x_wr_dat(0x40);			/* BT=0100 */   
		HX8347x_wr_cmd(0x21);
		HX8347x_wr_dat(0x00);   
		HX8347x_wr_cmd(0x1d);
		HX8347x_wr_dat(0x00);			/* VC2=100, VC1=100 */   
		HX8347x_wr_cmd(0x1e);
		HX8347x_wr_dat(0x01);			/* VC3=000 */   
		HX8347x_wr_cmd(0x1f);
		HX8347x_wr_dat(0x0e);			/* VRH=0110 */   
		HX8347x_wr_cmd(0x44);
		HX8347x_wr_dat(0x3c);			/* 3b VCM=101 1010,VCOMH=VREG1*0.845 */   
		HX8347x_wr_cmd(0x45);
		HX8347x_wr_dat(0x10);			/* 11 VDV=1 0001,VCOM=1.08*VREG1 */   
		_delay_ms(1);   
    
		/* for power supply setting */   
		HX8347x_wr_cmd(0x1c);
		HX8347x_wr_dat(0x0007);			/* AP=100 */   
		_delay_ms(2);   
		HX8347x_wr_cmd(0x1b);
		HX8347x_wr_dat(0x0018);			/* GASENB=0, PON=1, DK=1, XDK=0, DDVDH_TRI=0, STB=0 */  
		_delay_ms(4);   
		HX8347x_wr_cmd(0x1b);
		HX8347x_wr_dat(0x0010);			/* GASENB=0, PON=1, DK=0, XDK=0, DDVDH_TRI=0, STB=0 */   
		_delay_ms(4);   
		HX8347x_wr_cmd(0x43);
		HX8347x_wr_dat(0x80);			/* VCOMG=1 */  
		_delay_ms(1);   
    
		/*** Display ON Setting ***/
		HX8347x_wr_cmd(0x30);
		HX8347x_wr_dat(0x08);			/* SAPS1=1000 */   
		_delay_ms(4);   
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x04);			/* GON=0, DTE=0, D=01 */   
		_delay_ms(4);   
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x24);			/* GON=1, DTE=0, D=01 */   
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x2c);			/* GON=1, DTE=0, D=11 */   
		_delay_ms(4);   
		HX8347x_wr_cmd(0x26);
		HX8347x_wr_dat(0x3c);			/* GON=1, DTE=1, D=11 */   
	}

	else { for(;;);}					/* Invalid Device Code!! */

	HX8347x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8347x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8347x_wr_dat(COL_RED);
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
int HX8347x_draw_bmp(const uint8_t* ptr){

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
	HX8347x_clear();

   	/* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

   	/* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		HX8347x_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			HX8347x_wr_gram(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
