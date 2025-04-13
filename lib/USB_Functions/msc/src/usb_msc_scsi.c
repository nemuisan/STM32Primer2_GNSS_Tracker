/********************************************************************************/
/*!
	@file			usb_msc_scsi.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2024.07.12
	@brief          All processing related to the SCSI commands.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2016.01.15	V3.00	Changed definition compatibility.
		2017.03.29	V4.00	Removed retired variables.
		2023.03.08	V5.00	Removed redundant declaration.
		2024.07.12	V6.00	Fixed unused parameter.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
#include "usb_msc_scsi.h"
#include "usb_msc_mass_mal.h"
#include "usb_msc_bot.h"
#include "usb_regs.h"
#include "usb_msc_memory.h"
#include "platform_config.h"
#include "usb_lib.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  SCSI Inquiry Command routine.
*/
/**************************************************************************/
void SCSI_Inquiry_Cmd(uint8_t lun)
{
  uint8_t* Inquiry_Data;
  uint16_t Inquiry_Data_Length;

  if (CBW.CB[1] & 0x01)/*Evpd is set*/
  {
    Inquiry_Data = Page00_Inquiry_Data;
    Inquiry_Data_Length = 5;
  }
  else
  {

    if (lun == LUN_SDCARD)
    {
      Inquiry_Data = Standard_Inquiry_Data;
    }
    else
    {	/* Invalid state! Nemuisan said */
      Inquiry_Data = Standard_Inquiry_Data;
    }

    if (CBW.CB[4] <= STANDARD_INQUIRY_DATA_LEN)
      Inquiry_Data_Length = CBW.CB[4];
    else
      Inquiry_Data_Length = STANDARD_INQUIRY_DATA_LEN;

  }
  Transfer_Data_Request(Inquiry_Data, Inquiry_Data_Length);
}

/**************************************************************************/
/*! 
    @brief  SCSI ReadFormatCapacity Command routine.
*/
/**************************************************************************/
void SCSI_ReadFormatCapacity_Cmd(uint8_t lun)
{

  if (MAL_GetStatus(lun) != 0 )
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED);
    Bot_Abort(DIR_IN);
    return;
  }
  ReadFormatCapacity_Data[4] = (uint8_t)(Mass_Block_Count[lun] >> 24);
  ReadFormatCapacity_Data[5] = (uint8_t)(Mass_Block_Count[lun] >> 16);
  ReadFormatCapacity_Data[6] = (uint8_t)(Mass_Block_Count[lun] >>  8);
  ReadFormatCapacity_Data[7] = (uint8_t)(Mass_Block_Count[lun]);

  ReadFormatCapacity_Data[9] = (uint8_t)(Mass_Block_Size[lun] >>  16);
  ReadFormatCapacity_Data[10] = (uint8_t)(Mass_Block_Size[lun] >>  8);
  ReadFormatCapacity_Data[11] = (uint8_t)(Mass_Block_Size[lun]);
  Transfer_Data_Request(ReadFormatCapacity_Data, READ_FORMAT_CAPACITY_DATA_LEN);
}

/**************************************************************************/
/*! 
    @brief  SCSI ReadCapacity10 Command routine.
*/
/**************************************************************************/
void SCSI_ReadCapacity10_Cmd(uint8_t lun)
{

  if (MAL_GetStatus(lun))
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED);
    Bot_Abort(DIR_IN);
    return;
  }

  ReadCapacity10_Data[0] = (uint8_t)((Mass_Block_Count[lun] - 1) >> 24);
  ReadCapacity10_Data[1] = (uint8_t)((Mass_Block_Count[lun] - 1) >> 16);
  ReadCapacity10_Data[2] = (uint8_t)((Mass_Block_Count[lun] - 1) >>  8);
  ReadCapacity10_Data[3] = (uint8_t)(Mass_Block_Count[lun] - 1);

  ReadCapacity10_Data[4] = (uint8_t)(Mass_Block_Size[lun] >>  24);
  ReadCapacity10_Data[5] = (uint8_t)(Mass_Block_Size[lun] >>  16);
  ReadCapacity10_Data[6] = (uint8_t)(Mass_Block_Size[lun] >>  8);
  ReadCapacity10_Data[7] = (uint8_t)(Mass_Block_Size[lun]);
  Transfer_Data_Request(ReadCapacity10_Data, READ_CAPACITY10_DATA_LEN);
}

/**************************************************************************/
/*! 
    @brief  SCSI ModeSense6 Command routine.
*/
/**************************************************************************/
void SCSI_ModeSense6_Cmd (uint8_t lun)
{
  (void)lun;
  Transfer_Data_Request(Mode_Sense6_data, MODE_SENSE6_DATA_LEN);
}

/**************************************************************************/
/*! 
    @brief  SCSI ModeSense10 Command routine.
*/
/**************************************************************************/
void SCSI_ModeSense10_Cmd (uint8_t lun)
{
  (void)lun;
  Transfer_Data_Request(Mode_Sense10_data, MODE_SENSE10_DATA_LEN);
}

/**************************************************************************/
/*! 
    @brief  SCSI RequestSense Command routine.
*/
/**************************************************************************/
void SCSI_RequestSense_Cmd (uint8_t lun)
{
  (void)lun;
  uint8_t Request_Sense_data_Length;

  if (CBW.CB[4] <= REQUEST_SENSE_DATA_LEN)
  {
    Request_Sense_data_Length = CBW.CB[4];
  }
  else
  {
    Request_Sense_data_Length = REQUEST_SENSE_DATA_LEN;
  }
  Transfer_Data_Request(Scsi_Sense_Data, Request_Sense_data_Length);
}

