/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : rtc_esp.c
*
*  Description         : It contains rtc functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  29th Sep 2021     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <time.h>
#include <sys/time.h>

/* ESP specific */
#include "driver/timer.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"

#include "osa.h"

#include "rtc.h"
#include "trace.h"
#include "profile_data_types.h"

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define TIMER_DIVIDER         (16)  //  Hardware timer clock divider
#define TIMER_SCALE           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static U32 rtc_get_week_day(U8* cur_time);

static void rtc_esp_timer_init(U32 group, U32 timer, BOOLEAN auto_reload, uint64_t timer_interval_sec,void* pArg);

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

    timer_deinit(TIMER_GROUP_0, TIMER_0);

    timer_deinit(TIMER_GROUP_0, TIMER_1);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_SetTime(U8* cur_time)
{
	TRACE_INFO("%s() Entry \n", __FUNCTION__);

//	TRACE_INFO("Exit = %s\n", cur_time);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 RTC_GetTime(U8* get_time)
{

    time_t now;
    struct tm timeinfo;
    U32 week_day;

  //  UBaseType_t uxTaskPriority = uxTaskPriorityGet(NULL);

   // char* task_name = pcTaskGetName( NULL );

  //  TRACE_DBG("%s() Entry  Task Priority =  (%d) Task Name  = %s \n",__FUNCTION__, uxTaskPriority,task_name);


    TRACE_DBG("%s() Entry \n", __FUNCTION__);

   // TRACE_DBG("%s() Entry  Task Priority =  (%d) Task Name  = %s \n",__FUNCTION__, uxTaskPriority,task_name);

    time(&now);
    
    localtime_r(&now, &timeinfo);

    week_day = rtc_get_week_day(ctime(&now));
 
    printf("CURRENT TIME  = %s   %d \n",ctime(&now),timeinfo.tm_year);

    if(get_time)
    {   
        sprintf(get_time,"%d %d/%d/%d %d:%d:%d",week_day,timeinfo.tm_mday,timeinfo.tm_mon+1,timeinfo.tm_year-100+2000,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);  

      //  printf("get_time  = %s\n",get_time);
    }

    TRACE_DBG("%s() Exit \n", __FUNCTION__);
    return ( (timeinfo.tm_hour*60 *60)  + (timeinfo.tm_min*60) + (timeinfo.tm_sec));
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_GetDateTime(mDATE* pDate,  mTIME* pTime , U8* DateStr , U8* TimeStr)
{
    time_t now;
    struct tm timeinfo;

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    time(&now);
    
    localtime_r(&now, &timeinfo);
 
    printf("TIME  = %s\n",ctime(&now));
   
    pDate->DD = timeinfo.tm_mday;

    pDate->MM = timeinfo.tm_mon+1;

    pDate->YY = timeinfo.tm_year-100+2000;

    pDate->WD =  rtc_get_week_day(ctime(&now));

    pTime->HH = timeinfo.tm_hour;

    pTime->MM = timeinfo.tm_min;

    pTime->SS = timeinfo.tm_sec;

    if(DateStr)
    {
        sprintf(DateStr,"%d/%d/%d",pDate->DD,pDate->MM,pDate->YY);
    }
    if(TimeStr)
    {
        sprintf(TimeStr,"%d:%d:%d",pTime->HH,pTime->MM,pTime->SS);
    }
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : It only uses TIMER0 GROUP of both TIMER0/TIMER1 whichever is free 
*******************************************************************************/
S32 RTC_StartAlarm(mTIME* start_time ,mDATE* date, CALLBACK32_FCT pFunction, void* pArg)
{
    S32 i;

    U32 timer_id;

    U32 timer_interval_sec;

    U32 st;

    U32 ct;

    BOOLEAN is_time_zero = FALSE;

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
	TRACE_INFO("Alarm No = %d  HH = %d  MM = %d SS = %d\n", i, start_time->HH, start_time->MM, start_time->SS);

    pRtc->Alarm[i].callback.pArg  = pArg;
    pRtc->Alarm[i].callback.pFunction = pFunction; 

    ct = RTC_GetTime(NULL);

    if(date)
    {
        if(date->WD) 
        {
            TRACE_INFO("Week Day Given  \n");
            if((start_time->HH == 0x00) &&
               (start_time->MM == 0x00) &&
               (start_time->SS == 0x00))
            {
                timer_interval_sec = 86400 - ct;
            }            
        }
    }
    else
    {
        if ((start_time->HH == 00)  &&  (start_time->MM == 00)  &&  (start_time->SS == 00))
        {
            is_time_zero = TRUE; 
            start_time->HH =23;
            start_time->MM =59;
            start_time->SS =59;
        }

        /* no date  given , so  only time  */
        st = start_time->HH*3600 + start_time->MM*60 + start_time->SS ;
        timer_interval_sec = (st-ct) ;

        if(is_time_zero )
        {
            timer_interval_sec +=1;
        }

    }
    TRACE_INFO("Time for Next Alarm( No = %d)  = %d \n",i, timer_interval_sec);

    timer_id = (i == 0 ) ? TIMER_0 : TIMER_1;

    rtc_esp_timer_init(TIMER_GROUP_0,timer_id, FALSE, (uint64_t)timer_interval_sec,&pRtc->Alarm[i]);

    TRACE_INFO("%s Exit \n", __FUNCTION__);

    return i;
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : It is direct ISR 
*******************************************************************************/
void IRAM_ATTR RTC_AlarmISR(void *pThis)
{
    Rtc_Alarm* pAlarm = (Rtc_Alarm*) pThis;
#ifdef ESP32_S3_MINI
    U32 intr_status = TIMERG0.int_st_timers.val;
#else
    U32 intr_status = TIMERG0.int_st.val;
#endif
  //  U32 intr_status = TIMERG0.int_st_timers.val;

    uint64_t timer_counter_value ;

    U32 timer_id = 0;

    ets_printf("%s() Entry(Status = %x)   Alarm = %p \n",__FUNCTION__,intr_status,pThis );

#if 1
    /* Clear timer bits */
    if (intr_status & BIT(TIMER_0))
    {
        timer_id = TIMER_0;
#ifdef ESP32_S3_MINI
        TIMERG0.int_clr_timers.t0_int_clr =1;
#else
        TIMERG0.int_clr.t0 = 1; 
#endif
     //   TIMERG0.int_clr_timers.t0_int_clr =1;      
    }
    if (intr_status & BIT(TIMER_1))
    {                
        timer_id = TIMER_1;
#ifdef ESP32_S3_MINI
        TIMERG0.int_clr_timers.t1_int_clr =1; 
#else
        TIMERG0.int_clr.t1 = 1; 
#endif
     //   TIMERG0.int_clr_timers.t1_int_clr =1;       
    }
#endif

#if 0
    timer_counter_value = ((uint64_t) TIMERG0.hw_timer[timer_id].cnt_high) << 32
                              | TIMERG0.hw_timer[timer_id].cnt_low;

    ets_printf("%s() timer_counter_value = %llx) \n",__FUNCTION__,timer_counter_value );
#endif
    if(pAlarm->callback.pFunction)
        pAlarm->callback.pFunction(pAlarm->callback.pArg, timer_id);
    /* free the timer */
    pAlarm->is_used = FALSE;

    //TIMERG0.hw_timer[timer_id].config.alarm_en = TIMER_ALARM_EN;
    
    ets_printf("%s() Exit \n",__FUNCTION__ );
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void RTC_StopAlarm(S32 alarm_no)
{
    U32 timer_id ;

    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,alarm_no);
    
    if( alarm_no < 0)
    {
        TRACE_INFO("%s() INVALID HW TIMER \n", __FUNCTION__);
        return;  /* nothing to do , invalid timer  */
    }

    pRtc->Alarm[alarm_no].is_used = FALSE;

    timer_id = (alarm_no == 0 ) ? TIMER_0 : TIMER_1;

    timer_deinit(TIMER_GROUP_0, timer_id);
}


/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 rtc_get_week_day(U8* cur_time)
{
    U32 i;
    U8* p;
    U8* q;
    U8 temp[5];
    U8* wk[7] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    U32 week_day;

   // TRACE_INFO("%s() Entry \n", __FUNCTION__);

    /* Get weekday */
    p = cur_time;
    q = strchr(p,' ');
    memcpy(temp,p, q-p);
    temp[q-p] = 0x00;

    for(i=0; i<7; i++)
    {
        if (strstr(temp,wk[i]) != NULL)
        {
            week_day = i+1;
            break;
        }
    }
    return week_day;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void rtc_esp_timer_init(U32 group, U32 timer, BOOLEAN auto_reload, uint64_t timer_interval_sec,void* pArg)
{
    uint64_t tmr_value;

    /* Select and initialize basic parameters of the timer */
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    }; // default clock source is APB

    TRACE_INFO("%s() Entry(Alarm = %p) \n", __FUNCTION__,pArg);

    timer_init(group, timer, &config);

    /* Timer's counter will initially start from value below.
       Also, if auto_reload is set, this value will be automatically reload on alarm */
    timer_set_counter_value(group, timer, 0);

    tmr_value = timer_interval_sec * TIMER_SCALE;

    printf(" TIMER SET VALUE  = %llx \n",tmr_value );

    /* Configure the alarm value and the interrupt on alarm. */
    timer_set_alarm_value(group, timer, tmr_value);
    timer_enable_intr(group, timer);

    //timer_isr_callback_add(group, timer, RTC_AlarmISR, pArg, 0);
    //timer_isr_callback_add(group, timer, timer_group_isr_callback, pArg, 0);

    timer_isr_register(group, timer, RTC_AlarmISR, pArg, ESP_INTR_FLAG_IRAM, NULL);

    timer_start(group, timer);
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
