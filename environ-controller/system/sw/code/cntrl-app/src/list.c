/*******************************************************************************
* (C) Copyright 2016;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : list.c
*
*  Description         : It contains list of ant item functionality
*
*  Change history      : $Id$
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   29th Feb 2016    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "osa.h"
#include "qlist.h"

 /*****************************************************************************
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

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static osa_mutex_type_t list_Mutex;

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
void LIST_Init()
{    
   // pthread_mutex_init(&list_Mutex, NULL);
    OSA_InitMutex(&list_Mutex) ;  
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void LIST_DeInit()
{    
    OSA_DeInitMutex(&list_Mutex) ;  
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void LIST_Push(LIST** list, void* pArg)
{
    LIST* node;

    LIST* prev;

    LIST* front;

    OSA_LockMutex(&list_Mutex);

    //printf("LIST_PuSH BEGINNING() = %p\n",*list);
    
    node = malloc(sizeof(LIST));

    node->pArg = pArg ; 

    //printf("PUSHED = %p\n", pArg);

    //printf("MALLOCED  = %p\n", node);

    node->Next = NULL;

    if(*list)
    {
        front = *list;
        while(*list)
        {
            prev = *list;
            *list = (*list)->Next;
        }
        prev->Next = node; 
        *list = front;       
    }
    else
    {
        *list = node;
    } 
    //printf("LIST_PuSH()ENDDDDDDDDDDD = %p\n",*list);

    OSA_UnLockMutex(&list_Mutex);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void LIST_Pop(LIST** list, void** pArg)
{
    LIST* prev;

    OSA_LockMutex(&list_Mutex);

    //printf("LIST_Pop() = %p\n",*list);
 
    if(*list)
    {
        prev = *list ;

        *list = (*list)->Next;

        *pArg = prev->pArg;

        //printf("DEMALLOCED  = %p\n", prev);

        //printf("POPED = %p\n",prev->pArg); 

        free(prev);
    }

    OSA_UnLockMutex(&list_Mutex);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void LIST_Release(LIST** list)
{
    LIST* prev;

    OSA_LockMutex(&list_Mutex);

    //printf("LIST_Pop() = %p\n",*list);
 
    while(*list)
    {
        prev = *list ;

        *list = (*list)->Next;

        free(prev->pArg);

        //printf("DEMALLOCED  = %p\n", prev);

        //printf("POPED = %p\n",prev->pArg); 

        free(prev);
    } 

    *list = NULL;

    OSA_UnLockMutex(&list_Mutex);
}
                                           
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/


/*******************************************************************************
*                          End of File
*******************************************************************************/
