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
#include <string.h>
#include "device_wrapper.h"
#include "device.hpp"
#include "trace.h"
#include "rtc.h"

/*******************************************************************************
*                          Helper Types and constants
*******************************************************************************/

/*******************************************************************************
*                          Helper functions
*******************************************************************************/
static void device_action_timer_handler(void *arg);

static void device_action_timer_handler_wrapper(void *arg);

/*******************************************************************************
*                          Local data
*******************************************************************************/


/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern DEV_LIGHT  Light;

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

                               /* Base Class Functions  */

GrowConfig_GrowCycleInfo* DEV_OUTLET::pGrowInfo{0x00};

GrowConfig_GrowCycleStatus* DEV_OUTLET::pGrowStatus{0x00};

FUNC_PTR2 DEV_OUTLET::fpActivate{0x00};

FUNC_PTR2 DEV_OUTLET::fpDeActivate{0x00};

void DEV_OUTLET::mInit(DEVICE_TYPE name)
{
    dev_name = name;

    OSA_InitMutex(&run_mutex) ;
} 

void DEV_OUTLET::mSInit(GrowConfig_GrowCycleInfo*   pInfo,
                        GrowConfig_GrowCycleStatus* pStatus,
                        FUNC_PTR2 f1, 
                        FUNC_PTR2 f2)
{
    pGrowInfo = pInfo;
    pGrowStatus = pStatus;
    fpActivate = f1;
    fpDeActivate = f2;
    
}

U32 DEV_OUTLET::mGetSec(mTIME* pTime)
{    
    return (pTime->HH*3600 + pTime->MM*60 + pTime->SS);             
}

void  DEV_OUTLET::mTimeConvert(U32 time_in_sec , U8* pStr)
{
    mTIME t;

    U32  value_left;

    t.HH = time_in_sec / 3600 ;

    value_left = time_in_sec  % 3600;

    t.MM = value_left / 60;
 
    t.SS = value_left %  60 ;

    sprintf((char*)pStr,"%d:%d:%d", t.HH, t.MM ,t.SS);
}

void DEV_OUTLET::mStartTimer(U32 duration)
{
    TRACE_INFO("%s Entry (%d) \n", __FUNCTION__,duration);

    /* all the timers are mostly aperiodic */
    OSA_InitTimer(&action_timer,duration*1000, FALSE, device_action_timer_handler, this);

    OSA_StartTimer(&action_timer);
}

void DEV_OUTLET::mStopTimer()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    OSA_StopTimer(&action_timer);

    OSA_DeInitTimer(&action_timer);

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

void DEV_OUTLET::mDoTimerAction()
{
    mStopTimer();    
}

void DEV_OUTLET::mActivate(DEVICE_STATE  DevState)
{
    U32 i;

    cur_on_state = TRUE;
    if((!pGrowStatus->ac_power_fault) &&
       (!pGrowStatus->rs485_fuse_fault) &&
       (!pGrowStatus->panic_switch_on) &&
       (DevState != SUSPENDED))
    {
        fpActivate(dev_name);
	    is_dev_on = TRUE;  
    }
#if 1
    for(i=1; i<5 ; i++)
    {
        if( follow_list[i] != 0x00)
        {
            ((DEV_GEN*)follow_list[i])->mActivate();
        }
        if( flip_list[i] != 0x00)
        {
            ((DEV_GEN*)flip_list[i])->mDeActivate();
        }
    }
#endif
}

void DEV_OUTLET::mDeActivate(DEVICE_STATE  DevState)
{
    U32 i;

    cur_on_state = FALSE;

    if( DevState != FORCED)
    {
        fpDeActivate(dev_name);
	    is_dev_on = FALSE;  
    }
#if 1
    for(i=1; i<5 ; i++)
    {
        if( follow_list[i] != 0x00)
        {
             ((DEV_GEN*)follow_list[i])->mDeActivate();
        }
        if( flip_list[i] != 0x00)
        {
             ((DEV_GEN*)flip_list[i])->mActivate();
        }
    } 
#endif
}


void DEV_OUTLET::mSubscribe(DEVICE_TYPE dev_name, DEVICE_OP_STATE dev_state,DEV_OUTLET* dev_obj)
{
	if( dev_state == FOLLOW)
	{
		follow_list[dev_name] = dev_obj;
	}
	else
	{
		flip_list[dev_name] = dev_obj;
	}
}

