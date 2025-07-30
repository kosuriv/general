/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : device_wrapper.c
*
*  Description         : This file is stub file to test UART Tx/Rx functionality 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  26th Aug 2020     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "device_wrapper.h"
#include "device.hpp"

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
DEV_LIGHT  Light;

DEV_GEN    HoodVent;

DEV_GEN    Circ;

DEV_VENT   Vent;

DEV_GEN    Fxp1;

DEV_GEN    Fxp2;

DEV_OUTLET* DevList[] = { &Light, &HoodVent, &Circ, &Fxp1, &Fxp2, &Vent };

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description:  
* Remarks    : 
*******************************************************************************/
void DEVICE_Init(GrowConfig_GrowCycleInfo*    pGrowConfig,
                 GrowConfig_GrowCycleStatus*  pGrowStatus,
                 GrowConfig_DevLightInfo*     pLightConfig,
                 GrowConfig_DevGenInfo*       pHoodVentConfig,
                 GrowConfig_DevGenInfo*       pCircConfig,
                 GrowConfig_DevVentInfo*      pVentConfig,
                 GrowConfig_DevGenInfo*       pFxp1Config,
                 GrowConfig_DevGenInfo*       pFxp2Config,
                 FUNC_PTR2                    Activate,
                 FUNC_PTR2                    DeActivate)
{

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    Light.mSInit(pGrowConfig, pGrowStatus,Activate,DeActivate);

    Light.mInit(pLightConfig,DEVICE_LIGHT);

    HoodVent.mInit(pHoodVentConfig,DEVICE_HOOD_VENT);

    Circ.mInit(pCircConfig,DEVICE_CIRC);

    Fxp1.mInit(pFxp1Config,DEVICE_FXP1);

    Fxp2.mInit(pFxp2Config,DEVICE_FXP2);

    Vent.mInit(pVentConfig,DEVICE_VENT);

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_Execute()
{
	TRACE_INFO("%s Entry \n", __FUNCTION__);

	/* Clear the Follow & Flip device List */
	Light.mClearList();
	HoodVent.mClearList();
	Circ.mClearList();
	Fxp1.mClearList();
	Fxp2.mClearList();
	Vent.mClearList();

    /* check & subscribe to FOLLW-ED/FLIP-ED Device */
	HoodVent.mCheck();
	Circ.mCheck();
	Fxp1.mCheck();
	Fxp2.mCheck();

	/* Start Execution of Light Cycle */
    Light.mExecute();
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_GetStatusAll(void)
{
	Light.mGetStatus();
	HoodVent.mGetStatus();
    Circ.mGetStatus();
    Fxp1.mGetStatus();
    Fxp2.mGetStatus();
    Vent.mGetStatus();
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_Stop()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* Clear the Follow & Flip device List */
    Light.mClearList();
    HoodVent.mClearList();
    Circ.mClearList();
    Fxp1.mClearList();
    Fxp2.mClearList();
    Vent.mClearList();

    Light.mStop();
    HoodVent.mStop();
    Circ.mStop();
    Fxp1.mStop();
    Fxp2.mStop();
    Vent.mStop();

}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_LightCycleExecute()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* Start Execution of Light Cycle */
    Light.mExecute();

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_LeakModeExecute(BOOLEAN leak_on)
{

    HoodVent.mRunLeakMode(leak_on);
    Circ.mRunLeakMode(leak_on);
    Fxp1.mRunLeakMode(leak_on);
    Fxp2.mRunLeakMode(leak_on);

}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_FaultHandler(BOOLEAN fault_occured)
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    Light.mFaultHandler(fault_occured);
    HoodVent.mFaultHandler(fault_occured);
    Circ.mFaultHandler(fault_occured);
    Fxp1.mFaultHandler(fault_occured);
    Fxp2.mFaultHandler(fault_occured);
    Vent.mFaultHandler(fault_occured);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_StateHandler(DEVICE_TYPE device)
{
   // DevList[device]->mStateHandler();

    switch(device)
    {
        case DEVICE_LIGHT:
            Light.mStateHandler();
            break;

        case DEVICE_HOOD_VENT:
            HoodVent.mStateHandler();
            break;

        case DEVICE_CIRC:
            Circ.mStateHandler();
            break;

        case DEVICE_FXP1:
            Fxp1.mStateHandler();
            break;
        case DEVICE_FXP2:
            Fxp2.mStateHandler();
            break;

        case DEVICE_VENT:
            Vent.mStateHandler();
            break;        
    }  
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DEVICE_StopAlarm(DEVICE_TYPE device)
{
    Light.mStopAlarm();
}

/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/

#ifdef __cplusplus
}
#endif
/*******************************************************************************
*                          End of File
*******************************************************************************/
