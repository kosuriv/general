/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : smtp.c
*
*  Description         : It contains SMTP functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  9th Apr 2021      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "smtp.h"
#include "nw_wifi_esp.h"
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
#define SMTP_PORT 465  /* earlier it is to be 25 , offlate every mail server 
                         changed the port to 587 or 465 */

/* Size of Content Table */
#define SMTP_CONTENT_TABLE_SIZE 29

/* Macro to calculate size of buffer */
#define SMTP_GET_CONST_BUF_LENGTH(buf)   (sizeof(buf) - 1)

/* Maximum buffer size */
#define SMTP_MAX_BUFFER_SIZE 400


/* content table data structure definition */
typedef struct
{    
    U8* content;
    U16 length;
} smtp_email_content_table_field_t;


/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static U32 smtp_get_reply_code(U8* buf );

static SMTP_Status smtp_do_auth(S32 email_socket_id, U8* email_address, U8* password);

static SMTP_Status smtp_send_email(S32 email_socket_id, SMTP_account_t* account, SMTP_email_t* email);

static SMTP_Status smtp_send_rcpt_to_cmd(S32 email_socket_id , U8* addr );

static void smtp_init_content_table(const SMTP_account_t       *account,
                                    const SMTP_email_t               *email,
                                    smtp_email_content_table_field_t *content_table);


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/

/* Smtp commands and smtp headers */   
static const char const localhost[]                             = "PULSESPECTRUM";
static const char const smtp_helo[]                             = "HELO";
static const char const smtp_ehlo[]                             = "EHLO";
static const char const smtp_starttls[]                         = "STARTTLS";
static const char const smtp_auth_login[]                       = "AUTH LOGIN";
static const char const smtp_username_prompt[]                  = "Username:";
static const char const smtp_password_prompt[]                  = "Password:";
static const char const smtp_data[]                             = "DATA";
static const char const smtp_quit[]                             = "QUIT";
static const char const smtp_mail_from[]                        = "MAIL FROM: <";
static const char const smtp_rcpt_to[]                          = "RCPT TO: <";
static const char const smtp_return_newline[]                   = "\r\n";
static const char const smtp_end_of_rcpt_to[]                   = ">\r\n";
static const char const smtp_end_of_data[]                      = "\r\n.\r\n";
static const char const smtp_space[]                            = " ";
static const char const email_header_msg_id[]                   = "Message-ID: <>";
static const char const email_header_date[]                     = "Date: ";
static const char const email_header_from[]                     = "From: ";
static const char const email_header_mime_version[]             = "MIME-Version: 1.0";
static const char const email_header_to[]                       = "To: ";
static const char const email_header_cc[]                       = "Cc: ";
static const char const email_header_subject[]                  = "Subject: ";
static const char const email_header_content_type[]             = "Content-Type: text/plain; charset=\"utf-8\"";
static const char const email_header_content_tranfer_encoding[] = "Content-Transfer-Encoding: 8bit";


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
SMTP_Status SMTP_SendEmail( SMTP_account_t* account, SMTP_email_t* email)
{
    U8 tx_buf[128];

    U8 rx_buf[400];

    U8 enc_dec_buf[128];

    U32 size;

    U32 rx_len;

    U32 enc_len;

    SMTP_Status  status = SMTP_STATUS_OK;

    S32 email_socket_id;

    U8 local_ip[20];

    U32 reply_code;

    U8* p;
    U8* q;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* Email socket creation*/
    if(account->smtp_encryption == SMTP_EMAIL_ENCRYPTION_TLS)
    {
        NW_WIFI_ESP_OpenSocket(NW_WIFI_SSL_TCP_CLIENT,NW_ESP_WIFI_SOCKET_EMAIL,account->host_name, account->port,&email_socket_id);
    }
    else
    {
        NW_WIFI_ESP_OpenSocket(NW_WIFI_TCP_CLIENT,NW_ESP_WIFI_SOCKET_EMAIL,account->host_name, account->port,&email_socket_id);
    }
    NW_WIFI_ESP_GetLocalIpAddr(local_ip);


    /* Send EHLO */
    sprintf(tx_buf,"ehlo %s\r\n",local_ip);
    NW_WIFI_ESP_SendSocket(email_socket_id,tx_buf,strlen(tx_buf),0xFF);

    if(NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, SMTP_MAX_BUFFER_SIZE,0xFF) > 0)
    {

        TRACE_DBG("RECEIVED BUF = %s \n", rx_buf);        
        if(SMTP_SERVICE_READY != smtp_get_reply_code(rx_buf))
        {
            status = (S32)SMTP_STATUS_FAIL_TO_RECV;
            goto SMTP_EXIT;
        } 
    }
    TRACE_DBG("EHLO Sent\n");

    /* Do authentication */
    if(SMTP_STATUS_OK != smtp_do_auth(email_socket_id, account->email_address, account->password))
    {
        status = (S32)SMTP_STATUS_FAIL_IN_AUTH;
        goto SMTP_EXIT;
    }

    TRACE_DBG("Done AUTH \n");

    /* Send email */
    if(SMTP_STATUS_OK != smtp_send_email(email_socket_id, account, email))
    {
        status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
        goto SMTP_EXIT;
    }

    TRACE_DBG("EMAIL Sent \n");

    /*Send QUIT cmd*/
    sprintf(tx_buf,"QUIT \r\n");
    NW_WIFI_ESP_SendSocket(email_socket_id,tx_buf,strlen(tx_buf),0xFF);

    if(NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, SMTP_MAX_BUFFER_SIZE,0xFF) > 0)
    {
        if(SMTP_CLOSE != smtp_get_reply_code(rx_buf))
        {
            status = (S32)SMTP_STATUS_FAIL_TO_RECV;
            goto SMTP_EXIT;
        }
    }

    TRACE_DBG("QUIT Sent \n");
    
