/********************************************************************************/
/*!
	@file			ts.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        12.00
    @date           2013.11.30
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!

    @section HISTORY
		2010.12.31	V1.00	ReStart here.
		2011.03.10	V2.00	C++ Ready.
		2011.06.18  V3.00	Added SSD1963 Consideration 
							      and X clipper for some display's limitations
								  and Fixed Parent/Current Directory return bug.
		2011.10.14  V4.00   Added Chan's Tiny JPEG Decoder Support.
		2011.12.01  V5.00   Use FPU on IJG Decoding in use of STM32F4xx. 
		2012.01.31	V6.00	Fixed Pointer bugfix on IJG Decoding.
		2012.02.21	V7.00	Added Chan's Text Viewer Support.
		2012.03.12  V8.00   Added More FontSize.
							Fixed JPEG Decompress parameter@IJG.
		2012.04.01  V9.00   Separated to Filer & FileLoder Sections.
		2012.06.15 V10.00   Added External SRAM Support.
		2013.09.20 V11.00   Added more colour definitions.
		2013.11.30 V12.00   Added External SDRAM Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __TS_H
#define __TS_H 0x1200

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include <string.h>
#include <inttypes.h>
/* FatFs Ralation */
#include "ff.h"
/* Terminal Relation (Device dependent) */
#include "term_io.h"
/* Device Depend Includes */
#include "display_if.h"

/* Determine Filer Font Size */
#if USE_FILER_FONTX
 #include "display_if_support.h"
 
/* Terminal Screen Definitions */
#if defined(FONT8x8)
 /* Use Misaki Fonts(8pt) */
 #define TS_FONTW	4
 #define TS_FONTH	8

#elif defined(FONT10x10)
 /* Use M+ Fonts(10pt) */
 #define TS_FONTW	5
 #define TS_FONTH	10

#elif defined(FONT12x10)
 /* Use k12x10 Fonts(12x10pt) */
 #define TS_FONTW	6
 #define TS_FONTH	10

#elif defined(FONT12x12)  || defined(FONT12x12D) || defined(FONT12x12K) || defined(FONT12x12W) || \
      defined(FONT12x12M) || defined(FONT12x12L)
 /* Use M+ Fonts(12pt) or Kanamechou Fonts (12pt) or Wada Laboratry Hosomaru Fonts(12pt)
     or MAYAKU Fonts(12pt) or Dohgenzaka Fonts(12pt) or  Kodenmacho Fonts(12pt) */
 #define TS_FONTW	6
 #define TS_FONTH	12

#elif defined(FONT14x14) || defined(FONT14x14M) || defined(FONT14x14W)
 /* Use Shinnonome Gothic Fonts(14pt) or Wada Laboratry Hosomaru Fonts(14pt) or Dohgenzaka Fonts(14pt) */
 #define TS_FONTW	7
 #define TS_FONTH	14

#elif defined(FONT16x16) || defined(FONT16x16K) || defined(FONT16x16L) || defined(FONT16x16W)
 /* Use Tanuki Fonts(16pt) or Kyoukashotai Fonts(16pt) or Kodenma-cho Fonts(16pt)
     or Wada Laboratry Hosomaru Fonts(16pt) */
 #define TS_FONTW	8
 #define TS_FONTH	16

#elif defined(FONT20x20)
 /* Use Tanuki Fonts(20pt) */
 #define TS_FONTW	10
 #define TS_FONTH	20

#else
 #error "U MUST select font size in use of FONTX2! "
#endif

#else
 /* Use Built-in fonts */
 #define TS_FONTW	5
 #define TS_FONTH	8
#endif

/* Determine Filer Screen Size */
#if (((MAX_X/TS_FONTW)*TS_FONTW)==MAX_X)
 #define TS_WIDTH	((MAX_X)/(TS_FONTW))
#else
 #define TS_WIDTH	(((MAX_X)/(TS_FONTW))+TS_FONTW)
#endif
#define TS_HEIGHT	((MAX_Y)/(TS_FONTH))

/* Display Current Time or not */
#if defined(USE_STM32PRIMER2) || defined(USE_TIME_DISPLAY)
 #define TS_FILER_HEIGHT	(TS_HEIGHT-1)
#else
 #define TS_FILER_HEIGHT	(TS_HEIGHT)
#endif

/* Display Fonts Colour */
#define COL_NORMAL		COL_WHITE
#define COL_HIDDEN		COL_BLUE
#define COL_DIRECTORY	COL_AQUA
#define COL_READONLY	COL_GREEN
#define COL_TITLE		((COL_BLUE << 16) | COL_WHITE)
#define COL_STAT		((COL_BLUE << 16) | COL_WHITE)
#define COL_WBASE		COL_WHITE
#define COL_WTITLE		((COL_AQUA << 16) | COL_BLACK)
#define COL_WINPUT		((COL_GRAY << 16) | COL_WHITE)

/* Used for Next Filer Ready */
typedef struct {
	uint8_t  fontw;
	uint8_t  fonth;
	uint16_t width;
	uint16_t height;
	uint16_t filer_height;
} TS;

/* Terminal Screen Prototypes */
extern void ts_putc(uint8_t chr);
extern void ts_write(uint8_t row, uint8_t col, uint16_t chr);
extern void ts_locate(uint8_t row, uint8_t col, uint8_t csr);
extern void ts_rfsh(uint8_t top, uint8_t left, uint8_t bottom, uint8_t right);
extern void ts_rlup(uint8_t top, uint8_t left, uint8_t bottom, uint8_t right);
extern void ts_rldown(uint8_t top, uint8_t left, uint8_t bottom, uint8_t right);

extern void ts_timer(void);
extern void ts_rtc(void);

/* Array Buff[] must be declared FatFs Basic Routine! */
extern uint8_t Buff[];

/* Filer control command */
#define KEY_UP		'\x05'	/* ^[E] */
#define KEY_DOWN	'\x18'	/* ^[X] */
#define KEY_LEFT	'\x13'	/* ^[S] */
#define KEY_RIGHT	'\x04'	/* ^[D] */
#define KEY_OK		'\x0D'	/* [Enter] */
#define KEY_CAN		'\x1B'	/* [Esc] */
#define KEY_BS		'\x08'	/* [BS] */
#define KEY_SPC		' '		/* [Space] */
#define KEY_MKDIR	'K'		/* [K] */
#define KEY_DRIVE	'D'		/* [D] */
#define KEY_COPY	'C'		/* [C] */
#define KEY_REMOVE	'U'		/* [U] */
#define KEY_RENAME	'N'		/* [N] */
#define KEY_ALL		'A'		/* [A] */
#define KEY_ATTRIB	'T'		/* [T] */

#ifdef __cplusplus
}
#endif

#endif /* __TS_H */
