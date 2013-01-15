/********************************************************************************/
/*!
	@file			usb_prop.h
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
#ifndef __USB_PROP_H
#define __USB_PROP_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes */
#include "usb_conf.h"
#include "usb_core.h"

/* Externals */
extern void (*xinit)(void);
extern void (*xReset)(void);
extern void (*xStatus_In)(void);
extern void (*xStatus_Out)(void);
extern RESULT (*xData_Setup)(uint8_t RequestNo);
extern RESULT (*xNoData_Setup)(uint8_t RequestNo);
extern RESULT (*xGet_Interface_Setting)(uint8_t Interface, uint8_t AlternateSetting);
extern uint8_t* (*xGetDeviceDescriptor)(uint16_t Length);
extern uint8_t* (*xGetConfigDescriptor)(uint16_t Length);
extern uint8_t* (*xGetStringDescriptor)(uint16_t Length);

extern void (*xGetConfiguration)(void);
extern void (*xSetConfiguration)(void);
extern void (*xGetInterface)(void);
extern void (*xSetInterface)(void);
extern void (*xGetStatus)(void);
extern void (*xClearFeature)(void);
extern void (*xSetEndPointFeature)(void);
extern void (*xSetDeviceFeature)(void);
extern void (*xSetDeviceAddress)(void);

extern void nxinit(void);
extern void nxReset(void);
extern void nxStatus_In(void);
extern void nxStatus_Out(void);
extern RESULT nxData_Setup(uint8_t RequestNo);
extern RESULT nxNoData_Setup(uint8_t RequestNo);
extern RESULT nxGet_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
extern uint8_t* nxGetDeviceDescriptor(uint16_t Length);
extern uint8_t* nxGetConfigDescriptor(uint16_t Length);
extern uint8_t* nxGetStringDescriptor(uint16_t Length);

extern void nxGetConfiguration(void);
extern void nxSetConfiguration(void);
extern void nxGetInterface(void);
extern void nxSetInterface();
extern void nxGetStatus(void);
extern void nxClearFeature(void);
extern void nxSetEndPointFeature(void);
extern void nxSetDeviceFeature(void);
extern void nxSetDeviceAddress(void);

#ifdef __cplusplus
}
#endif

#endif	/*__USB_PROP_H */
