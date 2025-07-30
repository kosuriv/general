/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
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
*   venu kosuri  14th Dec 2020     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <string.h>

#include "data_types.h"
#include "spi.h"
#include "trace.h"




/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/

#define SPI_ENABLE_CS(pin , port)                    \
do {                                                 \
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);      \
} while (0);


#define SPI_DISABLE_CS(pin , port)                    \
do {                                                  \
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);     \
} while (0);

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void spi_Prepare(SPI_TxRxChannel*  pThis,
                        U8*               pBuf,
                        U32               size,
                        CALLBACK32_FCT    pFunction,
                        void*             pArg,
                        SPI_TXRX_MODE     mode);

static void spi_TxDmaIsrCallback(DMA_HandleTypeDef *hdma );

static void spi_RxDmaIsrCallback(DMA_HandleTypeDef *hdma );

static void spi_TxRxDmaIsrCallback(DMA_HandleTypeDef *hdma );


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern void *APP_intr_arg_list[];

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

    switch(spi_no)
    {
        case SPI1_HW_INDEX:
            pSpi->hspi.Instance = SPI1;
            APP_intr_arg_list[SPI1_IRQn] = pSpi;
            break;

        case SPI2_HW_INDEX:
            pSpi->hspi.Instance = SPI2;
            APP_intr_arg_list[SPI2_IRQn] = pSpi;
            break;

         case SPI3_HW_INDEX:
            pSpi->hspi.Instance = SPI3;
            APP_intr_arg_list[SPI3_IRQn] = pSpi;
            break;

        case SPI4_HW_INDEX:
            pSpi->hspi.Instance = SPI4;
            APP_intr_arg_list[SPI4_IRQn] = pSpi;
            break;

         case SPI5_HW_INDEX:
            pSpi->hspi.Instance = SPI5;
            APP_intr_arg_list[SPI5_IRQn] = pSpi;
            break;

        case SPI6_HW_INDEX:
            pSpi->hspi.Instance = SPI6;
            APP_intr_arg_list[SPI6_IRQn] = pSpi;
            break;        
    }

    pSpi->hspi.Instance = SPI1;
    pSpi->hspi.Init.Mode = SPI_MODE_MASTER;
    pSpi->hspi.Init.Direction = SPI_DIRECTION_2LINES;
    pSpi->hspi.Init.DataSize = SPI_DATASIZE_8BIT;

    pSpi->hspi.Init.NSS = SPI_NSS_HARD_OUTPUT;
    pSpi->hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    pSpi->hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    pSpi->hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    pSpi->hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    pSpi->hspi.Init.CRCPolynomial = 10;

    switch ( mode)
    {
        case SPI_MODE0:
            pSpi->hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
            pSpi->hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
            break;
        case SPI_MODE1:
            pSpi->hspi.Init.CLKPolarity = SPI_POLARITY_HIGH;
            pSpi->hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
            break;
        case SPI_MODE2:
            pSpi->hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
            pSpi->hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
            break;
        case SPI_MODE3:
            pSpi->hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
            pSpi->hspi.Init.CLKPhase = SPI_PHASE_2EDGE;
            break;    
    }
    if (HAL_SPI_Init(&pSpi->hspi) != HAL_OK)
    {
        Error_Handler();
    }

    //TRACE_INFO("SPI_CR1 Reg = %x \n", READ_REG(pSpi->hspi.Instance->CR1));
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SPI_DMAInit(SPI *pSpi)
{
    /* Initialize  DMA for  SPI */
    pSpi->hdma_spi_tx.Instance = DMA2_Stream3;
    pSpi->hdma_spi_tx.Init.Channel = DMA_CHANNEL_3;
    pSpi->hdma_spi_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    pSpi->hdma_spi_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    pSpi->hdma_spi_tx.Init.MemInc = DMA_MINC_ENABLE;
    pSpi->hdma_spi_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    pSpi->hdma_spi_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    pSpi->hdma_spi_tx.Init.Mode = DMA_NORMAL;
    pSpi->hdma_spi_tx.Init.Priority = DMA_PRIORITY_LOW;
    pSpi->hdma_spi_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    pSpi->hdma_spi_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    pSpi->hdma_spi_tx.Init.MemBurst = DMA_MBURST_SINGLE;
    pSpi->hdma_spi_tx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&pSpi->hdma_spi_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&pSpi->hspi,hdmatx,pSpi->hdma_spi_tx);

    /* SPI1_RX Init */
    pSpi->hdma_spi_rx.Instance = DMA2_Stream0;
    pSpi->hdma_spi_rx.Init.Channel = DMA_CHANNEL_3;
    pSpi->hdma_spi_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    pSpi->hdma_spi_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    pSpi->hdma_spi_rx.Init.MemInc = DMA_MINC_ENABLE;
    pSpi->hdma_spi_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    pSpi->hdma_spi_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    pSpi->hdma_spi_rx.Init.Mode = DMA_NORMAL;
    pSpi->hdma_spi_rx.Init.Priority = DMA_PRIORITY_LOW;
    pSpi->hdma_spi_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    pSpi->hdma_spi_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    pSpi->hdma_spi_rx.Init.MemBurst = DMA_MBURST_SINGLE;
    pSpi->hdma_spi_rx.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&pSpi->hdma_spi_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(&pSpi->hspi,hdmarx,pSpi->hdma_spi_rx);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
