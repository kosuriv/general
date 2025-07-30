/*******************************************************************************
* (C) Copyright 2019;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : app.c
*
*  Description         : This is main controller application code 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  23rd July 2020    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "app.h"
#include "app_setup.h"
#include "board.h"
#include "trace.h"
#include "jsmn.h"
#include "device_wrapper.h"
#include "ctrl_out.h"
#include "app_profile.h"
#include "app_fwug.h"
#include "smtp.h"
#include "adc.h"

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define APP_PROFILE_ACTION_TIMEOUT  24*60*60  //24 hrs


#define APP_LOCK_MSG_Q_MUTEX(p1,p2)                 \
do {                                            \
    if (p1 == FALSE) {                           \
        OSA_LockMutex(p2);  \
    }                                           \
} while (0);


#define APP_UNLOCK_MSG_Q_MUTEX(p1,p2)                 \
do {                                            \
    if (p1 == FALSE) {                           \
        OSA_UnLockMutex(p2);  \
    }                                           \
} while (0);

#define APP_OPEN_SD_CARD_FILE_ONCE 1 

#define APP_SD_CARD_FLAG   1 

#define APP_WEB_PROVISIONING_FLAG    0 

#define APP_MODBUS_FLAG  1

#define APP_MODBUS_SLAVE_ADDRESS  0x2a

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void app_rx_mqtt_message(void* pArg, BUFFER8* pBuffer );

static void app_action_thread_entry_func(void *arg);

static BOOLEAN  app_send_msg_req_to_q(U32 APP_MSG_TYPE, void* buf,U32 len,BOOLEAN is_isr_context);

static void app_release_msg_req(App_MsgReq *pMsg);

static void app_mqtt_msg_handler(U8 *pBuf, U32 len);

static void app_mqtt_process_grow_cycle_request(U8*        pBuf,
                                                jsmntok_t* pTok,
                                                U32        token_count);

static void app_mqtt_process_profile_request(U8*        pBuf,
                                             jsmntok_t* pTok,
                                             U32        token_count);

static void app_mqtt_process_provision_request(U8*        pBuf,
                                               jsmntok_t* pTok,
                                               U32        token_count);

static void app_do_start_grow_cycle(U32 profile_no,BOOLEAN user_profile,U8* pDate, U8* pTime);

static void app_do_stop_grow_cycle();

static void app_do_execute_profile();

static void app_do_stop_profile();

static void app_device_init();

static void app_update_growcycle_status_parameters(GrowConfig_GrowCycleStatus* pGrowStatus);

static void app_grow_cycle_status_handler(void *arg);

static void app_grow_cycle_send_status();

static BOOLEAN app_check_to_start_grow_cycle_or_not(U8* pDate, U8* pTime);

static void app_profile_rtc_alarm_handler(void* Arg , U32 flag);

static void app_grow_cycle_rtc_alarm_handler(void* Arg , U32 flag);

static void app_begin_grow_cycle();

static void app_grow_cycle_alarm_check();

static void	app_start_profile_alarm();

static void app_process_leak_mode();

static void app_process_panic_switch();

static void app_process_ac_power_fault();

//static void app_process_emr_fuse_fault();

static void app_process_rs485_fuse_fault();

static void app_process_hot_restrike_fault();

static void app_send_fault_info();

static void app_monitor_timer_handler(void *arg);

static void app_monitor_sensor();

static void app_monitor_backup_battery();

static void app_mqtt_process_device_request(U8*        pBuf,
                                             jsmntok_t* pTok,
                                             U32        token_count);

static void app_init_sd_card();

static void  app_sd_card_prepare();

static app_sd_card_store_status(U8* name, U8* Buf, U32 len);

static void app_check_reset_apply_recovery();

static void app_do_reset_recovery();

static void app_mqtt_send_fwug_status(BOOLEAN gc_restored );

static void app_sd_card_get_status_file_ptr();

static void app_sd_card_store( U8* Buf, U32 len);

static void app_send_email(U8* pMsg, U32 msg_len, U8* subject);

static void app_send_mqtt_cmd_ack(APP_MQTT_CMD_TYPE cmd);

static void app_process_device_timer_expiry_msg(U8* buf);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
APP App;

/* Smtp month abbrevation */
static const char const* app_smtp_month_abbreviations[] =
{
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
};

/* Smtp weak days abbrevation */
static const char const* app_smtp_weekday_abbreviations[] =
{
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
};

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
/* This is used to map data arguments for respectives ISRs */
void *APP_intr_arg_list[BOARD_INTR_MAX_IRQn];
#endif

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
extern void TEST_FREERTOS_Init();

extern void TEST_UART_Init(UART* UartDriver);

extern void TEST_DEV_Light(APP* pApp);

void TEST_DEV_RtcAlarm();

extern void TEST_Json();

extern void TEST_SPI_Init(SPI* SpiDriver);
#endif

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void APP_Start()
{
    NW_ESP_WIFI_tls_t       Tls;
    NW_ESP_WIFI_peap_ttls_t PeapTtls;
    MQTT_COM_Security_t     mqtt_security;

    CONFIG_PROFILE_TYPE prof_type = CONFIG_FACTORY_PROFILE;

	memset((U8*)&App, 0x00, sizeof(APP));

    /* Initialize any board specific peripherals etc.. */

    /* Initialize Heap module if any */

    /* Initialize any watch dog timer TBD  */

#ifdef NUCLEO_F439ZI_EVK_BOARD
	/* initialize UART3 driver for trace print */
	UART_Init(&App.Uart3Driver,115200,UART3_HW_INDEX,UART_PARITY_NONE,0);

	/* initialize trace module , currently uses UART3 */
	TRACE_Init(&App.Uart3Driver.huart);
#endif

	TRACE_DBG("Entry Into EC APP  Version = %s \n", APP_EC_VERSION );

    BACKUP_SRAM_Init();

    CTRL_OUT_Init();

    app_device_init();
   
    //App_fault_esp_init();

	CONFIG_Init(&App.Config);

    if(CONFIG_IsUserProvisonPresent())
    {
        TRACE_DBG("USER PROVISIONING  PRESENT \n" );

        prof_type = CONFIG_USER_PROFILE;
    }
#if APP_WEB_PROVISIONING_FLAG
    else
    {
        TRACE_INFO("User Provison Not Present, WEB Provision On \n" );
        WEB_PROVISION_Init(&App.WebProv,APP_WEB_PROVISIONING_PORT_NO);
        return;
    }
#endif

	CONFIG_ReadTag(0,prof_type,CONFIG_WIFI_INFO_TAG,(U8*)&App.WifiConfig);

    //App.WifiConfig.time_zone = -8;

	CONFIG_ReadTag(0,prof_type,CONFIG_MQTT_INFO_TAG,(U8*)&App.MqttConfig);

	CONFIG_ReadTag(0,prof_type,CONFIG_DEVICE_INFO_TAG,(U8*)&App.DevConfig);


	OSA_InitMsgQue(&App.pActionMsgQ,"app_action_msg_q", APP_MSGQ_NO_OF_MESSAGES, sizeof(void *));

    OSA_InitMutex(&App.msg_q_mutex) ;

	/* Create APP rx thread. It contains all handlers for the received messages & profile change funtinality  */
    OSA_ThreadCreate(&App.action_thread,
       		         APP_ACTION_THREAD_PRIORITY ,
					 app_action_thread_entry_func,
					 NULL,
                     "app_task",
			     	 NULL,
					 APP_ACTION_THREAD_STACK_SIZE,
					 0);

#if 1

#ifdef NUCLEO_F439ZI_EVK_BOARD
    /* initialize USART2 driver for ESP WIFI communication */
   // UART_Init(&App.Uart2Driver,115200,UART2_HW_INDEX);
    UART_Init(&App.Uart2Driver,9600,UART2_HW_INDEX,UART_PARITY_NONE,0);

    /* Start ESP WIFI driver */
    //  App.WifiConfig.time_zone = 13;
    NW_WIFI_ESP_Init(&App.NwEspDriver,
                     App.WifiConfig.ssid ,
                     App.WifiConfig.passphrase,  //Wifi Password
                     App.WifiConfig.serial_baudrate,      //baud rate
                     &App.Uart2Driver,
					 App.WifiConfig.time_zone);
#endif

#ifdef ESP32_S2_MINI

    if(App.WifiConfig.security == NW_ESP_WIFI_PSK)
    {  
        NW_WIFI_ESP_Init(&App.NwEspDriver,
                         App.WifiConfig.ssid,
                         App.WifiConfig.time_zone,
                         App.WifiConfig.security,  
                         App.WifiConfig.passphrase);
    }
    if(App.WifiConfig.security == NW_ESP_WIFI_TLS)
    { 
        strcpy(Tls.eap_id,App.WifiConfig.eap_identity); 

        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT, (U8*)&Tls.ca_len, 4);
        
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT, (U8*)&Tls.client_len, 4);
        
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT, (U8*)&Tls.key_len, 4);
        
        Tls.ca = malloc(Tls.ca_len+1);
        Tls.client = malloc(Tls.client_len+1);
        Tls.key = malloc(Tls.key_len+1);

        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT+4, Tls.ca,Tls.ca_len);

        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT+4, Tls.client, Tls.client_len);

        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT+4, Tls.key, Tls.key_len);

        strcpy(Tls.key_password,App.WifiConfig.passphrase); 
      

        NW_WIFI_ESP_Init(&App.NwEspDriver,
                         App.WifiConfig.ssid ,
                         App.WifiConfig.time_zone,
                         App.WifiConfig.security,  
                         &Tls);
    }
    if(App.WifiConfig.security == NW_ESP_WIFI_PEAP || App.WifiConfig.security == NW_ESP_WIFI_TTLS)
    { 
        strcpy(PeapTtls.eap_id,App.WifiConfig.eap_identity); 
        strcpy(PeapTtls.username,App.WifiConfig.username); 
        strcpy(PeapTtls.password,App.WifiConfig.passphrase); 

        NW_WIFI_ESP_Init(&App.NwEspDriver,
                         App.WifiConfig.ssid ,
                         App.WifiConfig.time_zone,
                         App.WifiConfig.security,  
                         &PeapTtls);
    }
