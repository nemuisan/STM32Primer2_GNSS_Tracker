/********************************************************************************/
/*!
	@file			touch_if.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2019.10.01
	@brief          Interface of Touch Panel Hardware Depend Layer				 @n
					Based On "ThaiEasyElec.com BlueScreen" Touch Driver Thanks ! @n

    @section HISTORY
		2010.04.14	V1.00	Start Here.
		2010.12.31	V2.00	Fixed Bit Definitions.
		2011.03.10	V3.00	C++ Ready.
		2011.05.30	V4.00	Separate from Device Depend Section.
		2013.04.04	V5.00	Added STMPE811 Device Handlings.
		2013.11.30	V6.00	Added STM32F429I-Discovery support.
		2016.06.01	V7.00	Added FT6x06 Device Handlings.
		2016.07.03	V8.00	Added SWAP or Reverse XY exec.
		2019.10.01	V9.00	Fixed some variable inclusion.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "touch_if.h"
/* check header file version for fool proof */
#if __TOUCH_IF_H != 0x0900
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#define CENTER_X1 	(((MAX_X-1)/2)-(8 *CurrentAnkDat->X_Size))
#define CENTER_X2 	(((MAX_X-1)/2)-(12*CurrentAnkDat->X_Size))
#define CENTER_Y1	(((MAX_Y-1)/2)-(1 *CurrentAnkDat->Y_Size))
#define CENTER_Y2	(((MAX_Y-1)/2)-(0 *CurrentAnkDat->Y_Size))

/* Variables -----------------------------------------------------------------*/
/* Raw Value */
Touch_t  TouchVal 	= {0xFFFF,0xFFFF};
Touch_t* pTouch 	= &TouchVal;

/* Calibrated Value */
Touch_t  PosVal 	= {0xFFFF,0xFFFF};
Touch_t* pPos 		= &PosVal;

/* Touch State Value */
uint8_t scaned_tc	= 0;
uint8_t hold_okes_tc= 0;
uint8_t last_pen 	= 0;
uint8_t tc_stat 	= TC_STAT_NONE;

/* Touch Calculate Value */
const long chalfx 	= MAX_X/2;
const long chalfy 	= MAX_Y/2;

/* Calibrated Position Value Relations */
long ccx, cm1x, cm2x;
long ccy, cm1y, cm2y;

/* Constants -----------------------------------------------------------------*/
/* MAXIMUM elements are upto 5. */
#if   (TC_TP_NUM == 5)	/* 25 points */
 const int tc_tpx[TC_TP_NUM] = {TC_TPX(0),TC_TPX(1),TC_TPX(2),TC_TPX(3),TC_TPX(4)};
 const int tc_tpy[TC_TP_NUM] = {TC_TPY(0),TC_TPY(1),TC_TPY(2),TC_TPY(3),TC_TPY(4)};
#elif (TC_TP_NUM == 4)	/* 16 points */
 const int tc_tpx[TC_TP_NUM] = {TC_TPX(0),TC_TPX(1),TC_TPX(2),TC_TPX(3)};
 const int tc_tpy[TC_TP_NUM] = {TC_TPY(0),TC_TPY(1),TC_TPY(2),TC_TPY(3)};
#elif (TC_TP_NUM == 3)	/* 9 points */
 const int tc_tpx[TC_TP_NUM] = {TC_TPX(0),TC_TPX(1),TC_TPX(2)};
 const int tc_tpy[TC_TP_NUM] = {TC_TPY(0),TC_TPY(1),TC_TPY(2)};
#elif (TC_TP_NUM == 2)	/* 4 points */
 const int tc_tpx[TC_TP_NUM] = {TC_TPX(0),TC_TPX(1)};
 const int tc_tpy[TC_TP_NUM] = {TC_TPY(0),TC_TPY(1)};
#endif

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
#if	defined(USE_TOUCH_CTRL)
/**************************************************************************/
/*! 
	Milliseconds wait routine.
*/
/**************************************************************************/
#ifndef __SYSTICK_H
inline static void _delay_ms (uint32_t ms)
{
	ms += ticktime;
	while (ticktime < ms);
}
#endif

#if defined(USE_STMPE811_I2C) || defined(USE_ADS7843)
/**************************************************************************/
/*! 
	Calculate Relative Position X.
*/
/**************************************************************************/
static uint16_t cal_posx(uint16_t x)
{
	long buf;
	buf = x - ccx;
	buf = buf*cm1x;
	buf = buf/cm2x;
	buf = buf + chalfx;
	if(buf<0) buf =0;
	return (buf);
}

