/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : uart.h
*
*  Description         : This file defines UART function declarations 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  29th July 2020    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef UART_H
#define UART_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "stm32f4xx_hal.h"
#include "data_types.h"
#include "osa.h"

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

typedef enum UART_NO
{
    UART1_HW_INDEX = 1,
    UART2_HW_INDEX,
    UART3_HW_INDEX,
    UART4_HW_INDEX,
    UART5_HW_INDEX,
    UART6_HW_INDEX,
    UART7_HW_INDEX,
    UART8_HW_INDEX,
    UART_MAX_NUM
} UART_NO;


typedef enum UART_MODE
{
    UART_MODE_0, /* receives or transmits specific size */
    UART_MODE_1,  /* receives or transmits till \r\n(up to new line) */
    UART_MODE_MODBUS_RTU 
} UART_MODE;

typedef enum UART_STATUS
{
    UART_STATUS_OK,
    UART_STATUS_BUSY,
    UART_STATUS_ERROR
} UART_STATUS;

typedef struct UART_TXRXCHANNEL
{
    BUFFER8            pData;
    CALLBACK32         callback;
    UINT32             flag;      /* 0 means receive certain length of characters */   
                                  /* 1 means receive till \r\n  */ 
                                  /* 2 means receive till timeout  t3.5 in Modbus, transmit after t3.5  */  
    osa_timer_type_t   timer_3_5; 

} UART_TXRXCHANNEL;

typedef struct UART
{
    UART_NO  uart_no;

    UART_HandleTypeDef   huart;

    UART_TXRXCHANNEL  tx;

    UART_TXRXCHANNEL  rx;

    U32                timer_val_3_5;

    U32                timer_val_1_5;

} UART;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void UART_Init(UART          *pUart,
               U32           baudrate,
               UART_NO       uart_no,
               U32           parity, 
               UART_MODE     mode);

void UART_Isr(IRQn_Type Irq_No) ;

UART_STATUS UART_Rx(UART               *pThis,
                    BYTE               *pBuf,
                    UINT32             size,
                    CALLBACK32_FCT     pFunction,
                    void               *pArg,
                    UART_MODE          mode);

UART_STATUS UART_Tx(UART               *pThis,
                    BYTE               *pBuf,
                    UINT32             size,
                    CALLBACK32_FCT     pFunction,
                    void               *pArg,
                    UART_MODE          mode);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
