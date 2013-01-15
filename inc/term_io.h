/********************************************************************************/
/*!
	@file			term_io.h
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
#ifndef __TERM_IO_H
#define __TERM_IO_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
#include <stdarg.h>
#include <string.h>
#include "xprintf.h"

/* Miscs */
#include "hw_config.h"
#include "ff.h"
#include "display_if.h"
#ifdef USE_ADS7843
 #include "touch_if.h"
#endif

/* #define xavail() keypressed() */
#define __kbhit() keypressed()

/* Externals */
extern char xgetc_n (void);
extern unsigned char xgetc (void);

/* Macros */
#define BTN_UP		0x05
#define BTN_DOWN	0x18
#define BTN_LEFT	0x13
#define BTN_RIGHT	0x04
#define BTN_OK		0x0D
#define BTN_ESC		0x1A
#define BTN_CAN		0x1B
#define BTN_POWER	0x7F
#define CMD_LOWBAT	0x1F


#ifdef __cplusplus
}
#endif

#endif	/* __TERM_IO_H */
