/********************************************************************************/
/*!
	@file			usb_msc_scsi_data.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2012.01.30
	@brief          Initialization of the SCSI data.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_msc_scsi.h"
#include "usb_msc_memory.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
uint8_t Page00_Inquiry_Data[] =
  {
    0x00, /* PERIPHERAL QUALIFIER & PERIPHERAL DEVICE TYPE*/
    0x00,
    0x00,
    0x00,
    0x00 /* Supported Pages 00*/
  };
uint8_t Standard_Inquiry_Data[] =
  {
    0x00,          /* Direct Access Device */
    0x80,          /* RMB = 1: Removable Medium */
    0x02,          /* Version: No conformance claim to standard */
    0x02,

    36 - 4,          /* Additional Length */
    0x00,          /* SCCS = 1: Storage Controller Component */
    0x00,
    0x00,
    /* Vendor Identification */
    'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ',
    /* Product Identification */
    'S', 'D', ' ', 'F', 'l', 'a', 's', 'h', ' ',
    'D', 'i', 's', 'k', ' ', ' ', ' ',
    /* Product Revision Level */
    '1', '.', '0', ' '
  };
 
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
uint8_t Mode_Sense6_data[] =
  {
    0x03,
    0x00,
    0x00,
    0x00,
  };

/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

uint8_t Mode_Sense10_data[] =
  {
    0x00,
    0x06,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00
  };
uint8_t Scsi_Sense_Data[] =
  {
    0x70, /*RespCode*/
    0x00, /*SegmentNumber*/
    NO_SENSE, /* Sens_Key*/
    0x00,
    0x00,
    0x00,
    0x00, /*Information*/
    0x0A, /*AdditionalSenseLength*/
    0x00,
    0x00,
    0x00,
    0x00, /*CmdInformation*/
    NO_SENSE, /*Asc*/
    0x00, /*ASCQ*/
    0x00, /*FRUC*/
    0x00, /*TBD*/
    0x00,
    0x00 /*SenseKeySpecific*/
  };
uint8_t ReadCapacity10_Data[] =
  {
    /* Last Logical Block */
    0,
    0,
    0,
    0,

    /* Block Length */
    0,
    0,
    0,
    0
  };

uint8_t ReadFormatCapacity_Data [] =
  {
    0x00,
    0x00,
    0x00,
    0x08, /* Capacity List Length */

    /* Block Count */
    0,
    0,
    0,
    0,

    /* Block Length */
    0x02,/* Descriptor Code: Formatted Media */
    0,
    0,
    0
  };


/* End Of File ---------------------------------------------------------------*/
