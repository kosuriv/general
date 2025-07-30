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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nw_wifi_esp.h"
#include "osa.h"
#include "utility.h"
#ifdef LINUX
#include "serial_comport.h"
#else
#include "uart.h"
#endif
#include "qlist.h"
#include "app_setup.h"
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
#define NW_ESP8266_MAX_RX_BUFFER_SIZE  APP_MAX_SIZE_PKT

#define NW_ESP8266_MAX_SSL_BUFFER_SIZE  4096


/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void  nw_wifi_esp_rx_thread_entry_func_old(void* pThis);

static void  nw_wifi_esp_rx_thread_entry_func(void* pThis);

static U32 nw_wifi_esp_set_echo_off();

static U32 nw_wifi_esp_set_wlan_mode(U32 mode);

static U32 nw_wifi_esp_start_wlan_connection(U8 *pSSID, U8 *pSeed);

static U32 nw_wifi_esp_start_tcp_client(U8* addr, U32 port ,BOOLEAN is_ssl,S32 socket_id);

static U32 nw_wifi_esp_stop_socket(S32 socket_id);

static void nw_wifi_esp_send_tcp(U8 *file_name, U32 mode);

static U32 nw_wifi_esp_stop_wlan_connection() ;

#if 1 //LINUX

inline nw_wifi_esp_get_next_output_from_console(U8** pRxBuf) __attribute__((always_inline));

inline BOOLEAN nw_wifi_esp_is_cmd_asynchrounous(U8* buffer)  __attribute__((always_inline));;

inline void nw_wifi_esp_tx(U8* buffer, U32 length) __attribute__((always_inline));;

#else

inline nw_wifi_esp_get_next_output_from_console(U8** pRxBuf);

inline BOOLEAN nw_wifi_esp_is_cmd_asynchrounous(U8* buffer);

inline void nw_wifi_esp_tx(U8* buffer, U32 length);

#endif

static U32 nw_wifi_esp_send_data(S32 socket_id, U8* buf, U32 length);

static U32 nw_wifi_esp_set_multiple_connections(U32 on_off);

static U32 nw_wifi_esp_set_sleep_mode(U32 on_off);

static U32 nw_wifi_esp_set_ssl_buffer_size(U32 size);

static void nw_wifi_esp_uart_rx_completed(void *pThis, UINT32 value);

static U32 nw_wifi_esp_enable_sntp(S32 time_zone);

static U32 nw_wifi_esp_set_ssl_cert_choice(U32 choice);

int nw_wifi_esp_ReceiveMqttTcpClient(char* buffer, int size, int timeout, S32 socket_id);

static void  nw_wifi_esp_post_socket_data(U8* pBuf);

static S32  nw_wifi_esp_allot_socket_id();

static void  nw_wifi_esp_free_all_sockets();

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static NW_WIFI_ESP* pNwWifi;

static BOOLEAN is_init_done = FALSE;

volatile  BOOLEAN data_prompt_flag = FALSE;

static U8 data_prompt = 0x00;

//static BOOLEAN is_tcp_client_ok = FALSE;

static BOOLEAN is_wifi_disconnected = FALSE;


U8 NW_WIFI_ESP_Rx_Buf[2048];

volatile U8* pSendBuf;

volatile int NW_WIFI_flag;

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
#ifdef LINUX
BOOLEAN NW_WIFI_ESP_Init(NW_WIFI_ESP*     pThis,
                          U8*             pSSID,
                          U8*             pPassPhrase,
                          U32             baud_rate,
                          U8*             pSerialPort,
                          S32             time_zone)

#else
BOOLEAN NW_WIFI_ESP_Init(NW_WIFI_ESP*     pThis,
                          U8*             pSSID,
                          U8*             pPassPhrase,
                          U32             baud_rate,
                          UART*           pSerialDriver,
						  S32             time_zone)
