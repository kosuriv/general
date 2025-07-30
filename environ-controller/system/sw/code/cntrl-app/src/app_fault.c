/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : app_fault.c
*
*  Description         : This file contains functions to for all EC fault 
*                        functionality                       
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*    Linta       11th oct 2021       1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "ctrl_out.h"
#include "trace.h"
#include "app.h"
/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/

#define ESP_INTR_FLAG_DEFAULT 0


/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/

static void IRAM_ATTR App_fault_gpio_isr_handler(void* arg);

static void IRAM_ATTR gpio_isr(void* arg);

static void gpioCallback(void* arg);
/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/

static APP App;

static const char *TAG = "example";

int last_status = -1;
/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/


/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/
extern App_send_fault_message(APP_MSG_TYPE  msg_id);

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_fault_esp_init()
{

#ifdef ESP32_S2_MINI
    int ret;
    gpio_config_t io_conf = {};

#define GPIO_INPUT_PIN_SEL  ((1ULL<<PANIC_MODE_SWITCH_Pin) |(1ULL<<RS485_FUSE_FAULT_Pin)|(1ULL<<HOT_RESTRIKE_FAULT_Pin)|(1ULL<<AC_POWER_FAULT_Pin) )
    
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    io_conf.intr_type = GPIO_INTR_HIGH_LEVEL;
 
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;

    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;

    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    ret = gpio_isr_register(App_fault_gpio_isr_handler, (void *)NULL, ESP_INTR_FLAG_LEVEL3, NULL);

    TRACE_INFO("GPIO ISR Registration Status  = %d \n",ret);

    gpio_intr_enable(PANIC_MODE_SWITCH_Pin);
    gpio_intr_enable(HOT_RESTRIKE_FAULT_Pin);
    gpio_intr_enable(AC_POWER_FAULT_Pin);
    gpio_intr_enable(RS485_FUSE_FAULT_Pin);


#if 0

    /* Set the GPIO as a input */
    gpio_set_direction(HOT_RESTRIKE_FAULT_Pin, GPIO_MODE_INPUT);
    gpio_set_direction(PANIC_MODE_SWITCH_Pin, GPIO_MODE_INPUT);
    gpio_set_direction(AC_POWER_FAULT_Pin, GPIO_MODE_INPUT);
    gpio_set_direction(RS485_FUSE_FAULT_Pin, GPIO_MODE_INPUT);

    /* Set interrupt type */
    gpio_set_intr_type(HOT_RESTRIKE_FAULT_Pin, GPIO_INTR_HIGH_LEVEL);
   // gpio_set_intr_type(PANIC_MODE_SWITCH_Pin, GPIO_INTR_HIGH_LEVEL);
    gpio_set_intr_type(AC_POWER_FAULT_Pin, GPIO_INTR_HIGH_LEVEL);
    gpio_set_intr_type(RS485_FUSE_FAULT_Pin, GPIO_INTR_HIGH_LEVEL);
       

    gpio_set_intr_type(PANIC_MODE_SWITCH_Pin, GPIO_INTR_HIGH_LEVEL);

    // Intterrupt number see below
 //   ret = gpio_isr_register(&gpio_isr, (void *)NULL, ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_INTRDISABLED, NULL);
    ret = gpio_isr_register(App_fault_gpio_isr_handler, (void *)NULL, ESP_INTR_FLAG_LEVEL3, NULL);

    printf("GPIO ISR Registration Status  = %d \n",ret);

    gpio_intr_enable(HOT_RESTRIKE_FAULT_Pin);
    gpio_intr_enable(PANIC_MODE_SWITCH_Pin);
    gpio_intr_enable(AC_POWER_FAULT_Pin);
    gpio_intr_enable(RS485_FUSE_FAULT_Pin);
 //  gpio_intr_disable(PANIC_MODE_SWITCH_Pin);

#endif
#endif
 
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_fault_process_ac_power_fault()
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
    if(HAL_GPIO_ReadPin(AC_POWER_FAULT_GPIO_Port, AC_POWER_FAULT_Pin))
    {
        if(!App.GrowStatus.ac_power_fault)
        {
            App.GrowStatus.ac_power_fault = TRUE;

            if(App.WorkingProfile.GrowInfo.to_report_ac_power_fault)
            {
                /* Send Alert Message indicating fault occured   */
                app_send_fault_info();
            }           
            DEVICE_FaultHandler(TRUE);
        }
    }
    else
    {
        if(App.GrowStatus.ac_power_fault)
        {
            /* Send Alert Message indicating fault cleared  */
            App.GrowStatus.ac_power_fault = FALSE;
            if(App.WorkingProfile.GrowInfo.to_report_ac_power_fault)
            {
                /* Send Alert Message indicating fault cleared   */
                app_send_fault_info();
            }
            /*  while starting light device , take restrike timer into account */
            DEVICE_FaultHandler(FALSE); 
        }               
    }
#endif

#ifdef ESP32_S2_MINI
    if(gpio_get_level(AC_POWER_FAULT_Pin))
    {
        if(!App.GrowStatus.ac_power_fault)
        {
            App.GrowStatus.ac_power_fault = TRUE;

            if(App.WorkingProfile.GrowInfo.to_report_ac_power_fault)
            {
                printf("ac power fault occured\n");
                /* Send Alert Message indicating fault occured   */
              //  app_send_fault_info();
            }           
            DEVICE_FaultHandler(TRUE);
        }
    }
    else
    {
        if(App.GrowStatus.ac_power_fault)
        {
            /* Send Alert Message indicating fault cleared  */
            App.GrowStatus.ac_power_fault = FALSE;
            if(App.WorkingProfile.GrowInfo.to_report_ac_power_fault)
            {
                printf("ac power fault cleared\n");
                /* Send Alert Message indicating fault cleared   */
              //  app_send_fault_info();
            }
            /*  while starting light device , take restrike timer into account */
            DEVICE_FaultHandler(FALSE); 
        }               
    }
#endif
 
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_fault_process_hot_restrike_fault()
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
    if(HAL_GPIO_ReadPin(HOT_RESTRIKE_FAULT_GPIO_Port, HOT_RESTRIKE_FAULT_Pin))
    {
        if(!App.GrowStatus.hot_restrike_fault)
        {
            App.GrowStatus.hot_restrike_fault = TRUE;

            if(App.WorkingProfile.GrowInfo.to_report_hot_restrike_fault)
            {
                /* Send Alert Message indicating fault occured   */
              //  app_send_fault_info();
            }           
            DEVICE_FaultHandler(TRUE);
        }
    }
#endif

#ifdef ESP32_S2_MINI

    printf("hot_restrike_fault occured11111\n");
    if(gpio_get_level(HOT_RESTRIKE_FAULT_Pin))
    {
        if(!App.GrowStatus.hot_restrike_fault)
        {
            App.GrowStatus.hot_restrike_fault = TRUE;

            if(App.WorkingProfile.GrowInfo.to_report_hot_restrike_fault)
            {
                printf("hot_restrike_fault occured\n");
                /* Send Alert Message indicating fault occured   */
              //  app_send_fault_info();
            }           
            DEVICE_FaultHandler(TRUE);
        }
    }
    else
    {
        printf("hot_restrike_fault cleared\n");
    } 
#endif

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_fault_process_leak_mode()
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
    if(HAL_GPIO_ReadPin(LEAK_MODE_GPIO_Port, LEAK_MODE_Pin))
    {
        DEVICE_LeakModeExecute(TRUE);
    }
    else
    {
        DEVICE_LeakModeExecute(FALSE);          
    }
#endif

#if 0
#ifdef ESP32_S2_MINI
    if(gpio_get_level(LEAK_MODE_Pin))
    {
        DEVICE_LeakModeExecute(TRUE);
    }
    else
    {
        DEVICE_LeakModeExecute(FALSE);          
    }
#endif
#endif


}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_fault_process_panic_switch()
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
    if(HAL_GPIO_ReadPin(PANIC_MODE_SWITCH_GPIO_Port, PANIC_MODE_SWITCH_Pin))
    {
        App.GrowStatus.panic_switch_on = FALSE;
        DEVICE_FaultHandler(FALSE);
    }
    else
    {
        App.GrowStatus.panic_switch_on = TRUE;
        DEVICE_FaultHandler(TRUE);      
    }
