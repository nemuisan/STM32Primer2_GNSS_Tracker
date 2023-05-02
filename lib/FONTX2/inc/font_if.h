/********************************************************************************/
/*!
	@file			font_if.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2023.05.01
	@brief          Interface of FONTX Driver								@n
                    Referred under URL thanks!								@n
					http://www.hmsoft.co.jp/lepton/software/dosv/fontx.htm	@n
					http://hp.vector.co.jp/authors/VA007110/technicalworkshop

    @section HISTORY
		2010.12.31	V1.00	Stable Release.
		2011.03.10	V2.00	C++ Ready.
		2011.05.11	V3.00	Selectable KanjiFonts to Reduce Memory Space.
		2014.03.31	V4.00	Fixed hardfault error on Cortex-M0 Devices.
		2017.06.07	V5.00	Added signature validation function.
		2019.02.01	V6.00	Fixed some compiler warnings.
		2023.05.01	V7.00	Fixed cosmetic bugfix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef FONT_IF_H 
#define FONT_IF_H 0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* Include Basis */
#include <inttypes.h>
#include <string.h>

/* Device Dependent Macros */
/* To Read uC Flash Memory Little Endian */ 
#define	READ_ADDR_UNIT8(ADDR) 	((uint8_t)*(uint8_t*)(addr_ofs+ADDR))
#define	READ_ADDR_UNIT16(ADDR) 	((uint16_t)*(uint16_t*)(addr_ofs+ADDR))
/* Retrive 8bit datas from Current pointer */
#define	READ_ADDR_UNIT8_C(ADDR) ((uint8_t)*(uint8_t*)(ADDR))


/**************************************************************************/
/*! 
    FONTX2 Alphabet Numeric Kana Definitions !!
*/
/**************************************************************************/
/* FONTX2 Structure offset of ANK */
#define	ANK_HEADER		(0)			/*    +0(6 byte) Includes "FONTX2" String */
#define ANK_FONTNAME	(6)			/*    +6(8 byte) Name of FONTX2			  */
#define ANK_XSIZE		(14)		/*   +14(1 byte) Xsize(in pixel)		  */
#define ANK_YSIZE		(15)		/*   +15(1 byte) Ysize(in pixel)		  */
#define ANK_CODETYPE	(16)		/* 	 +16(1 byte) Codetype(usually 0x01)   */
#define ANK_DATSTART	(17)		/* 	 +17(- byte) Ank Fontdata start		  */

/* Structs */
typedef struct {
	const char* AnkFileOffset;		/* Ank FONTX2 File Address Offset 	*/
	uint8_t  AnkSize;				/* Size of Ank Font (in byte) 		*/
	uint8_t  X_Size;				/* Xsize(in pixel) 					*/
	uint8_t  Y_Size;				/* Ysize(in pixel)					*/
} FontX_Ank;

/* Externals */
extern void InitFont_Ank(FontX_Ank* AnkDat,const char* addr_ofs);
extern uint8_t* GetPtr_Ank(uint8_t AnkCode);
extern void ChangeCurrentAnk(FontX_Ank* AnkDat);
extern uint8_t ChkFontSig_Ank(FontX_Ank* AnkDat);
extern void GetFontName_Ank(char* name);
extern FontX_Ank* CurrentAnkDat;


/**************************************************************************/
/*! 
    FONTX2 Kanji(2 Byte Charactor) Definitions !!
*/
/**************************************************************************/
/* FONTX2 Structure offset of KANJI(Sift-JIS) */
#define	KANJI_HEADER	(0)			/*    +0(6 byte) Includes "FONTX2" String */
#define KANJI_FONTNAME	(6)			/*    +6(8 byte) Name of FONTX2			  */
#define KANJI_XSIZE		(14)		/*   +14(1 byte) Xsize(in pixel)		  */
#define KANJI_YSIZE		(15)		/*   +15(1 byte) Ysize(in pixel)		  */
#define KANJI_CODETYPE	(16)		/* 	 +16(1 byte) Codetype(usually 0x01)   */
#define KANJI_TABLENUM	(17)		/* 	 +17(1 byte) Number of Table offset	  */
#define KANJI_DATSTART	(18)		/* 	 +18(- byte) Kanji Data offset   	  */

/* Kanji DataBlock Macros */
#define KANJI_START(n)   READ_ADDR_UNIT16( (KANJI_DATSTART+ 4*(n)) 	  )
#define KANJI_ENDE(n)    READ_ADDR_UNIT16( (KANJI_DATSTART+ 4*(n) +2) )
/* Kanji Font Only */
#define	READ_ADDR_UNIT16_C(ADDR) 	((uint16_t)*(uint16_t*)(CurrentKanjiDat->KanjiFileOffset+ADDR))
#define KANJI_START_C(n) READ_ADDR_UNIT16_C( (KANJI_DATSTART+ 4*(n))    )
#define KANJI_ENDE_C(n)  READ_ADDR_UNIT16_C( (KANJI_DATSTART+ 4*(n) +2) )

#define SJIS_TOOFU		(0x81A0)	/*  Toofu ga starto suru            */ 

/* Structs */
typedef struct {
	const char* KanjiFileOffset;	/* Kanji FONTX2 File Address Offset */
	const char* KanjiStartOffset;	/* Kanji FONTX2 Data Address Offset */
	uint8_t  KanjiSize;				/* Size of Kanji Font (in byte) 	*/
	uint8_t  X_Size;				/* Xsize(in pixel) 					*/
	uint8_t  Y_Size;				/* Ysize(in pixel)					*/
	uint8_t  KanjiTableNum;			/* Kanji Font Table Number  		*/ 
	uint16_t KanjiSearchTable[256]; /* Kanji Font Search Table	   		*/ 
} FontX_Kanji;

/* Externals */
extern void InitFont_Kanji(FontX_Kanji* KanjiDat,const char* addr_ofs);
extern uint8_t* GetPtr_Kanji(uint16_t SjisCode);
extern void ChangeCurrentKanji(FontX_Kanji* KanjiDat);
extern uint8_t ChkFontSig_Kanji(FontX_Kanji* KanjiDat);
extern void GetFontName_Kanji(char* name);
extern FontX_Kanji* CurrentKanjiDat;


/* This is example valiables,U can increase more font-tables! */
/* See "font_if_datatable.c" !*/
extern FontX_Ank   ANKFONT;
extern FontX_Kanji KANJIFONT;

/* Display helper macros */
#define Chr_PosY(y)		(CurrentAnkDat->Y_Size*(y))
#define Chr_PosX(x)		(CurrentAnkDat->X_Size*(x))
#define Sjis_PosY(y)	(CurrentKanjiDat->Y_Size*(y))
#define Sjis_PosX(x)	(CurrentKanjiDat->X_Size*(x))

#ifdef __cplusplus
}
#endif

#endif /* FONT_IF */