#endif
{  
    pNwWifi = pThis;

    U32 i;
     
    if(!is_init_done)
    {
        memset((U8*)pNwWifi,0x00,sizeof(NW_WIFI_ESP));

        OSA_InitSemaphore(&pNwWifi->data_status_sem,0,1);

        /* Create tx mutex for transferring */
        OSA_InitMutex(&pNwWifi->tx_mutex) ;

        OSA_InitMsgQue(&pNwWifi->pRxCmdQ,"cmd_q", NW_ESP_WIFI_MAX_NO_RX_MSGS, sizeof(void *));

      //  OSA_InitMsgQue(&pNwWifi->pRxDataQ,"data_q", NW_ESP_WIFI_MAX_NO_RX_MSGS, sizeof(void *));

#ifdef LINUX
        /* Init serial port */
        SERIAL_COMPORT_Init(baud_rate , pSerialPort);
#else
        pNwWifi->pSerialDriver = pSerialDriver;
        OSA_InitSemaphore(&pNwWifi->uart_rx_sem,0,1);
#endif


#ifdef LINUX
        /* Create rx thread */
        OSA_ThreadCreate(&pNwWifi->rx_thread, 0 ,nw_wifi_esp_rx_thread_entry_func_old, NULL, NULL, NULL,0 ,0);
#else
        /* Create rx thread */
        OSA_ThreadCreate(&pNwWifi->rx_thread,
        		         APP_ESP_WIFI_RX_THREAD_PRIORITY ,
						 nw_wifi_esp_rx_thread_entry_func,
						 NULL,
                         "esp_rx_thread",
						 NULL,
						 APP_MQTT_RX_THREAD_STACK_SIZE,
						 0);
#endif
        is_init_done = TRUE ;
    }
    pNwWifi->baud_rate = baud_rate; 

#ifdef LINUX
    strcpy(pNwWifi->serial_port, pSerialPort);
#endif

    strcpy(pNwWifi->ssid, pSSID);

    strcpy(pNwWifi->pass_phrase, pPassPhrase);

    data_prompt_flag = FALSE;

    pNwWifi->time_zone = time_zone;
    
    /* Create tx mutex for transferring */
    OSA_InitMutex(&pNwWifi->call_mutex) ;

    is_wifi_disconnected = TRUE;

    OSA_InitMsgQue(&pNwWifi->pRxMsgQ,"nw_rx_msg_q", 10, sizeof(void *));

    for(i=0; i < NW_ESP_WIFI_MAX_SOCKETS; i++)
    {
        OSA_InitMsgQue(&pNwWifi->Sockets[i].pRxSocDataQ,"data_q", NW_ESP_WIFI_MAX_NO_RX_MSGS, sizeof(void *));
    }


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
    U32 status ;

    OSA_LockMutex(&pNwWifi->call_mutex);

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    /* Start Wifi Connection */
    data_prompt_flag = FALSE;

    if(is_wifi_disconnected)
    {

    	while(!nw_wifi_esp_set_echo_off());

    	nw_wifi_esp_stop_socket(5);  /* pre cautionary purpose , close all sockets */

    	while(!nw_wifi_esp_set_wlan_mode(1));

    	while(!nw_wifi_esp_start_wlan_connection(pNwWifi->ssid,pNwWifi->pass_phrase));

    	while(!nw_wifi_esp_enable_sntp(pNwWifi->time_zone));

    	while(!nw_wifi_esp_set_multiple_connections(1));

    	is_wifi_disconnected = FALSE;
    }

    *socket_id =  nw_wifi_esp_allot_socket_id();

    if(*socket_id < 0)
    {
        TRACE_WARN("No socket alloted,  Negative Socket ID = %d\n", *socket_id);
        return -1;
    }

    switch(type)
    {
        case NW_WIFI_TCP_CLIENT:
            status = nw_wifi_esp_start_tcp_client(ip_addr, port,FALSE,*socket_id);
            break;

        case NW_WIFI_SSL_TCP_CLIENT:
            while(!nw_wifi_esp_set_ssl_buffer_size(NW_ESP8266_MAX_SSL_BUFFER_SIZE));
            while(!nw_wifi_esp_set_ssl_cert_choice(0x00)); /* no verifictaion of certs ,
                                                            it will avoid pre programming cert */
            status = nw_wifi_esp_start_tcp_client(ip_addr, port,TRUE,*socket_id);
            break;
    }

    if( status == 1)
    {
        pNwWifi->Sockets[*socket_id].is_socket_open = TRUE;
        pNwWifi->Sockets[*socket_id].socket_type = type;
        pNwWifi->Sockets[*socket_id].app_type = soc_app;
    }

    TRACE_DBG("Sending STATUS = %d\n", status);

    OSA_UnLockMutex(&pNwWifi->call_mutex);

    return (status == 1) ? 0 : -1;
}


/*******************************************************************************
* Name       : 
* Description: It closes TCP client socket 
* Remarks    : 
*******************************************************************************/
int NW_WIFI_ESP_CloseSocket(S32 socket_id)
{
	U32 status = 1;

    OSA_LockMutex(&pNwWifi->call_mutex);

	TRACE_DBG("%s() Entry \n", __FUNCTION__);

    if(pNwWifi->Sockets[socket_id].is_socket_open)
	{
        if(pNwWifi->Sockets[socket_id].app_type != NW_ESP_WIFI_SOCKET_MQTT)
        {
            pNwWifi->Sockets[socket_id].is_socket_open = FALSE;
        }
        status = nw_wifi_esp_stop_socket(socket_id);
    }

    OSA_UnLockMutex(&pNwWifi->call_mutex);

	return (status == 1) ? 0 : -1;

}

/*******************************************************************************
* Name       : 
* Description: It sends the data over TCP client socket 
* Remarks    : 
*******************************************************************************/
int NW_WIFI_ESP_SendSocket(S32 socket_id,char* buffer, int size, int timeout_ms)
{
	U32 status;
    U8* buf;

	OSA_LockMutex(&pNwWifi->call_mutex);

	TRACE_DBG("%s() Entry \n", __FUNCTION__);

 //   TRACE_DBG("Data(%d):- %s\n", size,buffer);

    if(!pNwWifi->Sockets[socket_id].is_socket_open)
    {
        OSA_UnLockMutex(&pNwWifi->call_mutex);
    	return -1;
    }

    status = nw_wifi_esp_send_data(socket_id,buffer,size);

    OSA_UnLockMutex(&pNwWifi->call_mutex);
    
    if( status  ==  0)
    {
        TRACE_WARN("%s() SEND ERROR\n",__FUNCTION__);
        return -1;

    }
    else if( status == 1)
    {
        return size;
    }
 //   return (status == 1) ? size : -1;
}


