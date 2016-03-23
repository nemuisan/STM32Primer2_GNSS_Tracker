/********************************************************************************/
/*!
	@file			sdio_stm32f1.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        21.00
    @date           2016.03.24
	@brief          SDIO Driver For STM32 HighDensity Devices				@n
					Based on STM32F10x_StdPeriph_Driver V3.4.0.

    @section HISTORY
		2011.01.20	V1.00	Start Here.
		2011.03.10	V2.00	C++ Ready.
		2012.04.17	V3.00	Added SD_GetCardStatus().
		2012.09.22  V4.00	Updated Support grater than 32GB Cards.
		2012.10.05  V5.00	Fixed ACMD41 Argument for SDXC(Not UHS-1 mode).
		2013.07.06  V6.00	Fixed over 4GB R/W Problem.
		2013.10.09	V7.00	Integrated with diskio_sdio.c.
		2014.01.15  V8.00   Improved Insertion detect(configuarable).
		2014.03.21  V9.00   Optimized SourceCodes.
		2014.11.18 V10.00   Added SD High Speed Mode(optional).
		2015.01.06 V11.00   Fixed SDIO_CK into suitable value(refered from RM0008_rev14).
		2015.01.23 V12.00   Added Handling SD High Speed Mode description.
		2015.02.14 V13.00	Optimized global structures.
		2015.03.14 V14.00	Removed unused code and improve stability on polling/dma mode.
		2015.11.28 V15.00	Fixed Read CSD/CID registers for disk_ioctl().
		2015.12.03 V16.00	Added Read OCR registers for disk_ioctl().
		2015.12.18 V17.00	Added Read SCR registers for disk_ioctl().
		2016.01.30 V18.00	Added MMCv4.x Cards PreSupport.
		2016.02.21 V19.00	Added MMCv3.x Cards(MMC Native 1-bit Mode) Support.
		2016.03.20 V20.00	Fixed MMCv3.x for stability problem.
		2016.03.24 V21.00	Added MMCv5.x Devices Support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __SDIO_STM32F1_H
#define __SDIO_STM32F1_H	0x2100

#ifdef __cplusplus
 extern "C" {
#endif

/* Generic Inclusion */
#include <stdbool.h>
#include <string.h>
#include "stm32f10x.h"
#include "diskio.h"
 
/* Uncomment the following line to select the SDIO Data transfer mode */ 
#define SD_DMA_MODE
//#define SD_POLLING_MODE

/* Uncomment the following line to select the SD Nomal/High Speed Mode */ 
/* Notice !
   STM32F1 Series can also set ClockBypass Enable.
   But when CPU running at 72MHz, SDIO_CK reaches 72MHz.
   This is excess over clocking!(SDIOCLK=HCLK=SDIO_CK on ClockBypass Enable)

   Nemuisan set SDIO_CK to 36MHz MAX and ClockBypass Disable,and falling edge
   when change into highspeedmode. 
   Of cource this procedure is irregal situation.But works almost SDHC cards!. 
*/
#define SD_NS_MODE
//#define SD_HS_MODE

/* Uncomment the following line to Disable Incert detection */  
/*#define SDIO_INS_DETECT	*/						/* Enable SDIO Incert Detection */
/* SDCARD Incert detection I/O Defninitions */
#define SD_DETECT_PIN                    GPIO_Pin_11                 /* PF.11 */
#define SD_DETECT_GPIO_PORT              GPIOF                       /* GPIOF */
#define SD_DETECT_GPIO_CLK               RCC_APB2Periph_GPIOF


