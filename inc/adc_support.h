/********************************************************************************/
/*!
	@file			adc_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.03.02
	@brief          Based on Keil's Sample Thanks!

    @section HISTORY
		2010.12.31	V1.00	Restart Here.
		2011.03.10	V2.00	C++ Ready.
		2019.09.20	V3.00	Fixed redundant declaration.
		2023.03.02	V4.00	Added voltage declarations.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ADC_SUPPORT_H
#define ADC_SUPPORT_H 0x0400

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
#define ADC_NB_CHANNELS   	5  /* = vbat + temp + 3-touchscreen */
#define ADC_DMA_SIZE     	(ADC_NB_CHANNELS * ADC_NB_SAMPLES)
#define ADC_VBAT_CH_OFS		0
#define ADC_TEMP_CH_OFS		1
#define ADC_TOUCH_CH1_OFS	2
#define ADC_TOUCH_CH2_OFS	3
#define ADC_TOUCH_CH3_OFS	4


/* Internal temperature sensor unique value */
#define AVG_SLOPE_UV		430		/* Typ:4.3mV/deg */
#define V25_MV				1430	/* Typ:1.43V */

/* Voltage Settings (in mV) of the STM32 Primer2*/
/* Nemui changed L6928D to LP3988IMFX-2.85/NOPB(2.85V output) */
/* Measured real Vdd voltage value */
#define VDD_VOLTAGE_MV  	2850
/* Real-Volt vs ADC-Volt compensation value @3400mV */
#define ADC_VREF_CALIB		955
/* VREF Value with adjastment(Vref = AVcc = Vdd*ADC_VREF_CALIB) */
#define ADC_VREF_MV	 	 	(VDD_VOLTAGE_MV*ADC_VREF_CALIB/1000)

/* Externals */
extern __IO uint16_t ADC_RegularConvertedValueTab[ADC_DMA_SIZE];
extern void ADC_DMA_Configuration(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_SUPPORT_H */
