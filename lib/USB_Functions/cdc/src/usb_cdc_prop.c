/********************************************************************************/
/*!
	@file			usb_cdc_prop.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2014.01.23
	@brief          All processing related to Virtual COM Port.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.18	V2.00	Added SEND_BREAK into no data class specific requests.
		2014.01.23	V3.00	Removed retired STM32F10X_CL Codes.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_pwr.h"
#include "usb_cdc_conf.h"
#include "usb_cdc_prop.h"
#include "usb_cdc_desc.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint8_t Request = 0;

LINE_CODING linecoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* no. of bits 8*/
  };

/* Constants -----------------------------------------------------------------*/
ONE_DESCRIPTOR CDC_Device_Descriptor =
  {
    (uint8_t*)Virtual_Com_Port_DeviceDescriptor,
    VIRTUAL_COM_PORT_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR CDC_Config_Descriptor =
  {
    (uint8_t*)Virtual_Com_Port_ConfigDescriptor,
    VIRTUAL_COM_PORT_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR CDC_String_Descriptor[4] =
  {
    {(uint8_t*)Virtual_Com_Port_StringLangID, VIRTUAL_COM_PORT_SIZ_STRING_LANGID},
    {(uint8_t*)Virtual_Com_Port_StringVendor, VIRTUAL_COM_PORT_SIZ_STRING_VENDOR},
    {(uint8_t*)Virtual_Com_Port_StringProduct, VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT},
    {(uint8_t*)Virtual_Com_Port_StringSerial, VIRTUAL_COM_PORT_SIZ_STRING_SERIAL}
  };

/* Function prototypes -------------------------------------------------------*/
extern void Get_SerialNum(uint8_t* string0,uint8_t* string1);
extern bool USART_Config(void);
extern void USART_Config_Default(void);

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief	Install CDC Functions.
*/
/**************************************************************************/
void CDC_SetStructure(void)
{
	Device_Table.Total_Endpoint	= CDC_EP_NUM;
	xinit						= Virtual_Com_Port_init;
    xReset 						= Virtual_Com_Port_Reset;
    xStatus_In					= Virtual_Com_Port_Status_In;
    xStatus_Out					= Virtual_Com_Port_Status_Out;
    xData_Setup					= Virtual_Com_Port_Data_Setup;
    xNoData_Setup				= Virtual_Com_Port_NoData_Setup;
    xGet_Interface_Setting		= Virtual_Com_Port_Get_Interface_Setting;
    xGetDeviceDescriptor		= Virtual_Com_Port_GetDeviceDescriptor;
    xGetConfigDescriptor		= Virtual_Com_Port_GetConfigDescriptor;
    xGetStringDescriptor		= Virtual_Com_Port_GetStringDescriptor;
	
	xGetConfiguration			= Virtual_Com_Port_GetConfiguration;
    xSetConfiguration			= Virtual_Com_Port_SetConfiguration;
    xGetInterface				= Virtual_Com_Port_GetInterface;
    xSetInterface				= Virtual_Com_Port_SetInterface;
    xGetStatus					= Virtual_Com_Port_GetStatus;
    xClearFeature				= Virtual_Com_Port_ClearFeature;
    xSetEndPointFeature			= Virtual_Com_Port_SetEndPointFeature;
    xSetDeviceFeature			= Virtual_Com_Port_SetDeviceFeature;
    xSetDeviceAddress			= Virtual_Com_Port_SetDeviceAddress;
}

/**************************************************************************/
/*! 
    @brief	Virtual COM Port Mouse init routine.
*/
/**************************************************************************/
void Virtual_Com_Port_init(void)
{

  /* Update the serial number string descriptor with the data from the unique
  ID*/
  Get_SerialNum(&Virtual_Com_Port_StringSerial[2],&Virtual_Com_Port_StringSerial[18]);

  pInformation->Current_Configuration = 0;

  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  /* configure the USART to the default settings */
  USART_Config_Default();

  bDeviceState = UNCONNECTED;
}

/**************************************************************************/
/*! 
    @brief	Virtual_Com_Port Mouse reset routine.
*/
/**************************************************************************/
void Virtual_Com_Port_Reset(void)
{
  /* Set Virtual_Com_Port DEVICE as not configured */
  pInformation->Current_Configuration = 0;

  /* Current Feature initialization */
  pInformation->Current_Feature = Virtual_Com_Port_ConfigDescriptor[7];

  /* Set Virtual_Com_Port DEVICE with the default Interface*/
  pInformation->Current_Interface = 0;

  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, CDC_ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, CDC_ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_BULK);
  SetEPTxAddr(ENDP1, CDC_ENDP1_TXADDR);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_DIS);

  /* Initialize Endpoint 2 */
  SetEPType(ENDP2, EP_INTERRUPT);
  SetEPTxAddr(ENDP2, CDC_ENDP2_TXADDR);
  SetEPRxStatus(ENDP2, EP_RX_DIS);
  SetEPTxStatus(ENDP2, EP_TX_NAK);

  /* Initialize Endpoint 3 */
 /* nemui
  SetEPType(ENDP3, EP_BULK);
  SetEPRxAddr(ENDP3, CDC_ENDP3_RXADDR);
  SetEPRxCount(ENDP3, VIRTUAL_COM_PORT_DATA_SIZE);
  SetEPRxStatus(ENDP3, EP_RX_VALID);
  SetEPTxStatus(ENDP3, EP_TX_DIS);
*/
  SetEPType(ENDP3, EP_BULK);
  SetEPDoubleBuff(ENDP3);
  SetEPDblBuffAddr(ENDP3, CDC_ENDP3_BUF0Addr, CDC_ENDP3_BUF1Addr);
  SetEPDblBuffCount(ENDP3, EP_DBUF_OUT, VIRTUAL_COM_PORT_DATA_SIZE);
  ClearDTOG_RX(ENDP3);
  ClearDTOG_TX(ENDP3);
  ToggleDTOG_TX(ENDP3);
  SetEPRxStatus(ENDP3, EP_RX_VALID);
  SetEPTxStatus(ENDP3, EP_TX_DIS); 

  /* Set this device to response on default address */
  SetDeviceAddress(0);

  bDeviceState = ATTACHED;
}

