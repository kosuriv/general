/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : device_vent.cpp
*
*  Description         : 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*  venu Kosuri    14th Oct 2020      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "device_wrapper.h"
#include "device.hpp"
#include "trace.h"
#include "rtc.h"
#include "sensor.h"

/*******************************************************************************
*                          Helper Types and constants
*******************************************************************************/
static void device_vent_rpt_timer_expiry_handler(void *arg);

static void device_vent_threshold_timer_expiry_handler(void *arg);

static void device_vent_rpt_timer_expiry_handler_wrapper(void *arg);

static void device_vent_threshold_timer_expiry_handler_wrapper(void *arg);

/*******************************************************************************
*                          Helper functions
*******************************************************************************/

/*******************************************************************************
*                          Local data
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern DEV_OUTLET* DevList[];

extern "C" void APP_send_timer_expiry_msg(U8* buffer , U32 length);

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

                             /* Device general  Class Functions  */

void DEV_VENT::mInit(GrowConfig_DevVentInfo* pInfo , DEVICE_TYPE name)
{
    U8 time[50];

    DEV_OUTLET::mInit(name);
    pVentInfo = pInfo;

    is_rpt_activation = FALSE;
    is_temp_activation = FALSE;
    is_humidity_activation = FALSE;
    rpt_event = NONE;
    switch_off_time = 0x00;

}

void DEV_VENT::mRun()
{
    U32 timer_duration_sec;

    BOOLEAN to_start_rpt_timer = FALSE;

    BOOLEAN to_start_threshold_timer = FALSE;

    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    OSA_LockMutex(&run_mutex);

    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

    /* Stop any previous RPT timer  as well as sensing timer */
    OSA_StopTimer(&rpt_cycle_timer);
    OSA_DeInitTimer(&rpt_cycle_timer);

    OSA_StopTimer(&sensing_timer);
    OSA_DeInitTimer(&sensing_timer);

    {

        if(pLight->is_light_cycle)
        {
            if(pVentInfo->is_rpttimer_enabled_in_lc)
            {
               // timer_duration_sec = mGetSec(&pVentInfo->lc_start_delay);
                //to_start_rpt_timer = TRUE;
                mRunRptMode_helper(mGetSec(&pVentInfo->lc_start_delay), mGetSec(&pVentInfo->lc_on_time), mGetSec(&pVentInfo->lc_off_time));
            }
            if((pVentInfo->is_temp_check_needed_in_lc) || (pVentInfo->is_humidity_check_needed_in_lc))
            {
                to_start_threshold_timer = TRUE;
            }
        }
        else
        {   
            if(pVentInfo->is_rpttimer_enabled_in_dc)
            {
               // timer_duration_sec = mGetSec(&pVentInfo->dc_start_delay);
               // to_start_rpt_timer = TRUE;
                mRunRptMode_helper(mGetSec(&pVentInfo->dc_start_delay), mGetSec(&pVentInfo->dc_on_time), mGetSec(&pVentInfo->dc_off_time));           
            }
            if((pVentInfo->is_temp_check_needed_in_dc) || (pVentInfo->is_humidity_check_needed_in_dc))
            {
                TRACE_INFO("%s SETTING FLAG (%d)  \n", __FUNCTION__,dev_name);
                to_start_threshold_timer = TRUE;
            }
        }

#if 0
        if(to_start_rpt_timer)
        {
            rpt_event = START_TIMER;
            OSA_InitTimer(&rpt_cycle_timer,timer_duration_sec*1000, FALSE,device_vent_rpt_timer_expiry_handler, this);
            OSA_StartTimer(&rpt_cycle_timer);
        }
#endif

        if(to_start_threshold_timer)
        {
            TRACE_INFO("%s Starting Sensor timer (%d)  \n", __FUNCTION__,dev_name);
            OSA_InitTimer(&sensing_timer,SENSING_INTERVAL*1000, TRUE,device_vent_threshold_timer_expiry_handler, this);
            OSA_StartTimer(&sensing_timer);
        }
    }
    TRACE_INFO("%s Exit(%d)  \n", __FUNCTION__,dev_name);

    OSA_UnLockMutex(&run_mutex);
}

void DEV_VENT::mRunRptMode_helper(U32 sdelay, U32 on_time, U32 off_time)
{

    TRACE_INFO("%s Values are  (%d) (%d) (%d)  \n", __FUNCTION__,sdelay,on_time,off_time);

    if(sdelay)
    {
        rpt_event = START_TIMER;
        OSA_InitTimer(&rpt_cycle_timer,sdelay*1000,FALSE, device_vent_rpt_timer_expiry_handler, this);
        OSA_StartTimer(&rpt_cycle_timer);
    }
    else if(on_time)
    {
        is_rpt_activation = TRUE;
        mActivate();
        rpt_event = ON_TIMER;
        OSA_InitTimer(&rpt_cycle_timer,on_time*1000,FALSE, device_vent_rpt_timer_expiry_handler, this);
        OSA_StartTimer(&rpt_cycle_timer);            
    }
    else if(off_time)
    {
        is_rpt_activation = FALSE;
        mDeActivate();
        rpt_event = OFF_TIMER;
        OSA_InitTimer(&rpt_cycle_timer,off_time*1000,FALSE, device_vent_rpt_timer_expiry_handler, this);
        OSA_StartTimer(&rpt_cycle_timer);
    }
    else
    {
        rpt_event = NONE;
    }
}

