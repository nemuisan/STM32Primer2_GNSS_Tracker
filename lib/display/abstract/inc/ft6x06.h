/********************************************************************************/
/*!
	@file			ft6x06.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.05.01
	@brief          Touch Screen Control Upperside 						@n
					Based on ST Microelectronics's Sample Thanks!
	
    @section HISTORY
		2016.06.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef FT6x06_H
#define FT6x06_H	0x0200

#ifdef __cplusplus
 extern "C" {
#endif

/* Basic Includes */
#define FT6x06_ADDR						0x70

/* Touch Macros */
  /* Maximum border values of the touchscreen pad */
#define  FT_6x06_MAX_WIDTH              ((uint16_t)800)     /* Touchscreen pad max width   */
#define  FT_6x06_MAX_HEIGHT             ((uint16_t)480)     /* Touchscreen pad max height  */
  /* Possible values of driver functions return status */
#define FT6x06_STATUS_OK                0
#define FT6x06_STATUS_NOT_OK            1
  /* Possible values of global variable 'TS_I2C_Initialized' */
#define FT6x06_I2C_NOT_INITIALIZED      0
#define FT6x06_I2C_INITIALIZED          1
  /* Max detectable simultaneous touches */
#define FT6x06_MAX_DETECTABLE_TOUCH     2
  /* Current mode register of the FT6x06 (R/W) */
#define FT6x06_DEV_MODE_REG             0x00
  /* Possible values of FT6x06_DEV_MODE_REG */
#define FT6x06_DEV_MODE_WORKING         0x00
#define FT6x06_DEV_MODE_FACTORY         0x04
#define FT6x06_DEV_MODE_MASK            0x7
#define FT6x06_DEV_MODE_SHIFT           4
  /* Gesture ID register */
#define FT6x06_GEST_ID_REG              0x01
  /* Possible values of FT6x06_GEST_ID_REG */
#define FT6x06_GEST_ID_NO_GESTURE       0x00
#define FT6x06_GEST_ID_MOVE_UP          0x10
#define FT6x06_GEST_ID_MOVE_RIGHT       0x14
#define FT6x06_GEST_ID_MOVE_DOWN        0x18
#define FT6x06_GEST_ID_MOVE_LEFT        0x1C
#define FT6x06_GEST_ID_ZOOM_IN          0x48
#define FT6x06_GEST_ID_ZOOM_OUT         0x49
  /* Touch Data Status register : gives number of active touch points (0..2) */
#define FT6x06_TD_STAT_REG              0x02
  /* Values related to FT6x06_TD_STAT_REG */
#define FT6x06_TD_STAT_MASK             0x0F
#define FT6x06_TD_STAT_SHIFT            0x00
  /* Values Pn_XH and Pn_YH related */
#define FT6x06_TOUCH_EVT_FLAG_PRESS_DOWN 0x00
#define FT6x06_TOUCH_EVT_FLAG_LIFT_UP    0x01
#define FT6x06_TOUCH_EVT_FLAG_CONTACT    0x02
#define FT6x06_TOUCH_EVT_FLAG_NO_EVENT   0x03
#define FT6x06_TOUCH_EVT_FLAG_SHIFT     6
#define FT6x06_TOUCH_EVT_FLAG_MASK      (3 << FT6x06_TOUCH_EVT_FLAG_SHIFT)
#define FT6x06_MSB_MASK                 0x0F
#define FT6x06_MSB_SHIFT                0
  /* Values Pn_XL and Pn_YL related */
#define FT6x06_LSB_MASK                 0xFF
#define FT6x06_LSB_SHIFT                0
#define FT6x06_P1_XH_REG                0x03
#define FT6x06_P1_XL_REG                0x04
#define FT6x06_P1_YH_REG                0x05
#define FT6x06_P1_YL_REG                0x06
  /* Touch Pressure register value (R) */
#define FT6x06_P1_WEIGHT_REG            0x07
  /* Values Pn_WEIGHT related  */
#define FT6x06_TOUCH_WEIGHT_MASK        0xFF
#define FT6x06_TOUCH_WEIGHT_SHIFT       0
  /* Touch area register */
#define FT6x06_P1_MISC_REG              0x08
  /* Values related to FT6x06_Pn_MISC_REG */
#define FT6x06_TOUCH_AREA_MASK         (0x04 << 4)
#define FT6x06_TOUCH_AREA_SHIFT        0x04
#define FT6x06_P2_XH_REG               0x09
#define FT6x06_P2_XL_REG               0x0A
#define FT6x06_P2_YH_REG               0x0B
#define FT6x06_P2_YL_REG               0x0C
#define FT6x06_P2_WEIGHT_REG           0x0D
#define FT6x06_P2_MISC_REG             0x0E
  /* Threshold for touch detection */
