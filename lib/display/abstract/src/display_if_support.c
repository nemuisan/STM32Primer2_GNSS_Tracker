/********************************************************************************/
/*!
	@file			display_if_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2024.08.01
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
		2023.08.01	V8.00	Revised release.
		2024.08.01	V9.00	Fixed drawcircle and font function.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "display_if_support.h"
/* check header file version for fool proof */
#if DISPLAY_IF_SUPPORT_H != 0x0900
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
/* See Also "display_if_basis.h" */
ScrCol_t ScrCol = { FrontG, BackG };
ScrCol_t* pScrCol = &ScrCol;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
/* Got Integer Route of Neuton method */
static inline int root_i(int x){
    int s=1, s2=1;
    do {
        s=(x/s+s)/2;
        s2=s+1;
        if (s*s<=x && x<s2*s2) break;
    } while(1);
    return s;
}
static inline double root(double x){
    double s=1, s2=1;
    if (x<=0) return 1;
    do { s2=s; s=(x/s+s)/2; 
		if((fabs(s2 - s) < DBL_EPSILON )) break; 
	} while(1);
    return s;
}

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
	Set FrontGraphic and BackGraphic(Basis Function).
*/
/**************************************************************************/
inline void Display_ChrCol(uint16_t fg, uint16_t bg)
{
	pScrCol->Fg = fg;
	pScrCol->Bg = bg;
}

/**************************************************************************/
/*! 
	Put Pixel Position(Basis Function).
*/
/**************************************************************************/
static inline void SetPixel(uint16_t x, uint16_t y)
{
	Display_rect_if(x,x,y,y);
}

/**************************************************************************/
/*! 
	Put Pixel.
*/
/**************************************************************************/
inline void PutPixel(uint16_t x, uint16_t y,uint16_t colour)
{
	SetPixel(x,y);
	Display_wr_dat_if(colour);
}


/**************************************************************************/
/*! 
	DRAW Rectangle Line with no Filler.
*/
/**************************************************************************/
inline void Display_DrawRect_If(uint32_t xs, uint32_t xe, uint32_t ys, uint32_t ye, uint16_t colour)
{
	volatile uint32_t n;

	if((xe < xs) || (ye < ys) ) return;

	n = xe - xs;
	do{
		PutPixel((xe-n),ys,colour);
	} while (n--);

	n = xe - xs;
	do{
		PutPixel((xe-n),ye,colour);
	} while (n--);

	n = ye - ys;
	do{
		PutPixel(xs,(ye-n),colour);
	} while (n--);

	n = ye - ys;
	do{
		PutPixel(xe,(ye-n),colour);
	} while (n--);
	
}

/**************************************************************************/
/*! 
	Fill Rectangle.
*/
/**************************************************************************/
inline void Display_FillRect_If(uint32_t xs, uint32_t xe, uint32_t ys, uint32_t ye, uint16_t colour)
{
	volatile uint32_t n;

	if((xe < xs) || (ye < ys) ) return;
	
	Display_rect_if(xs,xe,ys,ye);
	n = (xe-xs+1) * (ye-ys+1);
	
	do {
		Display_wr_dat_if(colour);
	} while (--n);

}


/**************************************************************************/
/*! 
	DRAW Line Function(There is no cripper yet).
*/
/**************************************************************************/
inline void Display_DrawLine_If(uint32_t xs, uint32_t xe, uint32_t ys, uint32_t ye, uint16_t colour)
{
	/* Bresenham Algorithm */
	int  wwx,hhy,x,y,n,sx,sy,e;
	uint16_t dx,dy;

	wwx = (int)(xe - xs);
	hhy = (int)(ye - ys);
	dx  = ABS(wwx);
	dy  = ABS(hhy);

	if (wwx > 0) sx = 1; else sx = -1;
	if (hhy > 0) sy = 1; else sy = -1;

	x = xs;
	y = ys;

	if ( dx >= dy )
	{
		e = dx;

		for(n=0; n<=dx; ++n){

			PutPixel(x,y,colour);
			x += sx;
			e += 2*dy;

			if (e >= 2*dx){
				e -= 2*dx;
				y += sy;
			}
		}

	}
	else
	{
		e = dy;

		for(n=0; n<=dy; ++n){

			PutPixel(x,y,colour);
			y += sy;
			e += 2*dx;

			if (e>=2*dy){
				e -= 2*dy;
				x += sx;
			}
		}
	}

}

