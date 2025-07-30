/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : main.c
*
*  Description         : This file contains MQTT functionality for both
                         subscription & publishing
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  28th Aug 2020     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <memory.h>
#include "MQTTClient.h"

#include <stdio.h>
#include <signal.h>

#include <sys/time.h>

#include "data_types.h"
#include "mqtt_com.h"

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
static void ReceivedMessage(void* pArg, BUFFER8* pBuffer );


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static MQTT_COM  MqttCom;
volatile int toStop = 0;

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
void usage()
{
    printf("MQTT stdout subscriber\n");
    printf("Usage: stdoutsub topicname <options>, where options are:\n");
    printf("  --host <hostname> (default is localhost)\n");
    printf("  --port <port> (default is 1883)\n");
    printf("  --qos <qos> (default is 2)\n");
    printf("  --delimiter <delim> (default is \\n)\n");
    printf("  --clientid <clientid> (default is hostname+timestamp)\n");
    printf("  --username none\n");
    printf("  --password none\n");
    printf("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
    exit(-1);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void cfinish(int sig)
{
    signal(SIGINT, NULL);
    toStop = 1;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
struct opts_struct
{
    char* clientid;
    int nodelimiter;
    char* delimiter;
    enum QoS qos;
    char* username;
    char* password;
    char* host;
    int port;
    int showtopics;
} opts =
{
    (char*)"stdout-subscriber", 0, (char*)"\n", QOS2, "wdb", "welcome", (char*)"192.168.1.9", 1883, 0
};

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void getopts(int argc, char** argv)
{
    int count = 2;
    
    while (count < argc)
    {
        if (strcmp(argv[count], "--qos") == 0)
        {
            if (++count < argc)
            {
                if (strcmp(argv[count], "0") == 0)
                    opts.qos = QOS0;
                else if (strcmp(argv[count], "1") == 0)
                    opts.qos = QOS1;
                else if (strcmp(argv[count], "2") == 0)
                    opts.qos = QOS2;
                else
                    usage();
            }
            else
                usage();
        }
        else if (strcmp(argv[count], "--host") == 0)
        {
            if (++count < argc)
                opts.host = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--port") == 0)
        {
            if (++count < argc)
                opts.port = atoi(argv[count]);
            else
                usage();
        }
        else if (strcmp(argv[count], "--clientid") == 0)
        {
            if (++count < argc)
                opts.clientid = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--username") == 0)
        {
            if (++count < argc)
                opts.username = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--password") == 0)
        {
            if (++count < argc)
                opts.password = argv[count];
            else
                usage();
        }
        else if (strcmp(argv[count], "--delimiter") == 0)
        {
            if (++count < argc)
                opts.delimiter = argv[count];
            else
                opts.nodelimiter = 1;
        }
        else if (strcmp(argv[count], "--showtopics") == 0)
        {
            if (++count < argc)
            {
                if (strcmp(argv[count], "on") == 0)
                    opts.showtopics = 1;
                else if (strcmp(argv[count], "off") == 0)
                    opts.showtopics = 0;
                else
                    usage();
            }
            else
                usage();
        }
        count++;
    }
    
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
int main(int argc, char** argv)
{
    int rc = 0;
    unsigned char buf[100];
    unsigned char readbuf[100];
    
    if (argc < 2)
        usage();
    
    char* topic = argv[1];

    if (strchr(topic, '#') || strchr(topic, '+'))
        opts.showtopics = 1;
    if (opts.showtopics)
        printf("topic is %s\n", topic);

    getopts(argc, argv);    

    signal(SIGINT, cfinish);
    signal(SIGTERM, cfinish);

    MQTT_COM_Init(&MqttCom,
                  opts.clientid,
                  opts.host,
                  opts.port,
                  opts.username,
                  opts.password,
                  topic, 
		  QOS2,
                  ReceivedMessage,
                  NULL);

   while(1)
   {
	if(toStop)
	{
		//MQTT_COM_SendMessage("ACKNOWLEDGED", 12);
		toStop = 0;
	}
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
static void ReceivedMessage(void* pArg, BUFFER8* pBuffer )
{
    printf("SOME MESSAGE RECEIVED\n");

    if (opts.showtopics)
        printf("%.*s\t", pBuffer->size, pBuffer->pBuf);
    if (opts.nodelimiter)
        printf("%.*s", (int)pBuffer->size, (char*)pBuffer->pBuf);
    else
        printf("%.*s%s", (int)pBuffer->size, (char*)pBuffer->pBuf, opts.delimiter);

		toStop = 1;

}

/*******************************************************************************
*                          End of File
*******************************************************************************/
