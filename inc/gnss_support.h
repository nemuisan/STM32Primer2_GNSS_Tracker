/********************************************************************************/
/*!
	@file			gnss_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        22.00
    @date           2024.07.11
	@brief          Interface of FatFs For STM32 uC.				@n
					Based on Chan's GNSS-Logger Program Thanks!

    @section HISTORY
		2011.03.10	V1.00	Start Here.
		2011.09.07	V2.00	Added RTC Synchronization from GPRMC.
		                    Fixed More Stability.
		2011.12.26	V3.00	Added PA6C Support.
		2012.08.31  V4.00   Imploved FatFs Support Function's Portability.
		2012.09.08	V5.00	Imploved PA6C Support.
		                     (Anti Interference & force 9600bps for 38400bps Firmware).
		2012.12.24  V6.00   Added Gms-g6a(MT3333 GLONASS MODE) Support.
		                    Imploved Error Handlings.
		2013.02.20  V7.00   Added Some MT3339/MT3333 Commands.
		2013.04.10  V8.00   Changed UART-Retarget Method.
		2013.10.09	V9.00	Adopted FatFs0.10.
		2015.02.28 V10.00	Buffer alignment set by 4Byte.
		2016.04.15 V11.00	Adopted FatFs0.12.
		2016.05.13 V12.00	Adopted Gms-g9(Titan3) new firmware.
		2017.05.23 V13.00	Adopted FatFs0.13.
		2017.11.01 V14.00	Add and fix more MTK Commands.
		2018.12.12 V15.00	Adopted XA1110 AXN5.x.x new firmware.
		2021.02.05 V16.00	Adopted xprintf update.
		2021.10.31 V17.00	Fixed MTK Commands parameter.
		2022.10.15 V18.00	Fixed filesystem robustness and change filename.
		2023.04.21 V19.00	Fixed cosmetic bugfix.
		2023.06.04 V20.00	Adopted u-blox SAM-M10Q module.
		2023.12.19 V21.00	Improved watchdog handlings.
		2024.07.11 V22.00	Fixed datetime setting bug.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef GNSS_SUPPORT_H
#define GNSS_SUPPORT_H 0x2200

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include <string.h> /* memset et al*/

/* Miscs */
#include "hw_config.h"
#include "diskio.h"
#include "ff.h"
#include "ff_rtc_if.h"
#include "rtc_support.h"

/* defines */
#define	CMD_NOMAL	0
#define	CMD_MTK		1

/* Function Prototypes */
extern void ff_support_timerproc(void);
extern void gps_task(void);
extern void ShutFileClose(void);
extern void ChkAckLimit(void);
extern volatile UINT cmd_mode;
static inline void GNSS_SetCmdModes(UINT mode)
{
	cmd_mode = mode;
}

#ifdef __cplusplus
}
#endif

#endif	/* GNSS_SUPPORT_H */
