/********************************************************************************/
/*!
	@file			usb_msc_conf.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.03.13
	@brief          Mass Storage Class Configurations.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2023.03.13	V2.00	Re-arranged endpoint address.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef USB_MSC_CONF_H
#define USB_MSC_CONF_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include "usb_conf.h"

/* Macros Plus */
/*-------------------------------------------------------------*/
/* MSC_EP_NUM */
/* defines how many endpoints are used by the device */
/*-------------------------------------------------------------*/
#define MSC_EP_NUM				(3)


/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
/* Buffer table base address */
/* Decreared in usb_conf.h */
/*#define BTABLE_ADDRESS      (0x00)*/
	
	
/* Each buffer has 64Bytes,8byte-aligned */

/* EP0 OUT/IN (Control Transfer) */
/* Tx/Rx buffer base address */
#define MSC_ENDP0_RXADDR        (0x18)
#define MSC_ENDP0_TXADDR        (0x58)

/* EP1 IN  (SDCard -> PC) Bulk */
/* Tx buffer base address */
#define MSC_ENDP1_TXADDR        (0x98)

/* EP2 OUT (PC-> SDCard) Bulk */
/* Rx double buffer base address */
#define MSC_ENDP2_BUF0Addr   	(0xD8)
#define MSC_ENDP2_BUF1Addr   	(0x118) 


/*-------------------------------------------------------------*/
/* -------------------   ISTR events  -------------------------*/
/*-------------------------------------------------------------*/
/* IMR_MSK */
/* mask defining which events has to be handled */
/* by the device application software */
#define MSC_IMR_MSK (CNTR_CTRM  | CNTR_WKUPM | CNTR_SUSPM | CNTR_ERRM  | CNTR_SOFM \
                   | CNTR_ESOFM | CNTR_RESETM )

#ifdef __cplusplus
}
#endif

#endif	/* USB_MSC_CONF_H */
