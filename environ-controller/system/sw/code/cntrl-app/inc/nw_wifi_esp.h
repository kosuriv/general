/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : nw_wifi_esp.h
*
*  Description         : This file contains NW API functions using ESP serial SDK
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  28th Aug 2020     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef NW_WIFI_ESP_H
#define NW_WIFI_ESP_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "osa.h"

#ifndef LINUX
#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "uart.h"
#endif
#endif


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Macro Definitions
*******************************************************************************/
#define NW_ESP_WIFI_MAX_SOCKETS  2

#define NW_ESP_WIFI_MAX_NO_RX_MSGS  20

/*******************************************************************************
*                          Type Declarations
*******************************************************************************/

typedef enum 
{
    NW_ESP_WIFI_PSK,
    NW_ESP_WIFI_PEAP,
    NW_ESP_WIFI_TLS,
    NW_ESP_WIFI_TTLS
} NW_ESP_WIFI_security_t;


typedef enum 
{
    NW_ESP_WIFI_SOCKET_MQTT,
    NW_ESP_WIFI_SOCKET_EMAIL,
    NW_ESP_WIFI_SOCKET_RAW
} NW_ESP_WIFI_socket_app_t;


typedef enum 
{
    NW_WIFI_TCP_CLIENT,
    NW_WIFI_UDP_CLINET,
    NW_WIFI_SSL_TCP_CLIENT,
    NW_WIFI_TCP_SERVER,
    NW_WIFI_UDP_SERVER, 
    NW_WIFI_SSL_TCP_SERVER   
}NW_ESP_WIFI_socket_type_t;


typedef struct 
{
    osa_msgq_type_t  pRxSocDataQ;

    BOOLEAN is_socket_open;

    NW_ESP_WIFI_socket_type_t  socket_type;

    NW_ESP_WIFI_socket_app_t   app_type;
	void* ssl;

    S32 socket_id;
    
}NW_ESP_WIFI_socket_t;


typedef struct 
{
    U8* ca;
    U32 ca_len;

    U8* key;
    U32 key_len;

    U8* client;
    U32 client_len;

    U8  eap_id[32];

    U8 key_password[32];
    
}NW_ESP_WIFI_tls_t;

typedef struct 
{
    U8 username[32];

    U8 password[32];

    U8 eap_id[32];
    
}NW_ESP_WIFI_peap_ttls_t;

typedef struct NW_WIFI_ESP
{ 

    /* serial port parameters */
    U32 baud_rate;

#ifdef LINUX
    U8 serial_port[10];
#endif
#ifdef NUCLEO_F439ZI_EVK_BOARD
    UART* pSerialDriver;
    osa_semaphore_type_t uart_rx_sem;
    S32 time_zone;  /* -11 to 13, for e.g. UTC it is 0 */
#endif 


    /* Wifi network parameters */
    U8 ssid[32];
    U8 pass_phrase[32];

#ifdef ESP32_S2_MINI
    U8 time_zone[24];
#endif

    osa_thread_handle rx_thread; 

    osa_mutex_type_t tx_mutex;

    osa_semaphore_type_t data_status_sem;
        
    osa_msgq_type_t pRxCmdQ;
   //osa_msgq_type_t pRxDataQ;



    osa_mutex_type_t call_mutex;

    osa_msgq_type_t pRxMsgQ;

    NW_ESP_WIFI_socket_t  Sockets[NW_ESP_WIFI_MAX_SOCKETS];

} NW_WIFI_ESP;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
#ifdef LINUX
BOOLEAN NW_WIFI_ESP_Init(NW_WIFI_ESP*     pThis,
                          U8*             pSSID,
                          U8*             pPassPhrase,
                          U32             baud_rate,
                          U8*             pSerialPort,
                          S32             time_zone);
#endif

#ifdef NUCLEO_F439ZI_EVK_BOARD
BOOLEAN NW_WIFI_ESP_Init(NW_WIFI_ESP*     pThis,
                          U8*             pSSID,
                          U8*             pPassPhrase,
                          U32             baud_rate,
                          UART*           pSerialDriver,
                          S32             time_zone);
#endif   


#ifdef ESP32_S2_MINI

BOOLEAN NW_WIFI_ESP_Init(NW_WIFI_ESP*    pThis,
                         U8*             pSSID,
                         U8*             pTimeZone,
                         NW_ESP_WIFI_security_t security,  
                         void*            p1);
#endif 

void NW_WIFI_ESP_Term();

int NW_WIFI_ESP_OpenSocket(NW_ESP_WIFI_socket_type_t type,
                            NW_ESP_WIFI_socket_app_t soc_app,
                            char* ip_addr, 
                            int port,
                            S32* socket_id);

int NW_WIFI_ESP_CloseSocket(S32 socket_id);

int NW_WIFI_ESP_SendSocket(S32 socket_id,char* buffer, int size, int timeout_ms);

int NW_WIFI_ESP_ReceiveSocket(S32 socket_id, char* buffer, int size, int timeout_ms);

U32 NW_WIFI_ESP_GetTime(U8* time);

U32 NW_WIFI_ESP_GetLocalIpAddr(U8* addr);

S8 NW_WIFI_ESP_ApInit();

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/



