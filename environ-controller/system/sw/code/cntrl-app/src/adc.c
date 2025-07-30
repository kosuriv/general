/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : adc.c
*
*  Description         : It contains uart driver functions( common across all
                         ADCs)
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  9th Nov 2020      1.0               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "adc.h"
#include "osa.h"
#include "trace.h"
#include "stm32f4xx_hal_adc.h"

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static osa_mutex_type_t adc_Mutex;

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
void ADC_Init()
{
   OSA_InitMutex(&adc_Mutex);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U16 ADC_ReadValuePolling(ADC_NO             adc_no,
                         U32                channel_no)

{
    ADC_HandleTypeDef hadc = {0};
    ADC_ChannelConfTypeDef sConfig = {0};

    U16 value;

    OSA_LockMutex(&adc_Mutex);

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    switch(adc_no)
    {
        case ADC_1:
            hadc.Instance = ADC1;
            break;

        case ADC_2:
            hadc.Instance = ADC2;
            break;

        case ADC_3:
            hadc.Instance = ADC3;
            break;
       
    }

    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc.Init.Resolution = ADC_RESOLUTION_12B;
    hadc.Init.ScanConvMode = DISABLE;
    hadc.Init.ContinuousConvMode = DISABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.NbrOfConversion = 1;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc) != HAL_OK)
    {
        TRACE_INFO("%s Entry11111111111 \n", __FUNCTION__);
        Error_Handler();
    }

    /* Configure for the selected ADC regular channel its corresponding rank in
       the sequencer and its sample time.*/
    sConfig.Channel = channel_no;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
    {
        TRACE_INFO("%s Entry222222222 \n", __FUNCTION__);
        Error_Handler();
    }

    if(HAL_ADC_Start(&hadc) != HAL_OK)
    {
        TRACE_INFO("%s Entry33333333333 \n", __FUNCTION__);
        Error_Handler();   
    }

#if 0
    while(!(__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC)));
#else
    if (HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY) != HAL_OK)
    {
        TRACE_INFO("%s Entry44444444444 \n", __FUNCTION__);
        Error_Handler();       
    }
#endif

    value = HAL_ADC_GetValue(&hadc);

    TRACE_INFO("%s Exit(%d) \n", __FUNCTION__,value);

    if(HAL_ADC_Stop(&hadc) != HAL_OK)
    {
        TRACE_INFO("%s Entry555555 \n", __FUNCTION__);
        Error_Handler();   
    }

    OSA_UnLockMutex(&adc_Mutex);

    return value;

}


/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
*                          End of File
*******************************************************************************/
