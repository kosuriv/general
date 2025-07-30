/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : config.h
*
*  Description         : This has all declarations for all profile datafor
                         configuration
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  7th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "memory_layout.h"
#include "profile.h"
#include "profile_data_types.h"
#include "grow_config.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

typedef enum
{
    CONFIG_SYSTEM_INFO_TAG,
	CONFIG_DEVICE_INFO_TAG,
	CONFIG_WIFI_INFO_TAG,
	CONFIG_MQTT_INFO_TAG,
    CONFIG_SMTP_INFO_TAG,
    CONFIG_GROW_CYCLE_INFO_TAG,
    CONFIG_DEV_LIGHT_INFO_TAG,
    CONFIG_DEV_HOOD_VENT_INFO_TAG,
    CONFIG_DEV_CIRC_INFO_TAG,
    CONFIG_DEV_FXP1_INFO_TAG,
    CONFIG_DEV_FXP2_INFO_TAG,
    CONFIG_DEV_VENT_INFO_TAG,
    CONFIG_LAST_TAG

} CONFIG_TAG;

               /* Define all tags with their parameters, initializing macro  */

              /* --------------  */

#define CONFIG_MANUFACTURER_NAME_LEN        16      /* Maximum length for manufacturer name */
#define CONFIG_MODEL_TYPE_LEN               16      /* Maximum length for model type */
#define CONFIG_SERIAL_NUMBER_LEN            16      /* Maximum length for serial number */

typedef struct __attribute__((aligned(4),packed)) Config_SystemInfo
{
    U8         manufacturer[CONFIG_MANUFACTURER_NAME_LEN]; /* Manufacturer name. */
    U8         model_type[CONFIG_MODEL_TYPE_LEN];          /* Model type. */
    U8         sn[CONFIG_SERIAL_NUMBER_LEN];               /* Serial number. */

} Config_SystemInfo;

#define CONFIG_SYSTEM_INFO_INIT(STORAGE, BUFFER, TAG, P1, P2, P3)                \
do {                                                                             \
    Config_SystemInfo *pBuf = (Config_SystemInfo *)BUFFER;                       \
    memset(BUFFER,0x00,sizeof(Config_SystemInfo));                               \
    strcpy((char *)pBuf->manufacturer, P1);                                      \
    strcpy((char *)pBuf->model_type, P2);                                        \
    strcpy((char *)pBuf->sn, P3);                                                \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(Config_SystemInfo), BUFFER);\
} while(0)

              /* --------------  */

#define CONFIG_DEVICE_NAME_LEN        24    /* Maximum length for device name */
#define CONFIG_FIRM_NAME_LEN          24    /* Maximum length for firm name */


typedef struct __attribute__((aligned(4),packed)) Config_DeviceInfo
{
    U8         device_name[CONFIG_DEVICE_NAME_LEN]; /* device name. */
    U8         firm_name[CONFIG_FIRM_NAME_LEN]; /* green house firm name */
    U32        device_id;                                /* device id. */
    U32        greenhouse_id;           /* green house id. */
    U32        mobile_app_id;           /* corresponding mobile application id*/
    CONTROLLER_TYPE  controller_type;        /* controller type */

} Config_DeviceInfo;

#define CONFIG_DEVICE_INFO_INIT(STORAGE, BUFFER, TAG, P1, P2, P3,P4,P5,P6)       \
do {                                                                             \
    Config_DeviceInfo *pBuf = (Config_DeviceInfo *)BUFFER;                       \
    memset(BUFFER,0x00,sizeof(Config_DeviceInfo));                               \
    strcpy((char *)pBuf->device_name, P1);                                       \
    strcpy((char *)pBuf->firm_name, P2);                                         \
    pBuf->device_id = P3;                                                        \
    pBuf->greenhouse_id = P4;                                                    \
    pBuf->mobile_app_id = P5;                                                    \
    pBuf->controller_type = P6;                                                    \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(Config_DeviceInfo), BUFFER);\
} while(0)

              /* --------------  */

#define CONFIG_WIFI_SSID_LEN        		32      /* Maximum length for WiFi SSID */
#define CONFIG_WIFI_PASSPHRASE_LEN          32      /* Maximum length for WiFi Password */
#define CONFIG_WIFI_TIMEZONE_LEN            24      /* Maximum length for country time zone */


