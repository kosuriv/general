/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : device_gen.cpp
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

/*******************************************************************************
*                          Helper functions
*******************************************************************************/
static void device_gen_rpt_timer_expiry_handler(void *arg);

static void device_gen_threshold_timer_expiry_handler(void *arg);

static void device_gen_rpt_timer_expiry_handler_wrapper(void *arg);

static void device_gen_threshold_timer_expiry_handler_wrapper(void *arg);

/*******************************************************************************
*                          Local data
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern DEV_OUTLET* DevList[];

extern DEV_VENT   Vent;  //used for CO2  mode 

extern int CO2_Calculate(int co2_error , int co2_rate);

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
void DEV_GEN::mInit(GrowConfig_DevGenInfo* pInfo , DEVICE_TYPE name)
{
    DEV_OUTLET::mInit(name);

    pDevInfo = pInfo;

	is_earlier_cycle_light =  FALSE; 
	is_earlier_cycle_dark =  FALSE; 
	rpt_event = NONE;
	is_start_timer = FALSE;
    previous_co2_val = 500;
	
}

void DEV_GEN::mRun()
{
    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];


    OSA_LockMutex(&run_mutex);

    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

    {
        switch ( pDevInfo->DevOpState)
        {
            case NATIVE:
                mRunNative();
                break;

            case FOLLOW:
			case FLIP:
             //   mRunFollowOrFlip(); earlier subscribed to the followed device , hence donot do anything
                break;
    
            case FLEXI:
                
                if (((pLight->is_light_cycle) && (pDevInfo->is_lc_on))  ||
                   ((!pLight->is_light_cycle) && (pDevInfo->is_dc_on)))
                {    
                    mRunFlexi();
                }
                break;
        }
    }
    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);

    OSA_UnLockMutex(&run_mutex);
}

void DEV_GEN::mRunNative()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    if(dev_name == DEVICE_HOOD_VENT)
    {
        mHoodVentNative();    
    }
	else if(dev_name == DEVICE_CIRC)
	{
        mCircNative(); 
	}

    TRACE_INFO("%s Exit \n", __FUNCTION__);

}

void DEV_GEN::mRunFlexi()
{
    BOOLEAN is_periodic;

   /*  light cycle  started/ended , profile changed ,
      or initial start of the system */
    TRACE_INFO("%s Entry \n", __FUNCTION__);

	switch ( pDevInfo->DevOpMode)
	{
		case RPTTIMER:
			mRunRptMode();
			break;

        case CO2:
		case HEATER:
		case COOLER:
		case HUMIDIFIER:
		case DEHUMIDIFIER:
			/* stop any previous timer of earlier cycle */
			OSA_StopTimer(&sensing_timer);
			OSA_DeInitTimer(&sensing_timer);
            if(pDevInfo->DevOpMode == CO2)
            {
                sensing_interval = CO2_SENSING_INTERVAL;
                is_periodic = TRUE;
            }
            else
            {
                sensing_interval =  SENSING_INTERVAL;
                is_periodic = TRUE;
            }
			OSA_InitTimer(&sensing_timer,sensing_interval*1000, is_periodic,device_gen_threshold_timer_expiry_handler, this);
			OSA_StartTimer(&sensing_timer);
			break;
        case LEAKMODE:
            /* Donot do any thing , ISR will take care of it */
            TRACE_INFO("LeakMode(%d) \n", dev_name);
            break;
	}
}

void DEV_GEN::mHoodVentNative()
{
 	U32 finish_time;

    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    TRACE_INFO("%s Entry \n", __FUNCTION__);

//	finish_time = mGetSec(&pDevInfo->FinishDelay);

    finish_time = mGetSec(&pGrowInfo->DevHoodVent_FinishDelay);

    if(pLight->is_light_cycle)
    {
        if(pDevInfo->is_lc_on)
        {
            mActivate();
            is_earlier_cycle_light =  TRUE;
        }
        else
        {
            mDeActivate();      
        }
    }
    else
    {
        if((is_earlier_cycle_light) && (dev_name == DEVICE_HOOD_VENT))
        {
            mStartTimer(finish_time);
        }
        else
        {
            //TRACE_INFO("%s Switch off  After Finish Delay \n", __FUNCTION__);
            mDeActivate();
        }
        is_earlier_cycle_light =  FALSE;
    }
}


