/********************************************************************************/
/*!
	@file			mmc_stm32f4.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2013.02.05
	@brief          MMC Driver For STM32 F-4  Devices					@n
					Based on STM32F2xx_StdPeriph_Driver V1.0.0.			@n
					Based on ChaN's FatFs Sample thanks!

    @section HISTORY
		2011.10.21	V1.00 Start Here.
		2012.01.31	V2.00 Fixed Variable-Handling for DMA.
		2012.03.31  V3.00 Optimized SPI Handlings.
		2012.08.31  V4.00 Updated Support Grater Than 32GB Cards.
		2013.02.05	V5.00 Reduced MAXIMUM SPI Clock.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "mmc_stm32f4.h"

/* Defines -------------------------------------------------------------------*/
/* MMC/SD command */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */

/* Variables -----------------------------------------------------------------*/
static volatile DSTATUS Stat = STA_NOINIT;		/* Disk status */
static volatile unsigned int Timer1, Timer2;	/* 100Hz decrement timer */
static uint8_t CardType;						/* Card type flags */
uint8_t csd[16];								/* For STM32F4 DMA */

#ifdef USE_SPIMMC_DMA
static DMA_InitTypeDef DMA_InitStructure;
const uint8_t rw_dummybyte =0xFF; 				/* For MMC Rx STM32F4 DMA Dummy Buffer */
#endif

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    @brief  Set slow clock (100k-400k).
	@param  None
    @retval : None
*/
/**************************************************************************/
static void FCLK_SLOW(void)
{
	uint32_t tmp;

	tmp = SPIMMC->CR1;
	tmp = ( tmp | SPI_BaudRatePrescaler_256 );
	SPIMMC->CR1 = tmp;
}

/**************************************************************************/
/*! 
    @brief  Set fast clock (depends on the CSD).
	@param  None
    @retval : None
*/
/**************************************************************************/
static void FCLK_FAST(void)
{
	uint32_t tmp;

	tmp = SPIMMC->CR1;
	tmp = ( tmp & ~SPI_BaudRatePrescaler_256 ) | SPIMMC_MAXBAUD;	/* Set to 21MHz */
	SPIMMC->CR1 = tmp;
}

/**************************************************************************/
/*! 
    @brief  Transmit a byte to MMC via SPI.
	@param  uint8_t out
    @retval : uint8_t
*/
/**************************************************************************/
static inline uint8_t xmit_spi(uint8_t out)
{
	/* Loop while DR register in not empty */
	/* while (SPI_I2S_GetFlagStatus(SPIMMC, SPI_I2S_FLAG_TXE) == RESET) { ; } */

	/* Send byte through the SPIMMC peripheral */
	SPIMMC->DR = out;

	/* Wait to receive a byte */
	while((SPIMMC->SR & SPI_I2S_FLAG_RXNE) == RESET) { ; }

	/* Return the byte read from the SPI bus */
	return SPIMMC->DR;
}

/**************************************************************************/
/*! 
    @brief  Receive a byte from MMC via SPI.
	@param  none
    @retval : uint8_t
*/
/**************************************************************************/
static inline uint8_t rcvr_spi(void)
{
	return xmit_spi(0xFF);
}
/* Alternative macro to receive data fast */
#define rcvr_spi_m(dst)  *(dst)=xmit_spi(0xff)

/**************************************************************************/
/*! 
    @brief  Wait for card ready.
	@param  none
    @retval : 1:OK, 0:Timeout
*/
/**************************************************************************/
static int wait_ready (	/* 1:Ready, 0:Timeout */
	unsigned int wt		/* Timeout [ms/10] */
)
{
	Timer2 = wt;
	rcvr_spi();
	do
		if (rcvr_spi() == 0xFF) return 1;
	while (Timer2);

	return 0;
}

/**************************************************************************/
/*! 
    @brief  Deselect the card and release SPI bus.
	@param  none
    @retval : none
*/
/**************************************************************************/
static inline void deselect(void)
{
	CS_HIGH();
	rcvr_spi();
}