typedef struct __attribute__((aligned(4),packed)) Config_WiFiInfo
{
    U8          ssid[CONFIG_WIFI_SSID_LEN+1]; 				 /* WiFi SSID. */
    U8          passphrase[CONFIG_WIFI_PASSPHRASE_LEN+1];    /* WiFi pass phrase in psk , password in PEAP , key password in TLS*/
    U32         serial_baudrate;
#ifdef NUCLEO_F439ZI_EVK_BOARD
    S32         time_zone;
#endif    

#ifdef ESP32_S2_MINI
    U8          time_zone[CONFIG_WIFI_TIMEZONE_LEN ];        /* WiFi time zone */
#endif

    U32         security;                                    /* WiFi security */
    U8          eap_identity[CONFIG_WIFI_SSID_LEN+1];        /* WiFi eap identity */
    U8          username[CONFIG_WIFI_SSID_LEN+1];            /* WiFi username */ 

} Config_WiFiInfo;

#ifdef NUCLEO_F439ZI_EVK_BOARD
#define CONFIG_WIFI_INFO_INIT(STORAGE, BUFFER, TAG, P1, P2, P3, P4)             \
do {                                                                            \
    Config_WiFiInfo *pBuf = (Config_WiFiInfo *)BUFFER;                          \
    memset(BUFFER,0x00,sizeof(Config_WiFiInfo));                                \
    strcpy((char *)pBuf->ssid, P1);                                             \
    strcpy((char *)pBuf->passphrase, P2);                                       \
    pBuf->serial_baudrate = P3;                                                 \
    pBuf->time_zone = P4;                                                       \
   (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(Config_WiFiInfo), BUFFER);  \
}while(0)
#endif


#ifdef ESP32_S2_MINI
#define CONFIG_WIFI_INFO_INIT(STORAGE, BUFFER, TAG, P1, P2, P3, P4)	    \
do {																		 	\
	Config_WiFiInfo *pBuf = (Config_WiFiInfo *)BUFFER;                       	\
	memset(BUFFER,0x00,sizeof(Config_WiFiInfo));                                \
	strcpy((char *)pBuf->ssid, P1);       	 		                            \
	strcpy((char *)pBuf->passphrase, P2);                                   	\
    pBuf->serial_baudrate = P3;                                                 \
    strcpy((char *)pBuf->time_zone, P4);                                        \
   (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(Config_WiFiInfo), BUFFER);  \                                           
}while(0)
#endif
              /* --------------  */

#define CONFIG_MQTT_HOST_LEN        		100      /* Maximum length for MQTT host */
#define CONFIG_MQTT_USERNAME_LEN			20		 /* Maximum length for MQTT User name */
#define CONFIG_MQTT_PASSWORD_LEN            20       /* Maximum length for MQTT Password */
#define CONFIG_MQTT_TOPIC_LEN				20		 /* Maximum length for MQTT Topic  */

typedef struct __attribute__((aligned(4),packed)) Config_MQTTInfo
{
	U8 		host[CONFIG_MQTT_HOST_LEN];
    U8      username[CONFIG_MQTT_USERNAME_LEN];
    U8      password[CONFIG_MQTT_PASSWORD_LEN];
    U8      topic[CONFIG_MQTT_TOPIC_LEN];
    U32		port_no;
    U32	    qos_type;
    U32     security;                                    
} Config_MQTTInfo;

#define CONFIG_MQTT_INFO_INIT(STORAGE, BUFFER, TAG, P1, P2, P3, P4, P5, P6)		 \
do {																		 	 \
	Config_MQTTInfo *pBuf = (Config_MQTTInfo *)BUFFER;                       	 \
	memset(BUFFER,0x00,sizeof(Config_MQTTInfo));                                 \
	strcpy((char *)pBuf->host, P1);       	 	                         	     \
	strcpy((char *)pBuf->username, P2);       	                                 \
	strcpy((char *)pBuf->password, P3);                                          \
	strcpy((char *)pBuf->topic, P4);                                             \
	pBuf->port_no = P5;                     							 	     \
	pBuf->qos_type = P6;                     								     \
	(STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(Config_MQTTInfo), BUFFER);	 \
}while(0)

              /* --------------  */

