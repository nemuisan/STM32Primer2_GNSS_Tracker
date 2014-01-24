/********************************************************************************/
/*!
	@file			usb_cdc_conf.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2014.01.23
	@brief          Communication Device Class Configurations.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Adopted STM32_USB-FS-Device_DriverV4.0.0.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __USB_CDC_CONF_H
#define __USB_CDC_CONF_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include "usb_conf.h"

/* Macros Plus */
/*-------------------------------------------------------------*/
/* CDC_EP_NUM */
/* defines how many endpoints are used by the device */
/*-------------------------------------------------------------*/
#define CDC_EP_NUM                          (4)


/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
/* buffer table base address */
/* Decreared in usb_conf.h */
/*#define BTABLE_ADDRESS      (0x00)*/

/* EP0  */
/* rx/tx buffer base address */
#define CDC_ENDP0_RXADDR        (0x40)
#define CDC_ENDP0_TXADDR        (0x80)

/* EP1  */
/* tx buffer base address */
#define CDC_ENDP1_TXADDR        (0xC0)
#define CDC_ENDP2_TXADDR        (0x100)
#define CDC_ENDP3_RXADDR        (0x110)

#define CDC_ENDP3_BUF0Addr      (0x110) /* Nemui */
#define CDC_ENDP3_BUF1Addr      (0x160) /* Nemui */


/*-------------------------------------------------------------*/
/* -------------------   ISTR events  -------------------------*/
/*-------------------------------------------------------------*/
/* IMR_MSK */
/* mask defining which events has to be handled */
/* by the device application software */
#define CDC_IMR_MSK (CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM | \
					 CNTR_ESOFM | CNTR_RESETM )
#define CDC_SOF_CALLBACK

#ifdef __cplusplus
}
#endif

#endif	/*__USB_CDC_CONF_H */