/**************************************************************************/
/*! 
	Calculate Relative Position Y.
*/
/**************************************************************************/
static uint16_t cal_posy(uint16_t y)
{
	long buf;
	buf = y - ccy;
	buf = buf*cm1y;
	buf = buf/cm2y;
	buf = buf + chalfy;
	if(buf<0) buf =0;
	return (buf);
}
#endif
#endif

#if defined(USE_ADS7843)
/**************************************************************************/
/*! 
	TouchScreen Low Level.
	Downed  :	return not 0.
	Released:	return 0.
*/
/**************************************************************************/
#define TC_PenINT TC_PenDown
inline uint8_t TC_PenDown(void) 
{
	return (!PENIRQ_CHK()); 
}

/**************************************************************************/
/*! 
	TouchScreen Low Level.
*/
/**************************************************************************/
inline void TC_ReadRaw() 
{	
	uint8_t RTemp[2][2];
	ASSART_CS_TOUCH();

	TouchSpi_Xmit(X_DIFFER_12);
	RTemp[0][0] = TouchSpi_Rcvr();
	RTemp[0][1] = TouchSpi_Rcvr();

	TouchSpi_Xmit(Y_DIFFER_12);
	RTemp[1][0] = TouchSpi_Rcvr();
	RTemp[1][1] = TouchSpi_Rcvr();

	/* -HHHHHHH LLLLL--- */
	/* ->                */
	/* ----HHHH HHHLLLLL */
	pTouch->X_Axis = ((uint16_t)(RTemp[0][0]<<5)) | (RTemp[0][1] >>3); 
	pTouch->Y_Axis = ((uint16_t)(RTemp[1][0]<<5)) | (RTemp[1][1] >>3); 

	NEGATE_CS_TOUCH();
}

/**************************************************************************/
/*! 
	TouchScreen Low Level.
*/
/**************************************************************************/
inline void TC_ScanPen(void)
{
	uint16_t tc_x_buf,tc_y_buf;
	uint16_t tc_x_max,tc_y_max;
	uint16_t tc_x_min,tc_y_min;


	if (TC_PenDown())	/* Downed Pen */
	{
		TC_ReadRaw();
		tc_x_buf = pTouch->X_Axis;
		tc_y_buf = pTouch->Y_Axis;

		tc_x_max = pTouch->X_Axis;
		tc_y_max = pTouch->Y_Axis;
		tc_x_min = pTouch->X_Axis;
		tc_y_min = pTouch->Y_Axis;

		TC_ReadRaw();
		tc_x_buf += pTouch->X_Axis;
		tc_y_buf += pTouch->Y_Axis;

		if (pTouch->X_Axis > tc_x_max)
			tc_x_max = pTouch->X_Axis;
		if (pTouch->X_Axis < tc_x_min)
			tc_x_min = pTouch->X_Axis;
		if (pTouch->Y_Axis > tc_y_max)
			tc_y_max = pTouch->Y_Axis;
		if (pTouch->Y_Axis < tc_y_min)
			tc_y_min = pTouch->Y_Axis;

		TC_ReadRaw();
		tc_x_buf += pTouch->X_Axis;
		tc_y_buf += pTouch->Y_Axis;

		if (pTouch->X_Axis > tc_x_max)
			tc_x_max = pTouch->X_Axis;
		if (pTouch->X_Axis < tc_x_min)
			tc_x_min = pTouch->X_Axis;
		if (pTouch->Y_Axis > tc_y_max)
			tc_y_max = pTouch->Y_Axis;
		if (pTouch->Y_Axis < tc_y_min)
			tc_y_min = pTouch->Y_Axis;

		TC_ReadRaw();
		tc_x_buf += pTouch->X_Axis;
		tc_y_buf += pTouch->Y_Axis;

		if (pTouch->X_Axis > tc_x_max)
			tc_x_max = pTouch->X_Axis;
		if (pTouch->X_Axis < tc_x_min)
			tc_x_min = pTouch->X_Axis;
		if (pTouch->Y_Axis > tc_y_max)
			tc_y_max = pTouch->Y_Axis;
		if (pTouch->Y_Axis < tc_y_min)
			tc_y_min = pTouch->Y_Axis;

		tc_x_buf -= tc_x_max;
		tc_x_buf -= tc_x_min;
		tc_y_buf -= tc_y_max;
		tc_y_buf -= tc_y_min;

		tc_x_buf = tc_x_buf >> 1;
		tc_y_buf = tc_y_buf >> 1;

		if (TC_PenDown())	/* Downed Pen */
		{
			pPos->X_Axis = cal_posx(tc_x_buf);
			pPos->Y_Axis = cal_posy(tc_y_buf);
			
			if (last_pen)  /* last_pen:1->1 */
			{
				tc_stat = TC_STAT_HOLD;
			}
			else		   /* last_pen:0->1 */
			{
				tc_stat = TC_STAT_DOWN;
			}

			last_pen = 1;
		}
	}
	else
	{
		if (last_pen)  /* last_pen:1->0 */
		{
			tc_stat = TC_STAT_UP;
		}
		else		   /* last_pen:0->0 */
		{
			tc_stat = TC_STAT_NONE;
		}

		last_pen = 0;
	}
}