#define CONFIG_SMTP_MAX_INFO_LENGTH               127   

typedef struct __attribute__((aligned(4),packed)) Config_SmtpInfo
{
    U8       email_address[CONFIG_SMTP_MAX_INFO_LENGTH+1];  /*  account's email address   */
    U8       password[CONFIG_SMTP_MAX_INFO_LENGTH+1];       /*  password                  */
    U8       smtp_host_name[CONFIG_SMTP_MAX_INFO_LENGTH+1]; /*  SMTP server address       */
    U32      smtp_port;                                     /*  SMTP server port          */
    BOOLEAN  smtp_encryption;                               /*  encryption                */
    U8       to_addresses[CONFIG_SMTP_MAX_INFO_LENGTH+1];   /* Recipients' email addresses separated by commas */    
    U8       cc_addresses[CONFIG_SMTP_MAX_INFO_LENGTH+1];   /* Cc email addresses separated by commas          */
    U8       bcc_addresses[CONFIG_SMTP_MAX_INFO_LENGTH+1];  /* Bcc email addresses separated by commas         */
    U8       signature[CONFIG_SMTP_MAX_INFO_LENGTH+1];      /* signature         */

} Config_SmtpInfo;

#define CONFIG_SMTP_INFO_INIT(STORAGE, BUFFER, TAG, P1, P2, P3, P4, P5, P6, P7, P8, P9)  \
do {                                                                            \
    Config_SmtpInfo *pBuf = (Config_SmtpInfo *)BUFFER;                          \
    memset(BUFFER,0x00,sizeof(Config_SmtpInfo));                                \
    strcpy((char *)pBuf->email_address, P1);                                    \
    strcpy((char *)pBuf->password, P2);                                         \
    strcpy((char *)pBuf->smtp_host_name, P3);                                   \
    pBuf->smtp_port = P4;                                                       \
    pBuf->smtp_encryption = P5;                                                 \
    strcpy((char *)pBuf->to_addresses, P6);                                     \
    if(P7)                                                                      \
    {                                                                           \
        strcpy((char *)pBuf->cc_addresses, P7);                                 \   
    }                                                                           \
    if(P8)                                                                      \
    {                                                                           \
        strcpy((char *)pBuf->bcc_addresses, P8);                               \   
    }                                                                           \
    if(P9)                                                                      \
    {                                                                           \
        strcpy((char *)pBuf->signature, P9);                                   \   
    }                                                                           \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(Config_SmtpInfo), BUFFER); \
}while(0)



              /* --------------  */
#define CONFIG_GROW_CYCLE_INFO_INIT(STORAGE, BUFFER, TAG)          			     		 \
do {                                                                                     \
    GrowConfig_GrowCycleInfo *pBuf = (GrowConfig_GrowCycleInfo *)BUFFER;                 \
    memset(BUFFER,0x00,sizeof(GrowConfig_GrowCycleInfo));                                \
    pBuf->measure_unit = UNIT_ENGLISH;                                                   \
    pBuf->clk = TWELEVE_HR;                                                              \
    pBuf->language = ENGLISH;                                                            \
    pBuf->low_temp_threshold_value = 25;                                                 \
    pBuf->high_temp_threshold_value = 35;                                                \
    pBuf->critical_temp_threshold_value = 30;                                            \
    pBuf->to_report_low_temp_threshold = TRUE;                                           \
    pBuf->to_report_high_temp_threshold = TRUE;                                          \
    pBuf->to_report_critical_temp_threshold = TRUE;                                      \
    pBuf->low_humidity_threshold_value = 30; /* 50 to 60% normal range*/                 \
    pBuf->high_humidity_threshold_value = 70;                                            \
    pBuf->to_report_low_humidity_threshold = TRUE;                                       \
    pBuf->to_report_high_humidity_threshold = TRUE;                                      \
    pBuf->low_co2_threshold_value = 900;     /* 1000 to 1300 PPM nomal */                \
    pBuf->high_co2_threshold_value = 1400;                                               \
    pBuf->to_report_low_co2_threshold = TRUE;                                            \
    pBuf->to_report_high_co2_threshold = TRUE;                                           \
    pBuf->to_report_battery_fault = TRUE;                                                \
    pBuf->to_report_ac_power_fault = TRUE;                                               \
    pBuf->to_report_emr_fuse_fault = TRUE;                                               \
    pBuf->to_report_rs485_fuse_fault = TRUE;                                             \
    pBuf->to_report_sensor_fault = TRUE;                                                 \
    pBuf->is_light_restrike_timer_present = TRUE;                                        \
    pBuf->light_restrike_timer_duration  = 20;                                           \
    pBuf->DevHoodVent_FinishDelay.HH = 1;                                                \
    pBuf->DevHoodVent_FinishDelay.MM = 0;                                                \
    pBuf->DevHoodVent_FinishDelay.SS = 0;                                                \
    pBuf->backup_battery_voltage = 3.6;                                                  \
    pBuf->backup_battery_low_threshold_voltage = 2.6;                                    \
    pBuf->backup_battery_high_threshold_voltage = 2.8;                                   \   
    pBuf->co2_vent_delay_time.HH = 0;                                                    \
    pBuf->co2_vent_delay_time.MM = 0;                                                    \
    pBuf->co2_vent_delay_time.SS = 10;                                                   \
    pBuf->sensor_height = 1;                                                             \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(GrowConfig_GrowCycleInfo), BUFFER); \
} while(0)

              /* --------------  */


