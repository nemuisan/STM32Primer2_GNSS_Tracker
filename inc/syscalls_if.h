/********************************************************************************/
/*!
	@file			syscalls_if.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2014.06.26
	@brief          syscall.c's Device Dependent Header Section.

    @section HISTORY
		2010.12.31	V1.00	ReStart Here.
		2011.03.10	V2.00	C++ Ready.
		2012.06.15  V3.00	Fixed _heap_end Definition.
		2014.06.26	V4.00	Added Version Check.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SYSCALLS_IF_H
#define __SYSCALLS_IF_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* Basis Inclusion (Device Dependent) */
#include "hw_config.h"

/* If U use "_heap_end" as current stack pointer, MUST uncomment it */
#define USE_SP_AS_HEAP_END

/* printf/scanf relation settings */
/* Platform dependent */
#define putch(x)  putch(x)
#define getch()   getch()

/* If U use scanf echoback, uncomment it */
/* #define ECHOBACK */

/*
Notice: 
		Line Feed     : Dec=10 Hex=0x0A ESC='\n'
		Caridge Return: Dec=13 Hex=0x0D ESC='\r'
*/

#ifdef __cplusplus
}
#endif

#endif /* __SYSCALLS_IF_H  */
