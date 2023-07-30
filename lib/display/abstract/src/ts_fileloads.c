/********************************************************************************/
/*!
	@file			ts_fileloads.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        25.00
    @date           2023.07.23
	@brief          Filer and File Loaders.

    @section HISTORY
		2023.07.23	See ts_ver.txt.

    @section LICENSE
		BSD License + IJG JPEGLIB license See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ts_fileloads.h"
/* check header file version for fool proof */
#if TS_FILELOADS_H != 0x2500
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
/* Used for Filer */
typedef struct {
	uint32_t fsize;
	uint8_t  fattr;
#if FF_USE_LFN
	char fname[FF_MAX_LFN+1];
#else
	char fname[13];
#endif
} DIRITEM;

#if FF_USE_LFN
 #define MAX_DIR_ITEM (((BUFSIZE/sizeof(DIRITEM)) > 200) ? 200 : (BUFSIZE/sizeof(DIRITEM)) )
#else
 #define MAX_DIR_ITEM (200)
#endif

/* Used for TEXT Viewer */
typedef struct {
	uint8_t  sbuf[512];
	uint32_t ltbl[1];
} TXVIEW;

/* Array Buff[] must be declared FatFs Basic Routine!(e.g. in ff_func_disp.c) */
extern uint8_t Buff[];

/* Set Performance Counter(for Testing and MCU dependent) */
#ifdef ENABLE_PERFORMANCE_MEASUREMENT
#warning "Enable Display Performance Counter!"
 extern void display_performance_ready_if(void);
 extern uint32_t display_performance_result_if(void);
#endif

/* Check using floating point unit on some graphic libraries */
/* Use double-precision hardware floating point unit on libpng? */
#if defined(LIBPNG_USE_DFPU)
 #warning "Use Hardware FloatingPoint Unit on LIBPNG!"
#endif
/* Use single/double-precision hardware floating point unit on libpng? */
#if defined (LIBJPEG_USE_FPU) || defined(LIBJPEG_USE_DFPU)
 #warning "Use Hardware FloatingPoint Unit on LIBJPEG!"
#endif

/* Variables -----------------------------------------------------------------*/
/* Used for Cursor and Original Movies */
extern uint8_t  CsrFlag;
extern uint32_t TmrFrm,TmrCsr;
extern uint16_t Row, Col, Attr;

/* Used for Filer VRAM */
extern uint16_t Vram[TS_HEIGHT][TS_WIDTH];

#if defined(USE_TFT_FRAMEBUFFER)
#if ((MAX_X*MAX_Y*2) > BUFSIZE)
 #define VBUFSIZE (MAX_X*MAX_Y*2)
 /* Assure D-Cache has 32byte,must be 32byte alignment */
 uint8_t vbuffer[VBUFSIZE] __attribute__ ((section(".extram"))) __attribute__ ((aligned (32)));
#endif
#endif

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
/* Load a 2-byte little-endian word */
static inline uint16_t LD_WORD (const uint8_t* ptr)
{
	uint16_t rv;

	rv = ptr[1];
	rv = rv << 8 | ptr[0];
	return rv;
}
/* Load a 4-byte little-endian word */
static inline uint32_t LD_DWORD(const uint8_t* ptr)
{
	uint32_t rv;

	rv = ptr[3];
	rv = rv << 8 | ptr[2];
	rv = rv << 8 | ptr[1];
	rv = rv << 8 | ptr[0];
	return rv;
}
/* Make RGB888 to RGB565 Colour format for PNG,JPEG Files */
static inline uint16_t MAKE_RGB888_TO_RGB565(uint8_t** pt)
{
	uint16_t d;
	#if !defined(USE_SSD1332_SPI_OLED)
		d =  ((*(*pt)++) >> 3) << 11;	/* R */
		d |= ((*(*pt)++) >> 2) << 5;	/* G */
		d |=  (*(*pt)++) >> 3;			/* B */
	#else                            
		d =   (*(*pt++)) >> 3;			/* R */
		d |= ((*(*pt++)) >> 2) << 5;	/* G */
		d |= ((*(*pt++)) >> 3) << 11;	/* B */
	#endif
	return d;
}
/* Make BGR888 to RGB565 Colour format for BMP Files */
static inline uint16_t MAKE_BGR888_TO_RGB565(uint8_t** pt)
{
	uint16_t d;
	#if !defined(USE_SSD1332_SPI_OLED)
		d =   (*(*pt)++) >> 3;			/* B */
		d |= ((*(*pt)++) >> 2) << 5;	/* G */
		d |= ((*(*pt)++) >> 3) << 11;	/* R */
	#else                            
		d =  ((*(*pt)++) >> 3) << 11;	/* B */
		d |= ((*(*pt)++) >> 2) << 5;	/* G */
		d |=  (*(*pt)++) >> 3; 			/* R */
	#endif
	return d;
}

#ifdef USE_IJG_LIB /* JPEG */
 extern void jpeg_fatfs_src (j_decompress_ptr cinfo, FIL * infile);
#endif


/* Functions -----------------------------------------------------------------*/
#if FF_USE_LFN
/**************************************************************************/
/*!
	ts_fileloads helper functio
	Get Long file name string.
*/
/**************************************************************************/
static const char* GetLFN(char* path, char* filename, DIR* dir, FILINFO* fno)
{
	int i =0;

	if ( (f_opendir(dir, path) == FR_OK) ) {
		while (f_readdir(dir, fno) == FR_OK && i < 256) {
			if(strcmp(filename, fno->altname)==0){
				if(*(fno->fname)) return (const char*)fno->fname;
				else 			  return (const char*)fno->altname;
			}
			i++;
		}
		return (const char*)filename;
	}
	else{
		return (const char*)filename;
	}

}
#endif

/**************************************************************************/
/*!
	ts_fileloads helper function.
	Extract file extention.
*/
/**************************************************************************/
static int strstr_ext (
	const char *src,
	const char *dst
)
{
	int si, di;
	char s, d;

	si = strlen(src);
	di = strlen(dst);
	if (si < di) return 0;
	si -= di; di = 0;
	do {
		s = src[si++];
		if (s >= 'a' && s <= 'z') s -= 0x20;
		d = dst[di++];
		if (d >= 'a' && d <= 'z') d -= 0x20;
	} while (s && s == d);
	return (s == d);
}


/**************************************************************************/
/*!
	ts_fileloads helper function.
	PushSW/TouchPanel and UART-Input wait subroutine.
*/
/**************************************************************************/
static inline void wait_anyinput(void)
{
	char c;

	do{
		c = xgetc_n();
		switch (c) {
		case BTN_RIGHT:
		case BTN_LEFT:
		case BTN_DOWN:
		case BTN_UP:
		case BTN_OK:
		case BTN_CAN:
		case BTN_ESC:
			break;
		default:
			c = 0;
		}
	}while(!c);
}










/**************************************************************************/
/*!
	TXT/BMP/JPEG/PNG/GIF/IMG LODER SECTIONS
*/
/**************************************************************************/



