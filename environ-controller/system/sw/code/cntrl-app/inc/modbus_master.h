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
#ifndef MODBUS_MASTER_H
#define MODBUS_MASTER_H

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
/* characterstic IDs i.e functional codes  */
typedef enum FC_CODE  {
    FC_SET_SENSOR_HEIGHT ,
    FC_GET_SENSOR_DATA
}FC_CODE;




/*******************************************************************************
*                          Type Declarations
*******************************************************************************/

typedef struct  MODBUS_MASTER
{
    U32 slave_addr;
} MODBUS_MASTER;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void MODBUS_Init();

BOOLEAN MODBUS_GetSetParam(U32 slave_addr, FC_CODE fc, void* buf);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
