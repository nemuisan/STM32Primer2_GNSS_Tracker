/********************************************************************************/
/*!
	@file			gnss_support.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        20.00
    @date           2023.06.04
	@brief          Interface of FatFs For STM32 uC.				@n
					Based on Chan's GNSS-Logger Program Thanks!

    @section HISTORY
		2011.03.10	V1.00	Start Here.
		2011.09.07	V2.00	Added RTC Synchronization from GPRMC.
		                    Fixed More Stability.
		2011.12.26	V3.00	Added PA6C Support.
		2012.08.31  V4.00   Imploved FatFs Support Function's Portability.
		2012.09.08	V5.00	Imploved PA6C Support.
		                     (Anti Interference & force 9600bps for 38400bps Firmware).
		2012.12.24  V6.00   Added Gms-g6a(MT3333 GLONASS MODE) Support.
		                    Imploved Error Handlings.
		2013.02.20  V7.00   Added Some MT3339/MT3333 Commands.
		2013.04.10  V8.00   Changed UART-Retarget Method.
		2013.10.09	V9.00	Adopted FatFs0.10.
		2015.02.28 V10.00	Buffer alignment set by 4Byte.
		2016.04.15 V11.00	Adopted FatFs0.12.
		2016.05.13 V12.00	Adopted Gms-g9(Titan3) new firmware.
		2017.05.23 V13.00	Adopted FatFs0.13.
		2017.11.01 V14.00	Add and fix more MTK Commands.
		2018.12.12 V15.00	Adopted XA1110 AXN5.x.x new firmware.
		2021.02.05 V16.00	Adopted xprintf update.
		2021.10.31 V17.00	Fixed MTK Commands parameter.
		2022.10.15 V18.00	Fixed filesystem robustness and change filename.
		2023.04.21 V19.00	Fixed cosmetic bugfix.
		2023.06.04 V20.00	Adopted u-blox SAM-M10Q module.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gnss_support.h"
/* check header file version for fool proof */
#if GNSS_SUPPORT_H!= 0x2000
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
/* Many GPS/GNSS modules default baud is 9600bps,8bit,no-parity,1stopbit */
#define GPS_UART_PORT	2
#define GPS_UART_BAUD	9600

/* GPS Sentences */
#define GPRMC_COL_VALID	2
#define GPRMC_COL_DATE  9 /* obsoleted */
#define GPRMC_COL_YMD  	9
#define GPRMC_COL_HMS  	1
#define GPGGA_POS_TYPE	6
#define GPGSV_NUM_VIEW	3
#define GXGSV_NUM_FILT	1

/* Synchronize the file in interval of 90Sec */
#define SYNC_INTERVAL	90

/* To Enable GxGSV Logging, Uncomment this */
//#define ENABLE_SATELLITE_ID_LOGGING

/* Avoid f_close() Foolproof */
#define STBY_STATE 		0
#define LOGGING_STATE 	1

/* Acklowledge Limit in Second */
#define ACK_LIMIT		7

/* MTK Commands */
#define PMTK_TEST							"$PMTK000"
#define PMTK_SET_AIC_MODE					"$PMTK286"
#define PMTK_SET_PERIODIC_MODE				"$PMTK225"
#define PMTK_SET_NMEA_BAUDRATE				"$PMTK251"		/* Unavaileble AXN5.x firmware,use $PGCMD,232 */
#define PMTK_SET_TUNNEL_SCENARIO			"$PMTK257"
#define PMTK_API_SET_SBAS_ENABLED			"$PMTK313"
#define PMTK_API_SET_SBAS_MODE				"$PMTK319"
#define PMTK_API_SET_DGPS_MODE				"$PMTK301"
#define PMTK_API_SET_SUPPORT_QZSS_NMEA		"$PMTK351"
#define PMTK_API_SET_STOP_QZSS				"$PMTK352"
#define PMTK_API_SET_GNSS_SEARCH_MODE		"$PMTK353"		/* Unavaileble AXN5.x firmware,use $PGCMD,229 */
#define PMTK_EASY_ENABLE					"$PMTK869"
#define PMTK_FR_MODE						"$PMTK886"
#define PGCMD_NMEA_BAUDRATE 				"$PGCMD,232"	/* Need for AXN5.x firmware */
#define PGCMD_SATELLITE_SEARCHMODE			"$PGCMD,229"	/* Need for AXN5.x firmware */
/* ATTENSION FOR AXN5.x.x Functions */
/* YOU MUST RE-POWER AFTER THIS COMMAND */
/* Flash to changed baudrate */
/*#define MTK_FLASH_BAUDRATE*/
/* Flash to changed satellite searchmode */
/*#define MTK_FLASH_SATELLITE*/


