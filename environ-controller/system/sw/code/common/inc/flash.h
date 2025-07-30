/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : flash.h
*
*  Description         : This file defines flash function declarations
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  29th July 2020    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef FLASH_H
#define FLASH_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"

#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "stm32f4xx_hal.h"
#endif

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
*                          Type Declarations
*******************************************************************************/
typedef enum
{
  FLASH_OK       = 0x00U,
  FLASH_ERROR    = 0x01U,
  FLASH_BUSY     = 0x02U,
  FLASH_TIMEOUT  = 0x03U

} FLASH_Status;

typedef struct FLASH_T
{

	/* private Data of Flash*/
    U32 reserved;

} FLASH_T;



/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void FLASH_Init(FLASH_T *flash);

FLASH_Status FLASH_Erase(FLASH_T *flash, U32 sector_address);
  
FLASH_Status FLASH_Write(FLASH_T *flash, U32 addr, const U8 *buf, U32 size);

void FLASH_Read(FLASH_T *flash,U32 addr, BYTE *buf, U32 size);

#ifdef NUCLEO_F439ZI_EVK_BOARD
void FLASH_ISR(IRQn_Type Irq_No);
#endif

void FLASH_EraseSize(FLASH_T *flash, U32 addr , U32 size);


#ifdef __cplusplus
}
#endif

#endif
/*******************************************************************************
*                          End of File
*******************************************************************************/