void DEV_VENT::mStartTimer(U32 duration)
{
    DEV_OUTLET::mStartTimer(duration);
}

void DEV_VENT::mStopTimer()
{
	DEV_OUTLET::mStopTimer();
}

void DEV_VENT::mDoTimerAction()
{
    mStopTimer();
	mRun();    
}

void DEV_VENT::mGetStatus()
{
   // TRACE_INFO("%s Entry \n", __FUNCTION__);
    pGrowStatus->dev_vent_config = pVentInfo->DevState;
    pGrowStatus->dev_vent_on = is_dev_on;
}

void DEV_VENT::mActivate()
{
	U32 i;
    U8 time[50];

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    if((is_rpt_activation) || (is_temp_activation) || (is_humidity_activation))
    {
        TRACE_INFO("%s Activate Flag Set  \n", __FUNCTION__);
    	DEV_OUTLET::mActivate(pVentInfo->DevState);
    }
    else
    {
		if(is_dev_on)
    	{
        	switch_off_time = RTC_GetTime(time);
    	}
        DEV_OUTLET::mDeActivate(pVentInfo->DevState);
        pGrowStatus->dev_vent_counter++;
    }

}

void DEV_VENT::mDeActivate()
{
	U32 i;
	U8 time[50];

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    if((is_rpt_activation) || (is_temp_activation) || (is_humidity_activation))
    {
        DEV_OUTLET::mActivate(pVentInfo->DevState);
    }
    else
    {
		if(is_dev_on)
    	{
        	switch_off_time = RTC_GetTime(time);
    	}
        DEV_OUTLET::mDeActivate(pVentInfo->DevState);
        pGrowStatus->dev_vent_counter++;
    }
    if(!is_dev_on)
    {
        switch_off_time = RTC_GetTime(time);
    }
}

void DEV_VENT::mClearList()
{
	DEV_OUTLET::mClearList();   
}


void DEV_VENT::mRptTimerAction()
{
    U32 timer_duration_sec;

    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    OSA_LockMutex(&run_mutex);

    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

    OSA_StopTimer(&rpt_cycle_timer);
    OSA_DeInitTimer(&rpt_cycle_timer);

    if(pLight->is_light_cycle)
    {
        mRptTimerAction_helper(mGetSec(&pVentInfo->lc_start_delay),mGetSec(&pVentInfo->lc_on_time), mGetSec(&pVentInfo->lc_off_time));
    }
    else
    {   
        mRptTimerAction_helper(mGetSec(&pVentInfo->dc_start_delay),mGetSec(&pVentInfo->dc_on_time), mGetSec(&pVentInfo->dc_off_time));
    }
    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);

    OSA_UnLockMutex(&run_mutex);
}

void DEV_VENT::mRptTimerAction_helper(U32 sdelay, U32 on_time, U32 off_time)
{
    TRACE_INFO("%s Values are  (%d) (%d) (%d) (%d) \n", __FUNCTION__,sdelay,on_time,off_time,rpt_event);

    switch(rpt_event)
    {
        case START_TIMER:
            if(on_time)
            {   
                is_rpt_activation = TRUE;
                mActivate();
                rpt_event = ON_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,on_time*1000,FALSE, device_vent_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);
            }
            else if(off_time)
            {
                is_rpt_activation = FALSE;
                mDeActivate();
                rpt_event = OFF_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,off_time*1000,FALSE, device_vent_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);                
            }
            break;

        case ON_TIMER:
            if(off_time)
            {
                is_rpt_activation = FALSE;
                mDeActivate();
                rpt_event = OFF_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,off_time*1000,FALSE, device_vent_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);                
            }    
            break;

        case OFF_TIMER:
            if(on_time)
            {
                is_rpt_activation = TRUE;
                mActivate();
                rpt_event = ON_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,on_time*1000,FALSE, device_vent_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);                
            }    
            break;
    }   
}

