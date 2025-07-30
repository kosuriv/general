/*******************************************************************************
* (C) Copyright 2014;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : utility.h
*
*  Description         : It Contains Utility prototypes
*
*  Change history      : $Id$
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   20th Jan 2014    1.1               Initial Creation
*  
*******************************************************************************/

#ifndef UTILITY_H
#define UTILITY_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "profile_data_types.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Macro Definitions
*******************************************************************************/
#define UTILITY_PRINT_HEX_MODE 0

#define UTILITY_PRINT_CHAR_MODE 1
/*******************************************************************************
*                          Type Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void UTILITY_PrintBuffer(U8* buf,U32 len, U8 mode);

U32  UTILITY_convert_time_duration_to_str(U32 time_in_sec , U8* pStr);

S32 UTILITY_date_cmp(mDATE* d1, mDATE* d2);

void UTILITY_convert_time_str(U8* time_str, mTIME* time);

void UTILITY_convert_date_str(U8* date_str, mDATE* date);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/



