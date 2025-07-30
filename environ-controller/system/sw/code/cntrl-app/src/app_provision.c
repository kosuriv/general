/*******************************************************************************
* (C) Copyright 2020;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : app_provision.c
*
*  Description         : It contains profile request set/get functionality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   29th oct 2020    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "stdlib.h"
#include "app_provision.h"
#include "app.h"
#include "trace.h"
#include "app_setup.h"
#include "app_profile.h"


/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define APP_PROVISION_MAX_TOKEN 100 

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static U32 app_fill_json_wifi_info(U8* pBuf,BOOLEAN user_profile);

static U32 app_fill_json_mqtt_info(U8* pBuf,BOOLEAN user_profile);

static U32 app_fill_json_device_info(U8* pBuf,BOOLEAN user_profile);

static U32 app_fill_json_grow_info(U8* pBuf,BOOLEAN user_profile);

static U32 app_fill_json_smtp_info(U8* pBuf,BOOLEAN user_profile);

static void app_update_json_wifi_info(U8*   pBuf, U32   len);

static void app_update_json_mqtt_info(U8*   pBuf,  U32   len);

static void app_update_json_device_info(U8*   pBuf,   U32   len);

static void app_update_json_grow_info(U8*   pBuf,  U32   len);

static void app_update_json_smtp_info(U8*   pBuf,  U32   len);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern APP App;

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_set_provision_info_request(U8*        pBuf,
                                               jsmntok_t* pTok,
                                               U32        token_count)
{
    U32 i;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* Get all the fields of all grow cycle info, device config & sensors config */
 	for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"wifi_info",APP_COMPARE_LENGTH("wifi_info")))
            {
                i++;              
				app_update_json_wifi_info((char *)(pBuf  + pTok[i].start), 
                                          pTok[i].end - pTok[i].start);
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"mqtt_info",APP_COMPARE_LENGTH("mqtt_info")))
            {
                i++; 
				app_update_json_mqtt_info((char *)(pBuf  + pTok[i].start), 
                                               pTok[i].end - pTok[i].start);
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"device_info",APP_COMPARE_LENGTH("device_info")))
            {
                i++; 
				app_update_json_device_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start);            
            }  
			else if (!memcmp((char*)(pBuf + pTok[i].start),"grow_info",APP_COMPARE_LENGTH("grow_info")))
            {
                i++; 
				app_update_json_grow_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start);            
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"smtp_info",APP_COMPARE_LENGTH("smtp_info")))
            {
                i++; 
                app_update_json_smtp_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start);            
            }
		}      
    }

 	/* user  provisioned  , just restart the system to tak enew user settings*/
 	App_Reset(RESET_USER_PROVISION);
  
    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_get_provision_info_request(U8*        pBuf,
                                               jsmntok_t* pTok,
                                               U32        token_count)
{
    U8* buf;

    U8* rsp;
   
    U32 i;

    BOOLEAN user_profile;
    U32 ca_len      = 0;
    U32 client_len  = 0;
    U32 key_len     = 0;
    U32 mqtt_ca_len      = 0;
    U32 mqtt_client_len  = 0;
    U32 mqtt_key_len     = 0;

    Config_WiFiInfo  WifiInfo;
    Config_WiFiInfo* pInfo;

    Config_MQTTInfo  MqttInfo;
    Config_MQTTInfo* pInfoMqtt;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    pInfo = &WifiInfo;
    pInfoMqtt = &MqttInfo;

 
    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"user_profile",APP_COMPARE_LENGTH("user_profile")))
            {
                i++;              
                user_profile = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);
            }
        }
    }

    if(user_profile)
    {
        CONFIG_ReadTag(0, CONFIG_USER_PROFILE, CONFIG_WIFI_INFO_TAG, pInfo );
    }
    else
    {
        CONFIG_ReadTag(0, CONFIG_FACTORY_PROFILE, CONFIG_WIFI_INFO_TAG, pInfo );       
    }
    if(pInfo->security == NW_ESP_WIFI_TLS)
    {    
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT, (U8*)&ca_len, 4);
       
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT,(U8*) &client_len, 4);
   
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT, (U8*)&key_len, 4);
    } 

    if(pInfoMqtt->security == MQTT_COM_SSL_SECURITY)
    {    
        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT, (U8*)&mqtt_ca_len, 4);
       
        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT,(U8*) &mqtt_client_len, 4);
   
        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT, (U8*)&mqtt_key_len, 4);
    } 
    
    if(mqtt_ca_len == 0xFFFFFFFF)
    {
        mqtt_ca_len = 0x00;
    }
    if(mqtt_client_len == 0xFFFFFFFF)
    {
        mqtt_client_len = 0x00;
    }
    if(mqtt_key_len == 0xFFFFFFFF)
    {
        mqtt_key_len = 0x00;
    }

    buf = (U8*)malloc(APP_MAX_SIZE_PKT + ca_len +client_len+key_len+mqtt_ca_len+mqtt_client_len+mqtt_key_len); // to save thread stack space , take it from heap
    TRACE_DBG("size =%d\n",APP_MAX_SIZE_PKT + ca_len +client_len+key_len+mqtt_ca_len+mqtt_client_len+mqtt_key_len);
    rsp = buf;
    

    /* fill common  json */
    rsp += App_fill_common_json_header(rsp, APP_MQTT_PROVISION_INFO);

    /* fill action json */
    rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",\"%s\":%d ", "action", "get","user_profile",user_profile);
    rsp +=sprintf((char *)rsp,"}"); 

    rsp +=app_fill_json_wifi_info(rsp,user_profile);
    rsp +=app_fill_json_mqtt_info(rsp,user_profile);
    rsp +=app_fill_json_device_info(rsp,user_profile);
    rsp +=app_fill_json_grow_info(rsp,user_profile);
    rsp +=app_fill_json_smtp_info(rsp,user_profile);

 
    rsp +=sprintf((char *)rsp,"]}}");

	//TRACE_DBG("Config Pkt(%d) = %s\n",rsp-buf,buf);
    
    if((rsp-buf) > (APP_MAX_SIZE_PKT+ca_len +client_len+key_len+mqtt_ca_len+mqtt_client_len+mqtt_key_len))
    {
        TRACE_FATAL("CONFIG PKT SIZE(%d) more than  APP_MAX_SIZE_PKT(%d)\n",rsp-buf,(APP_MAX_SIZE_PKT+ca_len +client_len+key_len+mqtt_ca_len+mqtt_client_len+mqtt_key_len));
    }
    else
    {
    	MQTT_COM_SendMessage(buf, rsp-buf);
    }
 
    free(buf);

    TRACE_INFO("%s Exit \n", __FUNCTION__);

}
                                           
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_wifi_info(U8* pBuf,BOOLEAN user_profile)
{
    U8* rsp ;

    Config_WiFiInfo  WifiInfo;

    Config_WiFiInfo* pInfo;

	U32 length = 0 ;
    pInfo = &WifiInfo;

    if(user_profile)
    {
        CONFIG_ReadTag(0, CONFIG_USER_PROFILE, CONFIG_WIFI_INFO_TAG, pInfo );
    }
    else
    {
        CONFIG_ReadTag(0, CONFIG_FACTORY_PROFILE, CONFIG_WIFI_INFO_TAG, pInfo );       
    }
    rsp = pBuf;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "wifi_info");

    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\"", "ssid",pInfo->ssid);