/* Defines */
typedef enum
{
/** 
  * @brief  SDIO specific error defines  
  */   
  SD_CMD_CRC_FAIL                    = (1), /*!< Command response received (but CRC check failed) */
  SD_DATA_CRC_FAIL                   = (2), /*!< Data bock sent/received (CRC check Failed) */
  SD_CMD_RSP_TIMEOUT                 = (3), /*!< Command response timeout */
  SD_DATA_TIMEOUT                    = (4), /*!< Data time out */
  SD_TX_UNDERRUN                     = (5), /*!< Transmit FIFO under-run */
  SD_RX_OVERRUN                      = (6), /*!< Receive FIFO over-run */
  SD_START_BIT_ERR                   = (7), /*!< Start bit not detected on all data signals in widE bus mode */
  SD_CMD_OUT_OF_RANGE                = (8), /*!< CMD's argument was out of range.*/
  SD_ADDR_MISALIGNED                 = (9), /*!< Misaligned address */
  SD_BLOCK_LEN_ERR                   = (10), /*!< Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
  SD_ERASE_SEQ_ERR                   = (11), /*!< An error in the sequence of erase command occurs.*/
  SD_BAD_ERASE_PARAM                 = (12), /*!< An Invalid selection for erase groups */
  SD_WRITE_PROT_VIOLATION            = (13), /*!< Attempt to program a write protect block */
  SD_LOCK_UNLOCK_FAILED              = (14), /*!< Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
  SD_COM_CRC_FAILED                  = (15), /*!< CRC check of the previous command failed */
  SD_ILLEGAL_CMD                     = (16), /*!< Command is not legal for the card state */
  SD_CARD_ECC_FAILED                 = (17), /*!< Card internal ECC was applied but failed to correct the data */
  SD_CC_ERROR                        = (18), /*!< Internal card controller error */
  SD_GENERAL_UNKNOWN_ERROR           = (19), /*!< General or Unknown error */
  SD_STREAM_READ_UNDERRUN            = (20), /*!< The card could not sustain data transfer in stream read operation. */
  SD_STREAM_WRITE_OVERRUN            = (21), /*!< The card could not sustain data programming in stream mode */
  SD_CID_CSD_OVERWRITE               = (22), /*!< CID/CSD overwrite error */
  SD_WP_ERASE_SKIP                   = (23), /*!< only partial address space was erased */
  SD_CARD_ECC_DISABLED               = (24), /*!< Command has been executed without using internal ECC */
  SD_ERASE_RESET                     = (25), /*!< Erase sequence was cleared before executing because an out of erase sequence command was received */
  SD_AKE_SEQ_ERROR                   = (26), /*!< Error in sequence of authentication. */
  SD_INVALID_VOLTRANGE               = (27),
  SD_ADDR_OUT_OF_RANGE               = (28),
  SD_SWITCH_ERROR                    = (29),
  SD_SDIO_DISABLED                   = (30),
  SD_SDIO_FUNCTION_BUSY              = (31),
  SD_SDIO_FUNCTION_FAILED            = (32),
  SD_SDIO_UNKNOWN_FUNCTION           = (33),

/** 
  * @brief  Standard error defines   
  */ 
  SD_INTERNAL_ERROR, 
  SD_NOT_CONFIGURED,
  SD_REQUEST_PENDING, 
  SD_REQUEST_NOT_APPLICABLE, 
  SD_INVALID_PARAMETER,  
  SD_UNSUPPORTED_FEATURE,  
  SD_UNSUPPORTED_HW,  
  SD_ERROR,  
  SD_OK = 0 
} SD_Error;

/** 
  * @brief  SDIO Transfer state  
  */   
typedef enum
{
  SD_TRANSFER_OK  = 0,
  SD_TRANSFER_BUSY = 1,
  SD_TRANSFER_ERROR
} SDTransferState;

/** 
  * @brief  SD Card States 
  */   
typedef enum
{
  SD_CARD_READY                  = ((uint32_t)0x00000001),
  SD_CARD_IDENTIFICATION         = ((uint32_t)0x00000002),
  SD_CARD_STANDBY                = ((uint32_t)0x00000003),
  SD_CARD_TRANSFER               = ((uint32_t)0x00000004),
  SD_CARD_SENDING                = ((uint32_t)0x00000005),
  SD_CARD_RECEIVING              = ((uint32_t)0x00000006),
  SD_CARD_PROGRAMMING            = ((uint32_t)0x00000007),
  SD_CARD_DISCONNECTED           = ((uint32_t)0x00000008),
  SD_CARD_ERROR                  = ((uint32_t)0x000000FF)
}SDCardState;


/** 
  * @brief  Card Specific Data: CSD Register   
  */ 