/**************************************************************************/
/*!
	S-JIS/Ascii TEXT file loader sections
*/
/**************************************************************************/
/**************************************************************************/
/*!
	TEXT file loader subfunction.
    Put a character into text buffer.
*/
/**************************************************************************/
static void txt_putc(uint8_t chr)
{

#if !(USE_FILER_FONTX) || !(defined(USE_KANJIFONT))
	if ((uint8_t)chr & 0x80) {
		chr = 1;
	}
#endif

	if (chr == '\n') {
		if (++Row >= TS_HEIGHT) {
			Row = TS_HEIGHT - 1;
		}
		Col = 0;
		return;
	}

	ts_write(Row, Col, ((uint16_t)Attr << 8) | chr);

	if (++Col >= TS_WIDTH) {
		Col = TS_WIDTH - 1;
	}

}
/**************************************************************************/
/*!
    TEXT file loader
	User defined call-back function to SJIS style text files.
*/
/**************************************************************************/
static void load_txt (
	FIL *fp,		/* Pointer to the open file object to load */
	void *work,
	unsigned int sz_work
)
{
	TXVIEW *tv = work;
	unsigned int lw, i, j, line, br, col, lines, max_lines;
	uint32_t cfp;
	uint8_t c;
	char k;

	/* Get MAXIMUM TXTFile line number */
	max_lines = (sz_work - (unsigned int)tv->ltbl + (unsigned int)&tv) / 4;

	/* Init file pointers */
	cfp = br = i = 0;
	tv->ltbl[0] = 0;

	/* Get Maximum charactor count per lines */
	for (line = 0; line < max_lines - 1; ) {
		if (i >= br) {
			cfp = f_tell(fp);
			f_read(fp, tv->sbuf, 512, &br);
			if (br == 0) break;
			i = 0;
		}
		if (tv->sbuf[i++] == '\n') tv->ltbl[++line] = cfp + i;
	}

	/* Init line pointers */
	lines = line;
	line = col = 0;

	/* Display TXT file ready */
	Display_clear_if();
	Attr = '\x87';

	/* Display text on screen */
	for (;;) {
		/* Adjust character location */
		ts_locate(0, 0 ,0);
		for (i = line; i < line + TS_HEIGHT && i < lines; i++) {
			f_lseek(fp, tv->ltbl[i]);
			lw = tv->ltbl[i + 1] - tv->ltbl[i];
			f_read(fp, tv->sbuf, lw > 512 ? 512 : lw, &br);
			for (j = 0; j < col && tv->sbuf[j] != '\n'; j++) {
				if (((tv->sbuf[j] >= 0x81 && tv->sbuf[j] <= 0x9F) || (tv->sbuf[j] >= 0xE0 && tv->sbuf[j] <= 0xFC))) j++;
			}
			if (tv->sbuf[j] != '\n') {
				if (j > col) txt_putc(' ');
				for ( ; tv->sbuf[j] != '\n' && j < TS_WIDTH + col; j++) {
					c = tv->sbuf[j];
					if (c == '\r' || c == '\t') c = ' ';
					txt_putc(c);
				}
			}
			for ( ; j < TS_WIDTH + col; j++) txt_putc(' ');
			txt_putc('\n');
		}

		for ( ; i < line + TS_HEIGHT; i++) {
			for (j = 0; j < TS_WIDTH; j++) txt_putc(' ');
		}

		k = xgetc();

		for (;;) {
			c = xgetc_n();
			if (!c) break;
			k = c;
		}

		switch (k) {
		case BTN_ESC:
		case BTN_CAN:
			return;
		case BTN_RIGHT:
			if (col < 512 - col) col++;
			break;
		case BTN_LEFT:
			if (col) col--;
			break;
		case BTN_DOWN:
			if (line + TS_HEIGHT < lines) line++;
			break;
		case BTN_UP:
			if (line) line--;
			break;
		default:
			k = 0;
		}
	}
}





/**************************************************************************/
/*!
	24bit BITMAP file loader section
*/
/**************************************************************************/

/**************************************************************************/
/*!
	BitMapFile Loader.
    Display Windows-24bit style BMP.
*/
/**************************************************************************/
static int load_bmp(FIL *fil)
{
	uint32_t n, m, biofs, bw, iw, bh, w, l, t;
	unsigned int bx;
	uint32_t xs, xe, ys, ye, i;
	uint8_t *p;
	char k, c;


	/* Load bitstream address offset  */
	biofs = LD_DWORD(Buff+10);
	/* Check plane count "1" */
	if (LD_WORD(Buff+26) != 1) return 0;
	/* Check BMP bit_counts "24(windows bitmap standard)" */
	if (LD_WORD(Buff+28) != 24) return 0;
	/* Check BMP compression "BI_RGB(no compresstion)"*/
	if (LD_DWORD(Buff+30) != 0) return 0;
	/* Load BMP width */
	bw = LD_DWORD(Buff+18);
	/* Load BMP height */
	bh = LD_DWORD(Buff+22);
	/* Check BMP width under 1280px */
	if (!bw || bw > 1280 || !bh) return 0;

	/* Calculate cata byte count per holizontal line */
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

	/* Clear display */
	Display_clear_if();


	l = t = 0;
	do {
	    /* Limit to MAX_Y */
		m = (bh <= MAX_Y) ? biofs : biofs + (bh - MAX_Y - t) * iw;
		if (f_lseek(fil, m) || fil->fptr != m) return 0;
		i = m % 512;
		f_read(fil, &Buff[i], BUFSIZE - i, &bx);
		m = ye; w = (bw > MAX_X) ? MAX_X : bw;
		do {
			Display_rect_if(xs, xe, m, m);
			if (i + iw > BUFSIZE) {
				n = BUFSIZE - i;
				memcpy(&Buff[i % 512], &Buff[i], n);
				i %= 512;
				f_read(fil, &Buff[i + n], BUFSIZE - i - n, &bx);
			}
			n = 0; p = &Buff[i + l * 3];
			do {
				n++;
				Display_wr_dat_if(MAKE_BGR888_TO_RGB565(&p));	/* BMP is BGR layout */
			} while (n < w);
			i += iw;
		} while (m-- > ys);

		k = xgetc();

		for (;;) {
			c = xgetc_n();
			if (!c) break;
			k = c;
		}

		switch (k) {
		case BTN_RIGHT:
			if (bw <= MAX_X) break;
			l = (l + MAX_X / 4 + MAX_X <= bw) ? l + MAX_X / 4 : bw - MAX_X;
			break;
		case BTN_LEFT:
			if (bw <= MAX_X) break;
			l = (l >= MAX_X / 4) ? l - MAX_X / 4 : 0;
			break;
		case BTN_DOWN:
			if (bh <= MAX_Y) break;
			t = (t + MAX_Y + MAX_Y / 4 <= bh) ? t + MAX_Y / 4 : bh - MAX_Y;
			break;
		case BTN_UP:
			if (bh <= MAX_Y) break;
			t = (t >= MAX_Y / 4) ? t - MAX_Y / 4 : 0;
			break;
		default:
			k = 0;
		}
	} while (k);

	return 1;
}





