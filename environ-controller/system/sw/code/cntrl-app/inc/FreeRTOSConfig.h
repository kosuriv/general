/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : FreeRTOSConfig.h
*
*  Description         : This file defines configuration settings for freeRTOS
                         Kernel functionality.
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  22nd July 2020    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef FREERTOSCONFIG_H
#define FREERTOSCONFIG_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "board.h"

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html
 */

/*----------------------------------------------------------*/

#define configUSE_TIMERS                            ( 1 )
#define configTIMER_TASK_PRIORITY                   ( 9 )
#define configTIMER_QUEUE_LENGTH                    ( 10 )  //command Q length of timer service stack
#define configTIMER_TASK_STACK_DEPTH                1024   // in words ( ( unsigned short ) (1024 / sizeof( portSTACK_TYPE )) )

#define configUSE_PREEMPTION        1
#define configUSE_IDLE_HOOK         0
#define configUSE_TICK_HOOK         0
#define configCPU_CLOCK_HZ          BOARD_CPU_CLOCK_HZ 
#define configTICK_RATE_HZ          ( ( TickType_t ) BOARD_TICK_RATE_HZ )
#define configMAX_PRIORITIES        ( 10 )
#define configMINIMAL_STACK_SIZE    ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE       ( ( size_t ) ( 25 * 1024 ) )
#define configMAX_TASK_NAME_LEN     ( 20 )
#define configUSE_TRACE_FACILITY    0
#define configUSE_16_BIT_TICKS      0
#define configIDLE_SHOULD_YIELD     1

#ifndef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY                    ( 1 )
#endif 

#ifndef configUSE_MUTEXES
#define configUSE_MUTEXES                           ( 1 )
#endif

#define configUSE_COUNTING_SEMAPHORES               ( 1 )
#define configUSE_RECURSIVE_MUTEXES                 ( 1 )
#define configFREE_TASKS_IN_IDLE                    ( 0 )
#define configQUEUE_REGISTRY_SIZE                   ( 8 )


/* Co-routine definitions. */
#define configUSE_CO_ROUTINES       0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )


#ifdef __NVIC_PRIO_BITS
    /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
    #define configPRIO_BITS             __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS             4        /* 15 priority levels */
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0xf
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 1

#define configKERNEL_INTERRUPT_PRIORITY ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_pcTaskGetTaskName                   ( 1 )
#define INCLUDE_vTaskPrioritySet                    ( 1 )
#define INCLUDE_uxTaskPriorityGet                   ( 1 )
#define INCLUDE_vTaskDelete                         ( 1 )
#define INCLUDE_vTaskCleanUpResources               ( 0 )
#define INCLUDE_vTaskSuspend                        ( 1 )
#define INCLUDE_vTaskDelayUntil                     ( 1 )
#define INCLUDE_vTaskDelay                          ( 1 )
#define INCLUDE_xTaskGetCurrentThread               ( 1 )
#define INCLUDE_vTaskForceAwake                     ( 0 )
#define INCLUDE_vTaskGetStackInfo                   ( 1 )
#define INCLUDE_xTaskIsTaskFinished                 ( 1 )
#define INCLUDE_uxTaskGetStackHighWaterMark         ( 1 )



/* Check for stack overflows - requires defining vApplicationStackOverflowHook */
//#define configCHECK_FOR_STACK_OVERFLOW              ( 2 )

/* Run a handler if a malloc fails - vApplicationMallocFailedHook */
#define configUSE_MALLOC_FAILED_HOOK                ( 1 )

//#define configASSERT(x)   {  if( ( x ) == 0 ) printf("ASSERT from = %s at = %d", __FILE__, __LINE__ );}

#define configASSERT(x)   {  if( ( x ) == 0 ) printf("ASSERT from = %s at = %d", __FUNCTION__, __LINE__ );}

#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler  

#define pvPortMalloc  malloc

#define vPortFree   free

/* Below code is specific to NW  ESP module , as UART2 interrupt should not be blocked
   by FreeRTOS & still use freeRTOS API to post the received message to NW RX thread  */

extern volatile int NW_WIFI_flag;

extern void  NW_WIFI_ProcessRx();

#define traceTASK_SWITCHED_OUT() \
{\
  if( NW_WIFI_flag == 1 )\
  {\
    NW_WIFI_ProcessRx();\
    NW_WIFI_flag = 0;\
  }\
}


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