#ifdef NUCLEO_F439ZI_EVK_BOARD
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "time_zone",pInfo->time_zone);
#endif
#ifdef ESP32_S2_MINI
    rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "time_zone",pInfo->time_zone);
#endif
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "wifi_security",pInfo->security);

    if(pInfo->security == NW_ESP_WIFI_PSK)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "passphrase",pInfo->passphrase);
    }
    else
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "eap_identity",pInfo->eap_identity);
    }
    if(pInfo->security == NW_ESP_WIFI_PEAP || pInfo->security == NW_ESP_WIFI_TTLS)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "username",pInfo->username);    
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "password",pInfo->passphrase);
    }    
    if(pInfo->security == NW_ESP_WIFI_TLS)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "key_password",pInfo->passphrase);
    
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT, (U8*)&length, 4);
        rsp +=sprintf((char *)rsp,",\"%s\":", "ca_cert");
        *rsp++='"';
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT+4, rsp, length);
        rsp +=length;
        *rsp++='"';
   
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT, (U8*)&length, 4);
        rsp +=sprintf((char *)rsp,",\"%s\":", "client_cert");
        *rsp++='"';
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT+4, rsp, length);
        rsp +=length;
        *rsp++='"';

        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT, (U8*)&length, 4);
        rsp +=sprintf((char *)rsp,",\"%s\":", "key_cert");
        *rsp++='"';
        FLASH_Read(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT+4, rsp, length);
        rsp +=length;
        *rsp++='"';
    }

    rsp +=sprintf((char *)rsp,"}}");
    return rsp - pBuf;
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_mqtt_info(U8* pBuf, BOOLEAN user_profile)
{
    U8* rsp ;

    Config_MQTTInfo  MqttInfo;

    Config_MQTTInfo* pInfo;

    U32 length = 0 ;

    pInfo = &MqttInfo;

    if(user_profile)
    {
        CONFIG_ReadTag(0, CONFIG_USER_PROFILE, CONFIG_MQTT_INFO_TAG, pInfo );
    }
    else
    {
        CONFIG_ReadTag(0, CONFIG_FACTORY_PROFILE, CONFIG_MQTT_INFO_TAG, pInfo );       
    }


    rsp = pBuf;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "mqtt_info");

    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\"", "hostname",pInfo->host);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "mqtt_security",pInfo->security);
    if(pInfo->security == MQTT_COM_NO_SECURITY)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "username",pInfo->username);
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "password",pInfo->password);
    }
    if(pInfo->security == MQTT_COM_SSL_SECURITY)
    {    
        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT, (U8*)&length, 4);        
        if(length != 0xFFFFFFFF)
        {
            rsp +=sprintf((char *)rsp,",\"%s\":", "mqtt_ca_cert");
            *rsp++='"';
            FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT+4, rsp, length);
            rsp +=length;
            *rsp++='"';
        }

        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT, (U8*)&length, 4);
        if(length != 0xFFFFFFFF)
        {
            rsp +=sprintf((char *)rsp,",\"%s\":", "mqtt_client_cert");
            *rsp++='"';
            FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT+4, rsp, length);
            rsp +=length;
            *rsp++='"';
        }

        FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT, (U8*)&length, 4);
        if(length != 0xFFFFFFFF)
        {
            rsp +=sprintf((char *)rsp,",\"%s\":", "mqtt_key_cert");
            *rsp++='"';
            FLASH_Read(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT+4, rsp, length);
            rsp +=length;
            *rsp++='"';
        }
    }  
    rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "topic",pInfo->topic);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "portno",pInfo->port_no);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "qos_type",pInfo->qos_type);
    rsp +=sprintf((char *)rsp,"}}");


    return rsp - pBuf;
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_device_info(U8* pBuf,BOOLEAN user_profile)
{
    U8* rsp ;

    Config_DeviceInfo  DeviceInfo;

    Config_DeviceInfo* pInfo;

    pInfo = &DeviceInfo;

    if(user_profile)
    {
        CONFIG_ReadTag(0, CONFIG_USER_PROFILE, CONFIG_DEVICE_INFO_TAG, pInfo );
    }
    else
    {
        CONFIG_ReadTag(0, CONFIG_FACTORY_PROFILE, CONFIG_DEVICE_INFO_TAG, pInfo );       
    }


    rsp = pBuf;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "device_info");

    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\"", "dev_name",pInfo->device_name);
    rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "fname",pInfo->firm_name);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "dev_id",pInfo->device_id);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "ghouse_id",pInfo->greenhouse_id);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "app_id",pInfo->mobile_app_id);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "controller_type",pInfo->controller_type);
    rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "ver_info",APP_EC_PROTOCOL_VERSION);
    rsp +=sprintf((char *)rsp,"}}");


    return rsp - pBuf;
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_grow_info(U8* pBuf,BOOLEAN user_profile)
{
    U8* rsp ;

    GrowConfig_GrowCycleInfo  GrowInfo;

    GrowConfig_GrowCycleInfo* pInfo;

    pInfo = &GrowInfo;

    if(user_profile)
    {
        CONFIG_ReadTag(0, CONFIG_USER_PROFILE, CONFIG_GROW_CYCLE_INFO_TAG, pInfo );
    }
    else
    {
        CONFIG_ReadTag(0, CONFIG_FACTORY_PROFILE, CONFIG_GROW_CYCLE_INFO_TAG, pInfo );       
    }

    rsp = pBuf;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "grow_info");

    rsp +=sprintf((char *)rsp,"\"%s\":%d", "measure_unit",pInfo->measure_unit);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "clk",pInfo->clk);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "language",pInfo->language);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_lo_tmp",pInfo->to_report_low_temp_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "lo_tmp_chk_val",pInfo->low_temp_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_hi_tmp",pInfo->to_report_high_temp_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "hi_tmp_chk_val",pInfo->high_temp_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_critical_tmp",pInfo->to_report_critical_temp_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "critical_tmp_chk_val",pInfo->critical_temp_threshold_value);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_lo_hmdy",pInfo->to_report_low_humidity_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "lo_hmdy_chk_val",pInfo->low_humidity_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_hi_hmdy",pInfo->to_report_high_humidity_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "hi_hmdy_chk_val",pInfo->high_humidity_threshold_value);


    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_lo_co2",pInfo->to_report_low_co2_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "lo_co2_chk_val",pInfo->low_co2_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_hi_co2",pInfo->to_report_high_co2_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "hi_co2_chk_val",pInfo->high_co2_threshold_value);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_battery_flt",pInfo->to_report_battery_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_frequency_flt",pInfo->to_report_ac_power_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_emr_fuse_flt",pInfo->to_report_emr_fuse_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_sensor_flt",pInfo->to_report_sensor_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_rs485_fuse_flt",pInfo->to_report_rs485_fuse_fault);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "light_restrike_tmr",pInfo->is_light_restrike_timer_present);

    rsp +=sprintf((char *)rsp,",\"%s\":\"", "light_restrike_tmr_dur");
    rsp +=UTILITY_convert_time_duration_to_str(pInfo->light_restrike_timer_duration , rsp);

    rsp +=sprintf((char *)rsp,"\",\"%s\":\"%d:%d:%d\"","dev_hoodvent_nt_finish_delay",pInfo->DevHoodVent_FinishDelay.HH,pInfo->DevHoodVent_FinishDelay.MM,pInfo->DevHoodVent_FinishDelay.SS);
   
    rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","co2_pva_vent_delay_time",pInfo->co2_vent_delay_time.HH,pInfo->co2_vent_delay_time.MM,pInfo->co2_vent_delay_time.SS);
        
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "battery_voltage",pInfo->backup_battery_voltage);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "battery_trig_voltage",pInfo->backup_battery_low_threshold_voltage);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "battery_hyst_voltage",pInfo->backup_battery_high_threshold_voltage);
    rsp +=sprintf((char *)rsp,",\"%s\":%d",    "sensor_height",pInfo->sensor_height);
    rsp +=sprintf((char *)rsp,"}}");


    return rsp - pBuf;
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_smtp_info(U8* pBuf,BOOLEAN user_profile)
{
    U8* rsp ;

    Config_SmtpInfo  SmtpInfo;

    Config_SmtpInfo* pInfo;

    pInfo = &SmtpInfo;

    if(user_profile)
    {
        CONFIG_ReadTag(0, CONFIG_USER_PROFILE, CONFIG_SMTP_INFO_TAG, pInfo );
    }
    else
    {
        CONFIG_ReadTag(0, CONFIG_FACTORY_PROFILE, CONFIG_SMTP_INFO_TAG, pInfo );       
    }


    rsp = pBuf;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "smtp_info");

    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\"", "email_address",pInfo->email_address);
    rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "email_password",pInfo->password);
    rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "smtp_host_name",pInfo->smtp_host_name);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "smtp_port",pInfo->smtp_port);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "smtp_security",pInfo->smtp_encryption);
    rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "to_addresses",pInfo->to_addresses);
    if(strlen(pInfo->cc_addresses))
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "cc_addresses",pInfo->cc_addresses);
    }
    if(strlen(pInfo->signature))
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%s\"", "signature",pInfo->signature);
    }
    rsp +=sprintf((char *)rsp,"}}");


    return rsp - pBuf;
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_update_json_wifi_info(U8*   pBuf,  U32   len)
{
    jsmn_parser j;

    jsmntok_t tok[APP_PROVISION_MAX_TOKEN];

    S32 token_count; 
    U32 i;
    U8* p;
    U32 ca_len      = 0;
    U32 client_len  = 0;
    U32 key_len     = 0;

    Config_WiFiInfo  WifiInfo;
    Config_WiFiInfo* pInfo;


	/* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROVISION_MAX_TOKEN);

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    pInfo = &WifiInfo;

    memset(pInfo,0x00,sizeof(Config_WiFiInfo));

    pInfo->serial_baudrate = 9600;

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"ssid",APP_COMPARE_LENGTH("ssid")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->ssid,p);
            }  
            else if (!memcmp((char*)(pBuf + tok[i].start),"passphrase",APP_COMPARE_LENGTH("passphrase")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->passphrase,p);

            }           
			else if (!memcmp((char*)(pBuf + tok[i].start),"time_zone",APP_COMPARE_LENGTH("time_zone")))
            {
                i++;  
#ifdef NUCLEO_F439ZI_EVK_BOARD           
                pInfo->time_zone = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
#endif
#ifdef ESP32_S2_MINI
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->time_zone,p);
#endif
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"username",APP_COMPARE_LENGTH("username")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->username,p);
            }   
            else if (!memcmp((char*)(pBuf + tok[i].start),"wifi_security",APP_COMPARE_LENGTH("wifi_security")))
            {
                 i++;              
                pInfo->security = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }   
            else if (!memcmp((char*)(pBuf + tok[i].start),"eap_identity",APP_COMPARE_LENGTH("eap_identity")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->eap_identity,p);
            }           
            else if (!memcmp((char*)(pBuf + tok[i].start),"ca_cert",APP_COMPARE_LENGTH("ca_cert")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';

                ca_len = strlen(p)+1;
                *(p+(ca_len-1)) = '\0';

                FLASH_EraseSize(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT , 0x1000);

                FLASH_Write(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT, (U8*)&ca_len, 4);

                FLASH_Write(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT+4, p, ca_len);
            }   
            else if (!memcmp((char*)(pBuf + tok[i].start),"client_cert",APP_COMPARE_LENGTH("client_cert")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';

                client_len = strlen(p)+1;
                *(p+(client_len-1)) = '\0';

                FLASH_EraseSize(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT , 0x1000);

                FLASH_Write(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT, (U8*)&client_len, 4);

                FLASH_Write(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT+4, p, client_len);
            }   
            else if (!memcmp((char*)(pBuf + tok[i].start),"key_cert",APP_COMPARE_LENGTH("key_cert")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';

                key_len = strlen(p)+1;
                *(p+(key_len-1)) = '\0';

                FLASH_EraseSize(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT , 0x1000);

                FLASH_Write(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT, (U8*)&key_len, 4);

                FLASH_Write(&App.FlashDriver, MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT+4, p, key_len);
            }   
            if(pInfo->security == NW_ESP_WIFI_PEAP || pInfo->security == NW_ESP_WIFI_TTLS) 
            {
                if (!memcmp((char*)(pBuf + tok[i].start),"password",APP_COMPARE_LENGTH("password")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';
                    strcpy(pInfo->passphrase,p);
                }   
            }
            else if(pInfo->security == NW_ESP_WIFI_TLS) 
            {
                if (!memcmp((char*)(pBuf + tok[i].start),"key_password",APP_COMPARE_LENGTH("key_pswd")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';
                    strcpy(pInfo->passphrase,p);
                }   
            }
            else if(pInfo->security == NW_ESP_WIFI_PSK) 
            {
                if (!memcmp((char*)(pBuf + tok[i].start),"passphrase",APP_COMPARE_LENGTH("passphrase")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';
                    strcpy(pInfo->passphrase,p);
                }   
            }     
        }
    }

    CONFIG_WriteTag(0, CONFIG_WIFI_INFO_TAG,(U8*)pInfo ,sizeof(Config_WiFiInfo) );

    TRACE_INFO("%s Exit\n", __FUNCTION__);

    //  UTILITY_PrintBuffer(pBuf,len,1);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_update_json_mqtt_info(U8*   pBuf,  U32   len)
{
	jsmn_parser j;

    jsmntok_t tok[APP_PROVISION_MAX_TOKEN];

    S32 token_count; 

    U32 i;

    U8* p;

    U32 ca_len      = 0;
    U32 client_len  = 0;
    U32 key_len     = 0;
    U32 length     = 0;
    Config_MQTTInfo  MqttInfo;

    Config_MQTTInfo* pInfo;

    
    /* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROVISION_MAX_TOKEN);

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    pInfo = &MqttInfo;
    memset(pInfo,0x00,sizeof(Config_MQTTInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"hostname",APP_COMPARE_LENGTH("hostname")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->host,p);
            } 
            else if (!memcmp((char*)(pBuf + tok[i].start),"mqtt_security",APP_COMPARE_LENGTH("mqtt_security")))
            {
                 i++;              
                pInfo->security = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            } 
            else if (!memcmp((char*)(pBuf + tok[i].start),"topic",APP_COMPARE_LENGTH("topic")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->topic,p);

            }  
            else if (!memcmp((char*)(pBuf + tok[i].start),"portno",APP_COMPARE_LENGTH("portno")))
            {
                i++;              
                pInfo->port_no = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }   
            else if (!memcmp((char*)(pBuf + tok[i].start),"qos_type",APP_COMPARE_LENGTH("qos_type")))
            {
                i++;              
                pInfo->qos_type = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"username",APP_COMPARE_LENGTH("username")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';
                    strcpy(pInfo->username,p);
                }
                else if (!memcmp((char*)(pBuf + tok[i].start),"password",APP_COMPARE_LENGTH("password")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';
                    strcpy(pInfo->password,p);
                }  

            else if (!memcmp((char*)(pBuf + tok[i].start),"mqtt_ca_cert",APP_COMPARE_LENGTH("mqtt_ca_cert")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';

                    ca_len = strlen(p);
                    *(p+(ca_len+1)) = '\0';

                    if(ca_len != 0)
                    {
                        FLASH_EraseSize(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT , 0x1000);

                        FLASH_Write(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT, (U8*)&ca_len, 4);

                        FLASH_Write(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT+4, p, ca_len);
                    }       
                }   
                else if (!memcmp((char*)(pBuf + tok[i].start),"mqtt_client_cert",APP_COMPARE_LENGTH("mqtt_client_cert")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';

                    client_len = strlen(p);
                    *(p+(client_len+1)) = '\0';

                    if(client_len != 0)
                    {
                        FLASH_EraseSize(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT , 0x1000);

                        FLASH_Write(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT, (U8*)&client_len, 4);
     
                        FLASH_Write(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT+4, p, client_len);
                    }
                }   
                else if (!memcmp((char*)(pBuf + tok[i].start),"mqtt_key_cert",APP_COMPARE_LENGTH("mqtt_key_cert")))
                {
                    i++;
                    p = (char*)(pBuf + tok[i].start);
                    *(p + tok[i].end - tok[i].start) = '\0';

                    key_len = strlen(p);
                    *(p+(key_len+1)) = '\0';
                
                    if(key_len != 0)
                    {
                        FLASH_EraseSize(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT , 0x1000);

                        FLASH_Write(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT, (U8*)&key_len, 4);

                        FLASH_Write(&App.FlashDriver, MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT+4, p, key_len);
                    }
                }
        }
    }
    CONFIG_WriteTag(0, CONFIG_MQTT_INFO_TAG, pInfo ,sizeof(Config_MQTTInfo) );

    TRACE_INFO("%s Exit\n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_update_json_device_info(U8*   pBuf,  U32   len)
{
    jsmn_parser j;

    jsmntok_t tok[APP_PROVISION_MAX_TOKEN];

    S32 token_count; 

    U32 i;

    U8* p;

    Config_DeviceInfo  DeviceInfo;

    Config_DeviceInfo* pInfo;

    /* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROVISION_MAX_TOKEN);

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    pInfo = &DeviceInfo;
    memset(pInfo,0x00,sizeof(Config_DeviceInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"dev_name",APP_COMPARE_LENGTH("dev_name")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->device_name,p);
            }  
            else if (!memcmp((char*)(pBuf + tok[i].start),"fname",APP_COMPARE_LENGTH("fname")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->firm_name,p);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"dev_id",APP_COMPARE_LENGTH("dev_id")))
            {
                i++;              
                pInfo->device_id = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }   
            else if (!memcmp((char*)(pBuf + tok[i].start),"ghouse_id",APP_COMPARE_LENGTH("ghouse_id")))
            {
                i++;              
                pInfo->greenhouse_id = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"app_id",APP_COMPARE_LENGTH("app_id")))
            {
                i++;              
                pInfo->mobile_app_id = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"controller_type",APP_COMPARE_LENGTH("controller_type")))
            {
                i++;              
                pInfo->controller_type = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
        }
    }
    CONFIG_WriteTag(0, CONFIG_DEVICE_INFO_TAG, pInfo ,sizeof(Config_DeviceInfo));

    TRACE_INFO("%s Exit\n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_update_json_grow_info(U8*   pBuf,  U32   len)
{
    jsmn_parser j;

    jsmntok_t tok[APP_PROVISION_MAX_TOKEN];

    S32 token_count; 

    U32 i;

    U8* p;

	mTIME time;


    GrowConfig_GrowCycleInfo  GrowInfo;

    GrowConfig_GrowCycleInfo*  pInfo;

    /* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROVISION_MAX_TOKEN);

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    pInfo = &GrowInfo;
    memset(pInfo,0x00,sizeof(GrowConfig_GrowCycleInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
    
            if (!memcmp((char*)(pBuf + tok[i].start),"measure_unit",APP_COMPARE_LENGTH("measure_unit")))
            {
                i++;        
                pInfo->measure_unit = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"clk",APP_COMPARE_LENGTH("clk")))
            {
                i++;        
                pInfo->clk = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"language",APP_COMPARE_LENGTH("language")))
            {
                i++;              
                pInfo->language = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_lo_tmp",APP_COMPARE_LENGTH("rpt_lo_tmp")))
            {
                i++;              
                pInfo->to_report_low_temp_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"lo_tmp_chk_val",APP_COMPARE_LENGTH("lo_tmp_chk_val")))
            {
                i++;        
                pInfo->low_temp_threshold_value = strtof((char *)(pBuf  + tok[i].start),NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_hi_tmp",APP_COMPARE_LENGTH("rpt_hi_tmp")))
            {
                i++;              
                pInfo->to_report_high_temp_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"hi_tmp_chk_val",APP_COMPARE_LENGTH("hi_tmp_chk_val")))
            {
                i++;              
                pInfo->high_temp_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_critical_tmp",APP_COMPARE_LENGTH("rpt_critical_tmp")))
            {
                i++;              
                pInfo->to_report_critical_temp_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"critical_tmp_chk_val",APP_COMPARE_LENGTH("critical_tmp_chk_val")))
            {
                i++;              
                pInfo->critical_temp_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_lo_hmdy",APP_COMPARE_LENGTH("rpt_lo_hmdy")))
            {
                i++;              
                pInfo->to_report_low_humidity_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"lo_hmdy_chk_val",APP_COMPARE_LENGTH("lo_hmdy_chk_val")))
            {
                i++;              
                pInfo->low_humidity_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_hi_hmdy",APP_COMPARE_LENGTH("rpt_hi_hmdy")))
            {
                i++;              
                pInfo->to_report_high_humidity_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"hi_hmdy_chk_val",APP_COMPARE_LENGTH("hi_hmdy_chk_val")))
            {
                i++;              
                pInfo->high_humidity_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_lo_co2",APP_COMPARE_LENGTH("rpt_lo_co2")))
            {
                i++;              
                pInfo->to_report_low_co2_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"lo_co2_chk_val",APP_COMPARE_LENGTH("lo_co2_chk_val")))
            {
                i++;              
                pInfo->low_co2_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_hi_co2",APP_COMPARE_LENGTH("rpt_hi_co2")))
            {
                i++;              
                pInfo->to_report_high_co2_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"hi_co2_chk_val",APP_COMPARE_LENGTH("hi_co2_chk_val")))
            {
                i++;              
                pInfo->high_co2_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_battery_flt",APP_COMPARE_LENGTH("rpt_battery_flt")))
            {
                i++;              
                pInfo->to_report_battery_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_frequency_flt",APP_COMPARE_LENGTH("rpt_battery_flt")))
            {
                i++;              
                pInfo->to_report_ac_power_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_emr_fuse_flt",APP_COMPARE_LENGTH("rpt_emr_fuse_flt")))
            {
                i++;              
                pInfo->to_report_emr_fuse_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_sensor_flt",APP_COMPARE_LENGTH("rpt_sensor_flt")))
            {
                i++;              
                pInfo->to_report_sensor_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_rs485_fuse_flt",APP_COMPARE_LENGTH("rpt_485_fuse_flt")))
            {
                i++;              
                pInfo->to_report_rs485_fuse_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            } 
            else if (!memcmp((char*)(pBuf + tok[i].start),"light_restrike_tmr",APP_COMPARE_LENGTH("light_restrike_tmr")))
            {
                i++;              
                pInfo->is_light_restrike_timer_present = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"light_restrike_dur",APP_COMPARE_LENGTH("light_restrike_dur")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                UTILITY_convert_time_str(p, &time);
                pInfo->light_restrike_timer_duration = (time.HH*3600) + (time.MM*60) + time.SS;
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"dev_hoodvent_nt_finish_delay",APP_COMPARE_LENGTH("dev_hoodvent_nt_finish_delay")))
            {
                        i++;
                        p = (char*)(pBuf + tok[i].start);
                        *(p + tok[i].end - tok[i].start) = '\0';
                        UTILITY_convert_time_str(p, &pInfo->DevHoodVent_FinishDelay);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"battery_voltage",APP_COMPARE_LENGTH("battery_voltage")))
            {
                i++;              
                pInfo->backup_battery_voltage = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"battery_trig_voltage",APP_COMPARE_LENGTH("battery_trig_voltage")))
            {
                i++;              
                pInfo->backup_battery_low_threshold_voltage = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"battery_hyst_voltage",APP_COMPARE_LENGTH("battery_hyst_voltage")))
            {
                i++;              
                pInfo->backup_battery_high_threshold_voltage = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"co2_pva_vent_delay_time",APP_COMPARE_LENGTH("co2_pva_vent_delay_time")))
            {
                        i++;
                        p = (char*)(pBuf + tok[i].start);
                        *(p + tok[i].end - tok[i].start) = '\0';
                        UTILITY_convert_time_str(p, &pInfo->co2_vent_delay_time);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"sensor_height",APP_COMPARE_LENGTH("sensor_height")))
            {
                i++;              
                pInfo->sensor_height = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
        }
    }
    CONFIG_WriteTag(0, CONFIG_GROW_CYCLE_INFO_TAG, pInfo ,sizeof(GrowConfig_GrowCycleInfo));

    TRACE_INFO("%s Exit\n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_update_json_smtp_info(U8*   pBuf,  U32   len)
{
    jsmn_parser j;

    jsmntok_t tok[APP_PROVISION_MAX_TOKEN];

    S32 token_count; 

    U32 i;

    U8* p;

    Config_SmtpInfo  SmtpInfo;

    Config_SmtpInfo* pInfo;

    /* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROVISION_MAX_TOKEN);

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

    pInfo = &SmtpInfo;
    memset(pInfo,0x00,sizeof(Config_SmtpInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"email_address",APP_COMPARE_LENGTH("email_address")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->email_address,p);
            }  
            else if (!memcmp((char*)(pBuf + tok[i].start),"email_password",APP_COMPARE_LENGTH("email_password")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->password,p);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"smtp_host_name",APP_COMPARE_LENGTH("smtp_host_name")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->smtp_host_name,p);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"smtp_port",APP_COMPARE_LENGTH("smtp_port")))
            {
                i++;              
                pInfo->smtp_port = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }   
            else if (!memcmp((char*)(pBuf + tok[i].start),"smtp_security",APP_COMPARE_LENGTH("smtp_security")))
            {
                i++;              
                pInfo->smtp_encryption = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"to_addresses",APP_COMPARE_LENGTH("to_addresses")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->to_addresses,p);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"cc_addresses",APP_COMPARE_LENGTH("cc_addresses")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->cc_addresses,p);
            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"signature",APP_COMPARE_LENGTH("signature")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
                strcpy(pInfo->cc_addresses,p);
            }
        }
    }
    CONFIG_WriteTag(0, CONFIG_SMTP_INFO_TAG, pInfo ,sizeof(Config_SmtpInfo));

    TRACE_INFO("%s Exit\n", __FUNCTION__);
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
