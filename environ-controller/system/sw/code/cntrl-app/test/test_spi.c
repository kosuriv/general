/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : test_uart.c
*
*  Description         : This file is stub file to test UART Tx/Rx functionality 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  26th Aug 2020     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "osa.h"
#include "data_types.h"
#include "trace.h"
#include "spi.h"



/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define SPI_TEST_BUFFSIZE 100

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void test_SpiRx(SPI_TXRX_MODE mode);

static void test_SpiTx(U8 *Input, SPI_TXRX_MODE mode);

static void test_RxCallback(void *pThis, UINT32 value);

static void test_TxCallback(void *pThis, UINT32 value);

static void test_SpiTxRx(U8 *Input, SPI_TXRX_MODE mode);

static void test_TxCallback(void *pThis, UINT32 value);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static SPI Spi;

static U8 RxBuf[SPI_TEST_BUFFSIZE];

static U8 TxBuf[SPI_TEST_BUFFSIZE];

static osa_semaphore_type_t tx_sem;

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
void TEST_SPI_Init()
{

    U8 buf[70];

    TRACE_DBG("%s() Entry \n",__FUNCTION__);

    SPI_Init(&Spi,SPI2_HW_INDEX,SPI_MODE0);

    memset(buf , 0xa5 ,sizeof(buf));

 //   test_SpiTx("FIRST PACKET" , DMA_POLLING);

  //  test_SpiTx(tx_buf,DMA_POLLING);
  //  test_SpiTxRx("FIRST PACKET FOR TXRX" , DMA_POLLING);

    test_SpiTxRx("FIRST PACKET FOR TXRX INTERRUPT " , DMA_INTERRUPT);

    test_SpiTxRx("SECOND TX  " , DMA_INTERRUPT);

#if 0
    SPI_Tx(&Spi,
           buf,
           70,
           NULL,
           NULL,
           DMA_POLLING);




    SPI_Rx(&Spi,
            buf,
            70,
            NULL,
            NULL,
            DMA_POLLING);


    UTILITY_PrintBuffer(buf,70 ,0);

#endif



    //test_SpiRx(DMA_INTERRUPT);   



}


/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_SpiRx(SPI_TXRX_MODE mode)
{

	memset(RxBuf,0x00,sizeof(RxBuf));

    SPI_Rx(&Spi,
            RxBuf,
            50,
            test_RxCallback,
            RxBuf,
            mode);


}

/*******************************************************************************
* Name       : 
* Input      : 
* Output     : 
* Returns    : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_RxCallback(void *pThis, UINT32 value)
{
    UINT8 *buf = (UINT8*)pThis;

    TRACE_DBG("Success In RX = %s\n",RxBuf);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_SpiTx(U8 *Input, SPI_TXRX_MODE mode)
{
    U32 size;

    size = sprintf(TxBuf, Input);

    SPI_Tx(&Spi,
           Input,
           size,
           NULL,
           NULL,
           mode);

}
 
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_SpiTxRx(U8 *Input, SPI_TXRX_MODE mode)
{
    U32 size;

    size = sprintf(TxBuf, Input);


    if ( mode == DMA_POLLING)
    {   
        SPI_TxRx(&Spi,Input,size,RxBuf,NULL,NULL,mode);
    }
    else
    {
        /* create SEMAPHORE */
        OSA_InitSemaphore(&tx_sem,0,1);

        SPI_TxRx(&Spi,Input,size,RxBuf,test_TxCallback,NULL,mode);

        /* wait for semaphore */ 
        OSA_TakeSemaphore(&tx_sem);   
    }



    UTILITY_PrintBuffer(RxBuf,size ,1);

}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_TxCallback(void *pThis, UINT32 value)
{    
    ets_printf("%s() Entry\n",__FUNCTION__);

    /*Release semaphore */
    OSA_GiveSemaphore(&tx_sem);
        
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
