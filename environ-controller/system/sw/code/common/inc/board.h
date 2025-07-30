/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : board.h
*
*  Description         : This file defines hw specific paramaters 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  22nd July 2020    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef BOARD_H
#define BOARD_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/

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
#ifdef NUCLEO_F439ZI_EVK_BOARD

#define BOARD_CPU_CLOCK_HZ    ( unsigned long ) 16000000

#define BOARD_TICK_RATE_HZ   1000

#define BOARD_INTR_MAX_IRQn   (DMA2D_IRQn +1)

#define BOARD_FLASH_START_ADDRESS   0x8000000
#define BOARD_FLASH_SIZE   384*1024

#define BOARD_RAM_START_ADDRESS   0x2000000
#define BOARD_RAM_SIZE   192*1024

#define BOARD_CCMRAM_START_ADDRESS   0x1000000
#define BOARD_CCMRAM_SIZE   64*1024

#ifdef NUCLEO_F439ZI_EVK_BOARD
#define BOARD_BACKUP_SRAM_START_ADDRESS    0x40024000
#endif

#define BOARD_BACKUP_SRAM_SIZE   4*1024

#endif


#ifdef ESP32_S2_MINI

#define BOARD_BACKUP_SRAM_START_ADDRESS    0x40024000
#define BOARD_BACKUP_SRAM_SIZE   4*1024

#endif

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/



#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
