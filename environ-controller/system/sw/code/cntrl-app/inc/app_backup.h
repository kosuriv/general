/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : app_backup.h
*
*  Description         : This has all declarations for all backup 
                         functionality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  25th Jan 2021     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef APP_BACKUP_H
#define APP_BACKUP_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "app.h"


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
void App_backup_store(APP_SW_RESET_CAUSE reason);

BOOLEAN App_backup_retreive(void);

APP_SW_RESET_CAUSE App_backup_reset_reason(void);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
