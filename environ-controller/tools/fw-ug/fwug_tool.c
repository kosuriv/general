/*******************************************************************************
* (C) Copyright 2016;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : main.c
*
*  Description         : It contains firmware fucntionality to send fw bin file 
*                        on MQTT client . 
*
*  Change history      :
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   6th Jan 2021     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pthread.h"
#include "semaphore.h"
#include "base64.h"

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define MAX_PKT_SIZE  1024 //512


typedef enum APP_MQTT_CMD_TYPE
{
    APP_MQTT_GROW_CYCLE_INFO, //to start or stop the grow cycle
    APP_MQTT_PROFILE_INFO,    //to set new or update profile, or get the profile 
    APP_MQTT_STATUS_INFO,     //to send status info with all alerts info 
    APP_MQTT_GENERAL_INFO,    /* to set general info such as email/sms ph no ,
                                 wifi network credentials, mqtt credentials */
    APP_MQTT_FAULT_INFO,      // to send fault status 
    APP_MQTT_DEVICE_REQUEST , // used to put device in Forced , suspend states etc  
    APP_MQTT_FW_UG_REQUEST    // to update the FIRMWARE of EC 
}APP_MQTT_CMD_TYPE;

#if 0
#define U8 unsigned char    

#define S8 signed char    

#define U16  unsigned short 

#define S16  signed short   

#define U32  unsigned long   

#define S32    signed long   

#define  CO_BOOLEAN U8   

#define FLOAT float
#endif


typedef enum MODE
{
    APP_FILE_TEXT_MODE,
    APP_FILE_BIN_MODE
}FILE_MODE;

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void fw_ug_send_file(U8 *file_name, U32 mode);

static void fw_ug_print(U8* buf,U32 len, U8 mode);

static void fw_wait();

void* tx_thread_entry(void * arg);
void* rx_thread_entry(void * arg);


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
pthread_t id;

sem_t  ack_sem;

U32 seq_no = 0x00;

pthread_mutex_t fw_tx_mutex;

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
main(int argc , char*argv[])
{
    U32 i;
    U32 count=0;
    U8 file_name[100];
    time_t rawtime;
    U8 *p;

    printf("argv[1]= %s\n", argv[1]);
    strcpy(file_name,argv[1]);

#if 0
    /* create eBill dirctory , this directory stores all the sent 
      Bills/Tickets/Tokens etc  */
    sprintf(dir_name, "%s  %s\\eBill","mkdir" ,getenv("HOMEDRIVE"));
    printf("dir_name =%s\n",dir_name);
    system(dir_name);
#endif

    printf("file_name =%s\n",file_name);

    sem_init(&ack_sem,0,0);

    pthread_mutex_init(&fw_tx_mutex, NULL);

    pthread_create(&id,NULL,rx_thread_entry,file_name);
    pthread_create(&id,NULL,tx_thread_entry,file_name);

  

    while(1);

}
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

void* tx_thread_entry(void * arg)
{
    U8* file_name = (U8*)arg;

    printf( "In TX  Thread= %s \n",file_name);
    fw_ug_send_file(file_name, APP_FILE_BIN_MODE);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void* rx_thread_entry(void * arg)
{
    U8 pkt[2048];

    U8* rsp;

    FILE* fp;
    int i;
    U32 recv_seq_no;
    U32 pre_recv_seq_no=0;
    U8* p;
    U8* q;
    
    
    printf( "In Thread \n");

    memset(pkt , 0x00,sizeof(pkt));

    rsp = &pkt[0]; 
#if 0
    rsp += sprintf((char *)rsp,"%s", "mosquitto_sub -t  ");
    rsp += sprintf((char *)rsp,"\"%s\" -h ", "test");
    rsp += sprintf((char *)rsp,"\"%s\" -p 1883 -u ", "localveggy.com");
    rsp += sprintf((char *)rsp,"\"%s\" -P \"%s\" ", "pulsespectrum","welcome");
#else
    rsp += sprintf((char *)rsp,"%s", "mosquitto_sub -t ");
    rsp += sprintf((char *)rsp,"\"%s\" -h ", "test");
    rsp += sprintf((char *)rsp,"\"%s\" -p 1885 --cafile \"%s\" ", "localveggy.com","../../../environ-controller/system/qa/tls-cert/localveggy_ca_365.pem");
    rsp += sprintf((char *)rsp," --cert \"%s\" --key \"%s\" ", "../../../environ-controller/system/qa/tls-cert/generic_client_cert_300.pem","../../../environ-controller/system/qa/tls-cert/generic_client_key_300.pem");

#endif

    printf("THE PKT = %s\n",pkt);

    fp = popen(pkt,"r");

    while(!feof(fp))
    {
        memset(pkt , 0x00,sizeof(pkt));

        if(fgets(pkt,1200,fp) != NULL)
        {
            printf("RECEIVED = %s\n",&pkt[2]);

            if(strstr(pkt,"is_pkt_ok"))
            { 
                p=strstr(pkt,"seq_no");
            //  printf("p = %s \n",p);
                p +=strlen("seq_no");
                p +=2;
                q = strchr(p,'}');
            //  printf("q = %s \n",q);
                *q= 0x00;
                recv_seq_no = strtol((char *)p, NULL, 10);     
                // recv_seq_no = atoi(&pkt[189]);
                printf("seq_rec =%d\n", recv_seq_no);

                if(seq_no == recv_seq_no)
                {
                    if(recv_seq_no == pre_recv_seq_no)
                    {
                        printf("Packet received repeatedly\n");           
                    }
                    else
                    {
                        pre_recv_seq_no = recv_seq_no;
                        printf("Releasing Semaphore\n");           
                        sem_post(&ack_sem); 
                    }
                }
            }
        }
    }
    pclose(fp);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void fw_ug_send_file(U8 *file_name, U32 mode)
{
    FILE* pFile;
    U32   size;
    size_t result;
    U32  no_of_pkts;
    U8  pkt[2048 ];
    U8* rsp;
    U32 to_read_size;
    U32 next_len;
    U32 i;
    U32 k=0;
    U8* pAckBuf = NULL;
    U8* p;
   //U32 seq_no = 0x00;
    U8* q;
    U8* enc_src[MAX_PKT_SIZE];
    U32 enc_len;


   

    printf("%s(%s) Entry\n",__FUNCTION__,file_name);

    memset(pkt , 0x00,sizeof(pkt));

    /* Open file */
    if(mode == APP_FILE_TEXT_MODE)
    {
        pFile = fopen ( file_name , "r" );
    }
    else
    {
        pFile = fopen ( file_name , "rb" );
    }  
    if (pFile==NULL)
    {
        printf("File Not opened\n");
        exit(-1);
    }

    /* obtain file size */
    fseek (pFile , 0 , SEEK_END);
    size = ftell (pFile);
    printf("File Size: %d\n",size);
    
    /* obtain file name */
    p=file_name; 
    while(strchr(p,'\\'))
    {
        p++;
    }

START: 
    rewind (pFile); 

    /* First send HEADER packet  */
    rsp = &pkt[0]; 


    rsp += sprintf((char *)rsp,"%s\"", "mosquitto_pub -m  ");

    rsp +=sprintf((char *)rsp,"{\\\"%s\\\":\\\"%s\\\",", "type", "mqtt");
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":\\\"%s\\\",", "device_name", "EC_7c_df_a1_40_fd_88");
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "device_id", 1);
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":\\\"%s\\\",", "firm_name", "pulse_firm");
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "greenhouse_id", 1);
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "mobile_app_id", 1);
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":\\\"%s\\\",", "ver", "1.0.0");
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "flag", 0);
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":{\\\"%s\\\":%d,\\\"%s\\\":[", "req", "cmd",APP_MQTT_FW_UG_REQUEST,"fields");

    rsp +=sprintf((char *)rsp,"{\\\"%s\\\":\\\"%s\\\",", "file_name",p);
    
    rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "file_size",size);

    q = rsp;
  