#endif
#ifdef NUCLEO_F439ZI_EVK_BOARD
    MQTT_COM_Init(&App.Mqtt,
                  App.DevConfig.device_name,   //use Device name as MQTT client id 
                  App.MqttConfig.host,
                  App.MqttConfig.port_no,
                  App.MqttConfig.username,
                  App.MqttConfig.password,
                  App.MqttConfig.topic,
                  App.MqttConfig.qos_type,
                  app_rx_mqtt_message, //call back function when it receives message
                  NULL);
#endif
#ifdef ESP32_S2_MINI

    if(App.MqttConfig.security == MQTT_COM_NO_SECURITY)
    {
        memset(&mqtt_security,0x00 , sizeof(MQTT_COM_Security_t));

        strcpy(mqtt_security.s.NoSsl.username,App.MqttConfig.username);
        strcpy(mqtt_security.s.NoSsl.password,App.MqttConfig.password);  

        MQTT_COM_Init(&App.Mqtt,
                      App.DevConfig.device_name,   //use Device name as MQTT client id 
                      App.MqttConfig.host,
                      App.MqttConfig.port_no,
                      App.MqttConfig.topic,
                      App.MqttConfig.qos_type,
                      app_rx_mqtt_message, //call back function when it receives message
                      NULL,
                      App.MqttConfig.security,
                      &mqtt_security);
    }

    if(App.MqttConfig.security == MQTT_COM_SSL_SECURITY)
    {
        memset(&mqtt_security,0x00 , sizeof(MQTT_COM_Security_t));

        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT, (U8*)&mqtt_security.s.Ssl.ca_len, 4);
        
        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT, (U8*)&mqtt_security.s.Ssl.client_len, 4);
        
        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT, (U8*)&mqtt_security.s.Ssl.key_len, 4);

        //TRACE_DBG("calen = %d\tclientlen=%d\tkeylen=%d\n",mqtt_security.s.Ssl.ca_len,mqtt_security.s.Ssl.client_len,mqtt_security.s.Ssl.key_len);

        if(mqtt_security.s.Ssl.ca_len == 0xFFFFFFFF)
        {
            mqtt_security.s.Ssl.ca_len = 0x00;
        }
        else
        {
            mqtt_security.s.Ssl.ca = malloc(mqtt_security.s.Ssl.ca_len+1);
            FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT+4, mqtt_security.s.Ssl.ca,mqtt_security.s.Ssl.ca_len);
        }

        if(mqtt_security.s.Ssl.client_len == 0xFFFFFFFF)
        {
            mqtt_security.s.Ssl.client_len = 0x00;
        }
        else
        {
            mqtt_security.s.Ssl.client = malloc(mqtt_security.s.Ssl.client_len+1);
            FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT+4, mqtt_security.s.Ssl.client, mqtt_security.s.Ssl.client_len);
        }

        if(mqtt_security.s.Ssl.key_len == 0xFFFFFFFF)
        {
            mqtt_security.s.Ssl.key_len = 0x00;
        }
        else
        {
            mqtt_security.s.Ssl.key = malloc(mqtt_security.s.Ssl.key_len+1);
            FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT+4, mqtt_security.s.Ssl.key, mqtt_security.s.Ssl.key_len);
        }


        MQTT_COM_Init(&App.Mqtt,
                      App.DevConfig.device_name,   //use Device name as MQTT client id 
                      App.MqttConfig.host,
                      App.MqttConfig.port_no,
                      App.MqttConfig.topic,
                      App.MqttConfig.qos_type,
                      app_rx_mqtt_message, //call back function when it receives message
                      NULL,
                      App.MqttConfig.security,
                      &mqtt_security);
    }
#endif

#endif
    RTC_Init(&App.Rtc);


#if 1  //TBD 
	/* Check the type of RESET */
	app_check_reset_apply_recovery();

#endif

#if APP_MODBUS_FLAG
    MODBUS_Init();  
#endif

#ifdef NUCLEO_F439ZI_EVK_BOARD

    NVIC_EnableIRQ(USART3_IRQn | SVCall_IRQn |  PendSV_IRQn | SysTick_IRQn | USART2_IRQn | RTC_Alarm_IRQn | EXTI15_10_IRQn | DMA2_Stream0_IRQn | DMA2_Stream3_IRQn );

    TRACE_DBG("Starting Scheduler\n");

    /* Start FreeRTOS Kernel  */
    vTaskStartScheduler();

    while(1);
#endif

}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : executes under RTC Alarm ISR
*******************************************************************************/
void APP_do_light_cycle_transition()
{
    ets_printf("%s Entry \n", __FUNCTION__);
    app_send_msg_req_to_q(APP_LIGHT_CYCLE_ALARM_MSG, NULL, 0,TRUE);
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
void App_apply_new_profile()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

#if 1
    app_send_msg_req_to_q(APP_APPLY_NEW_PROFILE_MSG, NULL, 0,FALSE);
#else
    app_start_profile_alarm();

	DEVICE_Stop();

    DEVICE_Execute();
#endif
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
void APP_FaultISR()
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
#if 0
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_10) != RESET)
  	{
    	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
        app_send_msg_req_to_q(APP_HOT_RESTRIKE_FAULT_MSG, NULL, 0,TRUE);
  	}
#endif
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET)
  	{
    	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
		app_send_msg_req_to_q(APP_LEAK_SENSOR_MSG, NULL, 0,TRUE);	
  	}
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_12) != RESET)
  	{
    	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
		app_send_msg_req_to_q(APP_PANIC_BUTTON_MSG, NULL, 0,TRUE);
  	}
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_13) != RESET)
  	{
    	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);
		app_send_msg_req_to_q(APP_AC_POWER_FAULT_MSG, NULL, 0,TRUE);
  	}
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_14) != RESET)
  	{
    	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
		app_send_msg_req_to_q(APP_HOT_RESTRIKE_FAULT_MSG, NULL, 0,TRUE);
  	}
	if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET)
  	{
    	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
		app_send_msg_req_to_q(APP_RS485_FUSE_FAULT_MSG, NULL, 0,TRUE);
  	}
