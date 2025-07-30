/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : mqtt_com.c
*
*  Description         : This file contains MQTT functionality for both
                         subscription & publishing
*
*  Change history      :
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  2nd Sep 2020      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <string.h>
#include <time.h>
#include <sys/time.h>



/* freeRTOS file */
#include "FreeRTOS.h"
#include "event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wpa2.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "esp_sntp.h"
#include "esp_system.h"

#include "esp_attr.h"
#include "esp_sleep.h"

#include "openssl/ssl.h"
#include "openssl/x509v3.h"


#include "nw_wifi_esp.h"
#include "trace.h"
#include "web_provision.h"



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
static void nw_esp_event_handler(void* arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void* event_data);

static void nw_esp_time_sync_notification_cb(struct timeval *tv);

static U32 nw_wifi_esp_start_tcp_client_connection(U8* ip_addr, U32 port,S32 *socket_id);

static U32 nw_wifi_esp_start_ssl_tcp_client_connection(U8* ip_addr, U32 port, SSL **ssl_id);

static S32  nw_wifi_esp_allot_socket_id();

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static NW_WIFI_ESP* pNwWifi;

static esp_netif_t* nw_esp_Intereface;

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/
    
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : p1 interpreted as passphrase for psk
               p1 interpreted as an object of NW_ESP_WIFI_peap_ttls_t structure for PEAP/TTLS
               p1 interpreted as an object of NW_ESP_WIFI_tls_t structure for TLS
*******************************************************************************/
BOOLEAN NW_WIFI_ESP_Init(NW_WIFI_ESP*    pThis,
                         U8*             pSSID,
                         U8*             pTimeZone,
                         NW_ESP_WIFI_security_t security,  
                         void*            p1)
{ 

    NW_ESP_WIFI_tls_t* pTls;
    NW_ESP_WIFI_peap_ttls_t* pPeapTtls;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); /* default wifi settings*/
    wifi_config_t sta_config;
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    U8 ip_addr[20];

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    S8 mac_addr[6]={0};
    esp_efuse_mac_get_default(mac_addr);

   TRACE_DBG( " DEVICE NAME  :EC_%x_%x_%x_%x_%x_%x\n",mac_addr[0],  mac_addr[1],mac_addr[2],mac_addr[3],  mac_addr[4],mac_addr[5]);
    pNwWifi = pThis;

    memset((U8*)pNwWifi,0x00,sizeof(NW_WIFI_ESP));

    strcpy(pNwWifi->ssid, pSSID);
    strcpy(pNwWifi->time_zone,pTimeZone);
    TRACE_DBG("SSID=%s\n",pNwWifi->ssid);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* creates LWIP task for TCP/IP functionality */
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* attach WIFI STA interface to LWIP  to create network interface */
    nw_esp_Intereface = esp_netif_create_default_wifi_sta();

    /* creates  Wi-Fi driver task and initialize the Wi-Fi driver */
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Start Wifi Connection  */
    memset(&sta_config,0x00,sizeof(wifi_config_t));
    strcpy(sta_config.sta.ssid, pNwWifi->ssid);

    /*PSK connection*/
    if(security == NW_ESP_WIFI_PSK)
    {
        strcpy(pNwWifi->pass_phrase, (U8*)p1);
        TRACE_DBG("PASS_PHRASE=%s\n",pNwWifi->pass_phrase);
        strcpy(sta_config.sta.password, pNwWifi->pass_phrase);
    }

    sntp_setoperatingmode(SNTP_OPMODE_POLL);

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &nw_esp_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &nw_esp_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config) );

    /*PEAP connection*/
    if(security == NW_ESP_WIFI_PEAP)
    {
        pPeapTtls = (NW_ESP_WIFI_peap_ttls_t*)p1;

        TRACE_DBG("USERNAME=%s\n",pPeapTtls->username);
        TRACE_DBG("PASSWORD=%s\n",pPeapTtls->password);
        TRACE_DBG("EAP_ID=%s\n",pPeapTtls->eap_id);

        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity(pPeapTtls->eap_id, strlen(pPeapTtls->eap_id)) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username(pPeapTtls->username, strlen(pPeapTtls->username)) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password(pPeapTtls->password, strlen(pPeapTtls->password)) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable());
    }

    /*TLS connection*/
    if(security == NW_ESP_WIFI_TLS)
    {
        pTls = (NW_ESP_WIFI_tls_t*) p1;  

        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity(pTls->eap_id, strlen(pTls->eap_id)) );

        TRACE_DBG("EAP_ID=%s\n",pTls->eap_id);

        /* validate server certificate */
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_ca_cert(pTls->ca, pTls->ca_len) );
   
        /* validate client certificate */
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_cert_key(pTls->client,pTls->client_len, pTls->key, pTls->key_len, pTls->key_password, strlen(pTls->key_password)) );

        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable());
    }

    /*TTLS connection*/
    if(security == NW_ESP_WIFI_TTLS)
    { 
        pPeapTtls = (NW_ESP_WIFI_peap_ttls_t*)p1;  

        TRACE_DBG("USERNAME=%s\n",pPeapTtls->username);
        TRACE_DBG("PASSWORD=%s\n",pPeapTtls->password);
        TRACE_DBG("EAP_ID=%s\n",pPeapTtls->eap_id);

        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_identity(pPeapTtls->eap_id, strlen(pPeapTtls->eap_id)) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username(pPeapTtls->username, strlen(pPeapTtls->username)) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password(pPeapTtls->password, strlen(pPeapTtls->password)) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_ttls_phase2_method(ESP_EAP_TTLS_PHASE2_EAP) );
        ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable());
    }

    ESP_ERROR_CHECK(esp_wifi_start() );
}


