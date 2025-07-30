/*******************************************************************************
* (C) Copyright 2022;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : sensor_app.c
*
*  Description         : This contains sensor  applictaion.
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  9th Sep 2022      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "sensor_app.h"
#include "trace.h"


/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define SENSOR_APP_ACTION_THREAD_PRIORITY 4

#define SENSOR_APP_ACTION_THREAD_STACK_SIZE  2000 

#define SENSOR_APP_MSGQ_NO_OF_MESSAGES  30


#define SENSOR_APP_LOCK_MSG_Q_MUTEX(p1,p2)      \
do {                                            \
    if (p1 == FALSE) {                          \
        OSA_LockMutex(p2);                      \
    }                                           \
} while (0);


#define SENSOR_APP_UNLOCK_MSG_Q_MUTEX(p1,p2)     \
do {                                             \
    if (p1 == FALSE) {                           \
        OSA_UnLockMutex(p2);                     \
    }                                            \
} while (0);

#define SENSOR_APP_MODBUS_SLAVE_ADDRESS  0x2a //make sure ec app also have the same flag

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void sensor_app_action_thread_entry_func(void *arg);

static void sensor_app_release_msg_req(Sensor_App_Msg_Req *pMsg );

static BOOLEAN  app_send_msg_req_to_q(U32 APP_MSG_TYPE, void* buf,U32 len,BOOLEAN is_isr_context);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
SENSOR_APP SensorApp;

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/
extern void TEST_SENSOR_ReadSensor();
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SENSOR_APP_Start()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    memset((U8*)&SensorApp, 0x00, sizeof(SENSOR_APP));

#if SENSOR2_SENSOR_AVAILABLE_FLAG
    SENSOR_Init(&SensorApp.Sensor);
#endif

#if 0   //make it 1 for unit testing of sensor 
    SENSOR_SetHeight(1);
#else
    MODBUS_Init(&SensorApp.ModbusSlave, SENSOR_APP_MODBUS_SLAVE_ADDRESS);
#endif


    /* Create APP rx thread. It contains all handlers for the received messages & profile change funtinality  */
    OSA_ThreadCreate(&SensorApp.action_thread,
                     SENSOR_APP_ACTION_THREAD_PRIORITY ,
                     sensor_app_action_thread_entry_func,
                     NULL,
                     "sensor_app_task",
                     NULL,
                     SENSOR_APP_ACTION_THREAD_STACK_SIZE,
                     0);

#if 0
    OSA_InitMsgQue(&SensorApp.pActionMsgQ,"sensor_app_action_msg_q", SENSOR_APP_MSGQ_NO_OF_MESSAGES, sizeof(void *));

    OSA_InitMutex(&SensorApp.msg_q_mutex) ;
#endif

}


/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void sensor_app_action_thread_entry_func(void *arg)
{
    Sensor_App_Msg_Req *pMsg = NULL;
    U16 co2; 
    S32 temperature;
    S32 humidity;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    while(1)
    {
#if 1

#if SENSOR2_SENSOR_AVAILABLE_FLAG 
        SENSOR_Read(&co2, &temperature, &humidity);
        SensorApp.ModbusSlave.sensor_value[0]= co2;
        SensorApp.ModbusSlave.sensor_value[1]= temperature;
        SensorApp.ModbusSlave.sensor_value[2]= humidity;
#endif
        TRACE_INFO("Sensor temp : %i   CO2 : %i  Humidity : %i \n", SensorApp.ModbusSlave.sensor_value[1], SensorApp.ModbusSlave.sensor_value[0] ,SensorApp.ModbusSlave.sensor_value[2]);

        OSA_Sleep(5);
#else
        TRACE_DBG( "WAIT ON ACTION MSG Q\n");

        /* Blocking call till time out  */
        OSA_ReceiveMsgQue(SensorApp.pActionMsgQ, &pMsg);


        TRACE_DBG("ACTION THREAD MESSAGE = %d\n",pMsg->type);

        switch(pMsg->type)
        {
            case SENSOR_APP_GET_SENSOR_MSG:
                SENSOR_Send();
                break;

            default:
                TRACE_WARN("Unknown Msg \n");
                break;
        }
        sensor_app_release_msg_req(pMsg);

#endif
    }

}



/*******************************************************************************
* Name       :
* Description:
* Remarks    : This always posts into Actin msg Q 
*******************************************************************************/
static BOOLEAN  app_send_msg_req_to_q(U32 APP_MSG_TYPE, void* buf,U32 len,BOOLEAN is_isr_context)
{
    Sensor_App_Msg_Req *msg;

    SENSOR_APP_LOCK_MSG_Q_MUTEX(is_isr_context,&SensorApp.msg_q_mutex);

    msg  = (Sensor_App_Msg_Req *) malloc(sizeof(Sensor_App_Msg_Req));

    if(msg)
    {
       // TRACE_INFO("%s Entry \n", __FUNCTION__);
        msg->buf = NULL;
        msg->len = 0;

        if(len != 0)
        {
            msg->buf = malloc(len);
            if(msg->buf)
            {
                memcpy((U8*)msg->buf,(U8*)buf,len);
                msg->len = len;
            }
            else
            {
                free(msg);
                SENSOR_APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&SensorApp.msg_q_mutex);
                return STATUS_ERR; 
            }
        }
        msg->type = APP_MSG_TYPE;

        if(is_isr_context)
        {
           // TRACE_INFO("%s ISR Context \n", __FUNCTION__);
            if(!OSA_SendMsgQueIsr(SensorApp.pActionMsgQ,msg)) 
            {
                TRACE_WARN("Failed to post to Sensor App queue IN ISR Context\n");
                SENSOR_APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&SensorApp.msg_q_mutex);
                return STATUS_ERR;
            }
        }
        else
        {
           // TRACE_INFO("%s NON ISR Context \n", __FUNCTION__);
            if(!OSA_SendMsgQue(SensorApp.pActionMsgQ,msg)) 
            {
                TRACE_WARN("Failed to post to Sensor App queue\n");
                SENSOR_APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&SensorApp.msg_q_mutex);
                return STATUS_ERR;
            }
        }
        SENSOR_APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&SensorApp.msg_q_mutex); 
        return STATUS_OK;  
    }
    SENSOR_APP_UNLOCK_MSG_Q_MUTEX(is_isr_context,&SensorApp.msg_q_mutex);
    return STATUS_ERR;
}



/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void sensor_app_release_msg_req(Sensor_App_Msg_Req *pMsg )
{
    if(pMsg->buf)
    {
        free(pMsg->buf);
    }
    free(pMsg);
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