#endif
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
void APP_send_hot_restrike_fault_msg()
{
    app_send_fault_info();
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
void App_Reset(APP_SW_RESET_CAUSE reason)
{
    TRACE_INFO("%s Entry\n", __FUNCTION__);

    if(App.ec_state == APP_GROW)
    {
#if APP_OPEN_SD_CARD_FILE_ONCE
        SD_CARD_close_file( &App.gc_status_file_ptr);
#endif
        /* stop profile RTC alarm immediately */
        RTC_StopAlarm(App.profile_rtc_alarm_no);

        /* stop status timer immediately */
        OSA_StopTimer(&App.status_timer);
        OSA_DeInitTimer(&App.status_timer);

        /* This is to switch of RTC alarm started by Light device & 
           switching off all devices gracefully  */
        DEVICE_Stop();
    }

    App_backup_store(reason);

#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_NVIC_SystemReset();
#endif

#ifdef ESP32_S2_MINI
    esp_restart();
#endif
}



/*******************************************************************************
* Name       :
* Description:
* Remarks    : it executes under freeRTOS timer task context
*******************************************************************************/
void App_fwug_timer_handler(void *arg)
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);
    /* post status timer message to app action thread . This is required
       not to burden RTOS timer thread */
    app_send_msg_req_to_q(APP_FWUG_TIMER_EXPIRY_MSG, NULL, 0,FALSE);   
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : executes under freeRTOS Thread timer 
*******************************************************************************/
void APP_send_timer_expiry_msg(U8* buffer , U32 length)
{
    app_send_msg_req_to_q(APP_DEVICE_TIMER_EXPIRY_MSG, buffer,length,FALSE);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : executes under freeRTOS Thread timer 
*******************************************************************************/
void App_send_fault_message(APP_MSG_TYPE  msg_id)
{
    app_send_msg_req_to_q(msg_id, NULL, 0,TRUE);
}
/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_rx_mqtt_message(void* pArg, BUFFER8* pBuffer )
{
    TRACE_DBG("%s() Entry \n",__FUNCTION__);

 //   RTC_GetTime(App.GrowStatus.rtc_time);

    //TRACE_DBG("MQTT Msg(%d) =%.*s\n",pBuffer->size, pBuffer->size, (char*)pBuffer->pBuf);

    /* put the message into  RxMsgQ TBD */
    app_send_msg_req_to_q(APP_MQTT_MSG, pBuffer->pBuf,pBuffer->size,FALSE);

   // RTC_GetTime(App.GrowStatus.rtc_time);

    TRACE_INFO("%s Exit\n", __FUNCTION__);

}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_action_thread_entry_func(void *arg)
{
	App_MsgReq *pMsg = NULL;

	U8 time[100];

	TRACE_INFO("%s Entry \n", __FUNCTION__);

#if APP_SD_CARD_FLAG
    
    /* initialize SD card */
    app_init_sd_card();

#endif

#ifdef NUCLEO_F439ZI_EVK_BOARD  

STEP_TIME:

	/* start RTC timer */

	NW_WIFI_ESP_GetTime(time);
    //TRACE_INFO("CURRENT NETWORK  TIME = %s\n",time);

	if (strstr(time ,"1970"))
	{
		goto STEP_TIME;
	}
	TRACE_INFO("DEVICE STARTED TIME = %s\n",time);

	RTC_SetTime(time);

    //app_send_email("Dummy Test Data", strlen("Dummy Test Data"), "Test Mail");

#endif

	while(1)
	{
		TRACE_DBG( "WAIT ON ACTION MSG Q\n");

//rtc_wdt_feed();

		/* Blocking call till time out  */
		OSA_ReceiveMsgQue(App.pActionMsgQ, &pMsg);



		TRACE_DBG("ACTION THREAD MESSAGE = %d\n",pMsg->type);

        switch(pMsg->type)
        {
            case APP_MQTT_MSG:
                app_mqtt_msg_handler(pMsg->buf, pMsg->len);
                break;

            case APP_PROFILE_TIMER_MSG:
                app_do_execute_profile();
                break;

			case APP_STATUS_TIMER_MSG:
				app_grow_cycle_send_status();
				break;

			case APP_GROW_CYCLE_ALARM_MSG:
				app_grow_cycle_alarm_check();
				break;

            case APP_LIGHT_CYCLE_ALARM_MSG:
                DEVICE_StopAlarm(DEVICE_LIGHT);
                DEVICE_LightCycleExecute();
                break;

            case APP_APPLY_NEW_PROFILE_MSG:  //Apply new profile 
                app_start_profile_alarm();
                App.apply_profile_no = 0x00;
                DEVICE_Stop();
                DEVICE_Execute();
                break;

			case APP_LEAK_SENSOR_MSG:
				app_process_leak_mode();
				break;

			case APP_PANIC_BUTTON_MSG:
			//	app_process_panic_switch();
                App_fault_process_panic_switch();
				break;

			case APP_AC_POWER_FAULT_MSG:
			//	app_process_ac_power_fault();
                App_fault_process_ac_power_fault();
				break;
#if 0
			case APP_EMR_FUSE_FAULT_MSG:
				app_process_emr_fuse_fault();
				break;
#endif

			case APP_RS485_FUSE_FAULT_MSG:
		//		app_process_rs485_fuse_fault();
                App_fault_process_rs485_fuse_fault();
				break;

            case APP_HOT_RESTRIKE_FAULT_MSG:
              //  app_process_hot_restrike_fault();
                App_fault_process_hot_restrike_fault();
                break;

			case APP_MONITOR_TIMER_MSG:
				app_monitor_sensor();
				break;

            case APP_DO_RESET_RECOVERY_MSG:
                app_do_reset_recovery();
                break;
            case APP_FWUG_TIMER_EXPIRY_MSG:
#if 1 //TBD 
                App_process_fwug_timer_expiry();
#endif
                break; 

            case APP_DEVICE_TIMER_EXPIRY_MSG:
                app_process_device_timer_expiry_msg(pMsg->buf);
                break;

            default:
                TRACE_WARN("Unknown Msg \n");
                break;
        }
        app_release_msg_req(pMsg);
	}
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : This always posts into Actin msg Q 
*******************************************************************************/
static BOOLEAN  app_send_msg_req_to_q(U32 APP_MSG_TYPE, void* buf,U32 len,BOOLEAN is_isr_context)
{
	App_MsgReq *msg;

    APP_LOCK_MSG_Q_MUTEX(is_isr_context,&App.msg_q_mutex);

    msg  = (App_MsgReq *) malloc(sizeof(App_MsgReq));

    if(msg)
    {
       // ets_printf("%s Entry \n", __FUNCTION__);
        msg->buf = NULL;
        msg->len = 0;

        if(len != 0)
        {
            msg->buf = malloc(len);
            if(msg->buf)
            {
                memcpy((U8*)msg->buf,(U8*)buf,len);
                msg->len = len;
            }
            else
            {
                free(msg);
                APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&App.msg_q_mutex);
                return STATUS_ERR; 
            }
        }
        msg->type = APP_MSG_TYPE;

        if(is_isr_context)
        {
            ets_printf("%s ISR Context \n", __FUNCTION__);
            if(!OSA_SendMsgQueIsr(App.pActionMsgQ,msg)) 
            {
                ets_printf("Failed to post to App queue IN ISR Context\n");
                APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&App.msg_q_mutex);
                return STATUS_ERR;
            }
        }
        else
        {
           // TRACE_INFO("%s NON ISR Context \n", __FUNCTION__);
            if(!OSA_SendMsgQue(App.pActionMsgQ,msg)) 
            {
                TRACE_WARN("Failed to post to App queue\n");
                APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&App.msg_q_mutex);
                return STATUS_ERR;
            }
        }
        APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&App.msg_q_mutex); 
        return STATUS_OK;  
    }
    APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&App.msg_q_mutex);
    return STATUS_ERR;
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_release_msg_req(App_MsgReq *pMsg )
{
    if(pMsg->buf)
    {
        free(pMsg->buf);
    }
    free(pMsg);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_mqtt_msg_handler(U8 *pBuf, U32 len)
{
    S32 token_count;     
    jsmn_parser p;
 //	jsmntok_t tok[APP_TOKEN_MAX_COUNT];
    jsmntok_t* tok;
    U32 i;

    U8* pName;
    U32 id;
    U8* pFirmName;
    U8* pTopic;
    U32 app_id;

    BOOLEAN is_valid = TRUE;

    U32 cmd_id;

//	static U8* last_buf = NULL;

//	static U32 last_buf_len = 0;

    BOOLEAN is_req_present=FALSE;

#if 0
	if(last_buf)
	{
		last_buf = (char *)realloc(last_buf, (last_buf_len + len));
		if(last_buf == NULL)
    	{
			TRACE_FATAL("Memory allocation for JSON Pkt FAILED \n");
        	exit(1);
    	}
    	strcat(last_buf,pBuf);
	}
#endif

    tok = malloc(sizeof(jsmntok_t)*APP_TOKEN_MAX_COUNT);

     /* process the message */
    jsmn_init(&p);
    token_count = jsmn_parse(&p, (const char*)pBuf, len, tok, APP_TOKEN_MAX_COUNT);

//	TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    if ((token_count == JSMN_ERROR_PART) || (token_count < 0))
    {
        TRACE_INFO("Received PKT not complete JSON Pkt %d\n",token_count);
        free(tok);
        return;
    }

#if 0
	if (token_count == JSMN_ERROR_PART)
	{
		TRACE_INFO("Received PKT not complete JSON Pkt %d\n",token_count);

        last_buf = pBuf;

		last_buf_len = len;

		return FALSE;
	}
	else
	{
		TRACE_INFO("Received Complete JSON Pkt %d\n",token_count);
		last_buf = NULL;

		last_buf_len = 0x00;
	}
#endif


    /* check the message is for this EC or not */
    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"device_name",APP_COMPARE_LENGTH("device_name")))
            {
                i++;
                pName = (char*)(pBuf + tok[i].start);
              // *(pName + pTok[i].end - pTok[i].start) = '\0';
//printf("device_name=%s",App.DevConfig.device_name);
         //       TRACE_DBG("Device Name(%d) = %s\n",  tok[i].end - tok[i].start,pName );
                if(memcmp(App.DevConfig.device_name,pName,tok[i].end - tok[i].start))
                {   
                    is_valid = FALSE;
                    break ;      
                }
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"device_id",APP_COMPARE_LENGTH("device_id")))
            {
                i++;              
                id = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                //TRACE_DBG("Device Id = %d\n", id);
                if(App.DevConfig.device_id != id)
                { 
                    is_valid = FALSE;
                    break ;      
                }
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"firm_name",APP_COMPARE_LENGTH("firm_name")))
            {
                i++;
                pFirmName = (char*)(pBuf + tok[i].start);
                // *(pFirmName + pTok[i].end - pTok[i].start) = '\0';
               // TRACE_DBG("Firm Name = %s\n", pFirmName);

                if(memcmp(App.DevConfig.firm_name,pFirmName,tok[i].end - tok[i].start))
                {
                    is_valid = FALSE;
                    break ;      
                }
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"mobile_app_id",APP_COMPARE_LENGTH("mobile_app_id")))
            {
                i++;              
                app_id = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                //TRACE_DBG("Mobile APP Id  = %d\n", app_id);
                if(App.DevConfig.mobile_app_id != app_id)
                {   
                    is_valid = FALSE;
                    break ;      
                }
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"greenhouse_id",APP_COMPARE_LENGTH("greenhouse_id")))
            {
            	i++;
                id = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
               // TRACE_DBG("Green House  Id = %d\n", id);
                if(App.DevConfig.greenhouse_id != id)
                {
                	is_valid = FALSE;
                    break ;
                }
            }

        }
    }
    if(!is_valid)
    {
        TRACE_WARN("WRONG Msg , Not for this EC \n");
        free(tok);
        return;
    }

    /* check the message is request from mobile APP or not */
    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"req",APP_COMPARE_LENGTH("req")))
            { 
                is_req_present=TRUE;
            }
        }
    }

    if(!is_req_present)
    {
        TRACE_INFO("No Request Found\n");
        free(tok);
        return;
    }


    /* Route the packet to appropriate handler */
    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"cmd", APP_COMPARE_LENGTH("cmd")))
            { 
                i++;              
                cmd_id = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                //if(cmd_id != APP_MQTT_FW_UG_REQUEST)
                {
                	app_send_mqtt_cmd_ack(cmd_id);
                }
                switch(cmd_id)
                {
                    case APP_MQTT_GROW_CYCLE_INFO:
                        app_mqtt_process_grow_cycle_request(pBuf, tok, token_count);
                        break;

                    case APP_MQTT_PROFILE_INFO:
                        app_mqtt_process_profile_request(pBuf, tok, token_count);
                        break;

                    case APP_MQTT_PROVISION_INFO:
                        app_mqtt_process_provision_request(pBuf, tok, token_count);
                        break;

					case APP_MQTT_DEVICE_REQUEST:
						app_mqtt_process_device_request(pBuf, tok, token_count);
						break;

                    case APP_MQTT_FW_UG_REQUEST:
                        App_mqtt_process_fwug_request(pBuf, tok, token_count);
                        break;

                    default:
                        TRACE_WARN("Unknown MQTT Cmd  , Not processed  = %d\n", cmd_id);
                        break;
                }           
            }

        }
    }
    free(tok);
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/


