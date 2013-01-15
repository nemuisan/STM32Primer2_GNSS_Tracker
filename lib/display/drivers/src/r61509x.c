/********************************************************************************/
/*!
	@file			r61509x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.04.29
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KIC30108-A				(R61509V)	8/16bit mode.		@n
					 -TFT8K3162                 (R61509V)   8/16bit mode.       @n
					 -BLC280-07D-1				(SPFD5420A)	8/16bit mode.		@n
					 -BLC300-02B-2				(SPFD5420A)	8/16bit mode.		@n
					 -FL260WQC01-A1T(KD156)		(R61509)	8/16bit&SPI mode. 	@n
					 -S95361					(ILI9326)   8/16bit mode.

    @section HISTORY
		2012.04.29	V1.00	Revised from r61509v.c

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "r61509x.h"
/* check header file version for fool proof */
#if __R61509X_H != 0x0100
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
inline void R61509x_reset(void)
{
#ifdef USE_R61509x_TFT
	R61509x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	R61509x_RD_SET();
	R61509x_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	R61509x_RES_CLR();							/* RES=L, CS=L   			*/
	R61509x_CS_CLR();

#elif  USE_R61509x_SPI_TFT
	R61509x_RES_SET();							/* RES=H, CS=H				*/
	R61509x_CS_SET();
	R61509x_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 50ms     			*/

	R61509x_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(30);								/* wait 30ms     			*/
	R61509x_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/

}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_R61509x_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void R61509x_wr_cmd(uint16_t cmd)
{
	R61509x_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	R61509x_CMD = (uint8_t)(cmd>>8);
	R61509x_WR();								/* WR=L->H					*/
#endif

	R61509x_CMD = cmd;							/* command(8bit_Low or 16bit)	*/
	R61509x_WR();								/* WR=L->H					*/

	R61509x_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void R61509x_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	R61509x_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	R61509x_WR();								/* WR=L->H					*/
	R61509x_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	R61509x_DATA = dat;							/* 16bit data 				*/
#endif
	R61509x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void R61509x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		R61509x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		R61509x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t R61509x_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	R61509x_wr_cmd(cmd);
	R61509x_WR_SET();

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


#elif USE_R61509x_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void R61509x_wr_cmd(uint16_t cmd)
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
inline void R61509x_wr_dat(uint16_t dat)
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
inline void R61509x_wr_block(uint8_t *p,unsigned int cnt)
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
inline  uint16_t R61509x_rd_cmd(uint16_t cmd)
{
#if 0
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
#endif
	return 0x1509;								/* FoxLink KD152 Module does NOT have SDO Pin...s**k!! */
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void R61509x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	R61509x_wr_cmd(0x0210);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	R61509x_wr_dat(OFS_COL + x);
	R61509x_wr_cmd(0x0211);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	R61509x_wr_dat(OFS_COL + width);
	R61509x_wr_cmd(0x0212);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	R61509x_wr_dat(OFS_RAW + y);
	R61509x_wr_cmd(0x0213);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	R61509x_wr_dat(OFS_RAW + height);

	R61509x_wr_cmd(0x0201);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	R61509x_wr_dat(OFS_RAW + y);
	R61509x_wr_cmd(0x0200);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	R61509x_wr_dat(OFS_COL + x);

	R61509x_wr_cmd(0x0202);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void R61509x_clear(void)
{
	volatile uint32_t n;

	R61509x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61509x_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void R61509x_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	R61509x_reset();

	/* Check Device Code */
	devicetype = R61509x_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0xB509)
	{
		/* Initialize R61509V */
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		_delay_ms(10);
		R61509x_wr_cmd(0x0400);
		R61509x_wr_dat(0x6200);
		R61509x_wr_cmd(0x0008);
		R61509x_wr_dat(0x0808);
		R61509x_wr_cmd(0x0010);
		R61509x_wr_dat(0x0010);
		R61509x_wr_cmd(0x0400);
		R61509x_wr_dat(0x6200);

		/* GAMMA */
		R61509x_wr_cmd(0x0300);
		R61509x_wr_dat(0x0c06);
		R61509x_wr_cmd(0x0301);
		R61509x_wr_dat(0x9d0f);
		R61509x_wr_cmd(0x0302);
		R61509x_wr_dat(0x0b05);
		R61509x_wr_cmd(0x0303);
		R61509x_wr_dat(0x1217);
		R61509x_wr_cmd(0x0304);
		R61509x_wr_dat(0x3333);
		R61509x_wr_cmd(0x0305);
		R61509x_wr_dat(0x1712);
		R61509x_wr_cmd(0x0306);
		R61509x_wr_dat(0x950b);	
		R61509x_wr_cmd(0x0307);
		R61509x_wr_dat(0x0f0d);
		R61509x_wr_cmd(0x0308);
		R61509x_wr_dat(0x060c); 
		R61509x_wr_cmd(0x0309);
		R61509x_wr_dat(0x0000);

		R61509x_wr_cmd(0x0010);
		R61509x_wr_dat(0x0014);
		R61509x_wr_cmd(0x0011);
		R61509x_wr_dat(0x0202);
		R61509x_wr_cmd(0x0012);
		R61509x_wr_dat(0x0101);
		R61509x_wr_cmd(0x0013);
		R61509x_wr_dat(0x0001);

		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x0100);
		R61509x_wr_dat(0x0730); 					/* BT,AP 0x0330 */
		R61509x_wr_cmd(0x0101);
		R61509x_wr_dat(0x0237);				 		/* DC0,DC1,VC */
		R61509x_wr_cmd(0x0103);
		R61509x_wr_dat(0x2b00); 					/* VDV	0x0f00 */
		R61509x_wr_cmd(0x0280);
		R61509x_wr_dat(0x4000); 					/* VCM		  */
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x81b0); 					/*VRH,VCMR,PSON,PON */
		_delay_ms(100);

		R61509x_wr_cmd(0x0001);
		R61509x_wr_dat(0x0100);
		R61509x_wr_cmd(0x0002);
		R61509x_wr_dat(0x0100);
		R61509x_wr_cmd(0x0003);						/* Entry Mode Set  Set GRAM write direction and BGR=1 */ 
		R61509x_wr_dat((1<<12)|(1<<9)|(1<<7)|(1<<5)|(1<<4)|(0<<3));
		R61509x_wr_cmd(0x0009);
		R61509x_wr_dat(0x0001);
	
		R61509x_wr_cmd(0x000C);
		R61509x_wr_dat(0x0000);						/* MCU interface */ 

		R61509x_wr_cmd(0x0090);
		R61509x_wr_dat(0x8000);
		R61509x_wr_cmd(0x000f);
		R61509x_wr_dat(0x0000);

		R61509x_wr_cmd(0x0500);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0501);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0502);
		R61509x_wr_dat(0x005f);
		R61509x_wr_cmd(0x0401);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x0404);
		R61509x_wr_dat(0x0000);
		_delay_ms(100);	
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0100);
		_delay_ms(100);

	}

	else if(devicetype == 0x1509)
	{
		/* Initialize R61509 */
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		_delay_ms(1);
		
		R61509x_wr_cmd(0x0600);
		R61509x_wr_dat(0x0001);
		_delay_ms(15);

		R61509x_wr_cmd(0x0600);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0606);
		R61509x_wr_dat(0x0000);
		_delay_ms(1);
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0001);
		_delay_ms(1);
		R61509x_wr_cmd(0x0110);
		R61509x_wr_dat(0x0001);
		_delay_ms(1);
		
		R61509x_wr_cmd(0x0100);
		R61509x_wr_dat(0x17B0);
		R61509x_wr_cmd(0x0101);
		R61509x_wr_dat(0x0147);
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x019D);
		R61509x_wr_cmd(0x0103);
		R61509x_wr_dat(0x3600);
		R61509x_wr_cmd(0x0281);
		R61509x_wr_dat(0x0010);
		_delay_ms(1);
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x01BD);
		_delay_ms(1);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0001);
		R61509x_wr_dat((0<<10)|(1<<8));
		R61509x_wr_cmd(0x0002);
		R61509x_wr_dat(0x0100);
		R61509x_wr_cmd(0x0003);						/* Entry Mode Set  Set GRAM write direction and BGR=1 */ 
		R61509x_wr_dat((1<<12)|(1<<9)|(1<<7)|(1<<5)|(1<<4)|(0<<3));
		R61509x_wr_cmd(0x0006);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0008);
		R61509x_wr_dat(0x0503);
		R61509x_wr_cmd(0x0009);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x000B);
		R61509x_wr_dat(0x0010);
		R61509x_wr_cmd(0x000C);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x000F);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x0010);
		R61509x_wr_dat(0x0013);
		R61509x_wr_cmd(0x0011);
		R61509x_wr_dat(0x0202);
		R61509x_wr_cmd(0x0012);
		R61509x_wr_dat(0x0300);
		R61509x_wr_cmd(0x0020);
		R61509x_wr_dat(0x021E);
		R61509x_wr_cmd(0x0021);
		R61509x_wr_dat(0x0202);
		R61509x_wr_cmd(0x0022);
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0021);
		R61509x_wr_dat(0x0100);
		R61509x_wr_cmd(0x0090);
		R61509x_wr_dat(0x8000);
		R61509x_wr_cmd(0x0092);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0100);
		R61509x_wr_dat(0x16B0);
		R61509x_wr_cmd(0x0101);
		R61509x_wr_dat(0x0147);
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x01BD);
		R61509x_wr_cmd(0x0103);
		R61509x_wr_dat(0x2A00);
		R61509x_wr_cmd(0x0107);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0110);
		R61509x_wr_dat(0x0001);

		R61509x_wr_cmd(0x0280);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0281);
		R61509x_wr_dat(0x000E);
		R61509x_wr_cmd(0x0282);
		R61509x_wr_dat(0x0000);

		/* GAMMA */
		R61509x_wr_cmd(0x0300);
		R61509x_wr_dat(0x0c06);
		R61509x_wr_cmd(0x0301);
		R61509x_wr_dat(0x9d0f);
		R61509x_wr_cmd(0x0302);
		R61509x_wr_dat(0x0b05);
		R61509x_wr_cmd(0x0303);
		R61509x_wr_dat(0x1217);
		R61509x_wr_cmd(0x0304);
		R61509x_wr_dat(0x3333);
		R61509x_wr_cmd(0x0305);
		R61509x_wr_dat(0x1712);
		R61509x_wr_cmd(0x0306);
		R61509x_wr_dat(0x950b);	
		R61509x_wr_cmd(0x0307);
		R61509x_wr_dat(0x0f0d);
		R61509x_wr_cmd(0x0308);
		R61509x_wr_dat(0x060c); 
		R61509x_wr_cmd(0x0309);
		R61509x_wr_dat(0x0000);
	
		R61509x_wr_cmd(0x0400);
		R61509x_wr_dat(0x3500);
		R61509x_wr_cmd(0x0401);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x0404);
		R61509x_wr_dat(0x0000);

		R61509x_wr_cmd(0x0500);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0501);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0502);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0503);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0504);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0505);
		R61509x_wr_dat(0x0000);

		R61509x_wr_cmd(0x06F0);
		R61509x_wr_dat(0x0000);

		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0061);	
		_delay_ms(1);
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0173);	
		_delay_ms(1);

	}

	else if(devicetype == 0x5420)
	{
		/* Initialize SPFD5420A */
		R61509x_wr_cmd(0x0606);
		R61509x_wr_dat(0x0000);
		_delay_ms(1);
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0001);
		_delay_ms(1);
		R61509x_wr_cmd(0x0110);
		R61509x_wr_dat(0x0001);
		_delay_ms(1);
		R61509x_wr_cmd(0x0100);
		R61509x_wr_dat(0x17B0);
		R61509x_wr_cmd(0x0101);
		R61509x_wr_dat(0x0147);
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x019D);
		R61509x_wr_cmd(0x0103);
		R61509x_wr_dat(0x3600);
		R61509x_wr_cmd(0x0281);
		R61509x_wr_dat(0x0010);
		_delay_ms(1);
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x01BD);
		_delay_ms(1);
		R61509x_wr_cmd(0x0000);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0001);
		R61509x_wr_dat((0<<10)|(1<<8));
		R61509x_wr_cmd(0x0002);
		R61509x_wr_dat(0x0100);
		R61509x_wr_cmd(0x0003);						/* Entry Mode Set  Set GRAM write direction and BGR=1 */ 
		R61509x_wr_dat((1<<12)|(1<<9)|(1<<7)|(1<<5)|(1<<4)|(0<<3));
		R61509x_wr_cmd(0x0006);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0008);
		R61509x_wr_dat(0x0503);
		R61509x_wr_cmd(0x0009);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x000B);
		R61509x_wr_dat(0x0010);
		R61509x_wr_cmd(0x000C);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x000F);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x0010);
		R61509x_wr_dat(0x0013);
		R61509x_wr_cmd(0x0011);
		R61509x_wr_dat(0x0202);
		R61509x_wr_cmd(0x0012);
		R61509x_wr_dat(0x0300);
		R61509x_wr_cmd(0x0020);
		R61509x_wr_dat(0x021E);
		R61509x_wr_cmd(0x0021);
		R61509x_wr_dat(0x0202);
		R61509x_wr_cmd(0x0022);
		R61509x_wr_dat(0x0100);
		R61509x_wr_cmd(0x0090);
		R61509x_wr_dat(0x8000);
		R61509x_wr_cmd(0x0092);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0100);
		R61509x_wr_dat(0x16B0);
		R61509x_wr_cmd(0x0101);
		R61509x_wr_dat(0x0147);
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x01BD);
		R61509x_wr_cmd(0x0103);
		R61509x_wr_dat(0x2A00);
		R61509x_wr_cmd(0x0107);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0110);
		R61509x_wr_dat(0x0001);

		R61509x_wr_cmd(0x0280);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0281);
		R61509x_wr_dat(0x000E);
		R61509x_wr_cmd(0x0282);
		R61509x_wr_dat(0x0000);

		/* Adjust the Gamma Curve */
		R61509x_wr_cmd(0x0300);
		R61509x_wr_dat(0x0101);
		R61509x_wr_cmd(0x0301);
		R61509x_wr_dat(0x0B2C);
		R61509x_wr_cmd(0x0302);
		R61509x_wr_dat(0x1030);
		R61509x_wr_cmd(0x0303);
		R61509x_wr_dat(0x3010);
		R61509x_wr_cmd(0x0304);
		R61509x_wr_dat(0x2C0B);
		R61509x_wr_cmd(0x0305);
		R61509x_wr_dat(0x0101);
		R61509x_wr_cmd(0x0306);
		R61509x_wr_dat(0x0807);
		R61509x_wr_cmd(0x0307);
		R61509x_wr_dat(0x0708);
		R61509x_wr_cmd(0x0308);
		R61509x_wr_dat(0x0107);
		R61509x_wr_cmd(0x0309);
		R61509x_wr_dat(0x0105);
		R61509x_wr_cmd(0x030A);
		R61509x_wr_dat(0x0F04);
		R61509x_wr_cmd(0x030B);
		R61509x_wr_dat(0x0F00);
		R61509x_wr_cmd(0x030C);
		R61509x_wr_dat(0x000F);
		R61509x_wr_cmd(0x030D);
		R61509x_wr_dat(0x040F);
		R61509x_wr_cmd(0x030E);
		R61509x_wr_dat(0x0501);
		R61509x_wr_cmd(0x030F);
		R61509x_wr_dat(0x0701);
		R61509x_wr_cmd(0x0400);
		R61509x_wr_dat(0x3500);
		R61509x_wr_cmd(0x0401);
		R61509x_wr_dat(0x0001);
		R61509x_wr_cmd(0x0404);
		R61509x_wr_dat(0x0000);
		/* Partial Display Control */
		R61509x_wr_cmd(0x0500);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0501);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0502);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0503);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0504);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0505);
		R61509x_wr_dat(0x0000);
		/* Panel Control */
		R61509x_wr_cmd(0x0600);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0606);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x06F0);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x07F0);
		R61509x_wr_dat(0x5420);
		R61509x_wr_cmd(0x07F3);
		R61509x_wr_dat(0x288A);
		R61509x_wr_cmd(0x07F4);
		R61509x_wr_dat(0x0022);
		R61509x_wr_cmd(0x07F5);
		R61509x_wr_dat(0x0041);
		R61509x_wr_cmd(0x07F0);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0173);	
	}

	else if(devicetype == 0x9326)
	{
		/* Initialize ILI9326 */
		/* Start Initial Sequence */
		R61509x_wr_cmd(0x0702);
		R61509x_wr_dat(0x3008);			/* Set internal timing, don't change this value */
		R61509x_wr_cmd(0x0705);
		R61509x_wr_dat(0x0036);			/* Set internal timing, don't change this value */
		R61509x_wr_cmd(0x070B);
		R61509x_wr_dat(0x1213);			/* Set internal timing, don't change this value */
		R61509x_wr_cmd(0x0001);
		R61509x_wr_dat(0x0100);			/* set SS and SM bit */
		R61509x_wr_cmd(0x0002);
		R61509x_wr_dat(0x0100);			/* set 1 line inversion */
		R61509x_wr_cmd(0x0003);
		R61509x_wr_dat(0x1030);			/* set GRAM write direction and BGR=1, Landscape 10A8, Portrait1030 */
		R61509x_wr_cmd(0x0008);
		R61509x_wr_dat(0x0808);			/* set the back porch and front porch */
		R61509x_wr_cmd(0x0009);
		R61509x_wr_dat(0x0000);			/* set non-display area refresh cycle ISC[3:0] */
		R61509x_wr_cmd(0x000C);
		R61509x_wr_dat(0x0000);			/* RGB interface setting */
		R61509x_wr_cmd(0x000F);
		R61509x_wr_dat(0x0000);			/* RGB interface polarity */
		/* Power On sequence  */
		R61509x_wr_cmd(0x0100);
		R61509x_wr_dat(0x0000);			/* SAP,BT[3:0],AP,DSTB,SLP,STB */
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x0000);			/* VREG1OUT voltage */
		R61509x_wr_cmd(0x0103);
		R61509x_wr_dat(0x0000);			/* VDV[4:0] for VCOM amplitude */
		_delay_ms(200); 				/* Dis-charge capacitor power voltage */
		R61509x_wr_cmd(0x0100);
		R61509x_wr_dat(0x1190);			/* SAP,BT[3:0],AP,DSTB,SLP,STB */
		R61509x_wr_cmd(0x0101);
		R61509x_wr_dat(0x0017);			/* DC1[2:0],DC0[2:0],VC[2:0] */
		_delay_ms(50); 					/* Delay 50ms */
		R61509x_wr_cmd(0x0102);
		R61509x_wr_dat(0x01BD);			/* VREG1OUT voltage */
		_delay_ms(50); 					/* Delay 50ms */
		R61509x_wr_cmd(0x0103);
		R61509x_wr_dat(0x3500);			/* VDV[4:0] for VCOM amplitude */
		R61509x_wr_cmd(0x0281);
		R61509x_wr_dat(0x0017);			/* VCM[5:0] for VCOMH */
		_delay_ms(50);
		R61509x_wr_cmd(0x0200);
		R61509x_wr_dat(0x0000);			/* GRAM horizontal Address */
		R61509x_wr_cmd(0x0201);
		R61509x_wr_dat(0x0000);			/* GRAM Vertical Address */
		/* Adjust the Gamma Curve */
		R61509x_wr_cmd(0x0300);
		R61509x_wr_dat(0x0000);
		R61509x_wr_cmd(0x0301);
		R61509x_wr_dat(0x0707);
		R61509x_wr_cmd(0x0302);
		R61509x_wr_dat(0x0606);
		R61509x_wr_cmd(0x0305);
		R61509x_wr_dat(0x0006);
		R61509x_wr_cmd(0x0306);
		R61509x_wr_dat(0x0A09);
		R61509x_wr_cmd(0x0307);
		R61509x_wr_dat(0x0203);
		R61509x_wr_cmd(0x0308);
		R61509x_wr_dat(0x0005);
		R61509x_wr_cmd(0x0309);
		R61509x_wr_dat(0x0007);
		R61509x_wr_cmd(0x030C);
		R61509x_wr_dat(0x0400);
		R61509x_wr_cmd(0x030D);
		R61509x_wr_dat(0x000A);
		/* Set GRAM area */
		R61509x_wr_cmd(0x0210);
		R61509x_wr_dat(0x0000);			/* Horizontal GRAM Start Address */
		R61509x_wr_cmd(0x0211);
		R61509x_wr_dat(0x00EF);			/* Horizontal GRAM End Address */
		R61509x_wr_cmd(0x0212);
		R61509x_wr_dat(0x0000);			/* Vertical GRAM Start Address */
		R61509x_wr_cmd(0x0213);
		R61509x_wr_dat(0x01AF);			/* Vertical GRAM Start Address */
		R61509x_wr_cmd(0x0400);
		R61509x_wr_dat(0x3500);			/* Gate Scan Line 432 lines */
		R61509x_wr_cmd(0x0401);
		R61509x_wr_dat(0x0001);			/* NDL,VLE,REV */
		R61509x_wr_cmd(0x0404);
		R61509x_wr_dat(0x0000);			/* set scrolling line */
		/* Partial Display Control */
		R61509x_wr_cmd(0x0500);
		R61509x_wr_dat(0x0000);			/* Partial Image 1 Display Position */
		R61509x_wr_cmd(0x0501);
		R61509x_wr_dat(0x0000);			/* Partial Image 1 RAM Start/End Address */
		R61509x_wr_cmd(0x0502);
		R61509x_wr_dat(0x0000);			/* Partial Image 1 RAM Start/End Address */
		R61509x_wr_cmd(0x0503);
		R61509x_wr_dat(0x0000);			/* Partial Image 2 Display Position */
		R61509x_wr_cmd(0x0504);
		R61509x_wr_dat(0x0000);			/* Partial Image 2 RAM Start/End Address */
		R61509x_wr_cmd(0x0505);
		R61509x_wr_dat(0x0000);			/* Partial Image 2 RAM Start/End Address */
		/* Panel Control */
		R61509x_wr_cmd(0x0010);
		R61509x_wr_dat(0x0010);			/* DIVI[1:0];RTNI[4:0] */
		R61509x_wr_cmd(0x0011);
		R61509x_wr_dat(0x0600);			/* NOWI[2:0];SDTI[2:0] */
		R61509x_wr_cmd(0x0020);
		R61509x_wr_dat(0x0002);			/* DIVE[1:0];RTNE[5:0] */
		R61509x_wr_cmd(0x0007);
		R61509x_wr_dat(0x0173);			/* 262K color and display ON */
		_delay_ms(20);
		R61509x_wr_cmd(0x0202); 		/* Write Data to GRAM */
	}

	else { for(;;);} /* Invalid Device Code!! */

	R61509x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	R61509x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61509x_wr_dat(COL_RED);
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
int R61509x_draw_bmp(const uint8_t* ptr){

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
	R61509x_clear();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		R61509x_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
			R61509x_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