/**************************************************************************/
/*!
	IMG streaming movie loader section
*/
/**************************************************************************/
/**************************************************************************/
/*!
	IMG Loader.
    Play streaming ChaN's Original format video file.
*/
/**************************************************************************/
static int load_img(FIL* fil, const char *filename)
{
	uint8_t f;						/* Cursor state */
	uint8_t run;					/* Streaming stat 1:play 0:pause */
	char k;							/* Input control val */
	unsigned int br;				/* FatFs read bytes return */
	unsigned int d;					/* FatFs read bytes */
	unsigned int x,y;				/* Frame XY size */
	unsigned int szfrm;				/* Size of frames (RGB565 format ,in bytes) */
	unsigned int nfrm;				/* Number of frames */
	unsigned int cfrm;				/* Current frames */
	long fd, tp;					/* Frame period, timer period */
	unsigned int x1,x2,y1,y2;		/* Frame rectangle val */

#if !defined(USE_TFT_FRAMEBUFFER)
	unsigned int n;					/* FatFs read bytes */
#elif defined(USE_TFT_FRAMEBUFFER)
	uint8_t* tbuf;					/* Video framebuffer */
	unsigned int vsize;				/* FatFs read bytes */
 #if defined(ENABLE_PERFORMANCE_MEASUREMENT)
	unsigned int fcount;
 #endif
#endif

	/* Check color depth (RGB565 format) */
	if (LD_WORD(Buff+6) != 16) return 0;

	/* Store frame XY size */
	x = LD_WORD(Buff+2);
	y = LD_WORD(Buff+4);
	if (!x || x > MAX_X || !y || y > MAX_Y) return 0;

	/* Seek data start position */
	d = LD_DWORD(Buff+8);
	if (f_lseek(fil, d) || d != fil->fptr) return 0;

	/* Store coursor flag */
	f = CsrFlag;
	CsrFlag = 0;

	/* Clear display to ready to streaming */
	Display_clear_if();

	/* Display file information(long file name if can) */
	ts_locate(0, 0 ,0);
	xprintf("\33\x87\f");
	xprintf("Playing Original stream img File\n");
	xprintf("->%s\n", filename);

	/* Set frame rectangle */
	x1 = (MAX_X - x) / 2;
	x2 = (MAX_X - x) / 2 + x - 1;
	y1 = (MAX_Y - y) / 2;
	y2 = (MAX_Y - y) / 2 + y - 1;
	Display_rect_if(x1, x2, y1, y2);
	
	/* Set size of frames (RGB565 format,in bytes) */
	szfrm = x * y * 2;
	
#if defined(USE_TFT_FRAMEBUFFER) && defined(ENABLE_PERFORMANCE_MEASUREMENT)
	Timer = 0;
	fcount = 0;
#endif

#if defined(USE_TFT_FRAMEBUFFER)
	if(szfrm > BUFSIZE) {
		tbuf = vbuffer;
		vsize = VBUFSIZE;
	}else{
		tbuf = Buff;
		vsize = BUFSIZE;
	}
#endif

	/* Store frame period (in us) */
	fd = (long)LD_DWORD(Buff+16);
	
	/* Store number of frames and init run mode */
	nfrm = LD_DWORD(Buff+12);
	cfrm = 0;
	run = 1;
	
	/* Store timer period */
	tp = TmrFrm + fd;
	
	/* Go streaming */
	for (;;) {
		d = szfrm;
		do {
		#if defined(USE_TFT_FRAMEBUFFER)
			if (f_read(fil, tbuf, d, &br) || d != br) goto li_exit;
			d -= br;
		#if defined(USE_TFT_VSYNC) /* Wait VSYNC if needed */
			Display_WaitVSYNC();
		#endif
			Display_wr_block_if(tbuf, d);
		#else
			n = (d > BUFSIZE) ? BUFSIZE : d;
			if (f_read(fil, Buff, n, &br) || n != br) goto li_exit;
			d -= br;
			Display_wr_block_if(Buff, n);
		#endif
		} while (d);
#if defined(USE_SSD1963_TFT) || (USE_HX8369A_TFT) || (USE_HX8363B_TFT)
		Display_wr_cmd_if(0x002C);	/* SSD1963/HX8369A/HX8363B consideration */
#endif
		tp += fd;
		cfrm++;

#if defined(USE_TFT_FRAMEBUFFER) && defined(ENABLE_PERFORMANCE_MEASUREMENT)
		fcount++;
		if(Timer >= 1000){
			ts_locate(2, 0 ,0);
			xprintf("%2d.%03dfps  ", fcount*1000/Timer,fcount*1000%Timer);
			fcount = 0;
			Timer = 0;
			Display_rect_if(x1, x2, y1, y2);
		}
#elif defined(USE_TFT_FRAMEBUFFER) && !defined(ENABLE_PERFORMANCE_MEASUREMENT)
		ts_locate(2, 0 ,0);
		xprintf("%d/%d frames    ", cfrm, nfrm);
		Display_rect_if(x1, x2, y1, y2);
#endif
		/* Key control and frame timer wait */
		for (;;) {
			k = xgetc_n();
			
			/* Exit player */
			if ((k == BTN_CAN) || (k == BTN_ESC)) goto li_exit;
			
			/* Pause & Resume */
			if (k == BTN_OK) {
				run ^= 1;
				tp = TmrFrm + fd; /* Re-set timer period */
			}
			
			/* Advnace & Return frames */
			if (cfrm < nfrm) {
				if (run && TmrFrm >= tp) break; /* Go to next frame on Playing */
				if (!run && k == BTN_UP) break; /* Go to next frame on Pause */
			}
			if (!run && k == BTN_DOWN && cfrm >= 2) { /* Go to previous two frames on Pause */
				if (f_lseek(fil, fil->fptr - szfrm * 2)) goto li_exit;
				cfrm -= 2;
				break;
			}
		}
	}

/* Exit */
li_exit:
	CsrFlag = f;
	Display_clear_if();
	return 1;
}



/**************************************************************************/
/*!
	JPEG file loader section
*/
/**************************************************************************/
#if defined(USE_TINYJPEG_LIB)
/**************************************************************************/
/*!
    JPEG file loader Lower-Side Using ChaN's JPEG file Decorder
    Copy image data to the display.
*/
/**************************************************************************/
static void disp_blt (
	int left,				/* Left end (-32768 to 32767) */
	int right,				/* Right end (-32768 to 32767, >=left) */
	int top,				/* Top end (-32768 to 32767) */
	int bottom,				/* Bottom end (-32768 to 32767, >=right) */
	const uint16_t *pat		/* Pattern data */
)
{
	int yc, xc, xs;
#if !defined(USE_TFT_FRAMEBUFFER)
	int xl;
	uint16_t pd;
#endif

	if (left > right || top > bottom) return; 	/* Check varidity */
	if (left > MaskR || right < MaskL  || top > MaskB || bottom < MaskT) return;	/* Check if in active area */

	yc = bottom - top + 1;			/* Vertical size */
	xc = right - left + 1; xs = 0;	/* Horizontal size and skip */

	if (top < MaskT) {		/* Clip top of source image if it is out of active area */
		pat += xc * (MaskT - top);
		yc -= MaskT - top;
		top = MaskT;
	}
	if (bottom > MaskB) {	/* Clip bottom of source image if it is out of active area */
		yc -= bottom - MaskB;
		bottom = MaskB;
	}
	if (left < MaskL) {		/* Clip left of source image if it is out of active area */
		pat += MaskL - left;
		xc -= MaskL - left;
		xs += MaskL - left;
		left = MaskL;
	}
	if (right > MaskR) {	/* Clip right of source image it is out of active area */
		xc -= right - MaskR;
		xs += right - MaskR;
		right = MaskR;
	}
	Display_rect_if(left, right, top, bottom);	/* Set rectangular area to fill */
#if defined(__DCACHE_PRESENT) /* Flush cache datas */
	SCB_CleanDCache_by_Addr((uint32_t*)pat, xc*yc*2);
#endif
#if defined(USE_TFT_FRAMEBUFFER)
	Display_wr_block_if((uint8_t*)pat, xc*yc);
#else
	do { /* Send image data */
		xl = xc;
		do {
			pd = *pat++;
			Display_wr_dat_if(pd);
		} while (--xl);
		pat += xs;
	} while (--yc);
#endif
}
/**************************************************************************/
/*!
    JPEG file loader Lower-Side Using Chan's JPEG file Decorder
	User defined call-back function to input JPEG data
*/
/**************************************************************************/
static unsigned int tjd_input (
	JDEC* jd,			/* Decoder object */
	BYTE* buff,			/* Pointer to the read buffer (NULL:skip) */
	unsigned int nd		/* Number of bytes to read/skip from input stream */
)
{
	unsigned int rb;
	FIL *fil = (FIL*)jd->device;	/* Input stream of this session */


	if (buff) {	/* Read nd bytes from the input strem */
		f_read(fil, buff, nd, &rb);
		return rb;	/* Returns number of bytes could be read */

	} else {	/* Skip nd bytes on the input stream */
		return (f_lseek(fil, f_tell(fil) + nd) == FR_OK) ? nd : 0;
	}
}
/**************************************************************************/
/*!
    JPEG file loader Lower-Side Using Chan's JPEG file Decorder
	User defined call-back function to output RGB bitmap.
*/
/**************************************************************************/
static unsigned int tjd_output (
	JDEC* jd,		/* Decoder object */
	void* bitmap,	/* Bitmap data to be output */
	JRECT* rect		/* Rectangular region to output */
)
{
	jd = jd;	/* Suppress warning (device identifier is not needed) */

	/* Check user interrupt at left end */
	if (!rect->left && _kbhit()) return 0;	/* Abort decompression */

	/* Put the rectangular into the display */
	disp_blt(rect->left, rect->right, rect->top, rect->bottom, (uint16_t*)bitmap);

	return 1;	/* Continue decompression */
}

