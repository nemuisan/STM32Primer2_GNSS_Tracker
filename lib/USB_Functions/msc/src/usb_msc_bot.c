/********************************************************************************/
/*!
	@file			usb_msc_bot.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2025.04.08
	@brief          BOT State Machine management.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.23	V4.00	Fixed BOT data buffer to 4byte-alignment.
		2025.04.08	V5.00	Changed IN/OUT double-buffered bulk transfer.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_msc_scsi.h"
#include "usb_regs.h"
#include "usb_mem.h"
#include "usb_msc_conf.h"
#include "usb_msc_bot.h"
#include "usb_msc_memory.h"
#include "usb_lib.h"
#include "hw_config.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint8_t Bot_State;
uint8_t Bulk_Data_Buff[BULK_MAX_PACKET_SIZE]  __attribute__ ((aligned (4)));
uint16_t Data_Len;
__IO Bulk_Only_CBW CBW;
__IO Bulk_Only_CSW CSW;
__IO uint32_t SCSI_LBA , SCSI_BlkLen;
extern uint32_t Max_Lun;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**************************************************************************/
/**
  * Function Name  : USB_SIL_DBLWrite (STM32 -> HOST_PC)
  * Description    : Write a buffer of data to a selected endpoint.
  * Input          : - bEpAddr: The address of the non control endpoint.
  *                  - Data_Pointer: The pointer to the buffer of data to be written
  *                    to the endpoint.
  *                  - Data_Length: Number of data to be written (in bytes).
  * Output         : None.
  * Return         : Status 0.
  **/
/**************************************************************************/
uint16_t USB_SIL_DBLWrite(uint8_t bEpAddr, uint8_t* Data_Pointer, uint16_t Data_Length)
{
	/* Endpoint type is Bulk and Double Buffer enabled */
	if(!(GetENDPOINT(bEpAddr & 0x7F) & EP_DTOG_RX)) /* NOT TX ie SW_BUF */
	{
		SetEPDblBuf0Count((bEpAddr & 0x7F), EP_DBUF_IN, Data_Length);
		FreeUserBuffer((bEpAddr & 0x7F), EP_DBUF_IN); /* Toggles EP_DTOG_RX / SW_BUF soon */
		UserToPMABufferCopy(Data_Pointer, GetEPDblBuf0Addr(bEpAddr & 0x7F), Data_Length);
	}
	else
	{
		SetEPDblBuf1Count((bEpAddr & 0x7F), EP_DBUF_IN, Data_Length);
		FreeUserBuffer((bEpAddr & 0x7F), EP_DBUF_IN); /* Toggles EP_DTOG_RX / SW_BUF soon */
		UserToPMABufferCopy(Data_Pointer, GetEPDblBuf1Addr(bEpAddr & 0x7F), Data_Length);
	}
	
	return 0;
}

/**************************************************************************/
/**
  * Function Name  : USB_SIL_DBLRead (STM32 <- HOST_PC)
  * Description    : Read a buffer of data to a selected endpoint.
  * Input          : - bEpAddr: The address of the non control endpoint.
  *                  - Data_Pointer: The pointer to which will be saved the 
  *                     received data buffer.
  * Output         : None.
  * Return         : Number of received data (in Bytes).
  **/
/**************************************************************************/
uint16_t USB_SIL_DBLRead(uint8_t bEpAddr, uint8_t* Data_Pointer)
{
	/* Endpoint type is Bulk and Double Buffer enabled */
	uint16_t Data_Length = 0;
	
	if (GetENDPOINT(bEpAddr & 0x7F) & EP_DTOG_TX) /* NOT RX ie SW_BUF */
	{
		FreeUserBuffer((bEpAddr & 0x7F), EP_DBUF_OUT); /* Toggles EP_DTOG_TX / SW_BUF soon */
		Data_Length = GetEPDblBuf0Count(bEpAddr & 0x7F);
		PMAToUserBufferCopy(Data_Pointer, GetEPDblBuf0Addr(bEpAddr & 0x7F), Data_Length);
	}
	else
	{
		FreeUserBuffer((bEpAddr & 0x7F), EP_DBUF_OUT); /* Toggles EP_DTOG_TX / SW_BUF soon */
		Data_Length = GetEPDblBuf1Count((bEpAddr & 0x7F));
		PMAToUserBufferCopy(Data_Pointer, GetEPDblBuf1Addr(bEpAddr & 0x7F), Data_Length);
	}
	
	return Data_Length;
}

