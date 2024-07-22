/********************************************************************************/
/*!
	@file			pwr_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2024.07.18
	@brief          Power Control and Battery Supervisor on STM32Primer2.

    @section HISTORY
		2009.12.26	V0.02	See Update.txt
		2011.03.10	V2.00	C++ Ready.
		2014.12.02	V3.00	Added WatchdogReset for USB functions.
		2014.12.22	V3.01	Enforce Watchdog handlings.
		2022.10.15	V4.00	Changed power-handlings,some codes and filename.
		2023.03.08	V5.00	Fixed lipo battery lower voltage limit.
		2023.12.19	V6.00	Improved watchdog handlings.
		2024.07.18	V7.00	Fixed empty argument.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "pwr_support.h"
/* check header file version for fool proof */
#if PWR_SUPPORT_H!= 0x0700
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint32_t BatState = BAT_MIDDLE;
__IO uint32_t WdtState = 0;

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
}

/**************************************************************************/
/*! 
    @brief  STM32 Primer2 Watchdog managements.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void WDT_Reset(void)
{
	if(WdtState == 1){
		WdtState =0;
		/* Reload IWDG counter */
		IWDG_ReloadCounter();
	}
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
		
		 /* Bad battery or USB powered only detection */
		if(CurrentVbat > NO_BAT_VOLTAGE)
		{
			if(CurrentVbat < MID_BAT_VOLTAGE) 	{BatState = BAT_LOW;}
			else 								{BatState = BAT_MIDDLE;}
			
			if(CurrentVbat < LOWER_BAT_VOLTAGE)
			{
				if(++vbatlow > LOWER_FILT_TIME) 
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
				vbatlow =0;
			}
		}
		else /* In case of USB Powered only */
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
    uint32_t vbat;
	static int16_t VBat=-1; /* signed 16bit int static */

	/* Averaging */
    vbat = 0;
    for (int i=0; i<ADC_NB_SAMPLES; i++)
        {
			vbat += ADC_RegularConvertedValueTab[ADC_VBAT_CH_OFS + i*ADC_NB_CHANNELS];
        }
    vbat = vbat / ADC_NB_SAMPLES;

    vbat = vbat & 0xFFF;						/* clip12bit */
	
	/* Divider bridge  Vbat <-> 1M -<--|-->- 1M <-> Gnd,@STM32 Primer2. */
    vbat = ( vbat * ADC_VREF_MV ) / (4096/2);	/* finds mV,divide by 4096(12bit) and *2(due to divide by2) */

	/* Baby flag check */
    if (VBat == -1)  VBat = (int16_t)vbat;			
    else             VBat = (VBat>>1) + (vbat>>1);

    return VBat;
}

/**************************************************************************/
/*! 
    @brief  This function is reserved (not in use).
	@param  None.
    @retval Degree Celsius.
*/
/**************************************************************************/
double GetTemp(void)
{
    s32 temp;

    // Measure temp
    //  s16 *p=&ADC_ConvertedValue[1];
    // will not help reduce mains ripple because conversions are SO FAST!!
    // Calculate the mean value
	// take avg of burst of 4 temp reads. may only help reject hi freq noise a bit
    //  temp = (p[0]+p[1]+p[2]+p[3])/4;
    // YRT 20081023 : add touchsreen adc channels for Primer2, buffer managed by DMA

    // Calculate the mean value
    temp = 0;
    for (int i=0; i<ADC_NB_SAMPLES; i++)
        {
			temp += ADC_RegularConvertedValueTab[ADC_TEMP_CH_OFS + i*ADC_NB_CHANNELS];
        }
    temp = temp / ADC_NB_SAMPLES;
    temp = temp & 0xFFF;
    temp = ( temp * ADC_VREF_MV ) / 0x1000;  		   //finds mV
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

	/* Setting IWDG timeout equal (for USB Functions) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	/* IWDG counter clock: LSI/64 */
	IWDG_SetPrescaler(IWDG_Prescaler_64);
	/* Set counter reload value to obtain 1000ms IWDG TimeOut.
	 MAXIMUM TIMEOUT is 6553.6mSec@/64(3276.8mSec@/32)
	*/
#if(((WDT_INTERVAL*LSI_FREQ)/(64*1000)) > 0xFFF)
 #error "OUT OF SETTINMG FOR IWDG TIMEOUT! upto 6553.5mSec!"
#endif
	IWDG_SetReload((WDT_INTERVAL*LSI_FREQ)/(64*1000));
	/* Reload IWDG counter */
	IWDG_ReloadCounter();
}

/* End Of File ---------------------------------------------------------------*/