/**************************************************************************/
/*! 
	Calibrate and Init TouchScreen.
*/
/**************************************************************************/
void TC_CalibScreen_If(void)
{
	int buf;

	int x[TC_TP_NUM*TC_TP_NUM];
	int y[TC_TP_NUM*TC_TP_NUM];

    long sum_xx = 0;
    long sum_yy = 0;
    long sum_fx = 0;
    long sum_fy = 0;
    long sum_fxx = 0;
    long sum_fyy = 0;

	float cx;
	float mx;
	float cy;
	float my;

    float mx_new;
    float my_new;

	float buf_mod1;

	long fx[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];
	long fy[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];
	long xx[TC_TP_NUM*TC_TP_NUM];
	long yy[TC_TP_NUM*TC_TP_NUM];
	long fxx[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];
	long fyy[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];

	uint8_t i,j,k;
	uint8_t good_press;

	char unkbuf[16];
	
	/* Read Thru at first */
	TC_ReadRaw();
	
	if(CHK_CALIBED()) /* check for valid touch screen calibration data */
	{

		Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);
		Display_Puts_If_Ex(5,5,(uint8_t*)"Touch The Screen To Calibrate",OPAQUE,COL_RED,COL_WHITE);

		k = 0;

		for (i=0;i<3;i++)
		{
			sprintf(unkbuf,"With in %c Sec",'3' - i);
			Display_Puts_If_Ex(5,5+(1*CurrentAnkDat->Y_Size),(uint8_t*)unkbuf,OPAQUE,COL_RED,COL_WHITE);

			for (j=0;j<10;j++)
			{
				if (TC_PenDown())
				{
					k = 1;
					break;
				}
				_delay_ms(100);
			}
			if (k)
				break;
		}

		if (k == 0) /* use old data */
		{
			TC_Restore_Calivalue();
			return;
		}
	}

	Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);

	Display_Puts_If_Ex(5,5,(uint8_t*)"Enter Calibration Mode!",OPAQUE,COL_RED,COL_WHITE);
	_delay_ms(500);

	Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);

	for (i=0; i<TC_TP_NUM; i++)
	{
		for (j=0; j<TC_TP_NUM; j++)
		{
			_delay_ms(100);
			
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y1,(uint8_t*)"Touch Blue Square",TRANSPARENT,COL_RED,COL_WHITE);
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y2,(uint8_t*)"      Twice      ",TRANSPARENT,COL_RED,COL_WHITE);

			for (k=0; k<CALIB_TEST_TIME; k++)
			{
				Display_FillRect_If(tc_tpx[i]-2,tc_tpx[i]+2,tc_tpy[j]-2,tc_tpy[j]+2,COL_BLUE);
				good_press = 0;
				while (!good_press)
				{
					while (!TC_PenDown());
					TC_ReadRaw();
					_delay_ms(40);
					if (TC_PenDown())
					{
						TC_ReadRaw();
						Display_FillRect_If(tc_tpx[i]-2,tc_tpx[i]+2,tc_tpy[j]-2,tc_tpy[j]+2,COL_YELLOW);
						/* _delay_ms(50); */ /* <- Debug only */
						while (TC_PenDown());
						good_press = 1;
					}
				}
				fx[i*TC_TP_NUM+j][k] = pTouch->X_Axis;
				fy[i*TC_TP_NUM+j][k] = pTouch->Y_Axis;
				_delay_ms(200);
			}

			Display_Puts_If_Ex(CENTER_X1,CENTER_Y1,(uint8_t*)"                  ",OPAQUE,COL_WHITE,COL_WHITE);
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y2,(uint8_t*)"                  ",OPAQUE,COL_WHITE,COL_WHITE);
			Display_FillRect_If(tc_tpx[i]-2,tc_tpx[i]+2,tc_tpy[j]-2,tc_tpy[j]+2,COL_WHITE);
			_delay_ms(100);
			
		}
	}

	for (i=0;i<TC_TP_NUM*TC_TP_NUM;i++)
	{
        x[i] = tc_tpx[i/TC_TP_NUM] - chalfx;
        y[i] = tc_tpy[i%TC_TP_NUM] - chalfy;
        xx[i] = x[i] * x[i];
        yy[i] = y[i] * y[i];
		
		for (j=0;j<CALIB_TEST_TIME;j++)
		{
			fxx[i][j] = fx[i][j] * x[i];
			fyy[i][j] = fy[i][j] * y[i];
		}
	}

    for (i = 0; i < TC_TP_NUM*TC_TP_NUM; i++)
    {
        sum_xx += (xx[i] + xx[i]);
        sum_yy += (yy[i] + yy[i]);
		
		for (j=0;j<CALIB_TEST_TIME;j++)
		{
		    sum_fx += fx[i][j];
			sum_fy += fy[i][j];
			sum_fxx += fxx[i][j];
			sum_fyy += fyy[i][j];
		}
    }

    cx = ((float)sum_fx)  / ((float)(TC_TP_NUM * TC_TP_NUM * CALIB_TEST_TIME));
    mx = ((float)sum_fxx) / ((float)sum_xx);
    cy = ((float)sum_fy)  / ((float)(TC_TP_NUM * TC_TP_NUM * CALIB_TEST_TIME));
    my = ((float)sum_fyy) / ((float)sum_yy);

	ccx = (long)cx;
	ccy = (long)cy;

    mx_new = mx;
    my_new = my;

	buf_mod1 = mx_new - (long)mx_new;
	if (buf_mod1 < 0)
		buf_mod1 = -buf_mod1;
	buf = 1;

    while ((buf_mod1 > 0.05) && (buf_mod1 < 0.95))
    {
        buf++;
        mx_new = mx*((float) buf);
		buf_mod1 = mx_new - (long)mx_new;
		if (buf_mod1 < 0)
			buf_mod1 = -buf_mod1;
    }

	cm1x = buf;
    cm2x = (long)(mx * buf);

	buf_mod1 = my_new - (long)my_new;
	if (buf_mod1 < 0)
		buf_mod1 = -buf_mod1;
    buf = 1;

    while ((buf_mod1 > 0.05) && (buf_mod1 < 0.95))
    {
        buf++;
        my_new = my*((float)buf);
		buf_mod1 = my_new - (long)my_new;
		if (buf_mod1 < 0)
			buf_mod1 = -buf_mod1;
    }

    cm1y = buf;
    cm2y = (long)(my * buf);

	TC_Store_Calivalue();

	Display_Puts_If_Ex(CENTER_X2,CENTER_Y1,(uint8_t*)"Calibration Completed!!",OPAQUE,COL_RED,COL_WHITE);
	_delay_ms(400);
}



