/********************************************************************************/
/*!
	@file			ts_fileloads.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2013.09.20
	@brief          Filer and File Loaders.

    @section HISTORY
		2012.04.15	V1.01	Start here.
		2012.06.12  V2.00   Added WAVE Player Handling.
		2012.07.10  V3.00   Added GIF Decorder Handling.
							Fixed libjpeg & libpng's Error Handlings.
		2013.09.20  V4.00   Fixed unused functions.

    @section LICENSE
		BSD License. See Copyright.txt
		
		---IJG JPEGLIB Notice---
		"this software is based in part on the work ofthe Independent JPEG Group".
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ts_fileloads.h"

/* Defines -------------------------------------------------------------------*/
/* Used for Filer */
typedef struct {
	uint32_t fsize;
	uint8_t  fattr;
	char fname[13];
} DIRITEM;

/* Used for TEXT Viewer */
typedef struct {
	uint8_t  sbuf[512];
	uint32_t ltbl[1];
} TXVIEW;

/* Variables -----------------------------------------------------------------*/
/* Used for Cursor and Original Movies */
extern uint8_t  CsrFlag, TmrCsr;
extern uint32_t TmrFrm;
extern uint16_t Row, Col, Attr;

/* Used for Filer VRAM */
extern uint16_t Vram[TS_HEIGHT][TS_WIDTH];

/* Used for Getting Filenames */
#if _USE_LFN
 extern char Lfname[_MAX_LFN+1];
#else
 char Sfname[13];
#endif

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
#ifdef USE_IJG_LIB /* JPEG */
 extern void jpeg_fatfs_src (j_decompress_ptr cinfo, FIL * infile);
#endif

