/********************************************************************************/
/*!
	@file			sdio_stm32f1.c
	@author			Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
	@version		36.00
	@date			2025.05.03
	@brief			SDIO Driver For STM32 HighDensity Devices				@n
					Based on STM32F10x_StdPeriph_Driver V3.4.0.

    @section HISTORY
		2025.05.03	V36.00	See sdio_stm32f1_ver.txt.

	@section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "sdio_stm32f1.h"
/* check header file version for fool proof */
#if SDIO_STM32F1_H!= 0x3600
#error "header file version is not correspond!"
#endif

/* Duplicate definition warning */
#if defined(SD_DMA_MODE) && !defined(SD_POLLING_MODE)
 #warning "Enable DMA Transfer Handling."
#else
 #warning "Enable FIFO-Polling Transfer Handling."
#endif

#if defined(SD_HS_MODE) && !defined(SD_NS_MODE)
 #warning "Enable SD HighSpeed Mode."
#endif

/* Variables -----------------------------------------------------------------*/

/* Defines -------------------------------------------------------------------*/
/** 
  * @brief  SDIO Static flags, TimeOut, FIFO Address  
  */
#define SDIO_NULL 						0
#define SDIO_STATIC_FLAGS               ((uint32_t)0x000005FF)
#define SDIO_CMD0TIMEOUT                ((uint32_t)0x00010000)

/** 
  * @brief  Mask for errors Card Status R1 (OCR Register) 
  */
#define SD_OCR_ADDR_OUT_OF_RANGE        ((uint32_t)0x80000000)
#define SD_OCR_ADDR_MISALIGNED          ((uint32_t)0x40000000)
#define SD_OCR_BLOCK_LEN_ERR            ((uint32_t)0x20000000)
#define SD_OCR_ERASE_SEQ_ERR            ((uint32_t)0x10000000)
#define SD_OCR_BAD_ERASE_PARAM          ((uint32_t)0x08000000)
#define SD_OCR_WRITE_PROT_VIOLATION     ((uint32_t)0x04000000)
#define SD_OCR_LOCK_UNLOCK_FAILED       ((uint32_t)0x01000000)
#define SD_OCR_COM_CRC_FAILED           ((uint32_t)0x00800000)
#define SD_OCR_ILLEGAL_CMD              ((uint32_t)0x00400000)
#define SD_OCR_CARD_ECC_FAILED          ((uint32_t)0x00200000)
#define SD_OCR_CC_ERROR                 ((uint32_t)0x00100000)
#define SD_OCR_GENERAL_UNKNOWN_ERROR    ((uint32_t)0x00080000)
#define SD_OCR_STREAM_READ_UNDERRUN     ((uint32_t)0x00040000)
#define SD_OCR_STREAM_WRITE_OVERRUN     ((uint32_t)0x00020000)
#define SD_OCR_CID_CSD_OVERWRIETE       ((uint32_t)0x00010000)
#define SD_OCR_WP_ERASE_SKIP            ((uint32_t)0x00008000)
#define SD_OCR_CARD_ECC_DISABLED        ((uint32_t)0x00004000)
#define SD_OCR_ERASE_RESET              ((uint32_t)0x00002000)
#define SD_OCR_AKE_SEQ_ERROR            ((uint32_t)0x00000008)
#define SD_OCR_ERRORBITS                ((uint32_t)0xFDFFE008)

/** 
  * @brief  Masks for R6 Response 
  */
#define SD_R6_GENERAL_UNKNOWN_ERROR     ((uint32_t)0x00002000)
#define SD_R6_ILLEGAL_CMD               ((uint32_t)0x00004000)
#define SD_R6_COM_CRC_FAILED            ((uint32_t)0x00008000)

#define SD_VOLTAGE_WINDOW_SD            ((uint32_t)0x80100000)
#define SD_OCR_XPC                      ((uint32_t)0x10000000)	/* Nemui added SDXC power ctrl (bit28) */
#define SD_OCR_S18                      ((uint32_t)0x01000000)	/* Nemui added Signaling 1.8V req&ans (bit24) */
#define SD_OCR_UHS2                     ((uint32_t)0x20000000)	/* Nemui added UHS-ii card detect (bit29) */
#define SD_HIGH_CAPACITY                ((uint32_t)0x40000000)
#define MMC_VOLTAGE_WINDOW	            ((uint32_t)0x80FF8000)
#define MMC_HIGH_CAPACITY	            ((uint32_t)0x40000000)	/* Bits[30:29]=1,0 is new host argument */
#define MMC_HIGH_CAPACITY_MASK          ((uint32_t)0x60000000)	/* Bits[30:29]=1,0 suggests block address */
#define SD_STD_CAPACITY                 ((uint32_t)0x00000000)
#define SD_CHECK_PATTERN                ((uint32_t)0x000001AA)

#define SD_MAX_VOLT_TRIAL               ((uint32_t)0x0000FFFF)
#define SD_ALLZERO                      ((uint32_t)0x00000000)

#define SD_WIDE_BUS_SUPPORT             ((uint32_t)0x00040000)
#define SD_SINGLE_BUS_SUPPORT           ((uint32_t)0x00010000)
#define SD_CARD_LOCKED                  ((uint32_t)0x02000000)

#define SD_DATATIMEOUT                  ((uint32_t)0x00FFFFFF)
#define SD_0TO7BITS                     ((uint32_t)0x000000FF)
#define SD_8TO15BITS                    ((uint32_t)0x0000FF00)
#define SD_16TO23BITS                   ((uint32_t)0x00FF0000)
#define SD_24TO31BITS                   ((uint32_t)0xFF000000)
#define SD_MAX_DATA_LENGTH              ((uint32_t)0x01FFFFFF)

#define SD_HALFFIFO                     ((uint32_t)0x00000008)
#define SD_HALFFIFOBYTES                ((uint32_t)0x00000020)

/** 
  * @brief  Command Class Supported 
  */
#define SD_CCCC_LOCK_UNLOCK             ((uint32_t)0x00000080)
#define SD_CCCC_WRITE_PROT              ((uint32_t)0x00000040)
#define SD_CCCC_ERASE                   ((uint32_t)0x00000020)

/** 
  * @brief  Following commands are SD Card Specific commands.
  *         SDIO_APP_CMD should be sent before sending these commands. 
  */
#define SDIO_SEND_IF_COND               ((uint32_t)0x00000008)


/* FatFs Glue */
#define SECTOR_SIZE		512		/* Must be Set "512" in use of SDCARD! 			*/
#define SDIO_DRIVE		0		/* Physical Drive Number set to 0. 				*/
#define SOCKWP			0		/* Write Protect Switch is not Supported.		*/
#define NO_ALIGN4CHK	0		/* 0:Do 4Byte aligned check on DMA Mode(Safe).
								   1:Skip 4Byte aligned check on DMA Mode(Unsafe). */

/* Variables -----------------------------------------------------------------*/
static uint32_t CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
static uint32_t CSD_Tab[4], CID_Tab[4], SCR_Tab[2], RCA = 0, OCR = 0;
static uint8_t  MMC_EXTCSDREV = 0;
static uint8_t  MMC_EXTCSDLIFE[2] = {0};
static uint8_t  MMC_EXTCSDEOL = 0;

static uint8_t SDSTATUS_Tab[64];
__IO uint64_t TotalNumberOfBytes = 0;
__IO uint32_t StopCondition = 0;
__IO SD_Error TransferError = SD_OK;
__IO uint32_t TransferEnd = 0;
__IO uint32_t NumberOfBytes = 0;

/* SDCard Structures */
SD_CardInfo SDCardInfo;
SD_CardStatus SDCardStatus;
SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;
#if defined(SD_DMA_MODE)
 DMA_InitTypeDef SDDMA_InitStructure;
 /* If unligned memory address situation,copy dmabuf to aligned by 4-Byte. */
 uint8_t dmabuf[SECTOR_SIZE] __attribute__ ((aligned (4)));
#endif

/* FatFs Glue */
volatile SD_Error Status = SD_OK;
static volatile DSTATUS Stat = STA_NOINIT;	/* Disk status */
static volatile uint32_t Timer1, Timer2;	/* 100Hz decrement timers */

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
static SD_Error CmdError(void);
static SD_Error CmdResp1Error(uint8_t cmd);
static SD_Error CmdResp7Error(void);
static SD_Error CmdResp3Error(void);
static SD_Error CmdResp2Error(void);
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca);
static SD_Error SDEnWideBus(FunctionalState NewState);
static SD_Error MMCEnWideBus(FunctionalState NewState);
static SD_Error IsCardProgramming(uint8_t *pstatus);
static SD_Error FindSCR(uint16_t rca, uint32_t *pscr);
static SD_Error SD_HighSpeed(void);
static SD_Error MMC_HighSpeed(void);
static SD_Error MMC_ReadExtCsd(MMCEXT_CSD *ExtCsd);
static uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes);
#if defined(SD_DMA_MODE)
 static void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize);
 static void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize);
 static uint32_t SD_DMAEndOfTransferStatus(void);
#endif

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief DeInitializes the SDIO interface.
	@param  None
    @retval : None
*/
/**************************************************************************/
void SD_DeInit(void)
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;

	/*!< Disable SDIO Clock */
	SDIO_ClockCmd(DISABLE);

	/*!< Set Power State to OFF */
	SDIO_SetPowerState(SDIO_PowerState_OFF);

	/*!< DeInitializes the SDIO peripheral */
	SDIO_DeInit();

	/*!< Disable the SDIO AHB Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, DISABLE);

	/*!< Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*!< Configure PD.02 CMD line */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}


/**************************************************************************/
/*! 
    @brief  Initializes the SD Card and put it into StandBy State (Ready for data 
            transfer).
	@param  None
    @retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SD_Error errorstatus = SD_OK;

	/* SDIO Peripheral Low Level Init */
	/*!< GPIOC and GPIOD Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD , ENABLE);

	/*!< Configure PC.08, PC.09, PC.10, PC.11, PC.12 pin: D0, D1, D2, D3, CLK pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/*!< Configure PD.02 CMD line */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/*!< Configure SD_SPI_DETECT_PIN pin: SD Card detect pin */
#ifdef SDIO_INS_DETECT
	RCC_APB2PeriphClockCmd(SD_DETECT_GPIO_CLK, ENABLE);
	GPIO_InitStructure.GPIO_Pin = SD_DETECT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(SD_DETECT_GPIO_PORT, &GPIO_InitStructure);
#endif

	/*!< Enable the SDIO AHB Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SDIO, ENABLE);

#if defined(SD_DMA_MODE)
	/*!< Enable the DMA2 Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	/* Initialize SDDMA Structure */
	SDDMA_InitStructure.DMA_PeripheralBaseAddr 	= (uint32_t)SDIO_FIFO_ADDRESS;
	SDDMA_InitStructure.DMA_MemoryBaseAddr 		= 0;
	SDDMA_InitStructure.DMA_DIR 				= DMA_DIR_PeripheralSRC;
	SDDMA_InitStructure.DMA_BufferSize 			= 0;
	SDDMA_InitStructure.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
	SDDMA_InitStructure.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
	SDDMA_InitStructure.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_Word;
	SDDMA_InitStructure.DMA_MemoryDataSize 		= DMA_MemoryDataSize_Word;
	SDDMA_InitStructure.DMA_Mode 				= DMA_Mode_Normal;
	SDDMA_InitStructure.DMA_Priority 			= DMA_Priority_High;
	SDDMA_InitStructure.DMA_M2M 				= DMA_M2M_Disable;