void DEV_GEN::mCircNative()
{
 	U32 start_delay;

    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    TRACE_INFO("%s Entry \n", __FUNCTION__);

	mStopTimer();

    if(pLight->is_light_cycle)
    {
        if(is_earlier_cycle_dark)
        {
            TRACE_INFO("%s Switch Over To Light Cycle\n", __FUNCTION__);
            mDeActivate();
            is_start_timer = FALSE;
            is_earlier_cycle_dark =  FALSE;
        }

		if(pDevInfo->is_lc_on)
		{
			if(is_start_timer)
			{
LC_START:
                TRACE_INFO("%s Light Cycle Start Timer Expiry\n", __FUNCTION__);
				mActivate();
				is_start_timer = FALSE;
			}
			else
			{
                TRACE_INFO("%s Starting Light Cycle Start Timer\n", __FUNCTION__);
				start_delay = mGetSec(&pDevInfo->lc_start_delay);
                if(start_delay == 0x00)
                    goto LC_START;

				mStartTimer(start_delay);
				is_start_timer = TRUE;
			}
            is_earlier_cycle_light =  TRUE;			 
		}
		else
		{
            TRACE_INFO("%s Light Cycle switch off\n", __FUNCTION__);
			mDeActivate();
		}

    }
    else
    {
        if(is_earlier_cycle_light)
        {
            TRACE_INFO("%s Switch Over To dark Cycle\n", __FUNCTION__);
            mDeActivate();
            is_start_timer = FALSE;
            is_earlier_cycle_light =  FALSE;
        }
		if(pDevInfo->is_dc_on)
		{
			if(is_start_timer)
			{
DC_START:
                TRACE_INFO("%s Dark Cycle Start Timer Expiry\n", __FUNCTION__);
				mActivate();
				is_start_timer = FALSE;
			}
			else
			{
                TRACE_INFO("%s Starting Dark Cycle Start Timer\n", __FUNCTION__);
				start_delay = mGetSec(&pDevInfo->dc_start_delay);

                if(start_delay == 0x00)
                    goto DC_START;

				mStartTimer(start_delay);
				is_start_timer = TRUE;
			}
            is_earlier_cycle_dark = TRUE;
		}
		else
		{
            TRACE_INFO("%s Dark Cycle switch off\n", __FUNCTION__);
			mDeActivate();
		}
    }

    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);
}

void DEV_GEN::mRunRptMode()
{
	U32 timer_duration_sec;

    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    TRACE_INFO("%s Entry \n", __FUNCTION__);

	/* Stop any previous timers */
	OSA_StopTimer(&rpt_cycle_timer);
	OSA_DeInitTimer(&rpt_cycle_timer);

	if(pLight->is_light_cycle)
	{
        mRunRptMode_helper(mGetSec(&pDevInfo->lc_start_delay),mGetSec(&pDevInfo->lc_on_time), mGetSec(&pDevInfo->lc_off_time));
	}
	else 
	{	
        mRunRptMode_helper(mGetSec(&pDevInfo->dc_start_delay),mGetSec(&pDevInfo->dc_on_time), mGetSec(&pDevInfo->dc_off_time));
	}
}

void DEV_GEN::mRunRptMode_helper(U32 sdelay, U32 on_time, U32 off_time)
{

    if(sdelay)
    {
        rpt_event = START_TIMER;
        OSA_InitTimer(&rpt_cycle_timer,sdelay*1000,FALSE, device_gen_rpt_timer_expiry_handler, this);
        OSA_StartTimer(&rpt_cycle_timer);
    }
    else if(on_time)
    {
        mActivate();
        rpt_event = ON_TIMER;
        OSA_InitTimer(&rpt_cycle_timer,on_time*1000,FALSE, device_gen_rpt_timer_expiry_handler, this);
        OSA_StartTimer(&rpt_cycle_timer);
            
    }
    else if(off_time)
    {
        mDeActivate();
        rpt_event = OFF_TIMER;
        OSA_InitTimer(&rpt_cycle_timer,off_time*1000,FALSE, device_gen_rpt_timer_expiry_handler, this);
        OSA_StartTimer(&rpt_cycle_timer);
    }
    else
    {
        rpt_event = NONE;
    }  
}

void DEV_GEN::mStartTimer(U32 duration)
{
    DEV_OUTLET::mStartTimer(duration);
}

void DEV_GEN::mStopTimer()
{
	DEV_OUTLET::mStopTimer();
}

void DEV_GEN::mDoTimerAction()
{
    mStopTimer();
	mRun();   			
}

