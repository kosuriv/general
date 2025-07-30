/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : device.cpp
*
*  Description         : 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*  venu Kosuri    7th Oct 2020      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "device.hpp"
#include "trace.h"
#include "rtc.h"
#include "utility.h"
#include "esp_log.h"
/*******************************************************************************
*                          Helper Types and constants
*******************************************************************************/
#define DEVICE_LIGHT_USE_RTC_ALARM   1

/*******************************************************************************
*                          Helper functions
*******************************************************************************/
static void device_light_rtc_alarm_callback(void* Arg , U32 alarm_no);

/*******************************************************************************
*                          Local data
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern DEV_LIGHT  Light;

extern DEV_GEN    HoodVent;

extern DEV_GEN    Circ;

extern DEV_VENT   Vent;

extern DEV_GEN    Fxp1;

extern DEV_GEN    Fxp2;

extern DEV_OUTLET* DevList[] ;

extern "C" void APP_do_light_cycle_transition();

extern "C" void APP_send_hot_restrike_fault_msg();

/*******************************************************************************
*                          Class Implementation
*******************************************************************************/

// =============================== Constructor =================================


// =============================== Destructor ==================================


// =============================== Copy Constructor ============================


// =============================== Assignment ==================================

/*******************************************************************************
*                          member functions
*******************************************************************************/

                               /* Light Class Functions  */

void DEV_LIGHT::mInit(GrowConfig_DevLightInfo* pInfo , DEVICE_TYPE name)
{
    DEV_OUTLET::mInit(name);

    pLightInfo = pInfo;
}

void DEV_LIGHT::mExecute()
{
	U8 time[50];

    TRACE_INFO("%s Entry \n", __FUNCTION__);

	/* This function maintains light cycle timings & starts respective light or dark cycles.
       whenever there is cycle change,it gets executed & call respective device run functions. */

    if(pLightInfo->LightCycle == ALWAYS_ON )
    {
        is_light_cycle = TRUE;
    }
    else if(pLightInfo->LightCycle == ALWAYS_OFF )
    {
        is_light_cycle = FALSE;
    }
    else
    {
        light_on_time = mGetSec(&pLightInfo->Light_OnTime);

        light_off_time = mGetSec(&pLightInfo->Light_OffTime);

        cur_time = RTC_GetTime(time);

        TRACE_INFO("TIME_NOW = %s \n", time);

        if(light_on_time < light_off_time)
        {
            if(cur_time < light_on_time)
            {
                /* start timer to switch on light when the time reaches */
               rtc_alarm_no = RTC_StartAlarm(&pLightInfo->Light_OnTime, NULL ,device_light_rtc_alarm_callback, this);
            }
            else if (( cur_time >= light_on_time) && (cur_time < light_off_time))
            {
                /* start timer to swith off when the time reaches */
                rtc_alarm_no = RTC_StartAlarm(&pLightInfo->Light_OffTime, NULL ,device_light_rtc_alarm_callback, this);                

                TRACE_INFO("IT IS LIGHT CYCLE NOW\n");

                is_light_cycle = TRUE;
                pGrowStatus->cycle_type = LIGHT;
            }
            else if  ( cur_time >= light_off_time)
            {
                TRACE_INFO("IT IS DARK CYCLE NOW \n");
                is_light_cycle = FALSE;
                pGrowStatus->cycle_type = DARK;
            }
        }
        else
        {
            if(cur_time < light_off_time) 
            {               
                /* so start timer to switch off light when the time reaches */
                rtc_alarm_no = RTC_StartAlarm(&pLightInfo->Light_OffTime, NULL ,device_light_rtc_alarm_callback, this);                
            }
            else if( ( cur_time >= light_off_time) && (cur_time < light_on_time))
            {
                /* start light on timer to switch on when the time reaches */
                rtc_alarm_no = RTC_StartAlarm(&pLightInfo->Light_OnTime, NULL ,device_light_rtc_alarm_callback, this);                
                TRACE_INFO("IT IS DARK CYCLE ALREADY \n");   
                is_light_cycle = FALSE;  
                pGrowStatus->cycle_type = DARK;          
            }
            else if  ( cur_time >= light_on_time)
            {
                TRACE_INFO("IT IS LIGHT CYCLE ALREADY \n");
                is_light_cycle = TRUE; 
                pGrowStatus->cycle_type = LIGHT;
            }
        }
    }

	/* run all devices */
    mRun();
    HoodVent.mRun();
    Circ.mRun();
    Fxp1.mRun();
    Fxp2.mRun();
    Vent.mRun();

    TRACE_INFO("%s Exit \n", __FUNCTION__);

}

