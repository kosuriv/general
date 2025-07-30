/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : app.h
*
*  Description         : This has all declarations for the controller applictaion.
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  23rd July 2020    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef APP_H
#define APP_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "flash.h"
#include "config.h"
#include "nw_wifi_esp.h"
#include "mqtt_com.h"
#include "rtc.h"
#include "grow_config.h"

#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "uart.h"
#include "adc.h"
#endif

#include "spi.h"  
#include "sd_card.h"
#include "web_provision.h"
#include "modbus_master.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

#define APP_COMPARE_LENGTH(s)  sizeof(s)-1

#define APP_EC_VERSION "1.0.0"

#define APP_EC_PROTOCOL_VERSION "1.0.0"  //protocol between EC & Mobile APP 

#define APP_EC_PROFILE_VERSION "1.0.0"  //storage of factory profile configuration 

#define APP_TOKEN_MAX_COUNT  300

#define APP_NO_OF_AVERAGE_SENSOR_READINGS  5  // last 5 readings average 
#define APP_MONITOR_PERIOD  5  //5 seconds to monitor sensors & battery voltage 

#define APP_WEB_PROVISIONING_PORT_NO  9393  //port for web server connection
/* App state */
typedef enum APP_STATE
{
    APP_IDLE,
    APP_GROW
}APP_STATE;

//messages  for app action thread 
typedef enum APP_MSG_TYPE
{
    APP_MQTT_MSG,
    APP_PROFILE_TIMER_MSG,
    APP_STATUS_TIMER_MSG,
    APP_GROW_CYCLE_ALARM_MSG,
    APP_LIGHT_CYCLE_ALARM_MSG,
    APP_APPLY_NEW_PROFILE_MSG,
    APP_LEAK_SENSOR_MSG,
    APP_PANIC_BUTTON_MSG,
    APP_AC_POWER_FAULT_MSG,
    APP_EMR_FUSE_FAULT_MSG,
    APP_RS485_FUSE_FAULT_MSG,
    APP_HOT_RESTRIKE_FAULT_MSG,
    APP_MONITOR_TIMER_MSG,
    APP_DO_RESET_RECOVERY_MSG,
    APP_FWUG_TIMER_EXPIRY_MSG,
    APP_DEVICE_TIMER_EXPIRY_MSG
}APP_MSG_TYPE;

/* commands get exchanged between EC & mobile Application */
typedef enum APP_MQTT_CMD_TYPE
{
    APP_MQTT_PROVISION_INFO,    /* to set provision info such as email/sms ph no ,wifi network credentials, mqtt credentials */
    APP_MQTT_PROFILE_INFO,    //to set new or update profile, or get the profile 
    APP_MQTT_GROW_CYCLE_INFO, //to start or stop the grow cycle
    APP_MQTT_STATUS_INFO,     //to send status info with all alerts info 
    APP_MQTT_FAULT_INFO,      // to send fault status 
    APP_MQTT_DEVICE_REQUEST , // used to put device in Forced , suspend states etc  
    APP_MQTT_FW_UG_REQUEST,    // to update the FIRMWARE of EC 
    APP_MQTT_FW_UG_STATUS     // to send the status of after FIRMWARE upgrade by EC 
}APP_MQTT_CMD_TYPE;


/*  message format for app action thread */
typedef struct App_Msg_Req
{
    APP_MSG_TYPE  type;
    U32   len;
    void* buf;
} App_MsgReq;


/* types of RESETs */
typedef enum APP_SW_RESET_CAUSE
{
    RESET_NONE,
    RESET_FIRMWARE_UPGRADE ,
    RESET_USER_PROVISION
}APP_SW_RESET_CAUSE;


typedef struct APP
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
	UART    Uart3Driver;
    UART    Uart2Driver; //used for ESP32
#endif

    SPI Spi1;
  
    SD_CARD SdCard;

	FLASH_T FlashDriver;

	CONFIG  Config;

    NW_WIFI_ESP  NwEspDriver;

    MQTT_COM     Mqtt;
    
    RTC_T  Rtc;

    osa_thread_handle action_thread;

    osa_msgq_type_t pActionMsgQ;

    osa_timer_type_t status_timer;

    APP_STATE  ec_state;

    Config_WiFiInfo  WifiConfig;

    Config_MQTTInfo  MqttConfig;

    Config_DeviceInfo DevConfig;

    GrowConfig_GrowCycleStatus  GrowStatus;

    S32 profile_rtc_alarm_no;

    S32 grow_cycle_rtc_alarm_no; //future alarm

    U8  grow_cycle_start_time[12];

    U8 grow_cycle_start_date[12]; 

    Config_Profile WorkingProfile;

    Config_Profile* pNewProfile;

    osa_timer_type_t apply_profile_timer;

    BOOLEAN apply_new_profile;

    U8  apply_profile_date[12];

    U8  apply_profile_time[12];

    U32 apply_profile_st;

    CONFIG_PROFILE_TYPE  apply_profile;

    U32 apply_profile_no;

    U8 gc_name[CONFIG_GROW_CYCLE_NAME_LEN+1];

    U32 gc_profile_no;

    U32 gc_reporting_period;

    U32 gc_total_days;

    CONFIG_PROFILE_TYPE  gc_profile;

    osa_timer_type_t monitor_timer;

    osa_mutex_type_t msg_q_mutex;

    void* gc_status_file_ptr;

    osa_timer_type_t fwug_timer;

    WEB_PROVISION   WebProv;

} APP;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void APP_Start();

void APP_do_light_cycle_transition();

void App_apply_new_profile();

void APP_FaultISR();

void APP_send_hot_restrike_fault_msg();

void App_Reset(APP_SW_RESET_CAUSE reason);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