void DEV_GEN::mGetStatus()
{
  //  TRACE_DBG("%s Entry \n", __FUNCTION__);

	switch(dev_name)
    {

        case DEVICE_HOOD_VENT:
            pGrowStatus->dev_hood_vent_config = pDevInfo->DevState;
			pGrowStatus->dev_hood_vent_on = is_dev_on;
            break;

        case DEVICE_CIRC:
            pGrowStatus->dev_circ_config = pDevInfo->DevState;
			pGrowStatus->dev_circ_on = is_dev_on;
            break;

        case DEVICE_FXP1:
            pGrowStatus->dev_fxp1_config = pDevInfo->DevState;
			pGrowStatus->dev_fxp1_on = is_dev_on;
            break;

        case DEVICE_FXP2:
            pGrowStatus->dev_fxp2_config = pDevInfo->DevState;
			pGrowStatus->dev_fxp2_on = is_dev_on;
            break;
    }
}


void DEV_GEN::mActivate()
{
	U32 i;
    if(mCheckVentStatus())
	    DEV_OUTLET::mActivate(pDevInfo->DevState); 
}

void DEV_GEN::mDeActivate()
{
	U32 i;

	DEV_OUTLET::mDeActivate(pDevInfo->DevState); 
    switch(dev_name)
    {
        case DEVICE_HOOD_VENT:
            pGrowStatus->dev_hood_vent_counter++;
            break;

        case DEVICE_CIRC:
            pGrowStatus->dev_circ_counter++;;
            break;

        case DEVICE_FXP1:
            pGrowStatus->dev_fxp1_counter++;;;
            break;

        case DEVICE_FXP2:
            pGrowStatus->dev_fxp2_counter++;;;
            break;
    }
}

void DEV_GEN::mRptTimerAction()
{
	U32 timer_duration_sec;

    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    OSA_LockMutex(&run_mutex);

    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

	OSA_StopTimer(&rpt_cycle_timer);
    OSA_DeInitTimer(&rpt_cycle_timer);

    if(pLight->is_light_cycle)
    {
        mRptTimerAction_helper(mGetSec(&pDevInfo->lc_start_delay),mGetSec(&pDevInfo->lc_on_time), mGetSec(&pDevInfo->lc_off_time));
    }
    else
    {   
        mRptTimerAction_helper(mGetSec(&pDevInfo->dc_start_delay),mGetSec(&pDevInfo->dc_on_time), mGetSec(&pDevInfo->dc_off_time));
    }
    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);

    OSA_UnLockMutex(&run_mutex);
}

void DEV_GEN::mRptTimerAction_helper(U32 sdelay, U32 on_time, U32 off_time)
{
    switch(rpt_event)
    {
        case START_TIMER:
            if(on_time)
            {
                mActivate();
                rpt_event = ON_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,on_time*1000,FALSE, device_gen_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);
            }
            else if(off_time)
            {
                mDeActivate();
                rpt_event = OFF_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,off_time*1000,FALSE, device_gen_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);                
            }
            break;

        case ON_TIMER:
            if(off_time)
            {
                mDeActivate();
                rpt_event = OFF_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,off_time*1000,FALSE, device_gen_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);                
            }    
            break;

        case OFF_TIMER:
            if(on_time)
            {
                mActivate();
                rpt_event = ON_TIMER;
                OSA_InitTimer(&rpt_cycle_timer,on_time*1000,FALSE, device_gen_rpt_timer_expiry_handler, this);
                OSA_StartTimer(&rpt_cycle_timer);                
            }    
            break;
    }   
}

