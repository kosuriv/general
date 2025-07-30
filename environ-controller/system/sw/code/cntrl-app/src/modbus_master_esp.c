/*******************************************************************************
* (C) Copyright 2022;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : modbus_master.c
*
*  Description         : It contains modbus functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  21st Sep  2022    1.0               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "esp_log.h"
#include "mbcontroller.h"
#include "sdkconfig.h"
#include "gpio.h"

#include "modbus_master.h"
#include "trace.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"
/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/

#ifdef ESP32_S3_MINI
#define  CONFIG_MB_UART_TXD  GPIO_NUM_17
#define  CONFIG_MB_UART_RXD  GPIO_NUM_18
#define  CONFIG_MB_UART_RTS  GPIO_NUM_12
#else
#define  CONFIG_MB_UART_TXD  GPIO_NUM_17
#define  CONFIG_MB_UART_RXD  GPIO_NUM_18
#define  CONFIG_MB_UART_RTS  GPIO_NUM_12
#endif

#define  CONFIG_MB_UART_BAUD_RATE    9600

#define CONFIG_MB_UART_PORT_NUM UART_NUM_1  //just check 

#define MB_PORT_NUM     (CONFIG_MB_UART_PORT_NUM)   // Number of UART port used for Modbus connection
#define MB_DEV_SPEED    (CONFIG_MB_UART_BAUD_RATE)  // The communication speed of the UART

#define MASTER_TAG "MODBUS_MASTER"


#define MASTER_CHECK(a, ret_val, str, ...) \
    if (!(a)) { \
        ESP_LOGE(MASTER_TAG, "%s(%u): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return (ret_val); \
    }





/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void modbus_master_init_param(U32 slave_addr,FC_CODE fc , mb_parameter_descriptor_t* pDesc);


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static  mb_parameter_descriptor_t modbus_master_device_parameters[2];

static  BOOLEAN modbus_master_is_descriptor_set = FALSE;
static  U8 name1[10];
static  U8 name2[10];
static  U8 type = 0;
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
void MODBUS_Init()
{
    // Initialize and start Modbus controller
    mb_communication_info_t comm = {
            .port = MB_PORT_NUM,
            .mode = MB_MODE_RTU,
            .baudrate = MB_DEV_SPEED,
            .parity = MB_PARITY_NONE
    };
    void* master_handler = NULL;
    esp_err_t err;

        // Set UART log level
    esp_log_level_set(MASTER_TAG, ESP_LOG_ERROR);

    err = mbc_master_init(MB_PORT_SERIAL_MASTER, &master_handler);

    MASTER_CHECK((master_handler != NULL), ESP_ERR_INVALID_STATE,
                                "mb controller initialization fail.");
    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                            "mb controller initialization fail, returns(0x%x).",
                            (uint32_t)err);
    err = mbc_master_setup((void*)&comm);
    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                            "mb controller setup fail, returns(0x%x).",
                            (uint32_t)err);

    // Set UART pin numbers
    err = uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD, CONFIG_MB_UART_RXD,
                              CONFIG_MB_UART_RTS, UART_PIN_NO_CHANGE);

    err = mbc_master_start();
    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
                            "mb controller start fail, returns(0x%x).",
                            (uint32_t)err);

   // Set driver mode to Half Duplex
    err = uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX);


    vTaskDelay(5);

    MASTER_CHECK((err == ESP_OK), ESP_ERR_INVALID_STATE,
            "mb serial set mode failure, uart_set_mode() returned (0x%x).", (uint32_t)err);

    modbus_master_is_descriptor_set = FALSE;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void MODBUS_Term()
{
    ESP_ERROR_CHECK(mbc_master_destroy());
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN MODBUS_GetSetParam(U32 slave_addr ,FC_CODE fc , void* buf)
{
 //   U8 name1[10];
 //  U8 name2[10];
 //   U8 type = 0;
    esp_err_t err = ESP_OK;
    U32 i;
    BOOLEAN status = FALSE;

    TRACE_INFO("%s Entry \n", __FUNCTION__);
#if 0 
    mb_parameter_descriptor_t param_descriptor;
    memset(&param_descriptor,0x00,sizeof(mb_parameter_descriptor_t));
    modbus_master_init_param(slave_addr, fc , &param_descriptor);
    param_descriptor.param_key = name;

    err = mbc_master_set_descriptor(&param_descriptor,1);

    TRACE_DBG("%s() Device Descriptor Set  err = %x\n",__FUNCTION__, err);
#else

    mb_parameter_descriptor_t* param_descriptor;
    if(!modbus_master_is_descriptor_set)
    {
        for(i=0; i<2; i++)
        {
            param_descriptor = &modbus_master_device_parameters[i];
            memset(param_descriptor,0x00,sizeof(mb_parameter_descriptor_t));
            if (i == 0)
            {  
                modbus_master_init_param(slave_addr, FC_SET_SENSOR_HEIGHT , param_descriptor);
                param_descriptor->param_key = name1;
            }
            else if (i == 1)
            {
                modbus_master_init_param(slave_addr, FC_GET_SENSOR_DATA , param_descriptor);
                param_descriptor->param_key = name2;
            }
        }
        err = mbc_master_set_descriptor(&modbus_master_device_parameters[0],2);
        TRACE_DBG("%s() Device Descriptor Set  err = %x\n",__FUNCTION__, err);
        modbus_master_is_descriptor_set = TRUE;
    }
#endif
    switch (fc)
    {
        case FC_SET_SENSOR_HEIGHT:
            param_descriptor = &modbus_master_device_parameters[0];
            strcpy(name1,"Height");
            TRACE_INFO("%s SETTING HEIGHT \n", __FUNCTION__);
            err = mbc_master_set_parameter(FC_SET_SENSOR_HEIGHT, (char*)param_descriptor->param_key,(U8*)buf, &type);
            if (err == ESP_OK) 
            {   
                TRACE_DBG("%s() FC_SET_SENSOR_HEIGHT SUCCESS  = %x\n",__FUNCTION__, err);
                status = TRUE; 
            }
            else
            {
                TRACE_DBG("%s() FC_SET_SENSOR_HEIGHT FAIL  = %x\n",__FUNCTION__, err); 
                status = FALSE;              
            }

            break;
       case FC_GET_SENSOR_DATA:
            param_descriptor = &modbus_master_device_parameters[1];
            strcpy(name2,"Value");
            TRACE_INFO("%s READING SENSOR  \n", __FUNCTION__);
            err = mbc_master_get_parameter(FC_GET_SENSOR_DATA, (char*)param_descriptor->param_key,(uint8_t*)buf, &type);
            if (err == ESP_OK) 
            { 
                TRACE_DBG("%s() FC_GET_SENSOR_DATA SUCCESS   = %x\n",__FUNCTION__, err); 
                status = TRUE;            
            }
            else 
            {
                TRACE_DBG("%s() Entry FC_GET_SENSOR_DATA FAIL   = %x\n",__FUNCTION__, err); 
                status = FALSE;               
            }
            break;
    }
    return status;
}

/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void modbus_master_init_param(U32 slave_addr,FC_CODE fc , mb_parameter_descriptor_t* pDesc)
{
    pDesc->cid = (U16)fc;

    pDesc->mb_slave_addr = slave_addr;
    pDesc->param_opts.opt1 = 0x00;
    pDesc->param_opts.opt2 = 0x00;
    pDesc->param_opts.opt3 = 0x00;
    switch (fc)
    {
        case FC_SET_SENSOR_HEIGHT:
            pDesc->param_offset = 0x00;  
           // pDesc->param_units = PARAM_TYPE_U32; units such as volts , celsius etc ...
            pDesc->mb_param_type = MB_PARAM_HOLDING;
            pDesc->mb_reg_start = 0x0;
            pDesc->mb_size = 1;
          //  pDesc->param_type = PARAM_TYPE_U16;
           // pDesc->param_size  = 2 ; //number of bytes 
            pDesc->access = PAR_PERMS_WRITE ;
            break;

        case FC_GET_SENSOR_DATA:
            pDesc->param_offset = 0x02; 
           // pDesc->param_units = PARAM_TYPE_U32;
            pDesc->mb_param_type = MB_PARAM_HOLDING;
            pDesc->mb_reg_start = 0x02;
            pDesc->mb_size = 6 ; //TBD
          //  pDesc->param_type = PARAM_TYPE_U8;
           // pDesc->param_size  = 6 ; //number of bytes
            pDesc->access = PAR_PERMS_READ ;
            break;
    }

}

/*******************************************************************************
*                          End of File
*******************************************************************************/
