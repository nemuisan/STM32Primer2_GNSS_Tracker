/********************************************************************************/
/*!
	@file			font_if_datatable.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2015.09.01
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

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __FONT_IF_DATATABLE_H 
#define __FONT_IF_DATATABLE_H 0x0700

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef FONT8x8
 #define ANKFONT	Misaki_Ank
 #define KANJIFONT	Misaki_Kanji
 
#elif  FONT10x10
 #define ANKFONT	Mplus_Ank
 #define KANJIFONT	Mplus_Kanji
 
#elif  FONT12x10
 #define ANKFONT	K12x10_Ank
 #define KANJIFONT	K12x10_Kanji
 
#elif  FONT12x12
 #define ANKFONT	Mplus_Ank
 #define KANJIFONT	Mplus_Kanji

#elif  FONT12x12D
 #define ANKFONT	Drug_Ank
 #define KANJIFONT	Drug_Kanji

#elif  FONT12x12K
 #define ANKFONT	Kaname_Ank
 #define KANJIFONT	Kaname_Kanji

#elif  FONT12x12L
 #define ANKFONT	Kodenma_Ank
 #define KANJIFONT	Kodenma_Kanji

#elif  FONT12x12M
 #define ANKFONT	Dohgenzaka_Ank
 #define KANJIFONT	Dohgenzaka_Kanji

#elif  FONT12x12W
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji
 
#elif  FONT14x14
 #define ANKFONT	Shinonome_Ank
 #define KANJIFONT	Shinonome_Kanji

#elif  FONT14x14M
 #define ANKFONT	Dohgenzaka_Ank
 #define KANJIFONT	Dohgenzaka_Kanji
 
#elif  FONT14x14W
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji
 
#elif  FONT16x16
 #define ANKFONT	Tanu_Ank
 #define KANJIFONT	Tanu_Kanji

#elif  FONT16x16K
 #define ANKFONT	Kyoukasho_Ank
 #define KANJIFONT	Kyoukasho_Kanji

#elif  FONT16x16L
 #define ANKFONT	Kodenma_Ank
 #define KANJIFONT	Kodenma_Kanji

#elif  FONT16x16W
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji

#elif  FONT16x16F
 #define ANKFONT	Honoka_Ank
 #define KANJIFONT	Honoka_Kanji

#elif  FONT16x16H
 #define ANKFONT	Hanazono_Ank
 #define KANJIFONT	Hanazono_Kanji

#elif  FONT16x16I
 #define ANKFONT	IL_Ank
 #define KANJIFONT	IL_Kanji

#elif  FONT20x20
 #define ANKFONT	Tanu_Ank
 #define KANJIFONT	Tanu_Kanji

#elif  FONT24x24I
 #define ANKFONT	IL_Ank
 #define KANJIFONT	IL_Kanji

#elif  FONT_DIGIT
 #define ANKFONT	Digit_Ank

#endif

/* Put Font Data Tables */
#if   defined(EXT_QSPIROM_SUPPORT)
 #define QSPI_ROM_OFFSET	0x90000000
 /* Kanji */
 #define font_table_kanji	((const char*)(QSPI_ROM_OFFSET + 0x10000))
 /* Alphabet Numeric Kana */
 #define font_table_ank		((const char*)(QSPI_ROM_OFFSET))

#elif defined(EXT_SPIFI_SUPPORT)
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

#endif /* __FONT_IF_DATATABLE_H */
