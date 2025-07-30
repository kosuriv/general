/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : rtc.c
*
*  Description         : It contains rtc driver functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  22nd Sep 2020     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "rtc.h"
#include "trace.h"

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static RTC_T* pRtc;

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_Init(RTC_T  *pThis)
{
	TRACE_INFO("%s Entry \n", __FUNCTION__);

  	pRtc = pThis;

    memset((U8*)pRtc,0x00,sizeof(RTC_T));

    pRtc->hrtc.Instance = RTC;
    pRtc->hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    pRtc->hrtc.Init.AsynchPrediv = 127;
    pRtc->hrtc.Init.SynchPrediv = 255;
    pRtc->hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    pRtc->hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    pRtc->hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&pRtc->hrtc) != HAL_OK)
    {
        Error_Handler();
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_SetTime(U8* cur_time)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	U8* p;

	U8* q;

	U8 temp[5];

	U8* wk[7] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
	U8* mm[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	U32 i;


	TRACE_INFO("%s() Entry \n", __FUNCTION__);

	TRACE_INFO("SNTP TIME = %s\n", cur_time);

	p = cur_time;
	q = strchr(p,' ');

	memcpy(temp,p, q-p);
	temp[q-p] = 0x00;

	for(i=0; i<7; i++)
	{
		if (strstr(temp,wk[i]) != NULL)
		{
			date.WeekDay = i+1;
			break;
		}
	}
//	TRACE_DBG("WEEK DAY  = %s  Value = %d\n", temp, date.WeekDay);

	q++;
	p=q;
	q = strchr(p,' ');

	memcpy(temp,p, q-p);
	temp[q-p] = 0x00;

	//TRACE_DBG("MONTH  = %s \n", temp);

#if 1
	for(i=0; i<12; i++)
	{
		if (strstr(temp,mm[i]) != NULL)
		{
			date.Month = i+1;
			break;
		}
	}
//	TRACE_DBG("MONTH  = %s  Value = %d\n", temp, date.Month);
#endif

	q++;
	p=q;
	q = strchr(p,' ');
	*q = 0x00;
	date.Date=atoi(p);
//	TRACE_DBG("DATE  = %d\n",date.Date);

	q++;
	p=q;
	q = strchr(p,':');
	*q = 0x00;
	time.Hours=atoi(p);
//	TRACE_DBG("Hours  = %d\n",time.Hours);

	q++;
	p=q;
	q = strchr(p,':');
	*q = 0x00;
	time.Minutes=atoi(p);
//	TRACE_DBG("Minutes  = %d\n",time.Minutes);

	q++;
	p=q;
	q = strchr(p,' ');
	*q = 0x00;
	time.Seconds=atoi(p);
//	TRACE_DBG("Seconds  = %d\n",time.Seconds);

	q +=3;
	date.Year = strtol((char *)(q), NULL, 10);
	strcpy(temp,q);
//	TRACE_DBG("Year  = %d  STRING =%s\n",date.Year,temp);

	time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	time.StoreOperation = RTC_STOREOPERATION_RESET;



#if 1
	if (HAL_RTC_SetTime(&pRtc->hrtc, &time, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_RTC_SetDate(&pRtc->hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
#endif
	TRACE_DBG(" RTC Time updated successfully \n");

}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 RTC_GetTime(U8* get_time)
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;

	TRACE_INFO("%s() Entry \n", __FUNCTION__);

	if (HAL_RTC_GetTime(&pRtc->hrtc, &time, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
	if (HAL_RTC_GetDate(&pRtc->hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}

//	TRACE_DBG("Hours  = %d\n",time.Hours);
//	TRACE_DBG("Minutes  = %d\n",time.Minutes);
//	TRACE_DBG("Seconds  = %d\n",time.Seconds);
	sprintf(get_time,"%d %d/%d/%d %d:%d:%d", date.WeekDay, date.Date,date.Month, date.Year+2000, time.Hours,time.Minutes,time.Seconds );
	TRACE_DBG("Cur time = %s\n",get_time);

	return ( (time.Hours*60 *60)  + (time.Minutes*60) + (time.Seconds));

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_GetDateTime(mDATE* pDate,  mTIME* pTime , U8* DateStr , U8* TimeStr)
{
   
    RTC_TimeTypeDef time;

    RTC_DateTypeDef date;

    TRACE_INFO("%s() Entry \n", __FUNCTION__);

    if (HAL_RTC_GetTime(&pRtc->hrtc, &time, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_RTC_GetDate(&pRtc->hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
    pDate->DD = date.Date;

    pDate->MM = date.Month;

    pDate->YY = date.Year+2000;

    pDate->WD = date.WeekDay;

    pTime->HH = time.Hours;

    pTime->MM = time.Minutes;

    pTime->SS = time.Seconds;

    if(DateStr)
    {
        sprintf(DateStr,"%d/%d/%d",date.Date,date.Month, date.Year+2000);
    }
    if(TimeStr)
    {
        sprintf(TimeStr,"%d:%d:%d",time.Hours,time.Minutes,time.Seconds);
    }
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
S32 RTC_StartAlarm(mTIME* time ,mDATE* date, CALLBACK32_FCT pFunction, void* pArg)
{
	U32 i;

  	RTC_AlarmTypeDef sAlarm = {0};

	TRACE_INFO("%s Entry \n", __FUNCTION__);

	for(i=0; i<2 ; i++)
	{
		if(!pRtc->Alarm[i].is_used)
		{
			pRtc->Alarm[i].is_used = TRUE;
			break;
		}
	}
	if(i ==2)
	{
		TRACE_WARN("NO FREE RTC ALARM \n");
		return -1;
	}
	TRACE_INFO("Alarm No = %d  HH = %d  MM = %d SS = %d\n", i, time->HH, time->MM, time->SS);

	pRtc->Alarm[i].callback.pArg  = pArg;
  	pRtc->Alarm[i].callback.pFunction = pFunction;	

    sAlarm.AlarmTime.Hours = (U8)time->HH;
	sAlarm.AlarmTime.Minutes = (U8)time->MM;
	sAlarm.AlarmTime.Seconds = (U8)time->SS;
	sAlarm.AlarmTime.SubSeconds = 0x0;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

	if(date == NULL)
	{
			/* ignore data field check */
		sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;
	}
	else
	{
		sAlarm.AlarmMask = RTC_ALARMMASK_NONE;

        if(date->DD)
        {
	    	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
		    sAlarm.AlarmDateWeekDay = date->DD;
        }
        else
        {
            sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
            sAlarm.AlarmDateWeekDay = date->WD;            
        }
	}
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	
	if(i == 0)
	{
		sAlarm.Alarm = RTC_ALARM_A;
	}
	else
	{
		sAlarm.Alarm = RTC_ALARM_B;
	}

	if (HAL_RTC_SetAlarm_IT(&pRtc->hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
	{
		TRACE_WARN("ALARAM SET ERROR\n");
		Error_Handler();
	}

	return i;

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_AlarmISR()
{
	U32 i;

	TRACE_INFO("%s() Entry \n", __FUNCTION__);

	if(__HAL_RTC_ALARM_GET_IT_SOURCE(&pRtc->hrtc, RTC_IT_ALRA) != (uint32_t)RESET)
  	{
    	/* Get the pending status of the AlarmA Interrupt */
    	if(__HAL_RTC_ALARM_GET_FLAG(&pRtc->hrtc, RTC_FLAG_ALRAF) != (uint32_t)RESET)
    	{
      		/* AlarmA callback */
            if(pRtc->Alarm[0].callback.pFunction)
                pRtc->Alarm[0].callback.pFunction(pRtc->Alarm[0].callback.pArg, 0);

      		/* Clear the AlarmA interrupt pending bit */
      		__HAL_RTC_ALARM_CLEAR_FLAG(&pRtc->hrtc,RTC_FLAG_ALRAF);

      		pRtc->Alarm[0].is_used = FALSE;
    	}
    	__HAL_RTC_ALARMA_DISABLE(&pRtc->hrtc);
 	 }

	if(__HAL_RTC_ALARM_GET_IT_SOURCE(&pRtc->hrtc, RTC_IT_ALRB) != (uint32_t)RESET)
  	{
    	/* Get the pending status of the AlarmA Interrupt */
    	if(__HAL_RTC_ALARM_GET_FLAG(&pRtc->hrtc, RTC_FLAG_ALRBF) != (uint32_t)RESET)
    	{
      		/* AlarmB callback */
            if(pRtc->Alarm[1].callback.pFunction)
                pRtc->Alarm[1].callback.pFunction(pRtc->Alarm[1].callback.pArg,1);

      		/* Clear the AlarmA interrupt pending bit */
      		__HAL_RTC_ALARM_CLEAR_FLAG(&pRtc->hrtc,RTC_FLAG_ALRBF);

      		pRtc->Alarm[1].is_used = FALSE;
    	}
   	    __HAL_RTC_ALARMB_DISABLE(&pRtc->hrtc);
 	 }	
	 __HAL_RTC_ALARM_EXTI_CLEAR_FLAG();
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_StopAlarm(S32 alarm_no)
{
    pRtc->Alarm[alarm_no].is_used = FALSE;

    if(alarm_no == 0)
    {
        HAL_RTC_DeactivateAlarm(&pRtc->hrtc, RTC_ALARM_A);
    }
    else
    {
        HAL_RTC_DeactivateAlarm(&pRtc->hrtc, RTC_ALARM_B);   
    }
}

/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/


/*******************************************************************************
*                          End of File
*******************************************************************************/
