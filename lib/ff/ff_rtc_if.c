/********************************************************************************/
/*!
	@file			ff_rtc_if.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.04.21
	@brief          FatFs Realtime Clock Section.					@n
					Hardware Abstraction Layer.

    @section HISTORY
		2012.08.27	V1.00	Start Here.
		2023.04.21	V2.00	Fixed cosmetic bugfix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ff_rtc_if.h"
/* check header file version for fool proof */
#if FF_RTC_IF_H!= 0x0200
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#define USE_STM32RTC

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  FatFs Realtime Clock Section.
	@param  RTC : rtc structure
    @retval : 1
*/
/**************************************************************************/
int rtc_gettime(FF_RTC *f_rtc)
{

#ifdef USE_STM32RTC
	/* See rtc_support.h */
	rtc = Time_GetCalendarTime();

	f_rtc->sec   = ts_sec;
	f_rtc->min   = ts_min;
	f_rtc->hour  = ts_hour;
	f_rtc->mday  = ts_mday;
	f_rtc->month = ts_mon+1;
	f_rtc->year  = ts_year;

#else
	/* 2011/11/13 22:23:24 */
	f_rtc->sec   = 24;
	f_rtc->min   = 23;
	f_rtc->hour  = 22;
	f_rtc->mday  = 13;
	f_rtc->month = 11;
	f_rtc->year  = 2011;
#endif

	return 1;
}

/**************************************************************************/
/*! 
    @brief  FatFs Realtime Clock Section.
	@param  RTC : rtc structure
    @retval : 1
*/
/**************************************************************************/
int rtc_settime(const FF_RTC *f_rtc)
{

#ifdef USE_STM32RTC
	/* See rtc_support.h */
	ts_sec	= f_rtc->sec;
	ts_min  = f_rtc->min;
	ts_hour = f_rtc->hour;
	ts_mday = f_rtc->mday;
	ts_mon  = f_rtc->month-1;
	ts_year = f_rtc->year;

	Time_SetCalendarTime(rtc);
#else
	/* 2011/11/13 22:23:24 */
	/* Do nothing */
#endif

	return 1;
}


/* End Of File ---------------------------------------------------------------*/
