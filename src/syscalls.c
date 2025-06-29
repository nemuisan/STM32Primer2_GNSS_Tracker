/********************************************************************************/
/*!
	@file			syscalls.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        8.00
    @date           2025.05.29
	@brief          syscall.c's Device Dependent Header Section.

    @section HISTORY
		2010.12.31	V1.00	ReStart here.
		2011.03.10	V2.00	C++ Ready.
		2012.06.15  V3.00	Fixed _heap_end definition.
		2014.06.26	V4.00	Added version check.
		2019.10.01	V5.00	Removed isatty() on GCC build.
		2023.01.24	V6.00	Fixed different signedness.
		2024.07.12	V7.00	Fixed unused parameter.
		2025.05.29	V8.00	Fixed implicit cast warnings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
/* This is platform dependent includion */
#include "syscalls_if.h"
/* check header file version for fool proof */
#if SYSCALLS_IF_H != 0x0800
#error "header file version is not correspond!"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <reent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
/* .ARM.exidx is sorted, so has to go in its own output section. */
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
	(void)r;
	(void)file;
	uint8_t c;
	size_t  i;
	uint8_t *p;

	p = (uint8_t *)ptr;
	for (i = 0; i < len; i++)
	{
		/* 20250515Nemui */
		do{		
			c = getch();
		}while(c == 0);
		/* 20250515Nemui */
		
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
			return (_ssize_t)i + 2;
		}
	}
	return (_ssize_t)i;
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
	(void)r;
	(void)file;
	size_t i;
	const uint8_t *p;
	
	p = (const uint8_t*) ptr;
	
	for (i = 0; i < len; i++) {
		if (*p == '\n' ) putch('\r');
		putch(*p++);
	}
	
	return (_ssize_t)i;
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
	(void)r;
	(void)file;
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
	(void)r;
	(void)file;
	(void)ptr;
	(void)dir;
	return (_off_t)0;	/*  Always indicate we are at file beginning. */
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
	(void)r;
	(void)file;
	/* Always set as character device. */
	st->st_mode = S_IFCHR;	
	/* assigned to strong type with implicit */
	/* signed/unsigned conversion. Required by newlib.*/
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Memory Allocator Glue Function.								@n
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
	(void)_s_r;
	char  *base;		/* errno should be set to ENOMEM on error */

	if (!heap_ptr) {	/* Initialize if first time through. */
		heap_ptr = end;
	}
	base = heap_ptr;	/* Point to end of heap. */
	
	if (heap_ptr + nbytes > _heap_end)
	{
			errno = ENOMEM;
			return (caddr_t) -1;
	}
	heap_ptr += nbytes;	/* Increase heap. */
	
	return base;		/* Return pointer to start of new heap area. */
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
#ifndef __GNUC__
int isatty(int file)
{
	return 1;
}
#endif


/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
void * _sbrk(ptrdiff_t incr)
{
	char  *base;

	if (!heap_ptr) {	/* Initialize if first time through. */
		heap_ptr = end;
	}
	base = heap_ptr;	/* Point to end of heap. */

	if (heap_ptr + incr > _heap_end)
	{
			errno = ENOMEM;
			return (caddr_t) -1;
	}

	heap_ptr += incr;     /* Increase heap. */

	return base;          /* Return pointer to start of new heap area. */
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _open(const char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	return 1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _close(int fd)
{
	(void)fd;
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _fstat(int fd, struct stat *st)
{
	(void)fd;
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
	(void)fd;
	return 1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _lseek(int fd, off_t pos, int whence)
{
	(void)fd;
	(void)pos;
	(void)whence;
	return 0;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
size_t _read(int fd, uint8_t *buf, size_t cnt)
{
	(void)fd;
	size_t i;

	for (i = 0; i < cnt; i++)
		*buf++ = getch();

	return i;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
size_t _write(int fd, const uint8_t *buf, size_t cnt)
{
	(void)fd;
	size_t i;

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
	(void)buf;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _init(struct tms *buf) {
	(void)buf;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _link(char *old, char *new) {
	(void)old;
	(void)new;
	errno = EMLINK;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _unlink(char *name) {
	(void)name;
	errno = ENOENT;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _wait(int *status) {
	(void)status;
	errno = ECHILD;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _execve(char *name, char **argv, char **env) {
	(void)name;
	(void)argv;
	(void)env;
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

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
void _exit(int n){
	(void)n;
label:  goto label; /* endless loop */
}
#endif

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _kill(int pid, int sig) __attribute__((weak));
int _kill(int pid, int sig) {
	(void)pid;
	(void)sig;
	errno = EINVAL;
	return -1;
}

/**************************************************************************/
/*! 
    @brief  Dummy OS Function for Newlib.
*/
/**************************************************************************/
int _getpid(int file) __attribute__((weak));
int _getpid(int file)
{
	(void)file;
	return 1;
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