/*******************************************************************************
* Name       : 
* Description: It sends the data over TCP client socket 
* Remarks    : 
*******************************************************************************/
int NW_WIFI_ESP_ReceiveSocket(S32 socket_id, char* buffer, int size, int timeout)
{
    U8* pBuf;
    U8* p;
    U8* q;

    U32 length_received;

RX:
    if(pNwWifi->Sockets[socket_id].app_type == NW_ESP_WIFI_SOCKET_MQTT)
    {
        return nw_wifi_esp_ReceiveMqttTcpClient(buffer, size, timeout, socket_id);
    }
    else
    {
        OSA_ReceiveMsgQue(pNwWifi->Sockets[socket_id].pRxSocDataQ, &pBuf);

        TRACE_DBG("Data POP from Q for socket_id = %d \n",socket_id);

        //TRACE_DBG("Poped String :%s\n",pBuf);

        if(!pNwWifi->Sockets[socket_id].is_socket_open)
        {
            /* to handle abnormal socket close  */
            TRACE_DBG("Socket CLOSED Received\n");
            free(pBuf);
            return -1;
        }

        /* Check if it is valid  data packet or not */
        if(!strstr(pBuf,"+IPD,"))
        {
            TRACE_DBG("Poped String :%s\n",pBuf);
            free(pBuf);
            goto RX;
        }

        /* Calculate the data length*/
        p = pBuf;    
        q = strchr(p,',');
        p = ++q;
        q = strchr(p,',');
        p = ++q;
        q = strchr(p,':');
        *q = 0x00;  
        length_received = strtol((char *)p, NULL, 10);
        *q = ':';  
        q++;
        TRACE_DBG("The length received = %d \n", length_received);

        // UTILITY_PrintBuffer(buffer , length_received, 1);

        if(length_received <= size)
        {
            memcpy(buffer,q,length_received);
            return length_received;
        }
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void NW_WIFI_ESP_Term()
{
#ifdef LINUX
    SERIAL_COMPORT_DeInit();
#else
    OSA_DeInitSemaphore(&pNwWifi->uart_rx_sem);
#endif

    /* close both rx & start threads */
    OSA_ThreadClose(pNwWifi->rx_thread);

    /* Terminate semaphores */
    OSA_DeInitSemaphore(&pNwWifi->data_status_sem);

    OSA_DeInitMutex(&pNwWifi->tx_mutex) ;

    OSA_DeInitMutex(&pNwWifi->call_mutex) ;

    is_init_done = FALSE;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 NW_WIFI_ESP_GetTime(U8* time)
{

    U8 cmd[25];
    U8* result = NULL;
    U8* pRxBuf= NULL;
    U32 status = 0 ;
    U8* q = NULL;

    OSA_LockMutex(&pNwWifi->call_mutex);

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd,"AT+CIPSNTPTIME?\r\n");
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);

    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"ERROR");
        if(result)
        {
            TRACE_DBG("SNTP not ON  \n");
            status=0;
            goto END5;
        }
        else 
        {
            result = strstr(pRxBuf,"OK");
            if(result)
            {
                status=1;
                goto END5;                
            }
            else
            {
                result = strstr(pRxBuf,"+CIPSNTPTIME:");
                if(result)
                {
                    q = strchr(pRxBuf,':');
                    q++;
                    strcpy(time,q);
                    time[strlen(time)-2] = 0x00;
                
                }
            }
        }        
    }
END5:
    free(pRxBuf);
    OSA_UnLockMutex(&pNwWifi->call_mutex);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void NW_WIFI_ProcessRx()
{
    //OSA_GiveSemaphoreIsr(&pNwWifi->uart_rx_sem);
    OSA_SendMsgQueIsr(pNwWifi->pRxMsgQ,(void *)(pSendBuf));
   // pSendBuf = NULL;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 NW_WIFI_ESP_GetLocalIpAddr(U8* ip_addr)
{
    U8 cmd[45];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;
    U8* p;
    U8* q;
    U32 no_of_networks;
    U32 i;   

    OSA_LockMutex(&pNwWifi->call_mutex);

    TRACE_DBG("%s() Entry\n",__FUNCTION__);

    memset(cmd,0x00,sizeof(cmd));
    sprintf(cmd,"AT+CIPSTA?\r\n");
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            printf("IP got it \n");
            status = 1;
            goto END4;
        }
        else
        {
           // result = strstr(pRxBuf,"gateway:\"");
            result = strstr(pRxBuf,"ip:\"");
            if(result)
            {
                printf("Retrieving Local IP = %s \n", result);
                p = strchr(result,'"');
                p++;
                q=p;     
                p = strchr(q,'"');
                memcpy(ip_addr, q,p-q);
                ip_addr[p-q]=0x00;
                TRACE_DBG("LocalIp =%s\n",ip_addr);
            }
        }   
    }
END4:
    free(pRxBuf);
    OSA_UnLockMutex(&pNwWifi->call_mutex);
    return status;
}

