/* Linker script UpperSide for STM32F103VET6! 		*/
/* Nemui Trinomius (http://nemuisan.blog.bai.ne.jp) */
/* Based on Martin Thomas's sample thanks !			*/

OUTPUT_FORMAT ("elf32-littlearm")

/* Memory Spaces Definitions */
MEMORY
{
	RAM1 (xrw)	: ORIGIN = 0x20000000, LENGTH = 64k		/* Main Embedded SRAM 		*/
	RAM5 (xrw)	: ORIGIN = 0x68000000, LENGTH = 0K		/* External SRAM (FSMC) 	*/
	ROM (rx)	: ORIGIN = 0x08000000, LENGTH = 1024K	/* Main Embedded FlashROM	*/
}

/* higher address of the stack bottom,AAPCS said "stack MUST BE 8byte alignment". */
_estack = (ORIGIN(RAM1)+LENGTH(RAM1)) & ~ 0x7;

/* higher address of the heap end */
/* _eheap = ORIGIN(RAM1)+LENGTH(RAM1); */


/* include the section management sub-script */
INCLUDE "STM32F1x_FLASH.ld"
