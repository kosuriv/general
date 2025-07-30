/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : mqtt_com.h
*
*  Description         : This file contains NW API functions using ESP serial SDK
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  2nd Sep 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef MQTT_COM_H
#define MQTT_COM_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "MQTTClient.h"
#endif

#include "data_types.h"
#include "osa.h"
#include "app_setup.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Macro Definitions
*******************************************************************************/
#define MQTT_COM_MAX_BUF_SIZE APP_MAX_SIZE_PKT

#define MQTT_COM_COMMAND_TIMEOUT_MS  5000

#define MQTT_COM_VERSION 3

#define MQTT_COM_KEEP_ALIVE_INTERVAL 60

#define MQTT_COM_COMMAND_CYCLE_TIMEOUT_MS  2000


#ifdef ESP32_S2_MINI
enum QoS { QOS0, QOS1, QOS2, SUBFAIL=0x80 };
#endif

/*******************************************************************************
*                          Type Declarations
*******************************************************************************/
typedef enum 
{
    MQTT_COM_NO_SECURITY,
    MQTT_COM_SSL_SECURITY
} MQTT_COM_Security_type;

typedef struct 
{
    U8* ca;
    U32 ca_len;

    U8* key;
    U32 key_len;

    U8* client;
    U32 client_len;
    
}MQTT_COM_ssl_security_t;

typedef struct 
{
    U8 username[32];

    U8 password[32];
    
}MQTT_COM_no_security_t;


typedef struct 
{
    union {

    MQTT_COM_no_security_t  NoSsl;

    MQTT_COM_ssl_security_t Ssl;

    }s;
}
MQTT_COM_Security_t;


typedef struct MQTT_COM
{
    U8 client_id[20];

    U8 host[100];

    U32 port;

    U8 topic[20];
     
    osa_thread_handle rx_thread; 

    CALLBACK   rx_callback;

#ifdef NUCLEO_F439ZI_EVK_BOARD

    Network n;

    MQTTClient c;
#endif

    enum QoS qos;

#ifdef NUCLEO_F439ZI_EVK_BOARD

    U8  write_buf[MQTT_COM_MAX_BUF_SIZE];

    U8 read_buf[MQTT_COM_MAX_BUF_SIZE];
#endif

    volatile S32 toStop;

    BOOLEAN is_connected ;

    MQTT_COM_Security_type     security;                                    

 //  MQTT_COM_ssl_t          Ssl;

 //   MQTT_COM_no_security_t  NoSsl;

} MQTT_COM;


/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
void MQTT_COM_Init(MQTT_COM*       pThis,
                   U8*             mqtt_client_id,
                   U8*             mqtt_host,
                   U32             mqtt_port,
                   U8*             username,
                   U8*             password,
                   U8*             topic, 
                   enum QoS        qos,
                   CALLBACK32_FCT  pFunction,
                   void            *pArg);
#endif
#ifdef ESP32_S2_MINI
void MQTT_COM_Init(MQTT_COM*       pThis,
                   U8*             mqtt_client_id,
                   U8*             mqtt_host,
                   U32             mqtt_port,
                   U8*             topic, 
                   enum QoS        qos,
                   CALLBACK32_FCT  pFunction,
                   void            *pArg,
                   MQTT_COM_Security_type security,
                   void*            p1);
#endif

BOOLEAN MQTT_COM_SendMessage(U8* Msg, U32 Msglength);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/



