/********************************************************************************/
/*!
	@file			ff_rtc_if.h
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
#ifndef FF_RTC_IF_H
#define FF_RTC_IF_H 0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* Generic Inclusion */
#include <inttypes.h>
#include <time.h>

/* uC Related Inclusion */
#include "rtc_support.h"

/* Defines */
typedef struct {
	uint16_t	year;	/* 1..4095 */
	uint8_t		month;	/* 1..12 */
	uint8_t		mday;	/* 1.. 31 */
	uint8_t		wday;	/* 1..7 */
	uint8_t		hour;	/* 0..23 */
	uint8_t		min;	/* 0..59 */
	uint8_t		sec;	/* 0..59 */
} FF_RTC;

/* Function Prototypes */
int rtc_settime(const FF_RTC *f_rtc);
int rtc_gettime(FF_RTC *f_rtc);
extern FF_RTC ff_rtc;

#ifdef __cplusplus
}
#endif

#endif /*  FF_RTC_IF_H */
