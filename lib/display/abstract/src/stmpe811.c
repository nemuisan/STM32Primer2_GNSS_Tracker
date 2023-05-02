/********************************************************************************/
/*!
	@file			stmpe811.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.05.01
	@brief          Touch Screen Control Upperside 
	
    @section HISTORY
		2013.03.20	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "touch_if_basis.h"
/* check header file version for fool proof */
#if STMPE811_H != 0x0200
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
	STMPE811 MiddleLevel access routine 
*/
/**************************************************************************/
uint8_t STMPE811_Init(void)
{
	int i;
	i = STMPE811_ReadWord(STMPE811_CHIP_ID);
	if(i!=0x0811) return 0;
	
	/* Reset Touch-screen controller      */
	STMPE811_WriteByte(STMPE811_SYS_CTRL1,0x02);
	_delay_ms(5);
	STMPE811_WriteByte(STMPE811_SYS_CTRL1,0x00);

	/* Enable TSC and ADC                 */
	STMPE811_WriteByte(STMPE811_SYS_CTRL2, 		 0x0C);   
	/* Enable Touch detect, FIFO          */
	STMPE811_WriteByte(STMPE811_INT_EN,			 0x07);
    /* Set sample time , 12-bit mode      */
	/* 0x69:0110 1001 124clk 12bit InternalRef */
	/* 0x49:0110 1001  80clk 12bit InternalRef */
	STMPE811_WriteByte(STMPE811_ADC_CTRL1,       0x49);
	_delay_ms(10);
    /* ADC frequency 3.25 MHz             */
	STMPE811_WriteByte(STMPE811_ADC_CTRL2,       0x01);
    /* Pins are used for touchscreen      */
	STMPE811_WriteByte(STMPE811_GPIO_ALT_FUNCT,  0x00);
    /* Set TSC_CFG register               */
	/* 0xF5:1111 0101 8sample 10mSecDelay  10mSecSettling */
	/* 0x9A:1001 1010 4sample  1mSecDelay 500uSecSettling */
	/* 0xC2:1100 0010 8sample 10uSecDelay 500uSecSettling */
	STMPE811_WriteByte(STMPE811_TSC_CFG,         0xC2);
    /* Threshold for FIFO                 */
	STMPE811_WriteByte(STMPE811_FIFO_TH,         0x01);
    /* FIFO reset                         */
	STMPE811_WriteByte(STMPE811_FIFO_STA,        0x01);
    /* FIFO not reset                     */
	STMPE811_WriteByte(STMPE811_FIFO_STA,        0x00);
    /* Fraction z                         */
	STMPE811_WriteByte(STMPE811_TSC_FRACTION_Z,  0x07);
    /* Drive 50 mA typical                */
	STMPE811_WriteByte(STMPE811_TSC_I_DRIVE,     0x01);
    /* Enable TSC                         */
	STMPE811_WriteByte(STMPE811_TSC_CTRL,        0x03);
	/* Pins are used for touchscreen      */
	STMPE811_WriteByte(STMPE811_GPIO_ALT_FUNCT,  0x00);
    /* Clear interrupt status             */
	STMPE811_WriteByte(STMPE811_INT_STA,         0xFF);
    /* Enable TSC                         */
	STMPE811_WriteByte(STMPE811_INT_CTRL,        0x03);

	return 1;
}

/**************************************************************************/
/*! 
	STMPE811 MiddleLevel access routine 
*/
/**************************************************************************/
void STMPE811_WriteWord(uint8_t adrs,uint16_t data)
{

start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	if(TS_TxData_I2C(data >> 8)!=TS_ACK) goto start;
	if(TS_TxData_I2C(data & 0x00FF)!=TS_ACK) goto start;
	TS_Stop_I2C();
}

/**************************************************************************/
/*! 
	STMPE811 MiddleLevel access routine 
*/
/**************************************************************************/
void STMPE811_WriteByte(uint8_t adrs,uint8_t data)
{

start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	if(TS_TxData_I2C(data)!=TS_ACK) goto start;
	TS_Stop_I2C();
	
}

/**************************************************************************/
/*! 
	STMPE811 MiddleLevel access routine 
*/
/**************************************************************************/
uint32_t STMPE811_ReadDWord(uint8_t adrs)
{
	uint32_t data;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	data =  TS_RxData_I2C(TS_ACK);		/* ACK */
	data <<= 8;
	data |=  TS_RxData_I2C(TS_ACK);		/* ACK */
	data <<= 8;
	data |=  TS_RxData_I2C(TS_ACK);		/* ACK */
	data <<= 8;
	data |= TS_RxData_I2C(TS_NACK);		/* NACK */
	TS_Stop_I2C();

	return data;
}

/**************************************************************************/
/*! 
	STMPE811 MiddleLevel access routine 
*/
/**************************************************************************/
uint16_t STMPE811_ReadWord(uint8_t adrs)
{
	uint16_t data;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	data =  TS_RxData_I2C(TS_ACK);		/* ACK */
	data <<= 8;
	data |= TS_RxData_I2C(TS_NACK);		/* NACK */
	TS_Stop_I2C();

	return data;
}

/**************************************************************************/
/*! 
	STMPE811 MiddleLevel access routine 
*/
/**************************************************************************/
uint8_t STMPE811_ReadByte(uint8_t adrs)
{
	uint8_t data;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;

	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	data = TS_RxData_I2C(TS_NACK);	/* NACK */
	TS_Stop_I2C();

	return data;
}

/**************************************************************************/
/*! 
	STMPE811 MiddleLevel access routine 
*/
/**************************************************************************/
uint32_t STMPE811_ReadFIFO(uint8_t axis,uint8_t adrs)
{
	uint32_t data=0;
	int i=0;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	
	TS_Start_I2C();
	if(TS_TxAddr_I2C(STMPE811_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	for(;i<axis;i++){
		data =  TS_RxData_I2C(TS_ACK);		/* ACK */
		data <<= 8;
	}

	data |= TS_RxData_I2C(TS_NACK);		/* NACK */
	TS_Stop_I2C();

	return data;
}

/* End Of File ---------------------------------------------------------------*/
