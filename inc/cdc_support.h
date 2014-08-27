/********************************************************************************/
/*!
	@file			cdc_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2014.07.16
	@brief          Interface of USB-CommunicationDeviceClass.

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.04.20	V2.00	Fixed Suitable Interruption level.
		2014.07.16	V3.00	Reset Systick to Suitable Frequency.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __CDC_SUPPORT_H
#define __CDC_SUPPORT_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include <string.h> /* memset et al*/

/* Miscs */
#include "usb_lib.h"
#include "usb_cdc_desc.h"
#include "usb_cdc_prop.h"
#include "usb_cdc_conf.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "hw_config.h"

/* Function Prototypes */
extern void cdc_task(void);

extern void (*xUART_IRQ)(void);
extern void CDC_IRQ(void);
extern void USART_Config_Default(void);
extern bool USART_Config(void);
extern void USB_To_USART_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes);
extern void Handle_USBAsynchXfer(void);

#ifdef __cplusplus
}
#endif

#endif	/*__CDC_SUPPORT_H */