/* STM32 SDIO+DMA Transfer MUST need 4byte alignmanet */
/* and MUST need 4byte-packed alignment */
#define ATTR_MEM	__attribute__ ((aligned (4)))

/* Variables -----------------------------------------------------------------*/
FF_RTC ff_rtc;						/* See ff_rtc_if.h */
FATFS FatFs[FF_VOLUMES];			/* File system object for each logical drive */
FIL File1;							/* File objects */
DIR Dir;							/* Directory object */
uint8_t Buff[1024] ATTR_MEM; 		/* Working buffer(MUST be 4byte aligned) */
volatile UINT Timer;				/* Performance timer (1kHz increment) */
volatile UINT l_stat = STBY_STATE;	/* Avoid f_close() Foolproof */
volatile UINT cmd_mode = CMD_NOMAL;	/* Detect command mode (MTK or Generic Nomal) */
volatile UINT ack_limit;			/* Acklowledge Limit */

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/
static void xSend_MTKCmd(const char* cmdstr,const char* datastr);

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
		if(cmd_mode == CMD_MTK){ /* GNSS MTK Mode */
			/* Wakeup(For MT333x) */
			xSend_MTKCmd(PMTK_TEST,"");
		}
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
	Calculate XOR Checksum.
*/
/**************************************************************************/
static uint8_t get_Checksum(char* cmdstr,int bytes)	
{
	uint8_t chksum = 0;
	char* pstr;

	if(*cmdstr != '$'){
		return chksum;
	}
	else{
		pstr = cmdstr+1;
		for(;;){
			if(((*pstr)=='*')|| !(--bytes)) break;
			chksum = chksum^(*pstr++);
		}
	}
	return chksum;
}