/**************************************************************************/
/*!
    JPEG file loader Upper-Side Using Chan's JPEG file Decorder
    Display JPEG Data.
*/
/**************************************************************************/
static void load_jpg (
	FIL *fp,					/* Pointer to the open file object to load */
	void *work,					/* Pointer to the working buffer (must be 4-byte aligned) */
	unsigned int sz_work		/* Size of the working buffer (must be power of 2) */
)
{
	JDEC jd;					/* Decoder object (124 bytes) */
	JRESULT rc;
	BYTE scale;

	/* Clear screen */
	Display_clear_if();

	/* Prepare to decompress the file */
	rc = jd_prepare(&jd, (void*)tjd_input, work, sz_work, fp);
	if (rc == JDR_OK) {

		/* Determine scale factor */
		for (scale = 0; scale < 3; scale++) {
			if ((jd.width >> scale) <= MAX_X && (jd.height >> scale) <= MAX_Y) break;
		}

		/* Display size information at bottom of screen */
		ts_locate(TS_HEIGHT - 1, 0 ,0);
		xprintf("\33\x87%ux%u 1/%u", jd.width, jd.height, 1 << scale);

		/* Start to decompress the JPEG file */
		rc = jd_decomp(&jd, (void*)tjd_output, scale);	/* Start to decompress */

	} else {

		/* Display error code */
		ts_locate(0, 0, 0);
		xprintf("\33\x87 Error: %d", rc);
	}

	/* Exit */
	wait_anyinput();
}

#elif defined(USE_IJG_LIB)
/**************************************************************************/
/*!
	JPEGFileLoader using IJG JPEG library.
    Custom error handling for IJG JPEG library.
*/
/**************************************************************************/
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

/**************************************************************************/
/*!
    JPEG file loader Upper-Side Using IJG JPEG library.
    Display JPEG file.
*/
/**************************************************************************/
static int load_jpeg(FIL *fil,int mode)
{
	/* JPEG relation */
	int denom,scale,row_stride;
	JSAMPARRAY	buffer;
	struct my_error_mgr jerr;
	struct jpeg_decompress_struct dcinfo;

	/* Scribe relation */
	uint8_t	 *p;
	uint16_t  x,y,dx,dy,i;
	uint32_t  w,h;

	/* Clear Screen */
	Display_clear_if();

	/* We set up the normal JPEG error routines, then override error_exit. */
	dcinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;

	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
	/* If we get here, the JPEG code has signaled an error.
	 * We need to clean up the JPEG object, close the input file, and return.
	 */
		ts_locate(0, 0 ,0);
		xprintf("\33\x87\fOOPS! Overdoze Memory Region!\n");
		xprintf("press any key\n");
		goto jpeg_end_decode;
	}

#ifdef ENABLE_PERFORMANCE_MEASUREMENT
	display_performance_ready_if();
#endif

	/* Allocate and initialize JPEG decompression object */
	jpeg_create_decompress(&dcinfo);
	jpeg_fatfs_src(&dcinfo, fil);
	jpeg_read_header(&dcinfo, TRUE);

	/* Calculate scalling */
	denom = 8;
	dcinfo.scale_denom = 8;
	dcinfo.scale_num = 8;

	w = dcinfo.image_width;
	h = dcinfo.image_height;

	if((w % denom) != 0) {
		w += (denom - (dcinfo.image_width % denom));
	}

	if((h % denom) != 0) {
		h += (denom - (dcinfo.image_height % denom));
	}

    dcinfo.scale_denom = dcinfo.scale_num = 8;
    denom = 8;

	/* Set byte Bboundery */
	for (scale = 8; scale > 1; scale--) {
		if (w / denom * scale <= MAX_X)
		break;
	}
	 while (h / denom * scale > MAX_Y) {
		scale--;
	}

	/* Check valid scalling */
	if(scale <= 0){
		ts_locate(0, 0 ,0);
		xprintf("\33\x87\ferror\nscaling region over!\n");
		xprintf("press any key\n");
		goto jpeg_end_decode;
	}

	/* Check upper limit (1280 pixel width) */
	if((dcinfo.image_width > dcinfo.image_height) && (dcinfo.image_width > 1280)){
		ts_locate(0, 0 ,0);
		xprintf("\33\x87\ferror\nimage_width limit over!\n");
		xprintf("press any key\n");
		goto jpeg_end_decode;
	}

	/* Decoding */
	dcinfo.two_pass_quantize 	= FALSE;
	dcinfo.dither_mode 			= JDITHER_ORDERED;
	dcinfo.scale_num 			= scale;
	dcinfo.out_color_space 		= JCS_RGB;
	dcinfo.dct_method 			= JDCT_DECODE_METHOD; /* See ts_fileloads.h */

	jpeg_calc_output_dimensions(&dcinfo);
	jpeg_start_decompress(&dcinfo);

	/* Centering */
	dx = dcinfo.output_width;
	dy = dcinfo.output_height;
	x = (MAX_X - dx) / 2;
	y = (MAX_Y - dy) / 2;
	Display_rect_if(x,x + dx - 1,y,y + dy - 1);

	/* JSAMPLEs per row in output buffer */
	row_stride = dcinfo.output_width * dcinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*dcinfo.mem->alloc_sarray)
				((j_common_ptr) &dcinfo, JPOOL_IMAGE, row_stride, 1);

	/* Display decoded line datas */
	while (dcinfo.output_scanline < dcinfo.output_height) {
		jpeg_read_scanlines(&dcinfo, buffer, 1);
	#if defined(USE_TFT_FRAMEBUFFER) || (USE_NT35516_TFT)
		Display_rect_if(x,x + dx - 1,y + dcinfo.output_scanline,y + dcinfo.output_scanline);
	#endif
		for(i = 0,p = buffer[0];i < dcinfo.output_width;i++) {
			Display_wr_dat_if(MAKE_RGB888_TO_RGB565(&p));
		}
	}
	
	/* Finish decompress */
	jpeg_finish_decompress(&dcinfo);

jpeg_end_decode:

	/* Free all of the memory associated with the jpeg */
    jpeg_destroy_decompress(&dcinfo);

