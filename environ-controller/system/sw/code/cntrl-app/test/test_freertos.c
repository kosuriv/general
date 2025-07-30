/*******************************************************************************
* (C) Copyright 2019;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : test_freertos.c
*
*  Description         : This file is stub file to test freertos functionality 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  23rd July 2020    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"


#include "data_types.h"

#include "osa.h"
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
#define TEST_FREERTOS_FIRST_THREAD_STACK_SIZE  200

#define TEST_FREERTOS_FIRST_THREAD_PRIORITY   6

#define TEST_FREERTOS_SECOND_THREAD_STACK_SIZE  200

#define TEST_FREERTOS_SECOND_THREAD_PRIORITY    5

#define TEST_FREERTOS_MAILBOX_SIZE  5  //no of messages 


/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void test_freertos_Thread1Entry(void* arg );

static void test_freertos_Thread2Entry(void* arg );

static void test_timer_handler(void *arg);


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
QueueHandle_t   MailBox1;

UCHAR  msg[50];


osa_timer_type_t test_timer = {0};



struct AMessage
{
   char ucMessageID;
   char ucData[ 20 ];
} xMessage;

/* Queue used to send and receive complete struct AMessage structures. */
QueueHandle_t xStructQueue = NULL;

/* Queue used to send and receive pointers to struct AMessage structures. */
QueueHandle_t xPointerQueue = NULL;



/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/



/*******************************************************************************
*                          Extern Function Definitions
**************************************;*****************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_FREERTOS_Timer()
{


 
    OSA_InitTimer(&test_timer, 1*1000, FALSE, test_timer_handler, NULL);

    OSA_StartTimer(&test_timer);

}

void TEST_FREERTOS_Timer2()
{
    TRACE_INFO("%s Entry \n", __FUNCTION__);

     OSA_StopTimer(&test_timer);

    OSA_DeInitTimer(&test_timer);


    OSA_InitTimer(&test_timer, 1*1000, FALSE, test_timer_handler, NULL);

    OSA_StartTimer(&test_timer);
}





/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_FREERTOS_Init()
{
    xTaskHandle FirstThreadId;

    xTaskHandle SecondThreadId;

    signed portBASE_TYPE res;

#if 0
    MailBox1 = xQueueCreate(TEST_FREERTOS_MAILBOX_SIZE, sizeof(void *));

    if (MailBox1 == NULL)
    {
       printf("MailBox NOT  Created\n");   
    }
    else
    {
       printf("MailBox Created\n");
    }



    U32 *p;

    p = (U32*)malloc(4);

    *p = 3456;

#endif

#if 1

    res = xTaskCreate((pdTASK_CODE)test_freertos_Thread1Entry,
                                           (signed char const *)"First Thread",
                                            TEST_FREERTOS_FIRST_THREAD_STACK_SIZE,
                                            NULL,
                                            TEST_FREERTOS_FIRST_THREAD_PRIORITY,
                                            &FirstThreadId);
#endif

#if 0

    res = xTaskCreate((pdTASK_CODE)test_freertos_Thread2Entry,
                                           (signed char const *)"Second Thread",
                                            TEST_FREERTOS_SECOND_THREAD_STACK_SIZE,
                                            NULL,
                                            TEST_FREERTOS_SECOND_THREAD_PRIORITY,
                                            &SecondThreadId);

    if(res == pdPASS)
    { 
        printf(" Thread Creation Done\n");
    }
    else
    {
        printf(" Thread Creation NOT  Done\n");
    } 




 /* Create the queue used to send complete struct AMessage structures.  This can
   also be created after the schedule starts, but care must be task to ensure
   nothing uses the queue until after it has been created. */
   xStructQueue = xQueueCreate(
                         /* The number of items the queue can hold. */
                         10,
                         /* Size of each item is big enough to hold the
                         whole structure. */
                         sizeof( xMessage ) );

   /* Create the queue used to send pointers to struct AMessage structures. */
   xPointerQueue = xQueueCreate(
                         /* The number of items the queue can hold. */
                         10,
                         /* Size of each item is big enough to hold only a
                         pointer. */
                         sizeof( &xMessage ) );

   if( ( xStructQueue == NULL ) || ( xPointerQueue == NULL ) )
   {
      /* One or more queues were not created successfully as there was not enough
      heap memory available.  Handle the error here.  Queues can also be created
      statically. */
   }

