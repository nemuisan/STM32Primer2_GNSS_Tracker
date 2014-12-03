/********************************************************************************/
/*!
	@file			power.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2014.12.02
	@brief          Power Control and Battery Supervisor on STM32Primer2.

    @section HISTORY
		2009.12.26	V0.02	See Update.txt
		2011.03.10	V2.00	C++ Ready.
		2014.12.02	V3.00	Added WatchdogReset for USB functions.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "hw_config.h"
/* include bitmap */

/* Defines -------------------------------------------------------------------*/
#define LSI_FREQ		40000UL	/* For Independent Watchdog(default:40000Hz) */
#define WDT_INTERVAL	1000UL	/* For Independent Watchdog(default:1000mSec) */

/* Variables -----------------------------------------------------------------*/
__IO uint32_t BatState = BAT_MIDDLE;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
static void ShutKey_Chk(void);
static void ShutVbat_Chk(void);

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  STM32 Primer2 Power managements.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void PWR_Mgn(void)
{
	ShutKey_Chk();
	ShutVbat_Chk();
	/* Reload IWDG counter */
	IWDG_ReloadCounter();
}


/**************************************************************************/
/*! 
    @brief  A Piece of STM32 Primer2 Power managements.
	@param  None.
    @retval None.
*/
/**************************************************************************/
static void ShutKey_Chk(void)
{
	/* execute 1mSec every in Systick Timer */
	static uint32_t shutcounts;

	if(GPIO_ReadInputDataBit(GPIOA, KEY_CT)==1)
		{
			/* Key Pressed sequencery 3000mSec */
			if(++shutcounts > SHUT_TIME)
				{
					/* Reload IWDG counter */
					IWDG_ReloadCounter();
					/* FatFs File Close */
					ShutFileClose();
					/* Power OFF by self */
					PWR_OFF();
					/* Reload IWDG counter eternal */
					for(;;){
						IWDG_ReloadCounter();
					}
				}
		}
	else
		{
			shutcounts = 0;
		}
}


/**************************************************************************/
/*! 
    @brief  A Piece of STM32 Primer2 Power managements.
	@param  None.
    @retval None.
*/
/**************************************************************************/
static void ShutVbat_Chk(void)
{
	/* execute 1mSec every in Systick Timer */
	static uint32_t vbatcounts;
	static uint32_t vbatlow;
	static uint32_t vbatbabyflag=15;
	static uint32_t CurrentVbat;
	
	if(vbatbabyflag)
	{
		CurrentVbat = GetVbat();
		vbatbabyflag--;
	}
	
	/* execute every 1Sec (about 1024mSec) */
	if(++vbatcounts > 1000 ){
		vbatcounts =0;
		CurrentVbat =  ((CurrentVbat * 3) + (GetVbat()) ) / 4;
		
		if(CurrentVbat > NO_BAT_VOLTAGE)
		{
			if(CurrentVbat < MID_BAT_VOLTAGE) 	{BatState = BAT_LOW;}
			else 								{BatState = BAT_MIDDLE;}
			
			if(CurrentVbat < LIPO_LOWER_VOLT)
			{
				if(++vbatlow > LOWER_FILT) 
				{
					ShutFileClose();
					PWR_OFF();
					for(;;);
				}
			}
			else
			{
				vbatlow =0;
			}
		}
		else
		{
				vbatlow =0;
		}
	}
}


/**************************************************************************/
/*! 
    @brief  A Piece of STM32 Primer2 Power managements.
	@param  None.
    @retval Singned int value at milliVolt order.
*/
/**************************************************************************/
int16_t GetVbat()
{
    uint16_t vbat;
	static int16_t VBat; /* signed 16bit int */
    int i;

    vbat = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++)
        {
        vbat += ADC_RegularConvertedValueTab[0 + i*ADC_NB_CHANNELS];
        }
    vbat = vbat / ADC_NB_SAMPLES;

    vbat = vbat & 0xFFF;
    vbat = ( vbat * VDD_VOLTAGE_MV ) / 0x1000;
	
	/* Divider bridge  Vbat <-> 1M -<--|-->- 1M <-> Gnd,@STM32 Primer2. */
    vbat *= 2;

    if ( VBat == -1)
        {
        VBat = vbat;
        }
    else
        {
        VBat = (VBat>>1) + (vbat>>1);
        }
    return VBat;
}

/**************************************************************************/
/*! 
    @brief  This function is reserved (not in use).
	@param  None.
    @retval Degree Celsius.
*/
/**************************************************************************/
double GetTemp()
{
    s32 temp;
    volatile int i;

    // Measure temp
    //    s16 *p=&ADC_ConvertedValue[1];     // intent; point to first of x results from same source - use a short name for it!
    // will not help reduce mains ripple because conversions are SO FAST!!
    // Calculate the mean value
    //    temp = (p[0]+p[1]+p[2]+p[3])/4;    // take avg of burst of 4 temp reads. may only help reject hi freq noise a bit
    // YRT 20081023 : add touchsreen adc channels for Primer2, buffer managed by DMA

    // Calculate the mean value
    temp = 0;
    for ( i = 0; i < ADC_NB_SAMPLES; i++)
        {
        temp += ADC_RegularConvertedValueTab[i];
        }
    temp = temp / ADC_NB_SAMPLES;
    temp = temp & 0xFFF;
    temp = ( temp * VDD_VOLTAGE_MV ) / 0x1000;  		   //finds mV
    temp = (((V25_MV-temp)*100000)/AVG_SLOPE_UV)+25000;    //gives approx temp x 1000 degrees C


    return (double)temp/1000;
}


/**************************************************************************/
/*! 
    @brief  Configures the SHUTDOWN(PC13) on STM32Primer2.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void PWR_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO_PWR clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_PWR, ENABLE);
	
	/* Configure GPIO for PWR as Output push-pull */
	GPIO_InitStructure.GPIO_Pin 	= SHUTDOWN;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_PWR, &GPIO_InitStructure);

	/* IWDG timeout equal to 1000 ms (the timeout may varies due to LSI frequency
	 dispersion) (for USB Functions) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	/* IWDG counter clock: LSI/32 */
	IWDG_SetPrescaler(IWDG_Prescaler_32);
	/* Set counter reload value to obtain 1000ms IWDG TimeOut.
	 Counter Reload Value = 1000ms/IWDG counter clock period
						  = (WDT_INTERTVAL/1000) / (LSI/32)
	*/
	IWDG_SetReload(LSI_FREQ/(32*(1000/WDT_INTERVAL)));
	/* Reload IWDG counter */
	IWDG_ReloadCounter();
}


/* End Of File ---------------------------------------------------------------*/