SMTP_EXIT:
    NW_WIFI_ESP_CloseSocket(email_socket_id);
    return status;
}

/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 smtp_get_reply_code(U8* buf )
{
    U8 error_code_str[4];

    memcpy( (void*) error_code_str, (void*) buf, 3 );

    error_code_str[3] = '\0';

    return atoi( (char *)error_code_str );
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static SMTP_Status smtp_do_auth(S32 email_socket_id, U8* email_address, U8* password)
{
    U8 tx_buf[150];
    U8 rx_buf[400];
    U8 enc_dec_buf[128];

    U32 rx_len;
    U32 size;
    U32 enc_len;

    U32 reply_code;

    SMTP_Status  status = SMTP_STATUS_OK ; 

    U8* p;


    /* Do Authentication */
    sprintf(tx_buf,"AUTH LOGIN \r\n");
    NW_WIFI_ESP_SendSocket(email_socket_id,tx_buf,strlen(tx_buf),0xFF);

    rx_len =NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, SMTP_MAX_BUFFER_SIZE,0xFF);
    if(rx_len  > 0)
    {
        reply_code  = smtp_get_reply_code(rx_buf);

        if( reply_code == SMTP_ACTION_COMPLETED)
        {
            /* do once again to receive  for  SMTP_MORE_INFO code 334 */ 
            rx_len = NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, SMTP_MAX_BUFFER_SIZE,0xFF);
            if(rx_len > 0)
            {
                reply_code  = smtp_get_reply_code(rx_buf);
                if(reply_code == SMTP_MORE_INFO)
                {
                    goto USERNAME;
                }                
            }    
        }
        else if(reply_code == SMTP_MORE_INFO)
        {
             goto USERNAME;
        } 
        else
        {
            status = (S32)SMTP_STATUS_FAIL_IN_AUTH;
            goto AUTH_EXIT;
            
        }
    }

USERNAME:

    p = strchr(rx_buf,' ');
    p++;
    BASE64_Dec(enc_dec_buf, p, rx_len-6);

    if(strstr((char *)enc_dec_buf, "Username:"))
    {
        /* Send user email id */
        size = strlen(email_address);
        enc_len = BASE64_EncLen(size);
        BASE64_Enc(enc_dec_buf, email_address, size);
        sprintf(&enc_dec_buf[enc_len],"\r\n");
        NW_WIFI_ESP_SendSocket(email_socket_id,enc_dec_buf,strlen(enc_dec_buf),0xFF);

        rx_len =NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, SMTP_MAX_BUFFER_SIZE,0xFF);
        if( rx_len > 0)
        {
            if(SMTP_MORE_INFO == smtp_get_reply_code(rx_buf))
            {
                p = strchr(rx_buf,' ');
                p++;
                BASE64_Dec(enc_dec_buf, p, rx_len-6);

                if(strstr((char *)enc_dec_buf, "Password:"))
                {
                     /* Send password  */
                    size = strlen(password);
                    enc_len = BASE64_EncLen(size);
                    BASE64_Enc(enc_dec_buf, password, size);
                    sprintf(&enc_dec_buf[enc_len],"\r\n");
                    NW_WIFI_ESP_SendSocket(email_socket_id,enc_dec_buf,strlen(enc_dec_buf),0xFF);

                    rx_len =NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, SMTP_MAX_BUFFER_SIZE,0xFF);
                    if( rx_len > 0)
                    {
                        if(SMTP_AUTH_SUCCESSFUL != smtp_get_reply_code(rx_buf))
                        {
                            status = (S32)SMTP_STATUS_FAIL_IN_AUTH;
                            goto AUTH_EXIT;
                        }
                    }
                } 
            }
            else
            {
                status = (S32)SMTP_STATUS_FAIL_IN_AUTH;
                goto AUTH_EXIT;
            } 
        }
    }

