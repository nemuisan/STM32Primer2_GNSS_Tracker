/********************************************************************************/
/*!
	@file			ili932x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        10.00
    @date           2013.04.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -YHY024006A				(ILI9325)	8/16bit mode.		@n
					 -EGO028Q02-F05				(ILI9325)	8/16bit mode.		@n
					 -WBX280V009				(ILI9325)	8/16bit mode.		@n
					 -V320P243282WSI7TB   		(ILI9320)	16bit mode.			@n
					 -PH240320T-030-XP1Q  		(R61505U)	8/16bit mode.		@n
					 -TFT-GG1N4198UTSW-W-TP-E  	(LGDP4531)	8/16bit,spi mode.	@n
					 -MR028-9325-51P-TP-A  		(ILI9328)	8/16bit,spi mode.	@n
					 -MR024-9325-51P-TP-B  		(ILI9325C)	8/16bit,spi mode.	@n
					 -OPFPCT0634-V0				(ILI9320)	8/16bit mode.		@n
					 -TS8026Y					(ST7781)	8bit mode only.		@n
					 -TFT1P3204-E				(R61505W)	8/16bit mode.		@n
					 -KD032C-2-TP 				(ILI9325C)	8/16bit,spi mode.   @n
					 -TFT1P2477-E				(R61505V)	8bit mode only.		@n
					 -VS3026A					(RM68050)	8/16bit mode.		@n
					 -KFM529B21-1A				(SPFD5408B)	8/16bit mode.		@n
					 -CH24Q304-LCM-A			(LGDP4535)  8/16bit mode.		@n
					 -KXM280H-T09				(ILI9331)   8/16bit mode.		@n
					 -DST2401PH					(R61580)   	8/16bit mode.		@n
					 -JLX320-002				(RM68090)   8bit mode only.		@n
					 -AT32Q02					(FT1505C)   8/16bit mode.

    @section HISTORY
		2010.03.01	V1.00	Stable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added ILI9328 device.
							Added 3-Wire Serial Support.
		2011.03.10	V4.00	C++ Ready.
		2011.10.25	V5.00	Added OPFPCT0634-V0 Consideration.
							Added DMA Transaction Support.
		2012.01.18	V6.00	Added ST7781 device.
		2012.04.20	V7.00	Added R61505W & R61505V device.
		2012.09.30	V8.00	Added SPDF5408A/B,RM68050 device.
						    Added LGDP4535 device.
		2013.01.03	V9.00   Added ILI9331,R61580,RM68090 device.
		2013.04.01 V10.00   Added FT1505C device.
							Fixed GGRAM Addressing.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili932x.h"
/* check header file version for fool proof */
#if __ILI932X_H != 0x1000
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
/* If U use OPFPCT0634-V0 Module,Uncomment below define */
/*#define MODULE_OPFPCT0634*/

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
inline void ILI932x_reset(void)
{
#ifdef USE_ILI932x_TFT
	ILI932x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI932x_RD_SET();
	ILI932x_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI932x_RES_CLR();							/* RES=L, CS=L   			*/
	ILI932x_CS_CLR();

#elif  USE_ILI932x_SPI_TFT
	ILI932x_RES_SET();							/* RES=H, CS=H				*/
	ILI932x_CS_SET();
	ILI932x_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	ILI932x_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(50);								/* wait 50ms     			*/
	ILI932x_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ILI932x_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI932x_wr_cmd(uint8_t cmd)
{
	ILI932x_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI932x_CMD = 0;
	ILI932x_WR();
#endif

	ILI932x_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	ILI932x_WR();								/* WR=L->H					*/

	ILI932x_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI932x_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI932x_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	ILI932x_WR();								/* WR=L->H					*/
#endif

	ILI932x_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	ILI932x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI932x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI932x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		ILI932x_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI932x_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	ILI932x_wr_cmd(cmd);
	ILI932x_WR_SET();

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


#elif USE_ILI932x_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI932x_wr_cmd(uint8_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI(0);
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ILI932x_wr_dat(uint16_t dat)
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
inline void ILI932x_wr_block(uint8_t *p,unsigned int cnt)
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
inline  uint16_t ILI932x_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI(0);
	SendSPI(cmd);
	
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
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI932x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI932x_wr_cmd(0x50);				/* Horizontal RAM Start ADDR */
	ILI932x_wr_dat(OFS_COL + x);
	ILI932x_wr_cmd(0x51);				/* Horizontal RAM End ADDR */
	ILI932x_wr_dat(OFS_COL + width);
	ILI932x_wr_cmd(0x52);				/* Vertical RAM Start ADDR */
	ILI932x_wr_dat(OFS_RAW + y);
	ILI932x_wr_cmd(0x53);				/* Vertical End ADDR */
	ILI932x_wr_dat(OFS_RAW + height);

	ILI932x_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	ILI932x_wr_dat(OFS_COL + x);
	ILI932x_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	ILI932x_wr_dat(OFS_RAW + y);

	ILI932x_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI932x_clear(void)
{
	volatile uint32_t n;

	ILI932x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI932x_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI932x_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	ILI932x_reset();

	/* Check Device Code */
	devicetype = ILI932x_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if((devicetype == 0x9325) || (devicetype == 0x5408) || (devicetype == 0x6809) || (devicetype == 0x6807))
	{
		/* Initialize ILI9325 & ILI9325C & RM68050 & RM68090 & SPFD5408A/B */
		ILI932x_wr_cmd(0xE3);						/* Set the internal vcore voltage */ 
		ILI932x_wr_dat(0x3008);
		ILI932x_wr_cmd(0xE7);						/* Set the internal vcore voltage */ 
		ILI932x_wr_dat(0x0012);
		ILI932x_wr_cmd(0xEF);						/* Set the internal vcore voltage */ 
		ILI932x_wr_dat(0x1231);

		ILI932x_wr_cmd(0x01);						/* Set SS and SM bit */ 
		ILI932x_wr_dat(0x0100);

		ILI932x_wr_cmd(0x02);						/* Set 1 line inversion */ 
		ILI932x_wr_dat(0x0700);

		ILI932x_wr_cmd(0x03);						/* Set GRAM write direction and BGR=1 */ 
		ILI932x_wr_dat((1<<12)|(1<<9)|(0<<7)|(1<<5)|(1<<4));

		ILI932x_wr_cmd(0x04);						/* Scalling Control */ 
		ILI932x_wr_dat(0x0000);
		
		ILI932x_wr_cmd(0x08);						/* Display Control 2(0x0207) */ 
		ILI932x_wr_dat(0x0207);						/* Set the back porch and front porch */

		ILI932x_wr_cmd(0x09);						/* Display Control 3(0x0000) */ 
		ILI932x_wr_dat(0x0000);						/* Set non-display area refresh cycle ISC[3:0] */

		ILI932x_wr_cmd(0x0a);						/* Frame Cycle Control 3(0x0000) */ 
		ILI932x_wr_dat(0x0000);						/* FMARK function */

		ILI932x_wr_cmd(0x0c);						/* Extern Display Interface Control 1(0x0000) */ 
		ILI932x_wr_dat(0x0000);						/* RGB interface setting */

		ILI932x_wr_cmd(0x0d);						/* Frame Maker Position */ 
		ILI932x_wr_dat(0x0000);

		ILI932x_wr_cmd(0x0f);						/* Extern Display Interface Control 2 */ 
		ILI932x_wr_dat(0x0000);						/* RGB interface polarity */
		
		/* ----------- Power ON Sequence ---------- */
		ILI932x_wr_cmd(0x10);						/* Power Control 1(0x16b0) */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x11);						/* Power Control 2(0x0001) */ 
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x12);						/* Power Control 3(0x0138) */ 
		ILI932x_wr_dat(0x0000);						/* VREG1OUT voltage */
		ILI932x_wr_cmd(0x13);						/* Power Control 4 */ 
		ILI932x_wr_dat(0x0000);						/* VDV[4:0] for VCOM amplitude */
		_delay_ms(200);								/* Dis-charge capacitor power voltage */

		ILI932x_wr_cmd(0x10);						/* Power Control 1(0x16b0) */ 
		ILI932x_wr_dat(0x1490);

		ILI932x_wr_cmd(0x11);						/* Power Control 2(0x0001) */ 
		ILI932x_wr_dat(0x0221);						/* R11h=0x0227 at VCI=2.8V */
		_delay_ms(50);
		
		ILI932x_wr_cmd(0x12);						/* Power Control 3(0x0138) */
		ILI932x_wr_dat(0x001c);						/* External reference voltage= Vci */
		_delay_ms(50);

		ILI932x_wr_cmd(0x13);						/* Power Control 4 */
		ILI932x_wr_dat(0x0a00);						/* R13=0F00 when R12=009E;VDV[4:0] for VCOM amplitude */

		ILI932x_wr_cmd(0x29);						/* Power Control 7 */ 
		ILI932x_wr_dat(0x000f);						/* R29=0019 when R12=009E;VCM[5:0] for VCOMH 0012  */

		ILI932x_wr_cmd(0x2b);						/* Frame Rate and Color Control */ 
		ILI932x_wr_dat(0x000d);						/* Frame Rate = 128Hz */
		_delay_ms(50);

		/* ----------- Adjust the Gamma Curve ---------- */
		ILI932x_wr_cmd(0x30);	
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x31);	
		ILI932x_wr_dat(0x0203);
		ILI932x_wr_cmd(0x32);	
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x35);	
		ILI932x_wr_dat(0x0205);
		ILI932x_wr_cmd(0x36);	
		ILI932x_wr_dat(0x030c);
		ILI932x_wr_cmd(0x37);	
		ILI932x_wr_dat(0x0607);
		ILI932x_wr_cmd(0x38);	
		ILI932x_wr_dat(0x0405);
		ILI932x_wr_cmd(0x39);	
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x3c);	
		ILI932x_wr_dat(0x0502);
		ILI932x_wr_cmd(0x3d);	
		ILI932x_wr_dat(0x1008);

		/* ------------------ Set GRAM area --------------- */
		ILI932x_wr_cmd(0x60);						/* Gate Scan Line */ 
		 if(devicetype == 0x5408)	ILI932x_wr_dat(0x2700); /* SPFD5408A/B Only! */ 
		 else					    ILI932x_wr_dat(0xa700); /* ILI9325 and so on */ 
		ILI932x_wr_cmd(0x61);						/* Driver Output Control */ 
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x6a);						/* Set scrolling line */ 
		ILI932x_wr_dat(0x0000);
		
		/* -------------- Partial Display Control --------- */
		ILI932x_wr_cmd(0x80);						/* Display Position? Partial Display 1 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x81);						/* RAM Address Start? Partial Display 1 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x82);						/* RAM Address End-Partial Display 1 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x83);						/* Display Position? Partial Display 2 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x84);						/* RAM Address Start? Partial Display 2 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x85);						/* RAM Address End-Partial Display 2 */ 
		ILI932x_wr_dat(0x0000);
		
		/* -------------- Panel Control ------------------- */
		ILI932x_wr_cmd(0x90);						/* Frame Cycle Control(0x0013) */ 
		ILI932x_wr_dat(0x0010);
		ILI932x_wr_cmd(0x92);						/* Panel Interface Control 2 */ 
		ILI932x_wr_dat(0x0600);
		ILI932x_wr_cmd(0x93);						/* Panel Interface Control 3 */ 
		ILI932x_wr_dat(0x0003);
		ILI932x_wr_cmd(0x95);						/* Frame Cycle Control(0x0110) */ 
		ILI932x_wr_dat(0x0110);
		ILI932x_wr_cmd(0x97);						/* Frame Cycle Control? */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x98);						/* Frame Cycle Control */ 
		ILI932x_wr_dat(0x0000);


		ILI932x_wr_cmd(0x07);						/* Display On */ 
		ILI932x_wr_dat((1<<8)|(1<<5)|(1<<4)|(1<<1)|(1<<0));
	}

	else if(devicetype == 0x9320)
	{
		/* Initialize ILI9320 */
		ILI932x_wr_cmd(0xE5);						/* Set the Vcore voltage and this setting is must.*/ 
		ILI932x_wr_dat(0x8000);
		ILI932x_wr_cmd(0x00);						/* Start Oscillation */ 
		ILI932x_wr_dat(0x0001); 
		_delay_ms(15);

		ILI932x_wr_cmd(0x01);						/* Driver Output Control */ 
		ILI932x_wr_dat(0x0100);
		ILI932x_wr_cmd(0x02);						/* Driver Waveform Control Set 1 line inversion */ 
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x03);						/* Entry Mode Set  Set GRAM write direction and BGR=1 */ 
		ILI932x_wr_dat((1<<12)|(1<<7)|(1<<5)|(1<<4));
		/* ILI932x_wr_dat((1<<15)|(1<<7)|(1<<4)|(1<<5)|(1<<12)); */ /* 262k colour */
		/* ILI932x_wr_dat(0x1018); */				/* original */

		ILI932x_wr_cmd(0x04);						/* Clear Resizing Control */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x08);						/* Display Control 2(0x0207) set the back porch and front porch */ 
		ILI932x_wr_dat(0x0202);
		ILI932x_wr_cmd(0x09);						/* Display Control 3(0x0000) set non-display area refresh cycle ISC[3:0] */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0a);						/* Frame Cycle Control 3(0x0000) */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0c);						/* Extern Display Interface Control 1(0x0000) */ 
		ILI932x_wr_dat((1<<0));
		ILI932x_wr_cmd(0x0d);						/* Frame Maker Position */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0f);						/* Extern Display Interface Control 2 */ 
		ILI932x_wr_dat(0x0000);	

		/* ----------- Power ON Sequence ---------- */
		ILI932x_wr_cmd(0x10);						/* Power Control 1 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x11);						/* Power Control 2 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x12);						/* Power Control 3 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x13);						/* Power Control 4 */ 
		ILI932x_wr_dat(0x0000);
		_delay_ms(200);								/* Discharge */

		ILI932x_wr_cmd(0x10);						/* Power Control 1(0x16b0) */ 
		ILI932x_wr_dat((1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));
		ILI932x_wr_cmd(0x11);						/* Power Control 2(0x0001) */ 
		ILI932x_wr_dat(0x0007);
		_delay_ms(50);

		ILI932x_wr_cmd(0x12);						/* Power Control 3(0x0138) */ 
		ILI932x_wr_dat((1<<8)|(1<<4)|(0<<0));

		_delay_ms(50);
		ILI932x_wr_cmd(0x13);						/* Power Control 4 */ 
		ILI932x_wr_dat(0x0b00);
		ILI932x_wr_cmd(0x29);						/* Power Control 7 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x2b);						/* Power Control VCM[4:0] for VCOMH */ 
		ILI932x_wr_dat((1<<4));
		_delay_ms(50);
		/* ILI932x_wr_dat((1<<15)|(1<<14)|(1<<4)); */ /* 262k colour + 16M dither */

		/* ----------- Adjust the Gamma Curve ---------- */
