/*******************************************************************************
* (C) Copyright 2022;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : modbus_master.h
*
*  Description         : This file defines MODBUS declarations 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  21st Sep 2022     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef MODBUS_SLAVE_H
#define MODBUS_SLAVE_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "osa.h"


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

typedef struct MODBUS_SLAVE
{
    osa_thread_handle modbus_slave_thread;

    U16 height;

    U16 sensor_value[3];

} MODBUS_SLAVE;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void MODBUS_Init(MODBUS_SLAVE* pThis, U32 slave_addr);



#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
