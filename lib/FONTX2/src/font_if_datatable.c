/********************************************************************************/
/*!
	@file			font_if_datatable.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        10.00
    @date           2023.09.01
	@brief          Interface of FONTX Driver								@n
                    Referred under URL thanks!								@n
					http://www.hmsoft.co.jp/lepton/software/dosv/fontx.htm	@n
					http://hp.vector.co.jp/authors/VA007110/technicalworkshop

    @section HISTORY
		2011.03.11	V1.00	Stable Release.
		2011.05.11	V2.00	Selectable KanjiFonts to Reduce Memory Space.
		2012.03.12	V3.00	Add More Fonts.
		2012.06.30	V4.00	Add External SRAM Support for KanjiFonts.
 		2013.09.20	V5.00	Improve FONTX2 inclusion ChaN Thanks!
		2015.08.01	V6.00	Add External SDRAM Support for KanjiFonts.
		2015.09.01	V7.00	Optimized KANJI Constructors for Specific MPU.
		2019.02.01	V8.00	Fixed some compiler warnings.
		2023.05.01	V9.00	Fixed cosmetic bugfix.
		2023.09.01 V10.00	Added Konatsu-Fonts.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "font_if_datatable.h"
/* check header file version for fool proof */
#if FONT_IF_DATATABLE_H != 0x1000
#error "header file version is not correspond!"
#endif

/* Defines   -----------------------------------------------------------------*/
/* uC specific settings */
#if   defined(STM32F7XX) || defined(STM32H7XX)
 #define ATTR_KANJI_RAM  __attribute__ ((section(".dtcm")))
#elif defined(STM32F4XX)
 #define ATTR_KANJI_RAM  __attribute__ ((section(".ccram")))
#else
 #define ATTR_KANJI_RAM
#endif

#if defined(EXT_QSPIROM_SUPPORT) || defined(EXT_SPIFI_SUPPORT)
/* Assume FONTX2 files is in ExternalROM */
/* IMPORT_BIN Macros are not necessary */
#define	IMPORT_BIN(sect, file, sym)	
#define	IMPORT_BIN_PART(sect, file, ofs, siz, sym)

#else
/* Import FONTX2 files as byte array */
/* incbin example ChaN Thanks! */
#define	IMPORT_BIN(sect, file, sym) __asm__ (\
		".section " #sect "\n"\
		".balign 4\n"\
		".global " #sym "\n"\
		#sym ":\n"\
		".incbin \"" file "\"\n"\
		".global _sizeof_" #sym "\n"\
		".set _sizeof_" #sym ", . - " #sym "\n"\
		".balign 4\n"\
		".section \".text\"\n")

#define	IMPORT_BIN_PART(sect, file, ofs, siz, sym) __asm__ (\
		".section " #sect "\n"\
		".balign 4\n"\
		".global " #sym "\n"\
		#sym ":\n"\
		".incbin \"" file "\"," #ofs "," #siz "\n"\
		".global _sizeof_" #sym "\n"\
		".set _sizeof_" #sym ", . - " #sym "\n"\
		".balign 4\n"\
		".section \".text\"\n")

#endif

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/
#if defined(FONT8x8)
#warning "USE MISAKI FONTS(8x8)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "misaki.h"
	};
	FontX_Kanji Misaki_Kanji ATTR_KANJI_RAM;
#endif
const char font_table_ank[] = { 
	#include "4x8.h" 
};
FontX_Ank   Misaki_Ank;

#elif defined(FONT10x10)
 #warning "USE M+ FONTS(10x10)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "mplusfx2/MPLZN10X.FNT", font_table_kanji);
	FontX_Kanji Mplus_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "mplusfx2/MPLHN10X.FNT",  font_table_ank);
 FontX_Ank   Mplus_Ank;

#elif defined(FONT10x10T)
 #warning "USE KONATSU FONTS(10x10)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "KONATSU/KONATZN10.FNT", font_table_kanji);
	FontX_Kanji Konatsu_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "KONATSU/KONATHN10.FNT",  font_table_ank);
 FontX_Ank   Konatsu_Ank;

#elif defined(FONT12x10)
#warning "USE K12x10 FONTS(12x10)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "k12x10x/k12x10.fnt", font_table_kanji);
	FontX_Kanji K12x10_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "k12x10x/k6x10.fnt",  font_table_ank);
 FontX_Ank   K12x10_Ank;

#elif defined(FONT12x12)
#warning "USE M+ FONTS(12x12)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "mplusfx2/MPLZN12X.FNT", font_table_kanji);
	FontX_Kanji Mplus_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "mplusfx2/MPLHN12X.FNT",  font_table_ank);
 FontX_Ank   Mplus_Ank;

#elif defined(FONT12x12D)
#warning "USE MAYAKU FONTS(12x12)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "drug12/DRUG12Z1.FEF", font_table_kanji);
	FontX_Kanji Drug_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "drug12/DRUG12HN.FEF",  font_table_ank);
 FontX_Ank   Drug_Ank;

#elif defined(FONT12x12K)
#warning "USE KANAMECHOU FONTS(12x12)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "KANAME/KNMZN12X.MNF", font_table_kanji);
	FontX_Kanji Kaname_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "KANAME/KNMHN12X.MNF",  font_table_ank);
 FontX_Ank   Kaname_Ank;

#elif defined(FONT12x12L)
#warning "USE KODENMA-CHO FONTS(12x12)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "VGOL12/GOTZN12X.TLF", font_table_kanji);
	FontX_Kanji Kodenma_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "VGOL12/GOTHN12X.TLF",  font_table_ank);
 FontX_Ank   Kodenma_Ank;

