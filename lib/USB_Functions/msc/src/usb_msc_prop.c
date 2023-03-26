/********************************************************************************/
/*!
	@file			usb_msc_prop.c
    @version        4.00
    @date           2023.03.20
	@brief          Mass Storage middle layer.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.20	V4.00	Removed redundant declaration.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_msc_desc.h"
#include "usb_msc_bot.h"
#include "usb_msc_conf.h"
#include "usb_msc_memory.h"
#include "usb_msc_mass_mal.h"
#include "usb_msc_prop.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint32_t Max_Lun = 0;
extern __IO long StableCount;

/* Constants -----------------------------------------------------------------*/
ONE_DESCRIPTOR MSC_Device_Descriptor __attribute__ ((aligned (4))) =
  {
    (uint8_t*)Mass_DeviceDescriptor,
    MASS_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR MSC_Config_Descriptor __attribute__ ((aligned (4))) =
  {
    (uint8_t*)Mass_ConfigDescriptor,
    MASS_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR MSC_String_Descriptor[5] __attribute__ ((aligned (4))) =
  {
    {(uint8_t*)Mass_StringLangID, MASS_SIZ_STRING_LANGID},
    {(uint8_t*)Mass_StringVendor, MASS_SIZ_STRING_VENDOR},
    {(uint8_t*)Mass_StringProduct, MASS_SIZ_STRING_PRODUCT},
    {(uint8_t*)Mass_StringSerial, MASS_SIZ_STRING_SERIAL},
    {(uint8_t*)Mass_StringInterface, MASS_SIZ_STRING_INTERFACE},
  };

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief	Install MSC Functions.
*/
/**************************************************************************/
void MSC_SetStructure(void)
{
	Device_Table.Total_Endpoint	= MSC_EP_NUM;
	xinit						= Mass_init;
    xReset 						= Mass_Reset;
    xStatus_In					= Mass_Status_In;
    xStatus_Out					= Mass_Status_Out;
    xData_Setup					= Mass_Data_Setup;
    xNoData_Setup				= Mass_NoData_Setup;
    xGet_Interface_Setting		= Mass_Get_Interface_Setting;
    xGetDeviceDescriptor		= Mass_GetDeviceDescriptor;
    xGetConfigDescriptor		= Mass_GetConfigDescriptor;
    xGetStringDescriptor		= Mass_GetStringDescriptor;
	
	xGetConfiguration			= Mass_GetConfiguration;
    xSetConfiguration			= Mass_SetConfiguration;
    xGetInterface				= Mass_GetInterface;
    xSetInterface				= Mass_SetInterface;
    xGetStatus					= Mass_GetStatus;
    xClearFeature				= Mass_ClearFeature;
    xSetEndPointFeature			= Mass_SetEndPointFeature;
    xSetDeviceFeature			= Mass_SetDeviceFeature;
    xSetDeviceAddress           = Mass_SetDeviceAddress;
}

/**************************************************************************/
/*! 
    @brief	Mass Storage init routine.
*/
/**************************************************************************/
void Mass_init()
{

  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum(&Mass_StringSerial[2],&Mass_StringSerial[18]);


  pInformation->Current_Configuration = 0;

  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  bDeviceState = UNCONNECTED;

}

/**************************************************************************/
/*! 
    @brief	Mass Storage reset routine.
*/
/**************************************************************************/
void Mass_Reset()
{
	/* Set the device as not configured */
	Device_Info.Current_Configuration = 0;

	/* Current Feature initialization */
	pInformation->Current_Feature = Mass_ConfigDescriptor[7];

	SetBTABLE(BTABLE_ADDRESS);

	/* Initialize Endpoint 0 */
	SetEPType(ENDP0, EP_CONTROL);
	SetEPTxStatus(ENDP0, EP_TX_NAK);
	SetEPRxAddr(ENDP0, MSC_ENDP0_RXADDR);
	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPTxAddr(ENDP0, MSC_ENDP0_TXADDR);
	Clear_Status_Out(ENDP0);
	SetEPRxValid(ENDP0);

	/* Initialize Endpoint 1 IN as TX (Device->HOST_PC) */
	SetEPType(ENDP1, EP_BULK);
	SetEPTxAddr(ENDP1, MSC_ENDP1_TXADDR);
	SetEPTxStatus(ENDP1, EP_TX_NAK); /* STM32 -> HOST Enable */
	SetEPRxStatus(ENDP1, EP_RX_DIS); /* STM32 -> HOST Disable */

	/* Initialize Endpoint 2 OUT as RX (HOST_PC->Device) */
	/* Nemui Changed to Double Buffer */
	SetEPType(ENDP2, EP_BULK);
	SetEPDoubleBuff(ENDP2);
	SetEPDblBuffAddr(ENDP2, MSC_ENDP2_BUF0Addr, MSC_ENDP2_BUF1Addr);
	SetEPDblBuffCount(ENDP2, EP_DBUF_OUT, Device_Property.MaxPacketSize); /* Full Speed MAX 64Bytes */
	ClearDTOG_RX(ENDP2);    /* Clear DTOG USB PERIPHERAL */
	ClearDTOG_TX(ENDP2);    /* Clear SW_BUF for APPLICATION */
    ToggleDTOG_TX(ENDP2);	/* NOT RX ie Toggle SW_BUF - Sets buf 1 as software buffer (buf 0 for first rx) */
	SetEPRxStatus(ENDP2, EP_RX_VALID);	/* HOST -> STM32 Enable */
	SetEPTxStatus(ENDP2, EP_TX_DIS);	/* HOST -> STM32 Disable */

	/* Set the device to response on default address */
	SetDeviceAddress(0);

	bDeviceState = ATTACHED;
	StableCount = BOT_STABLE_COUNT;

	CBW.dSignature = BOT_CBW_SIGNATURE;
	Bot_State = BOT_IDLE;
}

/**************************************************************************/
/*! 
    @brief	Handle the SetConfiguration request.
*/
/**************************************************************************/
void Mass_SetConfiguration(void)
{
  if (pInformation->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;

    /* Initialize Endpoint 1 */
    ClearDTOG_TX(ENDP1);

    /* Initialize Endpoint 2 */
    ClearDTOG_RX(ENDP2);

    Bot_State = BOT_IDLE; /* set the Bot state machine to the IDLE state */
  }
}

/**************************************************************************/
/*! 
    @brief	Handle the ClearFeature request.
*/
/**************************************************************************/
void Mass_ClearFeature(void)
{
  /* when the host send a CBW with invalid signature or invalid length the two
     Endpoints (IN & OUT) shall stall until receiving a Mass Storage Reset     */
  if (CBW.dSignature != BOT_CBW_SIGNATURE)
    Bot_Abort(BOTH_DIR);
}

/**************************************************************************/
/*! 
    @brief	Update the device state to addressed.
*/
/**************************************************************************/
void Mass_SetDeviceAddress (void)
{
	bDeviceState = ADDRESSED;
}

/**************************************************************************/
/*! 
    @brief	Mass Storage Status IN routine.
*/
/**************************************************************************/
void Mass_Status_In(void)
{
  return;
}

/**************************************************************************/
/*! 
    @brief	Mass Storage Status OUT routine.
*/
/**************************************************************************/
void Mass_Status_Out(void)
{
  return;
}

/**************************************************************************/
/*! 
    @brief	Handle the data class specific requests.
*/
/**************************************************************************/
RESULT Mass_Data_Setup(uint8_t RequestNo)
{
  uint8_t    *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == GET_MAX_LUN) && (pInformation->USBwValue == 0)
      && (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x01))
  {
    CopyRoutine = Get_Max_Lun;
  }
  else
  {
    return USB_UNSUPPORT;
  }

  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);

  return USB_SUCCESS;

}

