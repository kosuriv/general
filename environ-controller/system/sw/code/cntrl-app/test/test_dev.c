/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : test_uart.c
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
#include "data_types.h"
#include "profile_data_types.h"
#include "trace.h"
#include "app.h"
#include "rtc.h"
#include "jsmn.h"
#include "utility.h"


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
static void test_dev_TimeConvert(U32 time_in_sec , mTIME* t);

static void test_dev_RtcAlarmCallback(void* Arg , U32 flag);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/

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
void TEST_DEV_Light(APP* pApp)
{
    U8 time[50];

    U32 cur_time;

    cur_time = RTC_GetTime(time);
#if 1

    test_dev_TimeConvert (cur_time, &pApp->WorkingProfile.DevLightConfig.Light_OnTime);

        /* Add three minute */
    test_dev_TimeConvert (cur_time + 180, &pApp->WorkingProfile.DevLightConfig.Light_OffTime);
#else
    test_dev_TimeConvert (cur_time, &pApp->WorkingProfile.DevLightConfig.Light_OffTime);

        /* Add three minute */
    test_dev_TimeConvert (cur_time + 30, &pApp->WorkingProfile.DevLightConfig.Light_OnTime);

#endif

#if 0
    pApp->WorkingProfile.DevHoodVentConfig.is_native_mode_lc_on = TRUE;

    pApp->WorkingProfile.DevHoodVentConfig.FinishDelay.HH= 00;
    pApp->WorkingProfile.DevHoodVentConfig.FinishDelay.MM= 01;
    pApp->WorkingProfile.DevHoodVentConfig.FinishDelay.SS= 00;
#endif
    
    pApp->WorkingProfile.DevCircConfig.is_native_mode_lc_on = TRUE;
    pApp->WorkingProfile.DevCircConfig.is_native_mode_dc_on = TRUE;
    pApp->WorkingProfile.DevCircConfig.lc_start_delay.HH= 00;
    pApp->WorkingProfile.DevCircConfig.lc_start_delay.MM= 01;
    pApp->WorkingProfile.DevCircConfig.lc_start_delay.SS= 00;
    pApp->WorkingProfile.DevCircConfig.dc_start_delay.HH= 00;
    pApp->WorkingProfile.DevCircConfig.dc_start_delay.MM= 01;
    pApp->WorkingProfile.DevCircConfig.dc_start_delay.SS= 00;

    pApp->WorkingProfile.DevFxp1Config.lc_start_delay.HH= 00;
    pApp->WorkingProfile.DevFxp1Config.lc_start_delay.MM= 01;
    pApp->WorkingProfile.DevFxp1Config.lc_start_delay.SS= 00;

    pApp->WorkingProfile.DevFxp1Config.lc_on_time.HH= 00;
    pApp->WorkingProfile.DevFxp1Config.lc_on_time.MM= 02;
    pApp->WorkingProfile.DevFxp1Config.lc_on_time.SS= 00;

    pApp->WorkingProfile.DevFxp1Config.lc_off_time.HH= 00;
    pApp->WorkingProfile.DevFxp1Config.lc_off_time.MM= 02;
    pApp->WorkingProfile.DevFxp1Config.lc_off_time.SS= 00;

    pApp->WorkingProfile.DevFxp1Config.dc_start_delay.HH= 00;
    pApp->WorkingProfile.DevFxp1Config.dc_start_delay.MM= 01;
    pApp->WorkingProfile.DevFxp1Config.dc_start_delay.SS= 00;

    pApp->WorkingProfile.DevFxp1Config.dc_on_time.HH= 00;
    pApp->WorkingProfile.DevFxp1Config.dc_on_time.MM= 02;
    pApp->WorkingProfile.DevFxp1Config.dc_on_time.SS= 00;

    pApp->WorkingProfile.DevFxp1Config.dc_off_time.HH= 00;
    pApp->WorkingProfile.DevFxp1Config.dc_off_time.MM= 02;
    pApp->WorkingProfile.DevFxp1Config.dc_off_time.SS= 00;

#if 0
    pApp->WorkingProfile.DevFxp2Config.DevOpState = FLEXI;


    pApp->WorkingProfile.DevVentConfig.lc_start_delay.HH= 00;
    pApp->WorkingProfile.DevVentConfig.lc_start_delay.MM= 01;
    pApp->WorkingProfile.DevVentConfig.lc_start_delay.SS= 00;
    pApp->WorkingProfile.DevVentConfig.dc_start_delay.HH= 00;
    pApp->WorkingProfile.DevVentConfig.dc_start_delay.MM= 02;
    pApp->WorkingProfile.DevVentConfig.dc_start_delay.SS= 00;
#endif


    pApp->WorkingProfile.DevVentConfig.lc_on_time.HH= 00;
    pApp->WorkingProfile.DevVentConfig.lc_on_time.MM= 01;
    pApp->WorkingProfile.DevVentConfig.lc_on_time.SS= 00;
    pApp->WorkingProfile.DevVentConfig.dc_on_time.HH= 00;
    pApp->WorkingProfile.DevVentConfig.dc_on_time.MM= 01;
    pApp->WorkingProfile.DevVentConfig.dc_on_time.SS= 00;
    pApp->WorkingProfile.DevVentConfig.lc_off_time.HH= 00;
    pApp->WorkingProfile.DevVentConfig.lc_off_time.MM= 01;
    pApp->WorkingProfile.DevVentConfig.lc_off_time.SS= 00;
    pApp->WorkingProfile.DevVentConfig.dc_off_time.HH= 00;
    pApp->WorkingProfile.DevVentConfig.dc_off_time.MM= 01;
    pApp->WorkingProfile.DevVentConfig.dc_off_time.SS= 00;



}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_DEV_RtcAlarm()
{
    U8 time[50];

    U32 cur_time;

    mTIME t;

    cur_time = RTC_GetTime(time); 

    TRACE_INFO("%s() Entry = %s \n", __FUNCTION__,time);   

    test_dev_TimeConvert(cur_time + 60 ,&t);

    RTC_StartAlarm(&t,NULL,test_dev_RtcAlarmCallback, NULL);

    test_dev_TimeConvert(cur_time + 120 ,&t);

    RTC_StartAlarm(&t,NULL,test_dev_RtcAlarmCallback, NULL);
}


    U8 buf[200];

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_Json()
{
  /* process the message */

    jsmn_parser j;

    jsmntok_t tok[20];

    S32 token_count; 

    U32 i;


    sprintf(buf,"{\"f1\":34,\"g1\":[{\"g2\":\"venu\",\"g3\":\"here\"}]}");

    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)buf, buf, tok, 20);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    for ( i=0; i<token_count; i++)
    {
       printf("Type = %d\n", tok[i].type);

       printf("Start = %d\n", tok[i].start);

       printf("End = %d\n", tok[i].end);

       printf("Size = %d\n", tok[i].size);

    //   printf("Parent = %d\n", tok[i].parent);

       UTILITY_PrintBuffer(buf+tok[i].start,tok[i].end-tok[i].start,1);


       printf("\n\n");
    }    

}


/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_dev_TimeConvert(U32 time_in_sec , mTIME* t)
{

    U32  value_left;

    t->HH = time_in_sec / 3600 ;

    value_left = time_in_sec  % 3600;

    t->MM = value_left / 60;
 
    t->SS = value_left %  60 ;

    TRACE_INFO("TIME  Converted = %d:%d:%d\n", t->HH,t->MM, t->SS);

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_dev_RtcAlarmCallback(void* Arg , U32 flag)
{
   U8 time[50];

    U32 cur_time;

    mTIME t;

    cur_time = RTC_GetTime(time);
  
    TRACE_INFO("Alaram Callback(%d) = %s \n", flag,time);
}


/*******************************************************************************
*                          End of File
*******************************************************************************/
