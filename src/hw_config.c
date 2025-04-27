/********************************************************************************/
/*!
	@file			hw_config.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2025.04.21
	@brief          Configure Basis System on STM32Primer2.

    @section HISTORY
		2010.12.31	V1.00	Restart Here.
		2011.03.10	V2.00	C++ Ready.
		2022.10.15	V3.00	Fixed cosmetic bugfixes.
		2023.04.21	V4.00	Re-Fixed cosmetic bugfix.
		2023.06.01	V5.00	Added MTK_Command mode at gnss logging.
		2025.04.07	V6.00	Fixed typo comment.
		2025.04.21	V7.00	Re-defined NVIC priority settings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hw_config.h"
/* check header file version for fool proof */
#if HW_CONFIG_H!= 0x0700
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint16_t CmdKey=0;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief	Configures Main system clocks & power.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Set_System(void)
{
	/* SystemInit was already executed in asm startup Since StdPeriph V3.2.0 */
	/* SystemInit(); */

	/* Retrieve SystemClock Frequency */
	SystemCoreClockUpdate();

	/* NVIC configuration */
	NVIC_Configuration();

	/* Configure the LED on STM32Primer2. */
	LED_Configuration();

	/* Configure the KEY-Input on STM32Primer2. */
	KEY_Configuration();

	/* Configure the PWRControl on STM32Primer2. */
	PWR_Configuration();
	
	/* Configure the RTCModule on STM32Primer2. */
	RTC_Configuration();

	/* Configure the ADCControl on STM32Primer2. */
	ADC_DMA_Configuration();
}


/**************************************************************************/
/*! 
    @brief	Configures the LED on STM32Primer2.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void LED_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Reduce Power Comsumption */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
						   RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | 
						   RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_Init(GPIOE, &GPIO_InitStructure);
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
						   RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | 
						   RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, DISABLE);

	/* Enable GPIO_LED clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_LED | RCC_APB2Periph_GPIOB, ENABLE);
	
	/* Configure GPIO for LEDs as Output push-pull */
	GPIO_InitStructure.GPIO_Pin 	= LED_GREEN | LED_RED;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_LED, &GPIO_InitStructure);
	
	/* BackLight LED Control */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_8;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);	/* LED Backlight is Always OFF in GPS Tracker */
	
	/* Init LED */
	LED_GRN_OFF();
	LED_RED_OFF();
}


/**************************************************************************/
/*! 
    @brief	Configures the KEY-Input on STM32Primer2.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void KEY_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable 4 input clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	/* Configure GPIO for 4 input as "Iutput PullDown" */
	GPIO_InitStructure.GPIO_Pin 	= KEY_L | KEY_R | KEY_U | KEY_D;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IPD;
	GPIO_Init(GPIOE, &GPIO_InitStructure);


	/* Enable Center input clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure GPIO for Center (PBUTTON) input as "Input Floating" */
	/* Hardware pulldowned */
	GPIO_InitStructure.GPIO_Pin 	= KEY_CT;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/**************************************************************************/
/*! 
    @brief	Configures Vector Table base location.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
#ifdef  VECT_TAB_RAM
/* vector-offset (TBLOFF) from bottom of SRAM. defined in linker script */
extern uint32_t _isr_vectorsram_offs;
#endif
void NVIC_Configuration(void)
{
    /* 20090429Nemui */
    #ifdef  VECT_TAB_RAM  
      /* Set the Vector Table base location at 0x20000000 + _isr_vectorsram_offs */ 
      NVIC_SetVectorTable(NVIC_VectTab_RAM, (uint32_t)&_isr_vectorsram_offs); 
    #else  /* VECT_TAB_FLASH  */
      /* Set the Vector Table base address at 0x08000000 */
      NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
    #endif
    /* 20090429Nemui */

	/* Configure all the interrupt priority bits to the preempt priority group */
	/* On STM32F1,0~15 priority levels. */
	NVIC_SetPriorityGrouping(0U);

#if 0
#if defined(USE_STM32PRIMER2) || defined(USE_TIME_DISPLAY)
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/* Enable the RTC Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
#endif
}


/**************************************************************************/
/*! 
    @brief	4+1inputs key on STM32Primer2.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void JoyInp_Chk(void)
{
	static uint32_t keycounts=0;
	static uint32_t prev,now;
	
	/* execute every 5mSec */
	if(++keycounts >= 5 ){
		keycounts =0;
		now  = GPIO_ReadInputData(GPIOE) & (KEY_L | KEY_R | KEY_U | KEY_D);
		now |= GPIO_ReadInputData(GPIOA) & (KEY_CT);
		
		prev = (prev ^ now) & now;
		if (prev) CmdKey = prev;
		prev = now;
	}

}