#elif defined(USE_STMPE811_I2C)
/**************************************************************************/
/*! 
	TouchScreen Low Level.
	Downed  :	return 1.
	Released:	return 0.
*/
/**************************************************************************/
uint8_t TC_PenINT(void) 
{
	return (!PENIRQ_CHK()); 
/*
	if(!PENIRQ_CHK()){
		int tmp = STMPE811_ReadByte(STMPE811_TSC_CTRL) & STMPE811_TSC_STA;
		if(tmp==0)	return 0;
		else 	 	return 1;
	}
	else{
		return 0;
	}
*/
}

/**************************************************************************/
/*! 
	TouchScreen Low Level.
	Downed  :	return 1.
	Released:	return 0.
*/
/**************************************************************************/
uint8_t TC_PenDown(void)
{
	int tmp = STMPE811_ReadByte(STMPE811_TSC_CTRL) & STMPE811_TSC_STA;
	if(tmp==0)	return 0;
	else 	 	return 1;
}


/**************************************************************************/
/*! 
	TouchScreen Low Level.
*/
/**************************************************************************/
inline void TC_ReadRaw(void) 
{	
	/* If FIFO overflow, read all samples except the last one */
	uint8_t num = STMPE811_ReadByte(STMPE811_FIFO_SIZE);
	while (--num) (STMPE811_ReadFIFO(STMPE811_READFIFO_XY,STMPE811_DATA_XYZ));

	/* Retreive last taken sample */
	pTouch->X_Axis = (uint16_t)STMPE811_ReadWord(STMPE811_DATA_X);
	pTouch->Y_Axis = (uint16_t)STMPE811_ReadWord(STMPE811_DATA_Y);

    /* Clear interrupt flags */
	STMPE811_WriteByte(STMPE811_INT_STA, 0x02);
}

