/********************************************************************************/
/*!
	@file			font_if_datatable.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2023.05.01
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

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef FONT_IF_DATATABLE_H 
#define FONT_IF_DATATABLE_H 0x0900

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes */
#include "font_if.h"

/* Defines */
#ifdef FONT8x8
 extern FontX_Ank 	Misaki_Ank;
 extern FontX_Kanji Misaki_Kanji;
 #define ANKFONT	Misaki_Ank
 #define KANJIFONT	Misaki_Kanji
 
#elif  FONT10x10
 extern FontX_Ank 	Mplus_Ank;
 extern FontX_Kanji Mplus_Kanji;
 #define ANKFONT	Mplus_Ank
 #define KANJIFONT	Mplus_Kanji
 
#elif  FONT12x10
 extern FontX_Ank 	K12x10_Ank;
 extern FontX_Kanji K12x10_Kanji;
 #define ANKFONT	K12x10_Ank
 #define KANJIFONT	K12x10_Kanji
 
#elif  FONT12x12
 extern FontX_Ank 	Mplus_Ank;
 extern FontX_Kanji Mplus_Kanji;
 #define ANKFONT	Mplus_Ank
 #define KANJIFONT	Mplus_Kanji

#elif  FONT12x12D
 extern FontX_Ank 	Drug_Ank;
 extern FontX_Kanji Drug_Kanji;
 #define ANKFONT	Drug_Ank
 #define KANJIFONT	Drug_Kanji

#elif  FONT12x12K
 extern FontX_Ank 	Kaname_Ank;
 extern FontX_Kanji Kaname_Kanji;
 #define ANKFONT	Kaname_Ank
 #define KANJIFONT	Kaname_Kanji

#elif  FONT12x12L
 extern FontX_Ank 	Kodenma_Ank;
 extern FontX_Kanji Kodenma_Kanji;
 #define ANKFONT	Kodenma_Ank
 #define KANJIFONT	Kodenma_Kanji

#elif  FONT12x12M
 extern FontX_Ank 	Dohgenzaka_Ank;
 extern FontX_Kanji Dohgenzaka_Kanji;
 #define ANKFONT	Dohgenzaka_Ank
 #define KANJIFONT	Dohgenzaka_Kanji

#elif  FONT12x12W
 extern FontX_Ank 	Wlma_Ank;
 extern FontX_Kanji Wlma_Kanji;
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji
 
#elif  FONT14x14
 extern FontX_Ank 	Shinonome_Ank;
 extern FontX_Kanji Shinonome_Kanji;
 #define ANKFONT	Shinonome_Ank
 #define KANJIFONT	Shinonome_Kanji

#elif  FONT14x14M
 extern FontX_Ank 	Dohgenzaka_Ank;
 extern FontX_Kanji Dohgenzaka_Kanji;
 #define ANKFONT	Dohgenzaka_Ank
 #define KANJIFONT	Dohgenzaka_Kanji
 
#elif  FONT14x14W
 extern FontX_Ank 	Wlma_Ank;
 extern FontX_Kanji Wlma_Kanji;
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji
 
#elif  FONT16x16
 extern FontX_Ank 	Tanu_Ank;
 extern FontX_Kanji Tanu_Kanji;
 #define ANKFONT	Tanu_Ank
 #define KANJIFONT	Tanu_Kanji

#elif  FONT16x16K
 extern FontX_Ank 	Kyoukasho_Ank;
 extern FontX_Kanji Kyoukasho_Kanji;
 #define ANKFONT	Kyoukasho_Ank
 #define KANJIFONT	Kyoukasho_Kanji

#elif  FONT16x16L
 extern FontX_Ank 	Kodenma_Ank;
 extern FontX_Kanji Kodenma_Kanji;
 #define ANKFONT	Kodenma_Ank
 #define KANJIFONT	Kodenma_Kanji

#elif  FONT16x16W
 extern FontX_Ank 	Wlma_Ank;
 extern FontX_Kanji Wlma_Kanji;
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji

#elif  FONT16x16F
 extern FontX_Ank 	Honoka_Ank;
 extern FontX_Kanji Honoka_Kanji;
 #define ANKFONT	Honoka_Ank
 #define KANJIFONT	Honoka_Kanji

#elif  FONT16x16H
 extern FontX_Ank 	Hanazono_Ank;
 extern FontX_Kanji Hanazono_Kanji;
 #define ANKFONT	Hanazono_Ank
 #define KANJIFONT	Hanazono_Kanji

#elif  FONT16x16I
 extern FontX_Ank 	IL_Ank;
 extern FontX_Kanji IL_Kanji;
 #define ANKFONT	IL_Ank
 #define KANJIFONT	IL_Kanji

#elif  FONT20x20
 extern FontX_Ank 	Tanu_Ank;
 extern FontX_Kanji Tanu_Kanji;
 #define ANKFONT	Tanu_Ank
 #define KANJIFONT	Tanu_Kanji

#elif  FONT24x24I
 extern FontX_Ank 	IL_Ank;
 extern FontX_Kanji IL_Kanji;
 #define ANKFONT	IL_Ank
 #define KANJIFONT	IL_Kanji

#elif  FONT_DIGIT
 extern FontX_Ank 	Digit_Ank;
 #define ANKFONT	Digit_Ank

#endif

/* Put Font Data Tables External Devices */
#if   defined(EXT_QSPIROM_SUPPORT) /* For STM32 uC */
 #define QSPI_ROM_OFFSET	0x90000000
 /* Kanji */
 #define font_table_kanji	((const char*)(QSPI_ROM_OFFSET + 0x10000))
 /* Alphabet Numeric Kana */
 #define font_table_ank		((const char*)(QSPI_ROM_OFFSET))

#elif defined(EXT_SPIFI_SUPPORT) /* For NxP LPC uC */
 #define SPIFI_ROM_OFFSET	0x28000000
 /* Kanji */
 #define font_table_kanji	((const char*)(SPIFI_ROM_OFFSET + 0x10000))
 /* Alphabet Numeric Kana */
 #define font_table_ank		((const char*)(SPIFI_ROM_OFFSET))

#else
 /* Kanji */
 extern const char font_table_kanji[];
 /* Alphabet Numeric Kana */
 extern const char font_table_ank[];

#endif

#ifdef __cplusplus
}
#endif

#endif /* FONT_IF_DATATABLE_H */
