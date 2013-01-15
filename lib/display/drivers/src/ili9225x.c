/********************************************************************************/
/*!
	@file			ili9225x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.08.15
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TXDT200A-15V13		(ILI9225)	  	8bit mode. 			@n
					 -KXM220HS-V05			(ILI9226)		8/16bit mode.		@n
					 -RX020C-1				(S6D0164X1)		8bit mode.			@n
					 -BTL221722-276LP		(ILI9225B)		8bit Serial only.	

    @section HISTORY
		2012.08.15	V1.00	Revised from ili9225.c
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "ili9225x.h"
/* check header file version for fool proof */
#if __ILI9225X_H != 0x0100
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
inline void ILI9225x_reset(void)
{
#ifdef  USE_ILI9225x_TFT
	ILI9225x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9225x_RD_SET();
	ILI9225x_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9225x_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9225x_CS_CLR();

#elif  USE_ILI9225x_SPI_TFT
	ILI9225x_RES_SET();							/* RES=H, CS=H				*/
	ILI9225x_CS_SET();
	ILI9225x_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9225x_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(30);								/* wait 30ms     			*/
	ILI9225x_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ILI9225x_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9225x_wr_cmd(uint8_t cmd)
{
	ILI9225x_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9225x_CMD = 0;
	ILI9225x_WR();
#endif

	ILI9225x_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	ILI9225x_WR();								/* WR=L->H					*/

	ILI9225x_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI9225x_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9225x_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	ILI9225x_WR();								/* WR=L->H					*/
	ILI9225x_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	ILI9225x_DATA = dat;							/* 16bit data 				*/
#endif
	ILI9225x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9225x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9225x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9225x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9225x_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	ILI9225x_wr_cmd(cmd);
	ILI9225x_WR_SET();

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


#elif USE_ILI9225x_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9225x_wr_cmd(uint8_t cmd)
{
	ILI9225x_DC_CLR();							/* DC=L			 */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ILI9225x_DC_SET();							/* DC=H			 */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ILI9225x_wr_dat(uint16_t dat)
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
inline void ILI9225x_wr_block(uint8_t *p,unsigned int cnt)
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
inline uint16_t ILI9225x_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint8_t temp;

	ILI9225x_wr_cmd(0x66);						/* Register Read Mode */
	ILI9225x_wr_dat(0x01);

	DISPLAY_ASSART_CS();						/* CS=L		    */
	ILI9225x_DC_CLR();							/* DC=L			*/

	SendSPI(cmd);

	ILI9225x_DC_SET();							/* DC=H			*/
	temp = RecvSPI();							/* Dummy Read 	*/
	temp = RecvSPI();							/* Dummy Read 	*/
	temp = RecvSPI();							/* Upper Read 	*/
	val  = RecvSPI();							/* Lower Read	*/

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;

	DISPLAY_NEGATE_CS();						/* CS=H		    */

	ILI9225x_wr_cmd(0x66);						/* Register Write Mode */
	ILI9225x_wr_dat(0x00);

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI9225x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9225x_wr_cmd(0x37);				/* Horizontal RAM Start ADDR */
	ILI9225x_wr_dat(OFS_COL + x);
	ILI9225x_wr_cmd(0x36);				/* Horizontal RAM End ADDR */
	ILI9225x_wr_dat(OFS_COL + width);
	ILI9225x_wr_cmd(0x39);				/* Vertical RAM Start ADDR */
	ILI9225x_wr_dat(OFS_RAW + y);
	ILI9225x_wr_cmd(0x38);				/* Vertical End ADDR */
	ILI9225x_wr_dat(OFS_RAW + height);

	ILI9225x_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	ILI9225x_wr_dat(OFS_RAW + y);
	ILI9225x_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	ILI9225x_wr_dat(OFS_COL + x);

	ILI9225x_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9225x_clear(void)
{
	volatile uint32_t n;

	ILI9225x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9225x_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9225x_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	ILI9225x_reset();

	/* Check Device Code */
	devicetype = ILI9225x_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x9225)
	{
		/* Initialize ILI9225 & ILI9225B & ILI9225C */
		ILI9225x_wr_cmd(0x01);		/* set SS and NL bit */
		ILI9225x_wr_dat(0x011C);
		ILI9225x_wr_cmd(0x02);		/* set 1 line inversion */
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x03);		/* Set GRAM write direction and BGR=1 */ 
		ILI9225x_wr_dat((1<<12)|(0<<9)|(1<<8)|(1<<5)|(1<<4)|(0<<3));
		/* ILI9225x_wr_dat(0x1030); */ /* original */
		ILI9225x_wr_cmd(0x08);		/* set BP and FP */
		ILI9225x_wr_dat(0x0808);
		ILI9225x_wr_cmd(0x0B);		/* frame cycle */
		ILI9225x_wr_dat(0x1100);
		ILI9225x_wr_cmd(0x0C);		/* RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit */
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x0F);		/* Set frame rate----0801 */
		ILI9225x_wr_dat(0x1401);
		ILI9225x_wr_cmd(0x15);		/* set system interface */
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x20);	 	/* Set GRAM Address */
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x21);	 	/* Set GRAM Address */
		ILI9225x_wr_dat(0x0000);
		
		/* Power Up sequence */
		_delay_ms(50);
		ILI9225x_wr_cmd(0x10);		/* Set SAP,DSTB,STB----0A00 */
		ILI9225x_wr_dat(0x0800);
		ILI9225x_wr_cmd(0x11);		/* Set APON,PON,AON,VCI1EN,VC----1038 */
		ILI9225x_wr_dat(0x1F3F);
		_delay_ms(50);
		ILI9225x_wr_cmd(0x12);	 	/* Internal reference voltage= Vci;----1121 */
		ILI9225x_wr_dat(0x0121);
		ILI9225x_wr_cmd(0x13);	 	/* Set GVDD----0066 */
		ILI9225x_wr_dat(0x006F);
		ILI9225x_wr_cmd(0x14);	 	/* Set VCOMH/VCOML voltage----5F60 */
		ILI9225x_wr_dat(0x4349);
		
		/* Set GRAM area */
		ILI9225x_wr_cmd(0x30);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x31);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x32);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x33);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x34);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x35);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x36);
		ILI9225x_wr_dat(0x00AF);
		ILI9225x_wr_cmd(0x37);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x38);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x39);
		ILI9225x_wr_dat(0x0000);

		/* Adjust the Gamma Curve */
		ILI9225x_wr_cmd(0x50);
		ILI9225x_wr_dat(0x0001);  /* 0400 */
		ILI9225x_wr_cmd(0x51);
		ILI9225x_wr_dat(0x200B);  /* 060B */
		ILI9225x_wr_cmd(0x52);
		ILI9225x_wr_dat(0x0000);  /* 0C0A */
		ILI9225x_wr_cmd(0x53);
		ILI9225x_wr_dat(0x0404);  /* 0105 */
		ILI9225x_wr_cmd(0x54);
		ILI9225x_wr_dat(0x0C0C);  /* 0A0C */
		ILI9225x_wr_cmd(0x55);
		ILI9225x_wr_dat(0x000C);  /* 0B06 */
		ILI9225x_wr_cmd(0x56);
		ILI9225x_wr_dat(0x0101);  /* 0004 */
		ILI9225x_wr_cmd(0x57);
		ILI9225x_wr_dat(0x0400);  /* 0501 */
		ILI9225x_wr_cmd(0x58);
		ILI9225x_wr_dat(0x1108);  /* 0E00 */
		ILI9225x_wr_cmd(0x59);
		ILI9225x_wr_dat(0x050C);  /* 000E */
		_delay_ms(50);
		
		ILI9225x_wr_cmd(0x07);						   /* Display On */ 
		ILI9225x_wr_dat((1<<12)|(1<<4)|(1<<2)|(1<<1)|(1<<0));
	}

	else if(devicetype == 0x9226)
	{
		/* Initialize ILI9226 */
		/* Start Initial Sequence */
		ILI9225x_wr_cmd(0x01);
		ILI9225x_wr_dat(0x011C);		/* set SS and NL bit */
		ILI9225x_wr_cmd(0x02);
		ILI9225x_wr_dat(0x0100);		/* set 1 line inversion */
		ILI9225x_wr_cmd(0x03);
		ILI9225x_wr_dat(0x1030);		/* set GRAM write direction and BGR=1. */
		ILI9225x_wr_cmd(0x08);
		ILI9225x_wr_dat(0x0808);		/* set BP and FP */
		ILI9225x_wr_cmd(0x0C);
		ILI9225x_wr_dat(0x0000);		/* RGB interface setting R0Ch=0x0110 for RGB 18Bit and R0Ch=0111forRGB16Bit */
		ILI9225x_wr_cmd(0x0F);
		ILI9225x_wr_dat(0x0801);		/* Set frame rate */
		ILI9225x_wr_cmd(0x20);
		ILI9225x_wr_dat(0x0000);		/* Set GRAM Address */
		ILI9225x_wr_cmd(0x21);
		ILI9225x_wr_dat(0x0000);		/* Set GRAM Address */
		/* Power On sequence */
		_delay_ms(50);		 			/* Delay 50ms */
		ILI9225x_wr_cmd(0x10);
		ILI9225x_wr_dat(0x0A00);		/* Set SAP,DSTB,STB */
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x103B);		/* Set APON,PON,AON,VCI1EN,VC */
		_delay_ms(50);		 			/* Delay 50ms */
		ILI9225x_wr_cmd(0x12);
		ILI9225x_wr_dat(0x3121);		/* Internal reference voltage= Vci; */
		ILI9225x_wr_cmd(0x13);
		ILI9225x_wr_dat(0x0066);		/* Set GVDD */
		ILI9225x_wr_cmd(0x14);
		ILI9225x_wr_dat(0x4050);		/* Set VCOMH/VCOML voltage */
		/* Set GRAM area */
		ILI9225x_wr_cmd(0x30);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x31);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x32);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x33);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x34);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x35);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x36);
		ILI9225x_wr_dat(0x00AF);
		ILI9225x_wr_cmd(0x37);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x38);
		ILI9225x_wr_dat(0x00DB);
		ILI9225x_wr_cmd(0x39);
		ILI9225x_wr_dat(0x0000);
		/* Adjust the Gamma Curve */
		ILI9225x_wr_cmd(0x50);
		ILI9225x_wr_dat(0x0400);
		ILI9225x_wr_cmd(0x51);
		ILI9225x_wr_dat(0x080B);
		ILI9225x_wr_cmd(0x52);
		ILI9225x_wr_dat(0x0E0C);
		ILI9225x_wr_cmd(0x53);
		ILI9225x_wr_dat(0x0103);
		ILI9225x_wr_cmd(0x54);
		ILI9225x_wr_dat(0x0C0E);
		ILI9225x_wr_cmd(0x55);
		ILI9225x_wr_dat(0x0B08);
		ILI9225x_wr_cmd(0x56);
		ILI9225x_wr_dat(0x0004);
		ILI9225x_wr_cmd(0x57);
		ILI9225x_wr_dat(0x0301);
		ILI9225x_wr_cmd(0x58);
		ILI9225x_wr_dat(0x0800);
		ILI9225x_wr_cmd(0x59);
		ILI9225x_wr_dat(0x0008);
		_delay_ms(50);				/* Delay 50ms */
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x1017);
	}

	else if((devicetype == 0x0164) || (devicetype == 0x2201))
	{
		/* Initialize OTM2201 & S6D0164 */
		ILI9225x_wr_cmd(0xF0);
		ILI9225x_wr_dat(0x2201);
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0xF7);
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x15);
		ILI9225x_wr_dat(0x0010);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0012);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x001A);
		ILI9225x_wr_cmd(0x01);
		ILI9225x_wr_dat(0x011C);
		ILI9225x_wr_cmd(0x12);
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x03);
		ILI9225x_wr_dat(0x1030);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x08);
		ILI9225x_wr_dat(0x0808);
		ILI9225x_wr_cmd(0x0B);
		ILI9225x_wr_dat(0x1100);
		ILI9225x_wr_cmd(0x0C);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x0F);
		ILI9225x_wr_dat(0x0501);
		ILI9225x_wr_cmd(0x15);
		ILI9225x_wr_dat(0x0020);
		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0018);
		ILI9225x_wr_cmd(0x12);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x13);
		ILI9225x_wr_dat(0x0063);
		ILI9225x_wr_cmd(0x14);
		ILI9225x_wr_dat(0x556A);
		ILI9225x_wr_cmd(0x10);
		ILI9225x_wr_dat(0x0800);
		_delay_ms(50);

		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0118);
		_delay_ms(50);

		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0318);
		_delay_ms(50);

		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0718);
		_delay_ms(50);

		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0f18);
		_delay_ms(50);

		ILI9225x_wr_cmd(0x11);
		ILI9225x_wr_dat(0x0f38);
		_delay_ms(50);

		ILI9225x_wr_cmd(0x36);           
		ILI9225x_wr_dat(0x00af);
		ILI9225x_wr_cmd(0x37);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x38);
		ILI9225x_wr_dat(0x00db);
		ILI9225x_wr_cmd(0x39);
		ILI9225x_wr_dat(0x0000);
		ILI9225x_wr_cmd(0x50);
		ILI9225x_wr_dat(0x0001);
		ILI9225x_wr_cmd(0x51);
		ILI9225x_wr_dat(0x0208);
		ILI9225x_wr_cmd(0x52);
		ILI9225x_wr_dat(0x0805);
		ILI9225x_wr_cmd(0x53);
		ILI9225x_wr_dat(0x0404);
		ILI9225x_wr_cmd(0x54);
		ILI9225x_wr_dat(0x0c0c);
		ILI9225x_wr_cmd(0x55);
		ILI9225x_wr_dat(0x000c);
		ILI9225x_wr_cmd(0x56);
		ILI9225x_wr_dat(0x0100);
		ILI9225x_wr_cmd(0x57);
		ILI9225x_wr_dat(0x0400);
		ILI9225x_wr_cmd(0x58);
		ILI9225x_wr_dat(0x1108);
		ILI9225x_wr_cmd(0x59);
		ILI9225x_wr_dat(0x050c);
		ILI9225x_wr_cmd(0x0f);
		ILI9225x_wr_dat(0x0f01);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0012);
		ILI9225x_wr_cmd(0x07);
		ILI9225x_wr_dat(0x0017);

	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9225x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9225x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9225x_wr_dat(COL_RED);
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
int ILI9225x_draw_bmp(const uint8_t* ptr){

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
	ILI9225x_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		ILI9225x_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			ILI9225x_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
