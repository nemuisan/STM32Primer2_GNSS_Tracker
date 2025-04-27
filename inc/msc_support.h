/********************************************************************************/
/*!
	@file			msc_support.h
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
#ifndef MSC_SUPPORT_H
#define MSC_SUPPORT_H 0x1000

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include <string.h> /* memset et al*/

/* Miscs */
#include "usb_lib.h"
#include "usb_msc_desc.h"
#include "usb_msc_prop.h"
#include "usb_msc_conf.h"
#include "usb_msc_mass_mal.h"
#include "usb_pwr.h"
#include "usb_istr.h"
#include "hw_config.h"

/* Function Prototypes */
extern void msc_task(void);

#ifdef __cplusplus
}
#endif

#endif	/* MSC_SUPPORT_H */
