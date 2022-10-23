/********************************************************************************/
/*!
	@file			pwr_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2022.10.15
	@brief          Power Control and Battery Supervisor on STM32Primer2.

    @section HISTORY
		2009.12.26	V0.02	See Update.txt
		2011.03.10	V2.00	C++ Ready.
		2014.12.02	V3.00	Added WatchdogReset for USB functions.
		2014.12.22	V3.01	Enforce Watchdog handlings.
		2022.10.15	V4.00	Changed power-handlings,some codes and filename.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __PWR_SUPPORT_H
#define __PWR_SUPPORT_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include "stm32f10x.h"
#include "hw_config.h"


/* macros */
#define PWR_ON()		GPIO_ResetBits(GPIO_PWR, SHUTDOWN)
#define PWR_OFF()		GPIO_SetBits(GPIO_PWR, SHUTDOWN)

/* #define LIPO_LOWER_VOLT	3500 */  /* 3500mV */
#define LIPO_LOWER_VOLT	3375  	/* nemui changed to 3375mV for GPS Tr@cker */
#define NO_BAT_VOLTAGE	3000  	/* 3000mV */
#define MID_BAT_VOLTAGE	3600  	/* 3600mV */
#define LOWER_FILT		12    	/* 12Sec  */

/* Shutdown Button */
#define SHUT_TIME		3000  	/* 3000mSec */

/* IWWDG Timer Relation */
#define LSI_FREQ		40000UL	/* For Independent Watchdog(in Hz) */
#define WDT_INTERVAL	5000UL	/* For Independent Watchdog(in mSec) MAX6553.5mSec */

/* BATTERY STATE */
enum
{
	BAT_HIGH,
	BAT_MIDDLE,
	BAT_LOW
};
extern __IO uint32_t BatState;
#define LOWBATT_ALARM()  		(BatState == BAT_LOW)

/* Externals */
extern void PWR_Configuration(void);
extern void PWR_Mgn(void);
extern int16_t GetVbat(void);


#ifdef __cplusplus
}
#endif

#endif  /*__PWR_SUPPORT_H */
