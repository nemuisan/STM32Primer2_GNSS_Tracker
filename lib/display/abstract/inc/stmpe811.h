/********************************************************************************/
/*!
	@file			stmpe811.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2013.03.20
	@brief          Touch Screen Control Upperside 
	
    @section HISTORY
		2013.03.20	V1.00	Stable Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __STMPE811_H
#define __STMPE811_H	0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* Basic Includes */
#define STMPE811_ADDR			0x82		/* ADDR0 is GNDed */
/*#define STMPE811_ADDR			 0x88*/

/* Touch Macros */
#define STMPE811_CHIP_ID		0x00
#define STMPE811_ID_VER			0x01
#define STMPE811_SYS_CTRL1      0x03
#define STMPE811_SYS_CTRL2      0x04
#define STMPE811_INT_CTRL       0x09
#define STMPE811_INT_EN         0x0A
#define STMPE811_INT_STA        0x0B
#define STMPE811_GPIO_ALT_FUNCT 0x17
#define STMPE811_ADC_CTRL1      0x20
#define STMPE811_ADC_CTRL2      0x21
#define STMPE811_TSC_CTRL       0x40
#define STMPE811_TSC_CFG        0x41
#define STMPE811_FIFO_TH        0x4A
#define STMPE811_FIFO_STA       0x4B
#define STMPE811_FIFO_SIZE      0x4C
#define STMPE811_DATA_X         0x4D
#define STMPE811_DATA_Y         0x4F
#define STMPE811_DATA_Z         0x51
#define STMPE811_TSC_FRACTION_Z 0x56
#define STMPE811_TSC_I_DRIVE    0x58
#define STMPE811_TSC_SHIELD     0x59
#define STMPE811_DATA_XYZ       0xD7

#define STMPE811_TSC_STA		(1<<7)	
#define STMPE811_READFIFO_XY	3
#define STMPE811_READFIFO_XYZ	4

extern uint8_t STMPE811_ReadByte(uint8_t adrs);
extern uint16_t STMPE811_ReadWord(uint8_t adrs);
extern uint32_t STMPE811_ReadDWord(uint8_t adrs);
extern uint32_t STMPE811_ReadFIFO(uint8_t axis,uint8_t adrs);
extern void STMPE811_WriteByte(uint8_t adrs,uint8_t data);
extern void STMPE811_WriteWord(uint8_t adrs,uint16_t data);
extern uint8_t STMPE811_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __STMPE811_H */
