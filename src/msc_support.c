/********************************************************************************/
/*!
	@file			msc_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2020.05.30
	@brief          Interface of USB-MassStorageClass.

    @section HISTORY
		2011.07.06	V1.00	Start Here.
		2012.01.30	V2.00	Added Consideration CoOperate with CDC Function .
		2014.04.20	V3.00	Fixed Suitable Interruption level.
		2014.07.16	V4.00	Reset Systick to Suitable Frequency.
		2019.09.20	V5.00	Fixed redundant declaration.
		2020.05.30	V6.00	Display system version string.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "msc_support.h"
/* check header file version for fool proof */
#if __MSC_SUPPORT_H!= 0x0600
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
	Display_Puts_If(0,0,(uint8_t*)"Start Mass Storage",TRANSPARENT);
	Display_Puts_If(0,1*CurrentAnkDat->Y_Size,(uint8_t*)("System Version:"APP_VERSION),TRANSPARENT);

	/* USB-MSC Configurations */
  	USB_Disconnect_Config();
	USB_Cable_Config(DISABLE); /* fool ploof */
	MAL_Init(0);
	Set_USBClock();
	USB_Interrupts_Config();
	USB_Init();
	USB_Cable_Config(ENABLE);
	while (bDeviceState != CONFIGURED);

	while (1){}

}

/* End Of File ---------------------------------------------------------------*/