typedef struct
{
  __IO uint8_t  CSDStruct;            /*!< CSD structure */
  __IO uint8_t  SysSpecVersion;       /*!< System specification version */
  __IO uint8_t  Reserved1;            /*!< Reserved */
  __IO uint8_t  TAAC;                 /*!< Data read access-time 1 */
  __IO uint8_t  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
  __IO uint8_t  MaxBusClkFrec;        /*!< Max. bus clock frequency */
  __IO uint16_t CardComdClasses;      /*!< Card command classes */
  __IO uint8_t  RdBlockLen;           /*!< Max. read data block length */
  __IO uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed */
  __IO uint8_t  WrBlockMisalign;      /*!< Write block misalignment */
  __IO uint8_t  RdBlockMisalign;      /*!< Read block misalignment */
  __IO uint8_t  DSRImpl;              /*!< DSR implemented */
  __IO uint8_t  Reserved2;            /*!< Reserved */
  __IO uint32_t DeviceSize;           /*!< Device Size */
  __IO uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
  __IO uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
  __IO uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
  __IO uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
  __IO uint8_t  DeviceSizeMul;        /*!< Device size multiplier */
  __IO uint8_t  EraseGrSize;          /*!< Erase group size */
  __IO uint8_t  EraseGrMul;           /*!< Erase group size multiplier */
  __IO uint8_t  WrProtectGrSize;      /*!< Write protect group size */
  __IO uint8_t  WrProtectGrEnable;    /*!< Write protect group enable */
  __IO uint8_t  ManDeflECC;           /*!< Manufacturer default ECC */
  __IO uint8_t  WrSpeedFact;          /*!< Write speed factor */
  __IO uint8_t  MaxWrBlockLen;        /*!< Max. write data block length */
  __IO uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
  __IO uint8_t  Reserved3;            /*!< Reserded */
  __IO uint8_t  ContentProtectAppli;  /*!< Content protection application */
  __IO uint8_t  FileFormatGrouop;     /*!< File format group */
  __IO uint8_t  CopyFlag;             /*!< Copy flag (OTP) */
  __IO uint8_t  PermWrProtect;        /*!< Permanent write protection */
  __IO uint8_t  TempWrProtect;        /*!< Temporary write protection */
  __IO uint8_t  FileFormat;           /*!< File Format */
  __IO uint8_t  ECC;                  /*!< ECC code */
  __IO uint8_t  CSD_CRC;              /*!< CSD CRC */
  __IO uint8_t  Reserved4;            /*!< always 1*/
} SD_CSD;

/** 
  * @brief  Card Specific Data: ExtCSD Register for MMCv4.x
  */ 