#define CONFIG_DEV_LIGHT_INFO_INIT(STORAGE,BUFFER, TAG)									\
do{																						\
	GrowConfig_DevLightInfo * pBuf = (GrowConfig_DevLightInfo *)BUFFER;			    	\
	memset(BUFFER,0x00,sizeof(GrowConfig_DevLightInfo));                                \
    pBuf->DevState = USUAL;                                                             \
	pBuf->LightCycle = NORMAL;                                                          \
	pBuf->Light_OnTime.HH = 06;                                                         \
    pBuf->Light_OnTime.MM = 0;															\
    pBuf->Light_OnTime.SS = 0;															\
	pBuf->Light_OffTime.HH = 18;														\
    pBuf->Light_OffTime.MM = 0;															\
    pBuf->Light_OffTime.SS = 0;															\
	(STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(GrowConfig_DevLightInfo), BUFFER);	\
}while(0)

              /* --------------  */

#define CONFIG_DEV_HOOD_VENT_INFO_INIT(STORAGE,BUFFER, TAG)                              \
do {                                                                                     \
	GrowConfig_DevGenInfo * pBuf = (GrowConfig_DevGenInfo *)BUFFER;                      \
	memset(BUFFER,0x00,sizeof(GrowConfig_DevGenInfo));                                   \
    pBuf->DevState = USUAL;                                                              \
    pBuf->DevOpState = NATIVE;                                                           \
    pBuf->DevOpMode = RPTTIMER;                                                          \
    pBuf->ToFollowDevice = DEVICE_LIGHT;                                                 \
    pBuf->lc_start_delay.HH = 1;                                                             \
    pBuf->lc_start_delay.MM = 0;                                                             \
    pBuf->lc_start_delay.SS = 0;                                                             \
    pBuf->dc_start_delay.HH = 1;                                                             \
    pBuf->dc_start_delay.MM = 0;                                                             \
    pBuf->dc_start_delay.SS = 0;                                                             \
    pBuf->is_lc_on = TRUE;                                                   \
    pBuf->is_dc_on = TRUE;                                                   \
    pBuf->lc_on_time.HH = 2;                                                                 \
    pBuf->lc_on_time.MM = 0;                                                                 \
    pBuf->lc_on_time.SS = 0;                                                                 \
    pBuf->dc_on_time.HH = 2;                                                                 \
    pBuf->dc_on_time.MM = 0;                                                                 \
    pBuf->dc_on_time.SS = 0;                                                                 \
    pBuf->lc_off_time.HH = 2;                                                                 \
    pBuf->lc_off_time.MM = 0;                                                                 \
    pBuf->lc_off_time.SS = 0;                                                                 \
    pBuf->dc_off_time.HH = 2;                                                                 \
    pBuf->dc_off_time.MM = 0;                                                                 \
    pBuf->dc_off_time.SS = 0;                                                                 \
    pBuf->lc_upper_threshold = 0;                                                           \
    pBuf->lc_lower_threshold = 0;                                                           \
    pBuf->dc_upper_threshold = 0;                                                           \
    pBuf->dc_lower_threshold = 0;                                                           \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(GrowConfig_DevGenInfo), BUFFER);    \
}while(0)

              /* --------------  */

