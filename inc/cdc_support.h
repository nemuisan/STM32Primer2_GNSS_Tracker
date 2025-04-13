/********************************************************************************/
/*!
	@file			cdc_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2025.04.08
	@brief          Interface of USB-CommunicationDeviceClass.

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.04.20	V2.00	Fixed Suitable Interruption level.
		2014.07.16	V3.00	Reset Systick to Suitable Frequency.
		2019.09.20	V4.00	Fixed redundant declaration.
		2020.05.30	V5.00	Display system version string.
		2022.10.10	V6.00	Purge UART buffer on connect.
		2023.03.22	V7.00	Enable UART Rx interrupt on connect.
		2023.12.19  V8.00	Improved watchdog handlings.
		2025.04.08	V9.00	Changed minor function name.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef CDC_SUPPORT_H
#define CDC_SUPPORT_H 0x0900

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

/* CDC-USART Definition */
#define CDC_UART			USART2
#define CDC_UART_IRQ		USART2_IRQn

/* Function Prototypes */
extern void cdc_task(void);

extern void CDC_IRQ(void);
extern void USART_Config_Default(void);
extern bool USART_Config(void);
extern void USART_EnableRXInt(void);
extern void USART_DisableRXInt(void);
extern void USB_To_USART_Send_Data(uint8_t* data_buffer, uint8_t Nb_bytes);
extern void Handle_USBAsynchXfer(void);

#ifdef __cplusplus
}
#endif

#endif	/* CDC_SUPPORT_H */
