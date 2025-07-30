/*******************************************************************************
* (C) Copyright 2014;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : base64.h
*
*  Description         : It Contains base64 prototypes
*
*  Change history      : $Id$
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   15th Mar 2016    1.1               Initial Creation
*  
*******************************************************************************/

#ifndef BASE64_H
#define BASE64_H

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
void BASE64_Enc(U8 *dst, U8 *src, U32 len);

void BASE64_Dec(U8 *dst, U8 *src, U32 len);

U32 BASE64_EncLen(U32 s);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/