/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void  nw_wifi_esp_rx_thread_entry_func_old(void* pThis)
{
    U8* pBuf;
    U32 rx_len;
    BOOLEAN is_data = FALSE;
    U8* p;
    //U8* q;
    U32 data_len;
    U32 j;
    BOOLEAN is_next_byte_data;

    U8* Q;

    U8* P;

   //OSA_ThreadSetCloseState(OSA_THREAD_ENABLE_CLOSE_STATE);
    
   // TRACE_DBG("IN ESP RX THREAD ENTRY\n");

    while(1)
    { 

BEGIN:      
        pBuf = NULL;
        rx_len = 0x00;
        is_data = FALSE;
        data_len = 0x00;

        pBuf = (U8*)malloc(NW_ESP8266_MAX_RX_BUFFER_SIZE); /* allocate fresh buffer */

#if 1
        if(pBuf == NULL)
        {
            TRACE_DBG("No Received Buffer allocated\n");
            exit(-1);
        }
#endif
        Q = pBuf;

        //TRACE_DBG("ALLOCATED = %p \n", pBuf);
        memset(pBuf,0x00,4);
        while(1)
        {
#ifdef LINUX
            SERIAL_COMPORT_Read(pBuf, 1);
#else
	        UART_Rx(pNwWifi->pSerialDriver, pBuf, 1 , nw_wifi_esp_uart_rx_completed,NULL,UART_MODE_0);

	        OSA_TakeSemaphore(&pNwWifi->uart_rx_sem);
#endif
            //TRACE_DBG("RECEIVED CHAR = %c  %x \n", *pBuf, *pBuf);
#if 0
            if (data_prompt_flag == TRUE)
            {
                /* used to detect data prompt > */
                if (data_prompt != 0x3e)
                {
                    data_prompt = *pBuf;
                }
            }
#endif
            rx_len++;

            if( (data_prompt_flag == TRUE) && (rx_len == 2) && (*pBuf == 0x20) && (*(pBuf-1) == 0x3e) )
            {
            	OSA_SendMsgQue(pNwWifi->pRxCmdQ,(void *)(pBuf-1));
                pBuf++;
                *pBuf = 0x00; 
                data_prompt_flag = FALSE;              
                goto BEGIN;
            }

            if(rx_len >=2)
            {        
                /* cmd response comes with \r \n */
                if( ((*pBuf == '\n') && ((*(pBuf-1) == '\r')) ) && (is_data == FALSE) ) 
                {
                    pBuf++;
                    *pBuf = 0x00;               
                    break;
                }
                if(rx_len >=4)
                {
                    /* check for data arrival */
                    if( (*pBuf == 0x44) &&
                        (*(pBuf-1) == 0x50) &&
                        (*(pBuf-2) == 0x49) && 
                        (*(pBuf-3) == 0x2b)
                      )
                    {
                        is_data = TRUE;
                        is_next_byte_data = FALSE;                   
                    }
                }
                if(is_data)
                {
                    if( (*pBuf == 0x2c) && (!is_next_byte_data))
                    {
                        p = pBuf+1;
                    }
                    if( (*pBuf == 0x3a) && (!is_next_byte_data))
                    {
                        *pBuf = 0x00; 
                        data_len = strtol((char *)p, NULL, 10);
                        *pBuf = 0x3a;
                    //    TRACE_DBG("DATA LEN TO BE RECEIVED  = %d \n", data_len);

                        is_next_byte_data = TRUE;
                        j=0;
                    }
                    if(is_next_byte_data)
                    {
                        j++;
                        if(j == (data_len +1))
                        {
                            /* complete data received */
                        	//TRACE_DBG("COMPLETE  DATA RECEIVED  = %d \n", --j);
                            pBuf++;
                            *pBuf = 0x00;               
                            break;                           
                        }
                    }
                }
            }                      
            pBuf++;
        } //inner while(1)

       // TRACE_DBG("Received length = %d\n",rx_len);

        if( rx_len > 2)
        {
            //TRACE_DBG("RECEIVED = %s\n", pBuf-rx_len);

            if(is_data)
            {
            	//TRACE_DBG("RECEIVED DATA(len:%d) = %s\n",rx_len, pBuf-rx_len);
              //  OSA_SendMsgQue(pNwWifi->pRxDataQ,(void *)(pBuf-rx_len));
                nw_wifi_esp_post_data_correct_dataQ(pBuf);       
            }
            else
            {
                if(nw_wifi_esp_is_cmd_asynchrounous(pBuf-rx_len))
                {

#if 0
                    /* Handle asynchrounous events TBD  */
                   //
                //if((!is_tcp_client_ok)  && (strstr((pBuf-rx_len),"CLOSED") != NULL))
				//	if(!is_tcp_client_ok) 
					{
                		//TRACE_DBG("ASYNC MSG RECEIVED = %s  \n",pBuf-rx_len);
                		/* Send Asynchronous CLOSED  in Data Q to signal barupt closing */
                		OSA_SendMsgQue(pNwWifi->pRxDataQ,(void *)(pBuf-rx_len));
                		//OSA_SendMsgQue(pNwWifi->pRxDataQ,(void *)(pBuf-rx_len));
					}

                    else if((is_wifi_disconnected)  && (strstr((pBuf-rx_len),"WIFI DISCONNECT") != NULL))
                    {
                        OSA_SendMsgQue(pNwWifi->pRxDataQ,(void *)(pBuf-rx_len));
                    }


                	else
                	{
                		free((void *)(pBuf-rx_len));
                	}
#endif
                }
                else
                {
                    //TRACE_DBG("RECEIVED = %s\n", pBuf-rx_len);
                    OSA_SendMsgQue(pNwWifi->pRxCmdQ,(void *)(pBuf-rx_len));                 
  
                }
            }
        }
        else
        {
            /* it could be data send  response */
            if(strchr(pBuf-rx_len, 0x1b))
            {
                //TRACE_DBG("SENT DATA STATUS  = %s\n", pBuf-rx_len);
                //UTILITY_PrintBuffer(pBuf-rx_len , rx_len, 0);
                OSA_GiveSemaphore(&pNwWifi->data_status_sem);
            }
            free((void *)(pBuf-rx_len));
            P = (pBuf-rx_len);
        }  // check rx_len > 2
   
    }   
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_set_echo_off()
{
    U8 cmd[20];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;

    TRACE_DBG("nw_wifi_esp_set_echo_off:- Setting echo off\n");

    memset(cmd,0x00,sizeof(cmd));
    sprintf(cmd,"ATE0\r\n");
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("Echo Off\n");
            status = 1;
            goto END1;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("Echo Not set\n");
                goto END1;
            }
        }
    }
END1:
    free(pRxBuf);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_set_wlan_mode(U32 mode)
{
    U8 cmd[100];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;

    TRACE_DBG("nw_wifi_esp_set_wlan_mode:- Setting mode  \n");

    memset(cmd,0x00,sizeof(cmd));
   // sprintf(cmd,"AT+CWMODE_CUR=%d\r\n",mode);
    sprintf(cmd,"AT+CWMODE=%d\r\n",mode);
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("Setting WLAN  mode \n");
            status = 1;
            goto END2;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("NOT Set WLAN  mode to AP \n");
                goto END2;
            }
        }
    }
