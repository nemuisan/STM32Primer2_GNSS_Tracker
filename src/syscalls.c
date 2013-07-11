/********************************************************************************/
/*!
	@file			syscalls.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2013.07.10
	@brief          Syscall support functions for newlib console I/O with stdio.
					Based on Red Hat newlib C library examples thanks!
					Based on under URL thanks!
					 http://sourceware.org/newlib/libc.html#Syscalls

    @section HISTORY
		2012.08.27	V1.00	Start Here.
		2013.01.07	V2.00	Adopted "--specs=nano.specs" option.
		2013.07.10	V3.00	Adopted semihosting function on nanolib.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <reent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

/* This is platform dependent includion */
#include "syscalls_if.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
/* .ARM.exidx is sorted, so has to go in its own output section.  */
extern char *__exidx_start;
extern char *__exidx_end;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
#if !defined(USE_SEMIHOSTING)
/**************************************************************************/
/*! 
    @brief  Input Datas from STDIN with reentrancy.					@n
			"getch()" indicates retarget function.					@n
			See "syscalls_if.h"!
*/
/**************************************************************************/
_ssize_t _read_r(
	struct _reent *r,
	int file,
	void *ptr,
	size_t len)
{
	char c;
	int  i;
	unsigned char *p;

	p = (unsigned char*)ptr;
	for (i = 0; i < len; i++)
	{
		/* 20090521Nemui */
			do{		
				c = getch();
		}while(c == false);
		/* 20090521Nemui */

		*p++ = c;
		#ifdef ECHOBACK 
		 putch(c);
		#endif

		if (c == '\r' && i <= (len - 2)) /* 0x0D */
		{
			*p = '\n';					 /* 0x0A */
		  #ifdef ECHOBACK 
			putch('\n');				 /* 0x0A */
		  #endif
			return i + 2;
		}
	}
	return i;
}

/**************************************************************************/
/*! 
    @brief  Output Datas to STDOUT with reentrancy.					@n
			"putch(x)" indicates retarget function.					@n
			See "syscalls_if.h"!
*/
/**************************************************************************/
_ssize_t _write_r (
    struct _reent *r, 
    int file, 
    const void *ptr, 
    size_t len)
{
	int i;
	const unsigned char *p;
	
	p = (const unsigned char*) ptr;
	
	for (i = 0; i < len; i++) {
		if (*p == '\n' ) putch('\r');
		putch(*p++);
	}
	
	return len;
}

/**************************************************************************/
/*! 
    @brief  Dummy Function for Newlib C++ Destractor.
*/
/**************************************************************************/
void _fini(void)
{
	/* Nothing to do. */
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib with reentrancy.
*/
/**************************************************************************/
int _close_r(struct _reent *r, int file)
{
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib with reentrancy.
*/
/**************************************************************************/
_off_t _lseek_r(
    struct _reent *r, 
    int file, 
    _off_t ptr, 
    int dir)
{
	return (_off_t)0;	/*  Always indicate we are at file beginning.	*/
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib with reentrancy.
*/
/**************************************************************************/
int _fstat_r(
    struct _reent *r, 
    int file, 
    struct stat *st)
{
	/*  Always set as character device.				*/
	st->st_mode = S_IFCHR;	
	/* assigned to strong type with implicit 		*/
	/* signed/unsigned conversion.  Required by 	*/
	/* newlib.										*/
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Memory Allocator Clue Function.								@n
			Adjusts end of heap to provide more memory to				@n
			memory allocator, With Simple sanity checks.				@n
																		@n
			This implementation is not thread safe (despite taking a	@n
			_reent structure as a parameter). 							@n
			Since _s_r is not used in the current implementation, 		@n
			the following messages must be suppressed.
	@param  struct _reent *_s_r: not in use currently.
	@param  ptrdiff_t nbytes: specify bytes to use memory.
	@retval None.
*/
/**************************************************************************/
extern char end[];              /* MUST be set in linker script command!	*/
								/* file and is the end of statically 		*/
								/* allocated data (thus start of heap).		*/
extern char _heap_end[];		/* Bottom of HEAP region 					*/
static char *heap_ptr=NULL;		/* Points to current end of the heap.		*/
#ifdef USE_SP_AS_HEAP_END
 #warning "use stackpointer as _sbrk's heapend!"
 register char * stack_ptr asm ("sp");
 #define _heap_end stack_ptr
#endif

void * _sbrk_r(
    struct _reent *_s_r, 
    ptrdiff_t nbytes)
{
	char  *base;		/*  errno should be set to  ENOMEM on error	*/

	if (!heap_ptr) {	/*  Initialize if first time through.		*/
		heap_ptr = end;
	}
	base = heap_ptr;	/*  Point to end of heap.					*/
	
	if (heap_ptr + nbytes > _heap_end)
	{
			errno = ENOMEM;
			return (caddr_t) -1;
	}
	heap_ptr += nbytes;	/*  Increase heap.							*/
	
	return base;		/*  Return pointer to start of new heap area.	*/
}



/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
#ifdef __GNUC__
 int isatty(int file); /* avoid warning */
#endif
int isatty(int file)
{
	return 1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _getpid(int file)
{
	return 1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
void * _sbrk(ptrdiff_t incr)
{
	char  *base;

	/* Initialize if first time through. */
	if (!heap_ptr) heap_ptr = end;

	base = heap_ptr;      /*  Point to end of heap.                       */

	if (heap_ptr + incr > _heap_end)
	{
			errno = ENOMEM;
			return (caddr_t) -1;
	}
  
	heap_ptr += incr;     /*  Increase heap.                              */

	return base;          /*  Return pointer to start of new heap area.   */
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _open(const char *path, int flags, ...)
{
	return 1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _close(int fd)
{
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _fstat(int fd, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _isatty(int fd)
{
	return 1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _lseek(int fd, off_t pos, int whence)
{
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _read(int fd, char *buf, size_t cnt)
{
	*buf = getch();

	return 1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _write(int fd, const char *buf, size_t cnt)
{
	int i;

	for (i = 0; i < cnt; i++)
		putch(buf[i]);

	return cnt;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _times(struct tms *buf) {
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _link(char *old, char *new) {
	errno = EMLINK;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _unlink(char *name) {
	errno = ENOENT;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _wait(int *status) {
	errno = ECHILD;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _execve(char *name, char **argv, char **env) {
	errno = ENOMEM;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _fork(void) {
	errno = EAGAIN;
	return -1;
}
#endif

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
void _exit(int n) {
label:  goto label; /* endless loop */
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _kill(int pid, int sig) {
	errno = EINVAL;
	return -1;
}

/* Override fgets() in newlib with a version that does line editing */
/*
char *fgets(char *s, int bufsize, void *f)
{
  cgets(s, bufsize);
  return s;
}
*/

/* End Of File ---------------------------------------------------------------*/
