/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : app_setup.h
*
*  Description         : This has all declarations common for all modules
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  4th Sep 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef APP_SETUP_H
#define APP_SETUP_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
/* The following are threads & related parameters */
#define APP_ESP_WIFI_RX_THREAD_STACK_SIZE 512

#define APP_MQTT_RX_THREAD_STACK_SIZE  512

#define APP_ESP_WIFI_RX_THREAD_PRIORITY 9  //Timer task has highest priority i.e. 9

#define APP_MQTT_RX_THREAD_PRIORITY 7

//#define APP_STRAT_THREAD_PRIORITY 5

#define APP_ACTION_THREAD_PRIORITY 4

#define APP_ACTION_THREAD_STACK_SIZE  5000 //3584  

#define APP_MSGQ_NO_OF_MESSAGES  30

#define APP_MAX_SIZE_PKT 2048 

#define APP_WEB_ACTION_THREAD_PRIORITY 5

#define APP_WEB_ACTION_THREAD_STACK_SIZE  4000  

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