void DEV_GEN::mThresholdTimerAction()
{
	FLOAT cur_value;

	FLOAT upper_value_to_check;

	FLOAT lower_value_to_check;

    U8   time[50];

    U32  current_time;

    U32 ref_time;
	
	S32 co2_cylinder_on_time = 0;


    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    OSA_LockMutex(&run_mutex);

    TRACE_INFO("%s Entry \n", __FUNCTION__);

	if(pLight->is_light_cycle)
	{
		upper_value_to_check = pDevInfo->lc_upper_threshold;

		lower_value_to_check = pDevInfo->lc_lower_threshold;
	}
	else
	{
		if(pDevInfo->DevOpMode == CO2)
		{
			/* CO2 mode works only for light cycle */
			OSA_UnLockMutex(&run_mutex);
			return;			
		}
		upper_value_to_check = pDevInfo->dc_upper_threshold;

		lower_value_to_check = pDevInfo->dc_lower_threshold;		
	}	
	if ((pDevInfo->DevOpMode == HEATER) || (pDevInfo->DevOpMode == COOLER))
	{
		cur_value = SENSOR_Get(1, SENSOR_TEMPERATURE); 
	}
	else if(pDevInfo->DevOpMode == CO2) 
	{
        TRACE_INFO("CO2 Mode \n");

		cur_value = SENSOR_Get(1, SENSOR_CO2);
	}
    else
    {
        cur_value = SENSOR_Get(1, SENSOR_HUMIDITY);
    }

	switch ( pDevInfo->DevOpMode)
	{

		case HEATER:
		case HUMIDIFIER:
			if((cur_value >= upper_value_to_check) && ( upper_value_to_check))
			{
				mDeActivate();
			}
			if((cur_value <= lower_value_to_check) && ( lower_value_to_check)) 
			{
				mActivate();
			}
			break;
			
		case COOLER:
		case DEHUMIDIFIER:
			if((cur_value >= upper_value_to_check) && (upper_value_to_check))
			{
				mActivate();
			}
			if((cur_value <= lower_value_to_check)  && (lower_value_to_check))
			{
				mDeActivate();
			}
			break;

        case CO2: 
            TRACE_INFO("CO2 Mode  Check \n");
            if(pDevInfo->co2_src == GENERATOR)
            {
                TRACE_INFO("CO2 GENERATOR MODE   \n");
                if(cur_value >= upper_value_to_check)
                {
                    mDeActivate();
                }
                else if(cur_value <= lower_value_to_check) 
                {

                #if 1 
                    mActivate();
                #else 
                    //* check if whether Vent device is ON or not 
                    Vent.mGetStatus();
                    if(!pGrowStatus->dev_vent_on)
                    {
                        TRACE_INFO("CO2 Mode Device Vent is OFF    \n");
                        /*  check Vent outlet off time */
                        ref_time = mGetSec(&pGrowInfo->co2_vent_delay_time);

                        current_time = RTC_GetTime(time);

                        if( current_time - Vent.mGetSwitchOffTime() >= ref_time)
                        {
                            TRACE_INFO("CO2 is  on    \n");

                            /* Vent device is switched off long back */
                            mActivate();
                        }
                    }
                    else
                    {
                        TRACE_INFO("CO2 Mode Device Vent is ON \n");
                    }
                #endif
                }             
            }
            else
            {
                TRACE_INFO("CO2 CYLINDER MODE   \n");
				
				co2_cylinder_on_time = CO2_Calculate(cur_value-upper_value_to_check, previous_co2_val-cur_value);

                TRACE_INFO("Fuzzy Calculated Duration = %d \n", co2_cylinder_on_time);

                if( co2_cylinder_on_time > 0)
                {

                #if 1 
                    mActivate();
                #else
                    //* check if whether Vent device is ON or not 
                    Vent.mGetStatus();
                    if(!pGrowStatus->dev_vent_on)
                    {
                        TRACE_INFO("CO2 Mode Device Vent is OFF    \n");

                        /*  check Vent outlet off time */
                        ref_time = mGetSec(&pGrowInfo->co2_vent_delay_time);

                        current_time = RTC_GetTime(time);

                        if( current_time - Vent.mGetSwitchOffTime() >= ref_time)
                        {
                            TRACE_INFO("CO2 is  on    \n");
                            /* Vent device is switched off long back */
                            mActivate();
                        }                      
                    }
                    else
                    {
                        TRACE_INFO("CO2 Mode Device Vent is ON \n");
                    }
                #endif
                }
                else
                {
                    mDeActivate();
                }
            }
            TRACE_INFO("%s Exit \n", __FUNCTION__);
            break;
	}
    OSA_UnLockMutex(&run_mutex);
}

void DEV_GEN::mClearList()
{
	DEV_OUTLET::mClearList();   
}

void DEV_GEN::mSubscribe(DEVICE_TYPE dev_name, DEVICE_OP_STATE dev_state,DEV_OUTLET* dev_obj)
{
	DEV_OUTLET::mSubscribe(dev_name,dev_state,dev_obj);
}

