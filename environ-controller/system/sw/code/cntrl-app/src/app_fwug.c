/*******************************************************************************
* (C) Copyright 2021;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : app_fwug.c
*
*  Description         : It contains firmware upgrade functionality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   15th Jan 2021    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "stdlib.h"
#include "app_fwug.h"
#include "trace.h"
#include "app_setup.h"
#include "common.h"
#include "utility.h"

#ifdef ESP32_S2_MINI
#include "esp_ota_ops.h"
#endif

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define APP_FWUG_TIMER_PERIOD   60  // 1 minute

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void app_mqtt_send_fwug_ack(BOOLEAN is_ok, U32 exp_seg_no, U32 seq_no);

static void app_fwug_timer_handler(void *arg);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
#ifdef ESP32_S2_MINI
static esp_ota_handle_t  app_fwug_ota_handle = 0;
static esp_partition_t *app_fwug_partition = NULL;
#endif

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern APP App;

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/
extern void App_fwug_timer_handler(void *arg);

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_fwug_request(U8*        pBuf,
                                   jsmntok_t* pTok,
                                   U32        token_count)
{
    static U32 exp_seq_no = 0x00;

    static U32 total_size = 0x00;

    U32 seq_no;

    U8* pData;
    
    U8* pFile;

    U32 file_size;

    U32 chunk_size;

    U32 pkt_enc_len;

    U32 i;

    S32 err;    

    U8 file_name[100];

    U8* dec_buf;  

    BOOLEAN is_ok = FALSE;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    exp_seq_no++;

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            
            if (!memcmp((char*)(pBuf + pTok[i].start),"file_name",APP_COMPARE_LENGTH("file_name")))
            {
                i++;              
                pFile = (char *)(pBuf  + pTok[i].start);
                *(pFile + pTok[i].end - pTok[i].start) = '\0'; 
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"file_size",APP_COMPARE_LENGTH("file_size")))
            {
                i++;              
                file_size = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"seq_no",APP_COMPARE_LENGTH("seq_no")))
            {
                i++;              
                seq_no = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);
                if(seq_no == 0x1 )
                {
                    exp_seq_no = seq_no ;
                }
     
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"chunk_size",APP_COMPARE_LENGTH("chunk_size")))
            {
                i++;              
                chunk_size = strtol((char *)(pBuf  + pTok[i].start), NULL, 10); 

                total_size +=chunk_size;
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"pkt_enc_len",APP_COMPARE_LENGTH("pkt_enc_len")))
            {
                i++;              
                pkt_enc_len = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"pkt",APP_COMPARE_LENGTH("pkt")))
            {
                i++;              
                pData = (char *)(pBuf  + pTok[i].start);
                *(pData + pTok[i].end - pTok[i].start) = '\0';
            }
        }
    }

   // printf("file_name = %s file_size = %d , chunk_size = %d , enc_len = %d , seq_no = %d   exp_seq_no = %d\n", pFile, file_size, chunk_size , pkt_enc_len,seq_no,exp_seq_no );

    printf("exp_seq_no = %d  seq_no = %d\n",exp_seq_no, seq_no);
    if(exp_seq_no == seq_no )
    {
        is_ok = TRUE;

        TRACE_DBG("MATCHED\n");

#ifdef NUCLEO_F439ZI_EVK_BOARD
        sprintf(file_name,"%s/%s",COMMON_FWUG_DIR_NAME, pFile);
#endif

        dec_buf = malloc(chunk_size +3);

        if(dec_buf)
        {            
            BASE64_Dec(dec_buf, pData, pkt_enc_len);            

            if(seq_no == 1)
            {
                total_size = chunk_size;               
                OSA_InitTimer(&App.fwug_timer, APP_FWUG_TIMER_PERIOD*1000, FALSE, App_fwug_timer_handler, NULL);
               
                OSA_StartTimer(&App.fwug_timer);              
            }
            else
            {               
                OSA_ResetTimer(&App.fwug_timer);
            }
#ifdef NUCLEO_F439ZI_EVK_BOARD 
            if(SD_CARD_is_sd_card_present())
            {
                if(seq_no == 1)
                {
                    SD_CARD_delete_dir(COMMON_FWUG_DIR_NAME); 
                }
                if(SD_CARD_is_file_present(file_name))
                {
                    SD_CARD_append_file(dec_buf, chunk_size, file_name);
                }
                else
                {
                    TRACE_INFO("file name = %s\n", file_name);
                    SD_CARD_create_file(dec_buf, chunk_size, file_name);
                }
            }
#endif

#ifdef ESP32_S2_MINI
            if(seq_no == 1)
            {             
                app_fwug_partition = esp_ota_get_next_update_partition(NULL);
                //TRACE_DBG("Writing to partition subtype %d\taddr = 0x%x\n",app_fwug_partition->subtype,app_fwug_partition->address);

                err = esp_ota_begin(app_fwug_partition, file_size, &app_fwug_ota_handle);
                //TRACE_DBG("esp_ota_begin() return err = %d\n",err);
            }
            err = esp_ota_write(app_fwug_ota_handle, (const void *)dec_buf, chunk_size);
           //TRACE_DBG("esp_ota_write() return err  = %x\n",err);
#endif
            free(dec_buf);

            if(total_size == file_size)
            {
                TRACE_INFO("FW Received Completely \n");
                total_size  = 0x00;
                exp_seq_no = 0x00;

                OSA_StopTimer(&App.fwug_timer);
                OSA_DeInitTimer(&App.fwug_timer);

#ifdef NUCLEO_F439ZI_EVK_BOARD 
                /* Reset the System so that Bootloader does FW copying to Program FLASH */
                //HAL_NVIC_SystemReset();
                App_Reset(RESET_FIRMWARE_UPGRADE);
