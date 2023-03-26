/********************************************************************************/
/*!
	@file			usb_msc_bot.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.03.23
	@brief          BOT State Machine management.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.23	V4.00	Fixed BOT data buffer to 4byte-alignment.

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
__IO long StableCount = BOT_STABLE_COUNT;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  Mass Storage IN transfer callback.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void MSC_EP1_IN_Callback(void)
{
	switch (Bot_State)
	{
		case BOT_CSW_Send:
		case BOT_ERROR:
			if(StableCount > 0){
				StableCount--; 		/* About1~2sec */
				LED_RED_ON();
				_delay_us(15000);	/* why does this delay need??? anyway that stable it */
				LED_RED_OFF();
			}
			Bot_State = BOT_IDLE;
			break;
		case BOT_DATA_IN:
			switch (CBW.CB[0])
			{
				case SCSI_READ10:
					SCSI_Read10_Cmd(CBW.bLUN , SCSI_LBA , SCSI_BlkLen);
					break;
			}
			break;
		case BOT_DATA_IN_LAST:
			Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
			SetEPRxStatus(ENDP2, EP_RX_VALID);
			break;
		default:
			break;
	}
}

/**************************************************************************/
/*! 
    @brief  Mass Storage OUT transfer callback.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void MSC_EP2_OUT_Callback(void)
{
	uint8_t CMD;
	CMD = CBW.CB[0];

	/* Double Buffered RX */
	if (GetENDPOINT(ENDP2) & EP_DTOG_TX)
	{ 
		Data_Len = GetEPDblBuf0Count(ENDP2);
		if(Data_Len == 0) return; /* Detect zero length packet */
		FreeUserBuffer(ENDP2, EP_DBUF_OUT);	/* Toggles EP_DTOG_TX / SW_BUF soon */
		PMAToUserBufferCopy(Bulk_Data_Buff, MSC_ENDP2_BUF0Addr, Data_Len); 
	} 
	else
	{ 
		Data_Len= GetEPDblBuf1Count(ENDP2);
		if(Data_Len == 0) return; /* Detect zero length packet */
		FreeUserBuffer(ENDP2, EP_DBUF_OUT);	/* Toggles EP_DTOG_TX / SW_BUF soon */
		PMAToUserBufferCopy(Bulk_Data_Buff, MSC_ENDP2_BUF1Addr, Data_Len); 
	}


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
			Set_CSW (CSW_PHASE_ERROR, SEND_CSW_DISABLE);
			break;
		default:
			Bot_Abort(BOTH_DIR);
			Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
			Set_CSW (CSW_PHASE_ERROR, SEND_CSW_DISABLE);
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
    /* reset the CBW.dSignature to disable the clear feature until receiving a Mass storage reset*/
    CBW.dSignature = 0;
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, PARAMETER_LIST_LENGTH_ERROR);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
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
      Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
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
          /*Unsupported command*/

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
          Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
        }
      }
    }
  }
  else
  {
    /* Invalid CBW */
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(CBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
    Set_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
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
	USB_SIL_Write(EP1_IN, Data_Pointer, Data_Length);

	SetEPTxStatus(ENDP1, EP_TX_VALID);
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
void Set_CSW (uint8_t CSW_Status, uint8_t Send_Permission)
{
	CSW.dSignature = BOT_CSW_SIGNATURE;
	CSW.bStatus = CSW_Status;

	USB_SIL_Write(EP1_IN, ((uint8_t *)& CSW), CSW_DATA_LENGTH);

	Bot_State = BOT_ERROR;
	if (Send_Permission)
	{
		Bot_State = BOT_CSW_Send;
		SetEPTxStatus(ENDP1, EP_TX_VALID);
	}
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
      SetEPTxStatus(ENDP1, EP_TX_STALL);
      break;
    case DIR_OUT :
	  SetDouBleBuffEPStall(ENDP2, EP_DBUF_OUT);
	  FreeUserBuffer(ENDP2, EP_DBUF_OUT);
      break;
    case BOTH_DIR :
      SetEPTxStatus(ENDP1, EP_TX_STALL);
	  SetDouBleBuffEPStall(ENDP2, EP_DBUF_OUT);
	  FreeUserBuffer(ENDP2, EP_DBUF_OUT);
      break;
    default:
      break;
  }
}

/* End Of File ---------------------------------------------------------------*/