/**************************************************************************/
/*! 
    @brief	Update the device state to configured.
*/
/**************************************************************************/
void Virtual_Com_Port_SetConfiguration(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
}

/**************************************************************************/
/*! 
    @brief	Update the device state to addressed.
*/
/**************************************************************************/
void Virtual_Com_Port_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}

/**************************************************************************/
/*! 
    @brief	Virtual COM Port Status In Routine.
*/
/**************************************************************************/
void Virtual_Com_Port_Status_In(void)
{
  if (Request == SET_LINE_CODING)
  {
    USART_Config();
    Request = 0;
  }
}

/**************************************************************************/
/*! 
    @brief	Virtual COM Port Status OUT Routine.
*/
/**************************************************************************/
void Virtual_Com_Port_Status_Out(void)
{}

/**************************************************************************/
/*! 
    @brief	handle the data class specific requests.
*/
/**************************************************************************/
RESULT Virtual_Com_Port_Data_Setup(uint8_t RequestNo)
{
  uint8_t    *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;

  if (RequestNo == GET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
      CopyRoutine = Virtual_Com_Port_GetLineCoding;
    }
  }
  else if (RequestNo == SET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
    {
      CopyRoutine = Virtual_Com_Port_SetLineCoding;
    }
    Request = SET_LINE_CODING;
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
    @brief	handle the no data class specific requests.
*/
/**************************************************************************/
RESULT Virtual_Com_Port_NoData_Setup(uint8_t RequestNo)
{

  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
  {
    if (RequestNo == SET_COMM_FEATURE)
    {
      return USB_SUCCESS;
    }
    else if (RequestNo == SET_CONTROL_LINE_STATE)
    {
      return USB_SUCCESS;
    }
    else if (RequestNo == SEND_BREAK)
    {
      return USB_SUCCESS;
    }
  }

  return USB_UNSUPPORT;
}

/**************************************************************************/
/*! 
    @brief	Gets the device descriptor.
*/
/**************************************************************************/
uint8_t *Virtual_Com_Port_GetDeviceDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &CDC_Device_Descriptor);
}

/**************************************************************************/
/*! 
    @brief	Get the configuration descriptor.
*/
/**************************************************************************/
uint8_t *Virtual_Com_Port_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &CDC_Config_Descriptor);
}

/**************************************************************************/
/*! 
    @brief	Gets the string descriptors according to the needed index.
*/
/**************************************************************************/
uint8_t *Virtual_Com_Port_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  if (wValue0 > 4)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &CDC_String_Descriptor[wValue0]);
  }
}

/**************************************************************************/
/*! 
    @brief	test the interface and the alternate setting according to the
			supported one.
*/
/**************************************************************************/
RESULT Virtual_Com_Port_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  else if (Interface > 1)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}

/**************************************************************************/
/*! 
    @brief	Send the linecoding structure to the PC host.
*/
/**************************************************************************/
uint8_t *Virtual_Com_Port_GetLineCoding(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return(uint8_t *)&linecoding;
}

/**************************************************************************/
/*! 
    @brief	Set the linecoding structure fields.
*/
/**************************************************************************/
uint8_t *Virtual_Com_Port_SetLineCoding(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return(uint8_t *)&linecoding;
}

/* End Of File ---------------------------------------------------------------*/
