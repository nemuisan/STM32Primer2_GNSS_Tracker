/********************************************************************************/
/*!
	@file			usb_msc_memory.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2023.03.23
	@brief          Memory management layer.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2016.01.15	V3.00	Changed definition compatibility.
		2016.12.28	V4.00	Fixed address calculation above 4GB.
		2017.03.29	V5.00	Removed retired variables.
		2019.09.20	V6.00	Fixed shadowed variable.
		2023.03.23	V7.00	Fixed data_buffer capacity.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef USB_MSC_MEMORY_H
#define USB_MSC_MEMORY_H	0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* Include */
#include "stm32f10x.h"
#include "usb_regs.h"
#include "usb_mem.h"
#include "usb_conf.h"
#include "usb_lib.h"
#include "usb_msc_mass_mal.h"
#include "usb_msc_scsi.h"
#include "usb_msc_bot.h"
#include "sdio_stm32f1.h"
 
/* Macros Plus */
#define TXFR_IDLE     0
#define TXFR_ONGOING  1

/* Externals */
void Write_Memory (uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length);
void Read_Memory (uint8_t lun, uint32_t Memory_Offset, uint32_t Transfer_Length);

#ifdef __cplusplus
}
#endif

#endif /* USB_MSC_MEMORY_H */