void DEV_GEN::mCheck()
{
    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];
	if(pDevInfo->DevState == USUAL)
	{
		if( (pDevInfo->DevOpState == FOLLOW) || (pDevInfo->DevOpState == FLIP))
		{
            if (((pLight->is_light_cycle) && (pDevInfo->is_lc_on))  ||
                ((!pLight->is_light_cycle) && (pDevInfo->is_dc_on)))
            { 
			    DevList[pDevInfo->ToFollowDevice]->mSubscribe(dev_name,pDevInfo->DevOpState,this);
            }
		}
	}
}

void DEV_GEN::mStop()
{
    OSA_LockMutex(&run_mutex);
    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

    DEV_OUTLET::mStop();

	is_earlier_cycle_light =  FALSE; 
	is_earlier_cycle_dark =  FALSE;
    previous_co2_val  = 0x00; 

	if(is_start_timer)
	{
		mStopTimer();
		is_start_timer = FALSE; 
	}

    rpt_event = NONE;

	OSA_StopTimer(&rpt_cycle_timer);
    OSA_DeInitTimer(&rpt_cycle_timer);

	OSA_StopTimer(&sensing_timer);
    OSA_DeInitTimer(&sensing_timer);

    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,dev_name);
    OSA_UnLockMutex(&run_mutex);
}

void DEV_GEN::mRunLeakMode(BOOLEAN leak_on)
{
    TRACE_INFO("%s Entry(%d) \n", __FUNCTION__,dev_name);

    if( (pDevInfo->DevOpState == FLEXI) && (pDevInfo->DevOpMode == LEAKMODE))
    {
        if(leak_on)
        {
            mActivate();
        }
        else
        {
            mDeActivate();
        }
    }
    
}
void DEV_GEN::mFaultHandler(BOOLEAN fault_occured)
{
    OSA_LockMutex(&run_mutex);
    DEV_OUTLET::mFaultHandler(fault_occured, pDevInfo->DevState);
    OSA_UnLockMutex(&run_mutex);
}

void DEV_GEN::mStateHandler()
{
    OSA_LockMutex(&run_mutex);
    DEV_OUTLET::mStateHandler(pDevInfo->DevState);
    OSA_UnLockMutex(&run_mutex);
}

BOOLEAN DEV_GEN:: mCheckVentStatus()
{
    BOOLEAN status = TRUE;
    DEV_LIGHT* pLight = (DEV_LIGHT*)DevList[DEVICE_LIGHT];

    U32 ref_time;
    U32  current_time;
    U8   time[50];

    if(((pDevInfo->is_vent_lockout) ||  (pDevInfo->DevOpMode == CO2)) && (pDevInfo->DevOpState == FLEXI))
    {
        //* check if whether Vent device is ON or not 
        Vent.mGetStatus();
        if(!pGrowStatus->dev_vent_on)
        {
            TRACE_INFO("Device Vent is OFF\n");

            /*  check Vent outlet off time */
            ref_time = mGetSec(&pGrowInfo->co2_vent_delay_time);

            current_time = RTC_GetTime(time);

            if( current_time - Vent.mGetSwitchOffTime() >= ref_time)
            {
                TRACE_INFO(" Device Vent is OFF  more than stipulated time \n");
                status = TRUE;
            }
            else
            {
                status = FALSE;
            }
        }
        else
        {

            status = FALSE;          

        }
    }
    return status;

}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_gen_rpt_timer_expiry_handler(void *arg)
{
	DEV_GEN * Obj = (DEV_GEN*)arg;

    Device_Timer_Msg  dev_timer_msg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    dev_timer_msg.fp = device_gen_rpt_timer_expiry_handler_wrapper;

    dev_timer_msg.device_obj = (void*)Obj;

    APP_send_timer_expiry_msg((U8*)&dev_timer_msg,sizeof(dev_timer_msg));

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_gen_threshold_timer_expiry_handler(void *arg)
{
    DEV_GEN * Obj = (DEV_GEN*)arg;

    Device_Timer_Msg  dev_timer_msg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    dev_timer_msg.fp = device_gen_threshold_timer_expiry_handler_wrapper;

    dev_timer_msg.device_obj = (void*)Obj;

    APP_send_timer_expiry_msg((U8*)&dev_timer_msg,sizeof(dev_timer_msg));

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_gen_rpt_timer_expiry_handler_wrapper(void *arg)
{
    DEV_GEN * Obj = (DEV_GEN*)arg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    Obj->mRptTimerAction();
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_gen_threshold_timer_expiry_handler_wrapper(void *arg)
{
    DEV_GEN * Obj = (DEV_GEN*)arg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    Obj->mThresholdTimerAction();

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