#ifdef MODULE_OPFPCT0634
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0606);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0304);
		ILI932x_wr_cmd(0x33);
		ILI932x_wr_dat(0x0202);
		ILI932x_wr_cmd(0x34);
		ILI932x_wr_dat(0x0202);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0103);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x011d);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0404);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0404);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0404);
		ILI932x_wr_cmd(0x3C);
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x3D);
		ILI932x_wr_dat(0x0A1F);
#else
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0203);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x0407);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0607);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0106);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x3C);
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x3D);
		ILI932x_wr_dat(0x001E);
#endif
		
		/* ------------------ Set GRAM area --------------- */
		ILI932x_wr_cmd(0x60);						/* Driver Output Control */ 
		ILI932x_wr_dat(0x2700);
		ILI932x_wr_cmd(0x61);						/* Driver Output Control */ 
#ifdef MODULE_OPFPCT0634
		ILI932x_wr_dat(0x0000);
#else
		ILI932x_wr_dat(0x0001);
#endif

		ILI932x_wr_cmd(0x6a);						/* Vertical Srcoll Control */ 
		ILI932x_wr_dat(0x0000);

		/* -------------- Partial Display Control --------- */
		ILI932x_wr_cmd(0x80);						/* Display Position? Partial Display 1 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x81);						/* RAM Address Start? Partial Display 1 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x82);						/* RAM Address End-Partial Display 1 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x83);						/* Display Position? Partial Display 2 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x84);						/* RAM Address Start? Partial Display 2 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x85);						/* RAM Address End-Partial Display 2 */ 
		ILI932x_wr_dat(0x0000);
		
		/* -------------- Panel Control ------------------- */
		ILI932x_wr_cmd(0x90);						/* Frame Cycle Control(0x0013) */ 
		ILI932x_wr_dat((0<<7)|(16<<0));
		ILI932x_wr_cmd(0x92);						/* Panel Interface Control 2 */ 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x93);						/* Panel Interface Control 3 */ 
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x95);						/* Frame Cycle Control(0x0110) */ 
		ILI932x_wr_dat(0x0110);
		ILI932x_wr_cmd(0x97);						/* Frame Cycle Control? */ 
		ILI932x_wr_dat((0<<8));
		ILI932x_wr_cmd(0x98);						/* Frame Cycle Control */ 
		ILI932x_wr_dat(0x0000);

		ILI932x_wr_cmd(0x07);						/* Display On */ 
		ILI932x_wr_dat((1<<8)|(1<<5)|(1<<4)|(1<<1)|(1<<0));
	}

	else if(devicetype == 0x1505 || devicetype == 0x0505)
	{
		/* Initialize R61505 & R61505U */
		ILI932x_wr_cmd(0x00);
		ILI932x_wr_dat(0x0000);
		
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0001);
		_delay_ms(50);  
		
		ILI932x_wr_cmd(0x17);
		ILI932x_wr_dat(0x0001);
		_delay_ms(50);
		
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x17b0);
		ILI932x_wr_cmd(0x11);
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x011a);
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x0f00);
		ILI932x_wr_cmd(0x15);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x29);
		ILI932x_wr_dat(0x0009);
		ILI932x_wr_cmd(0xfd);
		ILI932x_wr_dat(0x0000);
		_delay_ms(50);
		
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x013a);
		_delay_ms(250);
		
		ILI932x_wr_cmd(0x01);
		ILI932x_wr_dat(0x0100);
		ILI932x_wr_cmd(0x02);
		ILI932x_wr_dat(0x0700);

		ILI932x_wr_cmd(0x03);						/* Entry Mode Set  Set GRAM write direction and BGR=1 */ 
		ILI932x_wr_dat((1<<12)|(1<<9)|(1<<7)|(1<<5)|(1<<4));

		ILI932x_wr_cmd(0x08);
		ILI932x_wr_dat(0x0808);
		ILI932x_wr_cmd(0x09);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0a);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0c);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0d);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x33);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x34);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x3a);
		ILI932x_wr_dat(0x0303);
		ILI932x_wr_cmd(0x3b);
		ILI932x_wr_dat(0x0303);
		ILI932x_wr_cmd(0x3c);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x3d);
		ILI932x_wr_dat(0x0808);
		
		ILI932x_wr_cmd(0x60);
		ILI932x_wr_dat(0x2700);
		ILI932x_wr_cmd(0x61);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x6a);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x90);
		ILI932x_wr_dat(0x0010);
		ILI932x_wr_cmd(0x92);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x93);
		ILI932x_wr_dat(0x0000);
	
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0021);
		_delay_ms(10);

		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0061);
		_delay_ms(250);

		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0173);
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0407);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0203);
		ILI932x_wr_cmd(0x33);
		ILI932x_wr_dat(0x0303);
		ILI932x_wr_cmd(0x34);
		ILI932x_wr_dat(0x0303);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0202);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x001f);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0407);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0203);
		ILI932x_wr_cmd(0x3a);
		ILI932x_wr_dat(0x0303);
		ILI932x_wr_cmd(0x3b);
		ILI932x_wr_dat(0x0303);
		ILI932x_wr_cmd(0x3c);
		ILI932x_wr_dat(0x0202);
		ILI932x_wr_cmd(0x3d);
		ILI932x_wr_dat(0x001f);
		_delay_ms(20);
	}

	else if(devicetype == 0x4531 || devicetype == 0x3145) /* 0x3145 is SPIMODE work around */
	{
		/* LGDP4531 */
		/* From Nemui-San
           CAUTION!! THIS INITIALIZE CODE IS UNSTABLE !!
		   TO OBTAIN CERTAIN CONTROL, YOU MUST GET INITIALIZATION
           WITHIN 5SEC SINCE POWERED TFT-LCM's MAIN VCC !! 
		*/
		ILI932x_wr_cmd(0x00);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x0628);
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x0006);
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x0A32);
		ILI932x_wr_cmd(0x11);
		ILI932x_wr_dat(0x0040);
		ILI932x_wr_cmd(0x15);
		ILI932x_wr_dat(0x0050);
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x0016);
		_delay_ms(50);
		
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x5660);
		_delay_ms(50);

		ILI932x_wr_cmd(0x13); 
		ILI932x_wr_dat(0x2A4E);
		ILI932x_wr_cmd(0x01); 
		ILI932x_wr_dat(0x0100);	
		ILI932x_wr_cmd(0x02); 
		ILI932x_wr_dat(0x0300);
		ILI932x_wr_cmd(0x03); 
		ILI932x_wr_dat((1<<12)|(1<<9)|(1<<7)|(1<<5)|(1<<4));
		ILI932x_wr_cmd(0x08); 
		ILI932x_wr_dat(0x0202);
		ILI932x_wr_cmd(0x0A); 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x30); 
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0402);
		ILI932x_wr_cmd(0x32); 
		ILI932x_wr_dat(0x0106);
		ILI932x_wr_cmd(0x33); 
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x34); 
		ILI932x_wr_dat(0x0104);
		ILI932x_wr_cmd(0x35); 
		ILI932x_wr_dat(0x0301);
		ILI932x_wr_cmd(0x36); 
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x37); 
		ILI932x_wr_dat(0x0305);
		ILI932x_wr_cmd(0x38); 
		ILI932x_wr_dat(0x0208);
		ILI932x_wr_cmd(0x39); 
		ILI932x_wr_dat(0x0F0B);
		_delay_ms(50);
	
		ILI932x_wr_cmd(0x41); 
		ILI932x_wr_dat(0x0002);
		ILI932x_wr_cmd(0x60); 
		ILI932x_wr_dat(0x2700);
		ILI932x_wr_cmd(0x61); 
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x90);
		ILI932x_wr_dat(0x0119);
		ILI932x_wr_cmd(0x92); 
		ILI932x_wr_dat(0x010A);
		ILI932x_wr_cmd(0x93); 
		ILI932x_wr_dat(0x0004);
		ILI932x_wr_cmd(0xA0); 
		ILI932x_wr_dat(0x0100);
		_delay_ms(50);
		ILI932x_wr_cmd(0x07); 
		ILI932x_wr_dat(0x0133);
		_delay_ms(50);
		ILI932x_wr_cmd(0xA0); 
		ILI932x_wr_dat(0x0000);

	}

	else if(devicetype == 0x9328)
	{
		/* Initialize ILI9328 */
		ILI932x_wr_cmd(0xE3);						/* Set the internal vcore voltage */ 
		ILI932x_wr_dat(0x3008);
		ILI932x_wr_cmd(0xE7);						/* Set the internal vcore voltage */ 
		ILI932x_wr_dat(0x0012);
		ILI932x_wr_cmd(0xEF);						/* Set the internal vcore voltage */ 
		ILI932x_wr_dat(0x1231);
		
		ILI932x_wr_cmd(0x01); 					/* set SS and SM bit 					*/
		ILI932x_wr_dat(0x0100);
		ILI932x_wr_cmd(0x02); 					/* set 1 line inversion 				*/
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x03);					/* Set GRAM write direction and BGR=1   */ 
		ILI932x_wr_dat((1<<12)|(1<<7)|(1<<5)|(1<<4));/* ILI932x_wr_dat(0x1030); *//* original */
		ILI932x_wr_cmd(0x04);					/* Resize register						*/
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x08); 					/* set the back porch and front porch	*/
		ILI932x_wr_dat(0x0207);
		ILI932x_wr_cmd(0x09);					 /* set non-display area refresh cycle ISC[3:0]	*/
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0A); 					/* FMARK function						*/
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0C); 					/* RGB interface setting				*/
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0D); 					/* Frame marker Position				*/
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0F); 					/* RGB interface polarity				*/
		ILI932x_wr_dat(0x0000);

		/* Power On sequence */
		ILI932x_wr_cmd(0x10); 					/* SAP, BT[3:0], AP, DSTB, SLP, STB		*/
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x11);	 				/* DC1[2:0], DC0[2:0], VC[2:0]			*/
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x12); 					/* VREG1OUT voltage						*/
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x13); 					/* VDV[4:0] for VCOM amplitude			*/
		ILI932x_wr_dat(0x0000);
		_delay_ms(200); 						/* Dis-charge capacitor power voltage   */
		ILI932x_wr_cmd(0x10);	 				/* SAP, BT[3:0], AP, DSTB, SLP, STB		*/
		ILI932x_wr_dat(0x1290);
		ILI932x_wr_cmd(0x11); 					/* DC1[2:0], DC0[2:0], VC[2:0]		*/
		ILI932x_wr_dat(0x0227);
		_delay_ms(50);      
		ILI932x_wr_cmd(0x12); 					/* Internal reference voltage= Vci;	*/
		ILI932x_wr_dat(0x001A);
		_delay_ms(50);      
		ILI932x_wr_cmd(0x13); 					/* Set VDV[4:0] for VCOM amplitude	*/
		ILI932x_wr_dat(0x1800);
		ILI932x_wr_cmd(0x29); 					/* Set VCM[5:0] for VCOMH			*/
		ILI932x_wr_dat(0x0028);
		ILI932x_wr_cmd(0x2B); 					/* Set Frame Rate					*/
		ILI932x_wr_dat(0x000C);
		_delay_ms(50);      

		/* Adjust the Gamma Curve */
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0305);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0003);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0304);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x000F);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0407);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0204);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x3C);
		ILI932x_wr_dat(0x0403);
		ILI932x_wr_cmd(0x3D);
		ILI932x_wr_dat(0x1604);

		/* Set GRAM area */
		ILI932x_wr_cmd(0x60); 					/* Gate Scan Line		*/
		ILI932x_wr_dat(0xA700);
		ILI932x_wr_cmd(0x61); 					/* NDL,VLE, REV			*/
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x6A); 					/* set scrolling line	*/
		ILI932x_wr_dat(0x0000);

		/* Partial Display Control */
		ILI932x_wr_cmd(0x80);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x81);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x82);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x83);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x84);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x85);
		ILI932x_wr_dat(0x0000);

		/* Panel Control */
		ILI932x_wr_cmd(0x90);
		ILI932x_wr_dat(0x0010);
		ILI932x_wr_cmd(0x92);
		ILI932x_wr_dat(0x0600);
		ILI932x_wr_cmd(0x07);						/* Display On */ 
		ILI932x_wr_dat((1<<8)|(1<<5)|(1<<4)|(1<<1)|(1<<0));

	}

	else if(devicetype == 0x7783)
	{
		/* Initialize ST7781 */
		ILI932x_wr_cmd(0xFF);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0xF3);
		ILI932x_wr_dat(0x0008);
		ILI932x_wr_cmd(0xF3);

		/* Display control setting */
		ILI932x_wr_cmd(0x01);				/* output direct */
		ILI932x_wr_dat(0x0100);
		ILI932x_wr_cmd(0x02);				/* line inversion */
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x03);				/* entry mode (65K,write ram direction ,BGR) */
		ILI932x_wr_dat((1<<12)|(1<<7)|(1<<5)|(1<<4));//ILI932x_wr_dat(0x1030);
		ILI932x_wr_cmd(0x08);				/* Front porch=3,Back porch=2 */
		ILI932x_wr_dat(0x0302);
		ILI932x_wr_cmd(0x09);				/* scan cycle */
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0A);				/* Fmark On */
		ILI932x_wr_dat(0x0000);

		/* initial power status */
		ILI932x_wr_cmd(0x10);				/* power control1 */
		ILI932x_wr_dat(0x0790);
		ILI932x_wr_cmd(0x11);				/* power control2 */ 
		ILI932x_wr_dat(0x0005);
		ILI932x_wr_cmd(0x12);				/* power control3 */
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x13);				/* power control4 */
		ILI932x_wr_dat(0x0000);
		_delay_ms(100);       				/* 100ms */

		/* Power supply startup 1 setting */
		ILI932x_wr_cmd(0x10);				/* power control1 */
		ILI932x_wr_dat(0x12b0);
		_delay_ms(50);        				/* 50ms */
		ILI932x_wr_cmd(0x11);				/* power control2 */
		ILI932x_wr_dat(0x0007);
		_delay_ms(50);            			/* 50ms */ 
		ILI932x_wr_cmd(0x12);				/* power control3 */
		ILI932x_wr_dat(0x008c);
		ILI932x_wr_cmd(0x13);				/* power control4 */
		ILI932x_wr_dat(0x1700);
		ILI932x_wr_cmd(0x29);				/* Vcomh setting */
		ILI932x_wr_dat(0x0022);
		_delay_ms(50);            			/*50ms */

		/* default gamma (CABC OFF) */
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0505);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0205);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0206);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x0408);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0504);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0206);
		ILI932x_wr_cmd(0x3c);
		ILI932x_wr_dat(0x0206);
		ILI932x_wr_cmd(0x3d);
		ILI932x_wr_dat(0x0408);
	
		ILI932x_wr_cmd(0x60);				/* Gate scan control */
		ILI932x_wr_dat(0xa700);
		ILI932x_wr_cmd(0x61);				/* Non-display Area setting */
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x90);				/* RTNI setting */
		ILI932x_wr_dat(0x0033);
		ILI932x_wr_cmd(0x07);				/* display on */
		ILI932x_wr_dat(0x0133);
		_delay_ms(50);

	}
	
	else if((devicetype == 0xB505) || (devicetype == 0xC505))
	{
		/* Initialize R61505W and R61505V */
		_delay_ms(10);
		ILI932x_wr_cmd(0x00);
		ILI932x_wr_cmd(0x00);
		ILI932x_wr_cmd(0x00);
		ILI932x_wr_cmd(0x00);
		ILI932x_wr_cmd(0xA4);
		ILI932x_wr_dat(0x0001);
		_delay_ms(10);

		ILI932x_wr_cmd(0x60);			/* NL */
		ILI932x_wr_dat(0x2700);
		ILI932x_wr_cmd(0x08);			/* FP & BP */
		ILI932x_wr_dat(0x0808);
		ILI932x_wr_cmd(0x30);			/* Gamma settings */
		ILI932x_wr_dat(0x0214);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x3715);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0604);
		ILI932x_wr_cmd(0x33);
		ILI932x_wr_dat(0x0E16);
		ILI932x_wr_cmd(0x34);
		ILI932x_wr_dat(0x2211);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x1500);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x8507);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x1407);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x1403);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0020);
		ILI932x_wr_cmd(0x90);			/* DIVI & RTNI */
		ILI932x_wr_dat(0x0015);
		ILI932x_wr_cmd(0x10);			/* BT,AP */
		ILI932x_wr_dat(0x0410);
		ILI932x_wr_cmd(0x11);			/* VC,DC0,DC1 */
		ILI932x_wr_dat(0x0237);
		ILI932x_wr_cmd(0x29);			/* VCM1 */
		ILI932x_wr_dat(0x0046);
		ILI932x_wr_cmd(0x2A);			/* VCMSEL,VCM2 */
		ILI932x_wr_dat(0x0046);
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x12);			/* VRH,VCMR,PSON=0,PON=0 */
		ILI932x_wr_dat(0x0189);
		ILI932x_wr_cmd(0x13);			/* VDV */
		ILI932x_wr_dat(0x1100);
		_delay_ms(150);

		ILI932x_wr_cmd(0x12);			/* PSON=1,PON=1 */
		ILI932x_wr_dat(0x01B9);
		ILI932x_wr_cmd(0x01);			/* Other mode settings */
		ILI932x_wr_dat(0x0100);
		ILI932x_wr_cmd(0x02);			/* BC0=1--Line inversion */
		ILI932x_wr_dat(0x0200);
		ILI932x_wr_cmd(0x03);
		ILI932x_wr_dat((1<<12)|(1<<9)|(1<<7)|(1<<5)|(1<<4));
		/* ILI932x_wr_dat(0x1030); */				/* original */
		ILI932x_wr_cmd(0x09);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x0A);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0D);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x0E);			/* VCOM equalize */
		ILI932x_wr_dat(0x0030);

		ILI932x_wr_cmd(0x61);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x6A);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x80);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x81);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x82);
		ILI932x_wr_dat(0x005F);
		ILI932x_wr_cmd(0x92);
		ILI932x_wr_dat(0x0100);
		ILI932x_wr_cmd(0x93);
		ILI932x_wr_dat(0x0701);
		_delay_ms(80);

		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0100);			/* BASEE=1--Display On */

	}

	else if(devicetype == 0x4535)
	{
		/* Initialize LGDP4535 */
		ILI932x_wr_cmd(0x15);
		ILI932x_wr_dat(0x0030); 
		ILI932x_wr_cmd(0x9A);
		ILI932x_wr_dat(0x0010); 
		ILI932x_wr_cmd(0x11);
		ILI932x_wr_dat(0x0020);
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x3428); 
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x0002); 
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x1038);
		_delay_ms(40);
	
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x0012);
		_delay_ms(40);
	
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x3420); 
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x303E);
		_delay_ms(70);

		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0000); 
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0402); 
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0307); 
		ILI932x_wr_cmd(0x33);
		ILI932x_wr_dat(0x0304); 
		ILI932x_wr_cmd(0x34);
		ILI932x_wr_dat(0x0004); 
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0401); 
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x0707); 
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0305); 
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0610); 
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0610); 

		ILI932x_wr_cmd(0x01);
		ILI932x_wr_dat(0x0100); 
		ILI932x_wr_cmd(0x02);
		ILI932x_wr_dat(0x0300); 
		ILI932x_wr_cmd(0x03);
		ILI932x_wr_dat((1<<12)|(0<<9)|(1<<7)|(1<<5)|(1<<4)); 
		ILI932x_wr_cmd(0x08);
		ILI932x_wr_dat(0x0808); 
		ILI932x_wr_cmd(0x0A);
		ILI932x_wr_dat(0x0008);

		ILI932x_wr_cmd(0x60);
		ILI932x_wr_dat(0x2700); 
		ILI932x_wr_cmd(0x61);
		ILI932x_wr_dat(0x0001); 
		ILI932x_wr_cmd(0x90);
		ILI932x_wr_dat(0x013E); 
		ILI932x_wr_cmd(0x92);
		ILI932x_wr_dat(0x0100); 
		ILI932x_wr_cmd(0x93);
		ILI932x_wr_dat(0x0100); 
	
		ILI932x_wr_cmd(0xA0);
		ILI932x_wr_dat(0x3000); 
		ILI932x_wr_cmd(0xA3);
		ILI932x_wr_dat(0x0010);
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0001); 
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0021); 
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0023); 
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0033); 
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0133); 
	}

	else if(devicetype == 0x9331)
	{
		/* Initialize ILI9331 */ 
		ILI932x_wr_cmd(0xE7);
		ILI932x_wr_dat(0x1014);
		ILI932x_wr_cmd(0x01);
		ILI932x_wr_dat((0<<10)|(1<<8));
		ILI932x_wr_cmd(0x02);
		ILI932x_wr_dat(0x0200);		/* set 1 line inversion */
		ILI932x_wr_cmd(0x03);		/* Set GRAM write direction and BGR=1 */ 
		ILI932x_wr_dat((1<<12)|(1<<7)|(1<<5)|(1<<4));
		ILI932x_wr_cmd(0x08);		/* display control2 porch 2line */ 
		ILI932x_wr_dat(0x0202);
		ILI932x_wr_cmd(0x09);
		ILI932x_wr_dat(0x0000);		/* display control3 (set nodisplay area cycle) */ 
		ILI932x_wr_cmd(0x0A);
		ILI932x_wr_dat(0x0000);		/* display control4 FMARK function */ 
		ILI932x_wr_cmd(0x0C);
		ILI932x_wr_dat(0x0001);		/* RGB interface 16bit */ 
		ILI932x_wr_cmd(0x0D);
		ILI932x_wr_dat(0x0000);		/* Frame marker */ 
		ILI932x_wr_cmd(0x0F);
		ILI932x_wr_dat(0x0000);		/* RGB interface polarity */   
     
		/* power on sequence*/   
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x0000);		/* slp,stb */ 
		ILI932x_wr_cmd(0x11);
		ILI932x_wr_dat(0x0007);   
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x0000);   
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x0000);   
		_delay_ms(20);  
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x1690);		/* slp,stb */ 
		ILI932x_wr_cmd(0x11);
		ILI932x_wr_dat(0x0227);   
		_delay_ms(5);  
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x000C);   
		_delay_ms(5);  
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x0800);   
     
		ILI932x_wr_cmd(0x29);
		ILI932x_wr_dat(0x0011);		/* slp,stb */ 
		ILI932x_wr_cmd(0x2B);
		ILI932x_wr_dat(0x000B);		/* fram rate */  
		_delay_ms(50);
	
		/* adjust the gamma curve */   
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0000);   
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0106);   
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0000);   
   
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0204);   
   
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x160A);   
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0707);   
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0106);   
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0707);   
		ILI932x_wr_cmd(0x3C);
		ILI932x_wr_dat(0x0402);   
		ILI932x_wr_cmd(0x3D);
		ILI932x_wr_dat(0x0C0F);   
		/* set gram area */ 
		ILI932x_wr_cmd(0x60);
		ILI932x_wr_dat(0x2700);		/* gate scan */
		ILI932x_wr_cmd(0x61);
		ILI932x_wr_dat(0x0001);		/* ndl vle rev */ 
		ILI932x_wr_cmd(0x6A);
		ILI932x_wr_dat(0x0000);		/* scrolling line */ 
		/* partial dispaly */   
		ILI932x_wr_cmd(0x80);
		ILI932x_wr_dat(0x0000);   
		ILI932x_wr_cmd(0x81);
		ILI932x_wr_dat(0x0000);   
		ILI932x_wr_cmd(0x82);
		ILI932x_wr_dat(0x0000);   
		ILI932x_wr_cmd(0x83);
		ILI932x_wr_dat(0x0000);   
		ILI932x_wr_cmd(0x84);
		ILI932x_wr_dat(0x0000);   
		ILI932x_wr_cmd(0x85);
		ILI932x_wr_dat(0x0000);   
		/* panel control */   
		ILI932x_wr_cmd(0x90);
		ILI932x_wr_dat(0x0010);   
		ILI932x_wr_cmd(0x92);
		ILI932x_wr_dat(0x0600);   
     
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0133);		 /* 262 or8  Color and display on */ 
	}

	else if(devicetype == 0x1580)
	{
		/* Initialize R61580 */
		ILI932x_wr_cmd(0xA4);
		ILI932x_wr_dat(0x0001);		 /* CALB=1 */ 
		_delay_ms(2);

		ILI932x_wr_cmd(0x60);
		ILI932x_wr_dat(0xA700);		 /* Driver Output Control */ 
		ILI932x_wr_cmd(0x08);
		ILI932x_wr_dat(0x0808);		 /* Display Control BP=8, FP=8 */ 
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0111);		 /* y control */ 
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x2410);		 /* y control */ 
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0501);		 /* y control */ 
		ILI932x_wr_cmd(0x33);
		ILI932x_wr_dat(0x050C);		 /* y control */ 
		ILI932x_wr_cmd(0x34);
		ILI932x_wr_dat(0x2211);		 /* y control */ 
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0C05);		 /* y control */ 
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x2105);		 /* y control */ 
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x1004);		 /* y control */ 
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x1101);		 /* y control */ 
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x1122);		 /* y control */ 
		ILI932x_wr_cmd(0x90);
		ILI932x_wr_dat(0x0019);		 /* 80Hz */ 
		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x0530);		 /* Power Control */ 
		ILI932x_wr_cmd(0x11);
		ILI932x_wr_dat(0x0237);
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x01BF);
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x1300);
		_delay_ms(100);

		ILI932x_wr_cmd(0x01);
		ILI932x_wr_dat((0<<10)|(1<<8));
		ILI932x_wr_cmd(0x02);
		ILI932x_wr_dat(0x0200);		/* set 1 line inversion */
		ILI932x_wr_cmd(0x03);		/* Set GRAM write direction and BGR=1 */ 
		ILI932x_wr_dat((1<<12)|(1<<7)|(1<<5)|(1<<4));

		ILI932x_wr_cmd(0x09);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x0A);
		ILI932x_wr_dat(0x0008);
		ILI932x_wr_cmd(0x0C);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x0D);
		ILI932x_wr_dat(0xD000);
		ILI932x_wr_cmd(0x0E);
		ILI932x_wr_dat(0x0030);
		ILI932x_wr_cmd(0x0F);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x20);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x21);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x29);
		ILI932x_wr_dat(0x0077);

		ILI932x_wr_cmd(0x61);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x6A);
		ILI932x_wr_dat(0x0000);

		ILI932x_wr_cmd(0x80);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x81);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x82);
		ILI932x_wr_dat(0x005F);

		ILI932x_wr_cmd(0x93);
		ILI932x_wr_dat(0x0701);

		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0100);
	}

	else if(devicetype == 0x0001)
	{
		/* Initialize FT1505C */
		/* Start intial Sequence */  	
		ILI932x_wr_cmd(0x2B);
		ILI932x_wr_dat(0x0003);
		ILI932x_wr_cmd(0x00);
		ILI932x_wr_dat(0x0001);
		_delay_ms(50);
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0000);
		_delay_ms(50);
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x0000);
		_delay_ms(50);
		ILI932x_wr_cmd(0x60);
		ILI932x_wr_dat(0xA700);
		ILI932x_wr_cmd(0x08);
		ILI932x_wr_dat(0x0405);

		/* Set gamma */
		ILI932x_wr_cmd(0x30);
		ILI932x_wr_dat(0x0001);
		ILI932x_wr_cmd(0x31);
		ILI932x_wr_dat(0x0303);
		ILI932x_wr_cmd(0x32);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x35);
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x36);
		ILI932x_wr_dat(0x1006);
		ILI932x_wr_cmd(0x37);
		ILI932x_wr_dat(0x0107);
		ILI932x_wr_cmd(0x38);
		ILI932x_wr_dat(0x0703);
		ILI932x_wr_cmd(0x39);
		ILI932x_wr_dat(0x0707);
		ILI932x_wr_cmd(0x3c);
		ILI932x_wr_dat(0x0005);
		ILI932x_wr_cmd(0x3d);
		ILI932x_wr_dat(0x0A1F);

		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x02C0);
		ILI932x_wr_cmd(0x11);
		ILI932x_wr_dat(0x0247);
		_delay_ms(100);
		ILI932x_wr_cmd(0x12);
		ILI932x_wr_dat(0x0118);
		_delay_ms(30);
		ILI932x_wr_cmd(0x13);
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x29);
		ILI932x_wr_dat(0x0000);
		_delay_ms(30);

		ILI932x_wr_cmd(0x01);
		ILI932x_wr_dat(0x0100);
		ILI932x_wr_cmd(0x02);
		ILI932x_wr_dat(0x0700);
		ILI932x_wr_cmd(0x03);		/* Set GRAM write direction and BGR=1 */ 
		ILI932x_wr_dat((1<<12)|(0<<9)|(0<<7)|(1<<5)|(1<<4)|(0<<3));
		ILI932x_wr_cmd(0x61);
		ILI932x_wr_dat(0x0007);
		ILI932x_wr_cmd(0x90);
		ILI932x_wr_dat(0x0110);
		ILI932x_wr_cmd(0x92);
		ILI932x_wr_dat(0x0000);

		ILI932x_wr_cmd(0x80);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x81);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x82);
		ILI932x_wr_dat(0x013f);
		ILI932x_wr_cmd(0x83);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x84);
		ILI932x_wr_dat(0x0000);
		ILI932x_wr_cmd(0x85);
		ILI932x_wr_dat(0x013f);

		ILI932x_wr_cmd(0x10);
		ILI932x_wr_dat(0x12f0);
		_delay_ms(20);

		/* Plan Control */
		ILI932x_wr_cmd(0x07);
		ILI932x_wr_dat(0x0133);
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI932x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI932x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI932x_wr_dat(COL_RED);
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
int ILI932x_draw_bmp(const uint8_t* ptr){

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
	ILI932x_clear();

  	/* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

  		/* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		ILI932x_rect(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* 65k colour access */
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
		#if 0
			/* 262k colour access */
			d2 = *p << 14;	  		/* Blue  */
			d  = (*p++ >> 4);		/* Blue  */
			d |= ((*p++>>2)<<4);	/* Green */
			d |= ((*p++>>2)<<10);	/* Red   */
			/* 262k +16M dither colour access */
			d2 = *p++ << 8;			/* Blue  */
			d  = *p++;				/* Green */
			d |= *p++ << 8;			/* Red   */
		#endif
			ILI932x_wr_dat(d);

		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
