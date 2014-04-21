/********************************************************************************/
/*!
	@file			display_if.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2011.03.10
	@brief          Interface of Display Device

    @section HISTORY
		2010.07.02	V1.00	Restart Here.
		2010.10.01	V2.00	Add Some Display Modules.
		2010.12.31	V3.00	Add Some Display Modules.
		2011.03.10	V4.00	C++ Ready.
		2013.03.31	V5.00	Added STMPE811 Device Handlings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __DISPLAY_IF_H
#define __DISPLAY_IF_H	0x0500

#ifdef __cplusplus
 extern "C" {
#endif

/* NEED FreeRTOS Support? */
#ifdef USE_FREERTOS
 #include "FreeRTOS.h"
 #include "task.h"
#endif

/* NEED Current Time Display ?(hidden function) */
/* #define USE_TIME_DISPLAY */

/* Select Display Device in MAKEFILE */
#ifdef	USE_ST7735_TFT
 #include "st7735.h"
 
#elif	USE_ST7735_SPI_TFT
 #include "st7735.h"
 
#elif	USE_ST7735R_TFT
 #include "st7735r.h"

#elif	USE_ST7735R_SPI_TFT
 #include "st7735r.h"

#elif	USE_ST7732_TFT
 #include "st7732.h"

#elif	USE_SSD1339_OLED || USE_SSD1339_SPI_OLED
 #include "ssd1339.h"

#elif	USE_SSD2119_TFT
 #include "ssd2119.h"
 
#elif	USE_SSD1963_TFT
 #include "ssd1963.h"

#elif	USE_SSD1289_TFT
 #include "ssd1289.h"

#elif	USE_ILI932x_TFT
 #include "ili932x.h"

#elif	USE_HX8309A_TFT
 #include "hx8309a.h"

#elif	USE_ILI9225x_TFT
 #include "ili9225x.h"
 
#elif	USE_ILI9225x_SPI_TFT
 #include "ili9225x.h"

#elif	USE_ILI9132_TFT
 #include "ili9132.h"

#elif	USE_ILI932x_SPI_TFT
 #include "ili932x.h"

#elif	USE_R61509x_SPI_TFT
 #include "r61509x.h"

#elif	USE_ILI9163x_TFT || USE_ILI9163x_SPI_TFT
 #include "ili9163x.h"

#elif	USE_ILI934x_TFT
 #include "ili934x.h"

#elif	USE_ILI934x_SPI_TFT
 #include "ili934x.h"
 
#elif	USE_SSD1283A_SPI_TFT
 #include "ssd1283a.h"

#elif	USE_S6D0128_TFT
 #include "s6d0128.h"
 
#elif	USE_S6D0129_TFT
 #include "s6d0129.h"
 
#elif	USE_S6D0129_SPI_TFT
 #include "s6d0129.h"

#elif	USE_S6D0144_TFT || USE_S6D0144_SPI_TFT
 #include "s6d0144.h"

#elif	USE_SEPS525_OLED || USE_SEPS525_SPI_OLED
 #include "seps525.h"
 
#elif	USE_S6B33B6x_CSTN || USE_S6B33B6x_SPI_CSTN
 #include "s6b33b6x.h"

#elif	USE_HX5051_OLED || USE_HX5051_SPI_OLED
 #include "hx5051.h"
 #define DISABLE_FASTWRITE
 
#elif	USE_R61509x_TFT
 #include "r61509x.h"

#elif	USE_R61514S_TFT
 #include "r61514s.h"
 
#elif	USE_HX8340B_TFT
 #include "hx8340b.h"

#elif	USE_HX8340B_SPI_TFT
 #include "hx8340b.h"

#elif	USE_HX8340BN_SPI_TFT
 #include "hx8340bn.h"

#elif	USE_HX8312A_TFT
 #include "hx8312a.h"

#elif	USE_HX8347x_TFT
 #include "hx8347x.h"

