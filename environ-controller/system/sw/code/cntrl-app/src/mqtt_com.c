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
#include "mqtt_com.h"
#include "app_setup.h"
#include "data_types.h"
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

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void   mqtt_com_rx_thread_entry_func(void* pThis);

static void mqtt_com_RxMessageCallback(MessageData* md);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static MQTT_COM *pMqttCom;

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
                   U8*             username,
                   U8*             password,
                   U8*             topic, 
                   enum QoS        qos,
                   CALLBACK32_FCT  pFunction,
                   void            *pArg)
{
    pMqttCom = pThis;

    memset(pMqttCom, 0x00, sizeof(MQTT_COM));

    strcpy (pMqttCom->client_id,mqtt_client_id);

    strcpy (pMqttCom->host,mqtt_host);
    pMqttCom->port = mqtt_port ;

    strcpy (pMqttCom->username,username);
    strcpy (pMqttCom->password,password);

    strcpy (pMqttCom->topic,topic);

    pMqttCom->qos = qos;

    pMqttCom->rx_callback.pFunction = pFunction;
    pMqttCom->rx_callback.pArg= pArg;


#ifdef LINUX
      /* Create mqtt rx thread */
    OSA_ThreadCreate(&pMqttCom->rx_thread, 0 ,mqtt_com_rx_thread_entry_func, pMqttCom, NULL, NULL,0 ,0);
#else
  /* Create mqtt rx thread */
    OSA_ThreadCreate(&pMqttCom->rx_thread,APP_MQTT_RX_THREAD_PRIORITY ,mqtt_com_rx_thread_entry_func, pMqttCom, "mqtt_rx_thread", NULL,APP_MQTT_RX_THREAD_STACK_SIZE ,0);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : It is always called after MQTT Initialization
*******************************************************************************/
BOOLEAN MQTT_COM_SendMessage(U8* Msg, U32 Msglength)
{
    S32 rc = -1;

    MQTTMessage message;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    if(pMqttCom->is_connected)
    {
    	memset(&message , 0x00, sizeof(MQTTMessage));
    	message.payloadlen = (int)Msglength;

    	message.payload = (char*)Msg;

        message.qos = pMqttCom->qos;

    	rc = MQTTPublish(&pMqttCom->c, pMqttCom->topic, &message);
    }
    else
    {
        TRACE_WARN("%s() MQTT Connection not Up . so DROPPED = %d\n", __FUNCTION__,rc);
        return FALSE;
    }
   
    if(rc == 0)
    {
        TRACE_INFO("%s Exit \n", __FUNCTION__);
        return TRUE;
    }
    else
    {

        TRACE_WARN("%s() MQTT Send Error , so DROPPED = %d\n", __FUNCTION__,rc);
        //MQTTDisconnect(&pMqttCom->c);
        //NetworkDisconnect(&pMqttCom->n);
        TRACE_WARN("%s() Exit DROPPED = %d\n", __FUNCTION__,rc);
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
static void  mqtt_com_rx_thread_entry_func(void* pThis)
{
    MQTT_COM* pMqttCom = (MQTT_COM*) pThis;

    S32 rc = 0;

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;


    MQTTClientOneTimeInit(&pMqttCom->c);


START_AGAIN:

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    pMqttCom->is_connected = FALSE;

    NetworkInit(&pMqttCom->n);

    rc = NetworkConnect(&pMqttCom->n, pMqttCom->host, pMqttCom->port);

    if(rc)
    {
        TRACE_WARN("%s MQQT Socket open Failed (%d), Try once again \n", __FUNCTION__,rc);
        vTaskDelay(100); // delay or reset ESP module TBD ?
        goto START_AGAIN; 
    }


    MQTTClientInit(&pMqttCom->c, &pMqttCom->n,
                   MQTT_COM_COMMAND_TIMEOUT_MS,
                   pMqttCom->write_buf, 
                   MQTT_COM_MAX_BUF_SIZE, 
                   pMqttCom->read_buf,
                   MQTT_COM_MAX_BUF_SIZE);
 
  //  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
     
    data.willFlag = 0;
    data.MQTTVersion = MQTT_COM_VERSION;
    data.clientID.cstring = pMqttCom->client_id;
    data.username.cstring = pMqttCom->username;
    data.password.cstring = pMqttCom->password;

    data.keepAliveInterval = MQTT_COM_KEEP_ALIVE_INTERVAL;
    data.cleansession = 1;
    TRACE_DBG("Connecting to %s %d\n", pMqttCom->host, pMqttCom->port);
    
    rc = MQTTConnect(&pMqttCom->c, &data);
    TRACE_DBG("Connected %d\n", rc);

    if( rc != -1)
    {
    	TRACE_DBG("Subscribing to %s\n",pMqttCom->topic);
    	rc = MQTTSubscribe(&pMqttCom->c, pMqttCom->topic, pMqttCom->qos,mqtt_com_RxMessageCallback);
    	TRACE_INFO("Subscribed %d\n", rc);

    	if(rc == -1)
    	{
        	goto START_AGAIN;
    	}
    }
    else
    {
    	goto START_AGAIN;
    }
	pMqttCom->is_connected = TRUE;

    while (!pMqttCom->toStop)
    {
        rc = MQTTYield(&pMqttCom->c, MQTT_COM_COMMAND_CYCLE_TIMEOUT_MS);
   
		if( rc == -1 )
		{
			TRACE_DBG("%s() SOME NETWORK ERROR\n", __FUNCTION__);
	   	    MQTTClientDeInit(&pMqttCom->c);
	   		goto START_AGAIN;
           	break;
		}
    }
    //MQTTDisconnect(&pMqttCom->c);
    NetworkDisconnect(&pMqttCom->n);

}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void mqtt_com_RxMessageCallback(MessageData* md)
{
    MQTTMessage* message = md->message;

    BUFFER8 pBuffer;

    pBuffer.size = (int)message->payloadlen;

    pBuffer.pBuf = (char*)message->payload;

    pMqttCom->rx_callback.pFunction(pMqttCom->rx_callback.pArg, &pBuffer);

}

/*******************************************************************************
*                          End of File
*******************************************************************************/
