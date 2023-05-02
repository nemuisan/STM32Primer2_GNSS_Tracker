/********************************************************************************/
/*!
	@file			stm32f10x_it.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.04.21
	@brief          Cortex-M3 Processor Exceptions Handlers.				@n
					And STM32F10x Peripherals Interrupt Handlers.			@n
					Device Dependent Section.

    @section HISTORY
		2010.03.17	V1.00	Restart Here.
		2011.01.20	V2.00	Rewrite Doxygen Style.
		2011.03.10	V3.00	C++ Ready.
		2014.01.23	V4.00	Removed retired STM32F10X_CL Codes.
		2023.04.21	V5.00	Fixed cosmetic bugfix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "hw_config.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**************************************************************************/
/*! 
    @brief	Handles NMI exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void NMI_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles Hard Fault exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles Memory Manage exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles Bus Fault exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles Usage Fault exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles SVCall exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void SVC_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles Debug Monitor exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void DebugMon_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles PendSVC exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void PendSV_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles SysTick Handler.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void SysTick_Handler(void)
{
	/* used for delay routine */
	TimingDelay_Decrement();
	
	/* used for power managements */
	PWR_Mgn();
	
	/* GPS Logger Mode Special */
	if(TaskStat == GNSS_LOGGING) ChkAckLimit();
	
	/* key inputs */
	/* JoyInp_Chk(); */
	/* FatFs relation inputs */
	/*ff_support_timerproc();*/
}



/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**************************************************************************/
/*! 
    @brief	Handles USB High Priority or CAN TX interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
	CTR_HP();
}


/**************************************************************************/
/*! 
    @brief	Handles USB Low Priority or CAN TX interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	USB_Istr();
}


/**************************************************************************/
/*! 
    @brief	Handles RealTimeClock interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void RTC_IRQHandler(void)
{
#if 0
#if defined(USE_STM32PRIMER2) || defined(USE_TIME_DISPLAY)
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
		/* Clear the RTC Second interrupt */
		RTC_ClearITPendingBit(RTC_IT_SEC);

		/* Enable time update */
		TimeDisplay = 1;

		/* Wait until last write operation on RTC registers has finished */
		/* RTC_WaitForLastTask(); */
	}
#endif
#endif
}


/* End Of File ---------------------------------------------------------------*/
