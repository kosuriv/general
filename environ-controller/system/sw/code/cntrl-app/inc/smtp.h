/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : smtp.h
*
*  Description         : This file contains SMTP protocol declarations
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  9th Apr 2021      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef SMTP_H
#define SMTP_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
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
/*  Macro to Maximum Host Name Length */
#define SMTP_MAX_HOST_NAME_LENGTH    128

/*  Macro to Maximum Account Information Length */
#define SMTP_MAX_INFO_LENGTH    64

/* Macro to Maximum Server IP Length */
#define SMTP_MAX_SERVER_IP_LENGTH    16

/* Macro to Seperate multiple email addrs*/
#define SMTP_MULTIPLE_MAIL_ADDRS_SEPERATOR      ','

/* SMTP Staus*/
typedef enum
{
    SMTP_STATUS_OK ,
    SMTP_STATUS_NETWORK_DOWN ,
    SMTP_STATUS_NO_SUCH_HOST ,     
    SMTP_STATUS_SOCKET_OPEN ,     
    SMTP_STATUS_FAIL_TO_CONNECT ,             
    SMTP_STATUS_MEM_ALLOCATION ,              
    SMTP_STATUS_FAIL_IN_TLS ,       
    SMTP_STATUS_FAIL_IN_AUTH ,      
    SMTP_STATUS_FAIL_TO_SEND_MAIL , 
    SMTP_STATUS_FAIL_TO_RECV       
} SMTP_Status;

/* smtp email encryption enum definition */
typedef enum
{
    SMTP_EMAIL_NO_ENCRYPTION,  
    SMTP_EMAIL_ENCRYPTION_TLS, 
} SMTP_email_encryption_t;

/* Smtp reply code enum definition */
typedef enum
{
    SMTP_SERVICE_READY    = 220,
    SMTP_AUTH_SUCCESSFUL  = 235,
    SMTP_ACTION_COMPLETED = 250,
    SMTP_MORE_INFO        = 334,
    SMTP_START_MAIL_INPUT = 354,
    SMTP_CLOSE = 221,
} SMTP_reply_code_t;

/* Email account data structure definition */
typedef struct
{
    U8*                    email_address;  /* Pointer to this account's email address  */
    U8*                    password;       /* Pointer to this account's password       */
    U8*                    host_name;      /* Pointer to SMTP server address string    */
    U32                    port;           /* SMTP server port                   */
    SMTP_email_encryption_t    smtp_encryption; /* Outgoing mail encryption   */
} SMTP_account_t;

/* Email data structure definition */
typedef struct
{    
    U8*    to_addresses;     /* Recipients' email addresses separated by commas */    
    U8*    cc_addresses;     /* Cc email addresses separated by commas          */
    U8*    bcc_addresses;    /* Bcc email addresses separated by commas         */
    U8*    subject;          /* Email subject                                   */
    U8*    content;          /* Email content/body                              */
    U8*    signature;        /* Signature content                               */
    U8     date[32];             /* Date string in email header format */  
} SMTP_email_t;


typedef struct SMTP
{    

} SMTP;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
SMTP_Status SMTP_SendEmail( SMTP_account_t* account, SMTP_email_t* email );

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
