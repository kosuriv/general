/*******************************************************************************
* (C) Copyright 2022;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : sensor2.h
*
*  Description         : This has all declarations for the sensor  module.
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  9th Sep 2022      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef SENSOR2_H
#define SENSOR2_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
#define SENSOR2_SENSOR_AVAILABLE_FLAG   1   // 1: sensor present 0: sensor absent

typedef struct SENSOR_T 
{
    BOOLEAN is_altitude_set;
    BOOLEAN is_sensor_started ;

} SENSOR_T;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void SENSOR_Init(SENSOR_T*  pThis);

void SENSOR_Read(U16* co2, S32* temperature, S32* humidity);

void SENSOR_SetHeight(U16 height);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