/**************************************************************************/
/*! 
    @brief  Select the card and wait for ready.
	@param  none
    @retval : 1:Successful, 0:Timeout
*/
/**************************************************************************/
static inline int select(void)
{
	CS_LOW();
	if (!wait_ready(50)) {
		deselect();
		return 0;
	}
	return 1;
}

/**************************************************************************/
/*! 
    @brief  Power Control.												  @m
			When the target system does not support socket power control, @n
			thereis nothing to do in these functions and chk_power always @n
			returns 1.
	@param  none
    @retval : Socket power state: 0=off, 1=on
*/
/**************************************************************************/
static int power_status(void)
{
	return PWR_ISON() ? 1 : 0;
}

/**************************************************************************/
/*! 
    @brief  Power Control.
	@param  none
    @retval : none
*/
/**************************************************************************/
static void power_on (void)
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable SPI & GPIO clocks */
	SPI_GPIO_CLK(ENABLE);
	SPI_PERIF_CLK(ENABLE);

#ifdef USE_PWRCTRL
	/* Configure I/O for Power FET */
	SPIMMC_PWR_CLK(ENABLE);

	GPIO_InitStructure.GPIO_Pin		= SPIMMC_PIN_PWR;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_OD; /* pull-up resistor at power FET */
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(GPIO_PWR, &GPIO_InitStructure);
	PWR_ON();
#endif

	for (Timer1 = 25; Timer1; );	/* Wait for 250ms */

	/* Configure I/O for Flash Chip select */
	GPIO_InitStructure.GPIO_Pin		= SPIMMC_PIN_CS;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed 	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType 	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_NOPULL;
	GPIO_Init(SPIMMC_PORT_CS, &GPIO_InitStructure);

	/* Deselect the Card: Chip Select high */
	CS_HIGH();


	/* Connect SPI pins to AF */  
	SPI_AF_SET();
	GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType	= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 	= GPIO_PuPd_UP;
        
	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin  	= SPIMMC_PIN_SCK;
	GPIO_Init(SPIMMC_PORT_SCK, &GPIO_InitStructure);

	/* SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin 	= SPIMMC_PIN_MOSI;
	GPIO_Init(SPIMMC_PORT_MOSI, &GPIO_InitStructure);

	/* SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin 	= SPIMMC_PIN_MISO;
	GPIO_Init(SPIMMC_PORT_MISO, &GPIO_InitStructure);


	/* SPI_MMC configuration */
	SPI_InitStructure.SPI_Direction	 		= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode		 		= SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize	 		= SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL		 		= SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA		 		= SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS		 		= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256; /* 84000kHz /256 = 328 < 400kHz */
	SPI_InitStructure.SPI_FirstBit	 		= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial 	= 7;
	SPI_Init(SPIMMC, &SPI_InitStructure);

	SPI_CalculateCRC(SPIMMC, DISABLE);

	/* Enable SPIx  */
	SPIMMC->CR1 |= SPI_CR1_SPE;

	/* drain SPI */
	while (!(SPIMMC->SR & SPI_I2S_FLAG_TXE)) { ; }
	SPIMMC->DR;

#ifdef USE_SPIMMC_DMA
	/* Enable DMA clock */
	RCC_AHB1PeriphClockCmd(SPIMMC_DMA_STREAM_CLOCK, ENABLE);
	
 	/* Disable SPI_DMA_TX/RX */
    SPIMMC_DMA_STREAM_RX->CR &= ~(uint32_t)DMA_SxCR_EN;
    SPIMMC_DMA_STREAM_TX->CR &= ~(uint32_t)DMA_SxCR_EN;
	DMA_DeInit(SPIMMC_DMA_STREAM_RX);
	DMA_DeInit(SPIMMC_DMA_STREAM_TX);
	
	/* shared DMA configuration values */
	DMA_InitStructure.DMA_Channel 				= SPIMMC_DMA_CHANNEL_RX;  
	DMA_InitStructure.DMA_PeripheralBaseAddr 	= (uint32_t)(&(SPIMMC->DR));
	DMA_InitStructure.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize 		= DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode 					= DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority 				= DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode 				= DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold 		= DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst 			= DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst 		= DMA_PeripheralBurst_Single;