/* Functions -----------------------------------------------------------------*/
#if _USE_LFN
/**************************************************************************/
/*! 
	ts_fileloads Support Function
	Get LongFileName String.
*/
/**************************************************************************/
static const char* GetLFN(char* path, char* filename, DIR* dir, FILINFO* fno)
{
	int i =0;

	if ( (f_opendir(dir, path) == FR_OK) ) {
		while (f_readdir(dir, fno) == FR_OK && i < 256) {
			if(strcmp(filename, fno->fname)==0){
				if(*(fno->lfname)) return (const char*)fno->lfname;
				else 			   return (const char*)fno->fname;
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
	ts_fileloads Support Function
	Extract File extention.
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
	ts_fileloads Support Function
	Key/TouchPanel-Press and UART-Input Wait Subroutine.
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
	TEXT file loader Subfunction.
    Put a character into Text Buffer.
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
	User defined call-back function to Sjis Style Text Files.
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

	/* Get MAXIMUM TXTFile Line Number */
	max_lines = (sz_work - (unsigned int)tv->ltbl + (unsigned int)&tv) / 4;

	/* Init File Pointers */
	cfp = br = i = 0;
	tv->ltbl[0] = 0;
	
	/* Get Maximum Charactor-amount Per Lines */
	for (line = 0; line < max_lines - 1; ) {
		if (i >= br) {
			cfp = f_tell(fp);
			f_read(fp, tv->sbuf, 512, &br);
			if (br == 0) break;
			i = 0;
		}
		if (tv->sbuf[i++] == '\n') tv->ltbl[++line] = cfp + i;
	}

	/* Init Line Pointers */
	lines = line;
	line = col = 0;
	
	/* Display TXT File Ready */
	Display_clear_if();
	Attr = '\x87';

	/* Display Text On Screen */
	for (;;) {
		/* Adjust Charactor Rocation */
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
	BitMapFile Loader.
    Display Windows24bit Style BMP.
*/
/**************************************************************************/
static int load_bmp(FIL *fil)
{
	uint32_t n, m, biofs, bw, iw, bh, w, l, t;
	unsigned int bx;
	uint32_t xs, xe, ys, ye, i;
	uint8_t *p;
	uint16_t d;
	char k, c;


	/* Load BitStream Address Offset  */
	biofs = LD_DWORD(Buff+10);
	/* Check Plane Count "1" */
	if (LD_WORD(Buff+26) != 1) return 0;
	/* Check BMP bit_counts "24(windows bitmap standard)" */
	if (LD_WORD(Buff+28) != 24) return 0;
	/* Check BMP Compression "BI_RGB(no compresstion)"*/
	if (LD_DWORD(Buff+30) != 0) return 0;
	/* Load BMP width */
	bw = LD_DWORD(Buff+18);
	/* Load BMP height */
	bh = LD_DWORD(Buff+22);
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
#if !defined(USE_SSD1332_SPI_OLED)
				d = *p++ >> 3;
				d |= (*p++ >> 2) << 5;
				d |= (*p++ >> 3) << 11;
#else
				d = (*p++ >> 3) << 11;
				d |= (*p++ >> 2) << 5;
				d |= *p++ >> 3;
#endif
				Display_wr_dat_if(d);
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
	IMG Loader.
    Play Streaming ChaN's Original Format VideoFile.
*/
/**************************************************************************/
static int load_img(FIL* fil)
{
	uint8_t f, stat;
	char k;
	unsigned int n, br;
	volatile uint32_t x,y;
	uint32_t d, szfrm, nfrm, cfrm;
	long fd, tp;


	if (LD_WORD(Buff+6) != 16) return 0;

	x = LD_WORD(Buff+2);
	y = LD_WORD(Buff+4);
	if (!x || x > MAX_X || !y || y > MAX_Y) return 0;

	d = LD_DWORD(Buff+8);
	if (f_lseek(fil, d) || d != fil->fptr) return 0;

	f = CsrFlag;
	CsrFlag = 0;

	Display_clear_if();
	Display_rect_if((MAX_X - x) / 2, (MAX_X - x) / 2 + x - 1, (MAX_Y - y) / 2, (MAX_Y - y) / 2 + y - 1);
	szfrm = x * y * 2;

	fd = (long)LD_DWORD(Buff+16);
	nfrm = LD_DWORD(Buff+12);
	cfrm = 0;
	stat = 1;
	tp = TmrFrm + fd;
	for (;;) {
		d = szfrm;
		do {
			n = (d > BUFSIZE) ? BUFSIZE : d;
			if (f_read(fil, Buff, n, &br) || n != br) goto li_exit;
			d -= br;
			Display_wr_block_if(Buff, n);
		} while (d);
#if USE_SSD1963_TFT
		Display_wr_cmd_if(0x002C);	/* SSD1963 Consideration */
#endif
		tp += fd;
		cfrm++;

		for (;;) {
			k = xgetc_n();
			if (k == BTN_CAN) goto li_exit;
			if (k == BTN_ESC) goto li_exit;
			if (k == BTN_OK) {
				stat ^= 1;
				tp = TmrFrm + fd;
			}
			if (cfrm < nfrm) {
				if (stat && TmrFrm >= tp) break;
				if (!stat && k == BTN_UP) break;
			}
			if (!stat && k == BTN_DOWN && cfrm >= 2) {
				if (f_lseek(fil, fil->fptr - szfrm * 2)) goto li_exit;
				cfrm -= 2;
				break;
			}
		}
	}

li_exit:
	CsrFlag = f;
	Display_clear_if();
	return 1;
}


#ifdef USE_TINYJPEG_LIB
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
	int yc, xc, xl, xs;
	uint16_t pd;

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
	do {	/* Send image data */
		xl = xc;
		do {
			pd = *pat++;
			Display_wr_dat_if(pd);
		} while (--xl);
		pat += xs;
	} while (--yc);
	
}
/**************************************************************************/
/*! 
    JPEG file loader Lower-Side Using Chan's JPEG file Decorder
	User defined call-back function to input JPEG data
*/
/**************************************************************************/
static unsigned int tjd_input (
	JDEC* jd,		/* Decoder object */
	BYTE* buff,		/* Pointer to the read buffer (NULL:skip) */
	unsigned int nd			/* Number of bytes to read/skip from input stream */
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
	if (!rect->left && __kbhit()) return 0;	/* Abort decompression */

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
	JDEC jd;		/* Decoder object (124 bytes) */
	JRESULT rc;
	BYTE scale;

	/* Clear screen Anyway */
	Display_clear_if();

	/* Prepare to decompress the file */
	rc = jd_prepare(&jd, tjd_input, work, sz_work, fp);
	if (rc == JDR_OK) {

		/* Determine scale factor */
		for (scale = 0; scale < 3; scale++) {
			if ((jd.width >> scale) <= MAX_X && (jd.height >> scale) <= MAX_Y) break;
		}

		/* Display size information at bottom of screen */
		ts_locate(TS_HEIGHT - 1, 0 ,0);
		xprintf("\33\x87%ux%u 1/%u", jd.width, jd.height, 1 << scale);

		/* Start to decompress the JPEG file */
		rc = jd_decomp(&jd, tjd_output, scale);	/* Start to decompress */

	} else {

		/* Display error code */
		ts_locate(0, 0, 0);
		xprintf("\33\x87 Error: %d", rc);
	}

	/* Exit Routine */
	/* To Rerturn to Push Any Key  */
	wait_anyinput();
}
#endif


#ifdef USE_IJG_LIB
/**************************************************************************/
/*! 
	JPEGFileLoader using IJG JPEG Library.
    Custom Error Handling for IJG JPEG Library.
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
    JPEG file loader Upper-Side Using IJG JPEG Library.
    Display JPEG File.
*/
/**************************************************************************/
static int load_jpeg(FIL *fil,int mode)
{
	/* JPEG Relation */
	int denom,scale,row_stride;
	JSAMPARRAY	buffer;
	struct my_error_mgr jerr;
	struct jpeg_decompress_struct dcinfo;

	/* Scribe Relation */
	uint8_t	 *p;
	uint16_t  x,y,dx,dy,d,i;
	uint32_t  w,h;

	/* Clear Screen Anyway */
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
	
	/* allocate and initialize JPEG decompression object */
	jpeg_create_decompress(&dcinfo);
	jpeg_fatfs_src(&dcinfo, fil);
	jpeg_read_header(&dcinfo, TRUE);
	
	/* Calculate Scalling */
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
	
	/* Set Byte Boundery */
	for (scale = 8; scale > 1; scale--) {
		if (w / denom * scale <= MAX_X)
		break;
	}
	 while (h / denom * scale > MAX_Y) {
		scale--;
	}

	/* Check Valid Scalling */
	if(scale <= 0){
		ts_locate(0, 0 ,0);
		xprintf("\33\x87\ferror\nscaling region over!\n");
		xprintf("press any key\n");
		goto jpeg_end_decode;
	}
	
	/* Check Upper Limit (1280 pixel width) */
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

	while (dcinfo.output_scanline < dcinfo.output_height) {
		jpeg_read_scanlines(&dcinfo, buffer, 1);
		for(i = 0,p = buffer[0];i < dcinfo.output_width;i++) {

		#if !defined(USE_SSD1332_SPI_OLED)
			d =  (*p++ >> 3) << 11;
			d |= (*p++ >> 2) << 5;
			d |=  *p++ >> 3;
		#else
			d =   *p++ >> 3;
			d |= (*p++ >> 2) << 5;
			d |= (*p++ >> 3) << 11;
		#endif

			Display_wr_dat_if(d);
		}
	}

	jpeg_finish_decompress(&dcinfo);
		
jpeg_end_decode:

	/* Free all of the memory associated with the jpeg */
    jpeg_destroy_decompress(&dcinfo);

	/* Exit Routine */
	/* To Rerturn to Push Any Key  */
	wait_anyinput();

   return 1;
}
#endif



#ifdef USE_LIBPNG
/**************************************************************************/
/*! 
    PNG File Loader Lower-Side Using libpng.
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
    PNG file loader Upper-Side Using libpng.
    Decode and Display PNG Data.
*/
/**************************************************************************/
static int load_png(FIL *fil, const char *title)  /* File is already open */
{
	/* Scribe Relation */
	uint16_t lx,ly,d;
	uint32_t i,k=0;
	UINT	 nb;

	/* libpng Structure Pointers */
	png_structp read_ptr;
	png_infop read_info_ptr, end_read_info_ptr;

	/* PNG Informations */
	uint8_t pngSignature[8];
	png_uint_32 width, height,nx,ny;
	int bit_depth, color_type, interlace_type;

	/* Draw to TFT-LCD Relations */
	uint32_t row_stride;
	uint8_t* p;
	uint8_t* row_buffer=NULL;

	/* Confirn PNG File */
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
			/* Diacard Allocated Row Stride */
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

#if 0
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

	/* Push Any Key to Start! */
	wait_anyinput();

	/* Setting Display Limit */
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
	row_stride = (png_get_rowbytes(read_ptr, read_info_ptr) + 3) & ~3; /* 3byte alignments */
	/*row_stride =png_get_rowbytes(read_ptr, read_info_ptr);*/
	row_buffer = png_malloc(read_ptr,row_stride);
   
   /* Diaplay PNG Data */
	for(k = 0;k < ny;k++) {
		png_read_row(read_ptr,row_buffer, NULL );
	
		for(i = 0,p = row_buffer;i < nx;i++) {
		#if !defined(USE_SSD1332_SPI_OLED)
			d  = (*p++ >> 3) << 11;	/* R	 */
			d |= (*p++ >> 2) << 5;	/* G 	 */	
			d |=  *p++ >> 3;		/* B 	 */
		#else
			d =   *p++ >> 3;		/* B	 */
			d |= (*p++ >> 2) << 5;	/* G 	 */	
			d |= (*p++ >> 3) << 11;	/* R	 */
		#endif
			p++;					/* Alpha Channel is Discarded... */
			
			Display_wr_dat_if(d);
		}
	}

	/* Discard Rest Data if u need. */
	if(height > ny){
		i= height-ny;
			for(k = 0;k < i;k++) {
			png_read_row(read_ptr,row_buffer, NULL );
		}
	}

	/* Discard Allocated Row Stride */
	png_free(read_ptr,row_buffer);

	/* PNG Read End Procedure */
	png_read_end(read_ptr, end_read_info_ptr);

png_exit:
	/* Free all of the memory associated with the read_ptr and read_info_ptr */
    png_destroy_read_struct(&read_ptr, &read_info_ptr, &end_read_info_ptr);

	/* Exit Routine */
	/* To Rerturn to Push Any Key  */
	wait_anyinput();

   return 1;
}
#endif


#ifdef USE_GIFLIB
/**************************************************************************/
/*! 
    GIF file loader Upper-Side Using GIFLib.
    Display GIF & Animation GIF Data.
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
	
	/* Scribe Relation */
	uint8_t  c;
	uint16_t d,DelayTime=0;
    volatile unsigned int i,j,n,lx,ly,Size,Row,Col,Width,Height;
	int ErrorCode,ExtCode,TranCol=0;
	
	/* Interlace relation */
    int
	InterlacedOffset[] = { 0, 4, 2, 1 }, 	/* The way Interlaced image should. */
	InterlacedJumps[]  = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */


	/* Load Gif File Pointer */
	if ((GifFile = DGifOpenFileHandle(fil,&ErrorCode)) == NULL) {
	    return 1;
	}

	/* ClearScreen Ready for Displaying GIF file */
	Display_clear_if();

	/* Set Colour Map and BackGround Colour */
	ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);

	/* Check Over Lange */
	if((GifFile->SWidth >MAX_X)||(GifFile->SHeight>MAX_Y))  {
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

	/* Set Colour map and BackGround Colour */
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

			Col 	= GifFile->Image.Left;
			Width 	= GifFile->Image.Width;
			Row 	= GifFile->Image.Top;
			Height 	= GifFile->Image.Height;
			if (GifFile->Image.Left + GifFile->Image.Width > GifFile->SWidth ||
			  GifFile->Image.Top + GifFile->Image.Height > GifFile->SHeight) {
				ts_locate(0, 0 ,0);
				xprintf("\33\x87\fImage %d is not confined to screen dimension, aborted.\n", GifFile->ImageCount);
				xprintf("press any key\n");
				goto gif_end;
			}
	
			if(TranCol == NO_TRANSPARENT_COLOR) {

				if(GifFile->Image.Interlace) {
					/* Need to perform 4 passes on the images: */
					for (i = 0; i < 4; i++)
					for (j = Row + InterlacedOffset[i]; j < Row + Height; j += InterlacedJumps[i]) {
						if (DGifGetLine(GifFile, &RowBuffer[Col], Width) == GIF_ERROR) {
							ts_locate(0, 0 ,0);
							xprintf("\33\x87\fDGifGetLine Error!\n");
							xprintf("press any key\n");
							goto gif_end;
						}

						/* Set Rectangle For Virtual Window */
						Display_rect_if(lx + Col,lx + Col + Width  - 1,
										ly + j  ,ly + j);

						for (n = 0; n < Width; n++) {
							/* Set Global or Local Colour Tables */
							if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
							else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
							ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];

					    #if !defined(USE_SSD1332_SPI_OLED)
							d  = (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
							d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
							d |=  ColorMapEntry->Blue  >> 3;		/* B 	 */
						#else
							d =   ColorMapEntry->Blue  >> 3;		/* B	 */
							d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
							d |= (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
						#endif
							Display_wr_dat_if(d);
						}
					}
				}
				else {

					/* Set Rectangle For Virtual Window */
					Display_rect_if(lx + Col,lx + Col + Width  - 1,
									ly + Row,ly + Row + Height - 1);
								
					for (i = 0; i < Height; i++) {

						if (DGifGetLine(GifFile, &RowBuffer[Col], Width) == GIF_ERROR) {
							ts_locate(0, 0 ,0);
							xprintf("\33\x87\fDGifGetLine Error!\n");
							xprintf("press any key\n");
							goto gif_end;
						}

						for (n = 0; n < Width; n++) {
							/* Set Global or Local Colour Tables */
							if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
							else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
							ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];
				
						#if !defined(USE_SSD1332_SPI_OLED)
							d  = (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
							d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
							d |=  ColorMapEntry->Blue  >> 3;		/* B 	 */
						#else
							d =   ColorMapEntry->Blue  >> 3;		/* B	 */
							d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
							d |= (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
						#endif
							Display_wr_dat_if(d);
						}
					}
				}

			}
			else{
				uint16_t tc;
				GifColorType ct = ColorMap->Colors[TranCol];
				#if !defined(USE_SSD1332_SPI_OLED)
					tc  = (ct.Red   >> 3) << 11;	/* R	 */
					tc |= (ct.Green >> 2) << 5;		/* G 	 */	
					tc |=  ct.Blue  >> 3;			/* B 	 */
				#else
					tc =   ct.Blue  >> 3;			/* B	 */
					tc |= (ct.Green >> 2) << 5;		/* G 	 */	
					tc |= (ct.Red   >> 3) << 11;	/* R	 */
				#endif

				if(GifFile->Image.Interlace) {
					/* Need to perform 4 passes on the images: */
					for (i = 0; i < 4; i++)
					for (j = Row + InterlacedOffset[i]; j < Row + Height; j += InterlacedJumps[i]) {
						if (DGifGetLine(GifFile, &RowBuffer[Col], Width) == GIF_ERROR) {
							ts_locate(0, 0 ,0);
							xprintf("\33\x87\fDGifGetLine Error!\n");
							xprintf("press any key\n");
							goto gif_end;
						}

						for (n = 0; n < Width; n++) {
							/* Set Global or Local Colour Tables */
							if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
							else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
							ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];

							Display_rect_if(lx + n,lx + n,ly + j,ly + j);
							
							#if !defined(USE_SSD1332_SPI_OLED)
								d  = (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
								d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
								d |=  ColorMapEntry->Blue  >> 3;		/* B 	 */
							#else
								d =   ColorMapEntry->Blue  >> 3;		/* B	 */
								d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
								d |= (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
							#endif
								if(tc != d) Display_wr_dat_if(d);
							}
						}
					}
				else {
					for (i = 0; i < Height; i++) {
						if (DGifGetLine(GifFile, &RowBuffer[Col], Width) == GIF_ERROR) {
							ts_locate(0, 0 ,0);
							xprintf("\33\x87\fDGifGetLine Error!\n");
							xprintf("press any key\n");
							goto gif_end;
						}
		
						for (n = 0; n < Width; n++) {
							/* Set Global or Local Colour Tables */
							if      (GifFile->Image.ColorMap) ColorMap = GifFile->Image.ColorMap;
							else if (GifFile->SColorMap) 	  ColorMap = GifFile->SColorMap;
							ColorMapEntry = &ColorMap->Colors[RowBuffer[n]];

							Display_rect_if(lx + n,lx + n,ly + Row + i,ly + Row + i);

							#if !defined(USE_SSD1332_SPI_OLED)
								d  = (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
								d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
								d |=  ColorMapEntry->Blue  >> 3;		/* B 	 */
							#else
								d =   ColorMapEntry->Blue  >> 3;		/* B	 */
								d |= (ColorMapEntry->Green >> 2) << 5;	/* G 	 */	
								d |= (ColorMapEntry->Red   >> 3) << 11;	/* R	 */
							#endif
								if(tc != d) Display_wr_dat_if(d);
						}
					}
				}
			}

			/* Gif Animation Delay With Input Interruption */
			for (n = 0; n < DelayTime ; n++) {
				_delay_ms(10);						/* Wait 0.01*n Sec */
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
	DGifCloseFile(GifFile);
	/* Exit Routine */
	/* To Rerturn to Push Any Key  */
	wait_anyinput();
   return 1;

	/* Case of escape from on loading AnimationGIF */
gif_esc:
	if( RowBuffer != NULL ) {
		free(RowBuffer);
		RowBuffer = NULL;
	}
	DGifCloseFile(GifFile);
   return 1;
 
}
#endif

/**************************************************************************/
/*! 
    Execute IMG,JPEG,BMP,PNG,TXT and WAVE Files.
*/
/**************************************************************************/
int load_file(char *path, char *filename, FIL *fil)
{
	unsigned int n;
	/*FRESULT res;*/


	/* Check File Read Valid */
	if (f_open(fil, path, FA_READ)) return 0;
	
	/* Read File Header */
	/*if (f_read(fil, Buff, 256, &n) || n != 256) return 0;*/
	f_read(fil, Buff, 256, &n);

	/* Execute Original Video File */ 
	if (!memcmp(Buff, "IM", 2)) {
		load_img(fil);
		f_close(fil);
		return RES_OK;
	}

	/* Execute SJIS-Style Text File */
	if (strstr_ext(path, ".TXT")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_txt(fil, (BYTE*)Buff, BUFSIZE);
		f_close(fil);
		return RES_OK;
	}

	/* Execute Windows 24BITMAP File */
	if (!memcmp(Buff, "BM", 2)) {
		load_bmp(fil);
		f_close(fil);
		return RES_OK;
	}

#if defined(USE_IJG_LIB)
	uint8_t JPEG_SOI[] = {0xFF,0xD8,0}; /* JPEG Merker */
	/* Execute JPEG File Using IJG JPEG Library(libjpeg) */
	if (!memcmp(Buff,JPEG_SOI,2)) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_jpeg(fil,1);
		f_close(fil);
		return RES_OK;
	}
#elif defined(USE_TINYJPEG_LIB)
	/* Execute JPEG File Using ChaN's JPEG Decorder */
	if (strstr_ext(path, ".JPG")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_jpg(fil, (BYTE*)Buff, BUFSIZE);
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_LIBPNG)
	/* Execute PNG File Using libpng  */
	if (strstr_ext(path, ".PNG")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_png(fil, filename);
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_GIFLIB)
	/* Execute gif File Using giflib */
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
		load_wav(fil, filename, (BYTE*)Buff, BUFSIZE);
		f_close(fil);
		return RES_OK;
	}
#endif

#if defined(USE_MP3_HELIX)
	/* Execute MPEG2-Layer3 file */
	if (strstr_ext(path, ".MP3")) {
		fil->fptr=0;  /* retrive file pointer to 0 offset */
		load_mp3(fil, filename, (BYTE*)Buff, BUFSIZE);
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
	xprintf(" %s", item->fname);
	for (n = strlen(item->fname); n < 12; n++) xputc(' ');
#if !defined(USE_SSD1332_SPI_OLED)
	if (item->fattr & AM_DIR) {
		xputs("   <DIR>   ");
	} else {
		xprintf("%10u", item->fsize);
	}
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


	/* Try 3 times */
    for(i=0;i<3;i++){
		if (!(f_opendir(dir, path))) break;
		if (i==2) return -1;
	}
	
	i = 0;
	diritem = (DIRITEM*)(void*)Buff;
	while (f_readdir(dir, fno) == FR_OK && fno->fname[0] && i < 200) {
		diritem[i].fsize = fno->fsize;
		diritem[i].fattr = fno->fattrib;
		strcpy(diritem[i].fname, fno->fname);
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

	while (xgetc_n());
	
#if _USE_LFN
	fno->lfname = Lfname;
	fno->lfsize = sizeof(Lfname);
#endif

	for (;;) {
		items = filer_load_dir(path, dir, fno);
		if (items < 0) {
			if (lv > 0) return BTN_ESC;
			ts_locate(0,0,1);
			xputs("\33\x87\fDisk error or No file system.\nSet a microSD card and push (A) to continue...");
			do {
				k = xgetc();
				if (k == BTN_CAN) return BTN_CAN;
				if (k == BTN_ESC) return BTN_ESC;
			} while (k != BTN_OK);
			continue;
		}
#if defined(USE_STM32PRIMER2) || defined(USE_TIME_DISPLAY)
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
#if defined(USE_STM32PRIMER2) || defined(USE_TIME_DISPLAY)
				on_filer =0;
#endif
				return BTN_CAN;
			}
			if (item >= items) continue;
			if (k == BTN_OK) {
				i = strlen(path);
				
#if _USE_LFN
				filenames = (char*)GetLFN(path,diritem[item].fname,dir,fno);
#else
				strcpy(Sfname, diritem[item].fname);
				filenames = Sfname;
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

#if defined(USE_STM32PRIMER2) || defined(USE_TIME_DISPLAY)
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
