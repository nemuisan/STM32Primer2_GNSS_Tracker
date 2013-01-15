/********************************************************************************/
/*!
	@file			systick.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.03.10
	@brief          delay mSec-order routine using systick timer			@n
					delay uSec-order routine using Timer5;

    @section HISTORY
		2010.03.05	V1.00	Restart Here.
		2011.01.20	V2.00	Using Timer5 for _delay_us(); instead of __NOP();.
		2011.03.10	V3.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "systick.h"

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
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );

	/* Setup SysTick Timer for 1 msec interrupts  */
	if (SysTick_Config(SystemCoreClock / interval))
	{ 
		/* Capture error */ 
		while (1);
	}
	
	/* Making MicroSecond-Order Timer */
	/* Enable timer clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	/* Time base configuration */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
	TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / USEC_INTERVAL) - 1;
	TIM_TimeBaseStructure.TIM_Period = UINT16_MAX; 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	/* Enable counter */
	TIM_Cmd(TIM5, ENABLE);
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
	uint16_t start = TIM5->CNT;
	/* use 16 bit count wrap around */
	while((uint16_t)(TIM5->CNT - start) <= uSec);
	
	/* This is the stupid method */
	/*
	while(uSec--){ 
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					__NOP();
					}
	*/
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
