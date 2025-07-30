/*******************************************************************************
* (C) Copyright 2015;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : osa_windows.h
*
*  Description         : It contains windows abstraction to OS calls
*
*  Change history      : $Id$
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   19th Feb 2016    1.1               Initial Creation
*  
*******************************************************************************/
#ifndef OSA_LINUX_H
#define OSA_LINUX_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

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

/* Thread related parameters */

/* Thread's handle */
typedef pthread_t  osa_thread_handle;

/* scheduling info (priority) */
typedef pthread_attr_t  osa_thread_priority;

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
typedef sem_t osa_semaphore_type_t;

/* Mutex related parameters */
typedef pthread_mutex_t osa_mutex_type_t;

/* msg queue related parameters */
typedef mqd_t osa_msgq_type_t;

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