SPI_STATUS SPI_Tx(SPI*            pSpi,
                  U8*             pBuf,
                  U32             size,
                  CALLBACK32_FCT  pFunction,
                  void*           pArg,
                  SPI_TXRX_MODE   mode)
{
	SPI_HandleTypeDef* hspi = &pSpi->hspi;

	//TRACE_INFO("%s() Entry\n",__FUNCTION__);

    spi_Prepare(&pSpi->tx,pBuf,size, pFunction,pArg,mode);

    if ( mode == DMA_INTERRUPT)
    {
#if 0
        HAL_SPI_Transmit_DMA(&pSpi->hspi, pBuf, (U16)size);
#else

        hspi->hdmatx->XferCpltCallback = spi_TxDmaIsrCallback;

          /* Enable the Tx DMA Stream/Channel */
        if (HAL_OK != HAL_DMA_Start_IT(hspi->hdmatx, (U32)pBuf, (U32)&hspi->Instance->DR,size))
        {
            /* Update SPI error code */
            SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_DMA);
            return;
        }

        /* Check if the SPI is already enabled */
        if ((hspi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
        {
            /* Enable SPI peripheral */
            __HAL_SPI_ENABLE(hspi);
        }

        /* Enable the SPI Error Interrupt Bit */
        __HAL_SPI_ENABLE_IT(hspi, (SPI_IT_ERR));

        /* Enable Tx DMA Request */
        SET_BIT(hspi->Instance->CR2, SPI_CR2_TXDMAEN);

#endif

    }
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
SPI_STATUS SPI_Rx(SPI*            pSpi,
                  U8*             pBuf,
                  U32             size,
                  CALLBACK32_FCT  pFunction,
                  void*           pArg,
                  SPI_TXRX_MODE   mode)
{
	SPI_HandleTypeDef* hspi = &pSpi->hspi;

//	TRACE_INFO("%s() Entry\n",__FUNCTION__);

    spi_Prepare(&pSpi->rx,pBuf,size, pFunction,pArg,mode);

    if ( mode == DMA_INTERRUPT)
    {
#if 0
        HAL_SPI_Receive_DMA( &pSpi->hspi, pBuf, (U16)size);      
#else
		SPI_TxRx(pSpi, pBuf,size,pBuf,pFunction,pArg,DMA_INTERRUPT);
#endif
    }
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

	SPI_HandleTypeDef* hspi = &pSpi->hspi;

//	TRACE_INFO("%s() Entry\n",__FUNCTION__);

	spi_Prepare(&pSpi->rx,pRxBuf,size, pRxFunction,pRxArg,mode);
	
    if ( mode == DMA_INTERRUPT)
    {
		if(pTxBuf == pRxBuf)
		{
		//	TRACE_INFO(" JUST SPI RECEPTION ONLY \n");
			hspi->hdmarx->XferCpltCallback     = spi_RxDmaIsrCallback;
		}
		else
		{
		//	TRACE_INFO("SPI TX & RX \n");
			hspi->hdmarx->XferCpltCallback     = spi_TxRxDmaIsrCallback;
		
		}
		  /* Enable the Rx DMA Stream/Channel  */
		if (HAL_OK != HAL_DMA_Start_IT(hspi->hdmarx, (U32)&hspi->Instance->DR, (U32)pRxBuf,size))
		{
    		/* Update SPI error code */
			SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_DMA);
			return;
  		}

		/* Enable Rx DMA Request */
  		SET_BIT(hspi->Instance->CR2, SPI_CR2_RXDMAEN);

		hspi->hdmatx->XferCpltCallback     = NULL;

		/* Enable the Tx DMA Stream/Channel  */
  		if (HAL_OK != HAL_DMA_Start_IT(hspi->hdmatx, (U32)pTxBuf, (U32)&hspi->Instance->DR,size))
  		{
    		/* Update SPI error code */
    		SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_DMA);
			return;
  		}

  		/* Check if the SPI is already enabled */
  		if ((hspi->Instance->CR1 & SPI_CR1_SPE) != SPI_CR1_SPE)
  		{
    		/* Enable SPI peripheral */
    		__HAL_SPI_ENABLE(hspi);
  		}
  		/* Enable the SPI Error Interrupt Bit */
  		__HAL_SPI_ENABLE_IT(hspi, (SPI_IT_ERR));

  		/* Enable Tx DMA Request */
  		SET_BIT(hspi->Instance->CR2, SPI_CR2_TXDMAEN);
    }
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SPI_TxDmaIsr(IRQn_Type Irq_No)
{
	SPI *pSpi = (SPI*) APP_intr_arg_list[Irq_No];

    CALLBACK32* pCallback = &pSpi->tx.callback;

	HAL_DMA_IRQHandler(&pSpi->hdma_spi_tx);

	//__HAL_SPI_DISABLE(&pSpi->hspi);

    if (pCallback->pFunction != NULL)
    {            
        pCallback->pFunction(pCallback->pArg, SPI_STATUS_OK);
    }
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SPI_RxDmaIsr(IRQn_Type Irq_No)
{
	SPI *pSpi = (SPI*) APP_intr_arg_list[Irq_No];

    CALLBACK32* pCallback = &pSpi->rx.callback;

	HAL_DMA_IRQHandler(&pSpi->hdma_spi_rx);

    if (pCallback->pFunction != NULL)
    {
    	pCallback->pFunction(pCallback->pArg, SPI_STATUS_OK);
    }
}

/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

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
* Remarks    : called by HAL_DMA_IRQHandler() in ISR context
*******************************************************************************/
static void spi_TxDmaIsrCallback(DMA_HandleTypeDef *hdma )
{
    SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)(((DMA_HandleTypeDef *)hdma)->Parent); 

    uint32_t tickstart;

    /* Init tickstart for timeout management*/
    tickstart = HAL_GetTick();

    //TRACE_INFO("%s() Entry \n",__FUNCTION__);

    /* DMA Normal Mode */
    if ((hdma->Instance->CR & DMA_SxCR_CIRC) != DMA_SxCR_CIRC)
    {
        /* Disable ERR interrupt */
        __HAL_SPI_DISABLE_IT(hspi, SPI_IT_ERR);

        /* Disable Tx DMA Request */
        CLEAR_BIT(hspi->Instance->CR2, SPI_CR2_TXDMAEN);

        /* Check the end of the transaction */
        if (SPI_EndRxTxTransaction(hspi, 100, tickstart) != HAL_OK)
        {
            SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
        }

        /* Clear overrun flag in 2 Lines communication mode because received data is not read */
        if (hspi->Init.Direction == SPI_DIRECTION_2LINES)
        {
            __HAL_SPI_CLEAR_OVRFLAG(hspi);
        }

        hspi->TxXferCount = 0U;
        hspi->State = HAL_SPI_STATE_READY;
  }

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : called by HAL_DMA_IRQHandler() in ISR context
*******************************************************************************/
static void spi_RxDmaIsrCallback(DMA_HandleTypeDef *hdma )
{

	SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)(((DMA_HandleTypeDef *)hdma)->Parent); /* Derogation MISRAC2012-Rule-11.5 */
	U32 tickstart;

    //TRACE_INFO("%s() Entry \n",__FUNCTION__);

	/* Init tickstart for timeout management*/
	tickstart = HAL_GetTick();

	/* DMA Normal Mode */
	if ((hdma->Instance->CR & DMA_SxCR_CIRC) != DMA_SxCR_CIRC)
	{
		/* Disable ERR interrupt */
		__HAL_SPI_DISABLE_IT(hspi, SPI_IT_ERR);

		/* Check if we are in Master RX 2 line mode */
		if ((hspi->Init.Direction == SPI_DIRECTION_2LINES) && (hspi->Init.Mode == SPI_MODE_MASTER))
    	{
      		/* Disable Rx/Tx DMA Request (done by default to handle the case master rx direction 2 lines) */
      		CLEAR_BIT(hspi->Instance->CR2, SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
    	}
    	else
    	{
      		/* Normal case */
      		CLEAR_BIT(hspi->Instance->CR2, SPI_CR2_RXDMAEN);
    	}

		/* Check the end of the transaction */
    	if (SPI_EndRxTransaction(hspi, 100, tickstart) != HAL_OK)
    	{
      		hspi->ErrorCode = HAL_SPI_ERROR_FLAG;
    	}

    	hspi->RxXferCount = 0U;
    	hspi->State = HAL_SPI_STATE_READY;
  }

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void spi_TxRxDmaIsrCallback(DMA_HandleTypeDef *hdma )
{
	SPI_HandleTypeDef *hspi = (SPI_HandleTypeDef *)(((DMA_HandleTypeDef *)hdma)->Parent); 
	U32 tickstart;

   // TRACE_INFO("%s() Entry \n",__FUNCTION__);

	/* Init tickstart for timeout management*/
	tickstart = HAL_GetTick();

  	/* DMA Normal Mode */
  	if ((hdma->Instance->CR & DMA_SxCR_CIRC) != DMA_SxCR_CIRC)
  	{
    	/* Disable ERR interrupt */
    	__HAL_SPI_DISABLE_IT(hspi, SPI_IT_ERR);

		/* Check the end of the transaction */
    	if (SPI_EndRxTxTransaction(hspi, 100, tickstart) != HAL_OK)
    	{
      		SET_BIT(hspi->ErrorCode, HAL_SPI_ERROR_FLAG);
    	}

    	/* Disable Rx/Tx DMA Request */
    	CLEAR_BIT(hspi->Instance->CR2, SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

    	hspi->TxXferCount = 0U;
    	hspi->RxXferCount = 0U;
    	hspi->State = HAL_SPI_STATE_READY;
  }

}


/*******************************************************************************
*                          End of File
*******************************************************************************/