/**************************************************************************/
/*! 
    @brief	Set Available SystemClock to 72MHz for USB Functions.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void SetSysClock72(void)
{
	RCC_ClocksTypeDef RCC_ClockFreq;

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_HSI );

    /* Enable PLL */
    RCC_PLLCmd( DISABLE );

    /* PLLCLK = 12MHz * 6 = 72 MHz */
    RCC_PLLConfig( RCC_PLLSource_HSE_Div1, RCC_PLLMul_6 );

    /* Enable PLL */
    RCC_PLLCmd( ENABLE );

    /* Wait till PLL is ready */
    while( RCC_GetFlagStatus( RCC_FLAG_PLLRDY ) == RESET )
        { ; }

    /* Enable Prefetch Buffer */
    FLASH->ACR |= FLASH_ACR_PRFTBE;

    /* Flash 2 wait state */
    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while( RCC_GetSYSCLKSource() != 0x08 )
        { ; }

    /* This function fills a RCC_ClocksTypeDef structure with the current frequencies
    of different on chip clocks (for debug purpose) */
    RCC_GetClocksFreq( &RCC_ClockFreq );
}

/**************************************************************************/
/*! 
    @brief	USB disconnect pin configuration.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USB_Disconnect_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable USB_DISCONNECT GPIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

	/* USB_DISCONNECT_PIN used as USB pull-up */
	GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
}

/**************************************************************************/
/*! 
    @brief	Configures USB Clock input (48MHz).
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Set_USBClock(void)
{
	/* Select USBCLK source */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);

	/* Enable the USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}


/**************************************************************************/
/*! 
    @brief	Software Connection/Disconnection of USB Cable.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USB_Cable_Config (FunctionalState NewState)
{

	if (NewState != DISABLE)
	{
		GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	}
	else
	{
		GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
	}
  
}


/**************************************************************************/
/*! 
    @brief	Power-off system clocks and power while entering suspend mode.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Enter_LowPowerMode(void)
{
	/* Set the device state to suspend */
	bDeviceState = SUSPENDED;
}


/**************************************************************************/
/*! 
    @brief	Restores system clocks and power while exiting suspend mode.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Leave_LowPowerMode(void)
{
	DEVICE_INFO *pInfo = &Device_Info;

	/* Set the device state to the correct state */
	if (pInfo->Current_Configuration != 0)
	{
		/* Device configured */
		bDeviceState = CONFIGURED;
	}
	else
	{
		bDeviceState = ATTACHED;
	}

}

/**************************************************************************/
/*! 
    @brief	Convert Hex 32Bits value into char.
	@param	value: Target valiable to change 32Bit HEX.
	@param	pbuf:  Receive Buffer for 32Bit HEX Stings.
	@param	value: Receive Buffer length.
    @retval	None.
*/
/**************************************************************************/
static void IntToUnicode (uint32_t value , uint8_t *pbuf , uint8_t len)
{
	uint8_t idx = 0;

	for( idx = 0 ; idx < len ; idx ++)
	{
		if( ((value >> 28)) < 0xA )
		{
			pbuf[ 2* idx] = (value >> 28) + '0';
		}
		else
		{
			pbuf[2* idx] = (value >> 28) + 'A' - 10; 
		}

		value = value << 4;
		pbuf[ 2* idx + 1] = 0;
	}
}

/**************************************************************************/
/*! 
    @brief	Create the serial number for StringDescriptor.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Get_SerialNum(uint8_t* string0,uint8_t* string1)
{
	uint32_t Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(__IO uint32_t*)(0x1FFFF7E8);
	Device_Serial1 = *(__IO uint32_t*)(0x1FFFF7EC);
	Device_Serial2 = *(__IO uint32_t*)(0x1FFFF7F0);

	Device_Serial0 += Device_Serial2;

	if (Device_Serial0 != 0)
	{
		IntToUnicode (Device_Serial0, string0, 8);
		IntToUnicode (Device_Serial1, string1, 4);
	}
}

/* End Of File ---------------------------------------------------------------*/