#ifdef ENABLE_PERFORMANCE_MEASUREMENT
	uint32_t end = display_performance_result_if();
	ts_locate(TS_FILER_HEIGHT-1, 0,0);
	xprintf("\33\x87 Decode in %duSec\n",end);
#endif

	/* Exit */
	wait_anyinput();

	return 1;
}
#endif





/**************************************************************************/
/*!
	PNG file loader section
*/
/**************************************************************************/
#ifdef USE_LIBPNG
/**************************************************************************/
/*!
    PNG File Loader Lower-Side using libpng.
    Retarget FatFs's Filesystem to libpng.
*/
/**************************************************************************/
static void fatfs_read_data(png_structp read_ptr, png_bytep data, png_size_t length)
{
	UINT rb = 0;
	FIL* fd;
	fd = (FIL*)png_get_io_ptr(read_ptr);
    if((f_read(fd, data, length , &rb)!= FR_OK)||(rb != length)){
		ts_locate(0, 0, 0);
		xprintf("\33\x87 FatFs Read Error!\n");
		if(rb < length) xprintf("Not Fully loaded!\n");
	}
}

/**************************************************************************/
/*!
    PNG file loader Upper-Side using libpng.
    Decode and display PNG data.
*/
/**************************************************************************/
static int load_png(FIL *fil, const char *title)  /* File is already open */
{
	/* Scribe relation */
	uint16_t lx,ly;
	uint32_t i,k=0;
	UINT	 nb;

	/* libpng structure pointers */
	png_structp read_ptr;
	png_infop read_info_ptr, end_read_info_ptr;

	/* PNG informations */
	uint8_t pngSignature[8];
	png_uint_32 width, height,nx,ny;
	int bit_depth, color_type, interlace_type;

	/* Draw to TFT-LCD relations */
	uint32_t row_stride;
	uint8_t* p;
	uint8_t* row_buffer=NULL;

	/* Confirn PNG file */
	f_read(fil, pngSignature, 8, &nb);
	if(!png_check_sig(pngSignature, 8)) return 1;

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also supply the
	 * the compiler header file version, so that we know if the application
	 * was compiled with a compatible version of the library.  REQUIRED
	 */
	read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (read_ptr == NULL)
	{
		return 1;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	read_info_ptr = png_create_info_struct(read_ptr);
	if (read_info_ptr == NULL)
	{
		png_destroy_read_struct(&read_ptr, NULL, NULL);
		return 1;
	}
	end_read_info_ptr = png_create_info_struct(read_ptr);
	if (!end_read_info_ptr) {
		png_destroy_read_struct(&read_ptr, &read_info_ptr, (png_infopp)NULL);
		return 1;
	}

   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */
   if (setjmp(png_jmpbuf(read_ptr)))
   {
		if( row_buffer != NULL ) {
			/* Diacard allocated row stride */
			png_free(read_ptr,row_buffer);
		}

        /* If we get here, we had a problem reading the file */
	  	/* Display error code */
		ts_locate(0, 0, 0);
		xprintf("\33\x87\f libpng Handling Error!\n");
		xprintf("Press any Key\n");
		goto png_exit;
	}

    /* If you are using replacement read functions, instead of calling
     * png_init_io() here you would call:
     */
	png_set_read_fn(read_ptr, (png_voidp)fil, fatfs_read_data);

	/* If we have already read some of the signature */
	png_set_sig_bytes(read_ptr, 8);

	/* The call to png_read_info() gives us all of the information from the
	 * PNG file before the first IDAT (image data chunk).  REQUIRED
	 */
	png_read_info(read_ptr, read_info_ptr);
	png_get_IHDR(read_ptr, read_info_ptr, &width, &height, &bit_depth, &color_type,
	   &interlace_type, NULL, NULL);

#if 1
	/* Set up the data transformations you want.  Note that these are all
	 * optional.  Only call them if you want/need them.  Many of the
	 * transformations only work on specific types of images, and many
	 * are mutually exclusive.
	 */

	/* Tell libpng to strip 16 bit/color files down to 8 bits/color */
	/*png_set_strip_16(read_ptr);*/

	/* Strip alpha bytes from the input data without combining with the
	* background (not recommended).
	*/
	/*png_set_strip_alpha(read_ptr);*/

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	* byte into separate bytes (useful for paletted and grayscale images).
	*/
	png_set_packing(read_ptr);

	/* Change the order of packed pixels to least significant bit first
	* (not useful if you are using png_set_packing). */
	png_set_packswap(read_ptr);

	/* Expand paletted colors into true RGB triplets */
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(read_ptr);

	/* Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel */
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(read_ptr);

	/* Expand paletted or RGB images with transparency to full alpha channels
	* so the data will be available as RGBA quartets.
	*/
	if (png_get_valid(read_ptr, read_info_ptr, PNG_INFO_tRNS)) {
		/* Convert tRNS Chunk into Alpha Channel */
		png_set_tRNS_to_alpha(read_ptr);
	} else if (!(color_type & PNG_COLOR_MASK_ALPHA)) {
		/* Add Alpha Channel if doesn't have tRNS nor Alpha Channel */
		png_set_add_alpha(read_ptr, 0xff, PNG_FILLER_AFTER);
	}

#if 0 /* Alpha channnel omake */
	/* Set the background color to draw transparent and alpha images over.
	 * It is possible to set the red, green, and blue components directly
	 * for paletted images instead of supplying a palette index.  Note that
	 * even if the PNG file supplies a background, you are not required to
	 * use it - you should use the (solid) application background if it has one.
	 */
	png_color_16 my_background, *image_background;

	if (png_get_bKGD(read_ptr, read_info_ptr, &image_background))
	  png_set_background(read_ptr, image_background,
						 PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);
	else
	  png_set_background(read_ptr, &my_background,
						 PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
#endif

#endif
	png_read_update_info(read_ptr, read_info_ptr);

	Display_clear_if();
	ts_locate(0, 0 ,0);
	xprintf("\33\x87\f\n");
	xprintf("%s\n", title);
	xprintf("width= %d\n",width);
	xprintf("height= %d\n",height);
	xprintf("col_depth= %d\n",bit_depth);
	xprintf("col_type= %d\n",color_type);

	/* Push any key to decode start */
	wait_anyinput();

#ifdef ENABLE_PERFORMANCE_MEASUREMENT
	display_performance_ready_if();
#endif

	/* Setting displaylimit */
	nx = width;
	ny = height;
	if((nx >1280)||(ny>1280)) {
		ts_locate(0, 0 ,0);
		xprintf("\33\x87\fOOPS Screen Size Over!\n");
		xprintf("press any key\n");
		goto png_exit;
	}
	if(nx >= MAX_X) nx = MAX_X;
	if(ny >= MAX_Y) ny = MAX_Y;

	/* Centering */
	lx = (MAX_X - nx) / 2;
	ly = (MAX_Y - ny) / 2;
	Display_rect_if(lx,lx + nx - 1,ly,ly + ny - 1);

	/* Allocate row stride buffer */
	row_stride = (png_get_rowbytes(read_ptr, read_info_ptr) + 3) & ~3; /* 4byte alignments */
	row_buffer = png_malloc(read_ptr,row_stride);

   /* Display PNG data */
	for(k = 0; k < ny ;k++) {
	#if defined(USE_TFT_FRAMEBUFFER) || (USE_NT35516_TFT)
		Display_rect_if(lx,lx + nx - 1,ly + k,ly + k);
	#endif
		png_read_row(read_ptr,row_buffer, NULL );

		for(i = 0,p = row_buffer;i < nx;i++) {
			Display_wr_dat_if(MAKE_RGB888_TO_RGB565(&p));
			p++;/* Alpha-Channel is discarded */
		}
	}

	/* Discard allocated row stride */
	png_free(read_ptr,row_buffer);

	/* PNG read end procedure */
	png_read_end(read_ptr, end_read_info_ptr);

png_exit:
	/* Free all of the memory associated with the read_ptr and read_info_ptr */
    png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_read_info_ptr);

#ifdef ENABLE_PERFORMANCE_MEASUREMENT
	uint32_t end = display_performance_result_if();
	ts_locate(TS_FILER_HEIGHT-1, 0,0);
	xprintf("\33\x87 Decode in %duSec\n",end);
#endif

	/* Exit*/
	wait_anyinput();

	return 1;
}
#endif






/**************************************************************************/
/*!
	GIF file loader section
*/
/**************************************************************************/
#ifdef USE_GIFLIB
/* Exchange into BGR565 Colour format */
static inline uint16_t MAKE_GIFBGR565(GifColorType* p)
{
	uint16_t d;
    #if !defined(USE_SSD1332_SPI_OLED)
		d  = (p->Red   >> 3) << 11;	/* R	 */
		d |= (p->Green >> 2) << 5;	/* G 	 */
		d |=  p->Blue  >> 3;		/* B 	 */
	#else
		d =   p->Red   >> 3;		/* B	 */
		d |= (p->Green >> 2) << 5;	/* G 	 */
		d |= (p->Blue  >> 3) << 11;	/* R	 */
	#endif
	return d;
}
/**************************************************************************/
/*!
    GIF file loader Upper-Side using giflib.
    Display GIF & Animation GIF data.
*/
/**************************************************************************/
static int load_gif(FIL *fil)
{
	/* giflib relations */
    GifRecordType 	RecordType;
    GifByteType*	Extension;
    GifRowType    	RowBuffer=NULL;
    GifFileType*	GifFile;
	GifColorType*	ColorMapEntry;
	ColorMapObject* ColorMap;

	/* Scribe relation */
	uint8_t  c;
	uint16_t d,DelayTime=0;
    volatile unsigned int i,j,n,lx,ly,Size,Left,Top,Width,Height;
	int ErrorCode,ExtCode,TranCol=0;

	/* Interlace relation */
    int InterlacedOffset[] = { 0, 4, 2, 1 }; 	/* The way Interlaced image should. */
	int InterlacedJumps[]  = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */

	/* Load GIF file pointer */
	if ((GifFile = DGifOpenFileHandle(fil,&ErrorCode)) == NULL) {
	    return 1;
	}

	/* Clear Screen */
	Display_clear_if();

	/* Set colour map or background colour */
	ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);

	/* Check over lange */
	if((GifFile->SWidth > MAX_X)||(GifFile->SHeight > MAX_Y))  {
		ts_locate(0, 0 ,0);
		xprintf("\33\x87\fOOPS Screen Size Over!\n");
		xprintf("press any key\n");
		goto gif_end;
	}

	/* Centering */
	lx = (MAX_X - (GifFile->SWidth))  / 2;
	ly = (MAX_Y - (GifFile->SHeight)) / 2;

    /* Allocate memory or one row which will be used as trash during reading
       image*/
    Size = GifFile->SWidth * sizeof(GifPixelType);/* Size in bytes one row.*/
    if ((RowBuffer = (GifRowType) malloc(Size)) == NULL) /* First row. */ {
		ts_locate(0, 0 ,0);
		xprintf("\33\x87\fFailed to allocate memory required, aborted.\n");
		xprintf("press any key\n");
		goto gif_end;
	}

	/* Set rowbuffer to backGround colour */
	for (i = 0; i < GifFile->SWidth; i++) RowBuffer[i] = GifFile->SBackGroundColor;

    /* Scan the content of the GIF file and load the image(s) in: */
    do {
		if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
			ts_locate(0, 0 ,0);
			xprintf("DGifGetRecordType Error!\n");
			xprintf("press any key\n");
			goto gif_end;
		}

	switch (RecordType) {
	    case IMAGE_DESC_RECORD_TYPE:
			if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
				ts_locate(0, 0 ,0);
				xprintf("\33\x87\fDGifGetImageDesc Error!\n");
				xprintf("press any key\n");
				goto gif_end;
			}

			Left 	= GifFile->Image.Left;
			Width 	= GifFile->Image.Width;
			Top 	= GifFile->Image.Top;
			Height 	= GifFile->Image.Height;
			if (GifFile->Image.Left + GifFile->Image.Width  > GifFile->SWidth   ||
			    GifFile->Image.Top  + GifFile->Image.Height > GifFile->SHeight) {
				ts_locate(0, 0 ,0);
				xprintf("\33\x87\fImage %d is not confined to screen dimension, aborted.\n", GifFile->ImageCount);
				xprintf("press any key\n");
				goto gif_end;
			}

			if(TranCol == NO_TRANSPARENT_COLOR) {

				if(GifFile->Image.Interlace) {
					/* Need to perform 4 passes on the images: */
					for (i = 0; i < 4; i++)
					for (j = Top + InterlacedOffset[i]; j < Top + Height; j += InterlacedJumps[i]) {
						if (DGifGetLine(GifFile, &RowBuffer[Left], Width) == GIF_ERROR) {
							ts_locate(0, 0 ,0);
							xprintf("\33\x87\fDGifGetLine Error!\n");
							xprintf("press any key\n");
							goto gif_end;
						}

						/* Set rectangle for virtual window */
						Display_rect_if(lx + Left,lx + Left + Width  - 1,
										ly + j  ,ly + j);

						for (n = 0; n < Width; n++) {
							/* Set global or local colour tables */
							if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
							else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
							ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];
							Display_wr_dat_if(MAKE_GIFBGR565(ColorMapEntry));
						}
					}
				}
				else {

					/* Set rectangle for virtual window */
					Display_rect_if(lx + Left,lx + Left + Width  - 1,
									ly + Top ,ly + Top  + Height - 1);

					for (i = 0; i < Height; i++) {

						if (DGifGetLine(GifFile, &RowBuffer[Left], Width) == GIF_ERROR) {
							ts_locate(0, 0 ,0);
							xprintf("\33\x87\fDGifGetLine Error!\n");
							xprintf("press any key\n");
							goto gif_end;
						}
					#if defined(USE_TFT_FRAMEBUFFER)
						Display_rect_if(lx + Left   ,lx + Left + Width  - 1,
										ly + Top +i ,ly + Top  + i);
					#endif
						for (n = 0; n < Width; n++) {
							/* Set global or local colour tables */
							if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
							else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
							ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];
							Display_wr_dat_if(MAKE_GIFBGR565(ColorMapEntry));
						}
					}
				}

			}
			else{
				GifColorType ct = ColorMap->Colors[TranCol];
				uint16_t tc = MAKE_GIFBGR565(&ct);

				if(GifFile->Image.Interlace) {
					/* Need to perform 4 passes on the images: */
					for (i = 0; i < 4; i++){
						for (j = Top + InterlacedOffset[i]; j < Top + Height; j += InterlacedJumps[i]) {
							if (DGifGetLine(GifFile, &RowBuffer[Left], Width) == GIF_ERROR) {
								ts_locate(0, 0 ,0);
								xprintf("\33\x87\fDGifGetLine Error!\n");
								xprintf("press any key\n");
								goto gif_end;
							}

							for (n = 0; n < Width; n++) {
								/* Set global or local colour tables */
								if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
								else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
								
								ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];
								Display_rect_if(lx + n,lx + n,ly + j,ly + j);
								d= MAKE_GIFBGR565(ColorMapEntry);
								if(tc != d) Display_wr_dat_if(d);
							}
						}
					}
				}
				else {
					for (i = 0; i < Height; i++) {
						if (DGifGetLine(GifFile, &RowBuffer[Left], Width) == GIF_ERROR) {
							ts_locate(0, 0 ,0);
							xprintf("\33\x87\fDGifGetLine Error!\n");
							xprintf("press any key\n");
							goto gif_end;
						}

						for (n = 0; n < Width; n++) {
							//* Set global or local colour tables */
							if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
							else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
							
							ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];
							Display_rect_if(lx + n,lx + n,ly + Top + i,ly +Top + i);
							d= MAKE_GIFBGR565(ColorMapEntry);
							if(tc != d) Display_wr_dat_if(d);
						}
					}
				}
			}

			/* Gif Animation delay with input interruption */
			for (n = 0; n < DelayTime ; n++) {
				_delay_ms(10);						/* Wait 0.01*n Seconds */
				c = xgetc_n();
				switch (c) {
				case BTN_CAN:
				case BTN_ESC:
				case BTN_OK:
				case BTN_RIGHT:
					goto gif_esc;
				default:
					;
				}
			}
		break;

	    case EXTENSION_RECORD_TYPE:
			/* Skip any extension blocks in file except comments: */
			if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
				ts_locate(0, 0 ,0);
				xprintf("\33\x87\fDGifGetExtension Error!\n");
				xprintf("press any key\n");
				goto gif_end;
			}
			while (Extension != NULL) {
			#if 0
				if(ExtCode == COMMENT_EXT_FUNC_CODE) {
					Extension[Extension[0]+1] = '\000';   /* Convert gif's pascal-like string */
					Comment = (char*) realloc(Comment, strlen(Comment) + Extension[0] + 1);
					strcat(Comment, (char*)Extension+1);
				}
			#endif
				if(ExtCode == GRAPHICS_EXT_FUNC_CODE ) {
					DelayTime= Extension[2] + 256 * Extension[3];
					if (Extension[1] & 0x01)	TranCol = (int)Extension[4];
					else						TranCol = NO_TRANSPARENT_COLOR;
				}

				if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
					ts_locate(0, 0 ,0);
					printf("\33\x87\fDGifGetExtensionNext Error!\n");
					xprintf("press any key\n");
					goto gif_end;
				}
			}

		break;
	    case TERMINATE_RECORD_TYPE:
		break;
	    default:		    /* Should be traps by DGifGetRecordType. */
		break;
	}
    }
    while (RecordType != TERMINATE_RECORD_TYPE);