/**************************************************************************/
/*! 
    @brief  Set Scsi Sense Data routine..
*/
/**************************************************************************/
void Set_Scsi_Sense_Data(uint8_t lun, uint8_t Sens_Key, uint8_t Asc)
{
  (void)lun;
  Scsi_Sense_Data[2] = Sens_Key;
  Scsi_Sense_Data[12] = Asc;
}

/**************************************************************************/
/*! 
    @brief  SCSI Start_Stop_Unit Command routine.
*/
/**************************************************************************/
void SCSI_Start_Stop_Unit_Cmd(uint8_t lun)
{
  (void)lun;
  Set_CSW (CSW_CMD_PASSED);
}

/**************************************************************************/
/*! 
    @brief  SCSI Read10 Command routine.
*/
/**************************************************************************/
void SCSI_Read10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr)
{
  (void)lun;
  if (Bot_State == BOT_IDLE)
  {
    if (!(SCSI_Address_Management(CBW.bLUN, SCSI_READ10, LBA, BlockNbr)))/*address out of range*/
    {
      return;
    }

    if ((CBW.bmFlags & 0x80) != 0)
    {
      Bot_State = BOT_DATA_IN;
      Read_Memory(lun, LBA , BlockNbr);
    }
    else
    {
      Bot_Abort(BOTH_DIR);
      Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      Set_CSW (CSW_CMD_FAILED);
    }
    return;
  }
  else if (Bot_State == BOT_DATA_IN)
  {
    Read_Memory(lun , LBA , BlockNbr);
  }
}

/**************************************************************************/
/*! 
    @brief  SCSI Write10 Command routine.
*/
/**************************************************************************/
void SCSI_Write10_Cmd(uint8_t lun , uint32_t LBA , uint32_t BlockNbr)
{

  if (Bot_State == BOT_IDLE)
  {
    if (!(SCSI_Address_Management(CBW.bLUN, SCSI_WRITE10 , LBA, BlockNbr)))/*address out of range*/
    {
      return;
    }

    if ((CBW.bmFlags & 0x80) == 0)
    {
      Bot_State = BOT_DATA_OUT;
      SetEPRxStatus(ENDP2, EP_RX_VALID);
    }
    else
    {
      Bot_Abort(DIR_IN);
      Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      Set_CSW (CSW_CMD_FAILED);
    }
    return;
  }
  else if (Bot_State == BOT_DATA_OUT)
  {
    Write_Memory(lun , LBA , BlockNbr);
  }
}

/**************************************************************************/
/*! 
    @brief  SCSI Verify10 Command routine.
*/
/**************************************************************************/
void SCSI_Verify10_Cmd(uint8_t lun)
{
  (void)lun;
  if ((CBW.dDataLength == 0) && !(CBW.CB[1] & BLKVFY))/* BLKVFY not set*/
  {
    Set_CSW (CSW_CMD_PASSED);
  }
  else
  {
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
    Set_CSW (CSW_CMD_FAILED);
  }
}

/**************************************************************************/
/*! 
    @brief  Valid Commands routine.
*/
/**************************************************************************/
void SCSI_Valid_Cmd(uint8_t lun)
{
  (void)lun;
  if (CBW.dDataLength != 0)
  {
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
    Set_CSW (CSW_CMD_FAILED);
  }
  else
    Set_CSW (CSW_CMD_PASSED);
}

/**************************************************************************/
/*! 
    @brief  Valid Commands routine.
*/
/**************************************************************************/
void SCSI_TestUnitReady_Cmd(uint8_t lun)
{
  if (MAL_GetStatus(lun))
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED);
    Bot_Abort(DIR_IN);
    return;
  }
  else
  {
    Set_CSW (CSW_CMD_PASSED);
  }
}

/**************************************************************************/
/*! 
    @brief  Format Commands routine.
*/
/**************************************************************************/
void SCSI_Format_Cmd(uint8_t lun)
{
  if (MAL_GetStatus(lun))
  {
    Set_Scsi_Sense_Data(CBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_CSW (CSW_CMD_FAILED);
    Bot_Abort(DIR_IN);
    return;
  }

}

/**************************************************************************/
/*! 
    @brief  Invalid Commands routine.
*/
/**************************************************************************/
void SCSI_Invalid_Cmd(uint8_t lun)
{
  (void)lun;
  if (CBW.dDataLength == 0)
  {
    Bot_Abort(DIR_IN);
  }
  else
  {
    if ((CBW.bmFlags & 0x80) != 0)
    {
      Bot_Abort(DIR_IN);
    }
    else
    {
      Bot_Abort(BOTH_DIR);
    }
  }
  Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
  Set_CSW (CSW_CMD_FAILED);
}

/**************************************************************************/
/*! 
    @brief  Test the received address.
*/
/**************************************************************************/
bool SCSI_Address_Management(uint8_t lun , uint8_t Cmd , uint32_t LBA , uint32_t BlockNbr)
{

  if ((LBA + BlockNbr) > Mass_Block_Count[lun] )
  {
    if (Cmd == SCSI_WRITE10)
    {
      Bot_Abort(BOTH_DIR);
    }
    Bot_Abort(DIR_IN);
    Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, ADDRESS_OUT_OF_RANGE);
    Set_CSW (CSW_CMD_FAILED);
    return (FALSE);
  }


  if (CBW.dDataLength != BlockNbr * Mass_Block_Size[lun])
  {
    if (Cmd == SCSI_WRITE10)
    {
      Bot_Abort(BOTH_DIR);
    }
    else
    {
      Bot_Abort(DIR_IN);
    }
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
    Set_CSW (CSW_CMD_FAILED);
    return (FALSE);
  }
  return (TRUE);
}

/* End Of File ---------------------------------------------------------------*/
