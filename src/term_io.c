/********************************************************************************/
/*!
	@file			term_io.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2011.03.10
	@brief          Interface of Chan's Universal string handler For STM32 uC. @n
					Based on Chan's FatFs Test Terminal Thanks!

    @section HISTORY
		2010.06.01	V1.00	Start Here.
		2010.08.28	V1.10	Add Etherpod+TFT-LCD Shield's Key.
		2010.12.31	V2.00	Removed uart includion.
		2011.03.10	V3.00	C++ Ready.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "term_io.h"
/* check header file version for fool proof */
#if __TERM_IO_H!= 0x0300
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief	Get Raw InputData and Return CommandData.
	@param	None
    @retval	CommandData.
*/
/**************************************************************************/
char xgetc_n (void)
{
	int c;
#if  defined (USE_ETHERPOD) 
	static int flip=0;
#endif
	c = 0;

	c = getch();

	if (c!=false){}
		
	else
	{
#if defined (USE_STM32PRIMER2)
		c = CmdKey;
		CmdKey = 0;
		
		switch (c) {

		case KEY_R:
			c = BTN_OK;
			break;
		case KEY_L:
			c = BTN_ESC;
			break;
		case KEY_U:
			c = BTN_UP;
			break;
		case KEY_D:
			c = BTN_DOWN;
			break;

		default:
			c = 0;
		}
#elif  defined (USE_ETHERPOD) 
		c = CmdKey;
		CmdKey = 0;
		
		switch (c) {

		case KEY_RT:
			c = BTN_OK;
			break;
		case KEY_LT:
			c = BTN_ESC;
			break;
		case KEY_UP:
			c = BTN_UP;
			break;
		case KEY_DN:
			c = BTN_DOWN;
			break;
		case KEY_CT:
			if ( flip ) GPIO_LED->BSRR = GPIO_Pin_LED1;
			else	    GPIO_LED->BRR  = GPIO_Pin_LED1;
			flip = !flip;
			break;
		default:
			c = 0;
		}
#elif defined (USE_REDBULL)
		c = CmdKey;
		CmdKey = 0;
		
		switch (c) {

		case KEY_R:
			c = BTN_OK;
			break;
		case KEY_L:
			c = BTN_ESC;
			break;
		case KEY_U:
			c = BTN_UP;
			break;
		case KEY_D:
			c = BTN_DOWN;
			break;

		default:
			c = 0;
		}
#endif

#if	defined(USE_ADS7843)
	if(scaned_tc){
		scaned_tc =0;
		TC_ScanPen();
		if (TOUCH_MIGI)		  {if(tc_stat==TC_STAT_DOWN) c = BTN_OK;}
		else if (TOUCH_HIDARI){if(tc_stat==TC_STAT_DOWN) c = BTN_ESC;}
		else if (TOUCH_UE)    {if((tc_stat==TC_STAT_DOWN || tc_stat==TC_STAT_HOLD)) c = BTN_UP;}
		else if (TOUCH_SHITA) {if((tc_stat==TC_STAT_DOWN || tc_stat==TC_STAT_HOLD)) c = BTN_DOWN;}
		else c = 0;
		}
#endif
	}

	return c;
}


/**************************************************************************/
/*! 
    @brief	Return CommandData Wrapper.
	@param	None
    @retval	CommandData.
*/
/**************************************************************************/
unsigned char xgetc (void)
{
	unsigned char c;

	do {
#if defined(USE_STM32PRIMER2) || defined(USE_TIME_DISPLAY)
/*		ts_rtc();*/
#endif
		c = xgetc_n();
	} while (!c);
	return c;
	
}

/* End Of File ---------------------------------------------------------------*/