#define CONFIG_DEV_CIRC_INFO_INIT(STORAGE,BUFFER, TAG)                                    \
do {                                                                                     \
    GrowConfig_DevGenInfo * pBuf = (GrowConfig_DevGenInfo *)BUFFER;                      \
    memset(BUFFER,0x00,sizeof(GrowConfig_DevGenInfo));                                   \
    pBuf->DevState = USUAL;                                                              \
    pBuf->DevOpState = NATIVE;                                                           \
    pBuf->DevOpMode = RPTTIMER;                                                          \
    pBuf->ToFollowDevice = DEVICE_LIGHT;                                                 \
    pBuf->lc_start_delay.HH = 1;                                                             \
    pBuf->lc_start_delay.MM = 0;                                                             \
    pBuf->lc_start_delay.SS = 0;                                                             \
    pBuf->dc_start_delay.HH = 1;                                                             \
    pBuf->dc_start_delay.MM = 0;                                                             \
    pBuf->dc_start_delay.SS = 0;                                                             \
    pBuf->is_lc_on = TRUE;                                                   \
    pBuf->is_dc_on = TRUE;                                                   \
    pBuf->lc_on_time.HH = 2;                                                                 \
    pBuf->lc_on_time.MM = 0;                                                                 \
    pBuf->lc_on_time.SS = 0;                                                                 \
    pBuf->dc_on_time.HH = 2;                                                                 \
    pBuf->dc_on_time.MM = 0;                                                                 \
    pBuf->dc_on_time.SS = 0;                                                                 \
    pBuf->lc_off_time.HH = 2;                                                                 \
    pBuf->lc_off_time.MM = 0;                                                                 \
    pBuf->lc_off_time.SS = 0;                                                                 \
    pBuf->dc_off_time.HH = 2;                                                                 \
    pBuf->dc_off_time.MM = 0;                                                                 \
    pBuf->dc_off_time.SS = 0;                                                                 \
    pBuf->lc_upper_threshold = 0;                                                           \
    pBuf->lc_lower_threshold = 0;                                                           \
    pBuf->dc_upper_threshold = 0;                                                           \
    pBuf->dc_lower_threshold = 0;                                                           \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(GrowConfig_DevGenInfo), BUFFER);    \
}while(0)

              /* --------------  */

#define CONFIG_DEV_VENT_INFO_INIT(STORAGE,BUFFER, TAG)                                   \
do {                                                                                      \
    GrowConfig_DevVentInfo * pBuf = (GrowConfig_DevVentInfo *)BUFFER;                     \
    memset(BUFFER,0x00,sizeof(GrowConfig_DevVentInfo));                                   \
    pBuf->DevState = USUAL;                                                               \
    pBuf->is_rpttimer_enabled_in_lc = TRUE;                                                \
    pBuf->lc_start_delay.HH = 1;                                                              \
    pBuf->lc_start_delay.MM = 0;                                                              \
    pBuf->lc_start_delay.SS = 0;                                                              \
    pBuf->lc_on_time.HH = 1;                                                          \
    pBuf->lc_on_time.MM = 0;                                                          \
    pBuf->lc_on_time.SS = 0;                                                          \
    pBuf->lc_off_time.HH = 1;                                                          \
    pBuf->lc_off_time.MM = 0;                                                          \
    pBuf->lc_off_time.SS = 0;                                                          \
    pBuf->is_temp_check_needed_in_lc = TRUE ;                                      \
    pBuf->lc_on_temp_threshold = 25;                                                        \
    pBuf->lc_off_temp_threshold = 35;                                                        \
    pBuf->is_humidity_check_needed_in_lc = TRUE ;                                      \
    pBuf->lc_on_humidity_threshold = 50;                                                        \
    pBuf->lc_off_humidity_threshold = 60;                                                        \
    pBuf->is_rpttimer_enabled_in_dc = TRUE;                                                \
    pBuf->dc_start_delay.HH = 1;                                                              \
    pBuf->dc_start_delay.MM = 0;                                                              \
    pBuf->dc_start_delay.SS = 0;                                                              \
    pBuf->dc_on_time.HH = 1;                                                          \
    pBuf->dc_on_time.MM = 0;                                                          \
    pBuf->dc_on_time.SS = 0;                                                          \
    pBuf->dc_off_time.HH = 1;                                                          \
    pBuf->dc_off_time.MM = 0;                                                          \
    pBuf->dc_off_time.SS = 0;                                                          \
    pBuf->is_temp_check_needed_in_dc = TRUE ;                                      \
    pBuf->dc_on_temp_threshold = 25;                                                        \
    pBuf->dc_off_temp_threshold = 35;                                                        \
    pBuf->is_humidity_check_needed_in_dc = TRUE ;                                      \
    pBuf->dc_on_humidity_threshold = 50;                                                        \
    pBuf->dc_off_humidity_threshold = 60;                                                        \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(GrowConfig_DevVentInfo), BUFFER);    \
}while(0)


              /* --------------  */

