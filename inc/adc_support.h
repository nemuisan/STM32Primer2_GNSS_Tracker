/********************************************************************************/
/*!
	@file			adc_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2019.09.20
	@brief          Based on Keil's Sample Thanks!

    @section HISTORY
		2010.12.31	V1.00	Restart Here.
		2011.03.10	V2.00	C++ Ready.
		2019.09.20	V3.00	Fixed redundant declaration.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ADC_SUPPORT_H
#define __ADC_SUPPORT_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "stm32f10x.h"

/* Defines */
#define ADC1_DR_Address		((uint32_t)0x4001244C)
#define ADC_NB_SAMPLES    	10
#define ADC_NB_CHANNELS   	5  /* = vbat + 3 touchscreen + temp */
#define ADC_DMA_SIZE     	(ADC_NB_CHANNELS * ADC_NB_SAMPLES)

#define AVG_SLOPE_UV		443
#define V25_MV				1420
/* Voltage (mV) of the STM32 Primer2*/
/* nemui changed another LDO */
#define VDD_VOLTAGE_MV  	2758

/* Externals */
extern __IO uint16_t ADC_RegularConvertedValueTab[ADC_DMA_SIZE];
extern void ADC_DMA_Configuration(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADC_SUPPORT_H */
