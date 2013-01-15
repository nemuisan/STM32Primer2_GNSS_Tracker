/********************************************************************************/
/*!
	@file			gps_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2013.01.07
	@brief          Interface of FatFs For STM32 uC.				@n
					Based on Chan's GPS-Logger Program Thanks!

    @section HISTORY
		2011.03.10	V1.00	Start Here.
		2011.09.07	V2.00	Add RTC Synchronization from GPRMC.
		                    Fixed More Stability.
		2011.12.26	V3.00	Add PA6C Support.
		2012.08.31  V4.00   Imploved FatFs Support Function's Portability.
		2012.09.08	V5.00	Imploved PA6C Support.
		                     (Anti Interference & force 9600bps for 38400bps Firmware).
		2012.12.24  V6.00   Add Gms-g6a Support.
							Imploved Error Handlings.
		2013.01.07  V6.01   Fixed Minor bug.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gps_support.h"
/* check header file version for fool proof */
#if __GPS_SUPPORT_H!= 0x0700
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
/* GT-723F,UP-501,PA6C default baud is 9600,8,n,1*/
#define GPS_UART_PORT	2
#define GPS_UART_BAUD	9600
/* GPS Sentences */
#define GPRMC_COL_VALID	2
#define GPRMC_COL_DATE  9 /*  obsoleted */
#define GPRMC_COL_YMD  	9
#define GPRMC_COL_HMS  	1
#define GPGGA_POS_TYPE	6
#define GPGSV_NUM_VIEW	3

/* Synchronize the file in interval of 90Sec */
#define SYNC_INTERVAL	90

/* To Enable GPGSV Logging, Uncomment this */
/*#define ENABLE_SATELLITE_ID_LOGGING*/

/* Avoid f_close() Foolproof */
#define STBY_STATE 		0
#define LOGGING_STATE 	1

/* Acklowledge Limit in Second */
#define ACK_LIMIT		5

/* Variables -----------------------------------------------------------------*/
FF_RTC ff_rtc;						/* See ff_rtc_if.h */
FATFS Fatfs[_VOLUMES];				/* File system object for each logical drive */
FIL File1;							/* File objects */
DIR Dir;							/* Directory object */
uint8_t Buff[512]; 					/* Working buffer */
volatile UINT Timer;				/* Performance timer (1kHz increment) */
volatile UINT l_stat=STBY_STATE;	/* Avoid f_close() Foolproof */
volatile UINT ack_limit;			/* Acklowledge Limit */

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    MUST called by a timer interrupt-handler every 1ms
*/
/**************************************************************************/
inline void ff_support_timerproc(void)
{
	Timer++;
}

/**************************************************************************/
/*! 
	RealTimeClock function
*/
/**************************************************************************/
uint32_t get_fattime (void)
{
	/* Get local time */
	rtc_gettime(&ff_rtc);

	/* Pack date and time into a DWORD variable */
	return	  ((DWORD)(ff_rtc.year - 1980) << 25)
			| ((DWORD)ff_rtc.month << 21)
			| ((DWORD)ff_rtc.mday << 16)
			| ((DWORD)ff_rtc.hour << 11)
			| ((DWORD)ff_rtc.min << 5)
			| ((DWORD)ff_rtc.sec >> 1);
}

/**************************************************************************/
/*! 
	Acklowledge Limit.
*/
/**************************************************************************/
void ChkAckLimit(void)
{

	if(ack_limit++ > ACK_LIMIT*1000){
		/* Wakeup(For MT333x) */
		cputs("$PMTK000*32\r\n");
		_delay_us(10000);
		ack_limit =0;
	}

}


/**************************************************************************/
/*! 
	File Close Function(gloval).
*/
/**************************************************************************/
void ShutFileClose(void)
{
	if(l_stat==STBY_STATE) return;
	
shutstat:
	if(f_close(&File1)) {goto shutstat;}
}


/**************************************************************************/
/*! 
	0: Power fail occured, >0: Number of bytes received.
*/
/**************************************************************************/
static uint8_t get_line_GPS(void)	
{
	uint16_t c, i = 0;

	for (;;) {
		/* Get a char from the incoming stream */
		c = xfunc_in();
		if (!c || (i == 0 && c != '$')) continue;
		Buff[i++] = c;
		if (c == '\n') break;
		if (i >= sizeof(Buff)) i = 0;
	}
	return i;
}

/**************************************************************************/
/*! 
	Get a column item.
*/
/**************************************************************************/
static uint8_t* gp_col(	/* Returns pointer to the item (returns a NULL when not found) */
	const uint8_t* buf,	/* Pointer to the sentence */
	uint8_t col			/* Column number (0 is the 1st item) */
) 
{
	uint8_t c;


	while (col) {
		do {
			c = *buf++;
			if (c <= ' ') return NULL;
		} while (c != ',');
		col--;
	}
	return (uint8_t*)buf;
}

/**************************************************************************/
/*! 
	Get a Value from sentence.
*/
/**************************************************************************/
static uint8_t gp_val(
	const uint8_t* db
)
{
	uint8_t n, m;


	n = *db++ - '0';
	if (n >= 10) return 0;
	m = *db - '0';
	if (m >= 10) return 0;

	return n * 10 + m;
}

/**************************************************************************/
/*! 
	Compare sentence header string.
	Correspond     :false
	Not Correspond :true
*/
/**************************************************************************/
static uint8_t gp_comp(uint8_t *str1, const char *str2)
{
	uint8_t c;

	do {
		c = *str2++;
	} while (c && c == *str1++);
	return c;
}

