/********************************************************************************/
/*!
	@file			usb_pwr.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.30
	@brief          Connection/disconnection & power management header.

    @section HISTORY
		2012.01.30	V1.00	Start Here

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_pwr.h"
#include "hw_config.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
__IO uint32_t bDeviceState = UNCONNECTED; /* USB device status */
__IO bool fSuspendEnabled = TRUE;  /* true when suspend is possible */

struct
{
  __IO RESUME_STATE eState;
  __IO uint8_t bESOFcnt;
}
ResumeS;

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  handles switch-off conditions.
	@param  None.
    @retval USB_SUCCESS.
*/
/**************************************************************************/
RESULT PowerOn(void)
{
#ifndef STM32F10X_CL
  uint16_t wRegVal;
  
  /*** cable plugged-in ? ***/
  /*while(!CablePluggedIn());*/
  USB_Cable_Config(ENABLE);

  /*** CNTR_PWDN = 0 ***/
  wRegVal = CNTR_FRES;
  _SetCNTR(wRegVal);

  /*** CNTR_FRES = 0 ***/
  wInterrupt_Mask = 0;
  _SetCNTR(wInterrupt_Mask);
  /*** Clear pending interrupts ***/
  _SetISTR(0);
  /*** Set interrupt mask ***/
  wInterrupt_Mask = CNTR_RESETM | CNTR_SUSPM | CNTR_WKUPM;
  _SetCNTR(wInterrupt_Mask);
#endif /* STM32F10X_CL */
  
  return USB_SUCCESS;
}

/**************************************************************************/
/*! 
    @brief  handles switch-off conditions.
	@param  None.
    @retval USB_SUCCESS.
*/
/**************************************************************************/
RESULT PowerOff()
{
#ifndef STM32F10X_CL  
  /* disable all interrupts and force USB reset */
  _SetCNTR(CNTR_FRES);
  /* clear interrupt status register */
  _SetISTR(0);
  /* Disable the Pull-Up*/
  USB_Cable_Config(DISABLE);
  /* switch-off device */
  _SetCNTR(CNTR_FRES + CNTR_PDWN);
  /* sw variables reset */
  /* ... */
#endif /* STM32F10X_CL */

  return USB_SUCCESS;
}

/**************************************************************************/
/*! 
    @brief  sets suspend mode operating conditions.
	@param  None.
    @retval USB_SUCCESS.
*/
/**************************************************************************/
void Suspend(void)
{

  /* suspend preparation */
  /* ... */
  
#ifndef STM32F10X_CL
  uint16_t wCNTR;

  /* macrocell enters suspend mode */
  wCNTR = _GetCNTR();
  wCNTR |= CNTR_FSUSP;
  _SetCNTR(wCNTR);
#endif /* STM32F10X_CL */
  
  /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
  /* power reduction */
  /* ... on connected devices */

#ifndef STM32F10X_CL
  /* force low-power mode in the macrocell */
  wCNTR = _GetCNTR();
  wCNTR |= CNTR_LPMODE;
  _SetCNTR(wCNTR);
#endif /* STM32F10X_CL */

  /* switch-off the clocks */
  /* ... */
  Enter_LowPowerMode();
}

/**************************************************************************/
/*! 
    @brief  Handles wake-up restoring normal operations.
	@param  None.
    @retval USB_SUCCESS.
*/
/**************************************************************************/
void Resume_Init(void)
{
  /* ------------------ ONLY WITH BUS-POWERED DEVICES ---------------------- */
  /* restart the clocks */
  /* ...  */
#ifndef STM32F10X_CL
  uint16_t wCNTR;
  
  /* CNTR_LPMODE = 0 */
  wCNTR = _GetCNTR();
  wCNTR &= (~CNTR_LPMODE);
  _SetCNTR(wCNTR);
#endif /* STM32F10X_CL */
  
  /* restore full power */
  /* ... on connected devices */
  Leave_LowPowerMode();

#ifndef STM32F10X_CL
  /* reset FSUSP bit */
  _SetCNTR(IMR_MSK);
#endif /* STM32F10X_CL */
  
  /* reverse suspend preparation */
  /* ... */
}

/**************************************************************************/
/*! 
    @brief  This is the state machine handling resume operations and
			timing sequence. The control is based on the Resume structure
			variables and on the ESOF interrupt calling this subroutine
			without changing machine state.
	@param  None.
    @retval None.
*/
/**************************************************************************/
void Resume(RESUME_STATE eResumeSetVal)
{
#ifndef STM32F10X_CL
  uint16_t wCNTR;
#endif /* STM32F10X_CL */

  if (eResumeSetVal != RESUME_ESOF)
    ResumeS.eState = eResumeSetVal;

  switch (ResumeS.eState)
  {
    case RESUME_EXTERNAL:
      Resume_Init();
      ResumeS.eState = RESUME_OFF;
      break;
    case RESUME_INTERNAL:
      Resume_Init();
      ResumeS.eState = RESUME_START;
      break;
    case RESUME_LATER:
      ResumeS.bESOFcnt = 2;
      ResumeS.eState = RESUME_WAIT;
      break;
    case RESUME_WAIT:
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
        ResumeS.eState = RESUME_START;
      break;
    case RESUME_START:
     #ifdef STM32F10X_CL
      OTGD_FS_SetRemoteWakeup();
     #else 
      wCNTR = _GetCNTR();
      wCNTR |= CNTR_RESUME;
      _SetCNTR(wCNTR);
     #endif /* STM32F10X_CL */
      ResumeS.eState = RESUME_ON;
      ResumeS.bESOFcnt = 10;
      break;
    case RESUME_ON:
    #ifndef STM32F10X_CL      
      ResumeS.bESOFcnt--;
      if (ResumeS.bESOFcnt == 0)
      {
     #endif /* STM32F10X_CL */    
       #ifdef STM32F10X_CL
        OTGD_FS_ResetRemoteWakeup();
       #else
        wCNTR = _GetCNTR();
        wCNTR &= (~CNTR_RESUME);
        _SetCNTR(wCNTR);
       #endif /* STM32F10X_CL */
        ResumeS.eState = RESUME_OFF;
     #ifndef STM32F10X_CL
      }
     #endif /* STM32F10X_CL */
      break;
    case RESUME_OFF:
    case RESUME_ESOF:
    default:
      ResumeS.eState = RESUME_OFF;
      break;
  }
}

/* End Of File ---------------------------------------------------------------*/
