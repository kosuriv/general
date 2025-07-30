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

#include "modbus_slave.h"
#include "trace.h"
#include "sensor2.h"

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define MODBUS_SLAVE_THREAD_PRIORITY 6

#define MODBUS_SLAVE_THREAD_STACK_SIZE  2000 
#ifdef ESP32_C3
#define  CONFIG_MB_UART_TXD  GPIO_NUM_0 
#define  CONFIG_MB_UART_RXD  GPIO_NUM_1
#define  CONFIG_MB_UART_RTS  GPIO_NUM_10
#else
#define  CONFIG_MB_UART_TXD  GPIO_NUM_17
#define  CONFIG_MB_UART_RXD  GPIO_NUM_18
#define  CONFIG_MB_UART_RTS  GPIO_NUM_12
#endif
#define  CONFIG_MB_UART_BAUD_RATE    9600

#define CONFIG_MB_UART_PORT_NUM UART_NUM_1  //just check 


#define MB_PORT_NUM     (CONFIG_MB_UART_PORT_NUM)   // Number of UART port used for Modbus connection
#define MB_DEV_SPEED    (CONFIG_MB_UART_BAUD_RATE)  // The communication speed of the UART

#define SLAVE_CHECK(a, ret_val, str, ...) \
    if (!(a)) { \
        ESP_LOGE(MASTER_TAG, "%s(%u): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
        return (ret_val); \
    }
#define MB_PAR_INFO_GET_TOUT                (10) // Timeout for get parameter info
#define MB_CHAN_DATA_MAX_VAL                (6)
#define MB_CHAN_DATA_OFFSET                 (0.2f)
#define MB_READ_MASK                        (MB_EVENT_INPUT_REG_RD \
                                                | MB_EVENT_HOLDING_REG_RD \
                                                | MB_EVENT_DISCRETE_RD \
                                                | MB_EVENT_COILS_RD)
#define MB_WRITE_MASK                       (MB_EVENT_HOLDING_REG_WR \
                                                | MB_EVENT_COILS_WR)
#define MB_READ_WRITE_MASK                  (MB_READ_MASK | MB_WRITE_MASK)
/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void modbus_slave_thread_entry_func(void *arg);


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static const char *SLAVE_TAG = "MODBUS_SLAVE";

MODBUS_SLAVE*  pModbusSlave;

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
void MODBUS_Init(MODBUS_SLAVE* pThis, U32 slave_addr)
{
    void* mbc_slave_handler = NULL;
    mb_communication_info_t comm_info; // Modbus communication parameters
    mb_register_area_descriptor_t reg_area; // Modbus register area descriptor structure
    esp_err_t err = ESP_OK;
    pModbusSlave = pThis;

    memset(pModbusSlave,0x00,sizeof(MODBUS_SLAVE));

#if !SENSOR2_SENSOR_AVAILABLE_FLAG
    /* for unit testing purpose */
    pModbusSlave->sensor_value[0] = 800;
    pModbusSlave->sensor_value[1] = 26;
    pModbusSlave->sensor_value[2] = 40;
#endif
        // Set UART log level
    esp_log_level_set(SLAVE_TAG, ESP_LOG_ERROR);


    ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE, &mbc_slave_handler)); // Initialization of Modbus controller

    // Setup communication parameters and start stack
    comm_info.mode = MB_MODE_RTU;
    comm_info.slave_addr = slave_addr ;
    comm_info.port = MB_PORT_NUM;
    comm_info.baudrate = MB_DEV_SPEED;
    comm_info.parity = MB_PARITY_NONE;
    ESP_ERROR_CHECK(mbc_slave_setup((void*)&comm_info));

    reg_area.type = MB_PARAM_HOLDING; // Set type of register area
    reg_area.start_offset = 0; // Offset of register area in Modbus protocol
    reg_area.address = (void*)&pModbusSlave->height; // Set pointer to storage instance
    reg_area.size = 2;; // Set the size of register storage instance
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

    // Initialization of Input Registers area
    reg_area.type = MB_PARAM_HOLDING;
    reg_area.start_offset = 2;
    reg_area.address = (void*)pModbusSlave->sensor_value;
    reg_area.size = 6<<2;
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));

 // Starts of modbus controller and stack
    ESP_ERROR_CHECK(mbc_slave_start());

    // Set UART pin numbers
    ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD,
                            CONFIG_MB_UART_RXD, CONFIG_MB_UART_RTS,
                            UART_PIN_NO_CHANGE));

    // Set UART driver mode to Half Duplex
    ESP_ERROR_CHECK(uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX));

    ESP_LOGI(SLAVE_TAG, "Modbus slave stack initialized.");
    ESP_LOGI(SLAVE_TAG, "Start modbus test...");


    /* Create APP rx thread. It contains all handlers for the received messages & profile change funtinality  */
    OSA_ThreadCreate(pModbusSlave->modbus_slave_thread,
                     MODBUS_SLAVE_THREAD_PRIORITY ,
                     modbus_slave_thread_entry_func,
                     NULL,
                     "modbus_slave_task",
                     NULL,
                     MODBUS_SLAVE_THREAD_STACK_SIZE,
                     0);

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void MODBUS_SLAVE_Term()
{
     ESP_ERROR_CHECK(mbc_slave_destroy());
}

/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void modbus_slave_thread_entry_func(void *arg)
{
    mb_param_info_t reg_info; 

    while(1)
    {
        mb_event_group_t event = mbc_slave_check_event(MB_READ_WRITE_MASK);

        const char* rw_str = (event & MB_READ_MASK) ? "READ" : "WRITE";

        TRACE_INFO("Modbus Event Received \n");

        ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));

        // Filter events and process them accordingly
        if(event &  MB_EVENT_HOLDING_REG_WR) 
        {
            if (reg_info.address == (uint8_t*)&pModbusSlave->height)
            {
                TRACE_INFO("Setting  Altitude = %i  \n",pModbusSlave->height);

#if SENSOR2_SENSOR_AVAILABLE_FLAG
                SENSOR_SetHeight((U16) pModbusSlave->height);
#endif
            }
        }
        else if (event & MB_EVENT_HOLDING_REG_RD)   
        {

            if (reg_info.address == (uint8_t*)pModbusSlave->sensor_value)
            {
                TRACE_INFO("Reading  Sensor Values \n");

#if SENSOR2_SENSOR_AVAILABLE_FLAG
              //  SENSOR_Read(&pModbusSlave->sensor_value[0], &pModbusSlave->sensor_value[1], &pModbusSlave->sensor_value[2]);
#endif
            }
        }
      
    }
}


/*******************************************************************************
*                          End of File
*******************************************************************************/
