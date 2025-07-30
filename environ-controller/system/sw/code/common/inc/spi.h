/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : spi.h
*
*  Description         : This file defines SPI function declarations
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  14th Dec 2020     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef SPI_H
#define SPI_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"

#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "stm32f4xx_hal.h"
#endif

#ifdef ESP32_S2_MINI
#include "driver/spi_master.h"
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

typedef enum SPI_NO
{
    SPI1_HW_INDEX = 1,
    SPI2_HW_INDEX,
    SPI3_HW_INDEX,
    SPI4_HW_INDEX,
    SPI5_HW_INDEX,
    SPI6_HW_INDEX,
    SPI_MAX_NUM
} SPI_NO;

typedef enum SPI_MODE
{
    SPI_MODE0,
    SPI_MODE1,
    SPI_MODE2,
    SPI_MODE3
} SPI_MODE;

typedef enum SPI_TXRX_MODE
{
    POLLING,   
    INTERRUPT_1, 
    DMA_POLLING,
    DMA_INTERRUPT 
} SPI_TXRX_MODE;

typedef enum SPI_STATUS
{
    SPI_STATUS_OK,
    SPI_STATUS_BUSY,
    SPI_STATUS_ERROR
} SPI_STATUS;

typedef struct SPI_TxRxChannel
{
    BUFFER8            pData;
    CALLBACK32         callback;
    SPI_TXRX_MODE      flag;
#ifdef ESP32_S2_MINI
    spi_device_handle_t h_esp_spi;
#endif

       
} SPI_TxRxChannel;

typedef struct SPI
{

    SPI_NO  spi_no;

#ifdef NUCLEO_F439ZI_EVK_BOARD
    SPI_HandleTypeDef hspi;

    DMA_HandleTypeDef hdma_spi_tx;

    DMA_HandleTypeDef hdma_spi_rx;
#endif

    SPI_TxRxChannel  tx;

    SPI_TxRxChannel  rx;

#ifdef ESP32_S2_MINI
    spi_device_handle_t h_esp_spi;
#endif

} SPI;



/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void SPI_Init(SPI           *pSpi,
              SPI_NO        spi_no,
              SPI_MODE      mode);

void SPI_DMAInit(SPI *pSpi);

SPI_STATUS SPI_Tx(SPI*           pThis,
                  U8*             pBuf,
                  U32             size,
                  CALLBACK32_FCT  pFunction,
                  void*           pArg,
                  SPI_TXRX_MODE   mode);

SPI_STATUS SPI_Rx(SPI*           pThis,
                  U8*             pBuf,
                  U32             size,
                  CALLBACK32_FCT  pFunction,
                  void*           pArg,
                  SPI_TXRX_MODE   mode);

SPI_STATUS SPI_TxRx(SPI*            pSpi,
                    U8*             pTxBuf,
                    U32             size,
                    U8*             pRxBuf,
                    CALLBACK32_FCT  pRxFunction,
                    void*           pRxArg,
                    SPI_TXRX_MODE   mode);

#ifdef NUCLEO_F439ZI_EVK_BOARD
void SPI_RxDmaIsr(IRQn_Type Irq_No);

void SPI_TxDmaIsr(IRQn_Type Irq_No);
#endif




#ifdef __cplusplus
}
#endif

#endif
/*******************************************************************************
*                          End of File
*******************************************************************************/
