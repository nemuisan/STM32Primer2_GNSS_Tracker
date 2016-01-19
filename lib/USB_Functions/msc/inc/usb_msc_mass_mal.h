/********************************************************************************/
/*!
	@file			usb_msc_mass_mal.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2016.01.15
	@brief          Descriptor Header for Mal.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.03.21	V2.00	Adopted FatFs10.0a
		2016.01.15	V3.00	Changed definition compatibility.
		
    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __MASS_MAL_H
#define __MASS_MAL_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes */

/* Macros Plus */
#define MAL_OK   	0
#define MAL_FAIL 	1
#define MAX_LUN  	1
#define LUN_SDCARD	0

/* Externals */
uint16_t MAL_Init (uint8_t lun);
uint16_t MAL_GetStatus (uint8_t lun);
uint16_t MAL_Read(uint8_t lun, uint64_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length);
uint16_t MAL_Write(uint8_t lun, uint64_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length);

#ifdef __cplusplus
}
#endif

#endif /* __MASS_MAL_H */