#define FT6x06_TH_GROUP_REG            0x80
  /* Values FT6x06_TH_GROUP_REG : threshold related  */
#define FT6x06_THRESHOLD_MASK          0xFF
#define FT6x06_THRESHOLD_SHIFT         0
  /* Filter function coefficients */
#define FT6x06_TH_DIFF_REG             0x85
  /* Control register */
#define FT6x06_CTRL_REG                0x86
  /* Values related to FT6x06_CTRL_REG */
  /* Will keep the Active mode when there is no touching */
#define FT6x06_CTRL_KEEP_ACTIVE_MODE    0x00
  /* Switching from Active mode to Monitor mode automatically when there is no touching */
#define FT6x06_CTRL_KEEP_AUTO_SWITCH_MONITOR_MODE  0x01
  /* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT6x06_TIMEENTERMONITOR_REG     0x87
  /* Report rate in Active mode */
#define FT6x06_PERIODACTIVE_REG         0x88
  /* Report rate in Monitor mode */
#define FT6x06_PERIODMONITOR_REG        0x89
  /* The value of the minimum allowed angle while Rotating gesture mode */
#define FT6x06_RADIAN_VALUE_REG         0x91
  /* Maximum offset while Moving Left and Moving Right gesture */
#define FT6x06_OFFSET_LEFT_RIGHT_REG    0x92
  /* Maximum offset while Moving Up and Moving Down gesture */
#define FT6x06_OFFSET_UP_DOWN_REG       0x93
  /* Minimum distance while Moving Left and Moving Right gesture */
#define FT6x06_DISTANCE_LEFT_RIGHT_REG  0x94
  /* Minimum distance while Moving Up and Moving Down gesture */
#define FT6x06_DISTANCE_UP_DOWN_REG     0x95
  /* Maximum distance while Zoom In and Zoom Out gesture */
#define FT6x06_DISTANCE_ZOOM_REG        0x96
  /* High 8-bit of LIB Version info */
#define FT6x06_LIB_VER_H_REG            0xA1
  /* Low 8-bit of LIB Version info */
#define FT6x06_LIB_VER_L_REG            0xA2
  /* Chip Selecting */
#define FT6x06_CIPHER_REG               0xA3
  /* Interrupt mode register (used when in interrupt mode) */
#define FT6x06_GMODE_REG                0xA4
#define FT6x06_G_MODE_INTERRUPT_MASK    0x03
#define FT6x06_G_MODE_INTERRUPT_SHIFT   0x00
  /* Possible values of FT6x06_GMODE_REG */
#define FT6x06_G_MODE_INTERRUPT_POLLING 0x00
#define FT6x06_G_MODE_INTERRUPT_TRIGGER 0x01
  /* Current power mode the FT6x06 system is in (R) */
#define FT6x06_PWR_MODE_REG             0xA5
  /* FT6x06 firmware version */
#define FT6x06_FIRMID_REG               0xA6
  /* FT6x06 Chip identification register */
#define FT6x06_CHIP_ID_REG              0xA8
  /*  Possible values of FT6x06_CHIP_ID_REG */
#define FT6206_ID_VALUE                 0x11
#define FT6x06_ID_VALUE                 0x5A
#define FT5336_ID_VALUE                 0x51
#define FT5216_ID_VALUE                 0x79
#define FT5316_ID_VALUE                 0x98
#define FT5x06_ID_VALUE                 0x87

  /* Release code version */
#define FT6x06_RELEASE_CODE_ID_REG      0xAF
  /* Current operating mode the FT6x06 system is in (R) */
#define FT6x06_STATE_REG                0xBC

/* FT6x06 register struct */
/* Register 0~7 ! */
typedef struct 
{
	uint8_t devmode;
	uint8_t gesture_id;
	uint8_t td_status;
	uint8_t xh;
	uint8_t xl;
	uint8_t yh;
	uint8_t yl;
} ft6x06_strct;


/* Externals */
extern uint8_t FT6x06_ReadByte(uint8_t adrs);
extern uint16_t FT6x06_ReadWord(uint8_t adrs);
extern uint32_t FT6x06_ReadDWord(uint8_t adrs);
extern uint8_t FT6x06_ReadNBytes(uint8_t adrs,uint8_t* buf,uint8_t len);
extern uint32_t FT6x06_ReadFIFO(uint8_t axis,uint8_t adrs);
extern void FT6x06_WriteByte(uint8_t adrs,uint8_t data);
extern void FT6x06_WriteWord(uint8_t adrs,uint16_t data);
extern uint8_t FT6x06_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* FT6x06_H */
