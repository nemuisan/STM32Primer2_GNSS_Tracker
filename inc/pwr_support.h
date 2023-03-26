/********************************************************************************/
/*!
	@file			pwr_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.03.08
	@brief          Power Control and Battery Supervisor on STM32Primer2.

    @section HISTORY
		2009.12.26	V0.02	See Update.txt
		2011.03.10	V2.00	C++ Ready.
		2014.12.02	V3.00	Added WatchdogReset for USB functions.
		2014.12.22	V3.01	Enforce Watchdog handlings.
		2022.10.15	V4.00	Changed power-handlings,some codes and filename.
		2023.03.08	V5.00	Fixed lipo battery lower voltage limit.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef PWR_SUPPORT_H
#define PWR_SUPPORT_H 0x0500

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include "stm32f10x.h"
#include "hw_config.h"


/* macros */
#define PWR_ON()			GPIO_ResetBits(GPIO_PWR, SHUTDOWN)
#define PWR_OFF()			GPIO_SetBits(GPIO_PWR, SHUTDOWN)

/* Battery voltage defines */
#define LOWER_BAT_VOLTAGE	3350UL	/* 3350mV */ /* Lipo battery lower limit voltage */
#define NO_BAT_VOLTAGE		1000UL  /* 1000mV */ /* Death battery or USB Powered only */
#define MID_BAT_VOLTAGE		3600UL  /* 3600mV */
#define LOWER_FILT_TIME		20    	/* 20Sec  */

/* Shutdown Button */
#define SHUT_TIME			3000UL  /* 3000mSec */

/* IWWDG Timer Relation */
#define LSI_FREQ			40000UL	/* For Independent Watchdog(in Hz) */
#define WDT_INTERVAL		5000UL	/* For Independent Watchdog(in mSec) MAX6553.5mSec */

/* BATTERY STATE */
enum
{
	BAT_HIGH,
	BAT_MIDDLE,
	BAT_LOW
};
extern __IO uint32_t BatState;
#define LOWBATT_ALARM()		(BatState == BAT_LOW)

/* Externals */
extern void PWR_Configuration(void);
extern void PWR_Mgn(void);
extern int16_t GetVbat(void);


#ifdef __cplusplus
}
#endif

#endif  /* PWR_SUPPORT_H */
