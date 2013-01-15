/********************************************************************************/
/*!
	@file			font_if_datatable.c
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

/* Includes ------------------------------------------------------------------*/
#include "font_if.h"

/* Defines   -----------------------------------------------------------------*/
#ifdef EXT_SRAM_SUPPORT
 #define _EXRAM  __attribute__ ((section(".extram")))
#else
 #define _EXRAM
#endif

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/
#if defined(FONT8x8)
#warning "USE MISAKI FONTS(8x8)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "misaki.h"
	};
	FontX_Kanji Misaki_Kanji _EXRAM;
#endif
const char font_table_ank[] = { 
	#include "4x8.h" 
};
FontX_Ank   Misaki_Ank;

#elif defined(FONT10x10)
#warning "USE M+ FONTS(10x10)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "MPLZN10X.h"
	};
	FontX_Kanji Mplus_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "MPLHN10X.h"
};
FontX_Ank   Mplus_Ank;

#elif defined(FONT12x10)
#warning "USE K12x10 FONTS(12x10)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "k12x10.h"
	};
	FontX_Kanji K12x10_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "k6x10.h"
};
FontX_Ank   K12x10_Ank;

#elif defined(FONT12x12)
#warning "USE M+ FONTS(12x12)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "MPLZN12X.h"
	};
	FontX_Kanji Mplus_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "MPLHN12X.h"
};
FontX_Ank   Mplus_Ank;

#elif defined(FONT12x12W)
#warning "USE WADA-KEN HOSOMARU GOTHIC FONTS(12x12)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "wlma212k.h"
	};
	FontX_Kanji Wlma_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "wlma212a.h"
};
FontX_Ank   Wlma_Ank;

#elif defined(FONT12x12K)
#warning "USE KANAMECHOU FONTS(12x12)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "KNMZN12X.h"
	};
	FontX_Kanji Kaname_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "KNMHN12X.h"
};
FontX_Ank   Kaname_Ank;

#elif defined(FONT14x14)
#warning "USE SHINONOME GOTHIC FONTS(14x14)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "SHGZN14X.h"
	};
	FontX_Kanji Shinonome_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "SHNHN14X.h"
};
FontX_Ank   Shinonome_Ank;

#elif defined(FONT14x14W)
#warning "USE WADA-KEN HOSOMARU GOTHIC FONTS(14x14)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "wlma214k.h"
	};
	FontX_Kanji Wlma_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "wlma214a.h"
};
FontX_Ank   Wlma_Ank;

#elif defined(FONT16x16)
#warning "USE TANUKI MAGIC FONTS(16x16)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "TANUZN16.h"
	};
	FontX_Kanji Tanu_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "TANUHN16.h"
};

FontX_Ank   Tanu_Ank;
#elif defined(FONT16x16W)
#warning "USE WADA-KEN HOSOMARU GOTHIC FONTS(16x16)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "wlma216k.h"
	};
	FontX_Kanji Wlma_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "wlma216a.h"
};
FontX_Ank   Wlma_Ank;

#elif defined(FONT20x20)
#warning "USE TANUKI MAGIC FONTS(20x20)"
#ifdef USE_KANJIFONT
	const char font_table_kanji[] = {
		#include "TANUZN20.h"
	};
	FontX_Kanji Tanu_Kanji _EXRAM;
#endif
const char font_table_ank[] = {
	#include "TANUHN20.h"
};
FontX_Ank   Tanu_Ank;

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