gif_end:
	if( RowBuffer != NULL ) {
		free(RowBuffer);
		RowBuffer = NULL;
	}
	DGifCloseFile(GifFile,&ErrorCode);

	/* Exit */
	wait_anyinput();

	return 1;

	/* Case of interrupt escape from on loading AnimationGIF */
gif_esc:
	if( RowBuffer != NULL ) {
		free(RowBuffer);
		RowBuffer = NULL;
	}
	DGifCloseFile(GifFile,&ErrorCode);
	
	return 1;
}
#endif





/**************************************************************************/
/*!
	Fileloader function section
*/
/**************************************************************************/
/**************************************************************************/
/*!
    Execute IMG,JPEG,BMP,PNG,TXT and WAVE Files.
*/
/**************************************************************************/
static int load_file(char *path, char *filename, FIL *fil)
{
	unsigned int n;

	/* Store filename buffer */
	/* Because "char *filename" address is equal to Buff[]. */
#if FF_USE_LFN
	char fname[FF_MAX_LFN+1];
#else
	char fname[13];
#endif

	/* Check file read valid */
	if (f_open(fil, path, FA_READ)) return 0;

	/* Store filename */
	strcpy(fname, filename);

	/* Read file header */
	/*if (f_read(fil, Buff, 256, &n) || n != 256) return 0;*/
	if(f_read(fil, Buff, 256, &n)) return 0;

	/* Execute original video file */
	if (!memcmp(Buff, "IM", 2)) {
		load_img(fil, fname);
		f_close(fil);
		return RES_OK;
	}

	/* Execute windows 24BITMAP file */
	if (!memcmp(Buff, "BM", 2)) {
		load_bmp(fil);
		f_close(fil);
		return RES_OK;
	}

	/* Execute SJIS/Ascii style text file */
	if (strstr_ext(path, ".TXT")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_txt(fil, (BYTE*)Buff, BUFSIZE);
		f_close(fil);
		return RES_OK;
	}

#if defined(USE_IJG_LIB)
	/* Execute JPEG File using IJG JPEG library(libjpeg) */
	uint8_t JPEG_SOI[] = {0xFF,0xD8,0}; /* JPEG Merker */
	if (!memcmp(Buff,JPEG_SOI,2)) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_jpeg(fil,1);
		f_close(fil);
		return RES_OK;
	}