/**************************************************************************/
/*! 
    @brief  Mass Storage IN(STM32 -> PC) transfer callback.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void MSC_EP1_IN_Callback(void)
{
	uint8_t CMD;
	CMD = CBW.CB[0];

	/* BOT State Machine */
	switch (Bot_State)
	{
		case BOT_DATA_IN:
			switch (CMD)
			{
				case SCSI_READ10:
					SCSI_Read10_Cmd(CBW.bLUN , SCSI_LBA , SCSI_BlkLen);
					break;
			}
			break;
		case BOT_DATA_IN_LAST:
			Set_CSW (CSW_CMD_PASSED);
			break;
	    case BOT_ERROR:
			Bot_State = BOT_IDLE;
			SetEPRxStatus(ENDP2, EP_RX_VALID);
			break;
		case BOT_CSW_Send:
			Bot_State = BOT_IDLE;
			break;
		default:
			break;
	}
}

/**************************************************************************/
/*! 
    @brief  Mass Storage OUT(STM32 <- PC) transfer callback.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void MSC_EP2_OUT_Callback(void)
{
	uint8_t CMD;
	CMD = CBW.CB[0];

	Data_Len = USB_SIL_DBLRead(ENDP2, Bulk_Data_Buff);

	/* BOT State Machine */
	switch (Bot_State)
	{
		case BOT_IDLE:
			CBW_Decode();
			break;
		case BOT_DATA_OUT:
			if (CMD == SCSI_WRITE10)
			{
				SCSI_Write10_Cmd(CBW.bLUN , SCSI_LBA , SCSI_BlkLen);
				break;
			}
			Bot_Abort(DIR_OUT);
			Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
			Set_CSW (CSW_PHASE_ERROR);
			break;
		default:
			Bot_Abort(DIR_OUT);
			Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
			Set_CSW (CSW_CMD_FAILED);
			break;
	}
}

/**************************************************************************/
/*! 
    @brief  Decode the received CBW and call the related SCSI command
            routine.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void CBW_Decode(void)
{
  uint32_t Counter;

  for (Counter = 0; Counter < Data_Len; Counter++)
  {
    *((uint8_t *)&CBW + Counter) = Bulk_Data_Buff[Counter];
  }
  CSW.dTag = CBW.dTag;
  CSW.dDataResidue = CBW.dDataLength;
  if (Data_Len != BOT_CBW_PACKET_LENGTH)
  {
    Bot_Abort(BOTH_DIR);
    /* reset the CBW.dSignature to disable the clear feature until receiving a Mass storage reset */
    CBW.dSignature = 0;
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, PARAMETER_LIST_LENGTH_ERROR);
    Set_CSW (CSW_CMD_FAILED);
    return;
  }

  if ((CBW.CB[0] == SCSI_READ10 ) || (CBW.CB[0] == SCSI_WRITE10 ))
  {
    /* Calculate Logical Block Address */
    SCSI_LBA = (CBW.CB[2] << 24) | (CBW.CB[3] << 16) | (CBW.CB[4] <<  8) | CBW.CB[5];
    /* Calculate the Number of Blocks to transfer */
    SCSI_BlkLen = (CBW.CB[7] <<  8) | CBW.CB[8];
  }

  if (CBW.dSignature == BOT_CBW_SIGNATURE)
  {
    /* Valid CBW */
    if ((CBW.bLUN > Max_Lun) || (CBW.bCBLength < 1) || (CBW.bCBLength > 16))
    {
      Bot_Abort(BOTH_DIR);
      Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      Set_CSW (CSW_CMD_FAILED);
    }
    else
    {
      switch (CBW.CB[0])
      {
        case SCSI_REQUEST_SENSE:
          SCSI_RequestSense_Cmd (CBW.bLUN);
          break;
        case SCSI_INQUIRY:
          SCSI_Inquiry_Cmd(CBW.bLUN);
          break;
        case SCSI_START_STOP_UNIT:
          SCSI_Start_Stop_Unit_Cmd(CBW.bLUN);
          break;
        case SCSI_ALLOW_MEDIUM_REMOVAL:
          SCSI_Start_Stop_Unit_Cmd(CBW.bLUN);
          break;
        case SCSI_MODE_SENSE6:
          SCSI_ModeSense6_Cmd (CBW.bLUN);
          break;
        case SCSI_MODE_SENSE10:
          SCSI_ModeSense10_Cmd (CBW.bLUN);
          break;
        case SCSI_READ_FORMAT_CAPACITIES:
          SCSI_ReadFormatCapacity_Cmd(CBW.bLUN);
          break;
        case SCSI_READ_CAPACITY10:
          SCSI_ReadCapacity10_Cmd(CBW.bLUN);
          break;
        case SCSI_TEST_UNIT_READY:
          SCSI_TestUnitReady_Cmd(CBW.bLUN);
          break;
        case SCSI_READ10:
          SCSI_Read10_Cmd(CBW.bLUN, SCSI_LBA , SCSI_BlkLen);
          break;
        case SCSI_WRITE10:
          SCSI_Write10_Cmd(CBW.bLUN, SCSI_LBA , SCSI_BlkLen);
          break;
        case SCSI_VERIFY10:
          SCSI_Verify10_Cmd(CBW.bLUN);
          break;
        case SCSI_FORMAT_UNIT:
          SCSI_Format_Cmd(CBW.bLUN);
          break;
		  
          /* Unsupported command */
        case SCSI_MODE_SELECT10:
          SCSI_Mode_Select10_Cmd(CBW.bLUN);
          break;
        case SCSI_MODE_SELECT6:
          SCSI_Mode_Select6_Cmd(CBW.bLUN);
          break;

        case SCSI_SEND_DIAGNOSTIC:
          SCSI_Send_Diagnostic_Cmd(CBW.bLUN);
          break;
        case SCSI_READ6:
          SCSI_Read6_Cmd(CBW.bLUN);
          break;
        case SCSI_READ12:
          SCSI_Read12_Cmd(CBW.bLUN);
          break;
        case SCSI_READ16:
          SCSI_Read16_Cmd(CBW.bLUN);
          break;
        case SCSI_READ_CAPACITY16:
          SCSI_READ_CAPACITY16_Cmd(CBW.bLUN);
          break;
        case SCSI_WRITE6:
          SCSI_Write6_Cmd(CBW.bLUN);
          break;
        case SCSI_WRITE12:
          SCSI_Write12_Cmd(CBW.bLUN);
          break;
        case SCSI_WRITE16:
          SCSI_Write16_Cmd(CBW.bLUN);
          break;
        case SCSI_VERIFY12:
          SCSI_Verify12_Cmd(CBW.bLUN);
          break;
        case SCSI_VERIFY16:
          SCSI_Verify16_Cmd(CBW.bLUN);
          break;
        default:
        {
          Bot_Abort(BOTH_DIR);
          Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
          Set_CSW (CSW_CMD_FAILED);
		  break;
        }
      }
    }
  }
  else
  {
    /* Invalid CBW */
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
    Set_CSW (CSW_CMD_FAILED);
  }
}