#if  0   
i = 0;
#else

    for(i=0; i<size;)
#endif
    {
        k++;
        next_len = i + MAX_PKT_SIZE ;
        if (next_len > size)
        {
            next_len = size;
        }
        to_read_size = next_len - i;
        i = next_len;

        printf("To READ SIZE (k = %d) = %d \n",k,to_read_size);

        pthread_mutex_lock(&fw_tx_mutex);
        seq_no++;
        pthread_mutex_unlock(&fw_tx_mutex);

        rsp = q;

        rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "seq_no",seq_no);

        rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "chunk_size",to_read_size);

    enc_len = BASE64_EncLen(to_read_size);

        rsp +=sprintf((char *)rsp,"\\\"%s\\\":%d,", "pkt_enc_len",enc_len);

        rsp +=sprintf((char *)rsp,"\\\"%s\\\":", "pkt");

    rsp +=sprintf((char *)rsp,"\\\"");

        result = fread(enc_src,1,to_read_size,pFile);

        if (result != to_read_size)
        {
            printf("File Read Error = %d \n",result);
            return;
        }

    BASE64_Enc(rsp, enc_src, to_read_size);

    rsp += enc_len; 

    rsp +=sprintf((char *)rsp,"\\\"");      

       //if(i != size)
        {
            rsp +=sprintf((char *)rsp,"}]}}");

            // -t "test" -h "localveggy.com" -p 1883 -u "pulsespectrum" -P "welcome"
#if 0
            rsp += sprintf((char *)rsp,"\"%s", " -t ");
            rsp += sprintf((char *)rsp,"\"%s\" -h ", "test1");
            rsp += sprintf((char *)rsp,"\"%s\" -p 1883 -u ", "localveggy.com");
            rsp += sprintf((char *)rsp,"\"%s\" -P \"%s\" ", "pulsespectrum","welcome");
#else
            rsp += sprintf((char *)rsp,"\"%s", " -t ");
            rsp += sprintf((char *)rsp,"\"%s\" -h ", "test");
            rsp += sprintf((char *)rsp,"\"%s\" -p 1885 --cafile \"%s\" ", "localveggy.com","../../../environ-controller/system/qa/tls-cert/localveggy_ca_365.pem");
            rsp += sprintf((char *)rsp," --cert \"%s\" --key \"%s\" ", "../../../environ-controller/system/qa/tls-cert/generic_client_cert_300.pem","../../../environ-controller/system/qa/tls-cert/generic_client_key_300.pem");
#endif 
       }
        printf("TRANSMITTED: %s\n", pkt);

        printf("FILE PKT SEND = %d \n",seq_no);

       // fw_ug_print( pkt,rsp-pkt, 0x00);  
      // fw_ug_print( pkt,rsp-pkt, 0x01);  
    system(pkt); 

    printf("Waiting for  Semaphore\n");

    if(sem_wait(&ack_sem)  !=0)
    {
    }

    printf("Acquired Semaphore\n");


//  sleep(2);
 
    //usleep(500000);   
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void fw_ug_print(U8* buf,U32 len, U8 mode)
{
    int  i = 0x00 ;

    printf(" Buffer Length To Print = %d\n  ",len);

    for( i=0; i<len; i++)
    {
        if ( mode == 0x00 )
        {
            printf("%x ", buf[i]);
        }
        else
        {
             printf("%c ", buf[i]);
        }

        if( ((i+1) % 16) == 0x00)
        {
            printf("\n");
        }
    }
    printf("\n ");
}




/*******************************************************************************
*                          End of File
*******************************************************************************/

