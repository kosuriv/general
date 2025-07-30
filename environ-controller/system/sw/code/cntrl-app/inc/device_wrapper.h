/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : dev_wrapper.h
*
*  Description         : This has all declarations for all device outlets
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  7th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef DEV_WRAPPER_H
#define DEV_WRAPPER_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "grow_config.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
typedef struct Device_Timer_Msg
{
    Callback_1_Arg fp;
    void* device_obj;
} Device_Timer_Msg;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void DEVICE_Init(GrowConfig_GrowCycleInfo*    pGrowInfo,
                 GrowConfig_GrowCycleStatus*  pGrowStatus,
                 GrowConfig_DevLightInfo*     pLightConfig,
                 GrowConfig_DevGenInfo*       pHoodVentConfig,
                 GrowConfig_DevGenInfo*       pCircConfig,
                 GrowConfig_DevVentInfo*      pVentConfig,
                 GrowConfig_DevGenInfo*       pFxp1Config,
                 GrowConfig_DevGenInfo*       pFxp2Config,
                 FUNC_PTR2                     Activate,
                 FUNC_PTR2                     DeActivate);

void DEVICE_Execute(); 

void DEVICE_GetStatusAll(void); 

void DEVICE_Stop();

void DEVICE_LightCycleExecute();

void DEVICE_LeakModeExecute(BOOLEAN leak_on);

void DEVICE_FaultHandler(BOOLEAN fault_occured);

void DEVICE_StateHandler(DEVICE_TYPE device);

void DEVICE_StopAlarm(DEVICE_TYPE device);

#ifdef __cplusplus
}
#endif

#endif
/*******************************************************************************
*                          End of File
*******************************************************************************/
