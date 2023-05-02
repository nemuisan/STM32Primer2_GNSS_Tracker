/********************************************************************************/
/*!
	@file			ads7843.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.05.01
	@brief          Touch Screen Control Upperside 
	
    @section HISTORY
		2011.03.10	V1.00	Stable Release.
		2023.05.01	V2.00	Fixed cosmetic bugfix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef ADS7843_H
#define ADS7843_H 0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* Basic Includes */

/* Touch Macros */
#define START_7843			(1<<7)
#define A2_7843				(1<<6)
#define A1_7843				(1<<5)
#define A0_7843				(1<<4)
#define SEL_YPLUS_7843		(A0_7843)
#define SEL_XPLUS_7843		(A2_7843 | A0_7843)
#define MODE_12BIT_7843		(0<<3)
#define MODE_08BIT_7843		(1<<3)
#define SINGLE_INP_7843		(1<<2)
#define DIFFER_INP_7843		(0<<2)
#define POWSAVE_EN_7843		(0<<0)
#define POWSAVE_DIS_7843	(3<<0)

/* Low Resolution */
#define X_SINGLE_08	(START_7843 | SEL_XPLUS_7843 | MODE_08BIT_7843 | SINGLE_INP_7843 | POWSAVE_EN_7843 )
#define Y_SINGLE_08	(START_7843 | SEL_YPLUS_7843 | MODE_08BIT_7843 | SINGLE_INP_7843 | POWSAVE_EN_7843 )
#define X_DIFFER_08	(START_7843 | SEL_XPLUS_7843 | MODE_08BIT_7843 | DIFFER_INP_7843 | POWSAVE_EN_7843 )
#define Y_DIFFER_08	(START_7843 | SEL_YPLUS_7843 | MODE_08BIT_7843 | DIFFER_INP_7843 | POWSAVE_EN_7843 )
/* High Resolution */
#define X_SINGLE_12	(START_7843 | SEL_XPLUS_7843 | MODE_12BIT_7843 | SINGLE_INP_7843 | POWSAVE_EN_7843 )
#define Y_SINGLE_12	(START_7843 | SEL_YPLUS_7843 | MODE_12BIT_7843 | SINGLE_INP_7843 | POWSAVE_EN_7843 )
#define X_DIFFER_12	(START_7843 | SEL_XPLUS_7843 | MODE_12BIT_7843 | DIFFER_INP_7843 | POWSAVE_EN_7843 )
#define Y_DIFFER_12	(START_7843 | SEL_YPLUS_7843 | MODE_12BIT_7843 | DIFFER_INP_7843 | POWSAVE_EN_7843 )


#ifdef __cplusplus
}
#endif

#endif /* ADS7843_H */
