/********************************************************************************/
/*!
	@file			power.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2011.03.10
	@brief          Power Control and Battery Supervisor on STM32Primer2.

    @section HISTORY
		2009.12.26	V0.02	See Update.txt
		2011.03.10	V2.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __POWER_H
#define __POWER_H 0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include "hw_config.h"


/* macros */
#define PWR_ON()		GPIO_ResetBits(GPIO_PWR, SHUTDOWN)
#define PWR_OFF()		GPIO_SetBits(GPIO_PWR, SHUTDOWN)
/* #define LIPO_LOWER_VOLT	3500 */  /* 3500mV */
#define LIPO_LOWER_VOLT	3375  /* nemui changed to 3375mV for GPS Tr@cker */
#define NO_BAT_VOLTAGE	3000  /* 3000mV */
#define MID_BAT_VOLTAGE	3600  /* 3600mV */
#define LOWER_FILT		12    /* 12Sec  */

#define SHUT_TIME		3000  /* 3000Sec */

enum
{
	BAT_HIGH,
	BAT_MIDDLE,
	BAT_LOW
};
	
/* Externals */
void PWR_Configuration(void);
void PWR_Mgn(void);
int16_t GetVbat(void);

extern __IO uint32_t BatState;
#define LOWBATT_ALARM  (BatState == BAT_LOW)

#ifdef __cplusplus
}
#endif

#endif  /*__POWER_H */