/**************************************************************************/
/*! 
	DRAW Circle Function With no Filler.
*/
/**************************************************************************/
#if 1 /* More Accuracy method by http://dencha.ojaru.jp thanks! */
inline void Display_DrawCircle_If(uint16_t x_ct,uint16_t y_ct,long diameter, uint16_t colour)
{
    PLOT_XY center;
    center.x=x_ct;
    center.y=y_ct;
    long cx = 0, cy=diameter/2, *px, *py, tmp;
    long dx, dy, x_sign, num_eigth, r_root2,y_sign =0;
    double d;

    r_root2 = (diameter>3)? root_i(diameter*diameter/8) :1;
    tmp = r_root2*r_root2*8-diameter*diameter;
    if (ABS(tmp)>ABS(tmp+8*(2*r_root2+1))) r_root2++;	/* near by (r*Å„2) */

    d = -diameter*diameter + 4*cy*cy -4*cy +2;
    dx = 4;
    dy = -8*cy+8;
	
    PLOT_XY mirror_center = center, now_center, start_po[8], end_po[8];
    if ((diameter&1) ==0){
        mirror_center.x++;
        mirror_center.y++;
    }

    /* Clipper */
    for(num_eigth=0; num_eigth <8; num_eigth++){
        start_po[num_eigth].y=diameter/2;
        start_po[num_eigth].x=0;
        end_po[num_eigth].x=end_po[num_eigth].y=r_root2;
    }

    for(int li=0;li<4;li++){
        if (li==0) { cy = center.y-(MAX_Y-1); y_sign=-1;}
        if (li==1) { cy = mirror_center.x;    y_sign= 1;}
        if (li==2) { cy = mirror_center.y;    y_sign= 1;}
        if (li==3) { cy = center.x-(MAX_X-1); y_sign=-1;}

        if (ABS(cy)>=(diameter/2)) {
            if (((li==0 || li==3) && cy<0) || ((li==1 || li==2) && cy>0)) continue;	/* Circle is Inside of Region */
            else return ;	/* Circle is Outside of Region */
        }

        tmp = diameter*diameter -4*cy*cy;
        cx = root_i(tmp/4); /* n=tmp/4; if (tmp%4) n++; */
        tmp -= 4*cx*cx;
        if (ABS(tmp)>=ABS(tmp -8*cx-4)) cx++;

		/* Clipper Divided Octal Regions */
        if (cy*y_sign>r_root2){
            /* 1,2 -> 3,4 -> 5,6 -> 7,0 */
            if (start_po[li*2+1].x<ABS(cx)) {
                start_po[li*2+1].y = ABS(cy);
                start_po[li*2+1].x = ABS(cx);
            }
            if (start_po[(li*2+2)%8].x<ABS(cx)) {
                start_po[(li*2+2)%8].y = ABS(cy);
                start_po[(li*2+2)%8].x = ABS(cx);
            }
        }
        else {
            start_po[li*2+1].y = start_po[(li*2+2)%8].y = 0;        /* Set Out of Region */
            start_po[li*2+1].x = start_po[(li*2+2)%8].x = diameter;	/* Set Out of Region */
            if (cy*y_sign<=r_root2 && cy*y_sign>0){
                /* Set Out of Region... 1,2 -> 3,4 -> 5,6 -> 7,0 */
                /* 0,3 -> 2,5 -> 4,7 -> 6,1 */
                if (end_po[li*2].x > ABS(cy)) {
                    end_po[li*2].y = ABS(cx);
                    end_po[li*2].x = ABS(cy);
                }
                if (end_po[(li*2+3)%8].x > ABS(cy)) {
                    end_po[(li*2+3)%8].y = ABS(cx);
                    end_po[(li*2+3)%8].x = ABS(cy);
                }
            }
            else {
                start_po[li*2].y = start_po[(li*2+3)%8].y = 0;
                start_po[li*2].x = start_po[(li*2+3)%8].x = diameter;
                if (cy*y_sign<=0 && cy*y_sign>-r_root2){
				    /* Set Out of Region... 0,3 -> 2,5 -> 4,7 -> 6,1 */
					/* 4,7 -> 6,1 -> 0,3 -> 2,5 */
                    if (start_po[(li*2+4)%8].x < ABS(cy)) {
                        start_po[(li*2+4)%8].y = ABS(cx);
                        start_po[(li*2+4)%8].x = ABS(cy);
                    }
                    if (start_po[(li*2+7)%8].x < ABS(cy)) {
                        start_po[(li*2+7)%8].y = ABS(cx);
                        start_po[(li*2+7)%8].x = ABS(cy);
                    }
                }
                else {
                    start_po[(li*2+4)%8].y = start_po[(li*2+7)%8].y = 0;
                    start_po[(li*2+4)%8].x = start_po[(li*2+7)%8].x = diameter;
					/* Set Out of Region...  4,7 -> 6,1 -> 0,3 -> 2,5 */
					/* 5,6 -> 7,0 -> 1,2 -> 3,4 */
                    if (end_po[(li*2+5)%8].x>ABS(cx)) {
                        end_po[(li*2+5)%8].y = ABS(cy);
                        end_po[(li*2+5)%8].x = ABS(cx);
                    }
                    if (end_po[(li*2+6)%8].x>ABS(cx)) {
                        end_po[(li*2+6)%8].y = ABS(cy);
                        end_po[(li*2+6)%8].x = ABS(cx);
                    }
                }
            }
        }
    }

    /* Draw Circle main loop */
    for(num_eigth=0; num_eigth<8 ; num_eigth++){
		/* Circle Divided Octal Regions */
        if (num_eigth<4){
                now_center.y = y_ct; y_sign=1; }            /* 0,1,2,3 */
        else{   now_center.y = mirror_center.y; y_sign=-1; }    /* 4,5,6,7 */
        if ((num_eigth%6)<=1){
                now_center.x = x_ct;  x_sign=1; }           /* 0,1,6,7 */
        else {  now_center.x = mirror_center.x; x_sign=-1; }    /* 2,3,4,5 */
        if ((num_eigth%4)%3){
                px = &cx; py = &cy; }   /* 0,3,4,7 */
        else {  px = &cy; py = &cx; }   /* 1,2,5,6 */

        /* initial value  */
        cy=start_po[num_eigth].y;
        cx=start_po[num_eigth].x;

        /* diameter */
        d = 4*cx*cx+4*cy*cy -4*cy+2 - diameter * diameter;
        dx = 8*cx+4;
        dy = -8*cy+8;

        /* Set Point */
        for (;cx<=end_po[num_eigth].x;cx++){
            if (d>0) {
                d += dy;
                dy+=8;
                cy--;
            }
            PutPixel((*px)*x_sign + now_center.x, (*py)*y_sign + now_center.y, colour);
            d += dx;
            dx+=8;
        }
    }
}