void DEV_LIGHT::mRun()
{

    OSA_LockMutex(&run_mutex);
    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

    {
        if(is_light_cycle)
        {
            mActivate();
        }
        else
        {
            mDeActivate();
        }
        
    }
    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);
    OSA_UnLockMutex(&run_mutex);
}

void DEV_LIGHT::mStartTimer(U32 duration)
{
    DEV_OUTLET::mStartTimer(duration);
}

void DEV_LIGHT::mStopTimer()
{
	DEV_OUTLET::mStopTimer();
}

void DEV_LIGHT::mDoTimerAction()
{

    OSA_LockMutex(&run_mutex);

    mStopTimer();

	pGrowStatus->hot_restrike_fault = FALSE;  

    if((!pGrowStatus->ac_power_fault) &&
       (!pGrowStatus->rs485_fuse_fault) &&
       (!pGrowStatus->panic_switch_on) &&
       (cur_on_state) && 
       (pLightInfo->DevState != SUSPENDED))
    {
        fpActivate(dev_name);
        is_dev_on = TRUE;            
    }
    else
    {
        fpDeActivate(dev_name);
        is_dev_on = FALSE;
    } 
    OSA_UnLockMutex(&run_mutex); 
}


void DEV_LIGHT::mGetStatus()
{
	U8 time[50];

  //  TRACE_DBG("%s Entry(%d) \n", __FUNCTION__,dev_name);

	light_on_time = mGetSec(&pLightInfo->Light_OnTime);

    light_off_time = mGetSec(&pLightInfo->Light_OffTime);

    cur_time = RTC_GetTime(time);

	pGrowStatus->light_mode = pLightInfo->LightCycle;

	pGrowStatus->dev_light_config = pLightInfo->DevState;

	pGrowStatus->dev_light_on = is_dev_on;

	if((pLightInfo->LightCycle == ALWAYS_ON ) || (pLightInfo->DevState == FORCED))
    {
		pGrowStatus->light_cycle_duration = TOTAL_DAY_DURATION;
		pGrowStatus->dark_cycle_duration = 0x00;
        pGrowStatus->elapsed_light_cycle_duration = cur_time;
        pGrowStatus->pending_light_cycle_duration = pGrowStatus->light_cycle_duration - cur_time;
        pGrowStatus->elapsed_dark_cycle_duration = 0x00;
        pGrowStatus->pending_dark_cycle_duration = 0x00;
            
    }
	else if((pLightInfo->LightCycle == ALWAYS_OFF ) || (pLightInfo->DevState == SUSPENDED))
	{
		pGrowStatus->dark_cycle_duration = TOTAL_DAY_DURATION;
		pGrowStatus->light_cycle_duration = 0x00;
        pGrowStatus->elapsed_dark_cycle_duration = cur_time;
        pGrowStatus->pending_dark_cycle_duration = pGrowStatus->dark_cycle_duration - cur_time;
        pGrowStatus->elapsed_light_cycle_duration = 0x00;
        pGrowStatus->pending_light_cycle_duration = 0x00;
	}
	else
	{
		if( light_on_time < light_off_time)
		{
			pGrowStatus->light_cycle_duration = light_off_time - light_on_time;

			pGrowStatus->dark_cycle_duration = TOTAL_DAY_DURATION - pGrowStatus->light_cycle_duration;

			if(cur_time < light_on_time)
			{
				pGrowStatus->elapsed_light_cycle_duration = 0x00;

				pGrowStatus->pending_light_cycle_duration = pGrowStatus->light_cycle_duration;

				pGrowStatus->elapsed_dark_cycle_duration = cur_time;

				pGrowStatus->pending_dark_cycle_duration = pGrowStatus->dark_cycle_duration - cur_time ;
			}
			else if (( cur_time >= light_on_time) && (cur_time < light_off_time))
			{
				pGrowStatus->elapsed_light_cycle_duration = cur_time - light_on_time;

				pGrowStatus->pending_light_cycle_duration = light_off_time - cur_time;

				pGrowStatus->elapsed_dark_cycle_duration = light_on_time;

				pGrowStatus->pending_dark_cycle_duration = pGrowStatus->dark_cycle_duration - light_on_time ;
			}
			else if  ( cur_time >= light_off_time)
			{
				pGrowStatus->elapsed_light_cycle_duration = pGrowStatus->light_cycle_duration;

				pGrowStatus->pending_light_cycle_duration = 0x00;

				pGrowStatus->elapsed_dark_cycle_duration = cur_time-light_off_time + light_on_time;

				pGrowStatus->pending_dark_cycle_duration = pGrowStatus->dark_cycle_duration - pGrowStatus->elapsed_dark_cycle_duration ;
			}
		}
		else
		{
			pGrowStatus->dark_cycle_duration = light_on_time - light_off_time;

			pGrowStatus->light_cycle_duration = TOTAL_DAY_DURATION - pGrowStatus->dark_cycle_duration;

			if(cur_time < light_off_time)
			{
				pGrowStatus->elapsed_dark_cycle_duration = 0x00;

				pGrowStatus->pending_dark_cycle_duration = pGrowStatus->dark_cycle_duration;

				pGrowStatus->elapsed_light_cycle_duration = cur_time;

				pGrowStatus->pending_light_cycle_duration = pGrowStatus->light_cycle_duration - cur_time;
			}
			else if (( cur_time >= light_off_time) && (cur_time < light_on_time))
			{
				pGrowStatus->elapsed_dark_cycle_duration = cur_time - light_off_time;

				pGrowStatus->pending_dark_cycle_duration = light_on_time - cur_time;

				pGrowStatus->elapsed_light_cycle_duration = light_off_time;

				pGrowStatus->pending_light_cycle_duration = pGrowStatus->light_cycle_duration - light_off_time;
			}
			else if  ( cur_time >= light_on_time)
			{
				pGrowStatus->elapsed_dark_cycle_duration = pGrowStatus->dark_cycle_duration;

				pGrowStatus->pending_dark_cycle_duration = 0x00;

				pGrowStatus->elapsed_light_cycle_duration = cur_time-light_on_time + light_off_time;

				pGrowStatus->pending_light_cycle_duration = pGrowStatus->light_cycle_duration - pGrowStatus->elapsed_light_cycle_duration;
			}
		}
	}
    
    /* convert to format HH:MM:SS */
    mTimeConvert(pGrowStatus->light_cycle_duration,pGrowStatus->lc_duration);
    mTimeConvert(pGrowStatus->elapsed_light_cycle_duration,pGrowStatus->elapsed_lc_duration);
    mTimeConvert(pGrowStatus->pending_light_cycle_duration,pGrowStatus->pending_lc_duration);
    mTimeConvert(pGrowStatus->dark_cycle_duration,pGrowStatus->dc_duration);
    mTimeConvert(pGrowStatus->elapsed_dark_cycle_duration,pGrowStatus->elapsed_dc_duration);
    mTimeConvert(pGrowStatus->pending_dark_cycle_duration,pGrowStatus->pending_dc_duration);

    //TRACE_DBG("%s Exit(%d) \n", __FUNCTION__,dev_name);
}


