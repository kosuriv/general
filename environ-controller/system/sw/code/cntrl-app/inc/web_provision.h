/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : web_provision.h
*
*  Description         : This file contains web provisioning functionality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Linta Jacob  30th Nov 2021     1.1               Initial Creation
*  
*******************************************************************************/
#ifndef WEB_PROVISION_H
#define WEB_PROVISION_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <lwip/sockets.h>

#include "data_types.h"
#include "osa.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Macro Definitions
*******************************************************************************/

/*******************************************************************************
*                          Type Declarations
*******************************************************************************/
#define SERVER_LISTEN_LENGTH   10
#define WEB_PROVISION_DATA_BUFFER_SIZE  18000


typedef struct WEB_PROVISION
{
    U32      server_socket_id;

    U32      client_socket_id[10]; 

    U8       ip_addr[20];
        
    U32      port_no;

    osa_thread_handle web_provision_listen_thread_id;

    osa_thread_handle web_provision_rx_thread_id[10];

    U8  *data_buf;

}WEB_PROVISION;


/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/

S8 WEB_PROVISION_Init(WEB_PROVISION *pThis, U32 port_no);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/



