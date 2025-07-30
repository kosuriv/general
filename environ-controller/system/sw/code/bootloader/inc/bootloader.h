/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : bootloader.h
*
*  Description         : 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  16th Jun 2020     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "spi.h"
#include "sd_card.h"
#include "flash.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
typedef struct BOOTLOADER_T
{

    SPI Spi1;

    SD_CARD SdCard;

    FLASH_T FlashDriver;

}BOOTLOADER_T;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void BOOTLOADER_Start();


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