static void app_mqtt_process_grow_cycle_request(U8*        pBuf,
                                                jsmntok_t* pTok,
                                                U32        token_count)
{
    U32 i;

    BOOLEAN is_grow_requested = FALSE;

    U8* pTime = NULL;

    U8* pDate = NULL; 

    U8* pName;

    BOOLEAN user_profile;   

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"action",APP_COMPARE_LENGTH("action")))
            {
                i++;
                if (!memcmp((char*)(pBuf + pTok[i].start),"start",APP_COMPARE_LENGTH("start")))
                {
                	is_grow_requested = TRUE;
                }
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"grow_name",APP_COMPARE_LENGTH("grow_name")))
            {
                i++;              
                pName = (char *)(pBuf  + pTok[i].start);
                *(pName + pTok[i].end - pTok[i].start) = '\0';
                strcpy(App.gc_name,pName); 
            }  
            else if (!memcmp((char*)(pBuf + pTok[i].start),"total_days",APP_COMPARE_LENGTH("total_days")))
            {
                i++;              
                App.gc_total_days = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
                TRACE_DBG("App.gc_total_days = %d\n", App.gc_total_days );
            }         
            else if (!memcmp((char*)(pBuf + pTok[i].start),"status_period",APP_COMPARE_LENGTH("status_period")))
            {
                i++;              
                App.gc_reporting_period = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
                TRACE_DBG("App.gc_reporting_period = %d\n", App.gc_reporting_period );
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"profile_no",APP_COMPARE_LENGTH("profile_no")))
            {
                i++;              
                App.gc_profile_no = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
                TRACE_DBG("profile_no= %d\n", App.gc_profile_no );
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"user_profile",APP_COMPARE_LENGTH("user_profile")))
            {
                i++;              
                user_profile = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
                TRACE_DBG("user_profile = %d\n", user_profile);
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"start_date",APP_COMPARE_LENGTH("start_date")))
            {
                i++;              
                pDate = (char *)(pBuf  + pTok[i].start);
                *(pDate + pTok[i].end - pTok[i].start) = '\0';
                strcpy(App.grow_cycle_start_date,pDate); 
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"start_time",APP_COMPARE_LENGTH("start_date")))
            {
                i++;              
                pTime = (char *)(pBuf  + pTok[i].start);
                *(pTime + pTok[i].end - pTok[i].start) = '\0'; 
                strcpy(App.grow_cycle_start_time,pTime);
            }       
        }
    }
    if((is_grow_requested) && (App.ec_state == APP_IDLE))
    {
        /* start new grow cycle  with the profile id given */
        App.GrowStatus.days_elapsed = -1;
        App.profile_rtc_alarm_no = -1;
        UTILITY_convert_time_duration_to_str(0x00, App.GrowStatus.grow_time);
        app_do_start_grow_cycle(App.gc_profile_no,user_profile,pDate,pTime);
    }
    else if((App.ec_state == APP_GROW) && (!is_grow_requested))
    {
    	/* stop the current  grow cycle  */
    	app_do_stop_grow_cycle();
    }    
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_mqtt_process_profile_request(U8*        pBuf,
                                             jsmntok_t* pTok,
                                             U32        token_count)
{
    U32 i;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"action",APP_COMPARE_LENGTH("action")))
            {
                i++;
                if (!memcmp((char*)(pBuf + pTok[i].start),"set",APP_COMPARE_LENGTH("set")))
                {
                    App_mqtt_process_set_profile_request(pBuf, pTok,token_count);
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"apply",APP_COMPARE_LENGTH("apply")))
                {
                    App_mqtt_process_apply_profile_request(pBuf, pTok,token_count);
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"modify",APP_COMPARE_LENGTH("modify")))
                {
                    App_mqtt_process_modify_profile_request(pBuf, pTok,token_count);
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"profile_erase",APP_COMPARE_LENGTH("profile_erase")))
                {
                    App_mqtt_process_erase_profile_request(pBuf, pTok,token_count);
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"erase",APP_COMPARE_LENGTH("erase")))
                {
                    CONFIG_EraseUserProfile();
                }
                else
                {
                    App_mqtt_process_get_profile_request(pBuf, pTok,token_count);
                }
            }
        }
    }
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_mqtt_process_provision_request(U8*        pBuf,
                                               jsmntok_t* pTok,
                                               U32        token_count)
{
    U32 i;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"action",APP_COMPARE_LENGTH("action")))
            {
                i++;
                if (!memcmp((char*)(pBuf + pTok[i].start),"set",APP_COMPARE_LENGTH("set")))
                {
                    App_mqtt_process_set_provision_info_request(pBuf, pTok,token_count);
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"get",APP_COMPARE_LENGTH("get")))
                {
                    App_mqtt_process_get_provision_info_request(pBuf, pTok,token_count);
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"erase",APP_COMPARE_LENGTH("erase")))
                {
                    CONFIG_EraseIndividualUserProfile(0);
                }
            }
        }
    }
    
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_do_start_grow_cycle(U32 profile_no, BOOLEAN user_profile,U8* pDate, U8* pTime)
{
    BOOLEAN to_start_now_or_not;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    to_start_now_or_not = app_check_to_start_grow_cycle_or_not(pDate,pTime);

    /* do profile read from flash  & update working profile  */
    if(user_profile)
    {
        App.gc_profile = CONFIG_USER_PROFILE;
        CONFIG_ReadProfile(profile_no, CONFIG_USER_PROFILE, &App.WorkingProfile);
    }
    else
    {
        App.gc_profile = CONFIG_FACTORY_PROFILE;
        CONFIG_ReadProfile(profile_no, CONFIG_FACTORY_PROFILE, &App.WorkingProfile);
    }
#if APP_MODBUS_FLAG
    while (!MODBUS_GetSetParam(APP_MODBUS_SLAVE_ADDRESS ,FC_SET_SENSOR_HEIGHT , (void*)&App.WorkingProfile.GrowInfo.sensor_height));
#endif
	//TRACE_INFO("%s Entry = %d  = %d \n", __FUNCTION__,App.DevLightConfig.DevState, App.DevLightConfig.LightCycle );
  
    if(to_start_now_or_not)
    {       
        app_begin_grow_cycle();
    }
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_do_stop_grow_cycle()
{
	TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* stop profile RTC alarm immediately */
    RTC_StopAlarm(App.profile_rtc_alarm_no);

    /* stop status timer immediately */
    OSA_StopTimer(&App.status_timer);
    OSA_DeInitTimer(&App.status_timer);

    OSA_StopTimer(&App.monitor_timer);
    OSA_DeInitTimer(&App.monitor_timer);

    DEVICE_Stop();

    memset(&App.GrowStatus,0x00,sizeof(GrowConfig_GrowCycleStatus));
    UTILITY_convert_time_duration_to_str(0x00, App.GrowStatus.grow_time);

#if APP_OPEN_SD_CARD_FILE_ONCE
	SD_CARD_close_file( &App.gc_status_file_ptr);
#endif

    App.gc_status_file_ptr = 0x00; 

    App.ec_state = APP_IDLE;

    App.apply_profile_no = 0x00;
    
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_do_execute_profile()
{
    BOOLEAN to_apply_now;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

	app_start_profile_alarm();

	/* update  grow cycle parameters */
	app_update_growcycle_status_parameters(&App.GrowStatus);

    //app_monitor_backup_battery();
   
	TRACE_INFO("days_elapsed = %d   total_days = %d  \n", App.GrowStatus.days_elapsed ,App.gc_total_days);

    if(App.GrowStatus.days_elapsed >= App.gc_total_days)
    {
        /* Grow cycle completed , stop it */
        app_do_stop_grow_cycle();
        return;
        
    }
#if APP_SD_CARD_FLAG

#if APP_OPEN_SD_CARD_FILE_ONCE
    app_sd_card_get_status_file_ptr();
#endif
#endif

    if(App.apply_new_profile)
    {
        if(App_check_to_apply_profile_now_or_not())
        {
            TRACE_INFO("%s Applying New Profile \n", __FUNCTION__);
             /* Time to apply the new profile */
            memcpy(&App.WorkingProfile,App.pNewProfile,sizeof(App.WorkingProfile));
            free(App.pNewProfile);
            App.pNewProfile = NULL;
            App.apply_new_profile = FALSE;
            memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
            memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));
            App.apply_profile_st = 0x00;
            App.gc_profile = App.apply_profile ;
            App.gc_profile_no = App.apply_profile_no;
            App.apply_profile_no = 0;
            App_apply_new_profile();
        }
        else
        {
            TRACE_INFO("%s Executing the  Profile \n", __FUNCTION__);

            DEVICE_Execute();
        }
    }
	else
	{
        TRACE_INFO("%s Executing the  Profile \n", __FUNCTION__);

		DEVICE_Execute();
	}
    TRACE_INFO("%s Exit \n", __FUNCTION__);
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_do_stop_profile()
{
	TRACE_INFO("%s Entry \n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_device_init()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    DEVICE_Init(&App.WorkingProfile.GrowInfo,
    			&App.GrowStatus,
			    &App.WorkingProfile.DevLightConfig,
				&App.WorkingProfile.DevHoodVentConfig,
				&App.WorkingProfile.DevCircConfig,
				&App.WorkingProfile.DevVentConfig,
				&App.WorkingProfile.DevFxp1Config,
				&App.WorkingProfile.DevFxp2Config,
				ActivateDevice,
				DeActivateDevice );
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_update_growcycle_status_parameters(GrowConfig_GrowCycleStatus* pGrowStatus)
{
    U8 time[50];
    U8* p;
    U8* q;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    RTC_GetTime(time);

    TRACE_DBG("TIME = %s\n", time);

    pGrowStatus->day_no++;

#if 1
    pGrowStatus->week_no =  pGrowStatus->day_no/7;

    if(pGrowStatus->day_no % 7)
    {
        pGrowStatus->week_no +=1;
    }
#else

    if(pGrowStatus->day_no <= 7 )
    {
        pGrowStatus->week_no =1;    
    }    
    else if((pGrowStatus->day_no % 7) == 1)
    {
        pGrowStatus->week_no +=1;    
    }
#endif

    p=time;
    q = strchr(p,' ');
    *q = 0x00;
    pGrowStatus->day_cur_week = atoi(p);
    *q = 0x25;

    p = time;
    q = strchr(p,' ');
    q++;

    pGrowStatus->days_elapsed++;

    TRACE_INFO("%s Exit = %s  \n", __FUNCTION__,q);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : it executes under freeRTOS timer task context
*******************************************************************************/
static void app_grow_cycle_status_handler(void *arg)
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);
	/* post status timer message to app action thread . This is required
	   not to burden RTOS timer thread */
	app_send_msg_req_to_q(APP_STATUS_TIMER_MSG, NULL, 0,FALSE);

   // RTC_GetTime(App.GrowStatus.rtc_time);
   
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : executes under app action thread .
*******************************************************************************/
static void app_grow_cycle_send_status()
{
	U8* buf;

    U8* rsp;

    U8* p;

    U8* q;

	U8 time[50];

    mTIME   g_time;
    U32     g_time_sec;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    if(App.ec_state == APP_IDLE)
    {
        TRACE_WARN("Not in Grow Cycle, Just ignore the timer message \n");
        return;
    }

    /* get RTC time */
    RTC_GetTime(App.GrowStatus.rtc_time);
    p = App.GrowStatus.rtc_time;
    q = strchr(p,' ');
    q++;

    /* update GROW time */
    TRACE_DBG("Grow time = %s \n",App.GrowStatus.grow_time);
    UTILITY_convert_time_str(App.GrowStatus.grow_time, &g_time);
    g_time_sec = g_time.HH*3600 + g_time.MM*60 + g_time.SS;
    g_time_sec +=App.gc_reporting_period;
    UTILITY_convert_time_duration_to_str(g_time_sec, App.GrowStatus.grow_time);

    /* get status of all device outlets */
    DEVICE_GetStatusAll();

    /* get latest sensor data TBD  */

    /* fill JSON format & send to MQTT */

    buf = malloc(APP_MAX_SIZE_PKT); // to save thread stack space , take it from heap
    rsp = buf;
    rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",", "type", "mqtt");
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "device_name", App.DevConfig.device_name);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "device_id", App.DevConfig.device_id);
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "farm_name", App.DevConfig.firm_name);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "greenhouse_id", App.DevConfig.greenhouse_id);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "mobile_app_id", App.DevConfig.mobile_app_id);
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "ver", APP_EC_PROTOCOL_VERSION);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "flag", 0);
    rsp +=sprintf((char *)rsp,"\"%s\":{\"%s\":%d,\"%s\":[", "res", "cmd",APP_MQTT_STATUS_INFO,"fields");

    rsp +=sprintf((char *)rsp,"{\"%s\":", "grow_cycle");
    {

        rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",", "name",App.gc_name);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "total_days",App.gc_total_days);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "days_elapsed",App.GrowStatus.days_elapsed);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "rtc_time",q);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "grow_time",App.GrowStatus.grow_time);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "profile_no",App.gc_profile_no);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "user_profile",App.gc_profile);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "transit_profile_no",App.apply_profile_no);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "day_no",App.GrowStatus.day_no);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "week_no",App.GrowStatus.week_no);
        rsp +=sprintf((char *)rsp,"\"%s\":%d}}", "day_cur_week",App.GrowStatus.day_cur_week);
    }
    rsp +=sprintf((char *)rsp,",{\"%s\":", "light_cycle");
    {
        rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "cur_cycle",App.GrowStatus.cycle_type);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "mode",App.GrowStatus.light_mode);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "lc_duration",App.GrowStatus.lc_duration);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "elapsed_lc_duration",App.GrowStatus.elapsed_lc_duration);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "pending_lc_duration",App.GrowStatus.pending_lc_duration);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "dc_duration",App.GrowStatus.dc_duration);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "elapsed_dc_duration",App.GrowStatus.elapsed_dc_duration);
        rsp +=sprintf((char *)rsp,"\"%s\":\"%s\"}}", "pending_dc_duration",App.GrowStatus.pending_dc_duration);
    }
    rsp +=sprintf((char *)rsp,",{\"%s\":", "dev_config");
    {
        rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "light_cfg",App.GrowStatus.dev_light_config);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "hood_vent_cfg",App.GrowStatus.dev_hood_vent_config);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "circ_cfg",App.GrowStatus.dev_circ_config);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "vent_cfg",App.GrowStatus.dev_vent_config);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "fxp1_cfg",App.GrowStatus.dev_fxp1_config);
        rsp +=sprintf((char *)rsp,"\"%s\":%d}}", "fxp2_cfg",App.GrowStatus.dev_fxp2_config);
    }


    rsp +=sprintf((char *)rsp,",{\"%s\":", "dev_status");
    {
#if 1
        rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "light_on",App.GrowStatus.dev_light_on);
        TRACE_DBG("Light Device State = %d\n",App.GrowStatus.dev_light_on);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "hood_vent_on",App.GrowStatus.dev_hood_vent_on);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "circ_on",App.GrowStatus.dev_circ_on);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "vent_on",App.GrowStatus.dev_vent_on);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "fxp1_on",App.GrowStatus.dev_fxp1_on);
        rsp +=sprintf((char *)rsp,"\"%s\":%d}}", "fxp2_on",App.GrowStatus.dev_fxp2_on);