typedef union
{
		struct _EXT_CSD
		{
			__IO uint8_t  Reserved26[32];
			__IO uint8_t  FLUSH_CACHE;
			__IO uint8_t  CACHE_CTRL;
			__IO uint8_t  POWER_OFF_NOTIFICATION;
			__IO uint8_t  PACKED_FAILURE_INDEX;
			__IO uint8_t  PACKED_COMMAND_STATUS;
			__IO uint8_t  CONTEXT_CONF[15];
			__IO uint8_t  EXT_PARTITIONS_ATTRIBUTE[2];
			__IO uint8_t  EXCEPTION_EVENTS_STATUS[2];
			__IO uint8_t  EXCEPTION_EVENTS_CTRL[2];
			__IO uint8_t  DYNCAP_NEEDED;
			__IO uint8_t  CLASS_6_CTRL;
			__IO uint8_t  INI_TIMEOUT_EMU;
			__IO uint8_t  DATA_SECTOR_SIZE;
			__IO uint8_t  USE_NATIVE_SECTOR;
			__IO uint8_t  NATIVE_SECTOR_SIZE;
			__IO uint8_t  VENDOR_SPECIFIC_FIELD[64];
			__IO uint8_t  Reserved25;
			__IO uint8_t  PROGRAM_CID_CSD_DDR_SUPPORT;
			__IO uint8_t  PERIODIC_WAKEUP;
			__IO uint8_t  TCASE_SUPPORT;
			__IO uint8_t  Reserved24;
			__IO uint8_t  SEC_BAD_BLK_MGMNT;
			__IO uint8_t  Reserved23;
			__IO uint8_t  ENH_START_ADDR[4];
			__IO uint8_t  ENH_SIZE_MULT[3];
			__IO uint8_t  GP_SIZE_MULT[12];
			__IO uint8_t  PARTITION_SETTING_COMPLETED;
			__IO uint8_t  PARTITIONS_ATTRIBUTE;
			__IO uint8_t  MAX_ENH_SIZE_MULT[3];
			__IO uint8_t  PARTITIONING_SUPPORT;
			__IO uint8_t  HPI_MGMT;
			__IO uint8_t  RST_n_FUNCTION;
			__IO uint8_t  BKOPS_EN;
			__IO uint8_t  BKOPS_START;
			__IO uint8_t  SANITIZE_START;
			__IO uint8_t  WR_REL_PARAM;
			__IO uint8_t  WR_REL_SET;
			__IO uint8_t  RPMB_SIZE_MULT;
			__IO uint8_t  FW_CONFIG;
			__IO uint8_t  Reserved22;
			__IO uint8_t  USER_WP;
			__IO uint8_t  Reserved21;
			__IO uint8_t  BOOT_WP;
			__IO uint8_t  BOOT_WP_STATUS;
			__IO uint8_t  ERASE_GROUP_DEF;
			__IO uint8_t  Reserved20;
			__IO uint8_t  BOOT_BUS_CONDITIONS;
			__IO uint8_t  BOOT_CONFIG_PROT;
			__IO uint8_t  PARTITION_CONFIG;
			__IO uint8_t  Reserved19;
			__IO uint8_t  ERASED_MEM_CONT;
			__IO uint8_t  Reserved18;
			__IO uint8_t  BUS_WIDTH;
			__IO uint8_t  Reserved17;
			__IO uint8_t  HS_TIMING;
			__IO uint8_t  Reserved16;
			__IO uint8_t  POWER_CLASS;
			__IO uint8_t  Reserved15;
			__IO uint8_t  CMD_SET_REV;
			__IO uint8_t  Reserved14;
			__IO uint8_t  CMD_SET;
			__IO uint8_t  Reserved13;
			__IO uint8_t  EXT_CSD_REV;
			__IO uint8_t  Reserved12;
			__IO uint8_t  Reserved11;
			__IO uint8_t  Reserved10;
			__IO uint8_t  DEVICE_TYPE;
			__IO uint8_t  DRIVER_STRENGTH;
			__IO uint8_t  OUT_OF_INTERRUPT_TIME;
			__IO uint8_t  PARTITION_SWITCH_TIME;
			__IO uint8_t  PWR_CL_52_195;
			__IO uint8_t  PWR_CL_26_195;
			__IO uint8_t  PWR_CL_52_360;
			__IO uint8_t  PWR_CL_26_360;
			__IO uint8_t  Reserved9;
			__IO uint8_t  MIN_PERF_R_4_26;
			__IO uint8_t  MIN_PERF_W_4_26;
			__IO uint8_t  MIN_PERF_R_8_26_4_52;
			__IO uint8_t  MIN_PERF_W_8_26_4_52;
			__IO uint8_t  MIN_PERF_R_8_52;
			__IO uint8_t  MIN_PERF_W_8_52;
			__IO uint8_t  Reserved8;
			__IO uint8_t  SEC_COUNT[4];
			__IO uint8_t  Reserved7;
			__IO uint8_t  S_A_TIMEOUT;
			__IO uint8_t  Reserved6;
			__IO uint8_t  S_C_VCCQ;
			__IO uint8_t  S_C_VCC;
			__IO uint8_t  HC_WP_GRP_SIZE;
			__IO uint8_t  REL_WR_SEC_C;
			__IO uint8_t  ERASE_TIMEOUT_MULT;
			__IO uint8_t  HC_ERASE_GRP_SIZE;
			__IO uint8_t  ACC_SIZE;
			__IO uint8_t  BOOT_SIZE_MULTI;
			__IO uint8_t  Reserved5;
			__IO uint8_t  BOOT_INFO;
			__IO uint8_t  obsolete2;
			__IO uint8_t  obsolete1;
			__IO uint8_t  SEC_FEATURE_SUPPORT;
			__IO uint8_t  TRIM_MULT;
			__IO uint8_t  Reserved4;
			__IO uint8_t  MIN_PERF_DDR_R_8_52;
			__IO uint8_t  MIN_PERF_DDR_W_8_52;
			__IO uint8_t  PWR_CL_200_195;
			__IO uint8_t  PWR_CL_200_360;
			__IO uint8_t  PWR_CL_DDR_52_195;
			__IO uint8_t  PWR_CL_DDR_52_360;
			__IO uint8_t  Reserved3;
			__IO uint8_t  INI_TIMEOUT_AP;
			__IO uint8_t  CORRECTLY_PRG_SECTORS_NUM[4];
			__IO uint8_t  BKOPS_STATUS[2];
			__IO uint8_t  POWER_OFF_LONG_TIME;
			__IO uint8_t  GENERIC_CMD6_TIME;
			__IO uint8_t  CACHE_SIZE[4];
			__IO uint8_t  Reserved2[255];
			__IO uint8_t  EXT_SUPPORT;
			__IO uint8_t  LARGE_UNIT_SIZE_M1;
			__IO uint8_t  CONTEXT_CAPABILITIES;
			__IO uint8_t  TAG_RES_SIZE;
			__IO uint8_t  TAG_UNIT_SIZE;
			__IO uint8_t  DATA_TAG_SUPPORT;
			__IO uint8_t  MAX_PACKED_WRITES;
			__IO uint8_t  MAX_PACKED_READS;
			__IO uint8_t  BKOPS_SUPPORT;
			__IO uint8_t  HPI_FEATURES;
			__IO uint8_t  S_CMD_SET;
			__IO uint8_t  Reserved1[7];
		} EXT_CSD;
   __IO uint8_t CsdBuf[512];
} MMCEXT_CSD;

