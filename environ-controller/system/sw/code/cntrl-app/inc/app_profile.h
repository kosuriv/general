/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : app_profile.h
*
*  Description         : This has all declarations for all profile request
                         functinality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  22nd  Oct 2020    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef APP_PROFILE_H
#define APP_PROFILE_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "app.h"
#include "jsmn.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
void App_mqtt_process_set_profile_request(U8*        pBuf,
                                          jsmntok_t* pTok,
                                          U32        token_count);

void App_mqtt_process_apply_profile_request(U8*        pBuf,
                                           jsmntok_t* pTok,
                                           U32        token_count);

void App_mqtt_process_get_profile_request(U8*        pBuf,
                                          jsmntok_t* pTok,
                                          U32        token_count);

BOOLEAN App_check_to_apply_profile_now_or_not();

void App_mqtt_process_erase_profile_request(U8*        pBuf,
                                           jsmntok_t* pTok,
                                           U32        token_count);

U32 App_fill_common_json_header(U8* in_rsp, APP_MQTT_CMD_TYPE cmd);




/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