#endif
}

/**************************************************************************/
/*! 
    @brief  Power Control.
	@param  none
    @retval : none
*/
/**************************************************************************/
static void power_off (void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	if (!(Stat & STA_NOINIT)) {
		CS_HIGH();
		wait_ready(50);
		deselect();
	}

	/* Disable SPI */
	SPIMMC->CR1 &= (uint16_t)~((uint16_t)SPI_CR1_SPE);
	SPI_I2S_DeInit(SPIMMC);

	/* All SPI-Pins to input with weak internal pull-downs */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	
	/* SPI SCK pin configuration */
	GPIO_InitStructure.GPIO_Pin = SPIMMC_PIN_SCK;
	GPIO_Init(SPIMMC_PORT_SCK, &GPIO_InitStructure);

	/* SPI MOSI pin configuration */
	GPIO_InitStructure.GPIO_Pin =  SPIMMC_PIN_MOSI;
	GPIO_Init(SPIMMC_PORT_MOSI, &GPIO_InitStructure);

	/* SPI MISO pin configuration */
	GPIO_InitStructure.GPIO_Pin =  SPIMMC_PIN_MISO;
	GPIO_Init(SPIMMC_PORT_MISO, &GPIO_InitStructure);

	/* Chip select internal pull-down too */
	GPIO_InitStructure.GPIO_Pin =  SPIMMC_PIN_CS;
	GPIO_Init(SPIMMC_PORT_CS, &GPIO_InitStructure);

	SPI_PERIF_CLK(DISABLE); 

	PWR_OFF();

	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
}

#ifdef USE_SPIMMC_DMA
/**************************************************************************/
/*! 
    @brief  Transmit/Receive Block using DMA.
	@param  -
    @retval : -
*/
/**************************************************************************/
static void stm32_dma_transfer(
	bool receive,		/* false for buff->SPI, true for SPI->buff               */
	const uint8_t *buff,/* receive true  : 512 byte data block to be transmitted
						   receive false : Data buffer to store received data    */
	unsigned int  btr 	/* receive true  : Byte count (must be multiple of 2)
						   receive false : Byte count (must be 512)              */
)
{
	/* static const uint8_t dummy =0xFF; */ /* another method to avoid auto variable asignment */

	/* Wait DMA Transfer result */
	while(SPIMMC_DMA_STREAM_TX->CR & (uint32_t)DMA_SxCR_EN) { ; }

	/* shared DMA configuration values */
	DMA_InitStructure.DMA_BufferSize		 	= btr;

	if(receive){
		/* DMA_SPI_RX configuration SPI_MMC RX ---------------------------------------------*/
		DMA_InitStructure.DMA_Memory0BaseAddr 	= (uint32_t)buff;
		DMA_InitStructure.DMA_DIR 			  	= DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_MemoryInc		  	= DMA_MemoryInc_Enable;
		DMA_Init(SPIMMC_DMA_STREAM_RX, &DMA_InitStructure);

		/* SPI_DMA_TX configuration SPI_MMC TX ---------------------------------------------*/
		DMA_InitStructure.DMA_Memory0BaseAddr 	= (uint32_t)(&rw_dummybyte);
		DMA_InitStructure.DMA_DIR 				= DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_MemoryInc		 	= DMA_MemoryInc_Disable;
		DMA_Init(SPIMMC_DMA_STREAM_TX, &DMA_InitStructure);
	}
	else{
		/* DMA_SPI_RX configuration SPI_MMC RX ---------------------------------------------*/
		DMA_InitStructure.DMA_Memory0BaseAddr 	= (uint32_t)(&rw_dummybyte);
		DMA_InitStructure.DMA_DIR			 	= DMA_DIR_PeripheralToMemory;
		DMA_InitStructure.DMA_MemoryInc		 	= DMA_MemoryInc_Disable;
		DMA_Init(SPIMMC_DMA_STREAM_RX, &DMA_InitStructure);

		/* SPI_DMA_TX configuration SPI_MMC TX ---------------------------------------------*/
		DMA_InitStructure.DMA_Memory0BaseAddr 	= (uint32_t)buff;
		DMA_InitStructure.DMA_DIR		 	 	= DMA_DIR_MemoryToPeripheral;
		DMA_InitStructure.DMA_MemoryInc		 	= DMA_MemoryInc_Enable;
		DMA_Init(SPIMMC_DMA_STREAM_TX, &DMA_InitStructure);
	}

	/* Enable SPI_MMC TX/RX request */
    SPIMMC->CR2 |= (SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx);

	/* Enable SPI_DMA_RX */
    SPIMMC_DMA_STREAM_RX->CR |= (uint32_t)DMA_SxCR_EN;
	/* Enable SPI_DMA_TX */
    SPIMMC_DMA_STREAM_TX->CR |= (uint32_t)DMA_SxCR_EN;

	/* Wait until SPI_DMA_RX Complete */
	while (DMA_GetFlagStatus(SPIMMC_DMA_STREAM_RX,SPIMMC_DMA_FLAG_RX_TC) == RESET) { ; } 
	/* Wait until SPI_DMA_TX Complete */
	/* while (DMA_GetFlagStatus(SPIMMC_DMA_STREAM_TX,SPIMMC_DMA_FLAG_TX_TC) == RESET) { ; } */

 	/* Disable SPI_DMA_TX/RX */
    SPIMMC_DMA_STREAM_RX->CR &= ~(uint32_t)DMA_SxCR_EN;
    SPIMMC_DMA_STREAM_TX->CR &= ~(uint32_t)DMA_SxCR_EN;
	SPIMMC->CR2 &= (uint16_t)~(SPI_I2S_DMAReq_Rx | SPI_I2S_DMAReq_Tx);

	/* Clear DMA_FSMC Transfer/ReceiveComplete Flag */
 	DMA_ClearFlag(SPIMMC_DMA_STREAM_RX,SPIMMC_DMA_FLAG_RX_TC);
 	DMA_ClearFlag(SPIMMC_DMA_STREAM_TX,SPIMMC_DMA_FLAG_TX_TC);

}
#endif /* USE_SPIMMC_DMA */