/**************************************************************************/
/*! 
	TouchScreen Low Level.
*/
/**************************************************************************/
inline void TC_ScanPen(void)
{
	if (TC_PenDown())	/* Downed Pen */
	{
		TC_ReadRaw();
		pPos->X_Axis = cal_posx(pTouch->X_Axis);
		pPos->Y_Axis = cal_posy(pTouch->Y_Axis);
		
		if (last_pen)  /* last_pen:1->1 */
		{
			tc_stat = TC_STAT_HOLD;
		}
		else		   /* last_pen:0->1 */
		{
			tc_stat = TC_STAT_DOWN;
		}
		last_pen = 1;
	
	}
	else
	{
		if (last_pen)  /* last_pen:1->0 */
		{
			tc_stat = TC_STAT_UP;
		}
		else		   /* last_pen:0->0 */
		{
			tc_stat = TC_STAT_NONE;
		}
		last_pen = 0;
	}
}

/**************************************************************************/
/*! 
	Calibrate and Init TouchScreen.
*/
/**************************************************************************/
void TC_CalibScreen_If(void)
{
	int buf;

	int x[TC_TP_NUM*TC_TP_NUM];
	int y[TC_TP_NUM*TC_TP_NUM];

    long sum_xx = 0;
    long sum_yy = 0;
    long sum_fx = 0;
    long sum_fy = 0;
    long sum_fxx = 0;
    long sum_fyy = 0;

	float cx;
	float mx;
	float cy;
	float my;

    float mx_new;
    float my_new;

	float buf_mod1;

	long fx[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];
	long fy[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];
	long xx[TC_TP_NUM*TC_TP_NUM];
	long yy[TC_TP_NUM*TC_TP_NUM];
	long fxx[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];
	long fyy[TC_TP_NUM*TC_TP_NUM][CALIB_TEST_TIME];

	uint8_t i,j,k;

	char unkbuf[16];

	/* Device Life Sign Check */
	if(!STMPE811_Init())
	{
		Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);
		for(;;){
			Display_Puts_If_Ex(5,5,(uint8_t*)"STMPE811 is NOT Init!",OPAQUE,COL_RED,COL_WHITE);
			_delay_ms(300);
			Display_Puts_If_Ex(5,5,(uint8_t*)"                     ",OPAQUE,COL_RED,COL_WHITE);
			_delay_ms(300);
		}
	}
	
	if(CHK_CALIBED()) /* check for valid touch screen calibration data */
	{

		Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);
		Display_Puts_If_Ex(5,5,(uint8_t*)"Touch The Screen To Calibrate",OPAQUE,COL_RED,COL_WHITE);

		k = 0;

		for (i=0;i<3;i++)
		{
			sprintf(unkbuf,"With in %c Sec",'3' - i);
			Display_Puts_If_Ex(5,5+(1*CurrentAnkDat->Y_Size),(uint8_t*)unkbuf,OPAQUE,COL_RED,COL_WHITE);

			for (j=0;j<10;j++)
			{
				if (TC_PenDown())
				{
					k = 1;
					break;
				}
				_delay_ms(100);
			}
			if (k)
				break;
		}

		if (k == 0) /* use old data */
		{
			TC_Restore_Calivalue();
			return;
		}
	}

	Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);

	Display_Puts_If_Ex(5,5,(uint8_t*)"Enter Calibration Mode!",OPAQUE,COL_RED,COL_WHITE);
	_delay_ms(500);

	Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);

	for (i=0; i<TC_TP_NUM; i++)
	{
		for (j=0; j<TC_TP_NUM; j++)
		{
			_delay_ms(100);	
			
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y1,(uint8_t*)"Touch Blue Square",TRANSPARENT,COL_RED,COL_WHITE);
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y2,(uint8_t*)"      Twice      ",TRANSPARENT,COL_RED,COL_WHITE);

			for (k=0; k<CALIB_TEST_TIME; k++)
			{
				Display_FillRect_If(tc_tpx[i]-2,tc_tpx[i]+2,tc_tpy[j]-2,tc_tpy[j]+2,COL_BLUE);
				while (!TC_PenDown());
					TC_ReadRaw();
					fx[i*TC_TP_NUM+j][k] = pTouch->X_Axis;
					fy[i*TC_TP_NUM+j][k] = pTouch->Y_Axis;
					Display_FillRect_If(tc_tpx[i]-2,tc_tpx[i]+2,tc_tpy[j]-2,tc_tpy[j]+2,COL_YELLOW);
					_delay_ms(300);
				while (TC_PenDown());
			}

			Display_Puts_If_Ex(CENTER_X1,CENTER_Y1,(uint8_t*)"                  ",OPAQUE,COL_WHITE,COL_WHITE);
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y2,(uint8_t*)"                  ",OPAQUE,COL_WHITE,COL_WHITE);
			Display_FillRect_If(tc_tpx[i]-2,tc_tpx[i]+2,tc_tpy[j]-2,tc_tpy[j]+2,COL_WHITE);
			_delay_ms(100);
			
		}
	}

	for (i=0;i<TC_TP_NUM*TC_TP_NUM;i++)
	{
        x[i] = tc_tpx[i/TC_TP_NUM] - chalfx;
        y[i] = tc_tpy[i%TC_TP_NUM] - chalfy;
        xx[i] = x[i] * x[i];
        yy[i] = y[i] * y[i];
		
		for (j=0;j<CALIB_TEST_TIME;j++)
		{
			fxx[i][j] = fx[i][j] * x[i];
			fyy[i][j] = fy[i][j] * y[i];
		}
	}

    for (i = 0; i < TC_TP_NUM*TC_TP_NUM; i++)
    {
        sum_xx += (xx[i] + xx[i]);
        sum_yy += (yy[i] + yy[i]);
		
		for (j=0;j<CALIB_TEST_TIME;j++)
		{
		    sum_fx += fx[i][j];
			sum_fy += fy[i][j];
			sum_fxx += fxx[i][j];
			sum_fyy += fyy[i][j];
		}
    }

    cx = ((float)sum_fx)  / ((float)(TC_TP_NUM * TC_TP_NUM * CALIB_TEST_TIME));
    mx = ((float)sum_fxx) / ((float)sum_xx);
    cy = ((float)sum_fy)  / ((float)(TC_TP_NUM * TC_TP_NUM * CALIB_TEST_TIME));
    my = ((float)sum_fyy) / ((float)sum_yy);

	ccx = (long)cx;
	ccy = (long)cy;

    mx_new = mx;
    my_new = my;

	buf_mod1 = mx_new - (long)mx_new;
	if (buf_mod1 < 0)
		buf_mod1 = -buf_mod1;
	buf = 1;

    while ((buf_mod1 > 0.05) && (buf_mod1 < 0.95))
    {
        buf++;
        mx_new = mx*((float) buf);
		buf_mod1 = mx_new - (long)mx_new;
		if (buf_mod1 < 0)
			buf_mod1 = -buf_mod1;
    }

	cm1x = buf;
    cm2x = (long)(mx * buf);

	buf_mod1 = my_new - (long)my_new;
	if (buf_mod1 < 0)
		buf_mod1 = -buf_mod1;
    buf = 1;

    while ((buf_mod1 > 0.05) && (buf_mod1 < 0.95))
    {
        buf++;
        my_new = my*((float)buf);
		buf_mod1 = my_new - (long)my_new;
		if (buf_mod1 < 0)
			buf_mod1 = -buf_mod1;
    }

    cm1y = buf;
    cm2y = (long)(my * buf);

	TC_Store_Calivalue();

	Display_Puts_If_Ex(CENTER_X2,CENTER_Y1,(uint8_t*)"Calibration Completed!!",OPAQUE,COL_RED,COL_WHITE);
	_delay_ms(400);
}

