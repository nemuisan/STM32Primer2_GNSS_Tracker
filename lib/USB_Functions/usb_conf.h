/********************************************************************************/
/*!
	@file			usb_conf.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.03.14
	@brief          USB Endpoint and Misc Function Wrappers.

    @section HISTORY
		2012.01.30	V1.00	Start Here
		2014.12.17	V2.00	Adopted GCC4.9.x.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.14	V4.00	Added external declaration.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef USB_CONF_H
#define USB_CONF_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include <stdbool.h>	/* Nemui */
#ifndef FALSE			/* in case these macros already exist */
#define FALSE	false	/* values of boolean */
#endif
#ifndef TRUE
#define TRUE	true
#endif

/* General Inclusion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#include "stm32f10x_it.h"
#include "stm32f10x_conf.h"
#include "platform_config.h"


/* Function Prototypes and Macros Plus */

/*-------------------------------------------------------------*/
/* --------------   Buffer Description Table  -----------------*/
/*-------------------------------------------------------------*/
/* buffer table base address */
#define BTABLE_ADDRESS      (0x00)

/*-------------------------------------------------------------*/
/* -------------------   ISTR events  -------------------------*/
/*-------------------------------------------------------------*/
/* IMR_MSK */
/* mask defining which events has to be handled */
/* by the device application software */
#define IMR_MSK (CNTR_CTRM  | CNTR_SOFM  | CNTR_RESETM )

/*#define CTR_CALLBACK*/
/*#define DOVR_CALLBACK*/
/*#define ERR_CALLBACK*/
/*#define WKUP_CALLBACK*/
/*#define SUSP_CALLBACK*/
/*#define RESET_CALLBACK*/
/*#define SOF_CALLBACK*/
/*#define ESOF_CALLBACK*/

/* CTR service routines */
/* associated to defined endpoints */
/*#define  EP1_IN_Callback   NOP_Process*/
#define  EP2_IN_Callback   NOP_Process
#define  EP3_IN_Callback   NOP_Process
#define  EP4_IN_Callback   NOP_Process
#define  EP5_IN_Callback   NOP_Process
#define  EP6_IN_Callback   NOP_Process
#define  EP7_IN_Callback   NOP_Process

#define  EP1_OUT_Callback   NOP_Process
/*#define  EP2_OUT_Callback   NOP_Process*/
/*#define  EP3_OUT_Callback   NOP_Process*/
#define  EP4_OUT_Callback   NOP_Process
#define  EP5_OUT_Callback   NOP_Process
#define  EP6_OUT_Callback   NOP_Process
#define  EP7_OUT_Callback   NOP_Process

extern void (* volatile xEP1_IN_Callback)(void);
extern void (* volatile xEP3_OUT_Callback)(void);
extern void (* volatile xEP2_OUT_Callback)(void);
extern void (* volatile xSOF_Callback)(void);

/* USB-CommunicationDeviceClass Functions Externals */
extern void CDC_EP1_IN_Callback(void);
extern void CDC_EP3_OUT_Callback(void);
extern void CDC_SOF_Callback(void);

/* USB-MassStorageClass Functions Externals */
extern void MSC_EP1_IN_Callback(void);
extern void MSC_EP2_OUT_Callback(void);

#ifdef __cplusplus
}
#endif

#endif	/* USB_CONF_H */
