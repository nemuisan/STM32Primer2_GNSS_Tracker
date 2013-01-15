/********************************************************************************/
/*!
	@file			ff_rtc_if.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.08.27
	@brief          FatFs Realtime Clock Section.					@n
					Hardware Abstraction Layer.

    @section HISTORY
		2012.08.27	V1.00 Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ff_rtc_if.h"

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
int rtc_gettime(FF_RTC *ff_rtc)
{

#ifdef USE_STM32RTC
	/* See rtc_support.h */
	rtc = Time_GetCalendarTime();

	ff_rtc->sec   = ts_sec;
	ff_rtc->min   = ts_min;
	ff_rtc->hour  = ts_hour;
	ff_rtc->mday  = ts_mday;
	ff_rtc->month = ts_mon+1;
	ff_rtc->year  = ts_year;

#else
	/* 2011/11/13 22:23:24 */
	ff_rtc->sec   = 24;
	ff_rtc->min   = 23;
	ff_rtc->hour  = 22;
	ff_rtc->mday  = 13;
	ff_rtc->month = 11;
	ff_rtc->year  = 2011;
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
int rtc_settime(const FF_RTC *ff_rtc)
{

#ifdef USE_STM32RTC
	/* See rtc_support.h */
	ts_sec	= ff_rtc->sec;
	ts_min  = ff_rtc->min;
	ts_hour = ff_rtc->hour;
	ts_mday = ff_rtc->mday;
	ts_mon  = ff_rtc->month-1;
	ts_year = ff_rtc->year;

	Time_SetCalendarTime(rtc);
#else
	/* 2011/11/13 22:23:24 */
	/* Do nothing */
#endif

	return 1;
}


/* End Of File ---------------------------------------------------------------*/