void DEV_VENT::mThresholdTimerAction()
{
    FLOAT cur_temp_value;
    FLOAT cur_humidity_value;

    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    OSA_LockMutex(&run_mutex);

  //  TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

    if((pVentInfo->is_temp_check_needed_in_lc) || (pVentInfo->is_temp_check_needed_in_dc))
    {
        cur_temp_value = SENSOR_Get(1, SENSOR_TEMPERATURE);
    }
    if((pVentInfo->is_humidity_check_needed_in_lc) || (pVentInfo->is_humidity_check_needed_in_dc))
    {
        cur_humidity_value = SENSOR_Get(1, SENSOR_HUMIDITY);
    }

    if(pLight->is_light_cycle)
    {
        if(pVentInfo->is_temp_check_needed_in_lc)
        {
			if(cur_temp_value >= pVentInfo->lc_on_temp_threshold)
			{
                is_temp_activation = FALSE;
				mDeActivate();
			}
			if(cur_temp_value <= pVentInfo->lc_off_temp_threshold)
			{
                is_temp_activation = TRUE;
				mActivate();
			}

        }
        if(pVentInfo->is_humidity_check_needed_in_lc)
        {

            if(cur_humidity_value >= pVentInfo->lc_on_humidity_threshold)
            {
                is_humidity_activation = FALSE;
                mDeActivate();
            }
            if(cur_humidity_value <= pVentInfo->lc_off_humidity_threshold)
            {
                is_humidity_activation = TRUE;
                mActivate();
            }            
        }
    }
    else
    {
        if(pVentInfo->is_temp_check_needed_in_dc)
        {
			if(cur_temp_value >= pVentInfo->dc_on_temp_threshold)
			{
                is_temp_activation = FALSE;
				mDeActivate();
			}
			if(cur_temp_value <= pVentInfo->dc_off_temp_threshold)
			{
                is_temp_activation = TRUE;
				mActivate();
			}
        }
        if(pVentInfo->is_humidity_check_needed_in_dc)
        {
            if(cur_humidity_value >= pVentInfo->dc_on_humidity_threshold)
            {
                is_humidity_activation = FALSE;
                mDeActivate();
            }
            if(cur_humidity_value <= pVentInfo->dc_off_humidity_threshold)
            {
                is_humidity_activation = TRUE;
                mActivate();
            }            
        }       
    } 
    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);
    OSA_UnLockMutex(&run_mutex); 
}

void DEV_VENT::mStop()
{
    OSA_LockMutex(&run_mutex);

    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);
    DEV_OUTLET::mStop(); 
    is_rpt_activation = FALSE;
    is_temp_activation = FALSE;
    is_humidity_activation = FALSE;
    rpt_event = NONE;

    OSA_StopTimer(&rpt_cycle_timer);
    OSA_DeInitTimer(&rpt_cycle_timer);

    OSA_StopTimer(&sensing_timer);
    OSA_DeInitTimer(&sensing_timer);

    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);
    OSA_UnLockMutex(&run_mutex);
}

void DEV_VENT::mFaultHandler(BOOLEAN fault_occured)
{
    OSA_LockMutex(&run_mutex);
    DEV_OUTLET::mFaultHandler(fault_occured,pVentInfo->DevState);
    OSA_UnLockMutex(&run_mutex);
}

void DEV_VENT::mStateHandler()
{
    OSA_LockMutex(&run_mutex);
    DEV_OUTLET::mStateHandler(pVentInfo->DevState);
    OSA_UnLockMutex(&run_mutex);
}

U32 DEV_VENT::mGetSwitchOffTime()
{
	return switch_off_time;
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_vent_rpt_timer_expiry_handler(void *arg)
{
    DEV_VENT * Obj = (DEV_VENT*)arg;

    Device_Timer_Msg  dev_timer_msg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    dev_timer_msg.fp = device_vent_rpt_timer_expiry_handler_wrapper;

    dev_timer_msg.device_obj = (void*)Obj;

    APP_send_timer_expiry_msg((U8*)&dev_timer_msg,sizeof(dev_timer_msg));

    TRACE_INFO("%s Exit \n", __FUNCTION__);

}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_vent_threshold_timer_expiry_handler(void *arg)
{
    DEV_VENT * Obj = (DEV_VENT*)arg;

    Device_Timer_Msg  dev_timer_msg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    dev_timer_msg.fp = device_vent_threshold_timer_expiry_handler_wrapper;

    dev_timer_msg.device_obj = (void*)Obj;

    APP_send_timer_expiry_msg((U8*)&dev_timer_msg,sizeof(dev_timer_msg));

    TRACE_INFO("%s Exit \n", __FUNCTION__);


}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_vent_rpt_timer_expiry_handler_wrapper(void *arg)
{
    DEV_VENT * Obj = (DEV_VENT*)arg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    Obj->mRptTimerAction();

    TRACE_INFO("%s Exit \n", __FUNCTION__);

}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_vent_threshold_timer_expiry_handler_wrapper(void *arg)
{
    DEV_VENT * Obj = (DEV_VENT*)arg;

    TRACE_INFO("%s Entry  \n", __FUNCTION__);

    Obj->mThresholdTimerAction();

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}


/*******************************************************************************
*                          End of File
*******************************************************************************/
