/********************************************************************************/
/*!
	@file			usb_msc_desc.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.30
	@brief          Descriptor Header for Mass Storage Device.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __USB_MSC_DESC_H
#define __USB_MSC_DESC_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include "stm32f10x.h"
 
/* Macros Plus */
#define MASS_SIZ_DEVICE_DESC              18
#define MASS_SIZ_CONFIG_DESC              32

#define MASS_SIZ_STRING_LANGID            4
#define MASS_SIZ_STRING_VENDOR            38
#define MASS_SIZ_STRING_PRODUCT           38
#define MASS_SIZ_STRING_SERIAL            26
#define MASS_SIZ_STRING_INTERFACE         16

/* Externals */
extern const uint8_t Mass_DeviceDescriptor[MASS_SIZ_DEVICE_DESC];
extern const uint8_t Mass_ConfigDescriptor[MASS_SIZ_CONFIG_DESC];
extern const uint8_t Mass_StringLangID[MASS_SIZ_STRING_LANGID];
extern const uint8_t Mass_StringVendor[MASS_SIZ_STRING_VENDOR];
extern const uint8_t Mass_StringProduct[MASS_SIZ_STRING_PRODUCT];
extern uint8_t Mass_StringSerial[MASS_SIZ_STRING_SERIAL];
extern const uint8_t Mass_StringInterface[MASS_SIZ_STRING_INTERFACE];

#ifdef __cplusplus
}
#endif

#endif /* __USB_MSC_DESC_H */
