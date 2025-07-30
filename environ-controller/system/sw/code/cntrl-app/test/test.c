/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : test_esp_flash.c
*
*  Description         : This file tests ESP program FLASH for read/write
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  14th Sep 2021     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "esp_spi_flash.h"
#include "esp_flash.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "nvs_flash.h"
#include "openssl/ssl.h"


#include "lwip/sockets.h"

#include "trace.h"
#include "memory_layout.h"
#include "utility.h"
#include "config.h"
#include "nw_wifi_esp.h"
#include "mqtt_com.h"
#include "smtp.h"
#include "adc.h"
#include "web_provision.h"
#include "modbus_master.h"
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#include "rtc.h"

#include "gpio.h"
#include "spi.h"  
#include "sd_card.h"
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
static void test_app_rx_mqtt_message(void* pArg, BUFFER8* pBuffer );

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static U8 test_buf[5120];

static CONFIG  Config;

static NW_WIFI_ESP  NwEspDriver;

static MQTT_COM     Mqtt;

static SMTP_account_t  account;

static SMTP_email_t     email;
    
static Config_SmtpInfo  pEmailInfo;

static Config_MQTTInfo  MqttConfig;

static Config_WiFiInfo  WifiConfig;

static WEB_PROVISION  WebProvision;

static MQTT_COM_no_security_t  Insecure;

static RTC_T  Rtc;

static SPI Spi1;

static SD_CARD SdCard;
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
void TEST_ESP_FLASH_Read()
{
    U32 flash_size = 0x00 ;
    U32 i;
    U32 j = 0x00;

    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    flash_size = spi_flash_get_chip_size();

    TRACE_DBG("FLASH SIZE = %d   integer size = %d \n", flash_size, sizeof(U32));

    spi_flash_read(APP_START_ADDRESS, test_buf, sizeof(test_buf));

    UTILITY_PrintBuffer(test_buf,64, 0x0);

    for(i=0; i<5000; i++)
    {
        if((i %256) == 0x00 )
            j = 0x00;
        test_buf[i] = j;
        j++;
    }

    spi_flash_erase_sector(MEMORY_FACTORY_PROFILE_START_ADDRESS / 0x1000);

    spi_flash_erase_sector((MEMORY_FACTORY_PROFILE_START_ADDRESS / 0x1000) +1);

    spi_flash_write(MEMORY_FACTORY_PROFILE_START_ADDRESS, test_buf,5000);

    memset(test_buf,0x00,5000);

    spi_flash_read(MEMORY_FACTORY_PROFILE_START_ADDRESS, test_buf, sizeof(test_buf));

    UTILITY_PrintBuffer(test_buf,16, 0x0);

    spi_flash_read(MEMORY_FACTORY_PROFILE_START_ADDRESS+0x1000, test_buf, sizeof(test_buf));

    UTILITY_PrintBuffer(test_buf,16, 0x0);

    spi_flash_erase_sector(MEMORY_FACTORY_PROFILE_START_ADDRESS / 0x1000);

    spi_flash_read(MEMORY_FACTORY_PROFILE_START_ADDRESS, test_buf, sizeof(test_buf));

    UTILITY_PrintBuffer(test_buf,16, 0x0);

    spi_flash_read(MEMORY_FACTORY_PROFILE_START_ADDRESS+0x1000, test_buf, sizeof(test_buf));

    UTILITY_PrintBuffer(test_buf,16, 0x0);

    
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_CONFIG_Init()
{
    Config_WiFiInfo  WifiConfig;

    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    CONFIG_Init(&Config);
    
    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);

    TRACE_DBG("SSID = %s\n", WifiConfig.ssid);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_Wifi()
{
    Config_WiFiInfo  WifiConfig;

    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    CONFIG_Init(&Config);
    
    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);
     printf("ssid = %s\t pass = %s\n",WifiConfig.ssid,WifiConfig.passphrase);

     NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);
}



