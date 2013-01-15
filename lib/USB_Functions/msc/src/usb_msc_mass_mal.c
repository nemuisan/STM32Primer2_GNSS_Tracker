/********************************************************************************/
/*!
	@file			usb_msc_mass_mal.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.30
	@brief          Descriptor Header for Mal.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "platform_config.h"
#include "sdio_stm32f1.h"
#include "usb_msc_mass_mal.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint32_t Mass_Memory_Size[2];
uint32_t Mass_Block_Size[2];
uint32_t Mass_Block_Count[2];

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
  uint16_t status = MAL_OK;

  switch (lun)
  {
    case 0:
      Status = SD_Init();
      break;
#ifdef USE_STM3210E_EVAL
    case 1:
      NAND_Init();
      break;
#endif
    default:
      return MAL_FAIL;
  }
  return status;
}

/**************************************************************************/
/*! 
    @brief  Write sectors.
*/
/**************************************************************************/
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{

  switch (lun)
  {
    case 0:
      Status = SD_WriteBlock((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      } 
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
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{

  switch (lun)
  {
    case 0:
	
      Status = SD_ReadBlock((uint8_t*)Readbuff, Memory_Offset, Transfer_Length);
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      }
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

  uint32_t DeviceSizeMul = 0, NumberOfBlocks = 0;
  uint64_t dwDevSize;  /* nemui */

  if (lun == 0)
  {
    if (SD_Init() == SD_OK)
    {
      SD_GetCardInfo(&SDCardInfo);
      SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
      DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);

      if(SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
      {
		/* nemui */
		dwDevSize  = (uint64_t)(SDCardInfo.SD_csd.DeviceSize + 1) * 512 * 1024;
		/* nemui  calculate highest LBA */
		Mass_Block_Count[0] = (dwDevSize - 1) / 512;
      }
      else
      {
        NumberOfBlocks  = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
        Mass_Block_Count[0] = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
      }
      Mass_Block_Size[0]  = 512;

      Status = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16)); 
      Status = SD_EnableWideBusOperation(SDIO_BusWide_4b); 
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      }
       
      Status = SD_SetDeviceMode(SD_DMA_MODE);         
      if ( Status != SD_OK )
      {
        return MAL_FAIL;
      } 
     
      Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];

      return MAL_OK;

    }
  }
  return MAL_FAIL;
}

/* End Of File ---------------------------------------------------------------*/
