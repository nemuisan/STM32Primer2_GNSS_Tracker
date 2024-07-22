/********************************************************************************/
/*!
	@file			usb_prop.h
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
#ifndef USB_PROP_H
#define USB_PROP_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes */
#include "usb_conf.h"
#include "usb_core.h"

/* Externals */
extern void (* volatile xinit)(void);
extern void (* volatile xReset)(void);
extern void (* volatile xStatus_In)(void);
extern void (* volatile xStatus_Out)(void);
extern RESULT (* volatile xData_Setup)(uint8_t RequestNo);
extern RESULT (* volatile xNoData_Setup)(uint8_t RequestNo);
extern RESULT (* volatile xGet_Interface_Setting)(uint8_t Interface, uint8_t AlternateSetting);
extern uint8_t* (* volatile xGetDeviceDescriptor)(uint16_t Length);
extern uint8_t* (* volatile xGetConfigDescriptor)(uint16_t Length);
extern uint8_t* (* volatile xGetStringDescriptor)(uint16_t Length);

extern void (* volatile xGetConfiguration)(void);
extern void (* volatile xSetConfiguration)(void);
extern void (* volatile xGetInterface)(void);
extern void (* volatile xSetInterface)(void);
extern void (* volatile xGetStatus)(void);
extern void (* volatile xClearFeature)(void);
extern void (* volatile xSetEndPointFeature)(void);
extern void (* volatile xSetDeviceFeature)(void);
extern void (* volatile xSetDeviceAddress)(void);

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
extern void nxSetInterface(void);
extern void nxGetStatus(void);
extern void nxClearFeature(void);
extern void nxSetEndPointFeature(void);
extern void nxSetDeviceFeature(void);
extern void nxSetDeviceAddress(void);

#ifdef __cplusplus
}
#endif

#endif	/* USB_PROP_H */
