/********************************************************************************/
/*!
	@file			ts_basis.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        22.00
    @date           2024.07.16
	@brief          Filer and File Loaders.

    @section HISTORY
		2024.07.16	See ts_ver.txt.

    @section LICENSE
		BSD License + IJG JPEGLIB license See Copyright.txt
*/
/********************************************************************************/
#ifndef TS_BASIS_H
#define TS_BASIS_H 0x2200

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

#elif defined(FONT10x10) || defined(FONT10x10T)
 /* Use M+ Fonts(10pt) or  Konatsu Fonts(10pt) */
 #define TS_FONTW	5
 #define TS_FONTH	10

#elif defined(FONT12x10)
 /* Use k12x10 Fonts(12x10pt) */
 #define TS_FONTW	6
 #define TS_FONTH	10

#elif defined(FONT12x12)  || defined(FONT12x12D) || defined(FONT12x12K) || defined(FONT12x12W) || \
      defined(FONT12x12M) || defined(FONT12x12L) || defined(FONT12x12T)
 /* Use M+ Fonts(12pt) or Kanamechou Fonts (12pt) or Wada Laboratry Hosomaru Fonts(12pt)
     or MAYAKU Fonts(12pt) or Dohgenzaka Fonts(12pt) or  Kodenmacho Fonts(12pt) or Konatsu Fonts(12pt) */
 #define TS_FONTW	6
 #define TS_FONTH	12

#elif defined(FONT14x14) || defined(FONT14x14M) || defined(FONT14x14W)
 /* Use Shinnonome Gothic Fonts(14pt) or Wada Laboratry Hosomaru Fonts(14pt) or Dohgenzaka Fonts(14pt) */
 #define TS_FONTW	7
 #define TS_FONTH	14

#elif defined(FONT16x16)  || defined(FONT16x16K) || defined(FONT16x16L) || defined(FONT16x16W) || \
      defined(FONT16x16F) || defined(FONT16x16H) || defined(FONT16x16I) || defined(FONT16x16T)
 /* Use Tanuki Fonts(16pt) or Kyoukashotai Fonts(16pt) or Kodenma-cho Fonts(16pt)
     or Wada Laboratry Hosomaru Fonts(16pt) or Honoka-Maru Gothic Fonts(16pt)
     or Hanazono Fonts(16pt) or IL Gothic Fonts(16pt) or Konatsu Fonts(16pt) */
 #define TS_FONTW	8
 #define TS_FONTH	16

#elif defined(FONT20x20)
 /* Use Tanuki Fonts(20pt) */
 #define TS_FONTW	10
 #define TS_FONTH	20

#elif defined(FONT24x24I)
 /* Use IL Gothic Font(24pt) */
 #define TS_FONTW	12
 #define TS_FONTH	24

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
 #define TS_WIDTH	(((MAX_X)/(TS_FONTW))+1)
#endif
#define TS_HEIGHT	((MAX_Y)/(TS_FONTH))

/* Display Current Time or not */
#if defined(USE_TIME_DISPLAY)
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
extern void ts_csrblink(void);
extern void ts_rtc(void);


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

#endif /* TS_BASIS_H */
