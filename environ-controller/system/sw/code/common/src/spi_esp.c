/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : spi.c
*
*  Description         : It contains SPI driver functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu /Linta  11th Oct 2021     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <string.h>

#include "gpio.h"

#include "spi.h"
#include "trace.h"
#include "utility.h"

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define SPI_MAX_TX_SIZE  1024

#define SPI_CLK_VALUE  10*1000*1000  //max clk speed  i.e 10 Mhz 

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static SPI_STATUS spi_TxRxHelper(SPI*            pSpi,
                                 U8*             pTxBuf,
                                 U32             size,
                                 U8*             pRxBuf,
                                 CALLBACK32_FCT  pFunction,
                                 void*           pArg,
                                 SPI_TXRX_MODE   mode,
                                 SPI_TxRxChannel*  pRxTxChannel);


static void spi_Prepare(SPI_TxRxChannel*  pThis,
                        U8*               pBuf,
                        U32               size,
                        CALLBACK32_FCT    pFunction,
                        void*             pArg,
                        SPI_TXRX_MODE     mode);


static void  spi_DmaIsr(spi_transaction_t *t);

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
void SPI_Init(SPI           *pSpi,
              SPI_NO        spi_no,
              SPI_MODE      mode)
{

    spi_host_device_t  spi_host_id;
    spi_common_dma_t  dma_channel_no;
    spi_bus_config_t buscfg;
    spi_device_interface_config_t devcfg;
    U32 mosi_pin;
    U32 miso_pin;
    U32 sclk_pin;
    U32 cs_pin;

    TRACE_INFO("%s() Entry\n",__FUNCTION__);

    switch(spi_no)
    {
        case SPI1_HW_INDEX:
            break;

        case SPI2_HW_INDEX:       
            spi_host_id = SPI2_HOST;
            dma_channel_no = SPI_DMA_CH_AUTO ;
#ifdef ESP32_S3_MINI
            mosi_pin = GPIO_NUM_37;
            miso_pin = GPIO_NUM_35;
            sclk_pin = GPIO_NUM_36;
            cs_pin = GPIO_NUM_38;

#else
            mosi_pin = GPIO_NUM_35;
            miso_pin = GPIO_NUM_37;
            sclk_pin = GPIO_NUM_36;
            cs_pin = GPIO_NUM_34;
#endif
            break;
    }
    gpio_reset_pin(GPIO_NUM_37);
    gpio_reset_pin(GPIO_NUM_35);
    gpio_reset_pin(GPIO_NUM_36);
    gpio_reset_pin(GPIO_NUM_38);

    memset(&buscfg,0x00, sizeof(buscfg));
    buscfg.miso_io_num = miso_pin;
    buscfg.mosi_io_num = mosi_pin;
    buscfg.sclk_io_num = sclk_pin;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = SPI_MAX_TX_SIZE;
   
    //Initialize SPI bus
    if(spi_bus_initialize(spi_host_id, &buscfg, dma_channel_no) == ESP_OK)
    {
        TRACE_DBG("SPI initialization success\n");
    }
    else
    {
        TRACE_WARN("SPI initialization failed\n");
        return;
    }

    memset(&devcfg,0x00, sizeof(devcfg));
    devcfg.clock_speed_hz = SPI_CLK_VALUE;          
    devcfg.mode =  mode;                        //SPI mode 
    devcfg.spics_io_num = cs_pin;               //CS pin
    devcfg.queue_size = 8;

    devcfg.post_cb = spi_DmaIsr;

    if(spi_bus_add_device(spi_host_id, &devcfg, &pSpi->h_esp_spi) == ESP_OK)
    {
        TRACE_DBG("SPI%d configured successfully.\n",spi_host_id);
    }
    else
    {
        TRACE_WARN("SPI%d configured  FAILED.\n",spi_host_id);
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SPI_DMAInit(SPI *pSpi)
{
   
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
SPI_STATUS SPI_Tx(SPI*            pSpi,
                  U8*             pTxBuf,
                  U32             size,
                  CALLBACK32_FCT  pFunction,
                  void*           pArg,
                  SPI_TXRX_MODE   mode)
{  
  //  TRACE_DBG("%s() Entry\n",__FUNCTION__);
  
    return spi_TxRxHelper(pSpi,pTxBuf,size,NULL,pFunction,pArg,mode,&pSpi->tx);    
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
SPI_STATUS SPI_Rx(SPI*            pSpi,
                  U8*             pRxBuf,
                  U32             size,
                  CALLBACK32_FCT  pFunction,
                  void*           pArg,
                  SPI_TXRX_MODE   mode)
{

    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    /* fill RX buffer  to use as TX buffer with some dummy data i.e ff*/
    memset (pRxBuf, 0xff , size);

    return spi_TxRxHelper(pSpi,pRxBuf,size,pRxBuf,pFunction,pArg,mode,&pSpi->rx);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
SPI_STATUS SPI_TxRx(SPI*            pSpi,
                    U8*             pTxBuf,
                    U32             size,
                    U8*             pRxBuf,
                    CALLBACK32_FCT  pRxFunction,
                    void*           pRxArg,
                    SPI_TXRX_MODE   mode)
{
//    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    return spi_TxRxHelper(pSpi,pTxBuf,size,pRxBuf,pRxFunction,pRxArg,mode,&pSpi->rx);
}

/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static SPI_STATUS spi_TxRxHelper(SPI*            pSpi,
                                 U8*             pTxBuf,
                                 U32             size,
                                 U8*             pRxBuf,
                                 CALLBACK32_FCT  pFunction,
                                 void*           pArg,
                                 SPI_TXRX_MODE   mode,
                                 SPI_TxRxChannel*  pRxTxChannel)

{
    S32 status;
    spi_transaction_t t;

  //  TRACE_DBG("%s() Entry\n",__FUNCTION__);
   

    if(spi_device_acquire_bus(pSpi->h_esp_spi, portMAX_DELAY) != ESP_OK)
    {    
        status = SPI_STATUS_ERROR;
        TRACE_WARN("SPI TX/RX BUS error \n");
        return status;
    } 
    else
    {
      //  TRACE_DBG("SPI TX/RX BUS ACQUIRED\n");

        memset(&t, 0x00, sizeof(t));
     
     //   TRACE_DBG("SPI Data size  = %d \n", size);

        t.length = 8*size; 

        t.tx_buffer = pTxBuf;

        t.rx_buffer = pRxBuf;
    
        if(pRxBuf)
        {
            t.rxlength = t.length;
        }

        t.user=(void*)NULL;    //D/C needs to be set to 1

      //  TRACE_DBG("SPI TX/RX Data length = %d \n", t.length);

        switch (mode)
        {
            case DMA_POLLING:
                if( spi_device_polling_transmit(pSpi->h_esp_spi, &t) == ESP_OK)
                {
                    TRACE_DBG("SPI Data TX/RX SUCCESS\n");         
                    status = SPI_STATUS_OK;
                }
                else
                {            
                    TRACE_WARN("SPI Data TX/RX Failed\n");
                    status = SPI_STATUS_ERROR;
                }   
                spi_device_release_bus(pSpi->h_esp_spi);
                break;

            case DMA_INTERRUPT:
                if(pRxBuf)
                {
                    spi_Prepare(pRxTxChannel,pRxBuf,size, pFunction,pArg,mode);
                }
                else
                {
                    spi_Prepare(pRxTxChannel,pTxBuf,size, pFunction,pArg,mode);       
                }

                /* handle function for interrupt mode , register spi_DmaIsr */
                pRxTxChannel->h_esp_spi = pSpi->h_esp_spi;
                t.user = (void*) pRxTxChannel;

                if(spi_device_transmit(pSpi->h_esp_spi, &t) == ESP_OK)
                {
              //      TRACE_DBG("SPI Data TX/RX SUCCESS in DMA INTERRUPT mode \n");         
                    status = SPI_STATUS_OK;
                }
                else
                {            
                    TRACE_WARN("SPI Data TX/RX Failed in DMA INTERRUPT mode \n");
                    status = SPI_STATUS_ERROR;
                } 
                break;
        }
    }    
    return status;    
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void spi_Prepare(SPI_TxRxChannel*  pThis,
                        U8*               pBuf,
                        U32               size,
                        CALLBACK32_FCT    pFunction,
                        void*             pArg,
                        SPI_TXRX_MODE     mode)
{
    /* save parameters for use in the ISR */
    pThis->callback.pArg  = pArg;
    pThis->callback.pFunction = pFunction;
    
    pThis->pData.pBuf  = pBuf;
    pThis->pData.size  = size;

    pThis->flag = mode;  
     
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    :  it is call back function registerd when SPI transaction is
               completed 
*******************************************************************************/
static void  spi_DmaIsr(spi_transaction_t *t)
{
    SPI_TxRxChannel*  pRxTxChannel;

    if(t->user)
    {
        pRxTxChannel = (SPI_TxRxChannel*)t->user;

        if(pRxTxChannel->callback.pFunction)
        {
            pRxTxChannel->callback.pFunction(pRxTxChannel->callback.pArg,SPI_STATUS_OK);
        }
        spi_device_release_bus( pRxTxChannel->h_esp_spi);   
    }
}
/*******************************************************************************
*                          End of File
*******************************************************************************/
