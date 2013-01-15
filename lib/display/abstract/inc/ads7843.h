/********************************************************************************/
/*!
	@file			ads7843.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.03.10
	@brief          Touch Screen Control Upperside 
	
    @section HISTORY
		2011.03.10	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __ADS7843_H
#define __ADS7843_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* Basic Includes */

/* Touch Macros */
#define START_7843	(1<<7)
#define A2_7843		(1<<6)
#define A1_7843		(1<<5)
#define A0_7843		(1<<4)
#define SEL_YPLUS	(A0_7843)
#define SEL_XPLUS	(A2_7843 | A0_7843)
#define MODE_12BIT	(0<<3)
#define MODE_08BIT	(1<<3)
#define SINGLE_INP	(1<<2)
#define DIFFER_INP	(0<<2)
#define POWSAVE_EN	(0<<0)
#define POWSAVE_DIS (3<<0)

/* Low Resolution */
#define X_SINGLE_08	(START_7843 | SEL_XPLUS | MODE_08BIT | SINGLE_INP | POWSAVE_EN )
#define Y_SINGLE_08	(START_7843 | SEL_YPLUS | MODE_08BIT | SINGLE_INP | POWSAVE_EN )
#define X_DIFFER_08	(START_7843 | SEL_XPLUS | MODE_08BIT | DIFFER_INP | POWSAVE_EN )
#define Y_DIFFER_08	(START_7843 | SEL_YPLUS | MODE_08BIT | DIFFER_INP | POWSAVE_EN )
/* High Resolution */
#define X_SINGLE_12	(START_7843 | SEL_XPLUS | MODE_12BIT | SINGLE_INP | POWSAVE_EN )
#define Y_SINGLE_12	(START_7843 | SEL_YPLUS | MODE_12BIT | SINGLE_INP | POWSAVE_EN )
#define X_DIFFER_12	(START_7843 | SEL_XPLUS | MODE_12BIT | DIFFER_INP | POWSAVE_EN )
#define Y_DIFFER_12	(START_7843 | SEL_YPLUS | MODE_12BIT | DIFFER_INP | POWSAVE_EN )


#ifdef __cplusplus
}
#endif

#endif /* __ADS7843_H */