/**************************************************************************/
/*! 
    Main GPS Task Routine.
*/
/**************************************************************************/
void gps_task(void)
{
	uint8_t  *p = NULL;
	uint16_t b;
	UINT s;
	volatile uint16_t c_sync=0;
	time_t utc;

	/* If MTK chip baud is 38400bps,then... */
	conio_init(GPS_UART_PORT,38400);
	/* Set to 9600 bps forcely */
	cputs("$PMTK251,9600*17\r\n");
	/* Wait Send String(instead of USART_TXBuffer_FreeSpace()*/
	_delay_ms(200);

	/* Enable Anti Interference Control(for MT333x Only!) */
	cputs("$PMTK286,1*23\r\n");
	/* Wait Send String(instead of USART_TXBuffer_FreeSpace()*/
	_delay_ms(200);

	/* Set UART and redirect to stdio */
	conio_init(GPS_UART_PORT,GPS_UART_BAUD);
	xfunc_out = putch;
	xfunc_in  = xgetc;

	/* Mount FatFs */
	f_mount(0, &Fatfs[0]);

	for (;;) {
startstat:
		/* "Wait for GPS Valid Data Acquisition" State */
		l_stat = STBY_STATE;
		LED_RED_ON();
		/* Wait for valid RMC sentence. */
		do {
			__WFI();
			do{
				b = get_line_GPS();
				/* Got UART String ? */
				if (b) break;
			}while(1);

			/* UART String Received */
			LED_RED_OFF();
			LED_GRN_ON();
	
			/* Get GPRMC & GNRMC Valid Flag Column */ 
			if (!gp_comp(Buff, "$GPRMC") || !gp_comp(Buff, "$GNRMC")){
				/* Skip this execution When not a GPRMC or GNRMC Sentence */
				p = gp_col(Buff,GPRMC_COL_VALID);
				LED_GRN_OFF();
			}
			ack_limit =0;
		} while (!p || *p != 'A'); /* Valid Flag ? */


		/* "GPS Sentence Logging" State */
		l_stat = LOGGING_STATE;
		LED_RED_OFF();
		LED_GRN_OFF();

		/* Synchronize STM32 RTC Module */
		p = gp_col(Buff,GPRMC_COL_HMS);
		/* if (!p) break; */
		rtc.tm_hour = gp_val(p);
		rtc.tm_min  = gp_val(p+2);
		rtc.tm_sec  = gp_val(p+4);
		p = gp_col(Buff,GPRMC_COL_YMD);
		/* if (!p) break; */
		rtc.tm_mday = gp_val(p);
		rtc.tm_mon  = gp_val(p+2) - 1;
		rtc.tm_year = gp_val(p+4) + 100;

		utc = mktime(&rtc);				/* Get UNIX epoch Time */
		utc += 3600 * 9;				/* Create Time in JST*/
		Time_SetUnixTime(utc);			/* Set UNIX epoch Time */

		/* Get a FileName */
		rtc = Time_GetCalendarTime();
		sprintf((char*)Buff,"%02u%02u%02u.log",
						rtc.tm_year % 100,
						rtc.tm_mon+1,
						rtc.tm_mday);

		/* Open or Create logfile */
		if (f_open(&File1, (const char*)Buff, FA_OPEN_ALWAYS | FA_WRITE) ||
		    f_lseek(&File1, File1.fsize)) {goto errstat;}

		/* Logging GPS Data */
		while ((b = get_line_GPS()) > 0)
		{

			if (!gp_comp(Buff,"$GPGGA"))
			{
				p = gp_col(Buff,GPGGA_POS_TYPE);
				if(*p != '0'){ /* 0 is invalid tracking data */
					LED_RED_ON();
					if (f_write(&File1, Buff, b, &s) || b != s) {goto errstat;}
				}
			}

			else if (!gp_comp(Buff,"$GPRMC"))
			{
				p = gp_col(Buff,GPRMC_COL_VALID);
				if(*p == 'A'){
					LED_RED_ON();
					if (f_write(&File1, Buff, b, &s) || b != s) {goto errstat;}
				}
			}

			/* Gms-g6a(MT3333) Support */
			if (!gp_comp(Buff,"$GNGGA"))
			{
				p = gp_col(Buff,GPGGA_POS_TYPE);
				if(*p != '0'){ /* 0 is invalid tracking data */
					LED_RED_ON();
					if (f_write(&File1, Buff, b, &s) || b != s) {goto errstat;}
				}
			}

			/* Gms-g6a(MT3333) Support */
			else if (!gp_comp(Buff,"$GNRMC"))
			{
				p = gp_col(Buff,GPRMC_COL_VALID);
				if(*p == 'A'){
					LED_RED_ON();
					if (f_write(&File1, Buff, b, &s) || b != s) {goto errstat;}
				}
			}

#if defined(ENABLE_SATELLITE_ID_LOGGING)
			else if (!gp_comp(Buff,"$GPGSV"))
			{
				char it;
				p = gp_col(Buff,GPGSV_NUM_VIEW);
				it = gp_val(p);
				if(it >= 5){
					LED_RED_ON();
					if (f_write(&File1, Buff, b, &s) || b != s) {goto errstat;}
				}
			}
#endif
			if(++c_sync > SYNC_INTERVAL) {
				LED_RED_ON();
				c_sync =0;
				if (f_sync(&File1)) {goto errstat;}
			}

			LED_RED_OFF();
			ack_limit =0;
			__WFI();
		}

	}

errstat:
		/* If got error state,close file forcibly */
		LED_RED_ON();
		if(f_close(&File1)) {goto errstat;}

		/* Retry Start */
		goto startstat;

}


/* End Of File ---------------------------------------------------------------*/
