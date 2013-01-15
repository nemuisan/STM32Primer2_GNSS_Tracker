/********************************************************************************/
/*!
	@file			font_if.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.01
    @date           2011.09.17
	@brief          Interface of FONTX Driver								@n
                    Refered under URL thanks!								@n
					http://www.hmsoft.co.jp/lepton/software/dosv/fontx.htm	@n
					http://hp.vector.co.jp/authors/VA007110/technicalworkshop

    @section HISTORY
		2010.12.31	V1.00	Stable Release.
		2011.03.10	V2.00	C++ Ready.
		2011.05.11	V3.00	Selectable KanjiFonts to Reduce Memory Space.
		2011.09.17	V3.01	Fixed handling Selectable KanjiFonts.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "font_if.h"

/* Defines   -----------------------------------------------------------------*/
#ifdef USE_KANJIFONT
 #warning "USE_KANJIFONTX2"
#else
 #warning "USE_ANKFONTX2 ONLY"
#endif

/* Variables -----------------------------------------------------------------*/
FontX_Ank*   CurrentAnkDat;
FontX_Kanji* CurrentKanjiDat;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    FontX2 Ank Initialize
*/
/**************************************************************************/
void InitFont_Ank(FontX_Ank* AnkDat,const char* addr_ofs)
{
	/* Store ANK Addres Offset */
	AnkDat->AnkFileOffset = addr_ofs;
	
	/* Store ANK Pixel Size */
	AnkDat->X_Size		  = READ_ADDR_UNIT8(ANK_XSIZE);
	AnkDat->Y_Size		  = READ_ADDR_UNIT8(ANK_YSIZE);

	/* Store ANK Byte Size */
	if(( (AnkDat->X_Size) % 8)){
		AnkDat->AnkSize =  ( (((AnkDat->X_Size) >> 3)+1) * (AnkDat->Y_Size));
	}
	else{
		AnkDat->AnkSize =  ( (((AnkDat->X_Size) >> 3))   * (AnkDat->Y_Size));
	}

}


/**************************************************************************/
/*! 
    Change Ank Fonts Pointe
*/
/**************************************************************************/
void ChangeCurrentAnk(FontX_Ank* AnkDat){

	CurrentAnkDat = AnkDat;
}


/**************************************************************************/
/*! 
   Get Ank Address Pointer
*/
/**************************************************************************/
uint8_t* GetPtr_Ank(uint8_t AnkCode){

	return ( (uint8_t*)((CurrentAnkDat->AnkFileOffset + ANK_DATSTART) + (AnkCode * CurrentAnkDat->AnkSize)) );
}


/**************************************************************************/
/*! 
   FontX2 Kanji Initialize
*/
/**************************************************************************/
void InitFont_Kanji(FontX_Kanji* KanjiDat,const char* addr_ofs)
{
	int i;

	/* Store Kanji Addres Offset */
	KanjiDat->KanjiFileOffset = addr_ofs;

	/* Store Kanji Pixel Size */
	KanjiDat->X_Size		  = READ_ADDR_UNIT8(KANJI_XSIZE);
	KanjiDat->Y_Size		  = READ_ADDR_UNIT8(KANJI_YSIZE);

	/* Store Kanji Byte Size */
	if(( (KanjiDat->X_Size) % 8)){
		KanjiDat->KanjiSize =  ( (((KanjiDat->X_Size) >> 3)+1) * (KanjiDat->Y_Size));
	}
	else{
		KanjiDat->KanjiSize =  ( (((KanjiDat->X_Size) >> 3))   * (KanjiDat->Y_Size));
	}

	/* Store Kanji Table Number */
	KanjiDat->KanjiTableNum = READ_ADDR_UNIT8(KANJI_TABLENUM);
	
	/* Store Kanji Search Table */
	KanjiDat->KanjiStartOffset =  (KanjiDat->KanjiFileOffset + (KanjiDat->KanjiTableNum  * 4) + KANJI_DATSTART);
	
	KanjiDat->KanjiSearchTable[0] = 0;

	for (i=1;i<KanjiDat->KanjiTableNum;i++){
		KanjiDat->KanjiSearchTable[i] = (KANJI_ENDE(i-1) - KANJI_START(i-1) + 1) + KanjiDat->KanjiSearchTable[i-1];
	}
}


/**************************************************************************/
/*! 
   Change Kanji Fonts Pointer 
*/
/**************************************************************************/
void ChangeCurrentKanji(FontX_Kanji* KanjiDat){

	CurrentKanjiDat = KanjiDat;
}


/**************************************************************************/
/*! 
  Search Kanji Block Table 
*/
/**************************************************************************/
static int SearchKanjiBlock(uint16_t SjisCode){
	int i;

	for (i=0;i<CurrentKanjiDat->KanjiTableNum;i++){

		if((KANJI_START_C(i) <= SjisCode) && (KANJI_ENDE_C(i) >= SjisCode)){
			return(i);
		}
	}
	return (-1);
}


/**************************************************************************/
/*! 
  Get Kanji Address Pointer 
*/
/**************************************************************************/
uint8_t* GetPtr_Kanji(uint16_t SjisCode){

	int FontBlock;
	uint32_t KanjiDatOfs;

	/* Store Kanji Font Block */
	FontBlock = SearchKanjiBlock(SjisCode);

	/* Invalide SJIS code change into TOOFU */
	if (FontBlock == -1){
		SjisCode = SJIS_TOOFU;
		/* Restore SjisCode as TOOFU */
		FontBlock = SearchKanjiBlock(SjisCode);
	}

	/* Store Relative Font Address Offset */
	KanjiDatOfs = (CurrentKanjiDat->KanjiSearchTable[FontBlock] + (SjisCode - KANJI_START_C(FontBlock))) * CurrentKanjiDat->KanjiSize;

	/* Return Absolute Font Address Offset */
	return((uint8_t*)(CurrentKanjiDat->KanjiStartOffset + KanjiDatOfs));

}

/* End Of File ---------------------------------------------------------------*/