#define CONFIG_DEV_FXP1_INFO_INIT(STORAGE, BUFFER, TAG)                                  \
do {                                                                                     \
    GrowConfig_DevGenInfo * pBuf = (GrowConfig_DevGenInfo *)BUFFER;                      \
    memset(BUFFER,0x00,sizeof(GrowConfig_DevGenInfo));                                   \
    pBuf->DevState = USUAL;                                                              \
    pBuf->DevOpState = FLEXI;                                                           \
    pBuf->DevOpMode = RPTTIMER;                                                          \
    pBuf->ToFollowDevice = DEVICE_LIGHT;                                                 \
    pBuf->lc_start_delay.HH = 1;                                                             \
    pBuf->lc_start_delay.MM = 0;                                                             \
    pBuf->lc_start_delay.SS = 0;                                                             \
    pBuf->dc_start_delay.HH = 1;                                                             \
    pBuf->dc_start_delay.MM = 0;                                                             \
    pBuf->dc_start_delay.SS = 0;                                                             \
    pBuf->is_lc_on = TRUE; \
    pBuf->is_dc_on = TRUE;                                                    \
    pBuf->lc_on_time.HH = 2;                                                                 \
    pBuf->lc_on_time.MM = 0;                                                                 \
    pBuf->lc_on_time.SS = 0;                                                                 \
    pBuf->dc_on_time.HH = 2;                                                                 \
    pBuf->dc_on_time.MM = 0;                                                                 \
    pBuf->dc_on_time.SS = 0;                                                                 \
    pBuf->lc_off_time.HH = 2;                                                                 \
    pBuf->lc_off_time.MM = 0;                                                                 \
    pBuf->lc_off_time.SS = 0;                                                                 \
    pBuf->dc_off_time.HH = 2;                                                                 \
    pBuf->dc_off_time.MM = 0;                                                                 \
    pBuf->dc_off_time.SS = 0;                                                                 \
    pBuf->lc_upper_threshold = 0;                                                           \
    pBuf->lc_lower_threshold = 0;                                                           \
    pBuf->dc_upper_threshold = 0;                                                           \
    pBuf->dc_lower_threshold = 0;                                                           \
    pBuf->is_vent_lockout = 1;                                                              \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(GrowConfig_DevGenInfo), BUFFER);    \
}while(0)


              /* --------------  */