/** 
  * @brief  Card Identification Data: CID Register   
  */
typedef struct
{
  __IO uint8_t  ManufacturerID;       /*!< ManufacturerID */
  __IO uint16_t OEM_AppliID;          /*!< OEM/Application ID */
  __IO uint32_t ProdName1;            /*!< Product Name part1 */
  __IO uint8_t  ProdName2;            /*!< Product Name part2*/
  __IO uint8_t  ProdRev;              /*!< Product Revision */
  __IO uint32_t ProdSN;               /*!< Product Serial Number */
  __IO uint8_t  Reserved1;            /*!< Reserved1 */
  __IO uint16_t ManufactDate;         /*!< Manufacturing Date */
  __IO uint8_t  CID_CRC;              /*!< CID CRC */
  __IO uint8_t  Reserved2;            /*!< always 1 */
} SD_CID;

/** 
  * @brief SD Card Status 
  */
typedef struct
{
  __IO uint8_t DAT_BUS_WIDTH;
  __IO uint8_t SECURED_MODE;
  __IO uint16_t SD_CARD_TYPE;
  __IO uint32_t SIZE_OF_PROTECTED_AREA;
  __IO uint8_t SPEED_CLASS;
  __IO uint8_t PERFORMANCE_MOVE;
  __IO uint8_t AU_SIZE;
  __IO uint16_t ERASE_SIZE;
  __IO uint8_t ERASE_TIMEOUT;
  __IO uint8_t ERASE_OFFSET;
} SD_CardStatus;


/** 
  * @brief SD Card information 
  */
typedef struct
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  uint64_t CardCapacity;  /*!< Card Capacity */
  uint32_t CardBlockSize; /*!< Card Block Size */
  uint16_t RCA;
  uint8_t CardType;
} SD_CardInfo;

/**
  * @}
  */
  
/** @defgroup STM32_EVAL_SDIO_SD_Exported_Constants
  * @{
  */ 

/** 
  * @brief SDIO Commands  Index 
  */
