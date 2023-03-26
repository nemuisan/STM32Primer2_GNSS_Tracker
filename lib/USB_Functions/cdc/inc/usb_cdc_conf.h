/********************************************************************************/
/*!
	@file			usb_cdc_conf.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.03.13
	@brief          Communication Device Class Configurations.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Adopted STM32_USB-FS-Device_DriverV4.0.0.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.13	V4.00	Re-arranged endpoint address.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef USB_CDC_CONF_H
#define USB_CDC_CONF_H	0x0400

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
#define CDC_EP_NUM				(4)


/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
/* Buffer table base address */
/* Decreared in usb_conf.h */
/*#define BTABLE_ADDRESS      (0x00)*/
	
	
/* Each buffer has 64Bytes,8byte-aligned */

/* EP0 OUT/IN (Control Transfer) */
/* Tx/Rx buffer base address */
#define CDC_ENDP0_RXADDR        (0x20)
#define CDC_ENDP0_TXADDR        (0x60)

/* EP1 IN  (UART -> PC) Bulk */
/* Tx buffer base address */
#define CDC_ENDP1_TXADDR        (0xA0)

/* EP2 IN  (UART -> PC) Interrupt */
/* Tx buffer base address */
#define CDC_ENDP2_TXADDR        (0xE0)

/* EP2 OUT (PC -> UART) Bulk */
/* Rx double buffer base address */
#define CDC_ENDP3_BUF0Addr      (0x120)
#define CDC_ENDP3_BUF1Addr      (0x160)


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

#endif	/* USB_CDC_CONF_H */
