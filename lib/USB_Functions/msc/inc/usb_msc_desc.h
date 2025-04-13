/********************************************************************************/
/*!
	@file			usb_msc_desc.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2025.04.03
	@brief          BOT State Machine management.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.23	V4.00	Fixed BOT data buffer to 4byte-alignment.
		2025.04.03	V5.00	Fixed dead-lock problem in EP1_IN_Callback.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef USB_MSC_DESC_H
#define USB_MSC_DESC_H	0x0500

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

#endif /* USB_MSC_DESC_H */
