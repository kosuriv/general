/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : uart.c
*
*  Description         : It contains uart driver functions( common across all
                          USARTS)
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  29th July 2020    1.0               Initial Creation
*   Venu kosuri  26th Aug  2020    1.1           UART interrupts added & tested
*   Venu kosuri  18th Nov  2020    1.2               MODBUS mode added
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <math.h>

#include "data_types.h"
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
#define IT_MASK                   ((uint16_t)0xFFFF)

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void uart_Prepare(UART_TXRXCHANNEL   *pThis,
                         BYTE               *pBuf,
                         UINT32             size,
                         CALLBACK32_FCT     pFunction,
                         void               *pArg,
                         UART_MODE          mode);

static void uart_ProcessRx(UART *pThis);

static void uart_ProcessTx(UART *pThis);

static void uart_modbus_rtu_tx_handler(void *arg);

static void uart_modbus_rtu_rx_handler(void *arg);

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
void UART_Init(UART          *pUart,
               U32           baudrate,
			   UART_NO       uart_no,
               U32           parity, 
               UART_MODE     mode)

{
	FLOAT val;
    switch(uart_no)
    {
        case UART1_HW_INDEX:
            pUart->huart.Instance = USART1;
            APP_intr_arg_list[USART1_IRQn] = pUart;
            break;

        case UART2_HW_INDEX:
            pUart->huart.Instance = USART2;
            APP_intr_arg_list[USART2_IRQn] = pUart;
            break;

        case UART3_HW_INDEX:
            pUart->huart.Instance = USART3;
            APP_intr_arg_list[USART3_IRQn] = pUart;
            break;
        case UART4_HW_INDEX:
            pUart->huart.Instance = UART4;
            APP_intr_arg_list[UART4_IRQn] = pUart;
            break;
        case UART5_HW_INDEX:
            pUart->huart.Instance = UART5;
            APP_intr_arg_list[UART5_IRQn] = pUart;
            break;

        case UART6_HW_INDEX:
            pUart->huart.Instance = USART6;
            APP_intr_arg_list[USART6_IRQn] = pUart;
            break;

        case UART7_HW_INDEX:
            pUart->huart.Instance = UART7;
            APP_intr_arg_list[UART7_IRQn] = pUart;
            break;

        case UART8_HW_INDEX:
            pUart->huart.Instance = UART8;
            APP_intr_arg_list[UART8_IRQn] = pUart;
            break;
    }
    pUart->uart_no = uart_no;
    pUart->huart.Init.BaudRate = baudrate;
    pUart->huart.Init.WordLength = UART_WORDLENGTH_8B;
    pUart->huart.Init.StopBits = UART_STOPBITS_1;
  //  pUart->huart.Init.Parity = UART_PARITY_NONE;
    pUart->huart.Init.Parity = parity;
    pUart->huart.Init.Mode = UART_MODE_TX_RX;
    pUart->huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    pUart->huart.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&pUart->huart) != HAL_OK)
    {
        Error_Handler();
    }

    if( mode == UART_MODE_MODBUS_RTU)
    {
        /* Initialise timers */
        val = 1000000 * (1 +  pUart->huart.Init.WordLength + (pUart->huart.Init.Parity == UART_PARITY_NONE ? 0 : 1) + pUart->huart.Init.StopBits) / pUart->huart.Init.BaudRate;
        pUart->timer_val_1_5 = round((val*1.5));
        pUart->timer_val_3_5 = round((val*3.5));

        OSA_InitTimer(&pUart->tx.timer_3_5, pUart->timer_val_3_5, FALSE, uart_modbus_rtu_tx_handler, pUart);

        OSA_InitTimer(&pUart->rx.timer_3_5, pUart->timer_val_3_5, FALSE, uart_modbus_rtu_rx_handler, pUart);
    }
   
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
UART_STATUS UART_Tx(UART               *pThis,
                    BYTE               *pBuf,
                    UINT32             size,
                    CALLBACK32_FCT     pFunction,
                    void               *pArg,
                    UART_MODE          mode)
{
    float val;

    uart_Prepare(&pThis->tx,pBuf,size, pFunction,pArg,mode);

    if( mode == UART_MODE_MODBUS_RTU)
    {
        OSA_StartTimer(&pThis->tx.timer_3_5);
    }
    else
    {
        __HAL_UART_ENABLE_IT(&pThis->huart,UART_IT_TXE);
    }

    return UART_STATUS_OK;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
UART_STATUS UART_Rx(UART               *pThis,
                    BYTE               *pBuf,
                    UINT32             size,
                    CALLBACK32_FCT     pFunction,
                    void               *pArg,
                    UART_MODE          mode)
{    
    uart_Prepare(&pThis->rx,pBuf,size, pFunction,pArg,mode);

    if( mode == UART_MODE_MODBUS_RTU)
    {
        pThis->rx.pData.size = 0x00;
    }

    __HAL_UART_ENABLE_IT(&pThis->huart,UART_IT_RXNE);
 	 	 
    return UART_STATUS_OK;
}


//volatile U8 ch;

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void UART_Isr(IRQn_Type Irq_No)
{
    UART *pUart = (UART*) APP_intr_arg_list[Irq_No];
    //USART_TypeDef *reg = pUart->huart.Instance;

    volatile U32 itmask;
    volatile U32 it_status_bit = RESET ;
    volatile U32 it_enable_bit = RESET;

    /* Check RX Interrupt */
    itmask = UART_IT_RXNE & IT_MASK;
    it_status_bit =  pUart->huart.Instance->SR & itmask;
    it_enable_bit = pUart->huart.Instance->CR1 & itmask;
    if( (it_status_bit != RESET) && (it_enable_bit != RESET))
    {
#if 1 
        if(pUart->uart_no == UART2_HW_INDEX)
        {
            U8 ch;
            ch = (uint8_t)(pUart->huart.Instance->DR & (uint8_t)0x00FF);
            pUart->rx.callback.pFunction(&ch, UART_STATUS_OK);
        }
        else
#endif
        {
            uart_ProcessRx(pUart);           
        }
    }

    /* Check TX Interrupt */
    itmask = UART_IT_TXE & IT_MASK;
    it_status_bit =  pUart->huart.Instance->SR & itmask;
    it_enable_bit = pUart->huart.Instance->CR1 & itmask;
    if( (it_status_bit != RESET) && (it_enable_bit != RESET))
    {
    	uart_ProcessTx(pUart);
    }
}


/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void uart_Prepare(UART_TXRXCHANNEL   *pThis,
                         BYTE               *pBuf,
                         UINT32             size,
                         CALLBACK32_FCT     pFunction,
                         void               *pArg,
                         UART_MODE          mode)
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
* Remarks    : 
*******************************************************************************/
static void  uart_ProcessRx(UART *pThis)
{
    CALLBACK32 *const pCallback = &pThis->rx.callback;

    BOOLEAN call_flag = FALSE; 

    static U32  prev_time;

    U32 cur_time;

    if(pThis->rx.pData.pBuf == NULL)
        return;   
    
    switch(pThis->rx.flag)
    {
        case UART_MODE_0 :
        	//HAL_UART_Receive(&pThis->huart, (U8 *)pThis->rx.pData.pBuf, 1, 0xFFFF);
        	 *pThis->rx.pData.pBuf = (uint8_t)(pThis->huart.Instance->DR & (uint8_t)0x00FF);
        	pThis->rx.pData.size--;
        	if (pThis->rx.pData.size == 0)
        	{
        		call_flag = TRUE;
        	}
        	else
        	{
        		pThis->rx.pData.pBuf++;
        	}
        	break;

         case UART_MODE_1:
        	 HAL_UART_Receive(&pThis->huart, (U8 *)pThis->rx.pData.pBuf, 1, 0xFFFF);

        	 if((*pThis->rx.pData.pBuf == '\n')&& ((*(pThis->rx.pData.pBuf-1) == '\r')))
        	 {
        		 *((pThis->rx.pData.pBuf)+1) = '\0';
        		 call_flag = TRUE;
        	 }
        	 else
        	 {
        		 pThis->rx.pData.pBuf++;
        	 }
        	 break;

        case UART_MODE_MODBUS_RTU:
            HAL_UART_Receive(&pThis->huart, (U8 *)pThis->rx.pData.pBuf, 1, 0xFFFF);
            cur_time = OSA_get_time();
            OSA_StopTimer(&pThis->tx.timer_3_5);
            pThis->rx.pData.size++;
            pThis->rx.pData.pBuf++;
            OSA_StartTimer(&pThis->tx.timer_3_5);

            if (pThis->rx.pData.size == 1 )
            {
                prev_time = cur_time;
            }
            else
            {
                if((cur_time - prev_time) > pThis->timer_val_1_5)
                {
                    /* ignore the packet */
                    pThis->rx.pData.pBuf -=pThis->rx.pData.size ;
                    pThis->rx.pData.size = 0x00; 
                    OSA_StopTimer(&pThis->tx.timer_3_5);                
                }
            }          
            break;
    }

    if( call_flag)
    {
    	__HAL_UART_DISABLE_IT(&pThis->huart,UART_IT_RXNE);
    	if (pCallback->pFunction != NULL)
        {
            pThis->rx.pData.pBuf = NULL;            
            pCallback->pFunction(pCallback->pArg, UART_STATUS_OK);
        }
    }
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void uart_ProcessTx(UART *pThis)
{
    CALLBACK32 *const pCallback = &pThis->tx.callback;
    BOOLEAN call_flag = FALSE;    

    switch(pThis->tx.flag)
    {
        case UART_MODE_0 :
        case UART_MODE_MODBUS_RTU:
        	// HAL_UART_Transmit(&pThis->huart, pThis->tx.pData.pBuf, 1, 0xFFFF);
        	 pThis->huart.Instance->DR = (*pThis->tx.pData.pBuf & (uint8_t)0xFF);
        	 pThis->tx.pData.size--;
        	 if (pThis->tx.pData.size == 0)
        	 {
        		 call_flag = TRUE;
        	 }
        	 else
        	 {
        		 pThis->tx.pData.pBuf++;
        	 }
        	 break;
        
         case UART_MODE_1:
        	 HAL_UART_Transmit(&pThis->huart, pThis->tx.pData.pBuf, 1, 0xFFFF);
        	 if((*pThis->tx.pData.pBuf == '\n')&&((*(pThis->tx.pData.pBuf-1) == '\r')))
        	 {
        		 call_flag = TRUE;
        	 }
        	 else
        	 {
        		 pThis->tx.pData.pBuf++;
        	 }
        	 break;
    }

    if( call_flag)
    {
    	__HAL_UART_DISABLE_IT(&pThis->huart,UART_IT_TXE);

        if (pCallback->pFunction != NULL)
        {            
            pCallback->pFunction(pCallback->pArg, UART_STATUS_OK);
        }
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void uart_modbus_rtu_tx_handler(void *arg)
{
    UART  *pThis  = (UART*)arg;

     __HAL_UART_ENABLE_IT(&pThis->huart,UART_IT_TXE);

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void uart_modbus_rtu_rx_handler(void *arg)
{
    UART  *pThis  = (UART*)arg;

    CALLBACK32 *const pCallback = &pThis->rx.callback;

    __HAL_UART_DISABLE_IT(&pThis->huart,UART_IT_RXNE);

    if (pCallback->pFunction != NULL)
    {
        pThis->rx.pData.pBuf = NULL;            
        pCallback->pFunction(pCallback->pArg, pThis->rx.pData.size);
    }
}


/*******************************************************************************
*                          End of File
*******************************************************************************/
