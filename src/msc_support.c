/********************************************************************************/
/*!
	@file			msc_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2023.03.23
	@brief          Interface of USB-MassStorageClass.

    @section HISTORY
		2011.07.06	V1.00	Start Here.
		2012.01.30	V2.00	Added Consideration Co-Operate with CDC Function .
		2014.04.20	V3.00	Fixed Suitable Interruption level.
		2014.07.16	V4.00	Reset Systick to suitable Frequency.
		2019.09.20	V5.00	Fixed redundant declaration.
		2020.05.30	V6.00	Display system version string.
		2023.03.23	V7.00	Added MAL_Init() successful check.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "msc_support.h"
/* check header file version for fool proof */
#if MSC_SUPPORT_H!= 0x0700
#error "header file version is not correspond!"
#endif

#include "display_if.h"
#include "display_if_support.h"
#include "font_if.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
extern __IO long StableCount;
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
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**************************************************************************/
/*!
    Main Mass Storage Class Task Routine.
*/
/**************************************************************************/
void msc_task(void)
{
	/* Available USB Clock Frequency */
	Set72();
	/* Retrive SystemClock Frequency and reset SysTick */
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
		USB_Cable_Config(DISABLE); /* fool ploof */

		Set_USBClock();
		USB_Interrupts_Config();
		USB_Init();
		USB_Cable_Config(ENABLE);
		while (bDeviceState != CONFIGURED);

#if 0 /* MSC Debug */
		Display_Puts_If(0,2*CurrentAnkDat->Y_Size,(uint8_t*)"Connected!",TRANSPARENT);
		while (StableCount > 0);
		Display_Puts_If(0,3*CurrentAnkDat->Y_Size,(uint8_t*)"Stabled!",TRANSPARENT);
#endif
		while (1){__WFI();}
	}
	else {
		Display_Puts_If(0,2*CurrentAnkDat->Y_Size,(uint8_t*)"SD Card Init Failed!",TRANSPARENT);
		Display_Puts_If(0,3*CurrentAnkDat->Y_Size,(uint8_t*)"Press Center Key to Power OFF!",TRANSPARENT);
		while (1){__WFI();}
	}

}

/* End Of File ---------------------------------------------------------------*/