END2:
    free(pRxBuf);
    return status;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_start_wlan_connection(U8 *pSSID, U8 *pPassPhrase) 
{
    U8  cmd[100];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0 ;
    U8 passphrase[100];
    U32 len;
    U8 psk[32];
    U32 i;
    U8* p;
    BOOLEAN is_psk_set = TRUE;
    
    TRACE_DBG("nw_wifi_esp_start_wlan_connection:- starting wlan connection \n");

    memset(passphrase,0x00,sizeof(passphrase));   
    strcpy(passphrase,pPassPhrase);
    memset(cmd,0x00,sizeof(cmd));
   // sprintf(cmd,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n",pSSID,passphrase);
    sprintf(cmd,"AT+CWJAP=\"%s\",\"%s\"\r\n",pSSID,passphrase);

    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));
    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("WLAN ASSOCIATION SUCCESSFUL\n");
            status = 1;
            goto  END3; 
        }
        else
        {
            result = strstr(pRxBuf,"FAIL");
            if(result)
            {
                TRACE_DBG("WLAN ASSOCIATION FAILED\n");
                status = 0;
                goto  END3; 
            }
        }
    }
END3:
    free(pRxBuf);
    return status;

}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_start_tcp_client(U8* ip_addr, U32 port, BOOLEAN is_ssl,S32 socket_id) 
{
    U8 cmd[100];
    U8* result = NULL;
    U8* pRxBuf= NULL;
    U32 status = 0 ;
    

    TRACE_DBG("%s  starting TCP client with %s, on  %d\n",__FUNCTION__,ip_addr,port);

    memset(cmd, 0x00, sizeof(cmd));

    if(is_ssl)
    {
    	sprintf(cmd,"AT+CIPSTART=%d,\"SSL\",\"%s\",%d,%d\r\n",socket_id,ip_addr,port,7200);
    }
    else
    {
    	sprintf(cmd,"AT+CIPSTART=%d,\"TCP\",\"%s\",%d,%d\r\n",socket_id,ip_addr,port,7200);
    }
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);

    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"ERROR");
        if(result)
        {
            TRACE_DBG("TCP Client NOT started \n");
            status=0;
            goto END5;
        }
        else 
        {
        	TRACE_DBG("TCP Client Started = %s\n",pRxBuf);
            result = strstr(pRxBuf,"OK");
            if(result)
            {
            	//TRACE_DBG("TCP Client RECEIVED OK \n");
                status=1;
                goto END5;                
            }
            else
            {
                result = strstr(pRxBuf,"ALREADY CONNECT");
                if(result)
                {
                	//TRACE_DBG("TCP Client ALREADY CONNECT \n");
                    status=1;
                    goto END5;                
                }
            }
        }        
    }
END5:
    free(pRxBuf);
    return status;
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_stop_socket(S32 socket_id) 
{
    U8 cmd[100];
    U8* result = NULL;
    U8* pRxBuf= NULL;
    U32 status = 0;

    memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd,"AT+CIPCLOSE=%d\r\n",socket_id);
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("Client connection closed successfully\n");
            status=1;
            goto END6;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
        }
        if(result)
        {
            TRACE_DBG("Failed to close connection\n");
            status=0;
            goto END6;
        }
    }
END6:
    free(pRxBuf);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_stop_wlan_connection() 
{

    U8  cmd[100];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0 ;

    TRACE_DBG("nw_wifi_esp_stop_wlan_connection:- disconnecting wlan connection \n");

    memset(cmd, 0x00, sizeof(cmd));
    sprintf(cmd,"AT+CWQAP\r\n");
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));    

    while(1)
    {
        free(pRxBuf);
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("WLAN DIS-ASSOCIATION SUCCESSFUL\n");
            status = 1;
            goto  END7; 
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("WLAN DIS-ASSOCIATION FAILED\n");
                status = 0;
                goto  END7; 
            }
        }
    }
END7:
    free(pRxBuf);
    return status;

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
inline nw_wifi_esp_get_next_output_from_console(U8** pRxBuf)
{
  //  OSA_TakeSemaphore(&pNwWifi->rx_cmd_sem);
 
  //  LIST_Pop(&pNwWifi->pRxList,pRxBuf);

    //TRACE_DBG("PROCESSING = %p  = %s \n", *pRxBuf,*pRxBuf);

    OSA_ReceiveMsgQue(pNwWifi->pRxCmdQ, pRxBuf);
   // TRACE_DBG("PROCESSING = %p  = %s \n", *pRxBuf,*pRxBuf);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
inline BOOLEAN nw_wifi_esp_is_cmd_asynchrounous(U8* buffer)
{
    BOOLEAN  is_asynch = FALSE;

    if((strstr(buffer,"WIFI DISCONNECT") != NULL) || 
       (strstr(buffer,"Disassociation Event") != NULL) ||         
       (strstr(buffer,"Out of StandBy-Alarm") != NULL) ||
#if STDBY_ASYNCH
       (strstr(buffer,"Out of StandBy-Timer") != NULL)||
#endif
       (strstr(buffer,"Out of Deep Sleep") != NULL)    ||
       (strstr(buffer,"CLOSED") != NULL)    ||
       (strstr(buffer,"ERROR: SOCKET FAILURE") != NULL)        
       )
    {
        is_asynch = TRUE;
    }
    return is_asynch;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
inline void nw_wifi_esp_tx(U8* buffer, U32 length)
{

    OSA_LockMutex(&pNwWifi->tx_mutex);

#ifdef LINUX
    SERIAL_COMPORT_Write(buffer,length);
#else
    UART_Tx(pNwWifi->pSerialDriver, buffer, length , NULL,NULL,UART_MODE_0);
#endif

    OSA_UnLockMutex(&pNwWifi->tx_mutex);
}



/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_send_data(S32 socket_id, U8* buf, U32 length) 
{
    U8 cmd[100];
    U8* result = NULL;
    U8* pRxBuf= NULL;
    U32 status = 0;

    memset(cmd, 0x00, sizeof(cmd));
    data_prompt_flag = TRUE;
    data_prompt = 0x00;

    if(buf)
    {
        sprintf(cmd,"AT+CIPSEND=%d,%d\r\n",socket_id,length);
    }
    else
    {
        sprintf(cmd,"AT+CIPSEND\r\n");
    }
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        free(pRxBuf);
#if 1
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);
#else
        OSA_ReceiveMsgQueTimeout(pNwWifi->pRxCmdQ, &pRxBuf,200);

       // xQueueReceive(pNwWifi->pRxCmdQ, &pRxBuf, 100);

        if( pRxBuf == NULL)
        {
            TRACE_DBG("TIME OUT ON MSG WAIT \n");

            return 0;
        }
#endif
        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("Processed =  %s  \n",pRxBuf);
#if 0 
          //  TRACE_DBG("Waiting for  DATA prompt = %x \n",data_prompt);
            while (data_prompt != 0x3e)
            {
                OSA_Usleep(100*1000);
            }
            data_prompt = 0x00;
          //  TRACE_DBG("Received DATA prompt = %x \n",data_prompt);
            data_prompt_flag = FALSE;
            status=1;
            break;

#else
            free(pRxBuf);
            nw_wifi_esp_get_next_output_from_console(&pRxBuf);
            //TRACE_DBG("Processing2  =  %s  \n",pRxBuf);
            result = strstr(pRxBuf,"> ");
            if(result)
            {
                TRACE_DBG("DATA Prompt Received =  %s  \n",pRxBuf);
                status=1;
                break;
            }
#endif      
        }
#if 1
        else
        {
            result = strstr(pRxBuf,"link is not valid");
            if(result)
            {
                TRACE_DBG("link is not valid\n");
                status=0;
            }
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("ERROR\n");
                status=0;
                goto END10;
            }

        }
