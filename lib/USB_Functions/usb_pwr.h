/********************************************************************************/
/*!
	@file			usb_pwr.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.03.07
	@brief          Connection/disconnection & power management header.

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Adopted STM32_USB-FS-Device_DriverV4.0.0.
		2023.03.07	V3.00	Fixed EP Buffer 8byte-boundery.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef USB_PWR_H
#define USB_PWR_H	0x0300

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes */
#include "usb_core.h"
#include "usb_type.h"

/* Macros Plus */
typedef enum _RESUME_STATE
{
  RESUME_EXTERNAL,
  RESUME_INTERNAL,
  RESUME_LATER,
  RESUME_WAIT,
  RESUME_START,
  RESUME_ON,
  RESUME_OFF,
  RESUME_ESOF
} RESUME_STATE;

typedef enum _DEVICE_STATE
{
  UNCONNECTED,
  ATTACHED,
  POWERED,
  SUSPENDED,
  ADDRESSED,
  CONFIGURED
} DEVICE_STATE;

/* Externals */
void Suspend(void);
void Resume_Init(void);
void Resume(RESUME_STATE eResumeSetVal);
RESULT PowerOn(void);
RESULT PowerOff(void);
extern __IO uint32_t bDeviceState;	/* USB device status */
extern __IO bool fSuspendEnabled;	/* True when suspend is possible */
extern __IO uint16_t EP[8];			/* USB Endpoint Buffer */

#ifdef __cplusplus
}
#endif

#endif	/* USB_PWR_H */
