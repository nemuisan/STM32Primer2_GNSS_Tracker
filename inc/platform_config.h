/********************************************************************************/
/*!
	@file			platform_config.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.03.10
	@brief          Device Dependent Section.

    @section HISTORY
		2010.12.31	V1.00	ReStart Here.
		2011.01.20	V2.00	Add EtherPod +TFT-LCD Shield's Porting..
		2011.03.10	V3.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported types ------------------------------------------------------------*/


#if !defined (USE_STM32PRIMER2)
 #error "Define STM32Primer2 Configurations!!"
#endif

/* Define the STM32Primer2 Hardware*/
/* LED Definitions */
#define GPIO_LED               		(GPIOE)
#define RCC_APB2Periph_GPIO_LED		(RCC_APB2Periph_GPIOE)
#define LED_GREEN       			(GPIO_Pin_0) /* LED0 */
#define LED_RED       				(GPIO_Pin_1) /* LED1 */

/* PWR Definitions */
#define GPIO_PWR              		(GPIOC)
#define RCC_APB2Periph_GPIO_PWR		(RCC_APB2Periph_GPIOC)
#define SHUTDOWN	       			(GPIO_Pin_13)

/* 4+1 Inputs KEY Definitions */
/* 4input (Up,Down,Right,Left) uses GPIOE */
#define KEY_U		       			(GPIO_Pin_3)
#define KEY_D       				(GPIO_Pin_4)
#define KEY_R		       			(GPIO_Pin_5)
#define KEY_L       				(GPIO_Pin_6)
/* 1input (Center) uses GPIOA */
#define KEY_CT       				(GPIO_Pin_8)

/* USB Connection */
#define USB_DISCONNECT                    (GPIOD)  
#define USB_DISCONNECT_PIN                (GPIO_Pin_3)
#define RCC_APB2Periph_GPIO_DISCONNECT    (RCC_APB2Periph_GPIOD)
/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_CONFIG_H */
