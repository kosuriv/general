/*******************************************************************************
* (C) Copyright 2016;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : list.h
*
*  Description         : It contains list of ant item functionality
*
*  Change history      : $Id$
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   29th Feb 2016    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef QLIST_H
#define QLIST_H

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
typedef struct LIST
{
    void* pArg;
    struct LIST* Next;
} LIST;


/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void LIST_Init();
void LIST_DeInit();
void LIST_Push(LIST** list, void* pArg);
void LIST_Pop(LIST** list, void** pArg);
void LIST_Release(LIST** list);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