#else
inline void Display_DrawCircle_If(uint16_t x_ct,uint16_t y_ct,long diameter, uint16_t colour)
{
	/* Bresenham Midpoint Algorithm */
   long cx, cy, d, dH, dD;

    d   = 1 - radius;
    dH  = 3;
    dD  = 5 - 2 * radius;
    cy  = radius;

    for (cx = 0; cx <= cy; cx++) {
        if (d < 0) {
            d   += dH;
            dH  += 2;
            dD  += 2;
        }
        else{
            d   += dD;
            dH  += 2;
            dD  += 4;
            --cy;
        }

        PutPixel( cy + x_ct,  cx + y_ct, colour);	/* Between   0- 45 */
        PutPixel( cx + x_ct,  cy + y_ct, colour);	/* Between  45- 90 */
        PutPixel(-cx + x_ct,  cy + y_ct, colour);	/* Between  90-135 */
        PutPixel(-cy + x_ct,  cx + y_ct, colour);	/* Between 135-180 */
        PutPixel(-cy + x_ct, -cx + y_ct, colour);	/* Between 180-225 */
        PutPixel(-cx + x_ct, -cy + y_ct, colour);	/* Between 225-270 */
        PutPixel( cx + x_ct, -cy + y_ct, colour);	/* Between 270-315 */
        PutPixel( cy + x_ct, -cx + y_ct, colour);	/* Between 315-360 */
    }
}
#endif

