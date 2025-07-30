/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : sd_card.h
*
*  Description         : This file defines SD Card function declarations 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  14th Dec 2020     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef SD_CARD_H
#define SD_CARD_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "stm32f4xx_hal.h"
#endif

#include "data_types.h"
#ifndef BOOTLOADER
#include "osa.h"
#endif
#include "spi.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

typedef enum SD_CARD_INTERFACE
{
    SPI_IFACE,           /* SD card accessed through SPI  */
    SDIO_IFACE           /* SD card accessed through SDIO  */
} SD_CARD_INTERFACE;

/* Prototype for driver write function handler.*/
typedef U32 (*SD_CARD_DRIVER_WRITE) (void*           handle,
                                     U8*             pBuf,
                                     U32             size,
                                     CALLBACK32_FCT  pFunction,
                                     void*           pArg,
                                     U32   mode);

/* Prototype for driver read function handler.*/
typedef U32 (*SD_CARD_DRIVER_READ) (void*            handle,
                                     U8*             pBuf,
                                     U32             size,
                                     CALLBACK32_FCT  pFunction,
                                     void*           pArg,
                                     U32   mode);


typedef struct SD_CARD_Driver
{
  //  SD_CARD_DRIVER_WRITE pwrite;       /* Function handler to write . */
  //  SD_CARD_DRIVER_READ pread;         /* Function handler to read  */
    union
    {
        SPI*  Spi;   
        void* Sdio; /* SDIO driver , needed if future HW changes to SDIO interface to SD card*/

    } handle;
} SD_CARD_Driver;

typedef enum
{
  SD_OK  ,
  SD_INTERNAL_ERROR, 
  SD_NOT_CONFIGURED,
  SD_REQUEST_PENDING, 
  SD_REQUEST_NOT_APPLICABLE, 
  SD_INVALID_PARAMETER,  
  SD_UNSUPPORTED_FEATURE,  
  SD_UNSUPPORTED_HW,  
  SD_ERROR  

} SD_CARD_Error;


typedef struct SD_CARD
{
    SD_CARD_INTERFACE Interface;

    SD_CARD_Driver Driver; 

#ifndef BOOTLOADER
    osa_semaphore_type_t rx_sem; 
#endif

    volatile BOOLEAN rx_flag;

    U32 card_type;

    U32 card_capacity;

} SD_CARD;

/*******************************************************************************
*                          Type Declarations
*******************************************************************************/



/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void SD_CARD_Init(SD_CARD* pThis,SD_CARD_INTERFACE iface, void* DriverHandle );

BOOLEAN SD_CARD_is_sd_card_present(void);

BOOLEAN SD_CARD_is_dir_present(U8* dir_name);

BOOLEAN SD_CARD_create_dir(U8* dir_name);

BOOLEAN SD_CARD_delete_dir(U8 *dir_name);

U32 SD_CARD_get_filelist_from_dir(U8 *dir_name, U8 **buffer, U8 buf_len);

BOOLEAN SD_CARD_create_file(U8* data , U32 data_len, U8 *file_name);

BOOLEAN SD_CARD_delete_file(U8 *file_name);

BOOLEAN SD_CARD_move_file(U8 *file_name , U8* src_dir, U8* dst_dir);

S32 SD_CARD_read_data_from_file(U8 *file_name, U8* data , U32 data_len);

BOOLEAN SD_CARD_write_data_to_file(U8* data , U32 data_len, U8 *file_name);

BOOLEAN SD_CARD_append_file(U8* buffer, U32 buf_len, U8 *file_name);

BOOLEAN SD_CARD_is_file_present(U8* file_name);

void* SD_CARD_get_file_ptr(U8* file_name);

void SD_CARD_append_file2(U8* buffer, U32 buf_len,  void **fPtr);

void SD_CARD_close_file( void **fPtr);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