void DEV_LIGHT::mActivate()
{
    U32 i;

  	/* fresh start of light cycle , reset  grow time*/
    UTILITY_convert_time_duration_to_str(0x00 , pGrowStatus->grow_time);

    DEV_OUTLET::mActivate(pLightInfo->DevState); 
}

void DEV_LIGHT::mDeActivate()
{
    U32 i;

    DEV_OUTLET::mDeActivate(pLightInfo->DevState);
    pGrowStatus->dev_light_counter++;
}

void DEV_LIGHT::mClearList()
{
	DEV_OUTLET::mClearList();   
}

void DEV_LIGHT::mSubscribe(DEVICE_TYPE dev_name, DEVICE_OP_STATE dev_state,DEV_OUTLET* dev_obj)
{
	DEV_OUTLET::mSubscribe(dev_name,dev_state,dev_obj);
}

void DEV_LIGHT::mStop()
{
    is_light_cycle = FALSE;
    RTC_StopAlarm(rtc_alarm_no);
    DEV_OUTLET::mStop(); 
}

void DEV_LIGHT::mFaultHandler(BOOLEAN fault_occured)
{
    OSA_LockMutex(&run_mutex);
    if(fault_occured && pGrowStatus->hot_restrike_fault && pGrowInfo->is_light_restrike_timer_present && !OSA_IsTimerRunning(&action_timer))
    {
        mStartTimer(pGrowInfo->light_restrike_timer_duration);
    }
    DEV_OUTLET::mFaultHandler(fault_occured,pLightInfo->DevState);
    OSA_UnLockMutex(&run_mutex);
}

void DEV_LIGHT::mStateHandler()
{
    OSA_LockMutex(&run_mutex);
    DEV_OUTLET::mStateHandler(pLightInfo->DevState);
    OSA_UnLockMutex(&run_mutex);
}

void DEV_LIGHT::mStopAlarm()
{
    RTC_StopAlarm(rtc_alarm_no);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : executes under RTC Alarm ISR
*******************************************************************************/
static void device_light_rtc_alarm_callback(void* Arg , U32 alarm_no)
{

    DEV_LIGHT* light = (DEV_LIGHT *) Arg;

#ifdef ESP32_S2_MINI
    ets_printf("%s Entry \n", __FUNCTION__);

    ets_printf("Alaram Callback(%d) \n", alarm_no);
#endif

    light->rtc_alarm_no = alarm_no;

	APP_do_light_cycle_transition();
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
