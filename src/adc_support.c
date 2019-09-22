/********************************************************************************/
/*!
	@file			adc_support.c
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

/* Includes ------------------------------------------------------------------*/
#include "adc_support.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;

__IO uint16_t ADC_RegularConvertedValueTab[ADC_DMA_SIZE];

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    DMA Driving A/D Converter Initialize
*/
/**************************************************************************/
void ADC_DMA_Configuration(void){

    /* Clocks for ADC */
    /* Enable DMA clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_DMA1, ENABLE );
    /* GPIO Speed valid for all further GPIO configurations*/
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* Configure PC.01 to PC.03 (ADC Channel10-13) as analog input ---------------*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
 
     /* Enable ADC1 clock */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE );
	
  
     /* DMA Channel 1 Configuration */
    DMA_DeInit( DMA1_Channel1 );

    DMA_InitStructure.DMA_PeripheralBaseAddr  = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr      = (uint32_t)ADC_RegularConvertedValueTab;
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize          = ADC_DMA_SIZE;
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority            = DMA_Priority_Low;
    DMA_InitStructure.DMA_M2M                 = DMA_M2M_Disable;
  
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
    /* Enable DMA Channel 1 */
    DMA_Cmd( DMA1_Channel1, ENABLE );

    /* ADC1 Configuration ------------------------------------------------------*/
    ADC_InitStructure.ADC_Mode                = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode        = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode  = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv    = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign           = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel        = ADC_NB_CHANNELS; 

    ADC_Init( ADC1, &ADC_InitStructure );

	/* Internal TempSence */
    ADC_TempSensorVrefintCmd(ENABLE); /*%%*/

    /* ADC1 Regular Channel 9 Configuration = Vbatt */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_9,  1, ADC_SampleTime_239Cycles5);

    /* ADC1 Regular Channel 16 internal temperature */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_16, 2, ADC_SampleTime_55Cycles5 ); 

    /* ADC1 Regular Channel 10 to 13 Configuration = touchscreen */
    ADC_RegularChannelConfig( ADC1, ADC_Channel_10, 3, ADC_SampleTime_239Cycles5 ); 
    ADC_RegularChannelConfig( ADC1, ADC_Channel_12, 4, ADC_SampleTime_239Cycles5 ); 
    ADC_RegularChannelConfig( ADC1, ADC_Channel_13, 5, ADC_SampleTime_239Cycles5 );

 
    /* Enable ADC1 external trigger conversion */
    ADC_ExternalTrigConvCmd( ADC1, ENABLE );  /* <==== moved above cal and above enable in circleOS 1.8(a)*/

    /* Enable ADC1  */
    ADC_Cmd(ADC1, ENABLE);

    /* Enable Vrefint channel 17  Calibrate Internal tempsence */
    ADC_TempSensorVrefintCmd(ENABLE);

    /* Enable ADC1 reset calibaration register */   
    ADC_ResetCalibration(ADC1);

    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));

    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));
 
  
    /* Enable ADC1's DMA interface */
    ADC_DMACmd( ADC1, ENABLE );

    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConvCmd( ADC1, ENABLE );
  
}


/* End Of File ---------------------------------------------------------------*/