/**************************************************************************/
/*! 
    @brief  Send the request response to the PC HOST.
	@param  uint8_t* Data_Address : point to the data to transfer.
	@param  uint16_t Data_Length  : the number of Bytes to transfer.
    @retval None.
*/
/**************************************************************************/
void Transfer_Data_Request(uint8_t* Data_Pointer, uint16_t Data_Length)
{
	USB_SIL_DBLWrite(ENDP1, Data_Pointer, Data_Length);

	Bot_State = BOT_DATA_IN_LAST;
	CSW.dDataResidue -= Data_Length;
	CSW.bStatus = CSW_CMD_PASSED;
}

/**************************************************************************/
/*! 
    @brief  Set the SCW with the needed fields.
	@param  uint8_t CSW_Status this filed can be CSW_CMD_PASSED,CSW_CMD_FAILED,
            or CSW_PHASE_ERROR.
    @retval None.
*/
/**************************************************************************/
void Set_CSW (uint8_t CSW_Status)
{
	CSW.dSignature = BOT_CSW_SIGNATURE;
	CSW.bStatus = CSW_Status;
	Bot_State = BOT_CSW_Send;
	
	if (CSW_Status != CSW_CMD_PASSED)
	{
		ClearDTOG_RX(ENDP1);
		ClearDTOG_TX(ENDP1); /* Clear the data toggle bits for the endpoint IN */
		Bot_State = BOT_ERROR;
	}
	
	USB_SIL_DBLWrite(ENDP1, ((uint8_t *)& CSW), CSW_DATA_LENGTH);
}

/**************************************************************************/
/*! 
    @brief  Stall the needed Endpoint according to the selected direction.
	@param  uint8_t Endpoint direction IN, OUT or both directions
    @retval None.
*/
/**************************************************************************/
void Bot_Abort(uint8_t Direction)
{
  switch (Direction)
  {
    case DIR_IN :
      SetDouBleBuffEPStall(ENDP1, EP_DBUF_IN);
      break;
    case DIR_OUT :
	  SetDouBleBuffEPStall(ENDP2, EP_DBUF_OUT);
	  FreeUserBuffer(ENDP2, EP_DBUF_OUT);
      break;
    case BOTH_DIR :
      SetDouBleBuffEPStall(ENDP1, EP_DBUF_IN);
	  SetDouBleBuffEPStall(ENDP2, EP_DBUF_OUT);
	  FreeUserBuffer(ENDP2, EP_DBUF_OUT);
      break;
    default:
      break;
  }
}

/* End Of File ---------------------------------------------------------------*/