/**************************************************************************/
/*! 
    @brief	Handle the no data class specific requests.
*/
/**************************************************************************/
RESULT Mass_NoData_Setup(uint8_t RequestNo)
{
  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      && (RequestNo == MASS_STORAGE_RESET) && (pInformation->USBwValue == 0)
      && (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00))
  {
    /* Initialize Endpoint 1 */
    ClearDTOG_TX(ENDP1);

    /* Initialize Endpoint 2 */
    ClearDTOG_RX(ENDP2);
	ToggleDTOG_TX(ENDP2); /* reset value of the data toggle bits for the endpoint out*/
	
    /*initialize the CBW signature to enable the clear feature*/
    CBW.dSignature = BOT_CBW_SIGNATURE;
    Bot_State = BOT_IDLE;

    return USB_SUCCESS;
  }
  return USB_UNSUPPORT;
}

/**************************************************************************/
/*! 
    @brief	Test the interface and the alternate setting according to the
			supported one.
*/
/**************************************************************************/
RESULT Mass_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;/* in this application we don't have AlternateSetting*/
  }
  else if (Interface > 0)
  {
    return USB_UNSUPPORT;/*in this application we have only 1 interfaces*/
  }
  return USB_SUCCESS;
}

/**************************************************************************/
/*! 
    @brief	Get the device descriptor.
*/
/**************************************************************************/
uint8_t *Mass_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &MSC_Device_Descriptor );
}

/**************************************************************************/
/*! 
    @brief	Get the configuration descriptor.
*/
/**************************************************************************/
uint8_t *Mass_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &MSC_Config_Descriptor );
}

/**************************************************************************/
/*! 
    @brief	Get the string descriptors according to the needed index.
*/
/**************************************************************************/
uint8_t *Mass_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;

  if (wValue0 > 5)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &MSC_String_Descriptor[wValue0]);
  }
}

/**************************************************************************/
/*! 
    @brief	Handle the Get Max Lun request.
*/
/**************************************************************************/
uint8_t *Get_Max_Lun(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = LUN_DATA_LENGTH;
    return 0;
  }
  else
  {
    return((uint8_t*)(&Max_Lun));
  }
}

/* End Of File ---------------------------------------------------------------*/