#endif
	/* End of LowLevel Init */


	SDIO_DeInit();

	errorstatus = SD_PowerON();

	if (errorstatus != SD_OK)
	{
		/*!< CMD Response TimeOut (wait for CMDSENT flag) */
		return(errorstatus);
	}

	errorstatus = SD_InitializeCards();

	if (errorstatus != SD_OK)
	{
		/*!< CMD Response TimeOut (wait for CMDSENT flag) */
		return(errorstatus);
	}

	/*!< Configure the SDIO peripheral */
	/*!< SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_TRANSFER_CLK_DIV) */ 
	SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
	SDIO_Init(&SDIO_InitStructure);

	/*!< Enable the SDIO Interrupt */
	NVIC_SetPriority(SDIO_IRQn,SDIO_IRQnPriority);
	NVIC_EnableIRQ(SDIO_IRQn);

	/*----------------- Read CSD/CID MSD registers ------------------*/
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_GetCardInfo(&SDCardInfo);
	}

	/*----------------- Select Card --------------------------------*/
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
	}

	 /* For MMC & eMMC */ 
    if(CardType == SDIO_MULTIMEDIA_CARD				|| \
       CardType == SDIO_HIGH_SPEED_MULTIMEDIA_CARD	|| \
	   CardType == SDIO_HIGH_CAPACITY_MMC_CARD)
	{
		/* Read ExtCSD register and get total sector count
		   for HighCapacity MMC & eMMC (above 4GB,block addressing) */
		if(SDCardInfo.SD_csd.SysSpecVersion >= 4){
			/* Retrieves ExtCSD for V4.x or above cards */
			MMCEXT_CSD ext_csd;
			errorstatus = MMC_ReadExtCsd(&ext_csd);
			if(errorstatus == SD_OK)
			{
				SDCardInfo.CardBlockSize = 1 << (SDCardInfo.SD_csd.RdBlockLen);
				SDCardInfo.CardCapacity = (uint64_t)((uint32_t)(ext_csd.EXT_CSD.SEC_COUNT[3] << 24 | \
																ext_csd.EXT_CSD.SEC_COUNT[2] << 16 | \
																ext_csd.EXT_CSD.SEC_COUNT[1] << 8  | \
																ext_csd.EXT_CSD.SEC_COUNT[0]));
				SDCardInfo.CardCapacity *= 512; /* Fixed to 512 byte for block addressing */
				MMC_EXTCSDREV = ext_csd.EXT_CSD.EXT_CSD_REV;
			}
		}
	}

	/*----------------- Enable SDC/MMC 4BitMode --------------------------------*/
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b);
	}

 	/*----------------- Enable HighSpeedMode --------------------------------*/
#if defined(SD_HS_MODE) && !defined(SD_NS_MODE)
	/* Configure to HighSpeed mode,if card can drive HS Mode. */
	if (errorstatus == SD_OK)
	{
		errorstatus = SD_EnableHighSpeed();
	}
#endif

	return(errorstatus);
}

  
/**************************************************************************/
/*! 
    @brief  Gets the cuurent sd card data transfer status.
	@param  None
    @retval SDTransferState: Data Transfer state.
			This value can be: 
			- SD_TRANSFER_OK: No data transfer is acting
			- SD_TRANSFER_BUSY: Data transfer is acting
*/
/**************************************************************************/
SDTransferState SD_GetStatus(void)
{
	SDCardState cardstate =  SD_CARD_TRANSFER;

	cardstate = SD_GetState();

	if (cardstate == SD_CARD_TRANSFER)
	{
		return(SD_TRANSFER_OK);
	}
	else if(cardstate == SD_CARD_ERROR)
	{
		return (SD_TRANSFER_ERROR);
	}
	else
	{
		return(SD_TRANSFER_BUSY);
	}
}


/**************************************************************************/
/*! 
    @brief  Returns the current card's state.
	@param  None
    @retval SDCardState: SD Card Error or SD Card Current State.
*/
/**************************************************************************/
SDCardState SD_GetState(void)
{
	uint32_t resp1 = 0;

	if(SD_Detect()== SD_PRESENT)
	{
		if (SD_SendStatus(&resp1) != SD_OK)
		{
			return SD_CARD_ERROR;
		}
		else
		{
			return (SDCardState)((resp1 >> 9) & 0x0F);
		}
	}
	else
	{
		return SD_CARD_ERROR;
	}
}


/**************************************************************************/
/*! 
    @brief  Detect if SD card is correctly plugged in the memory slot.
	@param  None
    @retval Return if SD is detected or not
*/
/**************************************************************************/
uint8_t SD_Detect(void)
{
	__IO uint8_t status = SD_PRESENT;

	/*!< Check GPIO to detect SD */
#ifdef SDIO_INS_DETECT

	if(SD_DETECT_GPIO_PORT->IDR & SD_DETECT_PIN)
	{
		status = SD_NOT_PRESENT;
	}
#endif
	return status;
}


/**************************************************************************/
/*! 
    @brief  Enquires cards about their operating voltage and configures 
			clock controls.
	@param  None
    @retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_PowerON(void)
{
	__IO SD_Error errorstatus = SD_OK;
	uint32_t response = 0, count = 0, validvoltage = 0;
	uint32_t SDType = SD_STD_CAPACITY;

	/*!< Power ON Sequence -----------------------------------------------------*/
	/*!< Configure the SDIO peripheral */
	/*!< SDIOCLK = HCLK, SDIO_CK = HCLK/(2 + SDIO_INIT_CLK_DIV) */
	/*!< SDIO_CK for initialization should not exceed 400 KHz */  
	SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
	SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
	SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
	SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
	SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
	SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
	SDIO_Init(&SDIO_InitStructure);

	/*!< Set Power State to ON */
	SDIO_SetPowerState(SDIO_PowerState_ON);

	/*!< Enable SDIO Clock */
	SDIO_ClockCmd(ENABLE);

	/*!< 2mSec Delay: To get at least 74 initial clocks. */
	_delay_ms(2);

	/*!< CMD0: GO_IDLE_STATE ---------------------------------------------------*/
	/*!< No CMD response required */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdError();

	if (errorstatus != SD_OK)
	{
		/*!< CMD Response TimeOut (wait for CMDSENT flag) */
		return(errorstatus);
	}

	/*!< CMD8: SEND_IF_COND ----------------------------------------------------*/
	/*!< Send CMD8 to verify SD card interface operating condition */
	/*!< Argument: - [31:12]: Reserved (shall be set to '0')
			   - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
			   - [7:0]: Check Pattern (recommended 0xAA) */
	/*!< CMD Response: R7 */
	SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp7Error();

	if (errorstatus == SD_OK)
	{
		/*!< SD Card 2.0 */
		CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0;
		SDType = SD_HIGH_CAPACITY;
	}
	else
	{
		/*!< SD Card 1.x or MMC */
		/*!< Re-issue CMD0 to reset card */
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdError();
		
		if (errorstatus != SD_OK)
		{
			/*!< CMD Response TimeOut (wait for CMDSENT flag) */
			return(errorstatus);
		}
	}

	/*!< CMD55 */
	SDIO_CmdInitStructure.SDIO_Argument = 0x00;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

	/*!< If errorstatus is Command TimeOut, it is a MMC card */
	/*!< If errorstatus is SD_OK it is a SD card: SD card 2.0 (voltage range mismatch)
	 or SD card 1.x */
	if (errorstatus == SD_OK)
	{
		/*!< SD CARD */
		/*!< Send ACMD41 SD_APP_OP_COND with Argument 0x80100000 */
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
		{
			/*!< SEND CMD55 APP_CMD with RCA as 0 */
			SDIO_CmdInitStructure.SDIO_Argument = 0x00;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
			
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD	  | \
												  SD_OCR_XPC | SD_OCR_S18 | \
												  SDType;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp3Error();
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			
			response = SDIO_GetResponse(SDIO_RESP1);
			OCR = response;
			validvoltage = (((response >> 31) == 1) ? 1 : 0);
			count++;
		}
		
		if (count >= SD_MAX_VOLT_TRIAL)
		{
			errorstatus = SD_INVALID_VOLTRANGE;
			return(errorstatus);
		}
		
		if (response &= SD_HIGH_CAPACITY)
		{
			CardType = SDIO_HIGH_CAPACITY_SD_CARD;
		}
		
	}
	else
	{	/*!< else MMC Card */
		/*!< Re-issue CMD0 to reset card to pre-idle state for eMMC */
		SDIO_CmdInitStructure.SDIO_Argument = 0xF0F0F0F0;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdError();
		
		if (errorstatus != SD_OK)
		{
			/*!< CMD Response TimeOut (wait for CMDSENT flag) */
			return(errorstatus);
		}
		
		_delay_ms(1); /* From Linux Kernel Implementation(mmc_ops.c) */
		
		/*!< Re-issue CMD0 to reset card */
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdError();
		
		if (errorstatus != SD_OK)
		{
			/*!< CMD Response TimeOut (wait for CMDSENT flag) */
			return(errorstatus);
		}
		
		_delay_ms(1); /* From Linux Kernel Implementation(mmc_ops.c) */
		
		/*!< Send CMD1 SEND_OP_COND with Argument 0x80FF8000 + Bits[30:29]=1,0 */
		while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
		{
			/*!< CMD1: SEND_OP_COND ----------------------------------------------------*/
			/*!< Send CMD1 to receive the contents of the Operating Conditions Register */
			/*!< CMD Response: R3 */
			SDIO_CmdInitStructure.SDIO_Argument = MMC_VOLTAGE_WINDOW | MMC_HIGH_CAPACITY;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp3Error();
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			
			response = SDIO_GetResponse(SDIO_RESP1);
			OCR = response;
			validvoltage = (((response >> 31) == 1) ? 1 : 0);
			count++;
			_delay_ms(4); /* From Linux Kernel Implementation(mmc_ops.c) */
		}
		
		if (count >= SD_MAX_VOLT_TRIAL)
		{
			/* Retry as non-MMC_HIGH_CAPACITY argument */
			response = 0, count = 0, validvoltage = 0;
			
			/*!< Send CMD1 SEND_OP_COND with Argument 0x80FF8000 */
			while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
			{
				/*!< CMD1: SEND_OP_COND ----------------------------------------------------*/
				/*!< Send CMD1 to receive the contents of the Operating Conditions Register */
				/*!< CMD Response: R3 */
				SDIO_CmdInitStructure.SDIO_Argument = MMC_VOLTAGE_WINDOW;
				SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;
				SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
				SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
				SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
				SDIO_SendCommand(&SDIO_CmdInitStructure);
				
				errorstatus = CmdResp3Error();
				if (errorstatus != SD_OK)
				{
					return(errorstatus);
				}
				
				response = SDIO_GetResponse(SDIO_RESP1);
				OCR = response;
				validvoltage = (((response >> 31) == 1) ? 1 : 0);
				count++;
				_delay_ms(4); /* From Linux Kernel Implementation(mmc_ops.c) */
			}
			
			if (count >= SD_MAX_VOLT_TRIAL)
			{
				errorstatus = SD_INVALID_VOLTRANGE;
				return(errorstatus);
			}
		}
		
		if ((response & MMC_HIGH_CAPACITY_MASK) == MMC_HIGH_CAPACITY)
		{
			CardType = SDIO_HIGH_CAPACITY_MMC_CARD;
		}
		else
		{
			CardType = SDIO_MULTIMEDIA_CARD;
		}
	}
	
	return(errorstatus);
}


/**************************************************************************/
/*! 
    @brief  Turns the SDIO output signals off.
	@param  None
    @retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_PowerOFF(void)
{
	SD_Error errorstatus = SD_OK;

	/*!< Set Power State to OFF */
	SDIO_SetPowerState(SDIO_PowerState_OFF);

	return(errorstatus);
}


/**************************************************************************/
/*! 
    @brief  Intialises all cards or single card as the case may be Card(s) come 
			into standby state.
	@param  None
    @retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_InitializeCards(void)
{
	SD_Error errorstatus = SD_OK;
	uint16_t rca = 0x01;

	if (SDIO_GetPowerState() == SDIO_PowerState_OFF)
	{
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return(errorstatus);
	}

	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
	{
		/*!< Send CMD2 ALL_SEND_CID */
		SDIO_CmdInitStructure.SDIO_Argument = 0x0;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp2Error();
		
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
		
		CID_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
		CID_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
		CID_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
		CID_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
	}
	
	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType)	||	(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) 	|| \
       (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType)	||	(SDIO_HIGH_CAPACITY_SD_CARD == CardType)     	|| \
       (SDIO_MULTIMEDIA_CARD == CardType)				||	(SDIO_HIGH_SPEED_MULTIMEDIA_CARD == CardType)   || \
       (SDIO_HIGH_CAPACITY_MMC_CARD  == CardType) )
	{
		/*!< Send CMD3 SET_REL_ADDR with argument 0 */
		/*!< SD Card publishes its RCA. */
		SDIO_CmdInitStructure.SDIO_Argument = 0x00;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);
		
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
	}

	if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
	{
		RCA = rca;
		
		/*!< Send CMD9 SEND_CSD with argument as card's RCA */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16);
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp2Error();
		
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
		
		CSD_Tab[0] = SDIO_GetResponse(SDIO_RESP1);
		CSD_Tab[1] = SDIO_GetResponse(SDIO_RESP2);
		CSD_Tab[2] = SDIO_GetResponse(SDIO_RESP3);
		CSD_Tab[3] = SDIO_GetResponse(SDIO_RESP4);
	}

	errorstatus = SD_OK; /*!< All cards get intialized */

	return(errorstatus);
}


