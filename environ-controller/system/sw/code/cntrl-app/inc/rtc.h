/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : rtc.h
*
*  Description         : This file defines RTC function declarations 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  22nd Sep 2020     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef RTC_H
#define RTC_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/

#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rtc.h"
#endif

#include "data_types.h"
#include "profile_data_types.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type Declarations
*******************************************************************************/

typedef struct Rtc_Alarm
{
    CALLBACK32         callback;
    BOOLEAN            is_used;      /* 0 means free ,1:currently in use */   
                                           
} Rtc_Alarm;

typedef struct RTC_T
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    RTC_HandleTypeDef hrtc;
#endif

    Rtc_Alarm Alarm[2];

} RTC_T;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/

void RTC_Init(RTC_T *pThis);

void RTC_SetTime(U8* cur_time);

U32 RTC_GetTime(U8* get_time);

void RTC_GetDateTime(mDATE* pDate,  mTIME* pTime , U8* DateStr , U8* TimeStr);

S32 RTC_StartAlarm(mTIME* time ,mDATE* date, CALLBACK32_FCT pFunction, void* pArg);

#ifdef NUCLEO_F439ZI_EVK_BOARD
void RTC_AlarmISR();
#endif

#ifdef ESP32_S2_MINI
void IRAM_ATTR RTC_AlarmISR(void *pThis);
#endif

void RTC_StopAlarm(S32 alarm_no);



#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
