/********************************************************************************/
/*!
	@file			msc_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2014.04.20
	@brief          Interface of USB-MassStorageClass.

    @section HISTORY
		2011.07.06	V1.00	Start Here.
		2012.01.30	V2.00	Added Consideration CoOperate with CDC Function .
		2014.04.20	V3.00	Fixed Suitable Interruption level.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "msc_support.h"
/* check header file version for fool proof */
#if __MSC_SUPPORT_H!= 0x0300
#error "header file version is not correspond!"
#endif

#include "display_if.h"
#include "display_if_support.h"

/* Defines -------------------------------------------------------------------*/
/* FONTX */
#define ANKFONT		Mplus_Ank

/* Variables -----------------------------------------------------------------*/
extern FontX_Ank   	ANKFONT;

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
	/* Retrive SystemClock Frequency */
	SystemCoreClockUpdate();

	/* Init Display Driver and FONTX Driver */
	Display_init_if();
	InitFont_Ank(&ANKFONT,font_table_ank);
	ChangeCurrentAnk(&ANKFONT);

	/* Diaplay MSC mode message */
	Display_clear_if();
	Display_Puts_If(0,0,(uint8_t*)"Start Mass Storage",OPAQUE);

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
