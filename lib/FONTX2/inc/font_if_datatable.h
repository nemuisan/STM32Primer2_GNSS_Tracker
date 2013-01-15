/********************************************************************************/
/*!
	@file			font_if_datatable.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2012.06.30
	@brief          Interface of FONTX Driver								@n
                    Refered under URL thanks!								@n
					http://www.hmsoft.co.jp/lepton/software/dosv/fontx.htm	@n
					http://hp.vector.co.jp/authors/VA007110/technicalworkshop

    @section HISTORY
		2011.03.11	V1.00	Stable Release.
		2011.05.11	V2.00	Selectable KanjiFonts to Reduce Memory Space.
		2012.03.12	V3.00	Add More Fonts.
		2012.06.30	V4.00	Add External SRAM Support for KanjiFonts.
 
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __FONT_IF_DATATABLE_H 
#define __FONT_IF_DATATABLE_H 0x0400

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
 
#elif  FONT12x12K
 #define ANKFONT	Kaname_Ank
 #define KANJIFONT	Kaname_Kanji
 
#elif  FONT12x12W
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji
 
#elif  FONT14x14
 #define ANKFONT	Shinonome_Ank
 #define KANJIFONT	Shinonome_Kanji
 
#elif  FONT14x14W
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji
 
#elif  FONT16x16
 #define ANKFONT	Tanu_Ank
 #define KANJIFONT	Tanu_Kanji
 
#elif  FONT16x16W
 #define ANKFONT	Wlma_Ank
 #define KANJIFONT	Wlma_Kanji
 
#elif  FONT20x20
 #define ANKFONT	Tanu_Ank
 #define KANJIFONT	Tanu_Kanji
 
#elif  FONT_DIGIT
 #define ANKFONT	Digit_Ank

#endif

/* Put Font Data Tables */
/* Kanji */
extern const char font_table_kanji[];
/* Alphabet Numeric Kana */
extern const char font_table_ank[];


#ifdef __cplusplus
}
#endif

#endif /* __FONT_IF_DATATABLE_H */
