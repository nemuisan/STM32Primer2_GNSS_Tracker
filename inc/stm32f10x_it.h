/********************************************************************************/
/*!
	@file			stm32f10x_it.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.03.10
	@brief          Cortex-M3 Processor Exceptions Handlers.				@n
					And STM32F10x Peripherals Interrupt Handlers.			@n
					Device Dependent Section.

    @section HISTORY
		2010.03.17	V1.00	Restart Here.
		2011.01.20	V2.00	Rewrite Doxygen Style.
		2011.03.10	V3.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __STM32F10x_IT_H
#define __STM32F10x_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifndef STM32F10X_CL
void USB_LP_CAN1_RX0_IRQHandler(void);
#endif /* STM32F10X_CL */

#if defined (USE_STM3210B_EVAL) || defined (USE_STM3210E_EVAL)
void USART1_IRQHandler(void);
#endif /* USE_STM3210B_EVAL or USE_STM3210E_EVAL */

#if defined (USE_STM3210B_EVAL) || defined (USE_ETHERPOD)      || defined (USE_STM32PRIMER2) || \
	defined (USE_GOLDBULL)      || defined (USE_CQ_STARM_COMP) || defined (USE_STM3210C_EVAL)
void USART2_IRQHandler(void);
#endif /* USE_CQ_STARM_COMP(STM32F107VCT) and EtherPod and STM32Primer2 */

#ifdef STM32F10X_CL
void OTG_FS_IRQHandler(void);
#endif /* STM32F10X_CL */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10x_IT_H */
