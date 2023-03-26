/********************************************************************************/
/*!
	@file			usb_msc_mass_mal.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2023.03.08
	@brief          Descriptor Header for Mal.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.03.21	V2.00	Adopted FatFs10.0a
		2016.01.15	V3.00	Changed definition compatibility.
		2017.01.15	V4.00	Fixed return correct disk capacity on eMMC.
		2017.03.29	V5.00	Fixed capacity calculation.
		2019.09.20	V6.00	Fixed shadowed variable.
		2023.03.08	V7.00	Added external declaration.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef MSC_MASS_MAL_H
#define MSC_MASS_MAL_H	0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes */
#include "platform_config.h"
#include "sdio_stm32f1.h"

/* Macros Plus */
#define MAL_OK   	0
#define MAL_FAIL 	1
#define MAX_LUN  	1
#define LUN_SDCARD	0

/* Externals */
extern uint16_t MAL_Init(uint8_t lun);
extern uint16_t MAL_GetStatus(uint8_t lun);
extern uint16_t MAL_Read(uint8_t lun, uint64_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length);
extern uint16_t MAL_Write(uint8_t lun, uint64_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length);

extern uint64_t Mass_Memory_Size[MAX_LUN];
extern uint32_t Mass_Block_Size[MAX_LUN];
extern uint32_t Mass_Block_Count[MAX_LUN];

#ifdef __cplusplus
}
#endif

#endif /* MSC_MASS_MAL_H */
