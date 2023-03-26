/********************************************************************************/
/*!
	@file			usb_msc_mass_mal.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2023.03.08
	@brief          Descriptor Header for Mal.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.03.21	V2.00	Adopted FatFs10.0a
		2016.01.15	V3.00	Changed definition compatibility.
		2017.01.15	V4.00	Fixed return correct disk capacity on eMMC.
		2017.03.29	V5.00	Fixed capacity calculation.
		2019.09.20	V6.00	Fixed shadowed variable.
		2023.03.08	V7.00	Added external declaration.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_msc_mass_mal.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint64_t Mass_Memory_Size[MAX_LUN];
uint32_t Mass_Block_Size[MAX_LUN];
uint32_t Mass_Block_Count[MAX_LUN];

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  Initializes the Media on the STM32.
*/
/**************************************************************************/
uint16_t MAL_Init(uint8_t lun)
{
	switch (lun)
	{
		case LUN_SDCARD:
			if ( SD_Init() != SD_OK ) return MAL_FAIL;
			break;
			
		default:
			return MAL_FAIL;
	}
	return MAL_OK;
}

/**************************************************************************/
/*! 
    @brief  Write sectors.
*/
/**************************************************************************/
uint16_t MAL_Write(uint8_t lun, uint64_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{

	switch (lun)
	{
		case LUN_SDCARD:
			if (SD_WriteBlock((uint8_t*)Writebuff, Memory_Offset, Transfer_Length) != SD_OK ) return MAL_FAIL;
			break;
			
		default:
			return MAL_FAIL;
	}
	return MAL_OK;
}

/**************************************************************************/
/*! 
    @brief  Read sectors.
*/
/**************************************************************************/
uint16_t MAL_Read(uint8_t lun, uint64_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{
	switch (lun)
	{
		case LUN_SDCARD:
			if (SD_ReadBlock((uint8_t*)Readbuff, Memory_Offset, Transfer_Length) != SD_OK ) return MAL_FAIL;
			break;

		default:
			return MAL_FAIL;
	}
	return MAL_OK;
}

/**************************************************************************/
/*! 
    @brief  Get status.
*/
/**************************************************************************/
uint16_t MAL_GetStatus (uint8_t lun)
{
	if (lun == LUN_SDCARD)
	{
		if (SD_GetCardInfo(&SDCardInfo) != SD_OK )	return MAL_FAIL;

		/* Set Block Number & Size (BlockSize Fixed to 512Byte) */
		Mass_Block_Count[LUN_SDCARD] = SDCardInfo.CardCapacity / 512;
		Mass_Block_Size[LUN_SDCARD]  = 512;

		/* Set Total Memory Size */
		Mass_Memory_Size[LUN_SDCARD] = (uint64_t)Mass_Block_Count[LUN_SDCARD] * Mass_Block_Size[LUN_SDCARD];

		return MAL_OK;
	}
	return MAL_FAIL;
}

/* End Of File ---------------------------------------------------------------*/