#endif

#ifdef ESP32_S2_MINI

    gpio_int_type_t intr_type;
    int level;
    level = gpio_get_level(PANIC_MODE_SWITCH_Pin);
    if(level)
    {      
        App.GrowStatus.panic_switch_on = TRUE;
        DEVICE_FaultHandler(TRUE); 
    }
    else
    {      
        App.GrowStatus.panic_switch_on = FALSE;
        DEVICE_FaultHandler(FALSE);
           
    }
    
#endif

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_fault_process_rs485_fuse_fault()
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
    if(HAL_GPIO_ReadPin(RS485_FUSE_FAULT_GPIO_Port, RS485_FUSE_FAULT_Pin))
    {
        if(App.GrowStatus.rs485_fuse_fault)
        {
            App.GrowStatus.rs485_fuse_fault = FALSE;
            if(App.WorkingProfile.GrowInfo.to_report_rs485_fuse_fault)
            {
                /* Send Alert Message indicating fault cleared  */
                app_send_fault_info();
            }
            DEVICE_FaultHandler(FALSE);
        }

    }
    else
    {
        if(!App.GrowStatus.rs485_fuse_fault)
        {
            App.GrowStatus.rs485_fuse_fault = TRUE; 
            if(App.WorkingProfile.GrowInfo.to_report_rs485_fuse_fault)
            {
                /* Send Alert Message indicating fault there   */
                app_send_fault_info();
            }
            DEVICE_FaultHandler(TRUE);
        }   
    }
