/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : device.hpp
*
*  Description         : 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   7th Oct 2020     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef DEVICE_HPP
#define DEVICE_HPP

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "grow_config.h"
#include "osa.h"

/*******************************************************************************
*                          Version Control
*******************************************************************************/

/*******************************************************************************
*                          Referenced classes and types
*******************************************************************************/

/*******************************************************************************
*                          Types and constants
*******************************************************************************/
#define TOTAL_DAY_DURATION  (U32)86400   //24*60*60 

#define SENSING_INTERVAL  30  //30 seconds 

#define CO2_SENSING_INTERVAL 10     /* 10 seconds , sensor board takes out average of
                                  3 reading out of five readinsg leaving high & low  */ 

typedef enum
{
    START_TIMER = 1,
    ON_TIMER,
    OFF_TIMER,
    NONE
} RPT_TIME_EVENT;

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Class Definition
*******************************************************************************/
class DEV_OUTLET   //Base class for all six device outlets 
{
    private:

    protected:
        osa_timer_type_t action_timer;

        DEVICE_TYPE      dev_name;

    public:
        static GrowConfig_GrowCycleInfo*  pGrowInfo;  // common to all six devices to process
        static GrowConfig_GrowCycleStatus* pGrowStatus;
        static FUNC_PTR2         fpActivate;
        static FUNC_PTR2         fpDeActivate;
        BOOLEAN                  is_dev_on;
        DEV_OUTLET*              follow_list[6];
        DEV_OUTLET*              flip_list[6];
        osa_mutex_type_t         run_mutex;
        BOOLEAN                  cur_on_state;

	    DEV_OUTLET()
        {
            fpActivate = 0x00;
            fpDeActivate = 0x00;            
        }  
 
        virtual void mInit(DEVICE_TYPE name);

        void mSInit(GrowConfig_GrowCycleInfo*   pInfo,
                        GrowConfig_GrowCycleStatus* pStatus,
                        FUNC_PTR2 f1, 
                        FUNC_PTR2 f2);

        U32 mGetSec(mTIME* pTime);

        void mTimeConvert(U32 time_in_sec , U8* pStr);

        virtual void mStartTimer(U32 duration);
        virtual void mDoTimerAction();
        virtual void mStopTimer();

        virtual void mGetStatus(){;}

        virtual void mActivate(DEVICE_STATE  DevState);
        virtual void mDeActivate(DEVICE_STATE  DevState);

        virtual void mSubscribe(DEVICE_TYPE dev_name, DEVICE_OP_STATE dev_state,DEV_OUTLET* dev_obj);
        virtual void mClearList();

        virtual void mStop();

        virtual void mFaultHandler(BOOLEAN fault_occured,DEVICE_STATE  DevState);

        virtual void mStateHandler(DEVICE_STATE  DevState);
};


class DEV_LIGHT : public DEV_OUTLET 
{
    private:
        GrowConfig_DevLightInfo* pLightInfo;

    public:  
        U32 light_on_time;
        U32 light_off_time;
        U32 cur_time;
        BOOLEAN is_light_cycle;
        U32 rtc_alarm_no;
    
        DEV_LIGHT(): DEV_OUTLET() 
        {            
            pLightInfo = 0x00;
            is_light_cycle = FALSE;
        }
    void mInit(GrowConfig_DevLightInfo* pInfo , DEVICE_TYPE name);

    void mExecute();

    void mRun(); 

    void mStartTimer(U32 duration);
    void mDoTimerAction();
    void mStopTimer();

    void mGetStatus();

    void mActivate();
    void mDeActivate();

    void mClearList();
    void mSubscribe(DEVICE_TYPE dev_name, DEVICE_OP_STATE dev_state,DEV_OUTLET* dev_obj); 

    void mStop();

    void mFaultHandler(BOOLEAN fault_occured);
    void mStateHandler();
    void mStopAlarm();
};


class DEV_GEN : public DEV_OUTLET 
{
    private:
        GrowConfig_DevGenInfo* pDevInfo;
        BOOLEAN is_earlier_cycle_light;
        BOOLEAN is_earlier_cycle_dark;
        osa_timer_type_t rpt_cycle_timer;
        osa_timer_type_t sensing_timer;
        RPT_TIME_EVENT   rpt_event;
        BOOLEAN          is_start_timer;
        U32              previous_co2_val;
        osa_timer_type_t co2_cylinder_timer;
        U32              sensing_interval;

    public:       
        DEV_GEN(): DEV_OUTLET() 
        {            
            pDevInfo = 0x00;
            is_earlier_cycle_light = FALSE;
        }
    void mInit(GrowConfig_DevGenInfo* pInfo, DEVICE_TYPE name);

    void mRun(); 
    void mRunNative();
    void mHoodVentNative();
    void mCircNative();
    void mRunFlexi();
    void mRunRptMode();
    void mRunRptMode_helper(U32 sdelay, U32 on_time, U32 off_time);
    void mRptTimerAction();
    void mRptTimerAction_helper(U32 sdelay, U32 on_time, U32 off_time);
    void mThresholdTimerAction();

    void mStartTimer(U32 duration);
    void mDoTimerAction();
    void mStopTimer();

    void mGetStatus();

    void mActivate();
    void mDeActivate();

    void mClearList();
    void mSubscribe(DEVICE_TYPE dev_name, DEVICE_OP_STATE dev_state,DEV_OUTLET* dev_obj);
    void mCheck();

    void mStop();

    void mRunLeakMode(BOOLEAN leak_on);

    void mFaultHandler(BOOLEAN fault_occured);
    void mStateHandler();
    BOOLEAN  mCheckVentStatus();
};


class DEV_VENT : public DEV_OUTLET 
{
    private:
        GrowConfig_DevVentInfo* pVentInfo;
        osa_timer_type_t rpt_cycle_timer;
        osa_timer_type_t sensing_timer;
        RPT_TIME_EVENT   rpt_event;
        BOOLEAN is_rpt_activation;
        BOOLEAN is_temp_activation;
        BOOLEAN is_humidity_activation;
        U32 switch_off_time;

    public:       
        DEV_VENT(): DEV_OUTLET() 
        {            
            pVentInfo = 0x00;
        }
    void mInit(GrowConfig_DevVentInfo* pInfo , DEVICE_TYPE name);

    void mRun();
    void mRunRptMode_helper(U32 sdelay, U32 on_time, U32 off_time); 
    void mRptTimerAction();
    void mRptTimerAction_helper(U32 sdelay, U32 on_time, U32 off_time);
    void mThresholdTimerAction();

    void mStartTimer(U32 duration);
    void mDoTimerAction();
    void mStopTimer();

    void mGetStatus();

    void mActivate();
    void mDeActivate();

    void mClearList();
    void mSubscribe(); 

    void mStop();

    void mFaultHandler(BOOLEAN fault_occured);
    void mStateHandler();
    U32 mGetSwitchOffTime();

};


/*******************************************************************************
*                          Include inline definition file
*******************************************************************************/

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