#elif defined(USE_TINYJPEG_LIB)
	/* Execute JPEG File using ChaN's JPEG decorder */
	if (strstr_ext(path, ".JPG")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
	#if (BUFSIZE > 32*1024)
	#warning "ChaN's JPEG library have 32kByte buffersize restriction"
		load_jpg(fil, (BYTE*)Buff, BUFSIZE);
	#else
		load_jpg(fil, (BYTE*)Buff, BUFSIZE);
	#endif
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_LIBPNG)
	/* Execute PNG File using libpng */
	if (strstr_ext(path, ".PNG")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_png(fil, fname);
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_GIFLIB)
	/* Execute gif File using giflib */
	if (strstr_ext(path, ".GIF")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_gif(fil);
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_WAVE_OUT)
	/* Execute RIFF-WAV file */
	if (strstr_ext(path, ".WAV")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_wav(fil, fname, (BYTE*)Buff, BUFSIZE);
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_MP3_HELIX)
	/* Execute MPEG2-Layer3 file */
	if (strstr_ext(path, ".MP3")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */

		/* DMA Buffer size check (STM32F407 work around ) */
	#if(BUFSIZE <= MP3_DMA_BUFFER_SIZE*2)
			/* If didn't have enough DMA-RAM,then... */
			/* Use heap memory as dma double buffer */
			uint8_t* twork = malloc(MP3_DMA_BUFFER_SIZE*2);
			if(twork == NULL)	return RES_ERROR;
			load_mp3(fil, fname, twork, MP3_DMA_BUFFER_SIZE*2);
			free(twork);
	#else
			load_mp3(fil, fname, (BYTE*)Buff, BUFSIZE);
	#endif
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_AAC_HELIX)
	/* Execute MPEG4 file */
	if (strstr_ext(path, ".AAC")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */

		/* DMA Buffer size check (STM32F407 work around ) */
	#if(BUFSIZE <= AAC_DMA_BUFFER_SIZE*2)
			/* If didn't have enough DMA-RAM,then... */
			/* Use heap memory as dma double buffer */
			uint8_t* twork = malloc(AAC_DMA_BUFFER_SIZE*2);
			if(twork == NULL)	return RES_ERROR;
			load_aac(fil, fname, twork, AAC_DMA_BUFFER_SIZE*2);
			free(twork);
	#else
			load_aac(fil, fname, (BYTE*)Buff, BUFSIZE);
	#endif
		f_close(fil);
		return RES_OK;
	}
#endif

	return RES_ERROR;
}


