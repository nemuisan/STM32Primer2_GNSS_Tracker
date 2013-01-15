/********************************************************************************/
/*!
	@file			usb_conf.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.30
	@brief          USB Endpoint and Misc Function Wrappers.

    @section HISTORY
		2012.01.30	V1.00	Start Here

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usb_core.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
void (*xEP1_IN_Callback)(void);
void (*xEP2_OUT_Callback)(void);
void (*xEP3_OUT_Callback)(void);
void (*xSOF_Callback)(void);

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
