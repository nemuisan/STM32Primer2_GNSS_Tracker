/********************************************************************************/
/*!
	@file			usb_prop.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2024.07.18
	@brief          USB Endpoint Wrappers.

    @section HISTORY
		2012.01.30	V1.00	Start Here
		2014.12.17	V2.00	Adopted GCC4.9.x.
		2023.03.07	V3.00	Fixed wrong functional connection at xSetDeviceAddress.
		2024.07.18	V4.00	Fixed empty argument.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_prop.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
void (* volatile xinit)(void);
void (* volatile xReset)(void);
void (* volatile xStatus_In)(void);
void (* volatile xStatus_Out)(void);
RESULT (* volatile xData_Setup)(uint8_t RequestNo);
RESULT (* volatile xNoData_Setup)(uint8_t RequestNo);
RESULT (* volatile xGet_Interface_Setting)(uint8_t Interface, uint8_t AlternateSetting);
uint8_t* (* volatile xGetDeviceDescriptor)(uint16_t Length);
uint8_t* (* volatile xGetConfigDescriptor)(uint16_t Length);
uint8_t* (* volatile xGetStringDescriptor)(uint16_t Length);

void (* volatile xGetConfiguration)(void);
void (* volatile xSetConfiguration)(void);
void (* volatile xGetInterface)(void);
void (* volatile xSetInterface)(void);
void (* volatile xGetStatus)(void);
void (* volatile xClearFeature)(void);
void (* volatile xSetEndPointFeature)(void);
void (* volatile xSetDeviceFeature)(void);
void (* volatile xSetDeviceAddress)(void);

/* Constants -----------------------------------------------------------------*/
DEVICE Device_Table =
  {
    3,
    1
  };

DEVICE_PROP Device_Property =
  {
  	nxinit,
  	nxReset,
  	nxStatus_In,
  	nxStatus_Out,
  	nxData_Setup,
  	nxNoData_Setup,
  	nxGet_Interface_Setting,
  	nxGetDeviceDescriptor,
  	nxGetConfigDescriptor,
  	nxGetStringDescriptor,
    0,
    0x40 /* MAX PACKET SIZE 64Bytes */
  };

USER_STANDARD_REQUESTS User_Standard_Requests =
  {
  	nxGetConfiguration,
  	nxSetConfiguration,
  	nxGetInterface,
  	nxSetInterface,
  	nxGetStatus,
  	nxClearFeature,
  	nxSetEndPointFeature,
  	nxSetDeviceFeature,
  	nxSetDeviceAddress
  };

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    init wrapper
*/
/**************************************************************************/
void nxinit(void)
{
	xinit();
}

/**************************************************************************/
/*! 
    Reset wrapper
*/
/**************************************************************************/
void nxReset(void)
{
	xReset();
}


/**************************************************************************/
/*! 
    Status_In wrapper
*/
/**************************************************************************/
void nxStatus_In(void)
{
	xStatus_In();
}


/**************************************************************************/
/*! 
    Status_Out wrapper
*/
/**************************************************************************/
void nxStatus_Out(void)
{
	xStatus_Out();
}


/**************************************************************************/
/*! 
    Data_Setup Wrapper
*/
/**************************************************************************/
RESULT nxData_Setup(uint8_t RequestNo)
{
	return xData_Setup(RequestNo);
}

/**************************************************************************/
/*! 
    NoData_Setup wrapper
*/
/**************************************************************************/
RESULT nxNoData_Setup(uint8_t RequestNo)
{
	return xNoData_Setup(RequestNo);
}

/**************************************************************************/
/*! 
    Get_Interface_Setting wrapper
*/
/**************************************************************************/
RESULT nxGet_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
	return xGet_Interface_Setting(Interface,AlternateSetting);
}

/**************************************************************************/
/*! 
    GetDeviceDescriptor wrapper
*/
/**************************************************************************/
uint8_t *nxGetDeviceDescriptor(uint16_t Length)
{
	return xGetDeviceDescriptor(Length);
}

/**************************************************************************/
/*! 
    GetConfigDescriptor wrapper
*/
/**************************************************************************/
uint8_t *nxGetConfigDescriptor(uint16_t Length)
{
	return xGetConfigDescriptor(Length);
}

/**************************************************************************/
/*! 
    GetStringDescriptor wrapper
*/
/**************************************************************************/
uint8_t *nxGetStringDescriptor(uint16_t Length)
{
	return xGetStringDescriptor(Length);
}

/**************************************************************************/
/*! 
    GetConfiguration wrapper
*/
/**************************************************************************/
void nxGetConfiguration(void){
	xGetConfiguration();
}

/**************************************************************************/
/*! 
    GetConfiguration wrapper
*/
/**************************************************************************/
void nxSetConfiguration(void){
	xSetConfiguration();
}

/**************************************************************************/
/*! 
    GetConfiguration wrapper
*/
/**************************************************************************/
void nxGetInterface(void){
	xGetInterface();
}

/**************************************************************************/
/*! 
    SetInterface wrapper
*/
/**************************************************************************/
void nxSetInterface(void){
	xSetInterface();
}

/**************************************************************************/
/*! 
    GetStatus wrapper
*/
/**************************************************************************/
void nxGetStatus(void){
	xGetStatus();
}

/**************************************************************************/
/*! 
    ClearFeature wrapper
*/
/**************************************************************************/
void nxClearFeature(void){
	xClearFeature();
}

/**************************************************************************/
/*! 
    SetEndPointFeature wrapper
*/
/**************************************************************************/
void nxSetEndPointFeature(void){
	xSetEndPointFeature();
}

/**************************************************************************/
/*! 
    SetDeviceFeature wrapper
*/
/**************************************************************************/
void nxSetDeviceFeature(void){
	xSetDeviceFeature();
}

/**************************************************************************/
/*! 
    SetDeviceAddress wrapper
*/
/**************************************************************************/
void nxSetDeviceAddress(void){
	xSetDeviceAddress();
}

/* End Of File ---------------------------------------------------------------*/