#endif
    }
    rsp +=sprintf((char *)rsp,",{\"%s\":", "dev_counter");
    {
        rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "light_cnt",App.GrowStatus.dev_light_counter);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "hood_vent_cnt",App.GrowStatus.dev_hood_vent_counter);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "circ_cnt",App.GrowStatus.dev_circ_counter);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "vent_cnt",App.GrowStatus.dev_vent_counter);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "fxp1_cnt",App.GrowStatus.dev_fxp1_counter);
        rsp +=sprintf((char *)rsp,"\"%s\":%d}}", "fxp2_cnt",App.GrowStatus.dev_fxp2_counter);
    }

    rsp +=sprintf((char *)rsp,",{\"%s\":", "sensor_data");
    {
        rsp +=sprintf((char *)rsp,"{\"%s\":%.2f,", "cur_temp",App.GrowStatus.cur_temp);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "cur_humidity",App.GrowStatus.cur_humidity);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "cur_co2",App.GrowStatus.cur_co2);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "max_temp",App.GrowStatus.max_temp);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "max_humidity",App.GrowStatus.max_humidity);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "max_co2",App.GrowStatus.max_co2);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "min_temp",App.GrowStatus.min_temp);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "min_humidity",App.GrowStatus.min_humidity);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "min_co2",App.GrowStatus.min_co2);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "low_temp_state",App.GrowStatus.low_temp_state);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "high_temp_state",App.GrowStatus.high_temp_state);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "low_humidity_state",App.GrowStatus.low_humidity_state);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "high_humidity_state",App.GrowStatus.high_humidity_state);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "low_co2_state",App.GrowStatus.low_co2_state);
        rsp +=sprintf((char *)rsp,"\"%s\":%d}}", "high_co2_state",App.GrowStatus.high_co2_state);


    }

    rsp +=sprintf((char *)rsp,",{\"%s\":", "fault");
    {
        rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "backup_battery_fault",App.GrowStatus.backup_battery_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%.2f,", "cur_battery_voltage",App.GrowStatus.cur_battery_volatge);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "ac_power_fault",App.GrowStatus.ac_power_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "hot_restrike_fault",App.GrowStatus.hot_restrike_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "emr_fuse_fault",App.GrowStatus.emr_fuse_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "rs485_fuse_fault",App.GrowStatus.rs485_fuse_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "critical_temp_fault",App.GrowStatus.critical_temp_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d}}", "sensor_fault",App.GrowStatus.sensor_fault);
    }
    rsp +=sprintf((char *)rsp,"]}}");

#if 0
{
  /* process the message */

    jsmn_parser j;

    jsmntok_t tok[APP_TOKEN_MAX_COUNT];

    S32 token_count; 


    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)buf, rsp-buf, tok, APP_TOKEN_MAX_COUNT);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    TRACE_DBG("The Status(%d) = %s\n",rsp-buf,buf);

    free(buf);

    return;

}
#endif

  //  TRACE_DBG("The Status(%d) = %s\n",rsp-buf,buf);

    RTC_GetTime(time);
#if APP_SD_CARD_FLAG
#if APP_OPEN_SD_CARD_FILE_ONCE
	app_sd_card_store(buf, rsp-buf);
#else
    app_sd_card_store_status(q, buf, rsp-buf);
