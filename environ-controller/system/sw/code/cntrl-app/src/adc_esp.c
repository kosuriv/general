/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : adc_esp.c
*
*  Description         : It contains uart driver functions( common across all
                         ADCs)
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Linta       22nd Oct 2021      1.0               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "data_types.h"
#include "adc.h"
#include "osa.h"
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
#define NO_OF_SAMPLES   64          //Multisampling

#define DEFAULT_VREF    3300 

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/

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
void ADC_Init()
{
    /* make BATTERY LOAD Connect pin as output */
   // gpio_set_direction(BATTERY_TEST_PULSE_Pin, GPIO_MODE_OUTPUT);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 ADC_ReadValuePolling(ADC_NO             adc_no,
                         U32                channel_no)

{
   
    U16 value;
    U32 adc_reading = 0x00;
    U32 voltage = 0x00;
    U32 i;
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type;
    adc_bits_width_t width = ADC_CHANNEL_BATTERY_VOLTAGE_BITWIDTH;
    adc_atten_t atten = ADC_ATTEN_DB_0;
    adc_unit_t unit = ADC_UNIT_1;
    uint32_t bit = 0x1;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    switch(adc_no)
    {
        case ADC_1:
            adc1_config_width(ADC_CHANNEL_BATTERY_VOLTAGE_BITWIDTH);
           // adc1_config_channel_atten(channel_no, atten);
            break;

        case ADC_2:
            
            break;

        case ADC_3:
           
            break;
       
    }
    
   
#if 0
    //Characterize ADC at particular atten
    val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, &adc_chars);

    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) 
    {
        printf("eFuse Vref\n");
    } 
    else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) 
    {
        printf("Two Point\n");
    }
    else 
    {
        printf("Default\n");
    }
#endif
 
    for (i = 0; i < NO_OF_SAMPLES; i++)
    {           
        adc_reading += adc1_get_raw(channel_no);
    }

    adc_reading /= NO_OF_SAMPLES;

    printf("adc_reading: %x\t \n", adc_reading);

#if 0

   // esp_adc_cal_get_voltage(channel_no, &adc_chars, &voltage);
    //Convert adc_reading to voltage in mV
   voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);

    printf("Raw: %x\tVoltage: %dmV\n", adc_reading, voltage);
#endif
  
    return adc_reading;

}


/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
*                          End of File
*******************************************************************************/
