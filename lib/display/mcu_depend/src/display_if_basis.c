/********************************************************************************/
/*!
	@file			display_if_basis.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2011.03.10
	@brief          Interface of Display Device Basics for STM32 Primer2.			@n
					"MCU Depend Layer"

    @section HISTORY
		2010.07.02	V1.00	Restart Here.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Changed Some.
		2011.03.10	V4.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "display_if_basis.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Driver Lowest Layer Settings.
*/
/**************************************************************************/
static void FSMC_Conf(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;

	/*-- FSMC Configuration ------------------------------------------------------*/
	p.FSMC_AddressSetupTime 		= 2;
	p.FSMC_AddressHoldTime 			= 2;
	p.FSMC_DataSetupTime 			= 2;
	p.FSMC_BusTurnAroundDuration 	= 5;
	p.FSMC_CLKDivision 				= 5;
	p.FSMC_DataLatency 				= 5;
	p.FSMC_AccessMode 				= FSMC_AccessMode_A;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;        /* MUST be 16b*/
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait= FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;  /* cf RM p363 + p384*/
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

	/* Enable FSMC Bank1_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);

}


/**************************************************************************/
/*! 
    Display Driver Lowest Layer Settings.
*/
/**************************************************************************/
static void GPIO_Conf(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	
	/* Enable FSMC, GPIOD, GPIOE, GPIOF, GPIOG and AFIO clocks */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
						   RCC_APB2Periph_GPIOF | RCC_APB2Periph_GPIOG |
						   RCC_APB2Periph_AFIO, ENABLE);

	/* Enable GPIOC clock (BackLight Control)*/
    /* RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE ); */

	/*-- GPIO Configuration ------------------------------------------------------*/
	/* SRAM Data lines configuration */
	GPIO_InitStructure.GPIO_Pin = DATA_PINS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DISPLAY_PORT_DATA, &GPIO_InitStructure); 

	/* NOE, NWE and NE1 configuration */  
	GPIO_InitStructure.GPIO_Pin 	= CTRL_RD;
	GPIO_Init(DISPLAY_PORT_RD, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin 	= CTRL_WR;
	GPIO_Init(DISPLAY_PORT_WR, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin 	= CTRL_DC;
	GPIO_Init(DISPLAY_PORT_DC, &GPIO_InitStructure);

	/* Reset : configured as regular GPIO, is not FSMC-controlled */
	GPIO_InitStructure.GPIO_Pin 	= CTRL_RES;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_Out_PP;
	GPIO_Init(DISPLAY_PORT_RES, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin 	= CTRL_CS;
	GPIO_Init(DISPLAY_PORT_CS, &GPIO_InitStructure);
	
	/* BackLight LED Control */
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_8;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_8);	/* LED Backlight is Always ON in MSC/CDC mode */
}

/**************************************************************************/
/*! 
    Display Driver Lowest Layer Settings.
*/
/**************************************************************************/
void Display_IoInit_If()
{
	GPIO_Conf();
	FSMC_Conf();
}


/* End Of File ---------------------------------------------------------------*/