/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_Mqtt()
{
    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    CONFIG_Init(&Config);

    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));
    memset(&MqttConfig,0x00, sizeof(Config_MQTTInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);



    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_MQTT_INFO_TAG,(U8*)&MqttConfig);

    NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);

    strcpy(Insecure.username,MqttConfig.username);
    strcpy(Insecure.password,MqttConfig.password); 

    MQTT_COM_Init(&Mqtt,
                  "DeviceEsp",   //MQTT parameters need be read from configuration profile TBD
                  MqttConfig.host,
                  MqttConfig.port_no,
                  MqttConfig.topic,
                  MqttConfig.qos_type,
                  test_app_rx_mqtt_message, //call back function when it receives message
                  NULL,
                  MqttConfig.security,
                  &Insecure);

    while (!MQTT_COM_SendMessage("Test_Data", 9));

    MQTT_COM_SendMessage("Some_Data", 9);
 
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_TcpClient()
{
    Config_WiFiInfo  WifiConfig;
    S32 socket_id;
    S8 send_msg[23] = "Test message to server\n";
    U32 status;
    S8 buffer[30];
    
    CONFIG_Init(&Config);
    
    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);

     NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);

    NW_WIFI_ESP_OpenSocket(NW_WIFI_TCP_CLIENT,
                           NW_ESP_WIFI_SOCKET_RAW,
                           "192.168.1.138", 
                           8000,
                           &socket_id);
    NW_WIFI_ESP_SendSocket(socket_id, send_msg, sizeof(send_msg), 2000);

    NW_WIFI_ESP_ReceiveSocket(socket_id, buffer, sizeof(buffer), 2000);

    TRACE_DBG("message from server = %s\n",buffer);

    NW_WIFI_ESP_CloseSocket(socket_id);

 
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_SslTcpClient()
{
    Config_WiFiInfo  WifiConfig;

    S32 socket_id;
    S8 send_msg[23] = "Test message to client\n";
    S8 buffer[20];
    
    CONFIG_Init(&Config);
    
    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);

     NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);

    NW_WIFI_ESP_OpenSocket(NW_WIFI_SSL_TCP_CLIENT,
                           NW_ESP_WIFI_SOCKET_RAW,
                           "smtp.gmail.com", 
                           465,
                           &socket_id);

   NW_WIFI_ESP_SendSocket(socket_id,send_msg, 25, 2000);

    NW_WIFI_ESP_ReceiveSocket(socket_id, buffer, sizeof(buffer), 2000);

   TRACE_DBG("message from server = %s\n",buffer); 

    NW_WIFI_ESP_CloseSocket(socket_id);


}



/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_Email()
{
    S8 i;
    U8* addr;
    
    
   
    TRACE_DBG("%s() Entry\n",__FUNCTION__);
    
    memset(&pEmailInfo,0x00,sizeof(Config_SmtpInfo));
    memset(&WifiConfig,0x00,sizeof(Config_WiFiInfo));
    memset(&account,0x00,sizeof(SMTP_account_t));
    memset(&email,0x00,sizeof(SMTP_email_t));

    CONFIG_Init(&Config);

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_SMTP_INFO_TAG,(U8*)&pEmailInfo);

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);

     NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);

    vTaskDelay(10000 / portTICK_PERIOD_MS);

    printf("ssid = %s\npass = %s\n",WifiConfig.ssid,WifiConfig.passphrase);
    printf("email_address = %s\npassword = %s\nto_addresses = %s\n",pEmailInfo.email_address,pEmailInfo.password,pEmailInfo.to_addresses);
  
    account.email_address = pEmailInfo.email_address;
    account.password = pEmailInfo.password;
    account.host_name = pEmailInfo.smtp_host_name;
    account.port = pEmailInfo.smtp_port;

    email.to_addresses = pEmailInfo.to_addresses;

    email.subject = "Test mail\n";
    email.content = "Mail from linta\n";

    account.smtp_encryption =  pEmailInfo.smtp_encryption;

    SMTP_SendEmail( &account,&email );
  
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_Adc()
{
    U32 value;

    float voltage;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    value = ADC_ReadValuePolling(ADC_1, ADC_CHANNEL_BATTERY_VOLTAGE);

    voltage = (float)value * (0.403)  ;

    printf("AD Value:  %d Voltage: %0.2f mV\n",value ,voltage);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_WebProvision()
{
    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    CONFIG_Init(&Config);
 
    WEB_PROVISION_Init(&WebProvision,80);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_TlsWifi()
{
   
    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    CONFIG_Init(&Config);
    
    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);

    NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);

       
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
__NOINIT_ATTR U8 buffer[10]={'A','B','C','D'};
void TEST_NOINIT_DRAM()
{
   
    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    printf("buffer[0] = %c \n",buffer[0]);

    buffer[0] = 'M';
    esp_restart();
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_Mqtt_ssl()
{
    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    CONFIG_Init(&Config);

    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));
    memset(&MqttConfig,0x00, sizeof(Config_MQTTInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);



    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_MQTT_INFO_TAG,(U8*)&MqttConfig);

     NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);

    strcpy(Insecure.username,MqttConfig.username);
    strcpy(Insecure.password,MqttConfig.password); 

    MQTT_COM_Init(&Mqtt,
                  "DeviceEsp",   //MQTT parameters need be read from configuration profile TBD
                  MqttConfig.host,
                  MqttConfig.port_no,
                  MqttConfig.topic,
                  MqttConfig.qos_type,
                  test_app_rx_mqtt_message, //call back function when it receives message
                  NULL,
                  MqttConfig.security,
                  &Insecure);

    while (!MQTT_COM_SendMessage("Test_Data", 9));

    MQTT_COM_SendMessage("Some_Data", 9);
 
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_ModbusMaster()
{
    U16 height = 0x1;

    U16  sensor_value[3];

    TRACE_DBG("%s() Entry\n",__FUNCTION__);

  //  CONFIG_Init(&Config);

    esp_log_level_set("*", ESP_LOG_ERROR); 
#if 1
    MODBUS_Init();

  //  MODBUS_GetSetParam(0x2a ,FC_SET_SENSOR_HEIGHT , (void*)&height);
while(1)
{
   vTaskDelay(500);
   MODBUS_GetSetParam(0x2a ,FC_GET_SENSOR_DATA , (void*)sensor_value);
 //   MODBUS_Term();

#endif
   TRACE_DBG("sensor_value[0] = %x sensor_value[1] = %x sensor_value[2] = %x   \n",sensor_value[0] ,sensor_value[1],sensor_value[2] );

   TRACE_DBG("co2 = %d temp = %d humidity = %d   \n",sensor_value[0] ,sensor_value[1],sensor_value[2] );
}
    while(1);

}

