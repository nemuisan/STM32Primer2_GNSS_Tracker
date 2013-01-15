/********************************************************************************/
/*!
	@file			systick.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2012.04.25
	@brief          delay mSec-order routine using systick timer			@n
					delay uSec-order routine using Timer5;

    @section HISTORY
		2010.03.05	V1.00	Restart Here.
		2011.01.20	V2.00	Using Timer5 for _delay_us(); instead of __NOP();.
		2011.03.10	V3.00	C++ Ready.
		2014.04.25  V4.00	Fixed Timer5 Clock definition fot _delay_us();

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SYSTICK_H
#define __SYSTICK_H 0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include "platform_config.h"

/* Systick Interval MilliSecond order (in Hz) */
#define INTERVAL		1000UL
/* Timer Interval MicroSecond order (in Hz) */
#define USEC_INTERVAL 	1000000UL

/* Inline Static Function */
extern __IO uint32_t TimingDelay;
static inline void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

/* Externals */
extern void _delay_ms(__IO uint32_t mSec);
extern void _delay_us(__IO uint16_t uSec);
extern void Delay(__IO uint32_t uCount);
extern void SysTickInit(__IO uint32_t interval);

#ifdef __cplusplus
}
#endif

#endif	/* __SYSTICK_H */
