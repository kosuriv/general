/*******************************************************************************
* (C) Copyright 2018;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : serial_comport.h
*
*  Description         : It contains seriport function declarations
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   27th Jan 2018     1.1               Initial Creation
*  
*******************************************************************************/

#ifndef SERIAL_COMPORT_H
#define SERIAL_COMPORT_H

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
*                          Macro Definitions
*******************************************************************************/

/*******************************************************************************
*                          Type Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void SERIAL_COMPORT_Init(U32 baud_rate, U8* ComPortName);

void SERIAL_COMPORT_DeInit();

void SERIAL_COMPORT_Read(U8* buf, U32 length);

void SERIAL_COMPORT_Write(U8* buf, U32 length);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/