static void test_profile_rtc_alarm_handler(void* Arg , U32 flag)
{
    ets_printf("%s Entry(%d) \n", __FUNCTION__,flag);
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_RtcTimer()
{

    Config_WiFiInfo  WifiConfig;

    mTIME time;

    mDATE date;

    U8 time_str[12];

    U8 date_str[12];


    TRACE_DBG("%s() Entry\n",__FUNCTION__);


    CONFIG_Init(&Config);
    RTC_Init(&Rtc);
    
    memset(&WifiConfig,0x00, sizeof(Config_WiFiInfo));

    CONFIG_ReadTag(0,CONFIG_FACTORY_PROFILE,CONFIG_WIFI_INFO_TAG,(U8*)&WifiConfig);
    printf("ssid = %s\t pass = %s\n",WifiConfig.ssid,WifiConfig.passphrase);

    NW_WIFI_ESP_Init(&NwEspDriver,
                      WifiConfig.ssid ,
                      WifiConfig.time_zone,
                      WifiConfig.security,  
                      WifiConfig.passphrase);

    vTaskDelay(1500);

    RTC_GetDateTime(&date, &time , date_str ,time_str);


    time.MM  +=1;


    RTC_StartAlarm(&time ,NULL, test_profile_rtc_alarm_handler, NULL);


    time.MM  +=1;



    RTC_StartAlarm(&time ,NULL, test_profile_rtc_alarm_handler, NULL);

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_sd_card()
{ 
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* Initialize SD card & file system */
#ifdef NUCLEO_F439ZI_EVK_BOARD
    SPI_Init(&Spi1,SPI1_HW_INDEX,SPI_MODE0);
#endif

#ifdef ESP32_S2_MINI
    SPI_Init(&Spi1,SPI2_HW_INDEX,SPI_MODE0);
#endif

    SPI_DMAInit(&Spi1);

    SD_CARD_Init(&SdCard,SPI_IFACE, &Spi1 );

    TRACE_DBG("Init SD Card COMPLETED\n");

    SD_CARD_create_file("Hello\r\n", 5, "root/test.txt");


    TRACE_DBG("FILE WRITTEN COMPLETELY\n");
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_Gpio()
{

    TRACE_INFO("%s Entry \n", __FUNCTION__);
gpio_reset_pin(GPIO_NUM_6);
    gpio_set_direction(GPIO_NUM_6, GPIO_MODE_OUTPUT);
gpio_reset_pin(GPIO_NUM_45);
gpio_reset_pin(GPIO_NUM_13);
gpio_reset_pin(GPIO_NUM_11);
gpio_reset_pin(GPIO_NUM_46);
gpio_reset_pin(GPIO_NUM_20);
gpio_reset_pin(GPIO_NUM_3);
  gpio_set_direction(GPIO_NUM_45, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_11, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_46, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_20, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_3, GPIO_MODE_OUTPUT);

gpio_set_level(GPIO_NUM_6, 0);

//TEST_sd_card();

    while(1)
    {
        TRACE_DBG("%s() SETTING \n",__FUNCTION__);
     //   gpio_set_level(GPIO_NUM_6, 0);

  gpio_set_level(GPIO_NUM_45, 1);
  gpio_set_level(GPIO_NUM_13, 1);
  gpio_set_level(GPIO_NUM_11, 1);
  gpio_set_level(GPIO_NUM_46, 1);
  gpio_set_level(GPIO_NUM_20, 1);
  gpio_set_level(GPIO_NUM_3, 1);

        vTaskDelay(1000);
   TRACE_DBG("%s() RESETTING \n",__FUNCTION__);
       // gpio_set_level(pin, 0);

  gpio_set_level(GPIO_NUM_45, 0);
  gpio_set_level(GPIO_NUM_13, 0);
  gpio_set_level(GPIO_NUM_11, 0);
  gpio_set_level(GPIO_NUM_46, 0);
  gpio_set_level(GPIO_NUM_20, 0);
  gpio_set_level(GPIO_NUM_3, 0);
 vTaskDelay(1000);
    }
}




/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_app_rx_mqtt_message(void* pArg, BUFFER8* pBuffer )
{
    TRACE_DBG("%s() Entry \n",__FUNCTION__);

   // TRACE_DBG("Msg Received(%d):- %s\n", (int)pBuffer->size, (char*)pBuffer->pBuf);

    printf("DATA=%.*s\r\n", pBuffer->size, pBuffer->pBuf);

    

    TRACE_INFO("%s Exit\n", __FUNCTION__);

}


/*******************************************************************************
*                          End of File
*******************************************************************************/