#endif
#endif
    RTC_GetTime(time);

    MQTT_COM_SendMessage(buf, rsp-buf);

    free(buf);

	TRACE_INFO("%s Exit\n", __FUNCTION__);
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : executes under app action thread .
*******************************************************************************/
static BOOLEAN app_check_to_start_grow_cycle_or_not(U8* pDate, U8* pTime)
{
    U8* p;

    U8* q;

    mDATE start_date;

    mTIME start_time;

    mDATE cur_date;

    mTIME cur_time;

    U8 cur_time_str[12];

    U8 cur_date_str[12];

    U32 st;

    U32 ct;

    S32 cmp_value;

    BOOLEAN status = FALSE;

    mTIME ref_time;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    memset((U8*)&start_date,0x00,sizeof(start_date));

    memset((U8*)&start_time,0x00,sizeof(start_time));

    memset((U8*)&ref_time,0x00,sizeof(ref_time));

    RTC_GetDateTime(&cur_date, &cur_time , cur_date_str ,cur_time_str);

    UTILITY_convert_date_str(pDate, &start_date);

    UTILITY_convert_time_str(pTime, &start_time);

    GET_TIME_IN_SECOND(start_time,st) ;

    GET_TIME_IN_SECOND(cur_time,ct) ;

    TRACE_DBG("cur_date = %s , cur_time = %s\n",cur_date_str ,cur_time_str );

    /* Check whether the grow cycle to be started or not */
    if((!pDate) && (!pTime))
    {        
        TRACE_INFO("No Start Date & Time given, START GROW CYCLE IMMEDIATELY \n");
        //App.profile_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_profile_rtc_alarm_handler, NULL);
        status = TRUE;
    }
    else if (!pDate)
    {
        TRACE_INFO("No Start Date given , Only Time given, Apply it today \n");

        if(st <= ct)  
        {
            TRACE_INFO("Start time passed already , START GROW CYCLE IMMEDIATELY \n");
            //App.profile_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_profile_rtc_alarm_handler, NULL);
            status = TRUE;
        }
        else
        {
            TRACE_INFO("Start time is yet to arrive , START GROW CYCLE ALARAM \n");
            App.grow_cycle_rtc_alarm_no = RTC_StartAlarm(&start_time, NULL ,app_grow_cycle_rtc_alarm_handler, NULL);
        }
    }
    else if(!pTime)
    {
        TRACE_INFO("No Start Time given, Only Date given \n");

        cmp_value = UTILITY_date_cmp(&start_date,&cur_date);
        if( cmp_value == 0) 
        {
            TRACE_INFO("Start Date Matched & Time Not given, START GROW CYCLE IMMEDIATELY \n");
            //App.profile_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_profile_rtc_alarm_handler, NULL);  
            status = TRUE;          
        }
        else if (cmp_value == -1)
        {
            TRACE_INFO("Start Date is yet to arrive  & Time Not given, START GROW CYCLE ALARAM\n");
            App.grow_cycle_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_grow_cycle_rtc_alarm_handler, NULL);
        }
        else
        {
            TRACE_INFO("Start Date Passed  already(%d) & Time Not given, START GROW CYCLE IMMEDIATELY \n",cmp_value);
            //App.profile_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_profile_rtc_alarm_handler, NULL);
            App.GrowStatus.days_elapsed = cmp_value-1;
            App.GrowStatus.day_no = cmp_value;

            status = TRUE;           
        }        
    }
    else
    {
        TRACE_INFO("Start Date & Start Time given \n");

        cmp_value = UTILITY_date_cmp(&start_date,&cur_date);

        if( cmp_value == 0)  
        {
            if(st <= ct)  
            {
                TRACE_INFO("Start time passed already , START GROW CYCLE IMMEDIATELY \n");
                //App.profile_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_profile_rtc_alarm_handler, NULL);
                status = TRUE;  
            }
            else
            {
                TRACE_INFO("Start time is yet to arrive , START GROW CYCLE ALARAM \n");
                App.grow_cycle_rtc_alarm_no = RTC_StartAlarm(&start_time, NULL ,app_grow_cycle_rtc_alarm_handler, NULL);
            }
        }
        else if (cmp_value == -1)
        {
            TRACE_INFO("Start Date is yet to arrive , START GROW CYCLE ALARAM \n");
            App.grow_cycle_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_grow_cycle_rtc_alarm_handler, NULL);     
        }
        else
        {
            TRACE_INFO("Start Date(%d) is passed already , START GROW CYCLE IMMEDIATELY \n",cmp_value);
            //App.profile_rtc_alarm_no = RTC_StartAlarm(&ref_time, NULL ,app_profile_rtc_alarm_handler, NULL); 
            App.GrowStatus.days_elapsed = cmp_value-1;
            App.GrowStatus.day_no = cmp_value;
            status = TRUE;  
        }
    } 
	return status;   
}




/*******************************************************************************
* Name       :
* Description:
* Remarks    : executes under RTC Alarm ISR
*******************************************************************************/
static void app_profile_rtc_alarm_handler(void* Arg , U32 flag)
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    TRACE_INFO("%s Entry \n", __FUNCTION__);
#endif
    app_send_msg_req_to_q(APP_PROFILE_TIMER_MSG, NULL,0,TRUE);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : executes under RTC Alarm ISR
*******************************************************************************/
static void app_grow_cycle_rtc_alarm_handler(void* Arg , U32 flag)
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    TRACE_INFO("%s Entry \n", __FUNCTION__);
#endif
	app_send_msg_req_to_q(APP_GROW_CYCLE_ALARM_MSG, NULL,0,TRUE);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
static void app_begin_grow_cycle()
{
	TRACE_INFO("%s Entry \n", __FUNCTION__);

    App.ec_state = APP_GROW ;

    App.GrowStatus.min_co2 = 0xffffffff;
    App.GrowStatus.min_temp = 0xffffffff;
    App.GrowStatus.min_humidity = 0xffffffff;

 // TEST_DEV_Light(&App);

 // TEST_FREERTOS_Init();
#if APP_SD_CARD_FLAG
    app_sd_card_prepare();
#endif

    app_send_msg_req_to_q(APP_PROFILE_TIMER_MSG, NULL, 0,FALSE);

#if 1
     OSA_InitTimer(&App.status_timer, App.gc_reporting_period*1000, TRUE, app_grow_cycle_status_handler, NULL);

     OSA_StartTimer(&App.status_timer);
#endif

#if APP_MODBUS_FLAG
     OSA_InitTimer(&App.monitor_timer, APP_MONITOR_PERIOD*1000, TRUE, app_monitor_timer_handler, NULL);

     OSA_StartTimer(&App.monitor_timer);
#endif

}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
static void	app_grow_cycle_alarm_check()
{	
    BOOLEAN to_start_now_or_not = FALSE;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    RTC_StopAlarm(App.grow_cycle_rtc_alarm_no);

    to_start_now_or_not = app_check_to_start_grow_cycle_or_not(App.grow_cycle_start_date,App.grow_cycle_start_time);

    if(to_start_now_or_not)
    {       
        app_begin_grow_cycle();
    }
}

/*******************************************************************************
* Name       :
* Description: It starts the next day profile timer 
* Remarks    : 
*******************************************************************************/
static void	app_start_profile_alarm()
{	
    mTIME time;

    mDATE date;

    U8 time_str[12];

    U8 date_str[12];

    U16 next_week_day = 0;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    RTC_StopAlarm(App.profile_rtc_alarm_no);

    RTC_GetDateTime(&date, &time , date_str ,time_str);

    memset((U8*)&time,0x00,sizeof(time));

    date.WD++;

    if (date.WD > 7)
    {
        next_week_day = 1;
    }
    else
    {
        next_week_day = date.WD % 8;
    }

    memset((U8*)&date,0x00,sizeof(date));

    date.WD = next_week_day;

    App.profile_rtc_alarm_no = RTC_StartAlarm(&time ,&date, app_profile_rtc_alarm_handler, NULL);
}

/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_process_leak_mode()
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
}

/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_process_panic_switch()
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
}

/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_process_ac_power_fault()
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
 			/*	while starting light device , take restrike timer into account */
			DEVICE_FaultHandler(FALSE);	
		}	   			
	}
#endif

}

#if 0
/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_process_emr_fuse_fault()
{
	if(HAL_GPIO_ReadPin(EMR_FUSE_FAULT_GPIO_Port, EMR_FUSE_FAULT_Pin))
	{
		if(App.GrowStatus.emr_fuse_fault)
		{
			App.GrowStatus.emr_fuse_fault = FALSE;

			if(App.WorkingProfile.GrowInfo.to_report_emr_fuse_fault)
			{
				/* Send Alert Message indicating fault cleared  */
				app_send_fault_info();
			}
			DEVICE_FaultHandler(FALSE);
		}	
	}
	else
	{
		if(!App.GrowStatus.emr_fuse_fault)
		{
			App.GrowStatus.emr_fuse_fault = TRUE;
			if(App.WorkingProfile.GrowInfo.to_report_emr_fuse_fault)
			{	
				/* Send Alert Message indicating fault there   */
				app_send_fault_info();
			}
			DEVICE_FaultHandler(TRUE);
		}			
	}
}
#endif

/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_process_rs485_fuse_fault()
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

App_fault_process_rs485_fuse_fault();

}



/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_process_hot_restrike_fault()
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
                app_send_fault_info();
            }           
            DEVICE_FaultHandler(TRUE);
        }
    }
