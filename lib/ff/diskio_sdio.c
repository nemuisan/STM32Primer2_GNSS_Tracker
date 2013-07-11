/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* by grqd_xp                                                            */
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
/* by nemui trinomius                                                    */
/* adopted to STM32F2xx_StdPeriph_Driver V1.0.0.						 */
/*-----------------------------------------------------------------------*/
#include <string.h>
#include "diskio.h"
#include "sdio_stm32f1.h"
#include "rtc_support.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */

#define SECTOR_SIZE 512

__IO SD_Error Status = SD_OK;
SD_CardStatus SDCardStatus;

/* dummy values */
#define SOCKPORT	1			/* Socket contact port */
#define SOCKWP		0			/* Write protect switch */
#define SOCKINS		0			/* Card detect switch */
 
 /*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static volatile
DWORD Timer1, Timer2;	/* 100Hz decrement timers */
 
/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
#define SDIO_DRIVE		0


/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */

DSTATUS disk_initialize (
                         BYTE drv				/* Physical drive nmuber (0..) */
                           )
{ 
  switch (drv) 
  {
    case SDIO_DRIVE:
    {     
      /* Initialize SD Card */
      Status = SD_Init(); 
      
      if (Status != SD_OK)
        return STA_NOINIT;
      else
        return 0x00;
    }
  }
  
  return STA_NOINIT;
  
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
                     BYTE drv		/* Physical drive nmuber (0..) */
                       )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {
      Status = SD_GetCardInfo(&SDCardInfo);

      if (Status != SD_OK)
        return STA_NOINIT;
      else
        return 0x00;
    }
  }
  
  return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
                   BYTE drv,		/* Physical drive nmuber (0..) */
                   BYTE *buff,		/* Data buffer to store read data */
                   DWORD sector,	/* Sector address (LBA) */
                   BYTE count		/* Number of sectors to read (1..255) */
                     )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {     
      SD_Error status = SD_OK;
	  
	 
	if(count==1)
    {
		status = SD_ReadBlock((uint8_t*)(buff), 
                              ((uint64_t)(sector)*SECTOR_SIZE),
                              SECTOR_SIZE);
	}
	else
    {
		status = SD_ReadMultiBlocks((uint8_t*)(buff), 
									((uint64_t)(sector)*SECTOR_SIZE),
									SECTOR_SIZE
									,count);
	}
	 
	  
	if (status == SD_OK)	return RES_OK;
	else					return RES_ERROR;
    }

  }
  return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
                    BYTE drv,			/* Physical drive nmuber (0..) */
                    const BYTE *buff,	/* Data to be written */
                    DWORD sector,		/* Sector address (LBA) */
                    BYTE count			/* Number of sectors to write (1..255) */
                      )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {     
      SD_Error status = SD_OK;
	  
	 
	if(count==1)
    {
		status = SD_WriteBlock((uint8_t*)(buff), 
                              ((uint64_t)(sector)*SECTOR_SIZE),
                              SECTOR_SIZE);
	}
	else
    {
		status = SD_WriteMultiBlocks((uint8_t*)(buff), 
									((uint64_t)(sector)*SECTOR_SIZE),
									SECTOR_SIZE
									,count);
	}
	 
	  
	if (status == SD_OK)	return RES_OK;
	else					return RES_ERROR;
    }

  }
  return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
                    BYTE drv,		/* Physical drive nmuber (0..) */
                    BYTE ctrl,		/* Control code */
                    void *buff		/* Buffer to send/receive control data */
                      )
{
  switch (drv) 
  {
    case SDIO_DRIVE:
    {      
      switch (ctrl)
      {
        case CTRL_SYNC:
          /* no synchronization to do since not buffering in this module */
          return RES_OK;
        case GET_SECTOR_SIZE:
          *(uint16_t*)buff = SECTOR_SIZE;
          return RES_OK;
        case GET_SECTOR_COUNT:
          *(uint32_t*)buff = SDCardInfo.CardCapacity / SECTOR_SIZE;
          return RES_OK;
        case GET_BLOCK_SIZE:
          *(uint32_t*)buff = SDCardInfo.CardBlockSize;
		  return RES_OK;
		/* Following command are not used by FatFs module */
		case MMC_GET_TYPE :		/* Get MMC/SDC type (uint8_t) */
			*(uint8_t*)buff = SDCardInfo.CardType;
			return RES_OK;
		case MMC_GET_CSD :		/* Read CSD (16 bytes) */
			memcpy((void *)buff,&SDCardInfo.SD_csd,16);
			return RES_OK;
		case MMC_GET_CID :		/* Read CID (16 bytes) */
			memcpy((void *)buff,&SDCardInfo.SD_cid,16);
			return RES_OK;
		case MMC_GET_OCR :		/* Read OCR (4 bytes) */
			*(uint32_t*)buff = SDCardInfo.SD_csd.MaxRdCurrentVDDMin;
			return RES_OK;
		case MMC_GET_SDSTAT :	/* Read SD status (64 bytes) */
			SD_GetCardStatus(&SDCardStatus);
			memcpy((void *)buff,&SDCardStatus,64);
			return RES_OK;
		default :
			return RES_OK;

      }
    }
  }
  return RES_PARERR;
}


/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure  (Platform dependent)                */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */
/* Not used On STM32Primer2 */
#if 0
void disk_timerproc (void)
{
	static BYTE pv;
	BYTE n, s;

	n = Timer1;						/* 100Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	n = pv;
	pv = SOCKPORT & (SOCKWP | SOCKINS);	/* Sample socket switch */

	if (n == pv) {					/* Have contacts stabled? */
		s = Stat;

		if (pv & SOCKWP)			/* WP is H (write protected) */
			s |= STA_PROTECT;
		else						/* WP is L (write enabled) */
			s &= ~STA_PROTECT;

		if (pv & SOCKINS)			/* INS = H (Socket empty) */
			s |= (STA_NODISK | STA_NOINIT);
		else						/* INS = L (Card inserted) */
			s &= ~STA_NODISK;

		Stat = s;
	}
}
#endif 