#elif	USE_HX8352x_TFT || USE_HX8352x_SPI_TFT
 #include "hx8352x.h"

#elif	USE_S1D19122_TFT
 #include "s1d19122.h"

#elif	USE_ILI9481_TFT
 #include "ili9481.h"

#elif	USE_ILI9481_SPI_TFT
 #include "ili9481.h"

#elif	USE_ILI9327_TFT
 #include "ili9327.h"
 
#elif	USE_SSD1332_SPI_OLED
 #include "ssd1332.h"
 
#elif	USE_S6E63D6_OLED
 #include "s6e63d6.h"

#elif	USE_S6E63D6_SPI_OLED
 #include "s6e63d6.h"

#elif	USE_LGDP4511_TFT
 #include "lgdp4511.h"

#elif	USE_LGDP452x_TFT
 #include "lgdp452x.h"

#elif	USE_SSD1286A_TFT
 #include "ssd1286a.h"

#elif	USE_TL1771_TFT
 #include "tl1771.h"

#elif	USE_S1D19105_TFT
 #include "s1d19105.h"

#elif	USE_REL225L01_TFT
 #include "rel225l01.h"

#elif	USE_HD66772_TFT
 #include "hd66772.h"

#elif	USE_HX8347x_SPI_TFT
 #include "hx8347x.h"
 
#elif	USE_SSD2119_SPI_TFT
 #include "ssd2119.h"
 
#elif	USE_SPFD54124_TFT || USE_SPFD54124_SPI_TFT
 #include "spfd54124.h"
 
#elif	USE_C1L5_06_TFT
 #include "c1l5_06.h"

#elif	USE_HX8345A_TFT
 #include "hx8345a.h"
 
#elif	USE_R61526_TFT
 #include "r61526.h"
 
#elif	USE_S6D0117_TFT
 #include "s6d0117.h"

#elif	USE_SSD1351_OLED || USE_SSD1351_SPI_OLED
 #include "ssd1351.h"

#elif	USE_C1E2_04_TFT
 #include "c1e2_04.h"

#elif	USE_D51E5TA7601_TFT
 #include "d51e5ta7601.h"

#elif	USE_HX8357A_TFT
 #include "hx8357a.h"

#elif	USE_UPD161704A_TFT
 #include "upd161704a.h"

#elif	USE_UPD161704A_SPI_TFT
 #include "upd161704a.h"

#elif	USE_ST7787_TFT
 #include "st7787.h"

#elif	USE_SPFD54126_TFT
 #include "spfd54126.h"

#elif	USE_R61503U_TFT
 #include "r61503u.h"

#elif	USE_HX8310A_TFT
 #include "hx8310a.h"

#elif	USE_HX8353x_TFT || USE_HX8353x_SPI_TFT
 #include "hx8353x.h"

#elif	USE_S6D0154_TFT
 #include "s6d0154.h"

#elif	USE_ILI9342_TFT
 #include "ili9342.h"

#elif	USE_LG4538_TFT || USE_LG4538_SPI_TFT
 #include "lg4538.h"

#elif	USE_NT35510_TFT
 #include "nt35510.h"
 
#elif	USE_BD663474_TFT || USE_BD663474_SPI_TFT
 #include "bd663474.h"
 
#elif	USE_ILI9488_TFT
 #include "ili9488.h"
 
#elif	USE_S6D02A1_TFT || USE_S6D02A1_SPI_TFT
 #include "s6d02a1.h"

#elif	USE_S6D05A1_TFT
 #include "s6d05a1.h"

#elif	USE_ILI9341_RGB_TFT
 #include "ili9341_rgb.h"

#elif	USE_HX8369A_TFT
 #include "hx8369a.h"
 

#else
 #error "None of the Selected Display Device!!"
#endif

/* Undefine if not in use of Touchpanel Driver */
#ifdef NO_TOUCH_CTRL
 #undef USE_TOUCH_CTRL
 #undef USE_ADS7843
 #undef USE_STMPE811_I2C
#endif

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_IF_H */
