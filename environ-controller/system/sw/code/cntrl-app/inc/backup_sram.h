/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : backup_sram.h
*
*  Description         : This has all declarations for backup sram driver
                         functinality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  25th Jan 2021     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef BACKUP_SRAM_H
#define BACKUP_SRAM_H

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

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void BACKUP_SRAM_Init(void);

void BACKUP_SRAM_Write(U32 Addr, U8* Buf ,U32 Len);

void BACKUP_SRAM_Read(U32 Addr, U8* Buf, U32 Len);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
