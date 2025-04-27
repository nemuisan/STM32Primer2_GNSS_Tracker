/********************************************************************************/
/*!
	@file			msc_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        10.00
    @date           2025.04.21
	@brief          Interface of USB-MassStorageClass.

    @section HISTORY
		2011.07.06	V1.00	Start Here.
		2012.01.30	V2.00	Added Consideration Co-Operate with CDC Function .
		2014.04.20	V3.00	Fixed Suitable Interruption level.
		2014.07.16	V4.00	Reset Systick to suitable Frequency.
		2019.09.20	V5.00	Fixed redundant declaration.
		2020.05.30	V6.00	Display system version string.
		2023.03.23	V7.00	Added MAL_Init() successful check.
		2023.12.19  V8.00	Improved watchdog handlings.
		2025.04.08	V9.00	Changed minor function name.
		2025.04.21 V10.00	Re-defined NVIC priority settings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "msc_support.h"
/* check header file version for fool proof */
#if MSC_SUPPORT_H!= 0x1000
#error "header file version is not correspond!"
#endif

#include "display_if.h"
#include "display_if_support.h"
#include "font_if.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*!
    @brief	Configures the USB interrupts.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
static void USB_Interrupts_Config(void)
{
	/* Enable USB_LP Interrupt */
	NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn,3);
	NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
	
	/* Enable USB_HP Interrupt */
	NVIC_SetPriority(USB_HP_CAN1_TX_IRQn,2);
	NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
}

/**************************************************************************/
/*!
    Main Mass Storage Class Task Routine.
*/
/**************************************************************************/
void msc_task(void)
{
	/* Set Available SystemClock to 72MHz for USB Functions */
	SetSysClock72();
	
	/* Retrieve SystemClock Frequency and reset SysTick */
	SysTickInit(INTERVAL);

	/* Init Display Driver and FONTX Driver */
	Display_init_if();
	InitFont_Ank(&ANKFONT,font_table_ank);
	ChangeCurrentAnk(&ANKFONT);

	/* Diaplay MSC mode message */
	Display_clear_if();
	Display_Puts_If(0,1*CurrentAnkDat->Y_Size,(uint8_t*)("System Version:"APP_VERSION),TRANSPARENT);
	
	/* USB-MSC Configurations */
	if(MAL_Init(LUN_SDCARD) == MAL_OK) {
		Display_Puts_If(0,0,(uint8_t*)"Start Mass Storage",TRANSPARENT);
		
		/* USB MSC Setting */
		USB_Disconnect_Config();
		USB_Cable_Config(DISABLE); /* Fool ploof */
		
		Set_USBClock();
		USB_Interrupts_Config();
		USB_Init();
		USB_Cable_Config(ENABLE);
		
		/* Wait CONFIGURED state */
		while (bDeviceState != CONFIGURED)
		{
			WDT_Reset();
		}
		
		/* Main loop */
		while (1){
			__WFI();
			WDT_Reset();
		}
	}
	else {
		Display_Puts_If(0,2*CurrentAnkDat->Y_Size,(uint8_t*)"SDCard/MMC Init Failed!",TRANSPARENT);
		Display_Puts_If(0,3*CurrentAnkDat->Y_Size,(uint8_t*)"Press Center Key to Power OFF!",TRANSPARENT);
		while (1){
			__WFI();
			WDT_Reset();
		}
	}

}

/* End Of File ---------------------------------------------------------------*/
