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
#include "data_types.h"
#include "trace.h"
#include "uart.h"



/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define UART_TEST_BUFFSIZE 100

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void test_UartRx(UART_MODE mode);

static void test_UartTx(U8 *Input, UART_MODE mode);

static void test_RxCallback(void *pThis, UINT32 value);

static void test_TxCallback(void *pThis, UINT32 value);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static UART* DriverToTest;

static U8 RxBuf[UART_TEST_BUFFSIZE];

static U8 TxBuf[UART_TEST_BUFFSIZE];

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
void TEST_UART_Init(UART* UartDriver)
{
    
    DriverToTest = UartDriver;

    TRACE_DBG("In TEST_UART_Init() \n");

#if 1 

    test_UartTx("FIRST PACKET" , UART_MODE_0);

    test_UartRx(UART_MODE_0);

#else

    test_UartTx("SECOND PACKET in MODE 1" , UART_MODE_1);

    test_UartRx(UART_MODE_1);

#endif

}


/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_UartRx(UART_MODE mode)
{

	memset(RxBuf,0x00,sizeof(RxBuf));

    UART_Rx(DriverToTest,
            RxBuf,
            4,
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
static void test_UartTx(U8 *Input, UART_MODE mode)
{
    U32 size;

    size = sprintf(TxBuf, Input);

    UART_Tx(DriverToTest,
            TxBuf,
            size,
            test_TxCallback,
            TxBuf,
            mode);

}
 

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_TxCallback(void *pThis, UINT32 value)
{
    
    if(UART_STATUS_OK == value)
    {
        TRACE_DBG("Success In TX \n");
    }
      
}



/*******************************************************************************
*                          End of File
*******************************************************************************/