#endif
    }
    if(buf)
    {
        nw_wifi_esp_tx(buf,length);

        /* Check for status */
        while(1)
        {
            free(pRxBuf);
            nw_wifi_esp_get_next_output_from_console(&pRxBuf);

            result = strstr(pRxBuf,"SEND OK");
            if(result)
            {
                TRACE_DBG("Received SEND OK\n");
                status=1;
                break;
            }
            else
            {
                result = strstr(pRxBuf,"SEND FAIL");
                if(result)
                {
                    TRACE_DBG("SEND FAIL\n");
                    status=0;
                    break;
                }
            }       
        }
    }
END10:
    free(pRxBuf);
    return status;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_set_multiple_connections(U32 on_off)
{
    U8 cmd[20];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;

    TRACE_DBG("nw_wifi_esp_set_multiple_connections \n");

    memset(cmd,0x00,sizeof(cmd));
    sprintf(cmd,"AT+CIPMUX=%d\r\n",on_off);
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("Setting multiple connections on \n");
            status = 1;
            goto END9;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("Setting multiple connections not set\n");
                goto END9;
            }
        }
        free(pRxBuf);
    }
END9:
    free(pRxBuf);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_set_sleep_mode(U32 on_off)
{
    U8 cmd[20];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;

    TRACE_DBG("nw_wifi_esp_set_multiple_connections \n");

    memset(cmd,0x00,sizeof(cmd));
    sprintf(cmd,"AT+SLEEP=%d\r\n",on_off);
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("Setting multiple connections on \n");
            status = 1;
            goto END9;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("Setting multiple connections not set\n");
                goto END9;
            }
        }
        free(pRxBuf);
    }
END9:
    free(pRxBuf);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_set_ssl_buffer_size(U32 size)
{
    U8 cmd[20];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;

    TRACE_DBG("nw_wifi_esp_set_ssl_buffer_size \n");

    memset(cmd,0x00,sizeof(cmd));
    sprintf(cmd,"AT+CIPSSLSIZE=%d\r\n",size);
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("Setting multiple connections on \n");
            status = 1;
            goto END9;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("Setting multiple connections not set\n");
                goto END9;
            }
        }
        free(pRxBuf);
    }
END9:
    free(pRxBuf);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : It is run under UART ISR as callback 