/*******************************************************************************
* Name       : 
* Description: It opens TCP client socket 
* Remarks    : 
*******************************************************************************/
int NW_WIFI_ESP_OpenSocket(NW_ESP_WIFI_socket_type_t type,
                           NW_ESP_WIFI_socket_app_t soc_app,
                           char* ip_addr, 
                           int port,
                           S32* socket_id)
{
    SSL *ssl;
    U32 socket_array_index;
    U32 status;

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    socket_array_index = nw_wifi_esp_allot_socket_id();

    if(socket_array_index < 0)
    {
        TRACE_WARN("No Free socket array element alloted = %d\n", socket_array_index);
        return -1;
    }

    switch(type)
    {
        case NW_WIFI_TCP_CLIENT:
            status = nw_wifi_esp_start_tcp_client_connection(ip_addr, port,socket_id);
            if(!status)
            {  
                TRACE_DBG("tcp client connected successfully...");    
                pNwWifi->Sockets[socket_array_index].socket_id = *socket_id;
            }
            break;

        case NW_WIFI_SSL_TCP_CLIENT:          
            status = nw_wifi_esp_start_ssl_tcp_client_connection(ip_addr, port,&ssl); 
            if(!status)
            {                
                TRACE_DBG("ssl tcp client connected successfully...");
                pNwWifi->Sockets[socket_array_index].ssl = (SSL*)ssl;
            }
            break;
    }
    if(!status)
    {
        *socket_id = socket_array_index;
        pNwWifi->Sockets[socket_array_index].is_socket_open = TRUE;
        pNwWifi->Sockets[socket_array_index].socket_type = type;
        pNwWifi->Sockets[socket_array_index].app_type = soc_app;
    }
    TRACE_DBG("%s() Exit \n", __FUNCTION__);

    return (status == 1) ? 0 : -1;
}


/*******************************************************************************
* Name       : 
* Description: It closes TCP client socket 
* Remarks    : 
*******************************************************************************/
int NW_WIFI_ESP_CloseSocket(S32 socket_index)
{
    U32 status;

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    switch(pNwWifi->Sockets[socket_index].socket_type)
    {
        case NW_WIFI_TCP_CLIENT:
            if(!close(pNwWifi->Sockets[socket_index].socket_id))
            {
                status = TRUE;
            }
            else
            {
                status = FALSE;
            }             
            break;

        case NW_WIFI_SSL_TCP_CLIENT:
            if(!close(pNwWifi->Sockets[socket_index].socket_id))
            {
                SSL_shutdown(pNwWifi->Sockets[socket_index].ssl);
                SSL_free(pNwWifi->Sockets[socket_index].ssl);
                status = TRUE;
            } 
            else
            {
                status = FALSE;
            }   
            break;
    }
    if(pNwWifi->Sockets[socket_index].is_socket_open)
    {
        pNwWifi->Sockets[socket_index].is_socket_open = FALSE;      
    }
    return (status == 1) ? 0 : -1;
}