#endif
    
}


/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/
U8 BUF[1000];


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_freertos_Thread1Entry(void* arg )
{
    struct AMessage xRxedStructure, *pxRxedPointer;

    printf("FIRST THREAD ENTRY \n");

    memset(BUF,0x25,1000);

  //  BUF[998] ='A';

   // BUF[999] =0x00;

 //   printf("FIRST THREAD ENTRY  = %s \n",BUF);

while(1)
{
                 vTaskDelay(120000); 

        __HAL_GPIO_EXTI_GENERATE_SWIT(GPIO_PIN_13);
}

   // while(1);

  // while(1)
   {

    
#if 0    
   if( xStructQueue != NULL )
   {
      /* Receive a message from the created queue to hold complex struct AMessage
      structure.  Block for 10 ticks if a message is not immediately available.
      The value is read into a struct AMessage variable, so after calling
      xQueueReceive() xRxedStructure will hold a copy of xMessage. */
      if( xQueueReceive( xStructQueue,
                         &( xRxedStructure ),
                         ( TickType_t ) 10 ) == pdPASS )
      {
         /* xRxedStructure now contains a copy of xMessage. */
      }
   }

#endif



        if( xPointerQueue != NULL )
        {
            /* Receive a message from the created queue to hold pointers.  Block for 10
            ticks if a message is not immediately available.  The value is read into a
            pointer variable, and as the value received is the address of the xMessage
            variable, after this call pxRxedPointer will point to xMessage. */
            if( xQueueReceive( xPointerQueue,
                         &( pxRxedPointer ),
                         ( TickType_t ) 10 ) == pdPASS )
            {
                /* *pxRxedPointer now points to xMessage. */

                printf("Received MessageId = %d \n", pxRxedPointer->ucMessageID);

                printf("Received Message = %s \n", pxRxedPointer->ucData);

            }
       }
         vTaskDelay(100);  

    }
}




/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_freertos_Thread2Entry(void* arg )
{
    
    struct AMessage *pxPointerToxMessage;

    printf("SECOND THREAD ENTRY \n");

while(1)
{
                 vTaskDelay(100);  
}


  //  while(1);

   // while(1)
    {
        xMessage.ucMessageID = 0xab;
        memset( &( xMessage.ucData ), 0x25, 20 );

#if 0
        /* Send the entire structure to the queue created to hold 10 structures. */
        xQueueSend( /* The handle of the queue. */
               xStructQueue,
               /* The address of the xMessage variable.  sizeof( struct AMessage )
               bytes are copied from here into the queue. */
               ( void * ) &xMessage,
               /* Block time of 0 says don't block if the queue is already full.
               Check the value returned by xQueueSend() to know if the message
               was sent to the queue successfully. */
               ( TickType_t ) 0 );

#endif


        /* Store the address of the xMessage variable in a pointer variable. */
         pxPointerToxMessage = &xMessage;

        /* Send the address of xMessage to the queue created to hold 10    pointers. */
        xQueueSend( /* The handle of the queue. */
               xPointerQueue,
               /* The address of the variable that holds the address of xMessage.
               sizeof( &xMessage ) bytes are copied from here into the queue. As the
               variable holds the address of xMessage it is the address of xMessage
               that is copied into the queue. */
               ( void * ) &pxPointerToxMessage,
               ( TickType_t ) 0 );
      
        vTaskDelay(100);
    }
}

extern  void APP_do_light_cycle_transition();
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void test_timer_handler(void *arg)
{

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    OSA_StopTimer(&test_timer);

    OSA_DeInitTimer(&test_timer);

    APP_do_light_cycle_transition();   
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