#endif

#ifdef ESP32_S2_MINI
    if(gpio_get_level(RS485_FUSE_FAULT_Pin))
    {
        if(App.GrowStatus.rs485_fuse_fault)
        {
            printf("rs485_fuse_fault cleared1111\n");
            App.GrowStatus.rs485_fuse_fault = FALSE;
            if(App.WorkingProfile.GrowInfo.to_report_rs485_fuse_fault)
            {
                printf("rs485_fuse_fault cleared\n");
                /* Send Alert Message indicating fault cleared  */
              //  app_send_fault_info();
            }
            DEVICE_FaultHandler(FALSE);
        }

    }
    else
    {
        if(!App.GrowStatus.rs485_fuse_fault)
        {
            printf("rs485_fuse_fault cleared1111\n");
            App.GrowStatus.rs485_fuse_fault = TRUE; 
            if(App.WorkingProfile.GrowInfo.to_report_rs485_fuse_fault)
            {
                printf("rs485_fuse_fault occured\n");
                /* Send Alert Message indicating fault there   */
              //  app_send_fault_info();
            }
            DEVICE_FaultHandler(TRUE);
        }   
    }
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
static void IRAM_ATTR App_fault_gpio_isr_handler(void* arg)
{

    uint32_t gpio_intr_status1 = READ_PERI_REG(GPIO_STATUS_REG);   //read status to get interrupt status for GPIO0-31

  //  uint32_t gpio_intr_status2 = READ_PERI_REG(GPIO_STATUS1_REG);   //read status to get interrupt status for GPIO32-63

    uint32_t bit_no = 0x1;

    U32 value ;

    ets_printf("%s() Entry GPIO0-31 = (%x) \n",__FUNCTION__,gpio_intr_status1);
    
    if( (bit_no << PANIC_MODE_SWITCH_Pin) & gpio_intr_status1)
    {
        value = gpio_get_level(PANIC_MODE_SWITCH_Pin);

        ets_printf("%s() PANIC  MODE ISR  Value = %d   \n",__FUNCTION__,value);

        if(value == 1)
        {
            ets_printf("%s() PANIC  MODE SET  \n",__FUNCTION__);
            gpio_set_intr_type(PANIC_MODE_SWITCH_Pin, GPIO_INTR_LOW_LEVEL);
        }
        else
        {
            ets_printf("%s() PANIC  MODE RESET  \n",__FUNCTION__);
            gpio_set_intr_type(PANIC_MODE_SWITCH_Pin, GPIO_INTR_HIGH_LEVEL);
        }
        App_send_fault_message(APP_PANIC_BUTTON_MSG);        
    }
    if( (bit_no << HOT_RESTRIKE_FAULT_Pin) & gpio_intr_status1)
    {
        value = gpio_get_level(HOT_RESTRIKE_FAULT_Pin);

        ets_printf("%s() HOT RESTRIKE FAULT ISR  Value = %d   \n",__FUNCTION__,value);

        if(value == 1)
        {
            ets_printf("%s() HOT RESTRIKE FAULT SET  \n",__FUNCTION__);
            gpio_set_intr_type(HOT_RESTRIKE_FAULT_Pin, GPIO_INTR_LOW_LEVEL);
        }
        else
        {
            ets_printf("%s() HOT RESTRIKE FAULT RESET  \n",__FUNCTION__);
            gpio_set_intr_type(HOT_RESTRIKE_FAULT_Pin, GPIO_INTR_HIGH_LEVEL);
        }
        App_send_fault_message(APP_HOT_RESTRIKE_FAULT_MSG);
    }
    if( (bit_no << AC_POWER_FAULT_Pin) & gpio_intr_status1)
    {
        value = gpio_get_level(AC_POWER_FAULT_Pin);

        ets_printf("%s() AC POWER FAULT ISR  Value = %d   \n",__FUNCTION__,value);


        if(value == 1)
        {
            ets_printf("%s() AC POWER FAULT SET  \n",__FUNCTION__);
            gpio_set_intr_type(AC_POWER_FAULT_Pin, GPIO_INTR_LOW_LEVEL);
        }
        else
        {
            ets_printf("%s() AC POWER FAULT RESET  \n",__FUNCTION__);
            gpio_set_intr_type(AC_POWER_FAULT_Pin, GPIO_INTR_HIGH_LEVEL);
        }
        App_send_fault_message(APP_AC_POWER_FAULT_MSG);   
    }
    if( (bit_no << RS485_FUSE_FAULT_Pin) & gpio_intr_status1)
    {
        value = gpio_get_level(RS485_FUSE_FAULT_Pin);

        ets_printf("%s() RS485 FUSE FAULT ISR  Value = %d   \n",__FUNCTION__,value);


        if(value == 1)
        {
            ets_printf("%s() RS485 FUSE FAULT SET  \n",__FUNCTION__);
            gpio_set_intr_type(RS485_FUSE_FAULT_Pin, GPIO_INTR_LOW_LEVEL);
        }
        else
        {
            ets_printf("%s() RS485 FUSE FAULT RESET  \n",__FUNCTION__);
            gpio_set_intr_type(RS485_FUSE_FAULT_Pin, GPIO_INTR_HIGH_LEVEL);
        }
        App_send_fault_message(APP_RS485_FUSE_FAULT_MSG);   
    }

    /* Clear the interrupts  */
    SET_PERI_REG_MASK(GPIO_STATUS_W1TC_REG, gpio_intr_status1); 

   // SET_PERI_REG_MASK(GPIO_STATUS1_W1TC_REG, gpio_intr_status2);
}

/*******************************************************************************
*                          End of File
*******************************************************************************/

