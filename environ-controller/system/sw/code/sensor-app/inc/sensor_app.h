/*******************************************************************************
* (C) Copyright 2022;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : sensor_app.h
*
*  Description         : This has all declarations for the sensor  applictaion.
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  9th Sep 2022      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef SENSOR_APP_H
#define SENSOR_APP_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "osa.h"
#include "sensor2.h"
#include "modbus_slave.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

#define SENSOR_APP_COMPARE_LENGTH(s)  sizeof(s)-1

#define SENSOR_APP_VERSION "1.0.0"

#define APP_TOKEN_MAX_COUNT  300

//messages  for app action thread 
typedef enum SENSOR_APP_MSG_TYPE
{
    SENSOR_APP_GET_SENSOR_MSG

}SENSOR_APP_MSG_TYPE;


/*  message format for app action thread */
typedef struct Sensor_App_Msg_Req
{
    SENSOR_APP_MSG_TYPE  type;
    U32   len;
    void* buf;
} Sensor_App_Msg_Req;


typedef struct SENSOR_APP
{
    osa_thread_handle action_thread;

    osa_msgq_type_t pActionMsgQ;

    osa_mutex_type_t msg_q_mutex;

    SENSOR_T  Sensor;

    MODBUS_SLAVE   ModbusSlave;

} SENSOR_APP;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void SENSOR_APP_Start();



#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
