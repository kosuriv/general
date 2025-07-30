/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : adc.h
*
*  Description         : This file defines ADC function declarations 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  9th Nov 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef ADC_H
#define ADC_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
    #include "stm32f4xx_hal.h"
#endif

#ifdef ESP32_S2_MINI
#include "driver/adc.h"
#endif

#include "data_types.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

#ifdef ESP32_S2_MINI
#define ADC_CHANNEL_BATTERY_VOLTAGE ADC_CHANNEL_0
#define ADC_CHANNEL_BATTERY_VOLTAGE_BITWIDTH   13 //ADC_WIDTH_BIT_13
#endif


/*******************************************************************************
*                          Type Declarations
*******************************************************************************/

typedef enum ADC_NO
{
    ADC_1= 1,
    ADC_2,
    ADC_3,
    ADC_MAX_NUM
} ADC_NO;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void ADC_Init();

U32 ADC_ReadValuePolling(ADC_NO             adc_no,
                         U32                channel_no);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