/**************************************************************************/
/*!
    Filer Functions.
*/
/**************************************************************************/
static void filer_draw_screen (
	const char *path,
	int items
)
{
	int i, j;
	uint16_t w;
	uint32_t n;
	DIRITEM *diritem = (DIRITEM*)(void*)Buff;

	for (j = 0; j < TS_FILER_HEIGHT; j++) {
		w = (j == 0 || j == TS_FILER_HEIGHT- 1) ? 0x1720 : 0x0720;
		for (i = 0; i < TS_WIDTH; i++)
			Vram[j][i] = w;
	}
	ts_rfsh(0, 0, TS_FILER_HEIGHT - 1, TS_WIDTH - 1);
	ts_locate(0,1,0);
	i = (strlen(path) > 24) ? strlen(path) - 24 : 0;
	xprintf("\33\x97%s/", &path[i]);

	for (i = 0, n = 0; i < items; i++)
		n += diritem[i].fsize;
	ts_locate(TS_FILER_HEIGHT - 1,0,0);

#if !defined(USE_SSD1332_SPI_OLED)
	xprintf("\33\x97 %u files, %u KB", items, n / 1024);
#else
	xprintf("\33\x97 %u files", items);
#endif
}


/**************************************************************************/
/*!
    Filer Functions.
*/
/**************************************************************************/
static void filer_put_item (
	const DIRITEM *item,
	int tblofs,
	unsigned int sel
)
{
	int n;
	char c;


	ts_locate(tblofs + 1,0,0);
	c = '\x87';
	if (item->fattr & AM_HID) c = '\x81';
	if (item->fattr & AM_RDO) c = '\x84';
	if (item->fattr & AM_DIR) c = '\x85';
	if (sel) c ^= 0x77;
	xputc('\33');
	xputc(c);

	ts_locate(tblofs + 1, 0, 0);
#if FF_USE_LFN
	/* Display long file name with round down */
	if(strlen(item->fname) >= (TS_WIDTH-3)) {
		int strl = strlen(item->fname);
		char* str = malloc(strl);
		strlcpy(str,item->fname,TS_WIDTH-3);
		/* Force round down */
		str[TS_WIDTH-4] = 0;
		/* In case of 2byte charactor */
		if(((str[TS_WIDTH-5] >= 0x81)&&(str[TS_WIDTH-5] <= 0x9F))||((str[TS_WIDTH-5] >= 0xE0)&&(str[TS_WIDTH-5] <= 0xEF))){
			str[TS_WIDTH-5] = 0;
		}
		xprintf(" %s", str);
		if(str != NULL) free(str);
	}
	else {
		xprintf(" %s", item->fname);
	}
	
	if(strlen(item->fname) < 13){
		for (n = strlen(item->fname); n < 12; n++) xputc(' ');
		if (item->fattr & AM_DIR) {
			xputs("   <DIR>   ");
		} else {
			xprintf("%10u", item->fsize);
		}
	}

#else
	/* Display short file name */
	xprintf(" %s", item->fname);
	for (n = strlen(item->fname); n < 12; n++) xputc(' ');
#if !defined(USE_SSD1332_SPI_OLED)
	if (item->fattr & AM_DIR) {
		xputs("   <DIR>   ");
	} else {
		xprintf("%10u", item->fsize);
	}
#endif
#endif
}

/**************************************************************************/
/*!
    Filer Functions.
*/
/**************************************************************************/
static int filer_load_dir (
	const char *path,
	DIR *dir,
	FILINFO *fno
)
{
	int i;
	DIRITEM *diritem;


	/* Try to read media upto 3 times */
    for(i=0;i<3;i++){
		if (!(f_opendir(dir, path))) break;
		if (i==2) return -1;
	}

	/* Read and store file names */
	/* exFAT is always long file name */
	i = 0;
	diritem = (DIRITEM*)(void*)Buff;
	while (f_readdir(dir, fno) == FR_OK && fno->fname[0] && i < MAX_DIR_ITEM) {
		diritem[i].fsize = fno->fsize;
		diritem[i].fattr = fno->fattrib;
#if FF_USE_LFN
		if(fno->altname[0]){
			if(strlen(fno->fname)>12) strcpy(diritem[i].fname, fno->altname);
			else					  strcpy(diritem[i].fname, fno->fname);
		} else {
			sprintf(diritem[i].fname,"%s",fno->fname); /* In case of exFAT */
		}
#else
		strcpy(diritem[i].fname, fno->fname);
#endif
		i++;
	}
	return i;
}


/**************************************************************************/
/*!
    Filer Main Functions.
*/
/**************************************************************************/
int filer(
	char *path,
	FIL *fil,
	DIR *dir,
	FILINFO *fno
)
{
	static int lv;
	int item = 0, ofs = 0, items, i;
	DIRITEM *diritem = (DIRITEM*)(void*)Buff;
	char k;
	char* filenames;


	for (;;) {
		items = filer_load_dir(path, dir, fno);
		if (items < 0) {
			if (lv > 0) return BTN_ESC;
			ts_locate(0,0,1);
			xputs("\33\x87\fDisk error or No file system.\nSet a microSD card and push (A) to continue...");
			do {
				ts_csrblink();
				k = xgetc_n();
				if (k == BTN_CAN) return BTN_CAN;
				if (k == BTN_ESC) return BTN_ESC;
			} while (k != BTN_OK);
			continue;
		}
#if defined(USE_TIME_DISPLAY)
		on_filer =1;
#endif
		filer_draw_screen(path, items);
		for (i = 0; i + ofs < items && i < TS_FILER_HEIGHT - 2; i++)
			filer_put_item(&diritem[i + ofs], i, ((item == i + ofs) ? 1 : 0));
		for (;;) {
			k = xgetc();
			if (k == BTN_ESC) return BTN_ESC;
			if (k == BTN_CAN)
			{
#if defined(USE_TIME_DISPLAY)
				on_filer =0;
#endif
				return BTN_CAN;
			}
			if (item >= items) continue;
			if (k == BTN_OK) {
				i = strlen(path);

#if FF_USE_LFN
				filenames = (char*)GetLFN(path,diritem[item].fname,dir,fno);
#else
				filenames = diritem[item].fname;
#endif

				path[i] = '/';
				strcpy(&path[i+1], diritem[item].fname);
				if (diritem[item].fattr & AM_DIR) {
					if(((*(diritem[item].fname))=='.')){
						if(((*(diritem[item].fname+1))=='.')){
						return BTN_CAN;
						}
					}
					else{
						lv++;
						filer(path, fil, dir, fno);
						lv--;
					}

				} else {

#if defined(USE_TIME_DISPLAY)
					on_filer =0;
					load_file(path, filenames, fil);
					on_filer =1;
					TimeDisplay =1;
#else
					load_file(path, filenames, fil);
#endif
				}
				path[i] = 0;
				break;
			}
			if (k == BTN_DOWN) {
				if (item + 1 >= items) continue;
				filer_put_item(&diritem[item], item - ofs, 0);
				item++;
				if (item - ofs >= TS_FILER_HEIGHT - 2) {
					ts_rlup(1, 0, TS_FILER_HEIGHT - 2, TS_WIDTH - 1);
					ofs++;
				}
				filer_put_item(&diritem[item], item - ofs, 1);
			}
			if (k == BTN_UP) {
				if (item == 0) continue;
				filer_put_item(&diritem[item], item - ofs, 0);
				item--;
				if (item < ofs) {
					ts_rldown(1, 0, TS_FILER_HEIGHT - 2, TS_WIDTH - 1);
					ofs--;
				}
				filer_put_item(&diritem[item], item - ofs, 1);
			}
		}
	}

}

/* End Of File ---------------------------------------------------------------*/
