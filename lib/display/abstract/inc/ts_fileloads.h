/********************************************************************************/
/*!
	@file			ts_fileloads.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2012.07.10
	@brief          Filer and File Loaders.

    @section HISTORY
		2012.04.15	V1.01	Start here.
		2012.06.12  V2.00   Added WAVE Player Handling.
		2012.07.10  V3.00   Added GIF Decorder Handling.
							Fixed libjpeg & libpng's Error Handlings.

    @section LICENSE
		BSD License. See Copyright.txt
		
		---IJG JPEGLIB Notice---
		"this software is based in part on the work ofthe Independent JPEG Group".
*/
/********************************************************************************/
#ifndef __TS_FILELOADS_H
#define __TS_FILELOADS_H 0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* basic includes */
#include "ts.h"

/* JPEG Decorder Definitions */
#ifdef USE_IJG_LIB
 #include "jpeglib.h"
/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

 #include <setjmp.h>
 /* From example.c */
 struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;		/* for return to caller */
};
typedef struct my_error_mgr * my_error_ptr;

#ifdef STM32F4XX
 #define JDCT_DECODE_METHOD JDCT_FLOAT
#else
 #define JDCT_DECODE_METHOD JDCT_ISLOW
#endif

/* ChaN's TINYJPEG Decorder Definitions */
#elif USE_TINYJPEG_LIB
 #include "tjpgd.h" /* Used for Tiny JPEG Decoder */
 #define MaskL 0
 #define MaskR MAX_X
 #define MaskT 0
 #define MaskB MAX_Y
#endif

/* PNG Decorder Definitions */
#ifdef USE_LIBPNG
 #include "zlib.h"
 #include "png.h"
 /* The png_jmpbuf() macro, used in error handling, became available in
  * libpng version 1.0.6.  If you want to be able to run your code with older
  * versions of libpng, you must define the macro yourself (but only if it
  * is not already defined by libpng!).
  */
 #ifndef png_jmpbuf
 #  define png_jmpbuf(png_ptr) ((png_ptr)->jmpbuf)
 #endif
#endif

/* GIF Decorder Definitions */
#ifdef USE_GIFLIB
 #include "gif_lib.h"
 #include "gif_lib_private.h"
 #define GIF_TRANSPARENT_SUPPORT
#endif

/* MP3 Decording */
#ifdef USE_MP3_HELIX
 #include "mp3_support.h"
#endif

/* RIFF-Wave Streaming */
#ifdef USE_WAVE_OUT
 #include "wav_support.h"
#endif

/* Terminal Screen Prototypes */
extern int filer(char *path, FIL *fil, DIR *dir, FILINFO *fno);
extern volatile uint32_t on_filer;	/* Used on STM32 Primer2 */ 

/* Array Buff[] must be declared FatFs Basic Routine!(e.g. in ff_support.c) */
extern uint8_t Buff[];

#ifdef __cplusplus
}
#endif

#endif /* __TS_FILELOADS_H */