#elif defined(USE_FT6x06_I2C)
volatile ft6x06_strct pkt_touch = {0};
#define TOUCH_PACKETS 7

/**************************************************************************/
/*! 
	TouchScreen Low Level.
	Downed  :	return 1.
	Released:	return 0.
*/
/**************************************************************************/
uint8_t TC_PenINT(void) 
{
	return (!PENIRQ_CHK()); 
/*
	if(!PENIRQ_CHK()){
		int tmp = FT6x06_ReadByte(FT6x06_TSC_CTRL) & FT6x06_TSC_STA;
		if(tmp==0)	return 0;
		else 	 	return 1;
	}
	else{
		return 0;
	}
*/
}

/**************************************************************************/
/*! 
	TouchScreen Low Level.
	Downed  :	return 1.
	Released:	return 0.
*/
/**************************************************************************/
uint8_t TC_PenDown(void)
{
	if(EXTI_GetITStatus(TOUCH_SRC_EXTI) != RESET)
	{
		/* Clear the EXTI line pending bit */
		EXTI_ClearITPendingBit(TOUCH_SRC_EXTI);
		return 1;
	}
	else return 0;
}

/**************************************************************************/
/*! 
	TouchScreen Low Level.
*/
/**************************************************************************/
inline void TC_ReadRaw(void) 
{	
	/* Read CTP Registers 0~7 */
	FT6x06_ReadNBytes(FT6x06_DEV_MODE_REG,(uint8_t*)&pkt_touch,TOUCH_PACKETS);
}

