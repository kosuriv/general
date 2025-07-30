/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : mqtt_com_esp.c
*
*  Description         : This file contains MQTT functionality for both
*                        subscription & publishing for ESP32s2 platfom.  
*
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  24th Sep 2021     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/

/* freeRTOS file */
#include "FreeRTOS.h"
#include "event_groups.h"

/* ESP32 S2 specific */
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

/* EC specific */
#include "mqtt_com.h"
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
#define MQTT_COM_ESP_PUBLISH_SUCCESS_BIT      BIT0
#define MQTT_COM_ESP_PUBLISH_FAILURE_BIT      BIT1
#define MQTT_COM_ESP_PUBLISH_DATA_BIT         BIT2

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void mqtt_com_esp_connect(MQTT_COM*   pMqttCom, void* p1);

static void mqtt_com_esp_event_handler(void *handler_args, 
                                       esp_event_base_t base,
                                       int32_t event_id,
                                       void *event_data);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static MQTT_COM *pMqttCom;

static esp_mqtt_client_handle_t mqtt_com_esp_client;

static EventGroupHandle_t mqtt_com_esp_publish_event_group; /* FreeRTOS event group*/

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
void MQTT_COM_Init(MQTT_COM*       pThis,
                   U8*             mqtt_client_id,
                   U8*             mqtt_host,
                   U32             mqtt_port,
                   U8*             topic, 
                   enum QoS        qos,
                   CALLBACK32_FCT  pFunction,
                   void            *pArg,
                   MQTT_COM_Security_type security,
                   void*            p1)
{

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    pMqttCom = pThis;

    memset(pMqttCom, 0x00, sizeof(MQTT_COM));

    strcpy (pMqttCom->client_id,mqtt_client_id);

    strcpy (pMqttCom->host,mqtt_host);
    pMqttCom->port = mqtt_port ;
    pMqttCom->security = security ;
    printf("mqtt host = %s\nport =%d\n",pMqttCom->host,pMqttCom->port);

    strcpy (pMqttCom->topic,topic);

    pMqttCom->qos = qos;

    pMqttCom->rx_callback.pFunction = pFunction;
    pMqttCom->rx_callback.pArg= pArg;

    mqtt_com_esp_publish_event_group = xEventGroupCreate();

    TRACE_INFO("%s Before Calling mqtt RX thread function \n", __FUNCTION__);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    mqtt_com_esp_connect(pMqttCom, p1);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : It is always called after MQTT Initialization
*******************************************************************************/
BOOLEAN MQTT_COM_SendMessage(U8* Msg, U32 Msglength)
{
    int msg_id ;

    EventBits_t bits;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    if(pMqttCom->is_connected)
    {
    	/* Send MQTT data  */
        msg_id = esp_mqtt_client_publish(mqtt_com_esp_client, pMqttCom->topic, Msg, Msglength,  pMqttCom->qos, 0);

        TRACE_INFO("%s MSG ID = %d  \n", __FUNCTION__, msg_id);

        if( msg_id == -1)
        {
            TRACE_WARN("%s() MQTT MSG ID ERROR NOT PROPOER  \n", __FUNCTION__);
            return FALSE;
        }

        xEventGroupSetBits(mqtt_com_esp_publish_event_group, MQTT_COM_ESP_PUBLISH_DATA_BIT);

        bits = xEventGroupWaitBits(mqtt_com_esp_publish_event_group,
                                   MQTT_COM_ESP_PUBLISH_SUCCESS_BIT | MQTT_COM_ESP_PUBLISH_FAILURE_BIT,
                                   pdFALSE,
                                   pdFALSE,
                                   portMAX_DELAY);

        TRACE_DBG("%s() After Event Group Receive (%d) MSG ID = %d   \n", __FUNCTION__,bits,msg_id);

        xEventGroupClearBits(mqtt_com_esp_publish_event_group,MQTT_COM_ESP_PUBLISH_SUCCESS_BIT | MQTT_COM_ESP_PUBLISH_FAILURE_BIT | MQTT_COM_ESP_PUBLISH_DATA_BIT);

        if( ( bits & MQTT_COM_ESP_PUBLISH_SUCCESS_BIT ) != 0 )
        {

            //xEventGroupClearBits(mqtt_com_esp_publish_event_group,MQTT_COM_ESP_PUBLISH_SUCCESS_BIT);
            TRACE_INFO("%s() MQTT Data Sent SUCCESS \n", __FUNCTION__);
            return TRUE;
        }
        else if( ( bits & MQTT_COM_ESP_PUBLISH_FAILURE_BIT ) != 0 )
        {
            //xEventGroupClearBits(mqtt_com_esp_publish_event_group,MQTT_COM_ESP_PUBLISH_FAILURE_BIT);
            TRACE_WARN("%s() MQTT Data Sent  FAIL  \n", __FUNCTION__);
            return FALSE;
        } 
    }
    else
    {
        TRACE_WARN("%s() MQTT Connection not Up . so DROPPED \n", __FUNCTION__);
        return FALSE;
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
static void mqtt_com_esp_connect(MQTT_COM*   pMqttCom, void* p1)
{
    esp_mqtt_client_config_t mqtt_cfg;

    MQTT_COM_Security_t*     pMqtt_security;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    pMqtt_security = (MQTT_COM_Security_t*)p1;

    memset(&mqtt_cfg,0x00 , sizeof(esp_mqtt_client_config_t));

    mqtt_cfg.client_id =pMqttCom->client_id;

    mqtt_cfg.host = pMqttCom->host;

    mqtt_cfg.port = pMqttCom->port;

    mqtt_cfg.lwt_topic = pMqttCom->topic;

    mqtt_cfg.lwt_qos = pMqttCom->qos;

    mqtt_cfg.keepalive = 60;

    mqtt_cfg.user_context = pMqttCom;

    if(pMqttCom->security == MQTT_COM_NO_SECURITY)
    {
        mqtt_cfg.username = pMqtt_security->s.NoSsl.username;
        mqtt_cfg.password = pMqtt_security->s.NoSsl.password;
    }
    if(pMqttCom->security == MQTT_COM_SSL_SECURITY)
    {
        mqtt_cfg.transport = MQTT_TRANSPORT_OVER_SSL;
        mqtt_cfg.skip_cert_common_name_check = TRUE;

        if(pMqtt_security->s.Ssl.ca_len != 0x00)
        {
            mqtt_cfg.cert_pem = pMqtt_security->s.Ssl.ca;
            mqtt_cfg.cert_len = 0x00;
        }
        if(pMqtt_security->s.Ssl.client_len != 0x00)
        {
            mqtt_cfg.client_cert_pem = pMqtt_security->s.Ssl.client;
            mqtt_cfg.client_cert_len = 0x00;
        }
        if(pMqtt_security->s.Ssl.key_len != 0x00)
        {
            mqtt_cfg.client_key_pem = pMqtt_security->s.Ssl.key;
            mqtt_cfg.client_key_len = 0x00;
        }
    }
 
    mqtt_com_esp_client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(mqtt_com_esp_client, ESP_EVENT_ANY_ID, mqtt_com_esp_event_handler, mqtt_com_esp_client);

    esp_mqtt_client_start(mqtt_com_esp_client);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void mqtt_com_esp_event_handler(void *handler_args, 
                                       esp_event_base_t base,
                                       int32_t event_id,
                                       void *event_data)
{
    esp_mqtt_event_handle_t  event = (esp_mqtt_event_handle_t)event_data;

    esp_mqtt_client_handle_t client = event->client;

    int msg_id;

    MQTT_COM* pMqttCom = (MQTT_COM*) event->user_context;

    BUFFER8 pBuffer;

    static  U32 total_len;

    static U8*  pDataBuf;

    static U32  prev_len;

  //  UBaseType_t uxTaskPriority = uxTaskPriorityGet(NULL);

   // char* task_name = pcTaskGetName( NULL );

    EventBits_t bits ;

    //TRACE_DBG("%s() Entry  Task Priority =  (%d) Task Name  = %s \n",__FUNCTION__, uxTaskPriority,task_name);

    //TRACE_DBG("event_base=%s, event_id=%d\n",base, event_id);

    //TRACE_DBG("pMQTTcom=%p   event pMQTTcom = %p \n", pMqttCom,(MQTT_COM*)event->user_context);

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            TRACE_DBG("MQTT_EVENT_CONNECTED\n");
            pMqttCom->is_connected = TRUE;
            esp_mqtt_client_subscribe(client,  pMqttCom->topic, 1);
            break;

        case MQTT_EVENT_DISCONNECTED:
            TRACE_DBG( "MQTT_EVENT_DISCONNECTED\n");
            pMqttCom->is_connected = FALSE;
           // esp_mqtt_client_start(client);
            bits =  xEventGroupGetBits( mqtt_com_esp_publish_event_group );
            if( ( bits & MQTT_COM_ESP_PUBLISH_DATA_BIT ) != 0 )
            {
                xEventGroupSetBits(mqtt_com_esp_publish_event_group, MQTT_COM_ESP_PUBLISH_FAILURE_BIT);
                TRACE_INFO("%s() MQTT PUBLISH FAILED LIKELY  \n", __FUNCTION__);
                return TRUE;
            }
            break;

        case MQTT_EVENT_SUBSCRIBED:
            TRACE_DBG( "MQTT_EVENT_SUBSCRIBED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            TRACE_DBG("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d\n", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            TRACE_DBG("MQTT_EVENT_PUBLISHED, msg_id=%d\n", event->msg_id);
            xEventGroupSetBits(mqtt_com_esp_publish_event_group, MQTT_COM_ESP_PUBLISH_SUCCESS_BIT);
            break;
        case MQTT_EVENT_DATA:
            TRACE_DBG( "MQTT_EVENT_DATA\n");
#if 0
            printf("TOPIC_LEN = %d\n", event->topic_len);
            printf("DATA_LEN = %d\n", event->data_len);
            printf("CUR_OFFSET = %d\n",event->current_data_offset);
            printf("TOTAL_DATA_LEN = %d  ADDR = %p \n", event->total_data_len,event->data);
            //printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            //printf("DATA=%.*s\r\n", event->data_len, event->data);
#endif
            if( event->topic_len)
            {
                total_len = event->total_data_len;
                prev_len = event->data_len;  /* new data */
                pDataBuf = (U8*)malloc(event->total_data_len);
                memcpy(pDataBuf,event->data,event->data_len);
            }
            else
            {
                memcpy(&pDataBuf[prev_len],event->data,event->data_len);

                prev_len += event->data_len; 
            }
            if( prev_len == event->total_data_len)
            {
                TRACE_DBG( "COMPLETE MQTT PKT RECEIVED(%d) \n", event->total_data_len);
    
                /* execute call back */
                pBuffer.size = (int)event->total_data_len;

                pBuffer.pBuf = (char*)pDataBuf;

                pMqttCom->rx_callback.pFunction(pMqttCom->rx_callback.pArg, &pBuffer);

                free(pDataBuf);

                pDataBuf = NULL;
            }
            break;
        case MQTT_EVENT_ERROR:
            TRACE_DBG( "MQTT_EVENT_ERROR = %x \n",event->error_handle->error_type);
/*
MQTT_ERROR_TYPE_NONE = 0
MQTT_ERROR_TYPE_TCP_TRANSPORT
MQTT_ERROR_TYPE_CONNECTION_REFUSED  */

#if 1
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) 
            {
                TRACE_DBG("reported from esp-tls = %x\n", event->error_handle->esp_tls_last_esp_err);
                TRACE_DBG("reported from tls stack = %x\n", event->error_handle->esp_tls_stack_err);
                TRACE_DBG("captured as transport's socket errno = %x\n",  event->error_handle->esp_transport_sock_errno);
                TRACE_DBG( "Last errno string (%s)\n", strerror(event->error_handle->esp_transport_sock_errno));

            }
#endif
            if(pDataBuf)
            {
                free(pDataBuf);
                pDataBuf = NULL;
            }
            break;
        default:
            TRACE_DBG( "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

/*******************************************************************************
*                          End of File
*******************************************************************************/


