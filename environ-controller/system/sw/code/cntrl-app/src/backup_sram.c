/*******************************************************************************
* (C) Copyright 2021;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : backup_sram.c
*
*  Description         : It contains backup sram driver functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   25th Jan 2021    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
//#include "main.h"
#include "stdlib.h"
#include "backup_sram.h"
#include "trace.h"
#include "stm32f4xx_hal.h"
//#include "stm32f4xx_hal_rcc_ex.h"


/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void BACKUP_SRAM_Init(void)
{
    HAL_PWR_EnableBkUpAccess();

    /* Enable backup domain access  */
    //  __HAL_RCC_PWR_CLK_ENABLE();

//BKPSRAM_BASE

    /* Enable backup regulator
       Wait for backup regulator to stabilize */
    HAL_PWREx_EnableBkUpReg();

   /* Disable access to Backup domain */
    //  HAL_PWR_DisableBkUpAccess();
    //  __HAL_RCC_BKPSRAM_CLK_ENABLE();
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void BACKUP_SRAM_Write(U32 Addr, U8* Buf, U32 Len)
{
    U8* src_addr;

    /* Enable clock to BKPSRAM */
    __HAL_RCC_BKPSRAM_CLK_ENABLE();

    /* Pointer write on specific location of backup SRAM */
    src_addr = (U8* )Addr;

    memcpy(src_addr,Buf, Len);

    /* Disable clock to BKPSRAM */
    __HAL_RCC_BKPSRAM_CLK_DISABLE();
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void BACKUP_SRAM_Read(U32 Addr, U8* Buf, U32 Len)
{
    U8* src_addr;

    /* Enable clock to BKPSRAM */
    __HAL_RCC_BKPSRAM_CLK_ENABLE();

    /* Pointer write on specific location of backup SRAM */
    src_addr = (U8* ) Addr;

    memcpy(Buf,src_addr, Len);

    /* Disable clock to BKPSRAM */
    __HAL_RCC_BKPSRAM_CLK_DISABLE();
}
                           
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/


/*******************************************************************************
*                          End of File
*******************************************************************************/