/**************************************************************************/
/*! 
	TouchScreen Low Level.
*/
/**************************************************************************/
inline void TC_ScanPen(void)
{
	if (TC_PenDown())	/* Downed Pen */
	{
		TC_ReadRaw();
		
		/* Check Touch Detect and Single Point */
		if((pkt_touch.td_status & 0x0F) >0){
		#if defined(TOUCH_REV_XY)
			pPos->Y_Axis = ((uint16_t)(pkt_touch.xh & 0x0F)<<8 | (uint16_t)pkt_touch.xl);
			pPos->X_Axis = (MAX_X-((uint16_t)(pkt_touch.yh & 0x0F)<<8 | (uint16_t)pkt_touch.yl));
		#else
			pPos->X_Axis = ((uint16_t)(pkt_touch.xh & 0x0F)<<8 | (uint16_t)pkt_touch.xl);
			pPos->Y_Axis = ((uint16_t)(pkt_touch.yh & 0x0F)<<8 | (uint16_t)pkt_touch.yl);
		#endif
			if (last_pen)  /* last_pen:1->1 */
			{
				tc_stat = TC_STAT_HOLD;
			}
			else		   /* last_pen:0->1 */
			{
				tc_stat = TC_STAT_DOWN;
			}
			last_pen = 1;
		}
		else /* pkt_touch.td_status == 0 */
		{
			pPos->Y_Axis = 0xFFFF;
			pPos->X_Axis = 0xFFFF;
			if (last_pen)  /* last_pen:1->0 */
			{
				tc_stat = TC_STAT_UP;
			}
			else		   /* last_pen:0->0 */
			{
				tc_stat = TC_STAT_NONE;
			}
			last_pen = 0;
		}
	}
	else
	{
		if (last_pen)  /* last_pen:1->0 */
		{
			tc_stat = TC_STAT_UP;
		}
		else		   /* last_pen:0->0 */
		{
			tc_stat = TC_STAT_NONE;
		}
		last_pen = 0;
	}
}

/**************************************************************************/
/*! 
	Calibrate and Init TouchScreen.
*/
/**************************************************************************/
void TC_CalibScreen_If(void)
{
	uint8_t str_id[20] = {0};
	uint8_t ft6x06_id = FT6x06_Init();

	/* Device Life Sign Check */
	if(ft6x06_id == 0)
	{
		Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_WHITE);
		for(;;){
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y1,(uint8_t*)"FT6x06 is NOT Init!",OPAQUE,COL_RED,COL_WHITE);
			_delay_ms(300);
			Display_Puts_If_Ex(CENTER_X1,CENTER_Y1,(uint8_t*)"                   ",OPAQUE,COL_RED,COL_WHITE);
			_delay_ms(300);
		}
	}
	sprintf((char*)str_id,"Chip ID:0x%02X",ft6x06_id);
	Display_Puts_If_Ex(CENTER_X2,CENTER_Y1,(uint8_t*)"Capacitive Touch Input!",TRANSPARENT,COL_BLUE,COL_WHITE);
	Display_Puts_If_Ex(CENTER_X1,CENTER_Y2,str_id,TRANSPARENT,COL_BLUE,COL_WHITE);
	_delay_ms(1000);
}
#endif

/* End Of File ---------------------------------------------------------------*/
