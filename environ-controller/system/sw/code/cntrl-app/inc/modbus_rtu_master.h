/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : modbus_rtu_master.h
*
*  Description         : This file contains MODBUS RTU master function declarations
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  17th Nov 2020     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef MODBUS_RTU_MASTER_H
#define MODBUS_RTU_MASTER_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "osa.h"
#include "uart.h"



/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Macro Definitions
*******************************************************************************/

/*******************************************************************************
*                          Type Declarations
*******************************************************************************/
typedef struct MODBUS_RTU_MASTER
{ 
    UART* pSerialDriver;

    CALLBACK32  callback;

} MODBUS_RTU_MASTER;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void MODBUS_RTU_MASTER_Init(MODBUS_RTU_MASTER*     pThis,
                            UART*                  pSerialDriver);

void MODBUS_RTU_MASTER_Tx(U8   SlaveAddr,
                          U8   fc,
                          U8   *pBuf,
                          U32    size);

void MODBUS_RTU_MASTER_Rx(U8                 *pBuf,
                          U32                size,
                          CALLBACK32_FCT     pFunction,
                          void               *pArg);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/



