/********************************************************************************/
/*!
	@file			usb_conf.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.03.14
	@brief          USB Endpoint and Misc Function Wrappers.

    @section HISTORY
		2012.01.30	V1.00	Start Here
		2014.12.17	V2.00	Adopted GCC4.9.x.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.14	V4.00	Added external declaration.

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