*******************************************************************************/
#ifndef LINUX
static void nw_wifi_esp_uart_rx_completed_old(void *pThis, UINT32 value)
{

#if 0
	OSA_GiveSemaphoreIsr(&pNwWifi->uart_rx_sem);
#else
    NW_WIFI_flag = 1;
    //  portEND_SWITCHING_ISR(pdTRUE);
    portYIELD_FROM_ISR( pdTRUE );
#endif
}
#endif


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void  nw_wifi_esp_rx_thread_entry_func(void* pThis)
{
    U8* pBuf;

  //  U32 rx_len;
    U8* result = NULL;

    U8* p;
    U8* q;
    U32 socket_id;
    U32 i;
    BOOLEAN to_post ;

    UART_Rx(pNwWifi->pSerialDriver, NULL, 1 , nw_wifi_esp_uart_rx_completed,NULL,UART_MODE_0);

    while(1)
    {
        /* Blocking call till time out  */
        OSA_ReceiveMsgQue(pNwWifi->pRxMsgQ, &pBuf);

       // rx_len = strlen(pBuf);

        //TRACE_DBG( "RECEIVED(%d): %s \n",rx_len, pBuf);

        result = strstr(pBuf,"+IPD,");

        if(!result)
        {
            if(nw_wifi_esp_is_cmd_asynchrounous(pBuf))
            {
                /* Handle asynchrounous events TBD  */
                if (strstr(pBuf,"CLOSED") != NULL)
                {
					U8 time[75];
                  // OSA_LockMutex(&pNwWifi->call_mutex);  /* to prevent other functions not to close */
					TRACE_DBG("Asynch close Received = %s\n", pBuf);
					RTC_GetTime(time);
					//TRACE_INFO("Current  Time = %s\n",time);
                    p = pBuf;
                    q = strchr(p,',');
                    *q = 0x00;  
                    socket_id = strtol((char *)p, NULL, 10);
                    *q = ',';

                    if(pNwWifi->Sockets[socket_id].is_socket_open)
                    {  
                        pNwWifi->Sockets[socket_id].is_socket_open = FALSE;                      
                        to_post = TRUE;
                    }
                    else
                    {
                        /* socket was not yet opened earlier, CLOSED msg may be while opening the socket */
                        to_post = FALSE;
                    }
                    if(to_post)
                    {
                        OSA_SendMsgQue(pNwWifi->Sockets[socket_id].pRxSocDataQ,(void *)pBuf);
                    }
                    else
                    {   
                        free((void *)pBuf);
                    }
                  //  OSA_UnLockMutex(&pNwWifi->call_mutex);
                }
                else if  (strstr(pBuf,"WIFI DISCONNECT") != NULL)
                {
					TRACE_DBG("Aynch wifi disconnect received = %s\n", pBuf);
                    is_wifi_disconnected = TRUE;
                    for(i=0; i < NW_ESP_WIFI_MAX_SOCKETS; i++)
                    {
						if(pNwWifi->Sockets[i].is_socket_open)
						{
                        	pNwWifi->Sockets[i].is_socket_open = FALSE;
                        	OSA_SendMsgQue(pNwWifi->Sockets[i].pRxSocDataQ,(void *)(pBuf));
						}
                    }
                }
                else
                {
                    free((void *)pBuf);
                }
            }
            else
            {
                OSA_SendMsgQue(pNwWifi->pRxCmdQ,(void *)(pBuf)); 
            }
        }
        else
        {
            //TRACE_DBG("RECEIVED DATA = %s\n", pBuf);
			//OSA_SendMsgQue(pNwWifi->pRxDataQ,(void *)(pBuf));
            nw_wifi_esp_post_socket_data(pBuf);
        }          
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : It is run under UART ISR as callback 
*******************************************************************************/
#ifndef LINUX
static void nw_wifi_esp_uart_rx_completed(void *pThis, UINT32 value)
{
    U8* pCh = (U8*) pThis;

    static U8* pBuf = NULL;
    static U32 rx_len;
    static BOOLEAN is_data = FALSE;
    static BOOLEAN is_next_byte_data;
    static U32 data_len = 0x00;
    static U8* p;

    static U32 j = 0x00;
  

    if(pBuf == NULL)
    {
        is_data = FALSE;
        rx_len = 0x00;
        data_len = 0x00;
        pBuf=NW_WIFI_ESP_Rx_Buf;
		j = 0x00;
		p = 0x00;
    }

    *pBuf = *pCh;
    rx_len++;

    if( (data_prompt_flag == TRUE) && (rx_len == 2) && (*pBuf == 0x20) && (*(pBuf-1) == 0x3e) )
	{
		/* valid data prompt */
		goto END;
 	}

	if(rx_len >=2)
	{       
		/* cmd response comes with \r \n */
		if( ((*pBuf == '\n') && ((*(pBuf-1) == '\r')) ) && (is_data == FALSE) ) 
		{
			if( rx_len == 2)
			{
				pBuf = NULL;
				return;
			}
			else
			{   /* valid command response */
				goto END;
			}
		}
		if(rx_len >=4)
		{
			/* check for data arrival */
			if( (*pBuf == 0x44) &&
                (*(pBuf-1) == 0x50) &&
                (*(pBuf-2) == 0x49) && 
                (*(pBuf-3) == 0x2b)
               )
			{
				is_data = TRUE;
				is_next_byte_data = FALSE;                   
			}
		}
		if(is_data)
		{
			if( (*pBuf == 0x2c) && (!is_next_byte_data))
			{
				p = pBuf+1;
			}
			if( (*pBuf == 0x3a) && (!is_next_byte_data))
            {
				*pBuf = 0x00; 
				data_len = strtol((char *)p, NULL, 10);
				*pBuf = 0x3a;
				is_next_byte_data = TRUE;
				j=0;
			}
			if(is_next_byte_data)
            {
                j++;
                if(j == (data_len +1))
                {
					/* all data received */
					goto END;                      
                }
             }
          }
	}                      
	pBuf++;
	return;
END:
	pBuf++;
	*pBuf = 0x00;
	pSendBuf = (U8*)malloc(rx_len+1);
	memcpy(pSendBuf,(pBuf-rx_len),rx_len);
	pSendBuf[rx_len] = 0x00;
	pBuf = NULL;
	NW_WIFI_flag = 1;
	portYIELD_FROM_ISR( pdTRUE );
}
#endif

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 nw_wifi_esp_enable_sntp(S32 time_zone)
{
    U8 cmd[20];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;

    TRACE_DBG("%s() Entry \n", __FUNCTION__);

    memset(cmd,0x00,sizeof(cmd));
    sprintf(cmd,"AT+CIPSNTPCFG=%d,%d\r\n",1,time_zone);
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("SNTP on  \n");
            status = 1;
            goto END9;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("SNTP set Error\n");
                goto END9;
            }
        }
        free(pRxBuf);
    }
END9:
    free(pRxBuf);
    return status;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    :  