/*******************************************************************************
* Name       : 
* Description: It sends the data over socket
* Remarks    : 
*******************************************************************************/
int NW_WIFI_ESP_SendSocket(S32 socket_index,char* buffer, int size, int timeout_ms)
{
    U32 status;
    switch(pNwWifi->Sockets[socket_index].socket_type)
    {
        case NW_WIFI_TCP_CLIENT:
            if(send(pNwWifi->Sockets[socket_index].socket_id, buffer, size, 0) > 0)
            {
                TRACE_DBG("Data sent successfully..\n"); 
                status = TRUE;
            }         
            else
            {
                status = FALSE;
            }                 
            break;

        case NW_WIFI_SSL_TCP_CLIENT:
            if(SSL_write(pNwWifi->Sockets[socket_index].ssl, buffer, size) > 0)
            {
                TRACE_DBG("Data sent successfully..\n"); 
                status = TRUE;
            }         
            else
            {
                status = FALSE;
            }        
            break;
    }
    return (status == 1) ? 0 : -1;
}


/*******************************************************************************
* Name       : 
* Description: It sends the data over TCP client socket 
* Remarks    : 
*******************************************************************************/
int NW_WIFI_ESP_ReceiveSocket(S32 socket_index, char* buffer, int size, int timeout)
{
    S32 length_received;

    switch(pNwWifi->Sockets[socket_index].socket_type)
    {
        case NW_WIFI_TCP_CLIENT:
            length_received = recv(pNwWifi->Sockets[socket_index].socket_id, buffer, size, 0);                 
            break;

        case NW_WIFI_SSL_TCP_CLIENT:
           
            length_received = SSL_read(pNwWifi->Sockets[socket_index].ssl, buffer, size);              
            break;
    }
    return length_received;
   
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void NW_WIFI_ESP_Term()
{
    TRACE_DBG("%s() Entry \n", __FUNCTION__);

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 NW_WIFI_ESP_GetTime(U8* time2)
{
    time_t now;

    struct tm timeinfo;

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    time(&now);
    
    localtime_r(&now, &timeinfo);

    printf("%d/%d/%d,%d:%d:%d\n",timeinfo.tm_year-100,timeinfo.tm_mon+1,timeinfo.tm_mday,timeinfo.tm_hour,timeinfo.tm_min,timeinfo.tm_sec);  

    printf("TIME  = %s\n",ctime(&now));

    strcpy(time2,ctime(&now));

    TRACE_DBG("%s() Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void NW_WIFI_ProcessRx()
{
    TRACE_DBG("%s() Entry \n", __FUNCTION__);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 NW_WIFI_ESP_GetLocalIpAddr(U8* ip_addr)
{
    esp_netif_ip_info_t ip_info = {0};

    struct in_addr iaddr = { 0 };

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    ESP_ERROR_CHECK(esp_netif_get_ip_info(nw_esp_Intereface, &ip_info)); 

    inet_addr_from_ip4addr(&iaddr, &ip_info.ip);;

    TRACE_DBG("IP ADDRESS = %s\n", inet_ntoa(iaddr));

    strcpy(ip_addr,inet_ntoa(iaddr));
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
S8 NW_WIFI_ESP_ApInit()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); /* default wifi settings*/
    wifi_config_t wifi_config;
    S8 ssid_buffer[22];
    S8 mac_addr[6]={0};

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

     /* creates LWIP task for TCP/IP functionality */
    ESP_ERROR_CHECK(esp_netif_init());

    esp_event_loop_create_default();
    nw_esp_Intereface = esp_netif_create_default_wifi_ap();

    /* creates  Wi-Fi driver task and initialize the Wi-Fi driver */
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
     
    esp_efuse_mac_get_default(mac_addr);

    sprintf(ssid_buffer,"EC_%x_%x_%x_%x_%x_%x", mac_addr[0]&255, mac_addr[1]&255, mac_addr[2]&255, mac_addr[3]&255, mac_addr[4]&255, mac_addr[5]&255);
    memset(&wifi_config,0x00,sizeof(wifi_config));

    strcpy(wifi_config.ap.ssid, ssid_buffer);
    wifi_config.ap.ssid_len= strlen(wifi_config.ap.ssid);
    wifi_config.ap.channel= 6;
    strcpy(wifi_config.ap.password, "adminpass");
    wifi_config.ap.max_connection= 1;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    TRACE_DBG("EC wifi AP mode is  completed. SSID:%s password:%s channel:%d",wifi_config.ap.ssid,wifi_config.ap.password,wifi_config.ap.channel);    

}

/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void nw_esp_event_handler(void* arg,
                                 esp_event_base_t event_base,
                                 int32_t event_id,
                                 void* event_data)
{
   // UBaseType_t uxTaskPriority = uxTaskPriorityGet(NULL);

   // char* task_name = pcTaskGetName( NULL );

   // TRACE_DBG("%s() Entry  Task Priority =  (%d) Task Name  = %s \n",__FUNCTION__, uxTaskPriority,task_name);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
    {
        TRACE_DBG("Received WIFI_EVENT_STA_START\n");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        TRACE_DBG("Received WIFI_EVENT_STA_DISCONNECTED, INITIATE RECONNECTION \n");
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        TRACE_DBG("Received IP_EVENT_STA_GOT_IP\n");

        /* set time zone */
      // setenv("TZ", "UTC+09:30", 1);

        TRACE_DBG("TIME ZONE = %s\n",pNwWifi->time_zone);

        setenv("TZ", pNwWifi->time_zone, 1);
        tzset();

        /* enable SNTP synchronisation */
 //      sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();

        /* register call back to get notified when SNTP synch completed */
        sntp_set_time_sync_notification_cb(nw_esp_time_sync_notification_cb);       
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void nw_esp_time_sync_notification_cb(struct timeval *tv)
{
    U8 time_str[50];

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    TRACE_DBG("SNTP SYNCHRONISATION COMPLETED\n");

    NW_WIFI_ESP_GetTime(time_str);

    printf("Time of Wifi Connection = %s \n", time_str);
}
  

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_start_tcp_client_connection(U8* ip_addr, U32 port,S32 *socket_id) 
{
    U32 status;
    struct sockaddr_in dest_addr;
   
    TRACE_DBG("%s  starting TCP client with %s, on  %d\n",__FUNCTION__,ip_addr,port);

    memset(&dest_addr, 0, sizeof(dest_addr));
      
    dest_addr.sin_addr.s_addr = inet_addr(ip_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    *socket_id =  socket(AF_INET, SOCK_STREAM, 0);
    
    if(*socket_id != -1)
    {    
        TRACE_DBG("Socket created, connecting to %s:%d\n", ip_addr, port);

        if(connect(*socket_id, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6)) != 0)
        {
            TRACE_DBG("Socket unable to connect\n");
            status =  FALSE;
        }
        else
        {
            status =  TRUE;
        }
    }
    else
    {
        status = FALSE;
    }
    return (status == 1) ? 0 : -1;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_start_ssl_tcp_client_connection(U8* ip_addr, U32 port, SSL **ssl) 
{
    
    U32 status;
    S32 socket_id;
    SSL_CTX *ctx;
    struct sockaddr_in dest_addr;  
    struct hostent *host;
 
    TRACE_DBG("%s  starting SSL TCP client with %s, on  %d\n",__FUNCTION__,ip_addr,port);

    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(&host, 0, sizeof(host));

    host = gethostbyname(ip_addr);
    dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
     
    socket_id =  socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_id != -1)
    {  
        TRACE_DBG("Socket created, connecting to %s:%d\n", ip_addr, port);
  
        if(connect(socket_id, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) 
        {
            TRACE_DBG("Socket unable to connect\n");
            status = FALSE;
        }
        else
        {
            TRACE_DBG("create SSL context .....\n");
    
            /* create new ssl context method */
            ctx = SSL_CTX_new(TLSv1_client_method());

            if(!ctx) 
            {
                TRACE_DBG("Failed to create SSL context\n");
                status = FALSE;
            }
            else
            {  
                TRACE_DBG("create SSL ......\n");

                /* create new ssl method using ssl context method */
                *ssl = SSL_new(ctx);    

                if (!*ssl) 
                {
                    TRACE_DBG("Failed to create SSL\n");
                    status = FALSE;
                }
                else
                {
                    /* set socket to ssl */
                    SSL_set_fd(*ssl, socket_id);

                    /* connect to the remote SSL server */
                    if (!SSL_connect(*ssl)) 
                    {
                        TRACE_DBG("Failed to connect ssl\n");
                        status = FALSE;
                    }
                    else
                    {
                        status = TRUE;
                    }
                }
             }
        }
    }
    else
    {
        TRACE_DBG("Unable to create socket\n");
        status = FALSE;
    }
    return (status == 1) ? 0 : -1;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static S32  nw_wifi_esp_allot_socket_id()
{
    S32 socket_id = -1;
    U32 i;

    for(i=0; i < NW_ESP_WIFI_MAX_SOCKETS; i++)
    {
        if(!pNwWifi->Sockets[i].is_socket_open)
        {
            socket_id = i;
            break;
        } 
    }
    return socket_id;
}
      
/*******************************************************************************
*                          End of File
*******************************************************************************/