AUTH_EXIT:
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static SMTP_Status smtp_send_email(S32 email_socket_id, SMTP_account_t* account, SMTP_email_t* email)
{
    U8 tx_buf[128];

    U8 rx_buf[128];

    U32 size;

    U32 rx_len;

    U32 enc_len;

    SMTP_Status  status = SMTP_STATUS_OK;

   // S32 email_socket_id;

    U8 local_ip[20];

    U32 reply_code;

    U8* p;

    U8* q;

    U32 len;

    U8  *data_ptr;

    U32  field_index;

    U32  content_index;

    S32   available_space;

    smtp_email_content_table_field_t content_table[SMTP_CONTENT_TABLE_SIZE];

    U8* buf = NULL;

    U32 buf_size ;


    /* send MAIL FROM  cmd */
    sprintf(tx_buf,"MAIL FROM: <%s>\r\n",account->email_address);
    NW_WIFI_ESP_SendSocket(email_socket_id,tx_buf,strlen(tx_buf),0xFF);

    if(NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, sizeof(rx_buf),0xFF) > 0)
    {
        if(SMTP_ACTION_COMPLETED != smtp_get_reply_code(rx_buf))
        {
            status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
            goto SMTP_MAIL_EXIT;
        } 
    }

    TRACE_DBG("MAIL FROM Sent \n");

    /* send RCPT TO cmd for  TO field */
    if(email->to_addresses)
    {
        if(SMTP_STATUS_OK !=  smtp_send_rcpt_to_cmd(email_socket_id, email->to_addresses))
        {
            status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
            goto SMTP_MAIL_EXIT;
        }
    }

    TRACE_DBG("TO  Sent \n");

    /* send RCPT TO cmd for  CC field */
    if(email->cc_addresses)
    {
        if(SMTP_STATUS_OK !=  smtp_send_rcpt_to_cmd(email_socket_id, email->cc_addresses))
        {
            status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
            goto SMTP_MAIL_EXIT;
        }
    }

    TRACE_DBG("CC  Sent \n");

    /* send RCPT TO cmd for  BCC field  */
    if(email->bcc_addresses)
    {
        if(SMTP_STATUS_OK !=  smtp_send_rcpt_to_cmd(email_socket_id, email->bcc_addresses))
        {
            status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
            goto SMTP_MAIL_EXIT;
        }
    }

    TRACE_DBG("BCC  Sent \n");


    sprintf(tx_buf,"DATA\r\n");
    NW_WIFI_ESP_SendSocket(email_socket_id,tx_buf,strlen(tx_buf),0xFF);

    if(NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, sizeof(rx_buf),0xFF) > 0)
    {
        if(SMTP_START_MAIL_INPUT != smtp_get_reply_code(rx_buf))
        {
            status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
            goto SMTP_MAIL_EXIT;
        } 
    }

    TRACE_DBG("DATA  Sent \n");

     /* Fill packets with email content and send */      
    smtp_init_content_table(account, email, content_table);

    buf = malloc(1024);
    buf_size = 1024;
    
    memset (buf, 0x00, buf_size);    
    data_ptr = buf;
    available_space = buf_size ;

    for ( field_index = 0; field_index < SMTP_CONTENT_TABLE_SIZE; field_index++ )
    {

//if(content_table[field_index].content)
//printf("filed_index(%d) =  %s \n",field_index,content_table[field_index].content);

        for ( content_index = 0; content_index < content_table[field_index].length; content_index++ )
        {
            /* Fill content */
            *data_ptr = *((content_table[field_index].content)+content_index);
            available_space--;
            data_ptr++;

            /* Check if last bytes */
            if ( ( field_index == SMTP_CONTENT_TABLE_SIZE - 1 ) && ( content_index == content_table[field_index].length - 1 ) )                  
            {
                len = buf_size - available_space ;

                NW_WIFI_ESP_SendSocket(email_socket_id,buf,len,0xFF);

                if(NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, sizeof(rx_buf),0xFF) > 0)
                {
                    if(SMTP_ACTION_COMPLETED != smtp_get_reply_code(rx_buf))
                    {
                        status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
                        goto SMTP_MAIL_EXIT;
                    } 
                }
            }
            /* Check if payload full */
            else if ( available_space == 0 )
            {
                NW_WIFI_ESP_SendSocket(email_socket_id,buf,buf_size,0xFF);

                if(NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, sizeof(rx_buf),0xFF) > 0)
                {
                    if(SMTP_ACTION_COMPLETED != smtp_get_reply_code(rx_buf))
                    {
                        status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
                        goto SMTP_MAIL_EXIT;
                    } 
                }

                data_ptr = buf;
                available_space = buf_size-1 ;
            }
        }
    }    