*                bit0�:-if set to be 1, certificate and private key will be 
*                       enabled, so SSL server can verify ESP8266; if 0,
*                       then will not.
*                bit1�:-if set to be 1, CA will be enabled, so ESP8266 can
*                      verify SSL server; if 0, then will not.
*******************************************************************************/
static U32 nw_wifi_esp_set_ssl_cert_choice(U32 choice)
{
    U8 cmd[20];
    U8* result = NULL;
    U8* pRxBuf = NULL;
    U32 status = 0;

    TRACE_DBG("%s() Entry \n",__FUNCTION__);

    memset(cmd,0x00,sizeof(cmd));
    sprintf(cmd,"AT+CIPSSLCCONF=%d\r\n",choice);
    TRACE_DBG("cmd(len:%d) = %s\n", strlen(cmd), cmd);
    nw_wifi_esp_tx(cmd, strlen(cmd));

    while(1)
    {
        nw_wifi_esp_get_next_output_from_console(&pRxBuf);

        result = strstr(pRxBuf,"OK");
        if(result)
        {
            TRACE_DBG("ssl cert option set \n");
            status = 1;
            goto END9;
        }
        else
        {
            result = strstr(pRxBuf,"ERROR");
            if(result)
            {
                TRACE_DBG("ssl cert option set not set\n");
                goto END9;
            }
        }
        free(pRxBuf);
    }
END9:
    free(pRxBuf);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
int nw_wifi_esp_ReceiveMqttTcpClient(char* buffer, int size, int timeout, S32 socket_id)
{

    static U8* pAckBuf = NULL;
    static U32 length_received;
    static U32 length_given_back = 0;
    U8* p;
    U8* q;
    static U8* s;
    static U32 index =0;

    U32 given_size;

    BOOLEAN to_continue = FALSE;

    U8* dst;

    U32 dst_buf_length ;

  //  TRACE_DBG("%s() Entry(%d) \n", __FUNCTION__,size);

    if(!pNwWifi->Sockets[socket_id].is_socket_open)
    {
        if(index !=0)
        {
            TRACE_INFO("Some previous BUFFER exit\n");
            free(pAckBuf);
        }
        return -1;
    }

    dst = buffer;

    dst_buf_length = size;

    if(index !=0)
    {
        //TRACE_DBG("The current index  = %d \n", index);
      
        if ( (length_received - length_given_back) > size )
        {
           // TRACE_DBG(">>>>> SIZE Requested(%d)\n", length_received - length_given_back);
            memcpy(buffer,&s[index],size);
            index +=size;
            given_size = size;
            length_given_back +=size;
        }
        else
        {
           // TRACE_DBG("<<<<<<<<<< SIZE Requested(%d)\n", length_received - length_given_back);
            memcpy(buffer,&s[index],length_received-length_given_back);
            index +=(length_received-length_given_back);
            given_size = length_received-length_given_back;
            length_given_back = length_received;
        }

        //TRACE_DBG("The updated  index  = %d \n", index);

        if (length_received == index )
        {
            index = 0;
            length_received = 0;
            length_given_back = 0;
            free(pAckBuf);
            s = NULL;
        //  TRACE_DBG("The whole data  is givenback \n");            
        }

        //TRACE_DBG("COPIED SIZE + %d \n",given_size);

        if(given_size == size)
        {
            return given_size;
        }
        else
        {
            dst = &buffer[given_size];

            length_given_back = given_size;

            dst_buf_length = size-given_size;
        }
    }
RX1:
  //  TRACE_DBG("%s() Entry(%d) \n", __FUNCTION__,size);

  //  TRACE_DBG("TIME OUT GIVEN TO WAIT = %d  \n",timeout);
#if 0 
    OSA_ReceiveMsgQue(pNwWifi->Sockets[socket_id].pRxSocDataQ, &pAckBuf);
#else
	if( OSA_MAX_DELAY_MS  == OSA_ReceiveMsgQueTimeout (pNwWifi->Sockets[socket_id].pRxSocDataQ, &pAckBuf,timeout))
	{
    //    TRACE_DBG("TIME OUT  \n");
        return 0;	
	}
#endif

    if(!pNwWifi->Sockets[socket_id].is_socket_open)
    {
        /* to handle abnormal close given by MQTT server */
        TRACE_DBG("Msg = %s \n",pAckBuf);
        free(pAckBuf);
        return -1;
    }
    TRACE_DBG("Data POP from Q \n");

    /* Check if it is valid  data packet or not */
    if(!strstr(pAckBuf,"+IPD,"))
    {
        /* case of first WIFI DISCONNECT message */
        TRACE_DBG("Poped String :%s\n",pAckBuf);
        free(pAckBuf);
        goto RX1;
    }
    //TRACE_DBG("POPED DATA = %s \n",pAckBuf); 

    /* Calculate the data length*/
    p = pAckBuf;    
    q = strchr(p,',');
    p = ++q;
    q = strchr(p,',');
    p = ++q;
    q = strchr(p,':');
    *q = 0x00;
    length_received = strtol((char *)p, NULL, 10);
    q++;
    TRACE_DBG("The length received = %d \n", length_received);

   // UTILITY_PrintBuffer(buffer , length_received, 1);

   if(dst_buf_length < length_received)
   {
       //TRACE_DBG("LESS space available size = %d  length_ received = % d\n", size, length_received);
       memcpy(dst,q,dst_buf_length);
       s = q;
       index +=dst_buf_length;
       length_given_back +=dst_buf_length;
       return dst_buf_length;
   }
   else
   {
       //   TRACE_DBG("Suffecient space available size = %d  length_ received = % d\n", size, length_received);
       /* suffecient space available in the given buffer */
       memcpy(dst,q,length_received);

       //   UTILITY_PrintBuffer(buffer , length_received, 0);

       index = 0;
       free(pAckBuf);
       s = NULL;

       return size;
   }
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void  nw_wifi_esp_post_socket_data(U8* pBuf)
{
    U8* p;
    U8* q;
    U32 socket_id;

    /* Calculate the data length*/
    p = pBuf;    
    q = strchr(p,',');
    p = ++q;
    q = strchr(p,',');
    *q = 0x00;  
    socket_id = strtol((char *)p, NULL, 10);
    *q = ',';

    OSA_SendMsgQue(pNwWifi->Sockets[socket_id].pRxSocDataQ,(void *)(pBuf));

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
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void  nw_wifi_esp_free_all_sockets()
{
    U32 i;

    for(i=0; i < NW_ESP_WIFI_MAX_SOCKETS; i++)
    {
        pNwWifi->Sockets[i].is_socket_open = FALSE; 
    }
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
