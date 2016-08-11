/********************************************************************************/
/*!
	@file			touch_if.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2016.06.01
	@brief          Interface of Touch Panel Hardware Depend Layer				 @n
					Based On "ThaiEasyElec.com BlueScreen" Touch Driver Thanks ! @n

    @section HISTORY
		2010.04.14	V1.00	Start Here.
		2010.12.31	V2.00	Fixed Bit Definitions.
		2011.03.10	V3.00	C++ Ready.
		2011.05.30	V4.00	Separate from Device Depend Section.
		2013.04.04	V5.00	Added STMPE811 Device Handlings.
		2013.11.30	V6.00	Added STM32F429I-Discovery support.
		2016.06.01	V7.00	Added FT6x06 Device Handlings.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __TOUCH_IF_H
#define __TOUCH_IF_H	0x0700

#ifdef __cplusplus
 extern "C" {
#endif

/* Device or MCU Depend Includes */
#include "touch_if_basis.h"

/* FreeRTOS compatibility */
#ifdef INC_FREERTOS_H
#define _delay_ms(x)    vTaskDelay(x/portTICK_RATE_MS);
#define _delay_us(x)    \
    for(volatile uint32_t Count=0;Count<=x;Count++){ 	\
						__NOP();				\
						__NOP();				\
						__NOP();				\
						__NOP();				\
						__NOP();				\
						__NOP();				\
						}
#define __SYSTICK_H

#endif


/* Calibration Settings */
#if defined(USE_32F429IDISCOVERY) /* For STM32F429I-Discovery */
 #define TC_TP_NUM			(2)
#else
 #define TC_TP_NUM			(5)
#endif
#if (TC_TP_NUM > 5) || (TC_TP_NUM < 2)
 #error "TC_TP_NUM MUST be 2 to 5!"
#endif

#define CALIB_TEST_TIME		(2)
#define TC_CALC_X			((MAX_X-20)/(TC_TP_NUM-1))
#define TC_CALC_Y			((MAX_Y-20)/(TC_TP_NUM-1))
#define TC_TPX(x)			(10+(TC_CALC_X)*(x))
#define TC_TPY(x)			(10+(TC_CALC_Y)*(x))

/* Valid Touch Point */
#if (MAX_Y < 480)
#define TOUCH_MIGI			((((MAX_X-40)   < pPos->X_Axis) && (pPos->X_Axis < MAX_X)) && \
							((((MAX_Y/2)-20)< pPos->Y_Axis) && (pPos->Y_Axis < ((MAX_Y/2)+20))))
							
#define TOUCH_HIDARI		(((0 < pPos->X_Axis) && (pPos->X_Axis < 40 )) && \
							((((MAX_Y/2)-20)< pPos->Y_Axis) && (pPos->Y_Axis < ((MAX_Y/2)+20))))
							
#define TOUCH_UE			(((((MAX_X/2)-20)< pPos->X_Axis) && (pPos->X_Axis < ((MAX_X/2)+20))) && \
							((0 < pPos->Y_Axis) && (pPos->Y_Axis < 50 )))

#define TOUCH_SHITA			(((((MAX_X/2)-20)< pPos->X_Axis) && (pPos->X_Axis < ((MAX_X/2)+20))) && \
							(((MAX_Y-50) < pPos->Y_Axis) && (pPos->Y_Axis < MAX_Y)))

#else
#define TOUCH_MIGI			((((MAX_X-80)   < pPos->X_Axis) && (pPos->X_Axis < MAX_X)) && \
							((((MAX_Y/2)-40)< pPos->Y_Axis) && (pPos->Y_Axis < ((MAX_Y/2)+40))))
							
#define TOUCH_HIDARI		(((0 < pPos->X_Axis) && (pPos->X_Axis < 40 )) && \
							((((MAX_Y/2)-40)< pPos->Y_Axis) && (pPos->Y_Axis < ((MAX_Y/2)+40))))
							
#define TOUCH_UE			(((((MAX_X/2)-40)< pPos->X_Axis) && (pPos->X_Axis < ((MAX_X/2)+40))) && \
							((0 < pPos->Y_Axis) && (pPos->Y_Axis < 80 )))

#define TOUCH_SHITA			(((((MAX_X/2)-40)< pPos->X_Axis) && (pPos->X_Axis < ((MAX_X/2)+40))) && \
							(((MAX_Y-80) < pPos->Y_Axis) && (pPos->Y_Axis < MAX_Y)))
#endif

/* Pen Status High Side */
typedef enum TC_STAT_enum
{
    TC_STAT_NONE = (0),  /* Pen is not found  */
    TC_STAT_DOWN = (1),  /* Pen Presed  */
    TC_STAT_HOLD = (2),  /* Pen hold */
    TC_STAT_UP   = (3),  /* Pen Released */
} TC_STAT_t;


/* X,Y Axis Struct */
typedef struct {
	uint16_t X_Axis;
	uint16_t Y_Axis;
}Touch_t;

/* Raw Value */
extern Touch_t  TouchVal;
extern Touch_t* pTouch;
/* Calibrated Value */
extern Touch_t  PosVal;
extern Touch_t* pPos;

/* TouchScreen Related Flags */
extern uint8_t scaned_tc;
extern uint8_t tc_stat;
extern uint8_t last_pen;

/* Extern Functions */
extern void TC_IoInit_If(void);
extern void TC_CalibScreen_If(void);
extern uint8_t TC_PenDown(void);
extern void TC_ReadRaw(void);
extern void TC_ScanPen(void);

#ifdef __cplusplus
}
#endif

#endif /* __TOUCH_IF_H */