#elif defined(FONT12x12M)
#warning "USE DOHGEN-ZAKA FONTS(12x12)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "VMNB12/MINZN12X.TLF", font_table_kanji);
	FontX_Kanji Dohgenzaka_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "VMNB12/MINHN12X.TLF",  font_table_ank);
 FontX_Ank   Dohgenzaka_Ank;

#elif defined(FONT12x12W)
#warning "USE WADA-KEN HOSOMARU GOTHIC FONTS(12x12)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "wlma2fnt/Wlma212k.fnt", font_table_kanji);
	FontX_Kanji Wlma_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "wlma2fnt/Wlma212a.fnt",  font_table_ank);
 FontX_Ank   Wlma_Ank;

#elif defined(FONT12x12T)
 #warning "USE KONATSU FONTS(12x12)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "KONATSU/KONATZN12.FNT", font_table_kanji);
	FontX_Kanji Konatsu_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "KONATSU/KONATHN12.FNT",  font_table_ank);
 FontX_Ank   Konatsu_Ank;

#elif defined(FONT14x14)
#warning "USE SHINONOME GOTHIC FONTS(14x14)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "shn14fx2/SHGZN14X.FNT", font_table_kanji);
	FontX_Kanji Shinonome_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "shn14fx2/SHNHN14X.FNT",  font_table_ank);
FontX_Ank   Shinonome_Ank;

#elif defined(FONT14x14M)
#warning "USE DOHGEN-ZAKA FONTS(14x14)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "VMNB12/MINZN14X.TLF", font_table_kanji);
	FontX_Kanji Dohgenzaka_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "VMNB12/MINHN14X.TLF",  font_table_ank);
 FontX_Ank   Dohgenzaka_Ank;

#elif defined(FONT14x14W)
#warning "USE WADA-KEN HOSOMARU GOTHIC FONTS(14x14)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "wlma2fnt/Wlma214k.fnt", font_table_kanji);
	FontX_Kanji Wlma_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "wlma2fnt/Wlma214a.fnt",  font_table_ank);
 FontX_Ank   Wlma_Ank;

#elif defined(FONT16x16)
#warning "USE TANUKI MAGIC FONTS(16x16)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "TANU/TANUZN16.fnt", font_table_kanji);
	FontX_Kanji Tanu_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "TANU/TANUHN16.fnt",  font_table_ank);
FontX_Ank   Tanu_Ank;

#elif defined(FONT16x16K)
#warning "USE NINGYOUCHOU FONTS(16x16)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "VKYO16/KYOZN16X.TLF", font_table_kanji);
	FontX_Kanji Kyoukasho_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "VKYO16/KYOHN16X.TLF",  font_table_ank);
 FontX_Ank   Kyoukasho_Ank;
 
#elif defined(FONT16x16L)
#warning "USE KODENMA-CHO FONTS(16x16)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "VGON16/GONZN16X.TLF", font_table_kanji);
	FontX_Kanji Kodenma_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "VGON16/GONHN16X.TLF",  font_table_ank);
 FontX_Ank   Kodenma_Ank;

#elif defined(FONT16x16W)
#warning "USE WADA-KEN HOSOMARU GOTHIC FONTS(16x16)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "wlma2fnt/Wlma216k.fnt", font_table_kanji);
	FontX_Kanji Wlma_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "wlma2fnt/Wlma216a.fnt",  font_table_ank);
 FontX_Ank   Wlma_Ank;

#elif defined(FONT16x16F)
#warning "USE HONOKA-MARU GOTHIC FONTS(16x16)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "honoka-marugo/HNMZN16.fnt", font_table_kanji);
	FontX_Kanji Honoka_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "honoka-marugo/HNMHN16.fnt",  font_table_ank);
 FontX_Ank   Honoka_Ank;

#elif defined(FONT16x16H)
#warning "USE HANAZONO FONTS(16x16)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "hanazono/HNZNZN16.fnt", font_table_kanji);
	FontX_Kanji Hanazono_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "hanazono/HNZNHN16.fnt",  font_table_ank);
 FontX_Ank   Hanazono_Ank;

#elif defined(FONT16x16T)
#warning "USE KONATSU FONTS(16x16)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "KONATSU/KONATZN16.FNT", font_table_kanji);
	FontX_Kanji Konatsu_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "KONATSU/KONATHN16.FNT",  font_table_ank);
 FontX_Ank   Konatsu_Ank;

#elif defined(FONT20x20)
#warning "USE TANUKI MAGIC FONTS(20x20)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "TANU/TANUZN20.fnt", font_table_kanji);
	FontX_Kanji Tanu_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "TANU/TANUHN20.fnt",  font_table_ank);
FontX_Ank   Tanu_Ank;

#elif defined(FONT24x24I)
#warning "USE IL FONTS(24x24)"
 #ifdef USE_KANJIFONT
	IMPORT_BIN(".rodata", "ILFONT/ILGZ24XF.FNT", font_table_kanji);
	FontX_Kanji IL_Kanji ATTR_KANJI_RAM;
 #endif
 IMPORT_BIN(".rodata", "ILFONT/ILGH24XF.FNT",  font_table_ank);
 FontX_Ank   IL_Ank;

#elif defined(FONT_DIGIT)
#warning "USE DIGIT FONT "
const char font_table_ank[] = {
	#include "DIGITHN.h"
};
FontX_Ank   Digit_Ank;


#else /* defined(NONE) */
	#error "not defined fonts!!"
#endif

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/


/* End Of File ---------------------------------------------------------------*/
