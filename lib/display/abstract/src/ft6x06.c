/********************************************************************************/
/*!
	@file			ft6x06.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.05.01
	@brief          Touch Screen Control Upperside 						@n
					Based on ST Microelectronics's Sample Thanks!
	
    @section HISTORY
		2016.06.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "touch_if_basis.h"
/* check header file version for fool proof */
#if FT6x06_H != 0x0200
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
uint8_t FT6x06_Init(void)
{
	uint8_t id;
	id = FT6x06_ReadByte(FT6x06_CHIP_ID_REG);
	if( (FT6206_ID_VALUE == id) ||
		(FT6x06_ID_VALUE == id) ||
		(FT5336_ID_VALUE == id) ||
		(FT5216_ID_VALUE == id) ||
		(FT5316_ID_VALUE == id) ||
	    (FT5x06_ID_VALUE == id) ) return id;
	else return 0;
}

/**************************************************************************/
/*! 
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
void FT6x06_WriteWord(uint8_t adrs,uint16_t data)
{

start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	if(TS_TxData_I2C(data >> 8)!=TS_ACK) goto start;
	if(TS_TxData_I2C(data & 0x00FF)!=TS_ACK) goto start;
	TS_Stop_I2C();
}

/**************************************************************************/
/*! 
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
void FT6x06_WriteByte(uint8_t adrs,uint8_t data)
{

start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	if(TS_TxData_I2C(data)!=TS_ACK) goto start;
	TS_Stop_I2C();
	
}

/**************************************************************************/
/*! 
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
uint8_t FT6x06_ReadNBytes(uint8_t adrs,uint8_t* buf,uint8_t len)
{

start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	for(int i=0;i<len;i++)
	{
		if(i==(len-1))  *(buf+i)=TS_RxData_I2C(TS_NACK);	/* NACK */
		else            *(buf+i)=TS_RxData_I2C(TS_ACK);		/* ACK */
	}		
	TS_Stop_I2C();

	return 1;
}

/**************************************************************************/
/*! 
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
uint32_t FT6x06_ReadDWord(uint8_t adrs)
{
	uint32_t data;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
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
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
uint16_t FT6x06_ReadWord(uint8_t adrs)
{
	uint16_t data;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	data =  TS_RxData_I2C(TS_ACK);		/* ACK */
	data <<= 8;
	data |= TS_RxData_I2C(TS_NACK);		/* NACK */
	TS_Stop_I2C();

	return data;
}

/**************************************************************************/
/*! 
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
uint8_t FT6x06_ReadByte(uint8_t adrs)
{
	uint8_t data;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;

	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	data = TS_RxData_I2C(TS_NACK);	/* NACK */
	TS_Stop_I2C();

	return data;
}

/**************************************************************************/
/*! 
	FT6x06 MiddleLevel access routine 
*/
/**************************************************************************/
uint32_t FT6x06_ReadFIFO(uint8_t axis,uint8_t adrs)
{
	uint32_t data=0;
	int i=0;
start:
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_WRITE)!=TS_ACK) goto start;
	if(TS_TxData_I2C(adrs)!=TS_ACK) goto start;
	
	TS_Start_I2C();
	if(TS_TxAddr_I2C(FT6x06_ADDR|TS_SD_READ)!=TS_ACK) goto start;
	
	for(;i<axis;i++){
		data =  TS_RxData_I2C(TS_ACK);		/* ACK */
		data <<= 8;
	}

	data |= TS_RxData_I2C(TS_NACK);		/* NACK */
	TS_Stop_I2C();

	return data;
}

/* End Of File ---------------------------------------------------------------*/
