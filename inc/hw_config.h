/********************************************************************************/
/*!
	@file			hw_config.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.03.10
	@brief          Configure Basis System on STM32Primer2.

    @section HISTORY
		2010.12.31	V1.00	Restart Here.
		2011.03.10	V2.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H 0x0200

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
#include "power.h"
#include "adc_support.h"
#include "rtc_support.h"
#include "uart_support_gps.h"
#include "sdio_stm32f1.h"

/* High Level Function */
#include "ff.h"
#include "diskio.h"
#include "term_io.h"
#include "gps_support.h"
#include "cdc_support.h"
#include "msc_support.h"


/* Macros */
#define countof(a)   	(sizeof(a) / sizeof(*(a)))
enum{ 
		GPS_LOGGING,
		STM32_MSC,
		STM32_VCOM,
	};

/* Power */
#define PWR_ON()		GPIO_ResetBits(GPIO_PWR, SHUTDOWN)
#define PWR_OFF()		GPIO_SetBits(GPIO_PWR, SHUTDOWN)
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

extern void Set_USBClock(void);
extern void Set72(void);
extern void Enter_LowPowerMode(void);
extern void Leave_LowPowerMode(void);
extern void USB_Cable_Config (FunctionalState NewState);
extern void Get_SerialNum(uint8_t* string0,uint8_t* string1);
extern void USB_Disconnect_Config(void);
extern __IO uint16_t CmdKey;
extern __IO int TaskStat;

#ifdef __cplusplus
}
#endif

#endif	/* __HW_CONFIG_H */
