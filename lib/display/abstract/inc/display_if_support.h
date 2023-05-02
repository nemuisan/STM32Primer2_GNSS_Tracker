/********************************************************************************/
/*!
	@file			display_if_support.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2014.12.18
	@brief          Interface of Display Device								@n
					Draw Line & Circle Algolithm is based on under URL TNX!	@n
					http://dencha.ojaru.jp/

    @section HISTORY
		2010.12.31	V1.00	Restart Here.
		2011.03.10	V2.00	C++ Ready.
		2011.06.14	V3.00	Altanate Filer Ready.
		2011.10.14	V3.10	Chenged FontColour Function Name.
		2012.01.03	V4.00	Fixed fontkanji & fontank Relations.
		2012.04.05	V5.01	Add Draw Circle Algorithm.
		2014.12.18	V6.00	Fixed Typo and Draw-Line Bugs.
		2023.05.01	V7.00	Fixed cosmetic bugfix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef DISPLAY_IF_SUPPORT_H
#define DISPLAY_IF_SUPPORT_H 0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* Device or MCU Depend Includes */
#include "display_if.h"
#include "font_if.h"
#include "font_if_datatable.h"

/* Defines for DisplayFonts */
enum{ 
		TRANSPARENT,
		OPAQUE
	};

/* Macro Function for DrawGraphics */
/* Got Absolute */
#define ABS(x) ((x) >= 0 ? (x) : -(x))

/* Valiable of Basic Functions */
typedef struct {
	uint16_t Fg;
	uint16_t Bg;
} ScrCol_t;
/* Used in Draw Circle */
typedef struct {
	uint16_t x;
	uint16_t y;
} PLOT_XY;

/* Function Prototypes */
extern void Display_ChrCol(uint16_t fg, uint16_t bg);
/* Font Relation */
extern int Display_Puts_If(uint16_t x, uint16_t y, uint8_t* pch,uint8_t trans);
extern int Display_Puts_If_Ex(uint16_t x, uint16_t y, uint8_t* pch, uint8_t trans, uint16_t fg,uint16_t bg);
/* Draw Gfx Relation */
extern void PutPixel(uint16_t x, uint16_t y,uint16_t colour);
extern void Display_DrawRect_If(uint32_t xs, uint32_t xe, uint32_t ys, uint32_t ye, uint16_t colour);
extern void Display_FillRect_If(uint32_t xs, uint32_t xe, uint32_t ys, uint32_t ye, uint16_t colour);
extern void Display_DrawLine_If(uint32_t xs, uint32_t xe, uint32_t ys, uint32_t ye, uint16_t colour);
extern void Display_DrawCircle_If(uint16_t x_ct,uint16_t y_ct,long diameter, uint16_t colour);
extern void Display_FillCircle_If(uint16_t x_ct,uint16_t y_ct,long diameter, uint16_t colour);
/* For External Filer */
extern void putank(uint16_t x, uint16_t y, uint8_t* pank, uint16_t fg,uint16_t bg);
extern void putkanji(uint16_t x, uint16_t y, uint8_t* pkanji, uint16_t fg,uint16_t bg);

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_IF_SUPPORT_H */
