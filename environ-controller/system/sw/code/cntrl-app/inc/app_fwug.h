/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : app_fwug.h
*
*  Description         : This has all declarations for all firmware upgrade 
                         functinality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  15th Jan 2021     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef APP_FWUG_H
#define APP_FWUG_H

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
void App_mqtt_process_fwug_request(U8*        pBuf,
                                   jsmntok_t* pTok,
                                   U32        token_count);

void App_process_fwug_timer_expiry();

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