#endif

#ifdef ESP32_S2_MINI
                err = esp_ota_end(app_fwug_ota_handle);
                //TRACE_DBG("esp_ota_end() return err = %d\t ESP_ERR_NOT_FOUND =%d\tESP_ERR_INVALID_ARG =%d\tESP_ERR_OTA_VALIDATE_FAILED =%d\n",ret,ESP_ERR_NOT_FOUND,ESP_ERR_INVALID_ARG,ESP_ERR_OTA_VALIDATE_FAILED);
                err = esp_ota_set_boot_partition(app_fwug_partition);                
                //TRACE_DBG(" esp_ota_set_boot_partition() return err = %d\n",err);

                App_Reset(RESET_FIRMWARE_UPGRADE);
               // esp_restart();
#endif
            }
            else
            {
                exp_seq_no = seq_no;
            }
        }
        else
        {
            TRACE_WARN("%s() Malloc Failed \n",__FUNCTION__);
        }
    }
    else
    {
        TRACE_WARN("%s() SOME ERROR IN FWUG PKT \n",__FUNCTION__); 
#ifdef NUCLEO_F439ZI_EVK_BOARD  
        SD_CARD_delete_dir(COMMON_FWUG_DIR_NAME);   
#endif

#ifdef ESP32_S2_MINI
        err = esp_ota_abort(&app_fwug_ota_handle);
        TRACE_DBG("esp_ota_abort() return err  = %x\n",err);
#endif
        is_ok = FALSE;
        exp_seq_no = 0x00;
        total_size = 0x00;
    }
    /* send ACK  */
    app_mqtt_send_fwug_ack(is_ok,exp_seq_no, seq_no);

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_process_fwug_timer_expiry()
{

    int err;
    TRACE_INFO("%s Entry \n", __FUNCTION__);

#ifdef NUCLEO_F439ZI_EVK_BOARD 
    SD_CARD_delete_dir(COMMON_FWUG_DIR_NAME);
#endif
 
#ifdef ESP32_S2_MINI
    err = esp_ota_abort(&app_fwug_ota_handle);
    TRACE_DBG("esp_ota_abort() return err  = %x\n",err);
#endif

    OSA_DeInitTimer(&App.fwug_timer);

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}
                           
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_mqtt_send_fwug_ack(BOOLEAN is_ok, U32 exp_seg_no, U32 seq_no)
{
    U8* buf = NULL;

    U8* rsp;
   
    U32 i;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

 //   buf = (U8*)malloc(APP_MAX_SIZE_PKT ); // to save thread stack space , take it from heap

    buf = (U8*)malloc(300 ); // to save thread stack space , take it from heap

    if(buf)
    {

        rsp = buf;

        /* fill common  json */
        rsp += App_fill_common_json_header(rsp, APP_MQTT_FW_UG_REQUEST);

        rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "is_pkt_ok", is_ok);

        if(!is_ok)
        {
            rsp +=sprintf((char *)rsp,"{\"%s\":%d,", "exp_seg_no", exp_seg_no); 
        }

        rsp +=sprintf((char *)rsp,"\"%s\":%d}]}}", "seq_no", seq_no);

        //UTILITY_PrintBuffer( buf,5, 0);

        MQTT_COM_SendMessage(buf, rsp-buf);

        free(buf);
    }
    else
    {
        TRACE_WARN("%s() MALLOC ERROR \n",__FUNCTION__);
    }
}


/*******************************************************************************
*                          End of File
*******************************************************************************/
