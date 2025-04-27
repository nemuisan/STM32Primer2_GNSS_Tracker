/********************************************************************************/
/*!
	@file			hw_config.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2025.04.21
	@brief          Configure Basis System on STM32Primer2.

    @section HISTORY
		2010.12.31	V1.00	Restart Here.
		2011.03.10	V2.00	C++ Ready.
		2022.10.15	V3.00	Fixed cosmetic bugfixes.
		2023.04.21	V4.00	Re-Fixed cosmetic bugfix.
		2023.06.01	V5.00	Added MTK_Command mode at gnss logging.
		2025.04.07	V6.00	Fixed typo comment.
		2025.04.21	V7.00	Re-defined NVIC priority settings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef HW_CONFIG_H
#define HW_CONFIG_H 0x0700

#ifdef __cplusplus
 extern "C" {
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

/* Function Inclusion */
#include "systick.h"
#include "pwr_support.h"
#include "adc_support.h"
#include "rtc_support.h"
#include "uart_support_gps.h"
#include "sdio_stm32f1.h"
#include "xprintf.h"

/* High Level Function */
#include "ff.h"
#include "diskio.h"
#include "gnss_support.h"
#include "cdc_support.h"
#include "msc_support.h"


/* Macros */
#define countof(a)   	(sizeof(a) / sizeof(*(a)))

/* Function modes enumeration */
enum{ 
		NO_SELECTED,
		GNSS_LOGGING,
		GNSS_LOGGING_MTK,
		USB_MSC,
		USB_VCOM
	};

/* LED */
#define	LED_GRN_ON()	GPIO_SetBits(GPIO_LED, LED_GREEN)
#define	LED_GRN_OFF()	GPIO_ResetBits(GPIO_LED, LED_GREEN)
#define	LED_RED_ON()	GPIO_SetBits(GPIO_LED, LED_RED)
#define	LED_RED_OFF()	GPIO_ResetBits(GPIO_LED, LED_RED)
#define Led_RW_ON()		LED_GRN_ON()
#define Led_RW_OFF()	LED_GRN_OFF()

/* Externals */
extern void Set_System(void);
extern void NVIC_Configuration(void);
extern void LED_Configuration(void);
extern void KEY_Configuration(void);
extern void JoyInp_Chk(void);

/* USB Device Relation */
extern void Set_USBClock(void);
extern void SetSysClock72(void);
extern void Enter_LowPowerMode(void);
extern void Leave_LowPowerMode(void);
extern void USB_Cable_Config (FunctionalState NewState);
extern void Get_SerialNum(uint8_t* string0,uint8_t* string1);
extern void USB_Disconnect_Config(void);

/* System Global Valuable */
extern __IO uint16_t CmdKey;
extern __IO int TaskStat;

#ifdef __cplusplus
}
#endif

#endif	/* HW_CONFIG_H */
