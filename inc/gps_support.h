/********************************************************************************/
/*!
	@file			gps_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2013.10.09
	@brief          Interface of FatFs For STM32 uC.				@n
					Based on Chan's GPS-Logger Program Thanks!

    @section HISTORY
		2011.03.10	V1.00	Start Here.
		2011.09.07	V2.00	Added RTC Synchronization from GPRMC.
		                    Fixed More Stability.
		2011.12.26	V3.00	Added PA6C Support.
		2012.08.31  V4.00   Imploved FatFs Support Function's Portability.
		2012.09.08	V5.00	Imploved PA6C Support.
		                     (Anti Interference & force 9600bps for 38400bps Firmware).
		2012.12.24  V6.00   Added Gms-g6a(MT3333 GLONASS MODE) Support.
							Imploved Error Handlings.
		2013.02.20  V7.00   Added Some MT3339/MT3333 Commands.
		2013.04.10  V8.00   Changed UART-Retarget Method.
		2013.10.09	V9.00	Adopted FatFs0.10.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __GPS_SUPPORT_H
#define __GPS_SUPPORT_H	0x0900

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include <string.h> /* memset et al*/

/* Miscs */
#include "hw_config.h"
#include "diskio.h"
#include "ff.h"
#include "ff_rtc_if.h"
#include "rtc_support.h"

/* Function Prototypes */
extern void ff_support_timerproc(void);
extern void gps_task(void);
extern void ShutFileClose(void);
extern void ChkAckLimit(void);

#ifdef __cplusplus
}
#endif

#endif	/* __GPS_SUPPORT_H */
