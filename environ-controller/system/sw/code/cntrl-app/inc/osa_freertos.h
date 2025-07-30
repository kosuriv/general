/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : osa_freertos.h
*
*  Description         : This is main controller application code 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  24th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef OSA_FREERTOS_H
#define OSA_FREERTOS_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "board.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
#include "timers.h"


#include "data_types.h"
/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

/* Thread's handle */
typedef xTaskHandle  osa_thread_handle;

/* scheduling info (priority) */
typedef unsigned int  osa_thread_priority;

/* thread entry point function */
typedef void*   osa_thread_entry_func ; 

/* thread entry point argument */
typedef void*  osa_thread_arguement ; 

/* Thread Name */
typedef char* osa_thread_name;

/* Thread's stack area address  */
typedef unsigned char* osa_thread_stack; 

/* Thread's  stack size in bytes */
typedef unsigned int  osa_thread_stack_size ;

/* space to store Thread's data */
typedef void* osa_thread_storage_data;

/* Semaphore related parameters */
typedef xSemaphoreHandle osa_semaphore_type_t;

/* Mutex related parameters */
typedef xSemaphoreHandle osa_mutex_type_t;

/* msg queue related parameters */
typedef QueueHandle_t osa_msgq_type_t;

typedef portSTACK_TYPE      osa_thread_stack_type_t;

typedef portTickType        osa_tick_type_t;



/*  Macro to start OS scheduler, called after starting the main thread in system */
#define OSA_START_TASK_SCHEDULER_POST()     vTaskStartScheduler()

/*  Width of stack, in terms of bytes. */
#define OSA_THREAD_STACK_WIDTH    (sizeof(portSTACK_TYPE))


#define OSA_TICK_RATE_MS                    portTICK_RATE_MS
#define OSA_TICK_RATE_HZ                    configTICK_RATE_HZ
#define OSA_MAX_DELAY_MS                    portMAX_DELAY
#define OSA_NEVER_TIMEOUT                   OSA_MAX_DELAY_MS


typedef struct
{
    xTimerHandle   handle;
    Callback_1_Arg function;
    void*          arg;
} osa_timer_type_t;

typedef U32   osa_time_type_t;


/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