/**************************************************************************/
/*! 
	DRAW Circle with Filler.
*/
/**************************************************************************/
inline void Display_FillCircle_If(uint16_t x_ct,uint16_t y_ct,long diameter, uint16_t colour)
{
	/* Bresenham Midpoint Algorithm */
	long cx, cy, d, dH, dD, n;
	long radius= diameter/2;
	
    d   = 1 - radius;
    dH  = 3;
    dD  = 5 - 2 * radius;
    cy  = radius;

    for (cx = 0; cx <= cy; cx++) {
        if (d < 0) {
            d   += dH;
            dH  += 2;
            dD  += 2;
        }
        else{
            d   += dD;
            dH  += 2;
            dD  += 4;
            --cy;
        }

		/* Between 0-45deg */
		n = 2*cy;
		do{
			PutPixel((cy-n)+ x_ct,cx + y_ct,colour);
		} while (n--);

		/* Between 45-90deg */
		n = 2*cx;
		do{
			PutPixel((cx-n)+ x_ct,cy + y_ct,colour);
		} while (n--);

		/* Between 270-315deg */
		n = 2*cx;
		do{
			PutPixel((cx-n)+ x_ct,-cy + y_ct,colour);
		} while (n--);

		/* Between 315-360deg */
		n = 2*cy;
		do{
			PutPixel((cy-n)+ x_ct,-cx + y_ct,colour);
		} while (n--);

    }
}

/**************************************************************************/
/*! 
    Draw Windows 24bitBMP File.
*/
/**************************************************************************/
int Display_DrawBmp_If(const uint8_t* ptr)
{
	uint32_t n, m, biofs, bw, iw, bh, w;
	uint32_t xs, xe, ys, ye, i;
	uint8_t *p;
	uint16_t d;

	/* Load BitStream Address Offset  */
	biofs = LD_UINT32(ptr+10);
	/* Check Plane Count "1" */
	if (LD_UINT16(ptr+26) != 1)  return 0;
	/* Check BMP bit_counts "24(windows bitmap standard)" */
	if (LD_UINT16(ptr+28) != 24) return 0;
	/* Check BMP Compression "BI_RGB(no compresstion)"*/
	if (LD_UINT32(ptr+30) != 0)  return 0;
	/* Load BMP width */
	bw = LD_UINT32(ptr+18);
	/* Load BMP height */
	bh = LD_UINT32(ptr+22);
	/* Check BMP width under 1280px */
	if (!bw || bw > 1280 || !bh) return 0;
	
	/* Calculate Data byte count per holizontal line */
	iw = ((bw * 3) + 3) & ~3;

	/* Centering */
	if (bw > MAX_X) {
		xs = 0; xe = MAX_X-1;
	} else {
		xs = (MAX_X - bw) / 2;
		xe = (MAX_X - bw) / 2 + bw - 1;
	}
	if (bh > MAX_Y) {
		ys = 0; ye = MAX_Y-1;
	} else {
		ys = (MAX_Y - bh) / 2;
		ye = (MAX_Y - bh) / 2 + bh - 1;
	}

	/* Clear Display */
	Display_clear_if();

    /* Limit to MAX_Y */
	m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y) * iw;

	/* Set First data offset */  
	i = m % 512;

	/* Set BMP's Bottom-Left point */
	m = ye;

    /* Limit MAX_X */
	w = (bw > MAX_X) ? MAX_X : bw;

	do {
		Display_rect_if(xs, xe, m, m);
		n = 0;
		p = (uint8_t*)ptr + i;
		do {
			n++;
			/* Decode to 65k(16bit) colour */
		#if !defined(USE_SSD1332_SPI_OLED)
			d = *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
		#else                            
			d =  (*p++ >> 3) << 11;
			d |= (*p++ >> 2) << 5;
			d |=  *p++ >> 3;
		#endif
			Display_wr_dat_if(d);
		} while (n < w);
		i += iw;

	} while (m-- > ys);

	return 1;
}

