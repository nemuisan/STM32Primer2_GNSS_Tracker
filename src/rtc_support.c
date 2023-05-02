/********************************************************************************/
/*!
	@file			rtc_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.04.21
	@brief          Based on Keil's Sample Thanks!

    @section HISTORY
		2010.01.19	V1.00	Restart Here.
		2010.12.31	V2.00	Renamed Filename.
		2011.03.10	V3.00	C++ Ready.
		2012.08.27	V4.00	Improve Portability
		2023.04.21	V5.00	Fixed cosmetic bugfix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "rtc_support.h"
#if RTC_SUPPORT_H!= 0x0500
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint32_t TimeDisplay = 0;
time_t current_time;
struct tm rtc;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    RealTimeClockmodule Initialize
*/
/**************************************************************************/
void RTC_Configuration(void)
{
	/* RTC Initialize check */
	if (BKP_ReadBackupRegister(BKP_DR5) != 0xA5A5) {
		/* Backup data register value is not correct or not yet programmed (when
		   the first time the program is executed) */

		/* CK_RTC clock selection */
		RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE );

		/* LSI clock stabilization time */
		/* for(i=0;i<5000;i++) { ; } */

		/* Allow access to BKP Domain */
		PWR_BackupAccessCmd(ENABLE);
		
		/* Write Baby Flag */
		BKP_WriteBackupRegister(BKP_DR5, 0xA5A5);
	
		/* Reset Backup Domain */
		BKP_DeInit();

		/* Enable LSE */
		RCC_LSEConfig(RCC_LSE_ON);

		/* Wait till LSE is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) { ; }

		/* Select LSE as RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		/* Enable RTC Clock */
		RCC_RTCCLKCmd(ENABLE);

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

		/* Enable the RTC Second */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

		/* Set RTC prescaler: set RTC period to 1sec */
		RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

		/* Set initial value */
		rtc.tm_year = 2009;
		rtc.tm_mon = (7)-1;
		rtc.tm_mday = 15;
		rtc.tm_hour = 23;
		rtc.tm_min = 45;
		rtc.tm_sec = 45;
		Time_SetCalendarTime(rtc);

		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

		/* Allow access to BKP Domain */
		PWR_BackupAccessCmd(ENABLE);
		
		/* Write Baby Flag */
		BKP_WriteBackupRegister(BKP_DR5, 0xA5A5);

		/* Lock access to BKP Domain */
		PWR_BackupAccessCmd(DISABLE);

	} else {

		/* CK_RTC clock selection */
		RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE );

		/* Wait for RTC registers synchronization */
		RTC_WaitForSynchro();
		
		/* Enable the RTC Second */
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		
		/* Wait until last write operation on RTC registers has finished */
		RTC_WaitForLastTask();

	}

}

/**************************************************************************/
/*! 
    Calculate CalendarTime using newlib
	UnixTime -> CalendarTime
*/
/**************************************************************************/
struct tm Time_ConvUnixToCalendar(time_t t)
{
	struct tm *t_tm;
	t_tm = localtime(&t);
	t_tm->tm_year += 1900;
	return *t_tm;
}

/**************************************************************************/
/*! 
    Calculate CalendarTime using newlib
	Calendar -> TimeUnixTime
*/
/**************************************************************************/
time_t Time_ConvCalendarToUnix(struct tm t)
{
	t.tm_year -= 1900;

	return mktime(&t);
}

/**************************************************************************/
/*! 
    Interface newlib time module between STM32uC RTC.
*/
/**************************************************************************/
time_t Time_GetUnixTime(void)
{
	return (time_t)RTC_GetCounter();
}

/**************************************************************************/
/*! 
    Interface newlib time module between STM32uC RTC.
*/
/**************************************************************************/
struct tm Time_GetCalendarTime(void)
{
	time_t t_t;
	struct tm t_tm;

	t_t = (time_t)RTC_GetCounter();
	t_tm = Time_ConvUnixToCalendar(t_t);
	return t_tm;
}

/**************************************************************************/
/*! 
    Interface newlib time module between STM32uC RTC.
*/
/**************************************************************************/
void Time_SetUnixTime(time_t t)
{
	PWR_BackupAccessCmd(ENABLE);
	RTC_WaitForLastTask();
	RTC_SetCounter((uint32_t)t);
	RTC_WaitForLastTask();
	PWR_BackupAccessCmd(DISABLE);
	return;
}

/**************************************************************************/
/*! 
    Interface newlib time module between STM32uC RTC.
*/
/**************************************************************************/
void Time_SetCalendarTime(struct tm t)
{
	Time_SetUnixTime(Time_ConvCalendarToUnix(t));
	return;
}

/* End Of File ---------------------------------------------------------------*/
