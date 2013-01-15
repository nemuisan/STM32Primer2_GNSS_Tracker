/********************************************************************************/
/*!
	@file			rtc_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2012.08.27
	@brief          Based on Keil's Sample Thanks!

    @section HISTORY
		2010.01.19	V1.00	Restart Here.
		2010.12.31	V2.00	Renamed Filename.
		2011.03.10	V3.00	C++ Ready.
		2012.08.27	V4.00	Improve Portability

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __RTC_SUPPORT_H
#define __RTC_SUPPORT_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "stm32f10x.h"

/* Function Prototypes */
extern void RTC_Configuration(void);
extern struct tm Time_ConvUnixToCalendar(time_t t);
extern time_t Time_ConvCalendarToUnix(struct tm t);
extern time_t Time_GetUnixTime(void);
extern struct tm Time_GetCalendarTime(void);
extern void Time_SetUnixTime(time_t);
extern void Time_SetCalendarTime(struct tm t);

/* External Glovals */
extern __IO uint32_t TimeDisplay;
extern time_t current_time;
extern struct tm rtc;

#define ts_hour	rtc.tm_hour
#define ts_min  rtc.tm_min
#define ts_sec  rtc.tm_sec

#define ts_mon  rtc.tm_mon
#define ts_mday rtc.tm_mday
#define ts_year rtc.tm_year

#ifdef __cplusplus
}
#endif

#endif /* __RTC_SUPPORT_H */