#define SD_CMD_GO_IDLE_STATE                       ((uint8_t)0)
#define SD_CMD_SEND_OP_COND                        ((uint8_t)1)
#define SD_CMD_ALL_SEND_CID                        ((uint8_t)2)
#define SD_CMD_SET_REL_ADDR                        ((uint8_t)3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define SD_CMD_SET_DSR                             ((uint8_t)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((uint8_t)5)
#define SD_CMD_HS_SWITCH                           ((uint8_t)6)
#define SD_CMD_SEL_DESEL_CARD                      ((uint8_t)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((uint8_t)8)
#define SD_CMD_SEND_CSD                            ((uint8_t)9)
#define SD_CMD_SEND_CID                            ((uint8_t)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((uint8_t)11) /*!< SD Card doesn't support it */
#define SD_CMD_STOP_TRANSMISSION                   ((uint8_t)12)
#define SD_CMD_SEND_STATUS                         ((uint8_t)13)
#define SD_CMD_HS_BUSTEST_READ                     ((uint8_t)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((uint8_t)15)
#define SD_CMD_SET_BLOCKLEN                        ((uint8_t)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((uint8_t)17)
#define SD_CMD_READ_MULT_BLOCK                     ((uint8_t)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((uint8_t)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((uint8_t)20) /*!< SD Card doesn't support it */
#define SD_CMD_SET_BLOCK_COUNT                     ((uint8_t)23) /*!< SD Card doesn't support it */
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((uint8_t)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((uint8_t)25)
#define SD_CMD_PROG_CID                            ((uint8_t)26) /*!< reserved for manufacturers */
#define SD_CMD_PROG_CSD                            ((uint8_t)27)
#define SD_CMD_SET_WRITE_PROT                      ((uint8_t)28)
#define SD_CMD_CLR_WRITE_PROT                      ((uint8_t)29)
#define SD_CMD_SEND_WRITE_PROT                     ((uint8_t)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((uint8_t)32) /*!< To set the address of the first write
                                                                  block to be erased. (For SD card only) */
#define SD_CMD_SD_ERASE_GRP_END                    ((uint8_t)33) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For SD card only) */
#define SD_CMD_ERASE_GRP_START                     ((uint8_t)35) /*!< To set the address of the first write block to be erased.
                                                                  (For MMC card only spec 3.31) */

#define SD_CMD_ERASE_GRP_END                       ((uint8_t)36) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For MMC card only spec 3.31) */

#define SD_CMD_ERASE                               ((uint8_t)38)
#define SD_CMD_FAST_IO                             ((uint8_t)39) /*!< SD Card doesn't support it */
#define SD_CMD_GO_IRQ_STATE                        ((uint8_t)40) /*!< SD Card doesn't support it */
#define SD_CMD_LOCK_UNLOCK                         ((uint8_t)42)
#define SD_CMD_APP_CMD                             ((uint8_t)55)
#define SD_CMD_GEN_CMD                             ((uint8_t)56)
#define SD_CMD_READ_OCR                            ((uint8_t)58)
#define SD_CMD_NO_CMD                              ((uint8_t)64)

/** 
  * @brief Following commands are SD Card Specific commands.
  *        SDIO_APP_CMD should be sent before sending these commands. 
  */
#define SD_CMD_APP_SD_SET_BUSWIDTH                 ((uint8_t)6)  /*!< For SD Card only */
#define SD_CMD_SD_APP_STAUS                        ((uint8_t)13) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_NUM_WRITE_BLOCKS        ((uint8_t)22) /*!< For SD Card only */
#define SD_CMD_SD_APP_OP_COND                      ((uint8_t)41) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CLR_CARD_DETECT          ((uint8_t)42) /*!< For SD Card only */
#define SD_CMD_SD_APP_SEND_SCR                     ((uint8_t)51) /*!< For SD Card only */
#define SD_CMD_SDIO_RW_DIRECT                      ((uint8_t)52) /*!< For SD I/O Card only */
#define SD_CMD_SDIO_RW_EXTENDED                    ((uint8_t)53) /*!< For SD I/O Card only */

/** 
  * @brief Following commands are SD Card Specific security commands.
  *        SDIO_APP_CMD should be sent before sending these commands. 
  */
#define SD_CMD_SD_APP_GET_MKB                      ((uint8_t)43) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_MID                      ((uint8_t)44) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RN1                  ((uint8_t)45) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RN2                  ((uint8_t)46) /*!< For SD Card only */
#define SD_CMD_SD_APP_SET_CER_RES2                 ((uint8_t)47) /*!< For SD Card only */
#define SD_CMD_SD_APP_GET_CER_RES1                 ((uint8_t)48) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_READ_MULTIPLE_BLOCK   ((uint8_t)18) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MULTIPLE_BLOCK  ((uint8_t)25) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_ERASE                 ((uint8_t)38) /*!< For SD Card only */
#define SD_CMD_SD_APP_CHANGE_SECURE_AREA           ((uint8_t)49) /*!< For SD Card only */
#define SD_CMD_SD_APP_SECURE_WRITE_MKB             ((uint8_t)48) /*!< For SD Card only */

/**
  * @}
  */ 
#define MMC_OCR_REG             0x40FF8080
#define MMC_POWER_REG           0x03BB0800
#define MMC_HIGHSPEED_REG       0x03B90100
#define MMC_4BIT_REG            0x03B70100
#define MMC_8BIT_REG            0x03B70200

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT                                 ((uint8_t)0x01)
#define SD_NOT_PRESENT                             ((uint8_t)0x00)

/** 
  * @brief Supported SD Memory Cards 
  */
#define SDIO_STD_CAPACITY_SD_CARD_V1_1             ((uint32_t)0x00000000)
#define SDIO_STD_CAPACITY_SD_CARD_V2_0             ((uint32_t)0x00000001)
#define SDIO_HIGH_CAPACITY_SD_CARD                 ((uint32_t)0x00000002)
#define SDIO_MULTIMEDIA_CARD                       ((uint32_t)0x00000003)
#define SDIO_SECURE_DIGITAL_IO_CARD                ((uint32_t)0x00000004)
#define SDIO_HIGH_SPEED_MULTIMEDIA_CARD            ((uint32_t)0x00000005)
#define SDIO_SECURE_DIGITAL_IO_COMBO_CARD          ((uint32_t)0x00000006)
#define SDIO_HIGH_CAPACITY_MMC_CARD                ((uint32_t)0x00000007)


/**
  * @brief  SDIO FIFO Address
  */
#define SDIO_FIFO_ADDRESS                ((uint32_t)0x40018080)
/** 
  * @brief  SDIO Intialization Frequency (400KHz max)
  */
#define SDIO_INIT_CLK_DIV                ((uint8_t)0xB2)	/* 72MHz(HCLK MAX Value)/(178+2)= 0.4MHz */
/** 
  * @brief  SDIO Data Transfer Frequency
  *         NomalMode     25MHz(SD Card)/26MHz(MMCv4.x or above)/20MHz(MMCv3.x)
  *         HighSpeedMode 50MHz(SD Card)/52MHz(MMCv4.x or above)
  *			PCLK2,SDIO_CK frequency must meets below equation.
  *           PCLK2 >= SDIO_CK * (3/8)
  * 		PCLK2=72MHz,SDIO_CK can take upto 36MHz(72MHz>=(3/8)*36MHz=13.5MHz).
  *  		PCLK2=24MHz,SDIO_CK can take upto 36MHz(24MHz>=(3/8)*36MHz=13.5MHz).       
  */
/* On STM32F1, SDIOCLK=HCLK.And SDIO_CK = SDIOCLK/(CLK_DIV+2). */
#ifdef SD_DMA_MODE
 #define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x1) 	/* 72MHz(HCLK MAX Value)/(1+2)= 24MHz MAX in Nomal Mode */
#else /* SD_POLLING_MODE */
 #define SDIO_TRANSFER_CLK_DIV            ((uint8_t)0x4) 	/* 72MHz(HCLK MAX Value)/(4+2)= 12MHz MAX in PollingMode(due to TX/RX OVERRUN) */
#endif

/* Function Prototypes */
void SD_DeInit(void);
SD_Error SD_Init(void);
SDTransferState SD_GetStatus(void);
SDCardState SD_GetState(void);
uint8_t SD_Detect(void);
SD_Error SD_PowerON(void);
SD_Error SD_PowerOFF(void);
SD_Error SD_InitializeCards(void);
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SD_GetCardStatus(SD_CardStatus *cardstatus);
SD_Error SD_EnableWideBusOperation(uint32_t WideMode);
SD_Error SD_SetDeviceMode(uint32_t Mode);
SD_Error SD_SelectDeselect(uint64_t addr);
SD_Error SD_ReadBlock(uint8_t *readbuff, uint64_t ReadAddr, uint16_t BlockSize);
SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SD_WriteBlock(uint8_t *writebuff, uint64_t WriteAddr, uint16_t BlockSize);
SD_Error SD_WriteMultiBlocks(uint8_t *writebuff, uint64_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SDTransferState SD_GetTransferState(void);
SD_Error SD_StopTransfer(void);
SD_Error SD_Erase(uint64_t startaddr, uint64_t endaddr);
SD_Error SD_SendStatus(uint32_t *pcardstatus);
SD_Error SD_SendSDStatus(uint32_t *psdstatus);
SD_Error SD_ProcessIRQSrc(void);
SD_Error SD_EnableHighSpeed(void);

extern __IO SD_Error Status;
extern SD_CardInfo SDCardInfo;
extern void _delay_ms(__IO uint32_t mSec);

#ifdef __cplusplus
}
#endif

#endif /* __SDIO_STM32F1_H */