/**************************************************************************/
/*! 
	Send MTK Command with Checksum Calculation.
*/
/**************************************************************************/
static void xSend_MTKCmd(const char* cmdstr,const char* datastr)	
{
	char strcmd[128];

	if(*datastr==0)
		xsprintf(strcmd,"%s*",cmdstr);
	else
		xsprintf(strcmd,"%s,%s*",cmdstr,datastr);
	xprintf("%s%X\r\n",strcmd,get_Checksum(strcmd,sizeof(strcmd)));
	while(!(WaitTxBuffer()));
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
		c = xfunc_input();
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

	/* Retarget xprintf() */
	xdev_out(putch);
	xdev_in(getch);

	if(cmd_mode == CMD_MTK){ /* GNSS MTK Mode */
		LED_RED_ON();
		LED_GRN_ON();
		/* If MTK chip baud is 38400bps or 115200bps,then... */
		conio_init(GPS_UART_PORT,38400);
		/* Set to 9600 bps forcely in 38400bps */
		xSend_MTKCmd(PMTK_SET_NMEA_BAUDRATE,"9600");
		_delay_ms(100);		/* Need Break Time */
		
		conio_init(GPS_UART_PORT,115200);
		/* Set to 9600 bps forcely in 115200bps */
		xSend_MTKCmd(PMTK_SET_NMEA_BAUDRATE,"9600");
		_delay_ms(100);		/* Need Break Time */

	#if defined(MTK_FLASH_BAUDRATE)
		/* Set to 9600 bps forcely in 115200bps for XA1110 */
		/* 1:9600bps,4:38400bps,6:115200bps(default) */
		/* Need Re-Power module */
		xSend_MTKCmd(PGCMD_NMEA_BAUDRATE ,"1");
		_delay_ms(100);		/* Need Break Time */
	#endif

		/* Set UART to 9600bps and redirect to stdio */
		conio_init(GPS_UART_PORT,GPS_UART_BAUD);

		/* Enable WAAS/SBAS */
		xSend_MTKCmd(PMTK_API_SET_SBAS_ENABLED,"1");
		xSend_MTKCmd(PMTK_API_SET_SBAS_MODE,"1");
		xSend_MTKCmd(PMTK_API_SET_DGPS_MODE,"2");

		/*----- For MT3339/MT3333 Specific Commands(avobe AXN3.8) -----*/
		/* Disable AlwaysLocate & Periodic Power Mode */
		xSend_MTKCmd(PMTK_SET_PERIODIC_MODE,"0");
		/* GNSS FAST TTFF:0 HIGH ACCURACY:1 */
		xSend_MTKCmd(PMTK_SET_TUNNEL_SCENARIO,"0");
		/* Enable Anti Interference Control ON:1 OFF:0 */
		xSend_MTKCmd(PMTK_SET_AIC_MODE,"0");
		/* Enable EASY */
		xSend_MTKCmd(PMTK_EASY_ENABLE,"1,1");
		/* Set 1 to Fitness(<5m/s) mode */
		xSend_MTKCmd(PMTK_FR_MODE,"1");
		/* Enable GPS/QZS/GLONASS/GALILEO */
		xSend_MTKCmd(PMTK_API_SET_GNSS_SEARCH_MODE,"1,1,1,0,0");

	#if defined(MTK_FLASH_SATELLITE)
		/* Enable GPS/QZS/GLONASS/GALILEO */
		/* Enable PMTK353 */
		/* Need Re-Power module */
		xSend_MTKCmd(PGCMD_SATELLITE_SEARCHMODE,"1,1,0,1,0");
	#endif
	
		LED_RED_OFF();
		LED_GRN_OFF();
	}
	else{ /* GNSS Nomal Mode */
		/* Set UART to 9600bps and redirect to stdio */
		conio_init(GPS_UART_PORT,GPS_UART_BAUD);
	}
	
	
	/* Mount Fatfs Drive */
	f_mount(&FatFs[0], "", 0);

	/* Flush UART RxBuffer for Safe */
	Flush_RXBuffer();

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
			if (!gp_comp(Buff,"$GPRMC") || !gp_comp(Buff,"$GQRMC") || !gp_comp(Buff,"$GNRMC"))
			{
				/* Skip this execution When not a GxRMC Sentence */
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
		xsprintf((char*)Buff,"%02u%02u%02u.log",
						rtc.tm_year % 100,
						rtc.tm_mon+1,
						rtc.tm_mday);

		/* Open or Create logfile */
		if (f_open(&File1, (const char*)Buff, FA_OPEN_ALWAYS | FA_WRITE) ||
		    f_lseek(&File1, File1.obj.objsize)) {goto errstat;}

		/* Sync Anyway */
		if (f_sync(&File1)) {goto errstat;}

		/* Logging GPS Data */
		while ((b = get_line_GPS()) > 0)
		{
			/* Get GxGGA Valid Flag Column */ 
			if (!gp_comp(Buff,"$GPGGA") || !gp_comp(Buff,"$GQGGA")|| !gp_comp(Buff,"$GNGGA"))
			{
				p = gp_col(Buff,GPGGA_POS_TYPE);
				if(*p != '0'){ /* 0 is invalid tracking data */
					LED_RED_ON();
					if (f_write(&File1, Buff, b, &s) || b != s) {goto errstat;}
				}
			}

			/* Get GxRMC Valid Flag Column */ 
			else if (!gp_comp(Buff,"$GPRMC") || !gp_comp(Buff,"$GQRMC") || !gp_comp(Buff,"$GNRMC"))
			{
				p = gp_col(Buff,GPRMC_COL_VALID);
				if(*p == 'A'){ /* A is valid tracking data */
					LED_RED_ON();
					if (f_write(&File1, Buff, b, &s) || b != s) {goto errstat;}
				}
			}

#if defined(ENABLE_SATELLITE_ID_LOGGING)
			/* Get GPGSV & GQGSV & GLGSV & GAGSV & GBGSV Valid Flag Column */
			/* QZGSV and BDGSV was "old" format,so didn't capture */
			else if (!gp_comp(Buff,"$GPGSV") || !gp_comp(Buff,"$GQGSV") || !gp_comp(Buff,"$GLGSV") || \
                     !gp_comp(Buff,"$GAGSV") || !gp_comp(Buff,"$GBGSV"))
			{
				char it;
				p = gp_col(Buff,GPGSV_NUM_VIEW);
				it = gp_val(p);
				if(it >= GXGSV_NUM_FILT){ /* Total sattelites in view */
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