/**************************************************************************/
/*! 
    @brief  Returns information about specific card.
	@param  cardinfo: pointer to a SD_CardInfo structure that contains all SD card 
			information.
    @retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;

	cardinfo->CardType = (uint8_t)CardType;
	cardinfo->RCA = (uint16_t)RCA;

	/*!< Byte 0 */
	tmp = (uint8_t)((CSD_Tab[0] & 0xFF000000) >> 24);
	cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
	cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
	cardinfo->SD_csd.Reserved1 = tmp & 0x03;

	/*!< Byte 1 */
	tmp = (uint8_t)((CSD_Tab[0] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.TAAC = tmp;

	/*!< Byte 2 */
	tmp = (uint8_t)((CSD_Tab[0] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.NSAC = tmp;

	/*!< Byte 3 */
	tmp = (uint8_t)(CSD_Tab[0] & 0x000000FF);
	cardinfo->SD_csd.MaxBusClkFrec = tmp;

	/*!< Byte 4 */
	tmp = (uint8_t)((CSD_Tab[1] & 0xFF000000) >> 24);
	cardinfo->SD_csd.CardComdClasses = tmp << 4;

	/*!< Byte 5 */
	tmp = (uint8_t)((CSD_Tab[1] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
	cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;

	/*!< Byte 6 */
	tmp = (uint8_t)((CSD_Tab[1] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.Reserved2 = 0; /*!< Reserved */

	if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0))
	{
		cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;
		
		/*!< Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
		
		/*!< Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
		cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
		
		cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
		
		/*!< Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
		cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
		cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
		/*!< Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
		cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
		
		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
		cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
		cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
		cardinfo->CardCapacity *= cardinfo->CardBlockSize;
	}
	else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
	{
		/*!< Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;
		
		/*!< Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
		
		cardinfo->SD_csd.DeviceSize |= (tmp << 8);
		
		/*!< Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
		
		cardinfo->SD_csd.DeviceSize |= (tmp);
		
		/*!< Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
		/* nemui fixed due to SD2.00 Capacity fomula is Size = (C_SIZE+1)�~2^19 */
		cardinfo->CardCapacity = ((uint64_t)cardinfo->SD_csd.DeviceSize + 1) * 512 * 1024;
		cardinfo->CardBlockSize = 512;
	}
	else if (CardType == SDIO_MULTIMEDIA_CARD)
	{
		cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;
		
		/*!< Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
		
		/*!< Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
		cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
		
		cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
		
		/*!< Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
		cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
		cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
		/*!< Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
		cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
		
		cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ;
		cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
		cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
		cardinfo->CardCapacity *= cardinfo->CardBlockSize; 
	}
	else if (CardType == SDIO_HIGH_CAPACITY_MMC_CARD)
	{
		cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;
		
		/*!< Byte 7 */
		tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF);
		cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
		
		/*!< Byte 8 */
		tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24);
		cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
		
		cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
		cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
		
		/*!< Byte 9 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16);
		cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
		cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
		cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
		/*!< Byte 10 */
		tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8);
		cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
		
		/* Notice:
		   HighCapacity MMC & eMMC need to read ExtCSD Register(CMD8)
		   to get total sector count. 
		   To read ExtCSD correctly,throw CMD7 at first.
		   Thus on s**kly SPD libraries,cannot execute CMD8 in this function scope.
		   Anyway,I set SDCardInfo value for USB-MSC Example.
		*/
		cardinfo->CardCapacity  = SDCardInfo.CardCapacity;
		cardinfo->CardBlockSize = SDCardInfo.CardBlockSize;
	}

	cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;

	/*!< Byte 11 */
	tmp = (uint8_t)(CSD_Tab[2] & 0x000000FF);
	cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
	cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

	/*!< Byte 12 */
	tmp = (uint8_t)((CSD_Tab[3] & 0xFF000000) >> 24);
	cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
	cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
	cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;

	/*!< Byte 13 */
	tmp = (uint8_t)((CSD_Tab[3] & 0x00FF0000) >> 16);
	cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
	cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.Reserved3 = 0;
	cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

	/*!< Byte 14 */
	tmp = (uint8_t)((CSD_Tab[3] & 0x0000FF00) >> 8);
	cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
	cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
	cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
	cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
	cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
	cardinfo->SD_csd.ECC = (tmp & 0x03);

	/*!< Byte 15 */
	tmp = (uint8_t)(CSD_Tab[3] & 0x000000FF);
	cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
	cardinfo->SD_csd.Reserved4 = 1;

	if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1)  || \
        (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0)  || \
		(CardType == SDIO_HIGH_CAPACITY_SD_CARD))
	{
		/*!< Byte 0 */
		tmp = (uint8_t)((CID_Tab[0] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ManufacturerID = tmp;
		
		/*!< Byte 1 */
		tmp = (uint8_t)((CID_Tab[0] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.OEM_AppliID = tmp << 8;
		
		/*!< Byte 2 */
		tmp = (uint8_t)((CID_Tab[0] & 0x000000FF00) >> 8);
		cardinfo->SD_cid.OEM_AppliID |= tmp;
		
		/*!< Byte 3 */
		tmp = (uint8_t)(CID_Tab[0] & 0x000000FF);
		cardinfo->SD_cid.ProdName1 = tmp << 24;
		
		/*!< Byte 4 */
		tmp = (uint8_t)((CID_Tab[1] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ProdName1 |= tmp << 16;
		
		/*!< Byte 5 */
		tmp = (uint8_t)((CID_Tab[1] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.ProdName1 |= tmp << 8;
		
		/*!< Byte 6 */
		tmp = (uint8_t)((CID_Tab[1] & 0x0000FF00) >> 8);
		cardinfo->SD_cid.ProdName1 |= tmp;
		
		/*!< Byte 7 */
		tmp = (uint8_t)(CID_Tab[1] & 0x000000FF);
		cardinfo->SD_cid.ProdName2 = tmp;
		
		/*!< Byte 8 */
		tmp = (uint8_t)((CID_Tab[2] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ProdRev = tmp;
		
		/*!< Byte 9 */
		tmp = (uint8_t)((CID_Tab[2] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.ProdSN = tmp << 24;
		
		/*!< Byte 10 */
		tmp = (uint8_t)((CID_Tab[2] & 0x0000FF00) >> 8);
		cardinfo->SD_cid.ProdSN |= tmp << 16;
		
		/*!< Byte 11 */
		tmp = (uint8_t)(CID_Tab[2] & 0x000000FF);
		cardinfo->SD_cid.ProdSN |= tmp << 8;
		
		/*!< Byte 12 */
		tmp = (uint8_t)((CID_Tab[3] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ProdSN |= tmp;
		
		/*!< Byte 13 */
		tmp = (uint8_t)((CID_Tab[3] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
		cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;
		
		/*!< Byte 14 */
		tmp = (uint8_t)((CID_Tab[3] & 0x0000FF00) >> 8);
		cardinfo->SD_cid.ManufactDate |= tmp;
		
		/*!< Byte 15 */
		tmp = (uint8_t)(CID_Tab[3] & 0x000000FF);
		cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
		cardinfo->SD_cid.Reserved2 = 1;
	}
	else if ((CardType == SDIO_MULTIMEDIA_CARD) || (CardType == SDIO_HIGH_CAPACITY_MMC_CARD))
	{
		/*!< Byte 0 */
		tmp = (uint8_t)((CID_Tab[0] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ManufacturerID = tmp;
		
		/*!< Byte 1 */
		tmp = (uint8_t)((CID_Tab[0] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.OEM_AppliID = tmp << 8;
		
		/*!< Byte 2 */
		tmp = (uint8_t)((CID_Tab[0] & 0x000000FF00) >> 8);
		cardinfo->SD_cid.OEM_AppliID |= tmp;
		
		/*!< Byte 3 */
		tmp = (uint8_t)(CID_Tab[0] & 0x000000FF);
		cardinfo->SD_cid.ProdName1 = tmp << 24;
		
		/*!< Byte 4 */
		tmp = (uint8_t)((CID_Tab[1] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ProdName1 |= tmp << 16;
		
		/*!< Byte 5 */
		tmp = (uint8_t)((CID_Tab[1] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.ProdName1 |= tmp << 8;
		
		/*!< Byte 6 */
		tmp = (uint8_t)((CID_Tab[1] & 0x0000FF00) >> 8);
		cardinfo->SD_cid.ProdName1 |= tmp;
		
		/*!< Byte 7 */
		tmp = (uint8_t)(CID_Tab[1] & 0x000000FF);
		cardinfo->SD_cid.ProdName2 = tmp << 16;
		
		/*!< Byte 8 */
		tmp = (uint8_t)((CID_Tab[2] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ProdName2 |= tmp;
		
		/*!< Byte 9 */
		tmp = (uint8_t)((CID_Tab[2] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.ProdRev = tmp;
		
		/*!< Byte 10 */
		tmp = (uint8_t)((CID_Tab[2] & 0x0000FF00) >> 8);
		cardinfo->SD_cid.ProdSN = tmp << 24;
		
		/*!< Byte 11 */
		tmp = (uint8_t)((CID_Tab[2] & 0x000000FF));
		cardinfo->SD_cid.ProdSN |= tmp << 16;
		
		/*!< Byte 12 */
		tmp = (uint8_t)((CID_Tab[3] & 0xFF000000) >> 24);
		cardinfo->SD_cid.ProdSN |= tmp << 8;
		
		/*!< Byte 13 */
		tmp = (uint8_t)((CID_Tab[3] & 0x00FF0000) >> 16);
		cardinfo->SD_cid.ProdSN |= tmp;
		
		/*!< Byte 14 */
		tmp = (uint8_t)((CID_Tab[3] & 0x0000FF00) >> 8);
		cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 4;
		
		/*!< Byte 14 */
		tmp = (uint8_t)((CID_Tab[3] & 0x0000FF00) >> 8);
		cardinfo->SD_cid.ManufactDate |= (tmp & 0xF0) >> 4;
		
		/*!< Byte 15 */
		tmp = (uint8_t)(CID_Tab[3] & 0x000000FF);
		cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
		cardinfo->SD_cid.Reserved2 = 1;
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Enables wide bus opeartion for the requeseted card if supported by 
			card.
	@param  WideMode: Specifies the SD card wide bus mode. 
		This parameter can be one of the following values:
		@arg SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
		@arg SDIO_BusWide_4b: 4-bit data transfer
		@arg SDIO_BusWide_1b: 1-bit data transfer
    @retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_EnableWideBusOperation(uint32_t WideMode)
{
	SD_Error errorstatus = SD_OK;

	/*!< MMC Card */
	if ((SDIO_MULTIMEDIA_CARD == CardType) || (SDIO_HIGH_SPEED_MULTIMEDIA_CARD == CardType) || \
		(SDIO_HIGH_CAPACITY_MMC_CARD  == CardType) )
	{
		if (SDIO_BusWide_8b == WideMode)
		{
			errorstatus = SD_UNSUPPORTED_FEATURE;
			return(errorstatus);
		}
		else if (SDIO_BusWide_4b == WideMode)
		{
			/* MMC WideBus Mode Supports above v4 cards! */
			if(SDCardInfo.SD_csd.SysSpecVersion >= 4)
			{
				errorstatus = MMCEnWideBus(ENABLE);
				
				if (SD_OK == errorstatus)
				{
					/*!< Configure the SDIO peripheral */
					SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
					SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
					SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
					SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
					SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
					SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
					SDIO_Init(&SDIO_InitStructure);
				}
			}
			else{
				/* Configure the SDMMC peripheral as MMCv3.x(16MHz@48MHzSCLK) */
				SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV+1; /* MMCv3.x card need below 20MHz clock! */
				SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
				SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
				SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
				SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
				SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
				SDIO_Init(&SDIO_InitStructure);
			}
		}
		else
		{
			errorstatus = MMCEnWideBus(DISABLE);
			
			if (SD_OK == errorstatus)
			{
				/* Configure the SDMMC peripheral as MMCv3.x(16MHz@48MHzSCLK) */
				SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV+1; /* MMCv3.x card need below 20MHz clock! */
				SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
				SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
				SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
				SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
				SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
				SDIO_Init(&SDIO_InitStructure);
			}
		}
	}
	/*!< SDCARD */
	else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || \
             (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		if (SDIO_BusWide_8b == WideMode)
		{
			errorstatus = SD_UNSUPPORTED_FEATURE;
			return(errorstatus);
		}
		else if (SDIO_BusWide_4b == WideMode)
		{
			errorstatus = SDEnWideBus(ENABLE);
			
			if (SD_OK == errorstatus)
			{
				/*!< Configure the SDIO peripheral */
				SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
				SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
				SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
				SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
				SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
				SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
				SDIO_Init(&SDIO_InitStructure);
			}
		}
		else
		{
			errorstatus = SDEnWideBus(DISABLE);
			
			if (SD_OK == errorstatus)
			{
				/*!< Configure the SDIO peripheral */
				SDIO_InitStructure.SDIO_ClockDiv = SDIO_TRANSFER_CLK_DIV; 
				SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
				SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
				SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
				SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
				SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
				SDIO_Init(&SDIO_InitStructure);
			}
		}
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Selects od Deselects the corresponding card.
	@param  addr: Address of the Card to be selected.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_SelectDeselect(uint64_t addr)
{
	SD_Error errorstatus = SD_OK;

	/*!< Send CMD7 SDIO_SEL_DESEL_CARD */
	SDIO_CmdInitStructure.SDIO_Argument =  (uint32_t)addr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SEL_DESEL_CARD);

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Allows to read one block from a specified address in a card.
	@param  readbuff: pointer to the buffer that will contain the received data
	@param  ReadAddr: Address from where data are to be read.  
	@param  BlockSize: the SD card Data block size.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_ReadBlock(uint8_t *readbuff, uint64_t ReadAddr, uint16_t BlockSize)
{
	SD_Error errorstatus = SD_OK;
	uint8_t power = 0;
	TotalNumberOfBytes = 0;
	TransferError = SD_OK;
	TransferEnd = 0;

#if defined(SD_POLLING_MODE)
	uint32_t count = 0, *tempbuff = (uint32_t *)readbuff;
#endif

	/*!< Clear all DPSM configuration */
	SDIO->DCTRL = 0x0;
	SDIO_DMACmd(DISABLE);
	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	if ((CardType == SDIO_HIGH_CAPACITY_SD_CARD) || (CardType == SDIO_HIGH_CAPACITY_MMC_CARD))
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}

	if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);

		/*!< Set Block Size for Card */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
		
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
	}
	else
	{
		errorstatus = SD_INVALID_PARAMETER;
		return(errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) power << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	TotalNumberOfBytes = BlockSize;

#if defined(SD_DMA_MODE)
	StopCondition = 0;
	SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
	SDIO_DMACmd(ENABLE);
	SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, BlockSize);
#endif

	/*!< Send CMD17 READ_SINGLE_BLOCK */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)ReadAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

#if defined(SD_POLLING_MODE)
	/*!< In case of single block transfer, no need of stop transfer at all.*/
	/*!< Polling mode */
	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
		{
			for (count = 0; count < SD_HALFFIFO; count++)
			{
				*(tempbuff + count) = SDIO_ReadData();
			}
			tempbuff += SD_HALFFIFO;
		}
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	count = SD_DATATIMEOUT;
	while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
	{
		*tempbuff = SDIO_ReadData();
		tempbuff++;
		count--;
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

#elif defined(SD_DMA_MODE)
	/*!< DMA mode */
	while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK))
	{}
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}
#endif

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Allows to read blocks from a specified address  in a card.
	@param  readbuff: pointer to the buffer that will contain the received data.
	@param  ReadAddr: Address from where data are to be read.
	@param  BlockSize: the SD card Data block size.
	@param  NumberOfBlocks: number of blocks to be read.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_ReadMultiBlocks(uint8_t *readbuff, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
	uint8_t power = 0;
	TotalNumberOfBytes = 0;
	TransferError = SD_OK;
	TransferEnd = 0;

#if defined(SD_POLLING_MODE)
	uint32_t count = 0, *tempbuff = (uint32_t *)readbuff;
#endif

	/*!< Clear all DPSM configuration */
	SDIO->DCTRL = 0x0;
	SDIO_DMACmd(DISABLE);
	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	if ((CardType == SDIO_HIGH_CAPACITY_SD_CARD) || (CardType == SDIO_HIGH_CAPACITY_MMC_CARD))
	{
		BlockSize = 512;
		ReadAddr /= 512;
	}

	if ((BlockSize > 0) && (BlockSize <= 2048) && (0 == (BlockSize & (BlockSize - 1))))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);
		
		/*!< Set Block Size for Card */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
		
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
	}
	else
	{
		errorstatus = SD_INVALID_PARAMETER;
		return(errorstatus);
	}

	/*!< Common to all modes */
	if (NumberOfBlocks * BlockSize > SD_MAX_DATA_LENGTH)
	{
		errorstatus = SD_INVALID_PARAMETER;
		return(errorstatus);
	}

	TotalNumberOfBytes = NumberOfBlocks * BlockSize;

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) power << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

#if defined(SD_DMA_MODE)
	StopCondition = 1;
	SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_RXOVERR | SDIO_IT_STBITERR, ENABLE);
	SDIO_DMACmd(ENABLE);
	SD_LowLevel_DMA_RxConfig((uint32_t *)readbuff, (NumberOfBlocks * BlockSize));
#endif

	/*!< Send CMD18 READ_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)ReadAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

#if defined(SD_POLLING_MODE)
	/*!< Polling mode */
	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DATAEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
		{
			for (count = 0; count < SD_HALFFIFO; count++)
			{
				*(tempbuff + count) = SDIO_ReadData();
			}
			tempbuff += SD_HALFFIFO;
		}
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)
	{
		/*!< In Case Of SD-CARD Send Command STOP_TRANSMISSION */
		if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType)	||	(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) 	|| \
			(SDIO_HIGH_CAPACITY_SD_CARD == CardType)     	||  \
			(SDIO_MULTIMEDIA_CARD == CardType)				||	(SDIO_HIGH_SPEED_MULTIMEDIA_CARD == CardType)   || \
			(SDIO_HIGH_CAPACITY_MMC_CARD  == CardType) )
		{
			/*!< Send CMD12 STOP_TRANSMISSION */
			SDIO_CmdInitStructure.SDIO_Argument = 0x0;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
			
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
		}
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	count = SD_DATATIMEOUT;
	while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
	{
		*tempbuff = SDIO_ReadData();
		tempbuff++;
		count--;
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

#elif defined(SD_DMA_MODE)
	/* DMA mode */
	while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK))
	{}
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}
#endif

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Allows to write one block starting from a specified address in a card.
	@param  writebuff: pointer to the buffer that contain the data to be transferred.
	@param  WriteAddr: Address from where data are to be read.   
	@param  BlockSize: the SD card Data block size.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_WriteBlock(uint8_t *writebuff, uint64_t WriteAddr, uint16_t BlockSize)
{
	SD_Error errorstatus = SD_OK;
	uint8_t  power = 0, cardstate = 0;
	__IO uint32_t timeout = 0;
	uint32_t cardstatus = 0;

#if defined(SD_POLLING_MODE)
	__IO uint32_t count = 0; 
	uint32_t bytestransferred = 0;
	uint32_t restwords = 0;
	uint32_t *tempbuff = (uint32_t *)writebuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	TotalNumberOfBytes = 0;

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 0;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	SDIO_DMACmd(DISABLE);

	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	if ((CardType == SDIO_HIGH_CAPACITY_SD_CARD) || (CardType == SDIO_HIGH_CAPACITY_MMC_CARD))
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/*!< Set the block size, both on controller and card */
	if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);
		
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
		
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
	}
	else
	{
		errorstatus = SD_INVALID_PARAMETER;
		return(errorstatus);
	}

	/*!< Wait till card is ready for data Added */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (RCA << 16);
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	cardstatus = SDIO_GetResponse(SDIO_RESP1);

	timeout = SD_DATATIMEOUT;

	while (((cardstatus & 0x00000100) == 0) && (timeout > 0))
	{
		timeout--;
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (RCA << 16);
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);
		
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
		cardstatus = SDIO_GetResponse(SDIO_RESP1);
	}

	if (timeout == 0)
	{
		return(SD_ERROR);
	}

#if defined(SD_DMA_MODE)
	StopCondition = 0;
	SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_TXUNDERR | SDIO_IT_STBITERR, ENABLE);
	SDIO_DMACmd(ENABLE);
	SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, BlockSize);
#endif

	/*!< Send CMD24 WRITE_SINGLE_BLOCK */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) WriteAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	TotalNumberOfBytes = BlockSize;

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) power << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	/*!< In case of single data block transfer no need of stop command at all */
#if defined(SD_POLLING_MODE)
	while (!(SDIO->STA & (SDIO_FLAG_DBCKEND | SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)
		{
			if ((TotalNumberOfBytes - bytestransferred) < 32)
			{
				restwords = ((TotalNumberOfBytes - bytestransferred) % 4 == 0) ? ((TotalNumberOfBytes - bytestransferred) / 4) : (( TotalNumberOfBytes -  bytestransferred) / 4 + 1);
				
				for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
				{
					SDIO_WriteData(*tempbuff);
				}
			}
			else
			{
				for (count = 0; count < 8; count++)
				{
					SDIO_WriteData(*(tempbuff + count));
				}
				tempbuff += 8;
				bytestransferred += 32;
			}
		}
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
		errorstatus = SD_TX_UNDERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}

#elif defined(SD_DMA_MODE)
	while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK))
	{}
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}
#endif

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/*!< Wait till the card is in programming state */
	errorstatus = IsCardProgramming(&cardstate);

	while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
	{
		errorstatus = IsCardProgramming(&cardstate);
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Allows to write blocks starting from a specified address in a card.
	@param  WriteAddr: Address from where data are to be read.
	@param  writebuff: pointer to the buffer that contain the data to be transferred.
	@param  BlockSize: the SD card Data block size.
	@param  NumberOfBlocks: number of blocks to be written.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_WriteMultiBlocks(uint8_t *writebuff, uint64_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
	SD_Error errorstatus = SD_OK;
	uint8_t  power = 0, cardstate = 0;
	__IO uint32_t count = 0; 
#if defined(SD_POLLING_MODE)
	uint32_t bytestransferred = 0;
	uint32_t restwords = 0;
	uint32_t *tempbuff = (uint32_t *)writebuff;
#endif

	TransferError = SD_OK;
	TransferEnd = 0;
	TotalNumberOfBytes = 0;

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 0;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Disable;
	SDIO_DataConfig(&SDIO_DataInitStructure);
	SDIO_DMACmd(DISABLE);

	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	if ((CardType == SDIO_HIGH_CAPACITY_SD_CARD) || (CardType == SDIO_HIGH_CAPACITY_MMC_CARD))
	{
		BlockSize = 512;
		WriteAddr /= 512;
	}

	/*!< Set the block size, both on controller and card */
	if ((BlockSize > 0) && (BlockSize <= 2048) && ((BlockSize & (BlockSize - 1)) == 0))
	{
		power = convert_from_bytes_to_power_of_two(BlockSize);
		
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) BlockSize;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
		
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
	}
	else
	{
		errorstatus = SD_INVALID_PARAMETER;
		return(errorstatus);
	}

	/*!< Wait till card is ready for data Added */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (RCA << 16);
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	/*!< Common to all modes */
	if (NumberOfBlocks * BlockSize > SD_MAX_DATA_LENGTH)
	{
		errorstatus = SD_INVALID_PARAMETER;
		return(errorstatus);
	}

	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		/*!< To improve performance */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) (RCA << 16);
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
		
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
		/*!< To improve performance */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)NumberOfBlocks;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT);
		
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
	}

	TotalNumberOfBytes = NumberOfBlocks * BlockSize;

#if defined(SD_DMA_MODE)
	StopCondition = 1;
	SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND | SDIO_IT_TXUNDERR | SDIO_IT_STBITERR, ENABLE);
	SDIO_DMACmd(ENABLE);
	SD_LowLevel_DMA_TxConfig((uint32_t *)writebuff, (NumberOfBlocks * BlockSize));
#endif

	/*!< Send CMD25 WRITE_MULT_BLOCK with argument data address */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)WriteAddr;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK);

	if (SD_OK != errorstatus)
	{
		return(errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = NumberOfBlocks * BlockSize;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) power << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

#if defined(SD_POLLING_MODE)
	while (!(SDIO->STA & (SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DATAEND | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)
		{
			if (!((TotalNumberOfBytes - bytestransferred) < SD_HALFFIFOBYTES))
			{
				for (count = 0; count < SD_HALFFIFO; count++)
				{
					SDIO_WriteData(*(tempbuff + count));
				}
				tempbuff += SD_HALFFIFO;
				bytestransferred += SD_HALFFIFOBYTES;
			}
			else
			{
				restwords = ((TotalNumberOfBytes - bytestransferred) % 4 == 0) ? ((TotalNumberOfBytes - bytestransferred) / 4) :
				((TotalNumberOfBytes - bytestransferred) / 4 + 1);
				
				for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
				{
					SDIO_WriteData(*tempbuff);
				}
			}
		}
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
		errorstatus = SD_TX_UNDERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)
	{
		/*!< In Case Of SD-CARD Send Command STOP_TRANSMISSION */
		if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType)	||	(SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) 	|| \
			(SDIO_HIGH_CAPACITY_SD_CARD == CardType)     	||  \
			(SDIO_MULTIMEDIA_CARD == CardType)				||	(SDIO_HIGH_SPEED_MULTIMEDIA_CARD == CardType)   || \
			(SDIO_HIGH_CAPACITY_MMC_CARD  == CardType) )
		{
			/*!< Send CMD12 STOP_TRANSMISSION */
			SDIO_CmdInitStructure.SDIO_Argument = 0x0;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
			
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
		}
	}

#elif defined(SD_DMA_MODE)
	while ((SD_DMAEndOfTransferStatus() == RESET) && (TransferEnd == 0) && (TransferError == SD_OK))
	{}
	if (TransferError != SD_OK)
	{
		return(TransferError);
	}

#endif

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/*!< Add some delay before checking the Card Status */
	for(count = 0; count < 0xFFFF; count++)
	{}
	/*!< Wait till the card is in programming state */
	errorstatus = IsCardProgramming(&cardstate);

	while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
	{
		errorstatus = IsCardProgramming(&cardstate);
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Gets the cuurent data transfer state.
	@param  None
	@retval SDTransferState: Data Transfer state.
			This value can be: 
			- SD_TRANSFER_OK: No data transfer is acting
			- SD_TRANSFER_BUSY: Data transfer is acting
*/
/**************************************************************************/
SDTransferState SD_GetTransferState(void)
{
	if (SDIO->STA & (SDIO_FLAG_TXACT | SDIO_FLAG_RXACT))
	{
		return(SD_TRANSFER_BUSY);
	}
	else
	{
		return(SD_TRANSFER_OK);
	}
}


/**************************************************************************/
/*! 
	@brief  Aborts an ongoing data transfer.
	@param  None
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_StopTransfer(void)
{
	SD_Error errorstatus = SD_OK;

	/*!< Send CMD12 STOP_TRANSMISSION  */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Allows to erase memory area specified for the given card.
	@param  startaddr: the start address.
	@param  endaddr: the end address.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_Erase(uint64_t startaddr, uint64_t endaddr)
{
	SD_Error errorstatus = SD_OK;
	uint32_t delay = 0;
	__IO uint32_t maxdelay = 0;
	uint8_t cardstate = 0;

	/*!< Check if the card coomnd class supports erase command */
	if (((CSD_Tab[1] >> 20) & SD_CCCC_ERASE) == 0)
	{
		errorstatus = SD_REQUEST_NOT_APPLICABLE;
		return(errorstatus);
	}

	maxdelay = 120000 / ((SDIO->CLKCR & 0xFF) + 2);

	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	if ((CardType == SDIO_HIGH_CAPACITY_SD_CARD) || (CardType == SDIO_HIGH_CAPACITY_MMC_CARD))
	{
		startaddr /= 512;
		endaddr /= 512;
	}

	/*!< According to sd-card spec 1.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
	if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		/*!< Send CMD32 SD_ERASE_GRP_START with argument as addr  */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) startaddr;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_START;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_START);
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
		
		/*!< Send CMD33 SD_ERASE_GRP_END with argument as addr  */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) endaddr;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_ERASE_GRP_END;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		
		errorstatus = CmdResp1Error(SD_CMD_SD_ERASE_GRP_END);
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
	}

	/*!< Send CMD38 ERASE */
	SDIO_CmdInitStructure.SDIO_Argument = 0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ERASE;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_ERASE);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	for (delay = 0; delay < maxdelay; delay++){}

	/*!< Wait till the card is in programming state */
	errorstatus = IsCardProgramming(&cardstate);
	delay = SD_DATATIMEOUT;
	while ((delay > 0) && (errorstatus == SD_OK) && ((SD_CARD_PROGRAMMING == cardstate) || (SD_CARD_RECEIVING == cardstate)))
	{
		errorstatus = IsCardProgramming(&cardstate);
		delay--;
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Returns the current card's status.
	@param  pcardstatus: pointer to the buffer that will contain the SD card 
			status (Card Status register).
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_SendStatus(uint32_t *pcardstatus)
{
	SD_Error errorstatus = SD_OK;

	if (pcardstatus == SDIO_NULL)
	{
		errorstatus = SD_INVALID_PARAMETER;
		return(errorstatus);
	}

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);


	errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	*pcardstatus = SDIO_GetResponse(SDIO_RESP1);

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Returns the current SD card's status.
	@param  psdstatus: pointer to the buffer that will contain the SD card status 
			(SD Status register).
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
SD_Error SD_SendSDStatus(uint32_t *psdstatus)
{
	SD_Error errorstatus = SD_OK;
	uint32_t count = 0;

	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	/*!< Set block size for card if it is not equal to current block size for card. */
	SDIO_CmdInitStructure.SDIO_Argument = 64;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	/*!< CMD55 */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 64;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_64b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	/*!< Send ACMD13 SD_APP_STAUS  with argument as card's RCA.*/
	SDIO_CmdInitStructure.SDIO_Argument = 0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_STAUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);
	errorstatus = CmdResp1Error(SD_CMD_SD_APP_STAUS);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
		{
			for (count = 0; count < 8; count++)
			{
				*(psdstatus + count) = SDIO_ReadData();
			}
			psdstatus += 8;
		}
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}

	count = SD_DATATIMEOUT;
	while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
	{
		*psdstatus = SDIO_ReadData();
		psdstatus++;
		count--;
	}
	/*!< Clear all the static status flags*/
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	psdstatus -= 16;
	for (count = 0; count < 16; count++)
	{
		psdstatus[count] = ((psdstatus[count] & SD_0TO7BITS)   << 24)|((psdstatus[count] & SD_8TO15BITS)  << 8) |
						   ((psdstatus[count] & SD_16TO23BITS) >> 8) |((psdstatus[count] & SD_24TO31BITS) >> 24);
	}
	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Allows to process all the interrupts that are high.
	@param  None
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
inline SD_Error SD_ProcessIRQSrc(void)
{

	if (SDIO_GetITStatus(SDIO_IT_DATAEND) != RESET)
	{
		if (StopCondition == 1)
		{
			TransferError = SD_StopTransfer();
		}
		else
		{
			TransferError = SD_OK;
		}
		SDIO_ClearITPendingBit(SDIO_IT_DATAEND);
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
				  SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
				  SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
		TransferEnd = 1;
		NumberOfBytes = 0;
		return(TransferError);
	}

	if (SDIO_GetITStatus(SDIO_IT_DCRCFAIL) != RESET)
	{
		SDIO_ClearITPendingBit(SDIO_IT_DCRCFAIL);
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
				  SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
				  SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
		NumberOfBytes = 0;
		TransferError = SD_DATA_CRC_FAIL;
		return(SD_DATA_CRC_FAIL);
	}

	if (SDIO_GetITStatus(SDIO_IT_DTIMEOUT) != RESET)
	{
		SDIO_ClearITPendingBit(SDIO_IT_DTIMEOUT);
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
				  SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
				  SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
		NumberOfBytes = 0;
		TransferError = SD_DATA_TIMEOUT;
		return(SD_DATA_TIMEOUT);
	}

	if (SDIO_GetITStatus(SDIO_IT_RXOVERR) != RESET)
	{
		SDIO_ClearITPendingBit(SDIO_IT_RXOVERR);
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
				  SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
				  SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
		NumberOfBytes = 0;
		TransferError = SD_RX_OVERRUN;
		return(SD_RX_OVERRUN);
	}

	if (SDIO_GetITStatus(SDIO_IT_TXUNDERR) != RESET)
	{
		SDIO_ClearITPendingBit(SDIO_IT_TXUNDERR);
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
				  SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
				  SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
		NumberOfBytes = 0;
		TransferError = SD_TX_UNDERRUN;
		return(SD_TX_UNDERRUN);
	}

	if (SDIO_GetITStatus(SDIO_IT_STBITERR) != RESET)
	{
		SDIO_ClearITPendingBit(SDIO_IT_STBITERR);
		SDIO_ITConfig(SDIO_IT_DCRCFAIL | SDIO_IT_DTIMEOUT | SDIO_IT_DATAEND |
				  SDIO_IT_TXFIFOHE | SDIO_IT_RXFIFOHF | SDIO_IT_TXUNDERR |
				  SDIO_IT_RXOVERR | SDIO_IT_STBITERR, DISABLE);
		NumberOfBytes = 0;
		TransferError = SD_START_BIT_ERR;
		return(SD_START_BIT_ERR);
	}

	return(SD_OK);
}


/**************************************************************************/
/*! 
	@brief  Checks for error conditions for CMD0.
	@param  None
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error CmdError(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t timeout;

	timeout = SDIO_CMD0TIMEOUT; /*!< 10000 */

	while ((timeout > 0) && (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) == RESET))
	{
		timeout--;
	}

	if (timeout == 0)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		return(errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Checks for error conditions for R7 response.
	@param  None
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error CmdResp7Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t timeout = SDIO_CMD0TIMEOUT;

	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)) && (timeout > 0))
	{
		timeout--;
		status = SDIO->STA;
	}

	if ((timeout == 0) || (status & SDIO_FLAG_CTIMEOUT))
	{
		/*!< Card is not V2.0 complient or card does not support the set voltage range */
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}

	if (status & SDIO_FLAG_CMDREND)
	{
		/*!< Card is SD V2.0 compliant */
		errorstatus = SD_OK;
		SDIO_ClearFlag(SDIO_FLAG_CMDREND);
		return(errorstatus);
	}
	
	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Checks for error conditions for R1 response.
	@param  cmd: The sent command index.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error CmdResp1Error(uint8_t cmd)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;

	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
	{
		status = SDIO->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return(errorstatus);
	}

	/*!< Check response received is of desired command */
	if (SDIO_GetCommandResponse() != cmd)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return(errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/*!< We have received response, retrieve it for analysis  */
	response_r1 = SDIO_GetResponse(SDIO_RESP1);

	if ((response_r1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
	{
		return(errorstatus);
	}

	if (response_r1 & SD_OCR_ADDR_OUT_OF_RANGE)
	{
		return(SD_ADDR_OUT_OF_RANGE);
	}

	if (response_r1 & SD_OCR_ADDR_MISALIGNED)
	{
		return(SD_ADDR_MISALIGNED);
	}

	if (response_r1 & SD_OCR_BLOCK_LEN_ERR)
	{
		return(SD_BLOCK_LEN_ERR);
	}

	if (response_r1 & SD_OCR_ERASE_SEQ_ERR)
	{
		return(SD_ERASE_SEQ_ERR);
	}

	if (response_r1 & SD_OCR_BAD_ERASE_PARAM)
	{
		return(SD_BAD_ERASE_PARAM);
	}

	if (response_r1 & SD_OCR_WRITE_PROT_VIOLATION)
	{
		return(SD_WRITE_PROT_VIOLATION);
	}

	if (response_r1 & SD_OCR_LOCK_UNLOCK_FAILED)
	{
		return(SD_LOCK_UNLOCK_FAILED);
	}

	if (response_r1 & SD_OCR_COM_CRC_FAILED)
	{
		return(SD_COM_CRC_FAILED);
	}

	if (response_r1 & SD_OCR_ILLEGAL_CMD)
	{
		return(SD_ILLEGAL_CMD);
	}

	if (response_r1 & SD_OCR_CARD_ECC_FAILED)
	{
		return(SD_CARD_ECC_FAILED);
	}

	if (response_r1 & SD_OCR_CC_ERROR)
	{
		return(SD_CC_ERROR);
	}

	if (response_r1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
	{
		return(SD_GENERAL_UNKNOWN_ERROR);
	}

	if (response_r1 & SD_OCR_STREAM_READ_UNDERRUN)
	{
		return(SD_STREAM_READ_UNDERRUN);
	}

	if (response_r1 & SD_OCR_STREAM_WRITE_OVERRUN)
	{
		return(SD_STREAM_WRITE_OVERRUN);
	}

	if (response_r1 & SD_OCR_CID_CSD_OVERWRIETE)
	{
		return(SD_CID_CSD_OVERWRITE);
	}

	if (response_r1 & SD_OCR_WP_ERASE_SKIP)
	{
		return(SD_WP_ERASE_SKIP);
	}

	if (response_r1 & SD_OCR_CARD_ECC_DISABLED)
	{
		return(SD_CARD_ECC_DISABLED);
	}

	if (response_r1 & SD_OCR_ERASE_RESET)
	{
		return(SD_ERASE_RESET);
	}

	if (response_r1 & SD_OCR_AKE_SEQ_ERROR)
	{
		return(SD_AKE_SEQ_ERROR);
	}
	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Checks for error conditions for R3 (OCR) response.
	@param  None
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error CmdResp3Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;

	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
	{
		status = SDIO->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	
	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);
	
	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Checks for error conditions for R2 (CID or CSD) response.
	@param  None
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error CmdResp2Error(void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;

	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
	{
		status = SDIO->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return(errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Checks for error conditions for R6 (RCA) response.
	@param  cmd: The sent command index.
	@param  prca: pointer to the variable that will contain the SD card relative 
			address RCA. 
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
	SD_Error errorstatus = SD_OK;
	uint32_t status;
	uint32_t response_r1;

	status = SDIO->STA;

	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CMDREND)))
	{
		status = SDIO->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return(errorstatus);
	}

	/*!< Check response received is of desired command */
	if (SDIO_GetCommandResponse() != cmd)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return(errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	/*!< We have received response, retrieve it.  */
	response_r1 = SDIO_GetResponse(SDIO_RESP1);

	if (SD_ALLZERO == (response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
	{
		*prca = (uint16_t) (response_r1 >> 16);
		return(errorstatus);
	}

	if (response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR)
	{
		return(SD_GENERAL_UNKNOWN_ERROR);
	}

	if (response_r1 & SD_R6_ILLEGAL_CMD)
	{
		return(SD_ILLEGAL_CMD);
	}

	if (response_r1 & SD_R6_COM_CRC_FAILED)
	{
		return(SD_COM_CRC_FAILED);
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Enables or disables the SDIO wide bus mode.
	@param  NewState: new state of the SDIO wide bus mode.
		This parameter can be: ENABLE or DISABLE.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error SDEnWideBus(FunctionalState NewState)
{
	SD_Error errorstatus = SD_OK;

	uint32_t scr[2] = {0, 0};

	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	/*!< Get SCR Register */
	errorstatus = FindSCR(RCA, scr);
	SCR_Tab[0] = scr[0];
	SCR_Tab[1] = scr[1];

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	/*!< If wide bus operation to be enabled */
	if (NewState == ENABLE)
	{
		/*!< If requested card supports wide bus operation */
		if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
		{
			/*!< Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
			
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			
			/*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_CmdInitStructure.SDIO_Argument = 0x2;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
			
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			return(errorstatus);
		}
		else
		{
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return(errorstatus);
		}
	}   /*!< If wide bus operation to be disabled */
	else
	{
		/*!< If requested card supports 1 bit mode operation */
		if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
		{
			/*!< Send CMD55 APP_CMD with argument as card's RCA.*/
			SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
			
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			
			/*!< Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
			SDIO_CmdInitStructure.SDIO_Argument = 0x00;
			SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
			SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
			SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
			SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
			SDIO_SendCommand(&SDIO_CmdInitStructure);
			
			errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);
			
			if (errorstatus != SD_OK)
			{
				return(errorstatus);
			}
			return(errorstatus);
		}
		else
		{
			errorstatus = SD_REQUEST_NOT_APPLICABLE;
			return(errorstatus);
		}
	}
}

/**************************************************************************/
/*! 
	@brief  Enables or disables the SDIO wide bus mode.
	@param  NewState: new state of the SDIO wide bus mode.
		This parameter can be: ENABLE or DISABLE.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error MMCEnWideBus(FunctionalState NewState)
{
	SD_Error errorstatus = SD_OK;

	if (SDIO_GetResponse(SDIO_RESP1) & SD_CARD_LOCKED)
	{
		errorstatus = SD_LOCK_UNLOCK_FAILED;
		return(errorstatus);
	}

	/*!< If wide bus operation to be enabled */
	if (NewState == ENABLE)
	{
		/* CMD6 */ 
		SDIO_CmdInitStructure.SDIO_Argument = MMC_POWER_REG;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		_delay_ms(5);
		
		errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
		
		/* CMD6 */ 
		SDIO_CmdInitStructure.SDIO_Argument = MMC_HIGHSPEED_REG;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		_delay_ms(5);
		
		errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
		
		/* CMD6 */ 
		SDIO_CmdInitStructure.SDIO_Argument = MMC_4BIT_REG;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		_delay_ms(5);
		
		errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
	}   /*!< If wide bus operation to be disabled */
	else
	{
		errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
		
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
	}

	return(errorstatus);
}

/**************************************************************************/
/*! 
	@brief  Checks if the SD card is in programming state.
	@param  pstatus: pointer to the variable that will contain the SD card state.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error IsCardProgramming(uint8_t *pstatus)
{
	SD_Error errorstatus = SD_OK;
	__IO uint32_t respR1 = 0, status = 0;

	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) RCA << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	status = SDIO->STA;
	while (!(status & (SDIO_FLAG_CCRCFAIL | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT)))
	{
		status = SDIO->STA;
	}

	if (status & SDIO_FLAG_CTIMEOUT)
	{
		errorstatus = SD_CMD_RSP_TIMEOUT;
		SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
		return(errorstatus);
	}
	else if (status & SDIO_FLAG_CCRCFAIL)
	{
		errorstatus = SD_CMD_CRC_FAIL;
		SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
		return(errorstatus);
	}

	status = (uint32_t)SDIO_GetCommandResponse();

	/*!< Check response received is of desired command */
	if (status != SD_CMD_SEND_STATUS)
	{
		errorstatus = SD_ILLEGAL_CMD;
		return(errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);


	/*!< We have received response, retrieve it for analysis  */
	respR1 = SDIO_GetResponse(SDIO_RESP1);

	/*!< Find out card status */
	*pstatus = (uint8_t) ((respR1 >> 9) & 0x0000000F);

	if ((respR1 & SD_OCR_ERRORBITS) == SD_ALLZERO)
	{
		return(errorstatus);
	}

	if (respR1 & SD_OCR_ADDR_OUT_OF_RANGE)
	{
		return(SD_ADDR_OUT_OF_RANGE);
	}

	if (respR1 & SD_OCR_ADDR_MISALIGNED)
	{
		return(SD_ADDR_MISALIGNED);
	}

	if (respR1 & SD_OCR_BLOCK_LEN_ERR)
	{
		return(SD_BLOCK_LEN_ERR);
	}

	if (respR1 & SD_OCR_ERASE_SEQ_ERR)
	{
		return(SD_ERASE_SEQ_ERR);
	}

	if (respR1 & SD_OCR_BAD_ERASE_PARAM)
	{
		return(SD_BAD_ERASE_PARAM);
	}

	if (respR1 & SD_OCR_WRITE_PROT_VIOLATION)
	{
		return(SD_WRITE_PROT_VIOLATION);
	}

	if (respR1 & SD_OCR_LOCK_UNLOCK_FAILED)
	{
		return(SD_LOCK_UNLOCK_FAILED);
	}

	if (respR1 & SD_OCR_COM_CRC_FAILED)
	{
		return(SD_COM_CRC_FAILED);
	}

	if (respR1 & SD_OCR_ILLEGAL_CMD)
	{
		return(SD_ILLEGAL_CMD);
	}

	if (respR1 & SD_OCR_CARD_ECC_FAILED)
	{
		return(SD_CARD_ECC_FAILED);
	}

	if (respR1 & SD_OCR_CC_ERROR)
	{
		return(SD_CC_ERROR);
	}

	if (respR1 & SD_OCR_GENERAL_UNKNOWN_ERROR)
	{
		return(SD_GENERAL_UNKNOWN_ERROR);
	}

	if (respR1 & SD_OCR_STREAM_READ_UNDERRUN)
	{
		return(SD_STREAM_READ_UNDERRUN);
	}

	if (respR1 & SD_OCR_STREAM_WRITE_OVERRUN)
	{
		return(SD_STREAM_WRITE_OVERRUN);
	}

	if (respR1 & SD_OCR_CID_CSD_OVERWRIETE)
	{
		return(SD_CID_CSD_OVERWRITE);
	}

	if (respR1 & SD_OCR_WP_ERASE_SKIP)
	{
		return(SD_WP_ERASE_SKIP);
	}

	if (respR1 & SD_OCR_CARD_ECC_DISABLED)
	{
		return(SD_CARD_ECC_DISABLED);
	}

	if (respR1 & SD_OCR_ERASE_RESET)
	{
		return(SD_ERASE_RESET);
	}

	if (respR1 & SD_OCR_AKE_SEQ_ERROR)
	{
		return(SD_AKE_SEQ_ERROR);
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Find the SD card SCR register value.
	@param  rca: selected card address.
	@param  pscr: pointer to the buffer that will contain the SCR value.
	@retval SD_Error: SD Card Error code.
*/
/**************************************************************************/
static SD_Error FindSCR(uint16_t rca, uint32_t *pscr)
{
	uint32_t index = 0;
	SD_Error errorstatus = SD_OK;
	uint32_t tempscr[2] = {0, 0};

	/*!< Set Block Size To 8 Bytes */
	/*!< Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)8;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	/*!< Send CMD55 APP_CMD with argument as card's RCA */
	SDIO_CmdInitStructure.SDIO_Argument = (uint32_t) rca << 16;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}
	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = 8;
	SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);


	/*!< Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
	SDIO_CmdInitStructure.SDIO_Argument = 0x0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
		{
			*(tempscr + index) = SDIO_ReadData();
			index++;
		}
	}

	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus = SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	*(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);

	*(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);

	return(errorstatus);
}

/**************************************************************************/
/*! 
	@brief  Switch mode High-Speed.
	@retval None
*/
/**************************************************************************/
static SD_Error SD_HighSpeed (void)
{
	SD_Error errorstatus = SD_OK;
	uint32_t scr[2] = {0, 0};
	uint32_t SD_SPEC = 0;
	uint8_t hs[64] = {0};
	uint32_t  count = 0, *tempbuff = (uint32_t *)hs;
	TransferError = SD_OK;
	TransferEnd = 0;
	StopCondition = 0;

	SDIO->DCTRL = 0x0;

	/*!< Get SCR Register */
	errorstatus = FindSCR(RCA, scr);

	if (errorstatus != SD_OK)
	{
		return(errorstatus);
	}

	/* Test the Version supported by the card */ 
	SD_SPEC = (scr[1]  & 0x01000000)||(scr[1]  & 0x02000000);

	if (SD_SPEC != SD_ALLZERO)
	{
		/* Set Block Size for Card */
		SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)64;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
		SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
		SDIO_DataInitStructure.SDIO_DataLength = 64;
		SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_64b ;
		SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
		SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
		SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
		SDIO_DataConfig(&SDIO_DataInitStructure);
		
		/*!< Send CMD6 switch mode */
		SDIO_CmdInitStructure.SDIO_Argument = 0x80FFFF01;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure); 
		errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);
		
		if (errorstatus != SD_OK)
		{
			return(errorstatus);
		}
		while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
		{
			if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
			{
				for (count = 0; count < 8; count++)
				{
					*(tempbuff + count) = SDIO_ReadData();
				}
				tempbuff += 8;
			}	
		}
		
		if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
		{
			SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
			errorstatus = SD_DATA_TIMEOUT;
			return(errorstatus);
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
		{
			SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
			errorstatus = SD_DATA_CRC_FAIL;
			return(errorstatus);
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
		{
			SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
			errorstatus = SD_RX_OVERRUN;
			return(errorstatus);
		}
		else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
		{
			SDIO_ClearFlag(SDIO_FLAG_STBITERR);
			errorstatus = SD_START_BIT_ERR;
			return(errorstatus);
		}
		count = SD_DATATIMEOUT;
		while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
		{
			*tempbuff = SDIO_ReadData();
			tempbuff++;
			count--;
		}
		
		/*!< Clear all the static flags */
		SDIO_ClearFlag(SDIO_STATIC_FLAGS);
		
		/* Test if the switch mode HS is ok */
		if( ((hs[16] & 0x0F) == 0x01) && /* Current mode 0:DS, 1:HS, 2:SDR50, 3:SDR104, 4:DDR50, 5:DDR200 */
		    ((hs[13] & 0x02) == 0x02) )  /* Group1:Support HS mode bit:1 */
		{
			/*!< Configure the SDIO peripheral */
			SDIO_InitStructure.SDIO_ClockDiv = 0; /* Set 0 to SDIO_CK =36MHz MAX */
			SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Falling;	/* This is a work around */
			SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
			SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
			SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
			SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
			SDIO_Init(&SDIO_InitStructure);
			errorstatus=SD_OK;	/* Enter SDHighSpeedMode */
		}
		else
		{
			errorstatus=SD_OK;	/* Still SDNomalMode */
			/*errorstatus=SD_UNSUPPORTED_FEATURE ;*/
		}  
	}
	return(errorstatus);
}

/**************************************************************************/
/*! 
	@brief  Switch mode High-SpeedMode.	
			But may not work properly due to f**kin' errata!!
	@retval None
*/
/**************************************************************************/
static SD_Error MMC_HighSpeed (void)
{
	SD_Error errorstatus = SD_OK;
	
	SDIO->DCTRL = 0x0;

	if(SDCardInfo.SD_csd.SysSpecVersion >= 4){
		/* CMD6 */ 
		SDIO_CmdInitStructure.SDIO_Argument = MMC_POWER_REG;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		_delay_ms(5);
		
		errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
		
		/* CMD6 */ 
		SDIO_CmdInitStructure.SDIO_Argument = MMC_HIGHSPEED_REG;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		_delay_ms(5);
		
		errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
		
		/* CMD6 */ 
		SDIO_CmdInitStructure.SDIO_Argument = MMC_4BIT_REG;
		SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SWITCH;
		SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
		SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
		SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
		SDIO_SendCommand(&SDIO_CmdInitStructure);
		_delay_ms(5);
		
		errorstatus = CmdResp1Error(SD_CMD_HS_SWITCH);
		if (SD_OK != errorstatus)
		{
			return(errorstatus);
		}
			/*!< Configure the SDIO peripheral */
			SDIO_InitStructure.SDIO_ClockDiv = 0; /* Set 0 to SDIO_CK =36MHz MAX */
			SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Falling;	/* This is a work around */
			SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
			SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
			SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
			SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
			SDIO_Init(&SDIO_InitStructure);
			errorstatus=SD_OK;	/* Enter SDHighSpeedMode */
	}

	return(errorstatus);
}

/**************************************************************************/
/*! 
	@brief  Switch mode High-SpeedMode.
	@retval None
*/
/**************************************************************************/
SD_Error SD_EnableHighSpeed(void)
{
	SD_Error errorstatus = SD_OK;

	/*!< MMC Card */
	if ((SDIO_MULTIMEDIA_CARD == CardType) || \
        (SDIO_HIGH_SPEED_MULTIMEDIA_CARD == CardType) || \
		(SDIO_HIGH_CAPACITY_MMC_CARD  == CardType) )
	{
		errorstatus = MMC_HighSpeed();
	}
	else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || \
             (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || \
             (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
	{
		errorstatus = SD_HighSpeed();
	}

	return(errorstatus);
}


/**************************************************************************/
/*! 
	@brief  Converts the number of bytes in power of two and returns the power.
	@param  nBytes: number of bytes.
	@retval None
*/
/**************************************************************************/
static uint8_t convert_from_bytes_to_power_of_two(uint16_t nBytes)
{
	uint8_t count = 0;

	while (nBytes != 1)
	{
		nBytes >>= 1;
		count++;
	}
	return(count);
}

/**************************************************************************/
/*! 
	@brief  Read ExtCSD Register for MMCv4.x.
	@retval None
*/
/**************************************************************************/
SD_Error MMC_ReadExtCsd(MMCEXT_CSD *ExtCsd)
{
	SD_Error errorstatus = SD_OK;
	uint32_t count = 0;
	uint32_t *ExtCsdBuf;
	ExtCsdBuf = (uint32_t *)(&ExtCsd->CsdBuf[0]);

	SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
	SDIO_DataInitStructure.SDIO_DataLength = (uint32_t)512;
	SDIO_DataInitStructure.SDIO_DataBlockSize = (uint32_t) 9 << 4;
	SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
	SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
	SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
	SDIO_DataConfig(&SDIO_DataInitStructure);

	/* CMD8 */ 
	SDIO_CmdInitStructure.SDIO_Argument = 0;
	SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_HS_SEND_EXT_CSD;
	SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
	SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
	SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
	SDIO_SendCommand(&SDIO_CmdInitStructure);

	errorstatus = CmdResp1Error(SD_CMD_HS_SEND_EXT_CSD);

	if (SD_OK != errorstatus)
	{
		return(errorstatus);
	}


	while (!(SDIO->STA &(SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
	{
		if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
		{
			for (count = 0; count < 8; count++)
			{
				*(ExtCsdBuf + count) = SDIO_ReadData();
			}
			ExtCsdBuf += 8;
		}
	}
	if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
		errorstatus = SD_DATA_TIMEOUT;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
		errorstatus = SD_DATA_CRC_FAIL;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
		errorstatus =SD_RX_OVERRUN;
		return(errorstatus);
	}
	else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
	{
		SDIO_ClearFlag(SDIO_FLAG_STBITERR);
		errorstatus = SD_START_BIT_ERR;
		return(errorstatus);
	}
	count = SD_DATATIMEOUT;

	while ((SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET) && (count > 0))
	{
		*ExtCsdBuf = SDIO_ReadData();
		ExtCsdBuf++;
		count--;
	}

	/*!< Clear all the static flags */
	SDIO_ClearFlag(SDIO_STATIC_FLAGS);

	return errorstatus;
}


#if defined(SD_DMA_MODE)
/**************************************************************************/
/*! 
	@brief  Configures the DMA2 Channel4 for SDIO Tx request.
	@param  BufferSRC: pointer to the source buffer
	@param  BufferSize: buffer size
	@retval None
*/
/**************************************************************************/
void SD_LowLevel_DMA_TxConfig(uint32_t *BufferSRC, uint32_t BufferSize)
{
	DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

	/*!< DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Channel4, DISABLE);

	/*!< DMA2 Channel4 Config */
	SDDMA_InitStructure.DMA_MemoryBaseAddr 	= (uint32_t)BufferSRC;
	SDDMA_InitStructure.DMA_DIR 			= DMA_DIR_PeripheralDST;
	SDDMA_InitStructure.DMA_BufferSize 		= BufferSize / 4;
	DMA_Init(DMA2_Channel4, &SDDMA_InitStructure);

	/*!< DMA2 Channel4 enable */
	DMA_Cmd(DMA2_Channel4, ENABLE);  
}


/**************************************************************************/
/*! 
	@brief  Configures the DMA2 Channel4 for SDIO Rx request.
	@param  BufferDST: pointer to the destination buffer
	@param  BufferSize: buffer size
	@retval None
*/
/**************************************************************************/
void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST, uint32_t BufferSize)
{
	DMA_ClearFlag(DMA2_FLAG_TC4 | DMA2_FLAG_TE4 | DMA2_FLAG_HT4 | DMA2_FLAG_GL4);

	/*!< DMA2 Channel4 disable */
	DMA_Cmd(DMA2_Channel4, DISABLE);

	/*!< DMA2 Channel4 Config */
	SDDMA_InitStructure.DMA_MemoryBaseAddr 	= (uint32_t)BufferDST;
	SDDMA_InitStructure.DMA_DIR 			= DMA_DIR_PeripheralSRC;
	SDDMA_InitStructure.DMA_BufferSize 		= BufferSize / 4;
	DMA_Init(DMA2_Channel4, &SDDMA_InitStructure);

	/*!< DMA2 Channel4 enable */
	DMA_Cmd(DMA2_Channel4, ENABLE); 
}


/**************************************************************************/
/*! 
	@brief  Returns the DMA End Of Transfer Status.
	@param  None
	@retval DMA SDIO Channel Status.
*/
/**************************************************************************/
uint32_t SD_DMAEndOfTransferStatus(void)
{
	return (uint32_t)DMA_GetFlagStatus(DMA2_FLAG_TC4);
}
#endif

/**************************************************************************/
/*! 
    @brief	Handles SDIO interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void SDIO_IRQHandler(void)
{
	/* Process All SDIO Interrupt Sources */
	SD_ProcessIRQSrc();
}

/**************************************************************************/
/*! 
    @brief	Get SDCard Status register.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
SD_Error SD_GetCardStatus(SD_CardStatus *cardstatus)
{
	SD_Error errorstatus = SD_OK;
	uint8_t tmp = 0;

	errorstatus = SD_SendSDStatus((uint32_t *)SDSTATUS_Tab);

	if (errorstatus  != SD_OK)
	{
		return(errorstatus);
	}

	/*!< Byte 0 */
	tmp = (uint8_t)((SDSTATUS_Tab[0] & 0xC0) >> 6);
	cardstatus->DAT_BUS_WIDTH = tmp;

	/*!< Byte 0 */
	tmp = (uint8_t)((SDSTATUS_Tab[0] & 0x20) >> 5);
	cardstatus->SECURED_MODE = tmp;

	/*!< Byte 2 */
	tmp = (uint8_t)((SDSTATUS_Tab[2] & 0xFF));
	cardstatus->SD_CARD_TYPE = tmp << 8;

	/*!< Byte 3 */
	tmp = (uint8_t)((SDSTATUS_Tab[3] & 0xFF));
	cardstatus->SD_CARD_TYPE |= tmp;

	/*!< Byte 4 */
	tmp = (uint8_t)(SDSTATUS_Tab[4] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA = tmp << 24;

	/*!< Byte 5 */
	tmp = (uint8_t)(SDSTATUS_Tab[5] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 16;

	/*!< Byte 6 */
	tmp = (uint8_t)(SDSTATUS_Tab[6] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp << 8;

	/*!< Byte 7 */
	tmp = (uint8_t)(SDSTATUS_Tab[7] & 0xFF);
	cardstatus->SIZE_OF_PROTECTED_AREA |= tmp;

	/*!< Byte 8 */
	tmp = (uint8_t)((SDSTATUS_Tab[8] & 0xFF));
	cardstatus->SPEED_CLASS = tmp;

	/*!< Byte 9 */
	tmp = (uint8_t)((SDSTATUS_Tab[9] & 0xFF));
	cardstatus->PERFORMANCE_MOVE = tmp;

	/*!< Byte 10 */
	tmp = (uint8_t)((SDSTATUS_Tab[10] & 0xF0) >> 4);
	cardstatus->AU_SIZE = tmp;

	/*!< Byte 11 */
	tmp = (uint8_t)(SDSTATUS_Tab[11] & 0xFF);
	cardstatus->ERASE_SIZE = tmp << 8;

	/*!< Byte 12 */
	tmp = (uint8_t)(SDSTATUS_Tab[12] & 0xFF);
	cardstatus->ERASE_SIZE |= tmp;

	/*!< Byte 13 */
	tmp = (uint8_t)((SDSTATUS_Tab[13] & 0xFC) >> 2);
	cardstatus->ERASE_TIMEOUT = tmp;

	/*!< Byte 13 */
	tmp = (uint8_t)((SDSTATUS_Tab[13] & 0x3));
	cardstatus->ERASE_OFFSET = tmp;

	return(errorstatus);
}



/**************************************************************************/
/*! 
    Public Functions For FatFs.
*/
/**************************************************************************/
/**************************************************************************/
/*!
  * @brief  Initializes a Drive
  * @param  not used
  * @retval DSTATUS: Operation status
*/
/**************************************************************************/
DSTATUS SD_Initialize()
{
	Stat = STA_NOINIT;
	
	/* Initialize SD Card */
	if(SD_Init() == SD_OK)
	{
		Stat = 0x00;
	}
	else
	{
		/* Anyway,DeInit */
		SD_DeInit();
	}

	return Stat;
}

/**************************************************************************/
/*!
  * @brief  Gets Disk Status
  * @param  not used
  * @retval DSTATUS: Operation status
*/
/**************************************************************************/
DSTATUS SD_Status()
{
	Stat = STA_NOINIT;

	if(SD_GetStatus() == SD_TRANSFER_OK)
	{
		Stat &= ~STA_NOINIT;
	}
	return Stat;
}

/**************************************************************************/
/*!
  * @brief  Reads Sector(s)
  * @param  lun : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
*/
/**************************************************************************/
DRESULT SD_Read(BYTE *buff, LBA_t sector, UINT count)
{
	Status = SD_OK;

	#if defined(SD_DMA_MODE) && (NO_ALIGN4CHK == 0) && !defined(SD_POLLING_MODE)
		if((uintptr_t)buff & 3)	/* Check 4-Byte Alignment */
		{	/* Unaligned Buffer Address Case (Slower) */
			for (unsigned int secNum = 0; secNum < count && Status == SD_OK; secNum++){
				Status =  SD_ReadBlock(dmabuf,
							  (uint64_t)(sector+secNum)*SECTOR_SIZE, 
							  SECTOR_SIZE);
				memcpy(buff+SECTOR_SIZE*secNum, dmabuf, SECTOR_SIZE);
			}
		} else {
			/* Aligned Buffer Address Case (Faster) */
			if(count==1){
				Status = SD_ReadBlock((uint8_t*)(buff), 
									  ((uint64_t)(sector)*SECTOR_SIZE),
									  SECTOR_SIZE);
			}
			else{
				Status = SD_ReadMultiBlocks((uint8_t*)(buff), 
											((uint64_t)(sector)*SECTOR_SIZE),
											SECTOR_SIZE
											,count);
			}
		}

	#else	/* POLLING MODE or NO Aligned Check DMA MODE */
		if(count==1){
			Status = SD_ReadBlock((uint8_t*)(buff), 
								  ((uint64_t)(sector)*SECTOR_SIZE),
								  SECTOR_SIZE);
		}
		else{
			Status = SD_ReadMultiBlocks((uint8_t*)(buff), 
										((uint64_t)(sector)*SECTOR_SIZE),
										SECTOR_SIZE
										,count);
		}
	#endif
	
	if (Status == SD_OK)	return RES_OK;
	else					return RES_ERROR;
}

#if FF_FS_READONLY == 0
/**************************************************************************/
/*!
  * @brief  Writes Sector(s)
  * @param  lun : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
*/
/**************************************************************************/
DRESULT SD_Write(const BYTE *buff, LBA_t sector, UINT count)
{
	Status = SD_OK;

	#if defined(SD_DMA_MODE) && (NO_ALIGN4CHK == 0) && !defined(SD_POLLING_MODE)
		if((uintptr_t)buff & 3)	/* Check 4-Byte Alignment */
		{	/* Unaligned Buffer Address Case (Slower) */
			for (unsigned int secNum = 0; secNum < count && Status == SD_OK; secNum++){
				memcpy(dmabuf, buff+SECTOR_SIZE*secNum, SECTOR_SIZE);
				Status = SD_WriteBlock(dmabuf,
							  (uint64_t)(sector+secNum)*SECTOR_SIZE, 
							  SECTOR_SIZE);
			}
		} else {
			/* Aligned Buffer Address Case (Faster) */
			if(count==1){
				Status = SD_WriteBlock((uint8_t*)(buff), 
									  ((uint64_t)(sector)*SECTOR_SIZE),
									  SECTOR_SIZE);
			}
			else{
				Status = SD_WriteMultiBlocks((uint8_t*)(buff), 
											((uint64_t)(sector)*SECTOR_SIZE),
											SECTOR_SIZE
											,count);
			}
		}

	#else	/* POLLING MODE or NO Aligned Check DMA MODE */
	#if defined(SD_DMA_MODE) && (NO_ALIGN4CHK == 1)
	 #warning "You are about to DMA Tx without unaligned check!"
	#endif
		if(count==1){
			Status = SD_WriteBlock((uint8_t*)(buff), 
								  ((uint64_t)(sector)*SECTOR_SIZE),
								  SECTOR_SIZE);
		}
		else{
			Status = SD_WriteMultiBlocks((uint8_t*)(buff), 
										((uint64_t)(sector)*SECTOR_SIZE),
										SECTOR_SIZE
										,count);
		}
	#endif

		if (Status == SD_OK)	return RES_OK;
		else					return RES_ERROR;
}
#endif /* FF_FS_READONLY */


/**************************************************************************/
/*! 
    @brief Initialize a Drive.
	@param  drv     : Physical drive number (0..).
    @retval DSTATUS :
*/
/**************************************************************************/
DSTATUS disk_initialize(uint8_t drv)
{ 
	Stat = STA_NOINIT;
	
	switch (drv) 
	{
		case SDIO_DRIVE:
		{     
			/* Initialize SD Card */
			Stat = SD_Initialize();
		}
  }

  return Stat;
}

/**************************************************************************/
/*! 
    @brief Return Disk Status.
	@param  drv     : Physical drive number (0..).
    @retval DSTATUS :
*/
/**************************************************************************/
DSTATUS disk_status(uint8_t drv)
{
	Stat = STA_NOINIT;
	
	switch (drv) 
	{
		case SDIO_DRIVE:
		{
			/* Get SD Card Status */
			Stat = SD_Status();
		}
	}
  
	return Stat;
}

/**************************************************************************/
/*! 
    @brief Read Sector(s).
	@param  drv     : Physical drive number (0..).
	@param  *buff   : Data buffer to store read data.
	@param  sector  : Sector address (LBA).
	@param  count   : Number of sectors to read.
    @retval DSTATUS :
*/
/**************************************************************************/
DRESULT disk_read(BYTE pdrv,BYTE *buff,LBA_t sector,UINT count)
{
	switch (pdrv) 
	{
		case SDIO_DRIVE:
		{     
			if(SD_Read(buff, sector, count)==RES_OK) return RES_OK;
			else									 return RES_ERROR;
		}
	}
	return RES_PARERR;
}

/**************************************************************************/
/*! 
    @brief Write Sector(s).
	@param  drv     : Physical drive number (0..).
	@param  *buff   : Data to be written.
	@param  sector  : Sector address (LBA).
	@param  count   : Number of sectors to write.
    @retval DSTATUS :
*/
/**************************************************************************/
#if FF_FS_READONLY == 0
DRESULT disk_write(BYTE pdrv,const BYTE *buff,LBA_t sector,UINT count)
{
	switch (pdrv) 
	{
		case SDIO_DRIVE:
		{     
			if(SD_Write(buff, sector, count)==RES_OK) return RES_OK;
			else									  return RES_ERROR;
		}
	}
	return RES_PARERR;
}
#endif /* FF_FS_READONLY */

/**************************************************************************/
/*! 
    @brief Miscellaneous Functions.
	@param  drv     : Physical drive number (0..).
	@param  ctrl    : Control code.
	@param  *buff   : Buffer to send/receive control data.
    @retval DSTATUS :
*/
/**************************************************************************/
DRESULT disk_ioctl(BYTE pdrv,BYTE cmd,void *buff)
{
	switch (pdrv) 
	{
		case SDIO_DRIVE:
		{
		  /*!< SDCARD */
	      if((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || \
             (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || \
             (SDIO_HIGH_CAPACITY_SD_CARD == CardType)) {
				SD_GetCardStatus(&SDCardStatus);
			}

		  switch (cmd)
		  {
		  	/* Make sure that no pending write process */
			case CTRL_SYNC:
			  /* no synchronization to do since not buffering in this module */
			  return RES_OK;
			  
			/* Get number of sectors on the disk (DWORD) */
			case GET_SECTOR_COUNT:
		#if FF_MAX_SS != FF_MIN_SS
			  *(uint32_t*)buff = SDCardInfo.CardCapacity / SDCardInfo.CardBlockSize;
		#else
			  *(uint32_t*)buff = SDCardInfo.CardCapacity / SECTOR_SIZE;
		#endif
			  return RES_OK;
			  
			/* Get R/W sector size (WORD) (needed at FF_MAX_SS != FF_MIN_SS) */
			case GET_SECTOR_SIZE :
			  *(uint16_t*)buff = SDCardInfo.CardBlockSize;
			  return RES_OK;
			  
			/* Get erase block size in unit of sector (DWORD) */
			case GET_BLOCK_SIZE :
				if ((SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) || (SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)){	/* SDC ver 2.00 */
					if(SDCardStatus.AU_SIZE <= 10) {
						*(uint32_t*)buff = 16UL << (SDCardStatus.AU_SIZE);
					}
					else if((SDCardStatus.AU_SIZE == 11) || (SDCardStatus.AU_SIZE == 12))  {
						*(uint32_t*)buff = 16384UL + 8192UL*(SDCardStatus.AU_SIZE-10);
					}
					else if((SDCardStatus.AU_SIZE == 13) || (SDCardStatus.AU_SIZE == 14))  {
						*(uint32_t*)buff = 32768UL + 16384UL*(SDCardStatus.AU_SIZE-12);
					}
					else { /* SDCardStatus.AU_SIZE == 15 */
						*(uint32_t*)buff = 131072UL; /* 131072(sector) * 512(byte :SDv2 blocksize) = 64MiB */
					}
				} else {					/* SDC ver 1.XX or MMC */
					if (SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) {	/* SDC ver 1.XX */
						*(uint32_t*)buff = (((CSD_Tab[2] & 0x00003F80) >> 7) + 1)  << (((CSD_Tab[3] & 0x00FF0000) >> 22) - 1);
					} else {					/* MMC */
						*(uint32_t*)buff = ((uint16_t)((CSD_Tab[2] & 0x00007C00) >> 10) + 1) * (((CSD_Tab[2] & 0x00000003) << 3) + ((CSD_Tab[2] & 0x000000E0) >> 5) + 1);
					}
				}
			  return RES_OK;
			  
			/* Following command are not used by FatFs module */
			case MMC_GET_TYPE :		/* Get MMC/SDC type (uint8_t) */
				switch (SDCardInfo.CardType)
				{
					case SDIO_STD_CAPACITY_SD_CARD_V1_1:
						*(uint8_t*)buff = CT_SD1;
						break;
					case SDIO_STD_CAPACITY_SD_CARD_V2_0:
						*(uint8_t*)buff = CT_SD2;
						break;
					case SDIO_HIGH_CAPACITY_SD_CARD:
						*(uint8_t*)buff = CT_SD2 | CT_BLOCK;
						break;
					case SDIO_MULTIMEDIA_CARD:
					case SDIO_HIGH_SPEED_MULTIMEDIA_CARD:
						*(uint8_t*)buff = CT_MMC;
						break;
					case SDIO_HIGH_CAPACITY_MMC_CARD:
						*(uint8_t*)buff = CT_MMC | CT_BLOCK;
						break;
					default:
						*(uint8_t*)buff = 0;
				}
				return RES_OK;
				
			case MMC_GET_CSD :		/* Read CSD (16 bytes) */
				/* STM32F1 Manual RM0008 says...
				   The most significant bit of the card status is received first.
                   The SDIO_RESP4 register(CSD_Tab[3]) LSB is always 0b.
				*/
				*((uint32_t *) buff + 0) = __REV(CSD_Tab[0]);
				*((uint32_t *) buff + 1) = __REV(CSD_Tab[1]);
				*((uint32_t *) buff + 2) = __REV(CSD_Tab[2]);
				*((uint32_t *) buff + 3) = __REV(CSD_Tab[3] | 0x00000001);
				return RES_OK;
				
			case MMC_GET_CID :		/* Read CID (16 bytes) */
				/* STM32F1 Manual RM0008 says...
				   The most significant bit of the card status is received first.
                   The SDIO_RESP4 register(CID_Tab[3]) LSB is always 0b.
				*/
				*((uint32_t *) buff + 0) = __REV(CID_Tab[0]);
				*((uint32_t *) buff + 1) = __REV(CID_Tab[1]);
				*((uint32_t *) buff + 2) = __REV(CID_Tab[2]);
				*((uint32_t *) buff + 3) = __REV(CID_Tab[3] | 0x00000001);
				return RES_OK;
				
			case MMC_GET_OCR :		/* Read OCR (4 bytes) */
				*((uint32_t *)buff) = __REV(OCR);
				return RES_OK;
				
			case MMC_GET_SDSTAT :	/* Read SD status (64 bytes) */
				if(SD_SendSDStatus((uint32_t*)buff) == SD_OK){
					return RES_OK;
				} else {
					return RES_ERROR;
				}
				
			case SD_GET_SCR :	/* Read SCR (2 bytes) */
				*((uint32_t *) buff + 0) = __REV(SCR_Tab[1]);
				*((uint32_t *) buff + 1) = __REV(SCR_Tab[0]);
				return RES_OK;
				
			case MMC_GET_EXTCSDREV :	/* Read ExtCSD Rev/Life Datas (4 bytes) */
				*((uint8_t *) buff + 0) = MMC_EXTCSDREV;
				*((uint8_t *) buff + 1) = MMC_EXTCSDLIFE[0];
				*((uint8_t *) buff + 2) = MMC_EXTCSDLIFE[1];
				*((uint8_t *) buff + 3) = MMC_EXTCSDEOL;
				return RES_OK;
			break;
			
			default :
				return RES_PARERR;
			}
		}
	}
	return RES_PARERR;
}


/**************************************************************************/
/*! 
    @brief Device Timer Interrupt Procedure.							@n
		   This function must be called in period of 10ms.
	@param  none
    @retval none
*/
/**************************************************************************/
/* Not used On STM32Primer2 */
#if !defined(USE_STM32PRIMER2)
void disk_timerproc (void)
{
	uint8_t n, s;

	n = Timer1;					/* 100Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;


	s = Stat;
	if (SOCKWP)					/* WP is H (write protected) */
		s |= STA_PROTECT;
	else						/* WP is L (write enabled) */
		s &= ~STA_PROTECT;

	if (!SD_Detect())			/* INS = H (Socket empty) */
		s |= (STA_NODISK | STA_NOINIT);
	else						/* INS = L (Card inserted) */
		s &= ~STA_NODISK;
	Stat = s;
}
#endif 

/* End Of File ---------------------------------------------------------------*/