/**************************************************************************/
/*! 
	Put SJIS Kanji Font With Transparency.
*/
/**************************************************************************/
static void putkanji_t(uint16_t x, uint16_t y, uint8_t* pkanji, uint16_t colour)
{

	uint8_t*	ptemp;
	int 		i,j,l;
	int 		k,m;

	l = CurrentKanjiDat->X_Size / 8;
	m = CurrentKanjiDat->X_Size % 8;

	ptemp = GetPtr_Kanji(((*pkanji) << 8) | *(pkanji+1));

	for(i=0; i<CurrentKanjiDat->Y_Size;i++)
	{
		for(k=0; k < l ;k++){
			
			for(j=0; j< 8;j++){

				if((*(ptemp)<<j)&0x80){
						SetPixel((x+j+(k*8)),(y+i));
						Display_wr_dat_if(colour);
					}
			}
			ptemp++;
		}

		for(j=0; j< m ;j++){

			if((*(ptemp)<<j)&0x80){
					SetPixel((x+j+(k*8)),(y+i));
					Display_wr_dat_if(colour);
				}
				
		}
		
		if (m != 0) ptemp++;	
	
	}

}


/**************************************************************************/
/*! 
	Put SJIS ANK Font With Transparency.
*/
/**************************************************************************/
static void putank_t(uint16_t x, uint16_t y, uint8_t* pank, uint16_t colour)
{

	uint8_t*	ptemp;
	int 		i,j,l;
	int 		k,m;

	l = CurrentAnkDat->X_Size / 8;
	m = CurrentAnkDat->X_Size % 8;

	ptemp = GetPtr_Ank(*pank);

	for(i=0; i<CurrentAnkDat->Y_Size;i++)
	{
		for(k=0; k < l ;k++){
			
			for(j=0; j< 8;j++){

				if((*(ptemp)<<j)&0x80){
						SetPixel((x+j+(k*8)),(y+i));
						Display_wr_dat_if(colour);
					}
			}
			ptemp++;
		}

		for(j=0; j< m ;j++){

			if((*(ptemp)<<j)&0x80){
					SetPixel((x+j+(k*8)),(y+i));
					Display_wr_dat_if(colour);
				}
		}
		if (m != 0) ptemp++;	

	}
}


/**************************************************************************/
/*! 
	Put SJIS Kanji Font With Occupy.
*/
/**************************************************************************/
void putkanji(uint16_t x, uint16_t y, uint8_t* pkanji, uint16_t fg,uint16_t bg)
{

	uint8_t*	ptemp;
	int 		i,j,l;
	int 		k,m;
	uint16_t 	wd;

	l = CurrentKanjiDat->X_Size / 8;
	m = CurrentKanjiDat->X_Size % 8;

	ptemp = GetPtr_Kanji(((*pkanji) << 8) | *(pkanji+1));
	
	Display_rect_if(x, x + CurrentKanjiDat->X_Size - 1,
					y, y + CurrentKanjiDat->Y_Size - 1);


	for(i=0; i<CurrentKanjiDat->Y_Size;i++)
	{
		for(k=0; k < l ;k++){
			
			for(j=0; j< 8;j++){
				wd = (((*(ptemp))<<j)&0x80) ?  fg : bg;
			#if defined(DISABLE_FASTWRITE)
				SetPixel((x+j+(k*8)),(y+i));
			#endif
				Display_wr_dat_if(wd);
			}
			ptemp++;
		}

		for(j=0; j< m ;j++){
				wd = (((*(ptemp))<<j)&0x80) ?  fg : bg;
			#if defined(DISABLE_FASTWRITE)
				SetPixel((x+j+(k*8)),(y+i));
			#endif
				Display_wr_dat_if(wd);
		}

		if (m != 0) ptemp++;	
	}

}


