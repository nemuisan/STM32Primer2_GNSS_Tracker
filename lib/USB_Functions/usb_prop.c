/********************************************************************************/
/*!
	@file			usb_prop.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.30
	@brief          USB Endpoint Wrappers.

    @section HISTORY
		2012.01.30	V1.00	Start Here

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_prop.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
void (*xinit)(void);
void (*xReset)(void);
void (*xStatus_In)(void);
void (*xStatus_Out)(void);
RESULT (*xData_Setup)(uint8_t RequestNo);
RESULT (*xNoData_Setup)(uint8_t RequestNo);
RESULT (*xGet_Interface_Setting)(uint8_t Interface, uint8_t AlternateSetting);
uint8_t* (*xGetDeviceDescriptor)(uint16_t Length);
uint8_t* (*xGetConfigDescriptor)(uint16_t Length);
uint8_t* (*xGetStringDescriptor)(uint16_t Length);

void (*xGetConfiguration)(void);
void (*xSetConfiguration)(void);
void (*xGetInterface)(void);
void (*xSetInterface)(void);
void (*xGetStatus)(void);
void (*xClearFeature)(void);
void (*xSetEndPointFeature)(void);
void (*xSetDeviceFeature)(void);
void (*xSetDeviceAddress)(void);

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
    0x40 /*MAX PACKET SIZE*/
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
void nxSetInterface(){
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
	xSetDeviceFeature();
}

/* End Of File ---------------------------------------------------------------*/