void DEV_OUTLET::mClearList()
{
	memset((U8*)follow_list,0x00,sizeof(follow_list));
	memset((U8*)flip_list,0x00,sizeof(flip_list));
}

void DEV_OUTLET::mStop()
{
    fpDeActivate(dev_name);
    is_dev_on = FALSE;
    cur_on_state = FALSE;
}

void DEV_OUTLET::mFaultHandler(BOOLEAN fault_occured, DEVICE_STATE  DevState)
{
    if(fault_occured)
    {
        /* some new fault, just deactivate  */
        fpDeActivate(dev_name);
        is_dev_on = FALSE;
    }
    else
    {
        /* check all faults cleared and device to be on at current time */
        if((!pGrowStatus->ac_power_fault) &&
           ((!pGrowStatus->hot_restrike_fault && (dev_name == DEVICE_LIGHT)) || (dev_name != DEVICE_LIGHT)) &&
           (!pGrowStatus->rs485_fuse_fault) &&
           (!pGrowStatus->panic_switch_on) &&
           (cur_on_state) && 
           (DevState != SUSPENDED))
        {
            fpActivate(dev_name);
            is_dev_on = TRUE;            
        }
        else
        {
            fpDeActivate(dev_name);
            is_dev_on = FALSE;
        }
    }
}

void DEV_OUTLET::mStateHandler(DEVICE_STATE  DevState)
{
    if(DevState == SUSPENDED)
    {
        fpDeActivate(dev_name);
        is_dev_on = FALSE;  
    }
    else if(DevState == FORCED)
    {
		/* check if there are no power faults */
 		if((!pGrowStatus->ac_power_fault) &&
           ((!pGrowStatus->hot_restrike_fault && (dev_name == DEVICE_LIGHT)) || (dev_name != DEVICE_LIGHT)) &&
           (!pGrowStatus->rs485_fuse_fault) &&
           (!pGrowStatus->panic_switch_on) )
		{
        	fpActivate(dev_name);
        	is_dev_on = TRUE;  
		}
    }
    else
    {
         /* check all faults cleared and device to be on at current time as per its normal state */
        if((!pGrowStatus->ac_power_fault) &&
           ((!pGrowStatus->hot_restrike_fault && (dev_name == DEVICE_LIGHT)) || (dev_name != DEVICE_LIGHT)) &&
           (!pGrowStatus->rs485_fuse_fault) &&
           (!pGrowStatus->panic_switch_on) &&
           (cur_on_state))
        {
            fpActivate(dev_name);
            is_dev_on = TRUE;            
        }
        else
        {
            fpDeActivate(dev_name);
            is_dev_on = FALSE;
        }
    }
}



/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_action_timer_handler(void *arg)
{
	DEV_OUTLET* Obj = (DEV_OUTLET*)arg;

    Device_Timer_Msg  dev_timer_msg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    dev_timer_msg.fp = device_action_timer_handler_wrapper;

    dev_timer_msg.device_obj = (void*)Obj;

    APP_send_timer_expiry_msg((U8*)&dev_timer_msg,sizeof(dev_timer_msg));

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void device_action_timer_handler_wrapper(void *arg)
{
    DEV_OUTLET* Obj = (DEV_OUTLET*)arg;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    if((DEV_LIGHT*)Obj == (DEV_LIGHT*)DevList[DEVICE_LIGHT])
    {
        DevList[DEVICE_LIGHT]->mDoTimerAction();
    }
    else if((DEV_GEN*)Obj == (DEV_GEN*) DevList[DEVICE_HOOD_VENT])
    {
        DevList[DEVICE_HOOD_VENT]->mDoTimerAction();
    }
    else if((DEV_GEN*)Obj == (DEV_GEN*)DevList[DEVICE_CIRC])
    {
        DevList[DEVICE_CIRC]->mDoTimerAction();
    }
    else if((DEV_VENT*)Obj == (DEV_VENT*)DevList[DEVICE_VENT])
    {
        DevList[DEVICE_VENT]->mDoTimerAction();
    }
    else if((DEV_GEN*)Obj == (DEV_GEN*)DevList[DEVICE_FXP1])
    {
        DevList[DEVICE_FXP1]->mDoTimerAction();
    }
    else if((DEV_GEN*)Obj == (DEV_GEN*)DevList[DEVICE_FXP2])
    {
        DevList[DEVICE_FXP2]->mDoTimerAction();
    }
    TRACE_INFO("%s Exit \n", __FUNCTION__);
}


/*******************************************************************************
*                          End of File
*******************************************************************************/