/**************************************************************************/
/*! 
    @brief  Receive a data packet from MMC.
	@param  none
    @retval : none
*/
/**************************************************************************/
static int rcvr_datablock (
	uint8_t 		*buff,			/* Data buffer to store received data */
	unsigned int 	 btr			/* Byte count (must be multiple of 4) */
)
{
	uint8_t token;


	Timer1 = 20;
	do {							/* Wait for data packet in timeout of 200ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && Timer1);
	if(token != 0xFE) return 0;		/* If not valid data token, retutn with error */

#ifdef USE_SPIMMC_DMA
	stm32_dma_transfer( true, buff, btr );
#else
	do {							/* Receive the data block into buffer */
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
		rcvr_spi_m(buff++);
	} while (btr -= 4);
#endif /* USE_SPIMMC_DMA */
	rcvr_spi();						/* Discard CRC */
	rcvr_spi();

	return 1;						/* Return with success */
}

/**************************************************************************/
/*! 
    @brief  Send a data packet to MMC.
	@param  none
    @retval : none
*/
/**************************************************************************/
#if _READONLY == 0
static int xmit_datablock (
	const uint8_t *buff,	/* 512 byte data block to be transmitted */
	uint8_t       token		/* Data/Stop token */
)
{
	uint8_t resp;
#ifndef USE_SPIMMC_DMA
	uint8_t wc;
#endif

	if (!wait_ready(50)) return 0;

	xmit_spi(token);					/* Xmit data token */
	if (token != 0xFD) {				/* Is data token */
#ifdef USE_SPIMMC_DMA
		stm32_dma_transfer( false, buff, 512 );
#else
		wc = 0;
		do {							/* Xmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (--wc);
#endif /* USE_SPIMMC_DMA */
		xmit_spi(0xFF);					/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();				/* Reveive data response */
		if ((resp & 0x1F) != 0x05)		/* If not accepted, return with error */
			return 0;
	}

	return 1;
}
#endif /* _READONLY */

/**************************************************************************/
/*! 
    @brief  Send a command packet to MMC .
	@param  none
    @retval : none
*/
/**************************************************************************/
static uint8_t send_cmd (		/* Returns R1 resp (bit7==1:Send failed) */
	uint8_t  cmd,				/* Command index */
	uint32_t arg				/* Argument */
)
{
	uint8_t n, res;


	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	deselect();
	if (!select()) return 0xFF;

	/* Send command packet */
	xmit_spi(0x40 | cmd);				/* Start + Command index */
	xmit_spi((uint8_t)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((uint8_t)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((uint8_t)(arg >> 8));		/* Argument[15..8] */
	xmit_spi((uint8_t)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}





/**************************************************************************/
/*! 
    Public Functions
*/
/**************************************************************************/
/**************************************************************************/
/*! 
    @brief  Initialize Disk Drive.
	@param  none
    @retval : none
*/
/**************************************************************************/
DSTATUS disk_initialize (
	uint8_t drv		/* Physical drive number (0) */
)
{
	uint8_t n, cmd, ty, ocr[4];


	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Force socket power on */
	FCLK_SLOW();
	for (n = 10; n; n--) rcvr_spi();	/* 80 dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		Timer1 = 100;						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2? */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while (Timer1 && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if (Timer1 && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			while (Timer1 && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!Timer1 || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
		}
	}
	CardType = ty;
	deselect();

	if (ty) {						/* Initialization succeded */
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */

		FCLK_FAST();

	} else {						/* Initialization failed */
		power_off();
	}

	return Stat;
}

/**************************************************************************/
/*! 
    @brief  Get Disk Status.
	@param  none
    @retval : none
*/
/**************************************************************************/
DSTATUS disk_status (
	uint8_t drv		/* Physical drive number (0) */
)
{
	if (drv) return STA_NOINIT;		/* Supports only single drive */
	return Stat;
}

/**************************************************************************/
/*! 
    @brief  Read Sector(s).
	@param  none
    @retval : none
*/
/**************************************************************************/
DRESULT disk_read (
	uint8_t drv,			/* Physical drive number (0) */
	uint8_t *buff,			/* Pointer to the data buffer to store read data */
	uint32_t sector,		/* Start sector number (LBA) */
	uint8_t count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block read */
		if ((send_cmd(CMD17, sector) == 0)	/* READ_SINGLE_BLOCK */
			&& rcvr_datablock(buff, 512))
			count = 0;
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) break;
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}

/**************************************************************************/
/*! 
    @brief  Write Sector(s).
	@param  none
    @retval : none
*/
/**************************************************************************/
DRESULT disk_write (
	uint8_t  drv,			/* Physical drive number (0) */
	const    uint8_t *buff,	/* Pointer to the data to be written */
	uint32_t sector,		/* Start sector number (LBA) */
	uint8_t  count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & CT_SDC) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	deselect();

	return count ? RES_ERROR : RES_OK;
}

/**************************************************************************/
/*! 
    @brief  Miscellaneous Functions.
	@param  none
    @retval : none
*/
/**************************************************************************/
DRESULT disk_ioctl (
	uint8_t drv,		/* Physical drive number (0) */
	uint8_t ctrl,		/* Control code */
	void    *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	uint8_t n, *ptr = buff;
	/*uint8_t csd[16];*/ /* local variable(CCRAM region) cannot DMA! */
	uint32_t *dp, st, ed, csize;


	if (drv) return RES_PARERR;					/* Check parameter */
	if (Stat & STA_NOINIT) return RES_NOTRDY;	/* Check if drive is ready */

	res = RES_ERROR;

	switch (ctrl) {
	
	case CTRL_POWER :
		switch (ptr[0]) {
		case 0:		/* Sub control code (POWER_OFF) */
			power_off();		/* Power off */
			res = RES_OK;
			break;
		case 1:		/* Sub control code (POWER_GET) */
			ptr[1] = (uint8_t)power_status();
			res = RES_OK;
			break;
		default :
			res = RES_PARERR;
		}
		break;

	case CTRL_SYNC :		/* Wait for end of internal write process of the drive */
		if (select()) {
			deselect();
			res = RES_OK;
		}
		break;

	case GET_SECTOR_COUNT :	/* Get drive capacity in unit of sector (uint32_t) */
		if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				csize = csd[9] + ((uint16_t)csd[8] << 8) + ((uint32_t)(csd[7] & 63) << 16) + 1;
				*(uint32_t*)buff = csize << 10;
			} else {					/* SDC ver 1.XX or MMC ver 3 */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((uint16_t)csd[7] << 2) + ((uint16_t)(csd[6] & 3) << 10) + 1;
				*(uint32_t*)buff = csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	case GET_SECTOR_SIZE :	/* Get sector size in unit of byte (uint16_t) */
		*(uint16_t*)buff = 512;
		res = RES_OK;
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (uint32_t) */
		if (CardType & CT_SD2) {	/* SDC ver 2.00 */
			if (send_cmd(ACMD13, 0) == 0) {	/* Read SD status */
				rcvr_spi();
				if (rcvr_datablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
					*(uint32_t*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDC ver 1.XX or MMC */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
				if (CardType & CT_SD1) {	/* SDC ver 1.XX */
					*(uint32_t*)buff = (((csd[10] & 63) << 1) + ((uint16_t)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMC */
					*(uint32_t*)buff = ((uint16_t)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	case CTRL_ERASE_SECTOR :	/* Erase a block of sectors (used when _USE_ERASE == 1) */
		if (!(CardType & CT_SDC)) break;				/* Check if the card is SDC */
		if (disk_ioctl(drv, MMC_GET_CSD, csd)) break;	/* Get CSD */
		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card */
		dp = buff; st = dp[0]; ed = dp[1];				/* Load sector block */
		if (!(CardType & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && wait_ready(3000))	/* Erase sector block */
			res = RES_OK;	/* FatFs does not check result of this command */
		break;

	/* Following command are not used by FatFs module */

	case MMC_GET_TYPE :		/* Get MMC/SDC type (uint8_t) */
		*ptr = CardType;
		res = RES_OK;
		break;

	case MMC_GET_CSD :		/* Read CSD (16 bytes) */
		if (send_cmd(CMD9, 0) == 0		/* READ_CSD */
			&& rcvr_datablock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_CID :		/* Read CID (16 bytes) */
		if (send_cmd(CMD10, 0) == 0		/* READ_CID */
			&& rcvr_datablock(ptr, 16))
			res = RES_OK;
		break;

	case MMC_GET_OCR :		/* Read OCR (4 bytes) */
		if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
			for (n = 4; n; n--) *ptr++ = rcvr_spi();
			res = RES_OK;
		}
		break;

	case MMC_GET_SDSTAT :	/* Read SD status (64 bytes) */
		if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
			rcvr_spi();
			if (rcvr_datablock(ptr, 64))
				res = RES_OK;
		}
		break;

	default:
		res = RES_PARERR;
	}

	deselect();

	return res;
}

/**************************************************************************/
/*! 
    @brief  Device Timer Interrupt Procedure.					@n
			This function must be called in period of 10ms 
	@param  none 
    @retval : none
*/
/**************************************************************************/
void disk_timerproc(void)
{
	uint8_t n, s;


	n = Timer1;						/* 100Hz decrement timer */
	if (n) Timer1 = --n;
	n = Timer2;
	if (n) Timer2 = --n;

	s = Stat;

	if (SOCKWP)				/* Write protected */
		s |= STA_PROTECT;
	else					/* Write enabled */
		s &= ~STA_PROTECT;
	if (SOCKINS)			/* Card inserted */
		s &= ~STA_NODISK;
	else					/* Socket empty */
		s |= (STA_NODISK | STA_NOINIT);

	Stat = s;
}


/* End Of File ---------------------------------------------------------------*/
