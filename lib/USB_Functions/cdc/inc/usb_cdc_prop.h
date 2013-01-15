/********************************************************************************/
/*!
	@file			usb_cdc_prop.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.30
	@brief          All processing related to Virtual COM Port.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __USB_CDC_PROP_H
#define __USB_CDC_PROP_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* Include */
#include "usb_core.h"

/* Macros Plus */
typedef struct
{
  uint32_t bitrate;
  uint8_t format;
  uint8_t paritytype;
  uint8_t datatype;
}LINE_CODING;

#define Virtual_Com_Port_GetConfiguration          NOP_Process
//#define Virtual_Com_Port_SetConfiguration          NOP_Process
#define Virtual_Com_Port_GetInterface              NOP_Process
#define Virtual_Com_Port_SetInterface              NOP_Process
#define Virtual_Com_Port_GetStatus                 NOP_Process
#define Virtual_Com_Port_ClearFeature              NOP_Process
#define Virtual_Com_Port_SetEndPointFeature        NOP_Process
#define Virtual_Com_Port_SetDeviceFeature          NOP_Process
//#define Virtual_Com_Port_SetDeviceAddress          NOP_Process

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

/* Externals */
void Virtual_Com_Port_init(void);
void Virtual_Com_Port_Reset(void);
void Virtual_Com_Port_SetConfiguration(void);
void Virtual_Com_Port_SetDeviceAddress (void);
void Virtual_Com_Port_Status_In (void);
void Virtual_Com_Port_Status_Out (void);
RESULT Virtual_Com_Port_Data_Setup(uint8_t);
RESULT Virtual_Com_Port_NoData_Setup(uint8_t);
RESULT Virtual_Com_Port_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting);
uint8_t *Virtual_Com_Port_GetDeviceDescriptor(uint16_t );
uint8_t *Virtual_Com_Port_GetConfigDescriptor(uint16_t);
uint8_t *Virtual_Com_Port_GetStringDescriptor(uint16_t);

uint8_t *Virtual_Com_Port_GetLineCoding(uint16_t Length);
uint8_t *Virtual_Com_Port_SetLineCoding(uint16_t Length);

extern void CDC_SetStructure(void);

#ifdef __cplusplus
}
#endif

#endif	/*__USB_CDC_PROP_H */
