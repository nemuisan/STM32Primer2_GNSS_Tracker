/********************************************************************************/
/*!
	@file			usb_msc_bot.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.03.23
	@brief          BOT State Machine management.
					Based On STMicro's Sample Thanks!

    @section HISTORY
		2012.01.30	V1.00	Start Here.
		2014.01.23	V2.00	Removed retired STM32F10X_CL Codes.
		2019.09.20	V3.00	Fixed shadowed variable.
		2023.03.23	V4.00	Fixed BOT data buffer to 4byte-alignment.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef USB_MSC_BOT_H
#define USB_MSC_BOT_H	0x0400

#ifdef __cplusplus
 extern "C" {
#endif

/* Basics */
/* Bulk-only Command Block Wrapper */

typedef struct _Bulk_Only_CBW
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataLength;
  uint8_t  bmFlags;
  uint8_t  bLUN;
  uint8_t  bCBLength;
  uint8_t  CB[16];
}
Bulk_Only_CBW;

/* Bulk-only Command Status Wrapper */
typedef struct _Bulk_Only_CSW
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataResidue;
  uint8_t  bStatus;
}
Bulk_Only_CSW;

/* Macros Plus */

/*****************************************************************************/
/*********************** Bulk-Only Transfer State machine ********************/
/*****************************************************************************/
#define BOT_IDLE					0       /* Idle state */
#define BOT_DATA_OUT				1       /* Data Out state */
#define BOT_DATA_IN 				2       /* Data In state */
#define BOT_DATA_IN_LAST			3       /* Last Data In Last */
#define BOT_CSW_Send  				4       /* Command Status Wrapper */
#define BOT_ERROR					5       /* error state */

#define BOT_CBW_SIGNATURE 			0x43425355
#define BOT_CSW_SIGNATURE			0x53425355
#define BOT_CBW_PACKET_LENGTH		31

#define CSW_DATA_LENGTH				0x000D

/* CSW Status Definitions */
#define CSW_CMD_PASSED				0x00
#define CSW_CMD_FAILED				0x01
#define CSW_PHASE_ERROR				0x02

#define SEND_CSW_DISABLE			0
#define SEND_CSW_ENABLE				1

#define DIR_IN						0
#define DIR_OUT						1
#define BOTH_DIR					2

/* Bulk Packet Size (upto 64byte USB-FullSpeed) */
#define BULK_MAX_PACKET_SIZE		64

/* why does this delay need??? anyway that stable it */
#define BOT_STABLE_COUNT			50

/* Externals */
extern void CBW_Decode(void);
extern void Transfer_Data_Request(uint8_t* Data_Pointer, uint16_t Data_Len);
extern void Set_CSW (uint8_t CSW_Status, uint8_t Send_Permission);
extern void Bot_Abort(uint8_t Direction);

extern uint8_t Bulk_Data_Buff[];
extern uint16_t Data_Len;
extern __IO uint8_t Bot_State;
extern __IO Bulk_Only_CBW CBW;
extern __IO Bulk_Only_CSW CSW;

#ifdef __cplusplus
}
#endif

#endif	/* USB_MSC_BOT_H */