SMTP_MAIL_EXIT:
    if(buf)
    {
        free(buf);
    }
    return status ;    

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static SMTP_Status smtp_send_rcpt_to_cmd(S32 email_socket_id , U8* addr )
{
    U8 tx_buf[128];
    U8 rx_buf[128];
    U8* p;
    U8* q;
    p = addr;
    SMTP_Status  status = SMTP_STATUS_OK;    
    do
    {            
        q = strchr(p,',' );

        if(q)
        {
            *q = 0x00;
        }
        sprintf(tx_buf,"RCPT TO: <%s>\r\n",p);
        NW_WIFI_ESP_SendSocket(email_socket_id,tx_buf,strlen(tx_buf),0xFF);

        if(NW_WIFI_ESP_ReceiveSocket(email_socket_id, rx_buf, 128,0xFF) > 0)
        {
            if(SMTP_ACTION_COMPLETED != smtp_get_reply_code(rx_buf))
            {
                status = (S32)SMTP_STATUS_FAIL_TO_SEND_MAIL;
                return status;
            } 
        }
        if(q)
        {
            *q = 0x00;
             p = ++q;
        }       
    } while(q); 
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void smtp_init_content_table(const SMTP_account_t             *account,
                                    const SMTP_email_t               *email,
                                    smtp_email_content_table_field_t *content_table)
{
    U32 index = 0;

    // Message-ID
    content_table[index].content   = (uint8_t*)email_header_msg_id;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_msg_id);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);

    // Date
    content_table[index].content   = (uint8_t*)email_header_date;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_date);
    content_table[index].content   = (uint8_t*)&email->date;
    content_table[index++].length  = strlen((char *)email->date);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);

    // From
    content_table[index].content   = (uint8_t*)email_header_from;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_from);
    content_table[index].content   = (uint8_t*)account->email_address;
    content_table[index++].length  = strlen( (char *)account->email_address );
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);

    // MIME-Version
    content_table[index].content   = (uint8_t*)email_header_mime_version;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_mime_version);
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);

    // To
    content_table[index].content   = (uint8_t*)email_header_to;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_to);
    content_table[index].content   = (uint8_t*)email->to_addresses;
    content_table[index++].length  = strlen( (char *)email->to_addresses );
    content_table[index].content   = (uint8_t*)smtp_return_newline;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);

    // Cc
    if(email->cc_addresses != NULL){
        content_table[index].content   = (uint8_t*)email_header_cc;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_cc);
        content_table[index].content   = (uint8_t*)email->cc_addresses;
        content_table[index++].length  = strlen( (char *)email->cc_addresses );
        content_table[index].content   = (uint8_t*)smtp_return_newline;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);
    }
    else{
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
    }

    // Subject
    if(email->subject != NULL){
        content_table[index].content   = (uint8_t*)email_header_subject;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_subject);
        content_table[index].content = (uint8_t*)email->subject;
        content_table[index++].length  = strlen( (char *)email->subject );
        content_table[index].content = (uint8_t*)smtp_return_newline;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);
    }
    else
    {
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
    }

    // Content Type
    if(email->content != NULL){
        content_table[index].content   = (uint8_t*)email_header_content_type;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_content_type);
        content_table[index].content   = (uint8_t*)smtp_return_newline;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);

    // Content Transfer Encoding
        content_table[index].content   = (uint8_t*)email_header_content_tranfer_encoding;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(email_header_content_tranfer_encoding);
        content_table[index].content   = (uint8_t*)smtp_return_newline;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);
        content_table[index].content   = (uint8_t*)smtp_return_newline;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);

    // Email body

        content_table[index].content   = (uint8_t*)email->content;
        content_table[index++].length  = strlen( (char *)email->content );
        content_table[index].content   = (uint8_t*)smtp_return_newline;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);
    }
    else
    {
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;

        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;

        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
    }
    // Email signature
    if(email->signature != NULL){
        content_table[index].content   = (uint8_t*)smtp_return_newline;
        content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_return_newline);
        content_table[index].content   = (uint8_t*)email->signature;
        content_table[index++].length  = strlen( (char *)email->signature );;

    }
    else
    {
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
        content_table[index].content   = NULL;
        content_table[index++].length  = 0;
    }
    // End of data
    content_table[index].content   = (uint8_t*)smtp_end_of_data;
    content_table[index++].length  = SMTP_GET_CONST_BUF_LENGTH(smtp_end_of_data);

   // printf( "INDEX VALUE = %d \n", index);
}

/*******************************************************************************
*                          End of File
*******************************************************************************/


