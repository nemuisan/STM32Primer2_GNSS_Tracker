/********************************************************************************/
/*!
	@file			usb_msc_prop.h
    @version        3.00
    @date           2019.09.20
	@brief          Mass Storage middle layer.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2019.09.20	V6.00	Fixed shadowed variable.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __USB_MSC_PROP_H
#define __USB_MSC_PROP_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* Include */
#include "usb_core.h"

/* Macros Plus */
#define Mass_GetConfiguration          NOP_Process
/* #define Mass_SetConfiguration          NOP_Process*/
#define Mass_GetInterface              NOP_Process
#define Mass_SetInterface              NOP_Process
#define Mass_GetStatus                 NOP_Process
/* #define Mass_ClearFeature              NOP_Process*/
#define Mass_SetEndPointFeature        NOP_Process
#define Mass_SetDeviceFeature          NOP_Process
/*#define Mass_SetDeviceAddress          NOP_Process*/

/* MASS Storage Requests*/
#define GET_MAX_LUN                0xFE
#define MASS_STORAGE_RESET         0xFF
#define LUN_DATA_LENGTH            1

/* Externals */
extern void Mass_init(void);
extern void Mass_Reset(void);
extern void Mass_SetConfiguration(void);
extern void Mass_ClearFeature(void);
extern void Mass_SetDeviceAddress (void);
extern void Mass_Status_In (void);
extern void Mass_Status_Out (void);
extern RESULT Mass_Data_Setup(uint8_t);
extern RESULT Mass_NoData_Setup(uint8_t);
extern RESULT Mass_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
extern uint8_t *Mass_GetDeviceDescriptor(uint16_t );
extern uint8_t *Mass_GetConfigDescriptor(uint16_t);
extern uint8_t *Mass_GetStringDescriptor(uint16_t);
extern uint8_t *Get_Max_Lun(uint16_t Length);

extern void MSC_SetStructure(void);

#ifdef __cplusplus
}
#endif

#endif /* __USB_MSC_PROP_H */
