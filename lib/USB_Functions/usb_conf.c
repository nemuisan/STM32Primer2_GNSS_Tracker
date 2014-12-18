/********************************************************************************/
/*!
	@file			usb_conf.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2014.12.17
	@brief          USB Endpoint and Misc Function Wrappers.

    @section HISTORY
		2012.01.30	V1.00	Start Here
		2014.12.17	V2.00	Adopted GCC4.9.x.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usb_core.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
void (* volatile xEP1_IN_Callback)(void)  = NULL;
void (* volatile xEP2_OUT_Callback)(void) = NULL;
void (* volatile xEP3_OUT_Callback)(void) = NULL;
void (* volatile xSOF_Callback)(void)     = NULL;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
void EP1_IN_Callback(void)
{
	xEP1_IN_Callback();
}

void EP3_OUT_Callback(void)
{
	xEP3_OUT_Callback();
}

void EP2_OUT_Callback(void)
{
	xEP2_OUT_Callback();
}

void SOF_Callback(void)
{
	xSOF_Callback();
}

/* End Of File ---------------------------------------------------------------*/