#define CONFIG_DEV_FXP2_INFO_INIT(STORAGE, BUFFER, TAG)                                  \
do {                                                                                     \
    GrowConfig_DevGenInfo * pBuf = (GrowConfig_DevGenInfo *)BUFFER;                      \
    memset(BUFFER,0x00,sizeof(GrowConfig_DevGenInfo));                                   \
    pBuf->DevState = USUAL;                                                              \
    pBuf->DevOpState = FLIP;                                                           \
    pBuf->DevOpMode = HEATER;                                                          \
    pBuf->ToFollowDevice = DEVICE_LIGHT;                                                 \
    pBuf->lc_start_delay.HH = 1;                                                             \
    pBuf->lc_start_delay.MM = 0;                                                             \
    pBuf->lc_start_delay.SS = 0;                                                             \
    pBuf->dc_start_delay.HH = 1;                                                             \
    pBuf->dc_start_delay.MM = 0;                                                             \
    pBuf->dc_start_delay.SS = 0;                                                             \
    pBuf->is_lc_on = TRUE;                                                   \
    pBuf->is_dc_on = TRUE;                                                    \
    pBuf->lc_on_time.HH = 4;                                                                 \
    pBuf->lc_on_time.MM = 0;                                                                 \
    pBuf->lc_on_time.SS = 0;                                                                 \
    pBuf->dc_on_time.HH = 4;                                                                 \
    pBuf->dc_on_time.MM = 0;                                                                 \
    pBuf->dc_on_time.SS = 0;                                                                 \
    pBuf->lc_off_time.HH = 4;                                                                 \
    pBuf->lc_off_time.MM = 0;                                                                 \
    pBuf->lc_off_time.SS = 0;                                                                 \
    pBuf->dc_off_time.HH = 4;                                                                 \
    pBuf->dc_off_time.MM = 0;                                                                 \
    pBuf->dc_off_time.SS = 0;                                                                 \
    pBuf->lc_upper_threshold = 35;                                                           \
    pBuf->lc_lower_threshold = 25;                                                           \
    pBuf->dc_upper_threshold = 35;                                                           \
    pBuf->dc_lower_threshold = 25;                                                           \
    (STORAGE)->pwrite(&(STORAGE)->media, TAG, sizeof(GrowConfig_DevGenInfo), BUFFER);    \
}while(0)

              /* --------------  */




#define CONFIG_FACTORY_PROFILE_SET_START_ADDRESS 	    MEMORY_FACTORY_PROFILE_START_ADDRESS

#define CONFIG_USER_PROFILE_SET_START_ADDRESS 			MEMORY_USER_PROFILE_START_ADDRESS

#ifdef NUCLEO_F439ZI_EVK_BOARD
#define CONFIG_PROFILE_0_MAX_SIZE   				(1024*6)		

#define CONFIG_PROFILE_MAX_SIZE    					(1024*6)
#endif

#ifdef ESP32_S2_MINI
#define CONFIG_PROFILE_0_MAX_SIZE                   (1024*8)        

#define CONFIG_PROFILE_MAX_SIZE                     (1024*8)
  
#endif


#define CONFIG_FACTORY_PROFILE_SET_INDEX  			0

#define CONFIG_USER_PROFILE_SET_INDEX  			1

#define CONFIG_PROFILE_MAX_TAG_LENGTH  				1024

#define CONFIG_PROFILE_SET_MAX_NO  					2

#define CONFIG_USER_PROFILE_MAX_NO  20   // No of possible user profiles as the user config space 128KB

typedef struct CONFIG
{ 
    U32 reserved;

} CONFIG;


typedef struct Config_Profile
{
    GrowConfig_GrowCycleInfo   GrowInfo;

    GrowConfig_DevLightInfo  DevLightConfig;
    GrowConfig_DevGenInfo    DevHoodVentConfig;
    GrowConfig_DevGenInfo    DevCircConfig;
    GrowConfig_DevVentInfo   DevVentConfig;
    GrowConfig_DevGenInfo    DevFxp1Config;
    GrowConfig_DevGenInfo    DevFxp2Config;

}Config_Profile;


typedef enum CONFIG_PROFILE_TYPE
{
    CONFIG_FACTORY_PROFILE,
    CONFIG_USER_PROFILE
}CONFIG_PROFILE_TYPE;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void CONFIG_Init(CONFIG *Config);

void CONFIG_WriteProfile(Config_Profile* pProf, U32 profile_no );

U32 CONFIG_WriteTag(U32 profile_no, U32 Tag, U8* pBuf ,U32 len );

BOOLEAN  CONFIG_ReadProfile(U32 profile_no , CONFIG_PROFILE_TYPE prof_type, Config_Profile* pProf);

void CONFIG_ReadTag(U32 profile_no, CONFIG_PROFILE_TYPE prof_type, U32 Tag, U8* pBuf );

BOOLEAN CONFIG_IsUserProvisonPresent();

void CONFIG_EraseUserProfile();

void CONFIG_EraseIndividualUserProfile(U32 profile_no);



#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