#endif
}
/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_send_fault_info()
{
	U8* buf;

    U8* rsp;
   
    U32 i;

    BOOLEAN user_profile;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    buf = (U8*)malloc(APP_MAX_SIZE_PKT ); // to save thread stack space , take it from heap

    rsp = buf;

    /* fill common  json */
    rsp += App_fill_common_json_header(rsp, APP_MQTT_FAULT_INFO);

 	rsp +=sprintf((char *)rsp,",{\"%s\":", "fault");
    {
        rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "backup_battery_fault",App.GrowStatus.backup_battery_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "ac_power_fault",App.GrowStatus.ac_power_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "hot_restrike_fault",App.GrowStatus.hot_restrike_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "emr_fuse_fault",App.GrowStatus.emr_fuse_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "rs485_fuse_fault",App.GrowStatus.rs485_fuse_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d,", "sensor_fault",App.GrowStatus.sensor_fault);
        rsp +=sprintf((char *)rsp,"\"%s\":%d", "critical_temp_fault",App.GrowStatus.critical_temp_fault);
    }
    rsp +=sprintf((char *)rsp,"]}}");

    if((rsp-buf) > APP_MAX_SIZE_PKT)
    {
        TRACE_FATAL("CONFIG PKT SIZE(%d) more than  APP_MAX_SIZE_PKT(%d)\n",rsp-buf,APP_MAX_SIZE_PKT);
    }
    else
    {
    	MQTT_COM_SendMessage(buf, rsp-buf);

        if (App.GrowStatus.backup_battery_fault  ||
            App.GrowStatus.ac_power_fault        ||
            App.GrowStatus.hot_restrike_fault    ||
            App.GrowStatus.emr_fuse_fault        ||
            App.GrowStatus.rs485_fuse_fault      ||
            App.GrowStatus.sensor_fault)
        {
            app_send_email(buf, rsp-buf, "FAULT Occured");
        }
        {
            app_send_email(buf, rsp-buf, "FAULT Restored");
        }
    } 
    free(buf);

    TRACE_INFO("%s Exit \n", __FUNCTION__);	
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    : it executes under freeRTOS timer task context
*******************************************************************************/
static void app_monitor_timer_handler(void *arg)
{
	/* post monitor timer message to app action thread . This is required
	   not to burden RTOS timer thread */
	app_send_msg_req_to_q(APP_MONITOR_TIMER_MSG, NULL, 0,FALSE);
   
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
static void app_monitor_sensor()
{
    BOOLEAN need_to_send_fault = FALSE;
    U16  sensor_value[3];


    BOOLEAN status = FALSE;
	//app_monitor_backup_battery();

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* get sensor values from sensor module */
    memset((U8*)sensor_value,0x00,sizeof(sensor_value));
    do 
    {
        status = MODBUS_GetSetParam(0x2a ,FC_GET_SENSOR_DATA , (void*)sensor_value);

        if(!status)
        {   
          //  OSA_Sleep(1); 
           OSA_Usleep(200000);
        } 
    } while (!status);
    TRACE_INFO("Sensor Values  temp : %i   CO2 : %i  Humidity : %i \n", sensor_value[1], sensor_value[0] ,sensor_value[2]);

#if 1 
#if 0
    App.GrowStatus.cur_co2 += sensor_value[0]/APP_NO_OF_AVERAGE_SENSOR_READINGS;
    App.GrowStatus.cur_temp  += sensor_value[1]/APP_NO_OF_AVERAGE_SENSOR_READINGS;
    App.GrowStatus.cur_humidity  += sensor_value[2]/APP_NO_OF_AVERAGE_SENSOR_READINGS;
#else
    App.GrowStatus.cur_co2 = sensor_value[0];
    App.GrowStatus.cur_temp  = sensor_value[1];
    App.GrowStatus.cur_humidity  = sensor_value[2];
#endif

    TRACE_INFO("Sensor Values  temp : %i   CO2 : %i  Humidity : %i \n", sensor_value[1], sensor_value[0] ,sensor_value[2]);

    /* calculate max min values */
    App.GrowStatus.max_co2 = (App.GrowStatus.max_co2 < sensor_value[0]) ? sensor_value[0]:App.GrowStatus.max_co2;
    App.GrowStatus.min_co2 = (App.GrowStatus.min_co2 > sensor_value[0]) ? sensor_value[0]:App.GrowStatus.min_co2;

    App.GrowStatus.max_temp = (App.GrowStatus.max_temp < sensor_value[1]) ? sensor_value[1]:App.GrowStatus.max_temp;
    App.GrowStatus.min_temp = (App.GrowStatus.min_temp > sensor_value[1]) ? sensor_value[1]:App.GrowStatus.min_temp;

    App.GrowStatus.max_humidity = (App.GrowStatus.max_humidity < sensor_value[2]) ? sensor_value[2]:App.GrowStatus.max_humidity;
    App.GrowStatus.min_humidity = (App.GrowStatus.min_humidity > sensor_value[2]) ? sensor_value[2]:App.GrowStatus.min_humidity;

    /* check CO2 */
    if((sensor_value[0] < App.WorkingProfile.GrowInfo.low_co2_threshold_value) &&
       (App.WorkingProfile.GrowInfo.to_report_low_co2_threshold))
    {
        App.GrowStatus.low_co2_state = TRUE;
    }
    else
    {
        App.GrowStatus.low_co2_state = FALSE;
    }

    if((sensor_value[0] > App.WorkingProfile.GrowInfo.high_co2_threshold_value) &&
       (App.WorkingProfile.GrowInfo.to_report_high_co2_threshold))
    {
        App.GrowStatus.high_co2_state = TRUE;
    }
    else
    {
        App.GrowStatus.high_co2_state = FALSE;
    }       
      /* check temperature */
    if((sensor_value[1] < App.WorkingProfile.GrowInfo.low_temp_threshold_value) &&
       (App.WorkingProfile.GrowInfo.to_report_low_temp_threshold))
    {
        //update status field 
        App.GrowStatus.low_temp_state = TRUE;
    }
    else
    {
        App.GrowStatus.low_temp_state = FALSE;
    } 
    if((sensor_value[1] > App.WorkingProfile.GrowInfo.high_temp_threshold_value) &&
       (App.WorkingProfile.GrowInfo.to_report_high_temp_threshold))
    {
        //update status field 
        App.GrowStatus.high_temp_state = TRUE;
    } 
    else
    {
        App.GrowStatus.high_temp_state = FALSE;
    } 
    if((sensor_value[1] > App.WorkingProfile.GrowInfo.critical_temp_threshold_value) &&
       (App.WorkingProfile.GrowInfo.to_report_critical_temp_threshold))
    {
        App.GrowStatus.critical_temp_fault = TRUE;

        need_to_send_fault = TRUE;
    }
    else
    {

        if(App.GrowStatus.critical_temp_fault )
        {
            /* check whether the fault occured earlier or not */
            need_to_send_fault = TRUE;
        }
        App.GrowStatus.critical_temp_fault = FALSE;
    } 

      /* check humidity */
    if((sensor_value[2] < App.WorkingProfile.GrowInfo.low_humidity_threshold_value) &&
       (App.WorkingProfile.GrowInfo.to_report_low_humidity_threshold))
    {
        App.GrowStatus.low_humidity_state = TRUE;
    }
    else
    {
        App.GrowStatus.low_humidity_state = FALSE;
    } 
    if((sensor_value[2] > App.WorkingProfile.GrowInfo.high_humidity_threshold_value) &&
       (App.WorkingProfile.GrowInfo.to_report_high_humidity_threshold))
    {
        App.GrowStatus.high_humidity_state = TRUE;
    } 
    else
    {
        App.GrowStatus.high_humidity_state = FALSE;
    }

    if(App.GrowStatus.critical_temp_fault )
    {
        TRACE_INFO("%s Critcal TEMP FAULT OCCURED, Sending Fault info \n", __FUNCTION__); 
        app_send_fault_info() ; 
    }
#endif

}

/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_monitor_backup_battery()
{

    U32 measured_battery_value = 0x00; 

#ifdef NUCLEO_F439ZI_EVK_BOARD
	// Read ADC , battery voltage TBD
    //connect the load 
    HAL_GPIO_WritePin(BATTERY_TEST_PULSE_GPIO_Port, BATTERY_TEST_PULSE_Pin, GPIO_PIN_SET);
    measured_battery_value = ADC_ReadValuePolling(ADC_1,ADC_CHANNEL_VBAT); 
    //dis connect load
    HAL_GPIO_WritePin(BATTERY_TEST_PULSE_GPIO_Port, BATTERY_TEST_PULSE_Pin, GPIO_PIN_RESET);
#endif

#ifdef ESP32_S2_MINI
    gpio_set_direction(BATTERY_TEST_PULSE_Pin, GPIO_MODE_OUTPUT);
    gpio_set_level(BATTERY_TEST_PULSE_Pin, GPIO_PIN_HIGH);
    measured_battery_value = ADC_ReadValuePolling(ADC_1,ADC_CHANNEL_BATTERY_VOLTAGE);
    gpio_set_level(BATTERY_TEST_PULSE_Pin, GPIO_PIN_LOW); 
#endif  

    App.GrowStatus.cur_battery_volatge = App.WorkingProfile.GrowInfo.backup_battery_voltage /0x1000; 

    TRACE_DBG("Battery Voltage = %d\n", measured_battery_value);

    if( App.GrowStatus.cur_battery_volatge <= App.WorkingProfile.GrowInfo.backup_battery_low_threshold_voltage)
    {
        if(!App.GrowStatus.backup_battery_fault)
        {
            App.GrowStatus.backup_battery_fault = TRUE;

            if(App.WorkingProfile.GrowInfo.to_report_battery_fault)
            {
                /* Send Alert Message indicating fault cleared  */
                app_send_fault_info();
            }
            DEVICE_FaultHandler(TRUE);
        }
    }
    else if (App.GrowStatus.cur_battery_volatge >= App.WorkingProfile.GrowInfo.backup_battery_high_threshold_voltage)
    {
        if(App.GrowStatus.backup_battery_fault)
        {
            App.GrowStatus.backup_battery_fault = FALSE;

            if(App.WorkingProfile.GrowInfo.to_report_battery_fault)
            {
                /* Send Alert Message indicating fault cleared  */
                app_send_fault_info();
            }
            DEVICE_FaultHandler(FALSE);
        }
    }			

}



/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_mqtt_process_device_request(U8*        pBuf,
                                             jsmntok_t* pTok,
                                             U32        token_count)
{
	DEVICE_STATE state = USUAL;

    U32 i;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"action",APP_COMPARE_LENGTH("action")))
            {
                i++;
                if (!memcmp((char*)(pBuf + pTok[i].start),"suspend",APP_COMPARE_LENGTH("suspend")))
                {
					state = SUSPENDED;
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"forced",APP_COMPARE_LENGTH("forced")))
                {
					state = FORCED;
                }
                else if (!memcmp((char*)(pBuf + pTok[i].start),"normal",APP_COMPARE_LENGTH("forced")))
                {
					state = USUAL;
                }
            }
        }
    }

	for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"light",APP_COMPARE_LENGTH("light")))
            {
				App.WorkingProfile.DevLightConfig.DevState = state;
				DEVICE_StateHandler(DEVICE_LIGHT);
            }
			else if (!memcmp((char*)(pBuf + pTok[i].start),"hoodvent",APP_COMPARE_LENGTH("hoodvent")))
            {
				App.WorkingProfile.DevHoodVentConfig.DevState = state;
				DEVICE_StateHandler(DEVICE_HOOD_VENT);
            }
			else if (!memcmp((char*)(pBuf + pTok[i].start),"circ",APP_COMPARE_LENGTH("circ")))
            {
				App.WorkingProfile.DevCircConfig.DevState = state;
				DEVICE_StateHandler(DEVICE_CIRC);
            }
			else if (!memcmp((char*)(pBuf + pTok[i].start),"vent",APP_COMPARE_LENGTH("vent")))
            {
				App.WorkingProfile.DevVentConfig.DevState = state;
				DEVICE_StateHandler(DEVICE_VENT);
            }
			else if (!memcmp((char*)(pBuf + pTok[i].start),"fxp1",APP_COMPARE_LENGTH("fxp1")))
            {
				App.WorkingProfile.DevFxp1Config.DevState = state;
				DEVICE_StateHandler(DEVICE_FXP1);
            }
			else if (!memcmp((char*)(pBuf + pTok[i].start),"fxp2",APP_COMPARE_LENGTH("fxp2")))
            {
				App.WorkingProfile.DevFxp2Config.DevState = state;
				DEVICE_StateHandler(DEVICE_FXP2);
            }
        }
    }
    
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_init_sd_card()
{ 
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* Initialize SD card & file system */
#ifdef NUCLEO_F439ZI_EVK_BOARD
    SPI_Init(&App.Spi1,SPI1_HW_INDEX,SPI_MODE0);
#endif

#ifdef ESP32_S2_MINI
    SPI_Init(&App.Spi1,SPI2_HW_INDEX,SPI_MODE0);
#endif

    SPI_DMAInit(&App.Spi1);

    SD_CARD_Init(&App.SdCard,SPI_IFACE, &App.Spi1 );

    TRACE_DBG("Init SD Card COMPLETED\n");
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void  app_sd_card_prepare()
{
    U8 status_dir[CONFIG_GROW_CYCLE_NAME_LEN+7];
    U8 fault_dir[CONFIG_GROW_CYCLE_NAME_LEN+7];

    sprintf(status_dir,"/%s/status",App.gc_name);
    sprintf(fault_dir,"/%s/faults",App.gc_name);

    if(SD_CARD_is_sd_card_present())
    {
        TRACE_INFO("SD Card Available \n");
        SD_CARD_create_dir(status_dir);
        SD_CARD_create_dir(fault_dir);
    }
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static app_sd_card_store_status(U8* name, U8* Buf, U32 len)
{
    U8 file_name[CONFIG_GROW_CYCLE_NAME_LEN+7+25];

	U8* p;
	U8* q;
	U32 i;


    for(i=0; i<2; i++)
    {
        p = name;
        q = strchr(p,'/');
        *q = '_';
    }

    p = name;
    q = strchr(p,' ');

#if 0
    *q = '_';


    for(i=0; i<2; i++)
    {
        p = name;
        q = strchr(p,':');
        *q = '_';
    }
    sprintf(file_name,"/%s/%s",App.gc_name, name);

#else

    *q = 0x00;
    sprintf(file_name,"/%s/%s",App.gc_name, name);
#endif

 //  sprintf(file_name,"/%s/status/%s",App.gc_name, name);

	Buf[len]='\r';
	Buf[len+1]='\n';

    if(SD_CARD_is_sd_card_present())
    {
        if(SD_CARD_is_file_present(file_name))
        {
            SD_CARD_append_file(Buf, len+2, file_name);
        }
        else
        {
            TRACE_INFO("file name = %s\n", file_name);
            SD_CARD_create_file(Buf, len+2, file_name);
        }
    }
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_check_reset_apply_recovery()
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    U32 reset_status ;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    reset_status = *((U32*)(PERIPH_BASE+RCC_CSR_OFFSET)) ;

    printf("RESET OCCURED ( %x )\n", reset_status);

    if(reset_status & 0x10000000)
    {
        TRACE_INFO("%s SW RESET \n", __FUNCTION__);

        app_send_msg_req_to_q(APP_DO_RESET_RECOVERY_MSG, NULL, 0,FALSE);

    	/* some SW  reset happened ,Do recovery  */        
        *((U32*)(PERIPH_BASE+RCC_CSR_OFFSET)) |= 0x01000000;
    }
#endif

#ifdef ESP32_S2_MINI
    esp_reset_reason_t reset_status ;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    reset_status = esp_reset_reason();

    printf("RESET OCCURED ( %x )\n", reset_status);

    if(reset_status == ESP_RST_SW)
    {
        TRACE_INFO("%s SW RESET \n", __FUNCTION__);

        app_send_msg_req_to_q(APP_DO_RESET_RECOVERY_MSG, NULL, 0,FALSE);
    }
#endif
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_do_reset_recovery()
{
    APP_SW_RESET_CAUSE reason;

	BOOLEAN gc_restored ;
    
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    reason = App_backup_reset_reason();

	TRACE_INFO("RESEST reason = %d \n",reason );

    switch(reason)
    {         
        case RESET_FIRMWARE_UPGRADE:
            if(App_backup_retreive())
            {
                TRACE_INFO("BACKUP RETREIVED SUCCESSFULLY \n" );

                if(App.ec_state == APP_GROW)
                {
                    app_begin_grow_cycle();                    
                } 
				gc_restored = TRUE;            
            } 
            else
            {
                TRACE_INFO("BACKUP NOT RETREIVED \n" );
				gc_restored = FALSE; 
            }
            /* send firmware upgrade status */
			app_mqtt_send_fwug_status(gc_restored );
            break;

        default :
            TRACE_INFO("IGNORE RESET \n" );
            break;
    }
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_mqtt_send_fwug_status(BOOLEAN gc_restored )
{
    U8* buf;

    U8* rsp;
   
    U32 i = 0x00;

    BOOLEAN status ;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    buf = (U8*)malloc(APP_MAX_SIZE_PKT ); // to save thread stack space , take it from heap

    rsp = buf;

    /* fill common  json */
    rsp += App_fill_common_json_header(rsp, APP_MQTT_FW_UG_STATUS);

    rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",", "app_ver", APP_EC_VERSION);

    rsp +=sprintf((char *)rsp,"\"%s\":%d}]}}", "grow_cycle_restored", gc_restored);

    i = 0x00;

RESEND:
    status = MQTT_COM_SendMessage(buf, rsp-buf);

    if(!status)
    {
        i++;
        OSA_Sleep(1);
        if( i<5)
        {
            goto RESEND;
        }
    }

    free(buf);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_sd_card_get_status_file_ptr()
{
    U8 file_name[CONFIG_GROW_CYCLE_NAME_LEN+7+25];

    U8* p;
    U8* q;
    U32 i;
	U8* date;
    TRACE_INFO("%s() Entry \n", __FUNCTION__);

	if(App.gc_status_file_ptr)
	{
		TRACE_INFO("Closing Earlier file ptr \n");
		SD_CARD_close_file(&App.gc_status_file_ptr);
	}

	/* get RTC time */
   	RTC_GetTime(App.GrowStatus.rtc_time);
	p = App.GrowStatus.rtc_time;
    q = strchr(p,' ');
    q++;

	date = q;
    for(i=0; i<2; i++)
    {
        p = date;
        q = strchr(p,'/');
        *q = '_';
    }

    p = date;
    q = strchr(p,' ');
    *q = 0x00;

    sprintf(file_name,"/%s/%s.txt",App.gc_name, date);

	TRACE_INFO("file name = %s\n", file_name);

    App.gc_status_file_ptr = SD_CARD_get_file_ptr(file_name);

    TRACE_INFO("%s() Exit \n", __FUNCTION__);
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_sd_card_store( U8* Buf, U32 len)
{
    TRACE_INFO("%s() Entry \n", __FUNCTION__);
    Buf[len]='\r';
    Buf[len+1]='\n';
    SD_CARD_append_file2(Buf, len+2, &App.gc_status_file_ptr );
    TRACE_INFO("%s() Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_send_email(U8* pMsg, U32 msg_len, U8* subject)
{
    CONFIG_PROFILE_TYPE prof_type = CONFIG_FACTORY_PROFILE;

    Config_SmtpInfo*  pEmailInfo;

    SMTP_account_t  account;

    SMTP_email_t     email;

    mDATE Date;

    mTIME Time;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    pEmailInfo = malloc(sizeof(Config_SmtpInfo));  //to save thread stack space , take it from heap

    memset(pEmailInfo,0x00,sizeof(Config_SmtpInfo));

    if(CONFIG_IsUserProvisonPresent())
    {
        TRACE_DBG("USER PROVISIONING  PRESENT \n" );

        prof_type = CONFIG_USER_PROFILE;
    }

    CONFIG_ReadTag(0,prof_type,CONFIG_SMTP_INFO_TAG,(U8*)pEmailInfo);

    /* fill account details */
    memset(&account ,0x00, sizeof(SMTP_account_t));
    account.email_address = pEmailInfo->email_address;
    account.password = pEmailInfo->password;
    account.host_name = pEmailInfo->smtp_host_name;
    account.port = pEmailInfo->smtp_port;
    account.smtp_encryption = pEmailInfo->smtp_encryption;

    /* fill email details */
    memset(&email ,0x00, sizeof(SMTP_email_t));
    email.to_addresses = pEmailInfo->to_addresses;

    if(strlen(pEmailInfo->cc_addresses))
    {
        email.cc_addresses = pEmailInfo->cc_addresses;
    }
    if(strlen(pEmailInfo->bcc_addresses))
    {
        email.bcc_addresses = pEmailInfo->bcc_addresses;
    }
    if(strlen(pEmailInfo->signature))
    {
        email.signature = pEmailInfo->signature;
    }
    email.subject = subject;
    email.content = pMsg;

    RTC_GetDateTime(&Date,&Time,NULL,NULL);
    snprintf( (char *)email.date,
              sizeof( email.date ),
              "%s, %.02d %s %.04d %.02d:%.02d:%.02d +0000",
              app_smtp_weekday_abbreviations[Date.WD],
              Date.DD,
              app_smtp_month_abbreviations[Date.MM-1],
              Date.YY,
              Time.HH,
              Time.MM,
              Time.SS);

    /* send email */
    if( SMTP_STATUS_OK != SMTP_SendEmail(&account, &email ))
    {
        TRACE_WARN("Email Sending Failed  \n" );
    }
    free(pEmailInfo);
}



/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_send_mqtt_cmd_ack(APP_MQTT_CMD_TYPE cmd)
{
    U8* buf;

    U8* rsp;

    buf = malloc(512);

    rsp = buf;

    rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",", "type", "mqtt");
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "device_name", App.DevConfig.device_name);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "device_id", App.DevConfig.device_id);
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "farm_name", App.DevConfig.firm_name);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "greenhouse_id", App.DevConfig.greenhouse_id);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "mobile_app_id", App.DevConfig.mobile_app_id);
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "ver", APP_EC_PROTOCOL_VERSION);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "flag", 0);
    rsp +=sprintf((char *)rsp,"\"%s\":{\"%s\":%d }}", "ack", "cmd",cmd);

    MQTT_COM_SendMessage(buf, rsp-buf);

    free(buf);
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_process_device_timer_expiry_msg(U8* buf)
{
    Device_Timer_Msg*  dev_timer_msg = (Device_Timer_Msg* ) buf;

    TRACE_DBG("%s() Entry \n",__FUNCTION__ );

    dev_timer_msg->fp(dev_timer_msg->device_obj);
}
/*******************************************************************************
*                          End of File
*******************************************************************************/