/**************************************************************************/
/*! 
	Put SJIS ANK Font With Occupy.
*/
/**************************************************************************/
void putank(uint16_t x, uint16_t y, uint8_t* pank, uint16_t fg,uint16_t bg)
{

	uint8_t*	ptemp;
	int 		i,j,l;
	int 		k,m;
	uint16_t 	wd;

	l = CurrentAnkDat->X_Size / 8;
	m = CurrentAnkDat->X_Size % 8;

	ptemp = GetPtr_Ank(*pank);

	Display_rect_if(x, x + CurrentAnkDat->X_Size - 1,
					y, y + CurrentAnkDat->Y_Size - 1);


	for(i=0; i<CurrentAnkDat->Y_Size;i++)
	{
		for(k=0; k < l ;k++){
			
			for(j=0; j< 8;j++){
				wd = (((*(ptemp))<<j)&0x80) ?  fg : bg;
			#if defined(DISABLE_FASTWRITE)
				SetPixel((x+j+(k*8)),(y+i));
			#endif
				Display_wr_dat_if(wd);
			}
			ptemp++;
		}

		for(j=0; j< m ;j++){
				wd = (((*(ptemp))<<j)&0x80) ?  fg : bg;
			#if defined(DISABLE_FASTWRITE)
				SetPixel((x+j+(k*8)),(y+i));
			#endif
				Display_wr_dat_if(wd);
		}

		if (m != 0) ptemp++;	
	}

}


/**************************************************************************/
/*! 
	Put Shift-JIS Fonts Basic.
*/
/**************************************************************************/
int Display_Puts_If(uint16_t x, uint16_t y, uint8_t* pch, uint8_t trans)
{

	while(*pch){

		if (x >= MAX_X || y >= MAX_Y) return 0;

		if(((*pch >= 0x81)&&(*pch <= 0x9F))||((*pch >= 0xE0)&&(*pch <= 0xFC)))
	  		{
				if(trans == TRANSPARENT) putkanji_t(x,y,pch,pScrCol->Fg);
				else					 putkanji  (x,y,pch,pScrCol->Fg,pScrCol->Bg);
				
				pch +=2;
				x += CurrentKanjiDat->X_Size;
	  		}
		else
			{
				if(trans == TRANSPARENT) putank_t(x,y,pch,pScrCol->Fg);
				else					 putank  (x,y,pch,pScrCol->Fg,pScrCol->Bg);

				pch +=1;
				x += CurrentAnkDat->X_Size;
			}
	}

	return 1;
}

/**************************************************************************/
/*! 
	Put Shift-JIS Fonts Advanced.
*/
/**************************************************************************/
int Display_Puts_If_Ex(uint16_t x, uint16_t y, uint8_t* pch, uint8_t trans, uint16_t fg,uint16_t bg)
{

	while(*pch){

		if (x >= MAX_X || y >= MAX_Y) return 0;

		if(((*pch >= 0x81)&&(*pch <= 0x9F))||((*pch >= 0xE0)&&(*pch <= 0xFC)))
	  		{
				if(trans == TRANSPARENT) putkanji_t(x,y,pch,fg);
				else					 putkanji  (x,y,pch,fg,bg);
				
				pch +=2;
				x += CurrentKanjiDat->X_Size;
	  		}
		else
			{
				if(trans == TRANSPARENT) putank_t(x,y,pch,fg);
				else					 putank  (x,y,pch,fg,bg);

				pch +=1;
				x += CurrentAnkDat->X_Size;
			}
	}

	return 1;
}

/* End Of File ---------------------------------------------------------------*/
