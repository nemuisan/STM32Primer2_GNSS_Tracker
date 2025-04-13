/********************************************************************************/
/*!
	@file			systick.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2025.04.03
	@brief          delay mSec-order routine using systick timer			@n
					delay uSec-order routine using TIM3~5;

    @section HISTORY
		2010.03.05	V1.00	Restart Here.
		2011.01.20	V2.00	Using Timer5 for _delay_us(); instead of __NOP();.
		2011.03.10	V3.00	C++ Ready.
		2014.04.25  V4.00	Fixed Timer5 Clock definition fot _delay_us();
		2017.07.31  V5.00	Fixed portability for uSec timer;
		2023.04.21	V6.00	Fixed cosmetic bugfix.
		2025.04.03	V7.00	Fixed retrieve current SystemCoreClock on SysTickInit();

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "systick.h"
/* check header file version for fool proof */
#if SYSTICK_H!= 0x0700
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint32_t TimingDelay;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief	SysTickTimer Initialize(every 1mSec)
	@param	interval: Set Systick timer interval ratio.
    @retval	none
*/
/**************************************************************************/
void SysTickInit(__IO uint32_t interval)
{
	/* Making MilliSecond-Order Timer */
	/* Select Clock Source  */
	SystemCoreClockUpdate();
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(SystemCoreClock / interval))
	{ 
		/* Capture error */ 
		while (1);
	}
	
	/* Making MicroSecond-Order Timer (upto 65535 usec) */
	/* Enable timer clock */
	USEC_TIMx_CLKEN();

	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Prescaler 	= ((SystemCoreClock)/USEC_INTERVAL) - 1;
	TIM_TimeBaseStructure.TIM_Period 		= UINT16_MAX; 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode 	= TIM_CounterMode_Up;
	TIM_TimeBaseInit(USEC_TIMx, &TIM_TimeBaseStructure);

	/* Enable counter */
	TIM_Cmd(USEC_TIMx, ENABLE);
}


/**************************************************************************/
/*! 
    @brief	Delay Millisecond Order (upto 16777214mSec!)
	@param	mSec: 24-bit value due to systick downcount timer
    @retval	none
*/
/**************************************************************************/
void _delay_ms(__IO uint32_t mSec)
{ 
	TimingDelay = mSec;

	while(TimingDelay != 0);
}

/**************************************************************************/
/*! 
    @brief	Delay Microsecond Order (upto 65534uSec!)
	@param	uSec 
    @retval	none
*/
/**************************************************************************/
void _delay_us(__IO uint16_t uSec)
{

#if 1
	if(uSec){
		USEC_TIMx_CNT = 0;
		/* use 16 bit count wrap around */
		while((uint16_t)(USEC_TIMx_CNT) <= uSec);
	}

#else
	/* This is the stupid method */
	while(uSec--){ 
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					}
#endif
}

/**************************************************************************/
/*! 
    @brief	Delay Stupid.
	@param	nCount
    @retval	none
*/
/**************************************************************************/
void Delay(__IO uint32_t nCount)
{
	while(nCount--){ 
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					}
}

/* End Of File ---------------------------------------------------------------*/
