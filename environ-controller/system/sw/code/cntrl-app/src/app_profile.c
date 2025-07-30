/*******************************************************************************
* (C) Copyright 2020;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : app_profile.c
*
*  Description         : It contains profile request set/get functionality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   22nd oct 2020    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "stdlib.h"
#include "app_profile.h"
#include "app.h"
#include "trace.h"
#include "app_setup.h"

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define APP_PROFILE_DEV_MAX_TOKEN 50 

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static U32 app_fill_json_grow_info(U8* in_rsp,GrowConfig_GrowCycleInfo* pInfo);

static U32 app_fill_json_dev_light_info(U8* in_rsp,GrowConfig_DevLightInfo* pInfo);

static U32 app_fill_json_dev_gen_info(U8* in_rsp, 
                                      GrowConfig_DevGenInfo* pInfo,
                                      DEVICE_TYPE  device);

static U32 app_fill_json_dev_vent_info(U8* in_rsp, GrowConfig_DevVentInfo*   pInfo);

static void app_apply_profile_action_timer_handler(void* Arg);


static void app_parse_json_grow_info(U8*   pBuf, 
                                     U32   len,
                                     GrowConfig_GrowCycleInfo* pInfo);


static void app_parse_json_dev_light_info(U8*   pBuf, 
                                          U32   len,
                                          GrowConfig_DevLightInfo* pInfo);

static void app_parse_json_dev_gen_info(U8*   pBuf, 
                                        U32   len,
                                        GrowConfig_DevGenInfo* pInfo,
									    DEVICE_TYPE  device );

static void app_parse_json_dev_vent_info(U8*   pBuf, 
                                          U32   len,
                                          GrowConfig_DevVentInfo* pInfo);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/


/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern APP App;

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_set_profile_request(U8*        pBuf,
                                          jsmntok_t* pTok,
                                          U32        token_count)
{
    BOOLEAN to_apply_immediately = FALSE;

    BOOLEAN to_save;

    U32 profile_no = 0;

    U8* pTime = NULL;

    U8* pDate = NULL; 

    U32 i;

    BOOLEAN is_date = FALSE;

    BOOLEAN is_time = FALSE;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
    memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"profile_no",APP_COMPARE_LENGTH("profile_no")))
            {
                i++;              
                profile_no = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);

                if(profile_no > CONFIG_USER_PROFILE_MAX_NO)
                {
                    TRACE_WARN("%s () USER PROFILE DOESN'T EXIST (%d  > %d) \n",__FUNCTION__,profile_no,CONFIG_USER_PROFILE_MAX_NO);
                    return;
                }

            }
			else if (!memcmp((char*)(pBuf + pTok[i].start),"apply_now",APP_COMPARE_LENGTH("apply_now")))
            {
                i++;              
                to_apply_immediately = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"apply_date",APP_COMPARE_LENGTH("apply_date")))
            {
                i++;              
                pDate = (char *)(pBuf  + pTok[i].start);
                *(pDate + pTok[i].end - pTok[i].start) = '\0';
                strcpy(App.apply_profile_date,pDate); 
                is_date = TRUE;
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"apply_time",APP_COMPARE_LENGTH("apply_time")))
            {
                i++;              
                pTime = (char *)(pBuf  + pTok[i].start);
                *(pTime + pTok[i].end - pTok[i].start) = '\0'; 
                strcpy(App.apply_profile_time,pTime);
                is_time = TRUE;
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"to_save",APP_COMPARE_LENGTH("to_save")))
            {
                i++;              
                to_save = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
        }
    }
	if(!to_apply_immediately)
	{
    	to_apply_immediately = App_check_to_apply_profile_now_or_not();
    }

    if( to_apply_immediately) 
    {
        App.pNewProfile = &App.WorkingProfile;
    }
    else
    {
	   TRACE_INFO("New Profile Allocation \n");
       App.pNewProfile =  (Config_Profile*)malloc(sizeof(Config_Profile));
    }

    memset(App.pNewProfile,0x00,sizeof(Config_Profile));
    CONFIG_ReadTag(0, CONFIG_USER_PROFILE,CONFIG_GROW_CYCLE_INFO_TAG, &App.pNewProfile->GrowInfo);

    if((!is_date) && (!is_time) && (!to_apply_immediately))
    {
        TRACE_DBG("No need to Apply the profile , it is only to SET \n");
    }
    else
    {    
        App.apply_profile = CONFIG_USER_PROFILE;
        App.apply_profile_no = profile_no;
    }

    /* Get all the fields of all grow cycle info, device config & sensors config */
 	for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
#if 0
            if (!memcmp((char*)(pBuf + pTok[i].start),"grow_info",APP_COMPARE_LENGTH("grow_info")))
            {
                i++;              
				app_parse_json_grow_info((char *)(pBuf  + pTok[i].start), 
                                          pTok[i].end - pTok[i].start,
                                          &App.pNewProfile->GrowInfo);
            }
#endif
            if (!memcmp((char*)(pBuf + pTok[i].start),"dev_light",APP_COMPARE_LENGTH("dev_light")))
            {
                i++; 
				app_parse_json_dev_light_info((char *)(pBuf  + pTok[i].start), 
                                              pTok[i].end - pTok[i].start,
                                              &App.pNewProfile->DevLightConfig);
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_hood_vent",APP_COMPARE_LENGTH("dev_hood_vent")))
            {
                i++; 
				app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevHoodVentConfig,
										     DEVICE_HOOD_VENT);            
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_circ",APP_COMPARE_LENGTH("dev_circ")))
            {
                i++;
				app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevCircConfig,
                                             DEVICE_CIRC); 
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_vent",APP_COMPARE_LENGTH("dev_vent")))
            {
                i++;
				app_parse_json_dev_vent_info((char *)(pBuf  + pTok[i].start), 
                                              pTok[i].end - pTok[i].start,
                                              &App.pNewProfile->DevVentConfig); 
            }
			else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_fxp1",APP_COMPARE_LENGTH("dev_fxp1")))
            {
                i++;
				app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevFxp1Config,
                                             DEVICE_FXP1);               
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_fxp2",APP_COMPARE_LENGTH("dev_fxp2")))
            {
                i++;
				app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevFxp2Config,
											 DEVICE_FXP2); 
            }
        }
    }

    if(to_save)
    {
		TRACE_INFO("SAVING PROFILE(%d)\n",profile_no);

        /* store the profile  */
        CONFIG_WriteProfile(App.pNewProfile, profile_no);

        if((!is_date) && (!is_time) && (!to_apply_immediately))
        {

	   		TRACE_INFO("New Profile Allocated Getting Freed \n");
            /*only storing profile, hence free the buffer alloted */
            free(App.pNewProfile);
        }    
    }

    if((App.ec_state == APP_GROW) && (to_apply_immediately)) 
    {
		TRACE_INFO("APPLYING PROFILE NOW \n");
        /* profile needs to be applied immediately, apply */
        App.apply_new_profile = FALSE;
        App.pNewProfile = NULL;
        memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
        memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));
        App.apply_profile_st = 0x00;
        App.gc_profile = App.apply_profile ; 
        App.gc_profile_no = App.apply_profile_no;
        App.apply_profile_no = 0;
        App_apply_new_profile();
    }

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_apply_profile_request(U8*        pBuf,
                                           jsmntok_t* pTok,
                                           U32        token_count)
{
    BOOLEAN to_apply_immediately = FALSE;

    BOOLEAN to_read_user_profile = FALSE;

    U32 profile_no = 0;

    U8* pTime = NULL;

    U8* pDate = NULL; 

    U32 i;

    BOOLEAN is_date = FALSE;

    BOOLEAN is_time = FALSE;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
    memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"profile_no",APP_COMPARE_LENGTH("profile_no")))
            {
                i++;              
                profile_no = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);

                if(profile_no > CONFIG_USER_PROFILE_MAX_NO)
                {
                    TRACE_WARN("%s () USER PROFILE DOESN'T EXIST (%d  > %d) \n",__FUNCTION__,profile_no,CONFIG_USER_PROFILE_MAX_NO);
                    return;
                }
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"apply_now",APP_COMPARE_LENGTH("apply_now")))
            {
                i++;              
                to_apply_immediately = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"apply_date",APP_COMPARE_LENGTH("apply_date")))
            {
                i++;              
                pDate = (char *)(pBuf  + pTok[i].start);
                *(pDate + pTok[i].end - pTok[i].start) = '\0';
                strcpy(App.apply_profile_date,pDate); 
                is_date = TRUE;
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"apply_time",APP_COMPARE_LENGTH("apply_time")))
            {
                i++;              
                pTime = (char *)(pBuf  + pTok[i].start);
                *(pTime + pTok[i].end - pTok[i].start) = '\0';
                strcpy(App.apply_profile_time,pTime);
                is_time = TRUE;
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"user_profile",APP_COMPARE_LENGTH("user_profile")))
            {
                i++;              
                to_read_user_profile = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
        }
    }
    if(!to_apply_immediately)
    {
        to_apply_immediately = App_check_to_apply_profile_now_or_not();
    }

    if( to_apply_immediately) 
    {
        App.pNewProfile = &App.WorkingProfile;
    }
    else
    {
	   TRACE_INFO("New Profile Allocation \n");
       App.pNewProfile =  (Config_Profile*)malloc(sizeof(Config_Profile));
    }

    if(to_read_user_profile)
    {
	   	TRACE_INFO("Reading User Profile(%d) \n", profile_no);
        if(!CONFIG_ReadProfile(profile_no, CONFIG_USER_PROFILE, App.pNewProfile))
        {
            TRACE_WARN(" %s()  User Profile%d Not Present \n",__FUNCTION__, profile_no);
            return;
        }
        App.apply_profile = CONFIG_USER_PROFILE;
    }
    else
    {
	    TRACE_INFO("Reading Factory Profile(%d) \n", profile_no);
		CONFIG_ReadProfile(profile_no, CONFIG_FACTORY_PROFILE, App.pNewProfile);
        App.apply_profile = CONFIG_FACTORY_PROFILE;
    }
    App.apply_profile_no = profile_no;

    if((App.ec_state == APP_GROW) && (to_apply_immediately)) 
    {
		TRACE_INFO("APPLYING PROFILE NOW \n");
        /* profile needs to be applied immediately, apply */
        App.apply_new_profile = FALSE;
        App.pNewProfile = NULL;
        memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
        memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));
        App.apply_profile_st = 0x00;
        App.gc_profile = App.apply_profile ;
        App.gc_profile_no = App.apply_profile_no;
        App.apply_profile_no = 0;
        App_apply_new_profile();
    }

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_modify_profile_request(U8*        pBuf,
                                             jsmntok_t* pTok,
                                             U32        token_count)
{

    U32 profile_no;
    
    U32 i;

    BOOLEAN user_profile = FALSE;

    BOOLEAN to_save;

    U32 status;
    
    U8* buf;

    U8* rsp;


    TRACE_INFO("%s Entry \n", __FUNCTION__);


    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"profile_no",APP_COMPARE_LENGTH("profile_no")))
            {
                i++;              
                profile_no = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);

                if(profile_no > CONFIG_USER_PROFILE_MAX_NO)
                {
                    TRACE_WARN("%s () USER PROFILE DOESN'T EXIST (%d  > %d) \n",__FUNCTION__,profile_no,CONFIG_USER_PROFILE_MAX_NO);
                    return;
                }
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"user_profile",APP_COMPARE_LENGTH("user_profile")))
            {
                i++;              
                user_profile = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"to_save",APP_COMPARE_LENGTH("to_save")))
            {
                i++;              
                to_save = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);  
            }
        }
    }

    if(!user_profile)
    {
        TRACE_WARN("Factory profile selected to Modify \n");
        return;
    }

    if(( App.gc_profile == user_profile) && (App.gc_profile_no == profile_no))
    {
        //It is running profile in the grow cycle 
        App.pNewProfile = &App.WorkingProfile;
    }
    else
    {
        TRACE_INFO("New Profile Allocation \n");
        App.pNewProfile =  (Config_Profile*)malloc(sizeof(Config_Profile));
		memset(App.pNewProfile,0x00,sizeof(Config_Profile));
        if(!CONFIG_ReadProfile(profile_no,user_profile, App.pNewProfile))
        {
            TRACE_WARN(" %s()  User Profile%d Not Present \n",__FUNCTION__, profile_no);
            return;
        }
    }


    /* Get all the fields of all grow cycle info, device config & sensors config */
    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
#if 0
            if (!memcmp((char*)(pBuf + pTok[i].start),"grow_info",APP_COMPARE_LENGTH("grow_info")))
            {
                i++;              
                app_parse_json_grow_info((char *)(pBuf  + pTok[i].start), 
                                          pTok[i].end - pTok[i].start,
                                          &App.pNewProfile->GrowInfo);
                if(to_save)
                {   
                    status = CONFIG_WriteTag(profile_no, CONFIG_GROW_CYCLE_INFO_TAG, &App.pNewProfile->GrowInfo, sizeof(App.pNewProfile->GrowInfo));
                    if(status != STATUS_OK)
                    {
                        goto SAVE_FAILURE;
                    }
                }
            }
#endif
            if (!memcmp((char*)(pBuf + pTok[i].start),"dev_light",APP_COMPARE_LENGTH("dev_light")))
            {
                i++; 
                app_parse_json_dev_light_info((char *)(pBuf  + pTok[i].start), 
                                              pTok[i].end - pTok[i].start,
                                              &App.pNewProfile->DevLightConfig);
                if(to_save)
                {   
                    status = CONFIG_WriteTag(profile_no, CONFIG_DEV_LIGHT_INFO_TAG, &App.pNewProfile->DevLightConfig, sizeof(App.pNewProfile->DevLightConfig));
                    if(status != STATUS_OK)
                    {
                        goto SAVE_FAILURE;
                    }
                }
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_hood_vent",APP_COMPARE_LENGTH("dev_hood_vent")))
            {
                i++; 
                app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevHoodVentConfig,
                                             DEVICE_HOOD_VENT);  
                if(to_save)
                {   
                    status = CONFIG_WriteTag(profile_no, CONFIG_DEV_HOOD_VENT_INFO_TAG, &App.pNewProfile->DevHoodVentConfig, sizeof(App.pNewProfile->DevHoodVentConfig));
                    if(status != STATUS_OK)
                    {
                        goto SAVE_FAILURE;
                    }
                }          
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_circ",APP_COMPARE_LENGTH("dev_circ")))
            {
                i++;
                app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevCircConfig,
                                             DEVICE_CIRC); 
                if(to_save)
                {   
                    status = CONFIG_WriteTag(profile_no, CONFIG_DEV_CIRC_INFO_TAG, &App.pNewProfile->DevCircConfig, sizeof(App.pNewProfile->DevCircConfig));
                    if(status != STATUS_OK)
                    {
                        goto SAVE_FAILURE;
                    }
                }
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_vent",APP_COMPARE_LENGTH("dev_vent")))
            {
                i++;
                app_parse_json_dev_vent_info((char *)(pBuf  + pTok[i].start), 
                                              pTok[i].end - pTok[i].start,
                                              &App.pNewProfile->DevVentConfig); 
                if(to_save)
                {   
                    status = CONFIG_WriteTag(profile_no, CONFIG_DEV_VENT_INFO_TAG, &App.pNewProfile->DevVentConfig, sizeof(App.pNewProfile->DevVentConfig));
                    if(status != STATUS_OK)
                    {
                        goto SAVE_FAILURE;
                    }
                }
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_fxp1",APP_COMPARE_LENGTH("dev_fxp1")))
            {
                i++;
                app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevFxp1Config,
                                             DEVICE_FXP1); 
                if(to_save)
                {   
                    status = CONFIG_WriteTag(profile_no, CONFIG_DEV_FXP1_INFO_TAG, &App.pNewProfile->DevFxp1Config, sizeof(App.pNewProfile->DevFxp1Config));
                    if(status != STATUS_OK)
                    {
                        goto SAVE_FAILURE;
                    }
                }              
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"dev_fxp2",APP_COMPARE_LENGTH("dev_fxp2")))
            {
                i++;
                app_parse_json_dev_gen_info((char *)(pBuf  + pTok[i].start), 
                                             pTok[i].end - pTok[i].start,
                                             &App.pNewProfile->DevFxp2Config,
                                             DEVICE_FXP2); 
                if(to_save)
                {   
                    status = CONFIG_WriteTag(profile_no, CONFIG_DEV_FXP2_INFO_TAG, &App.pNewProfile->DevFxp2Config, sizeof(App.pNewProfile->DevFxp2Config));
                    if(status != STATUS_OK)
                    {
                        goto SAVE_FAILURE;
                    }
                }
            }
        }
    }

    memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
    memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));

    if((App.ec_state == APP_GROW) && (App.pNewProfile == &App.WorkingProfile)) 
    {
        TRACE_INFO("APPLYING MODIFIED PROFILE NOW \n");
        /* profile needs to be applied immediately, apply */
        App.apply_new_profile = FALSE;
        App.pNewProfile = NULL;
        memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
        memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));
        App.apply_profile_st = 0x00;
        App_apply_new_profile();
    }

SAVE_FAILURE:  //Send Modify response 
    if(App.pNewProfile != &App.WorkingProfile)
    {
        free(App.pNewProfile);
    }

    buf = (U8*)malloc(APP_MAX_SIZE_PKT ); // to save thread stack space , take it from heap

    rsp = buf;
    /* fill common  json */
    rsp += App_fill_common_json_header(rsp, APP_MQTT_PROFILE_INFO);

    /* fill action json */
    rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",\"%s\":%d ", "action", "modify","profile_no",profile_no);

    rsp +=sprintf((char *)rsp,",\"%s\":%d ", "user_profile", user_profile);   

    rsp +=sprintf((char *)rsp,",\"%s\":%d ", "saved_ok", status); 

    rsp +=sprintf((char *)rsp,"}"); 

    rsp +=sprintf((char *)rsp,"]}}");

    //TRACE_DBG("Config Pkt(%d) = %s\n",rsp-buf,buf);
    
    if((rsp-buf) > APP_MAX_SIZE_PKT)
    {
        TRACE_FATAL("CONFIG PKT SIZE(%d) more than  APP_MAX_SIZE_PKT(%d)\n",rsp-buf,APP_MAX_SIZE_PKT);
    }
    else
    {
        OSA_Sleep(1);  //MQTT  issue , work around ,to be checked TBD
        MQTT_COM_SendMessage(buf, rsp-buf);
    }
 
    free(buf);

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_get_profile_request(U8*        pBuf,
                                          jsmntok_t* pTok,
                                          U32        token_count)
{
    U8* buf;

    U8* rsp;

    BOOLEAN to_read_working_profile = FALSE;

    BOOLEAN to_read_user_profile = FALSE;

    U32 profile_no;
    
    U32 i;

    U32 size;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    buf = (U8*)malloc(APP_MAX_SIZE_PKT ); // to save thread stack space , take it from heap

    rsp = buf;

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"working_profile",APP_COMPARE_LENGTH("working_profile")))
            {
                i++;              
                to_read_working_profile = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);
            }
            else if (!memcmp((char*)(pBuf + pTok[i].start),"user_profile",APP_COMPARE_LENGTH("user_profile")))
            {
                i++;              
                to_read_user_profile = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);
            } 
            else if (!memcmp((char*)(pBuf + pTok[i].start),"profile_no",APP_COMPARE_LENGTH("profile_no")))
            {
                i++;              
                profile_no = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);
                if(profile_no > CONFIG_USER_PROFILE_MAX_NO)
                {
                    TRACE_WARN("%s () USER PROFILE DOESN'T EXIST (%d  > %d) \n",__FUNCTION__,profile_no,CONFIG_USER_PROFILE_MAX_NO);
                    return;
                }
            }            
        }
    }
    if( to_read_working_profile)
    {
        App.pNewProfile = &App.WorkingProfile;
        profile_no = App.gc_profile_no;
    }
    else
    {
        App.pNewProfile =  (Config_Profile*)malloc(sizeof(Config_Profile));

        if(to_read_user_profile)
        {
            if(!CONFIG_ReadProfile(profile_no, CONFIG_USER_PROFILE, App.pNewProfile))
            {
                TRACE_WARN(" %s()  User Profile%d Not Present \n",__FUNCTION__, profile_no);
                return;
            }
        }
        else
        {
            CONFIG_ReadProfile(profile_no, CONFIG_FACTORY_PROFILE, App.pNewProfile);
        }
    }


    /* fill common  json */
    rsp += App_fill_common_json_header(rsp, APP_MQTT_PROFILE_INFO);

    /* fill action json */
    rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",\"%s\":%d ", "action", "get","profile_no",profile_no);
    if(to_read_working_profile)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%d ", "user_profile", App.gc_profile); 
        rsp +=sprintf((char *)rsp,",\"%s\":%d ", "working_profile", to_read_working_profile); 
    }
    if(to_read_user_profile)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%d ", "user_profile", to_read_user_profile);   
    }
    rsp +=sprintf((char *)rsp,"}"); 

 //   rsp +=app_fill_json_grow_info(rsp,&App.pNewProfile->GrowInfo);

    rsp +=app_fill_json_dev_light_info(rsp,&App.pNewProfile->DevLightConfig);

    rsp +=app_fill_json_dev_gen_info(rsp,&App.pNewProfile->DevHoodVentConfig,DEVICE_HOOD_VENT);

    rsp +=app_fill_json_dev_gen_info(rsp,&App.pNewProfile->DevCircConfig,DEVICE_CIRC);

    rsp +=app_fill_json_dev_vent_info(rsp,&App.pNewProfile->DevVentConfig);

    rsp +=app_fill_json_dev_gen_info(rsp,&App.pNewProfile->DevFxp1Config,DEVICE_FXP1);

    rsp +=app_fill_json_dev_gen_info(rsp,&App.pNewProfile->DevFxp2Config,DEVICE_FXP2);

    rsp +=sprintf((char *)rsp,"]}}");

	//TRACE_DBG("Config Pkt(%d) = %s\n",rsp-buf,buf);
    
    if((rsp-buf) > APP_MAX_SIZE_PKT)
    {
        TRACE_FATAL("CONFIG PKT SIZE(%d) more than  APP_MAX_SIZE_PKT(%d)\n",rsp-buf,APP_MAX_SIZE_PKT);
    }
    else
    {
    	OSA_Sleep(1);  //MQTT  issue , work around ,to be checked TBD
    	MQTT_COM_SendMessage(buf, rsp-buf);
    }
 
    free(buf);

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}


/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN App_check_to_apply_profile_now_or_not()
{
    U8* p;

    U8* q;

    mDATE start_date;

    mTIME start_time;

    mDATE cur_date;

    mTIME cur_time;

    U8 cur_time_str[12];

    U8 cur_date_str[12];

    U32 st;

    U32 ct;

    S32 cmp_value;

    BOOLEAN status = FALSE;

    mTIME ref_time;

    U8* pDate = NULL;

    U8* pTime = NULL;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("apply_profile_date Len = %d\n", strlen(App.apply_profile_date));

    TRACE_DBG("apply_profile_time Len = %d\n", strlen(App.apply_profile_time));

    if(strlen(App.apply_profile_date) != 0)
    {
        pDate = App.apply_profile_date;
    }

    if(strlen(App.apply_profile_time) != 0)
    {
        pTime = App.apply_profile_time;
    }

    memset((U8*)&start_date,0x00,sizeof(start_date));

    memset((U8*)&start_time,0x00,sizeof(start_time));

    memset((U8*)&ref_time,0x00,sizeof(ref_time));

    RTC_GetDateTime(&cur_date, &cur_time , cur_date_str ,cur_time_str);

    UTILITY_convert_date_str(pDate, &start_date);

    UTILITY_convert_time_str(pTime, &start_time);

    GET_TIME_IN_SECOND(start_time,st) ;

    GET_TIME_IN_SECOND(cur_time,ct) ;

    TRACE_DBG("cur_date = %s , cur_time = %s\n",cur_date_str ,cur_time_str );
  //  TRACE_DBG("start_date = %s , start_time = %s\n",pDate ,pTime );

    TRACE_DBG("st = %d , ct = %d\n",st ,ct );

    /* Check whether the grow cycle to be started or not */
    if((!pDate) && (!pTime))
    {        
        TRACE_INFO("No Start Date & Time given, DONOT APPLY PROFILE \n");
    }
    else if (!pDate)
    {
        TRACE_INFO("No Start Date given , Only Time given, Apply it today \n");
        TRACE_DBG("start_time = %s\n",pTime );

        if(st <= ct)  
        {
            TRACE_INFO("Start time passed already , APPLY PROFILE IMMEDIATELY \n");
            status = TRUE;
        }
        else
        {
            TRACE_INFO("Start time yet to arrive , START PROFILE APPLY TIMER \n");
            OSA_InitTimer(&App.apply_profile_timer,(st-ct)*1000, FALSE, app_apply_profile_action_timer_handler, NULL);
            OSA_StartTimer(&App.apply_profile_timer);
            App.apply_profile_st = st;
        }

    }
    else if(!pTime)
    {
        TRACE_INFO("No Start Time given, Only Date given for the profile \n");

        TRACE_DBG("start_date = %s \n",pDate );

        cmp_value = UTILITY_date_cmp(&start_date,&cur_date);
        if( cmp_value == 0) 
        {
            TRACE_INFO("Start Date Matched & Time Not given,APPLY PROFILE IMMEDIATELY \n");
            status = TRUE;          
        }
        else if (cmp_value == -1)
        {
            TRACE_INFO("Start Date is yet to arrive  & Time Not given, WAIT FOR PROFILE ALARAM \n");
            App.apply_new_profile = TRUE;
        }
        else
        {
            TRACE_INFO("Start Date Passed  already  & Time Not given,APPLY PROFILE IMMEDIATELY\n");
            status = TRUE;           
        }        
    }
    else
    {
        TRACE_INFO("Start Date & Start Time given For the Profile \n");

        TRACE_DBG("start_date = %s , start_time = %s\n",pDate ,pTime );

        cmp_value = UTILITY_date_cmp(&start_date,&cur_date);

        if( cmp_value == 0)  
        {
            if(st <= ct)  
            {
                TRACE_INFO("Start time passed already, APPLY PROFILE IMMEDIATELY\n");
                status = TRUE;  
            }
            else
            {
                TRACE_INFO("Start time is yet to arrive(%d) , START PROFILE APPLY TIMER \n",st-ct);
                OSA_InitTimer(&App.apply_profile_timer,(st-ct)*1000,FALSE, app_apply_profile_action_timer_handler, NULL);
                OSA_StartTimer(&App.apply_profile_timer);
                App.apply_profile_st = st;
            }
        }
        else if (cmp_value == -1)
        {
            TRACE_INFO("Start Date is yet to arrive , WAIT FOR PROFILE ALARAM  \n");
            App.apply_new_profile = TRUE; 
        }
        else
        {
            TRACE_INFO("Start Date is passed already , APPLY PROFILE IMMEDIATELY \n");
            status = TRUE;  
        }
    } 
    return status;   
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void App_mqtt_process_erase_profile_request(U8*        pBuf,
                                           jsmntok_t* pTok,
                                           U32        token_count)
{ 
    U32 profile_no = 0;

    U32 i;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    for (i = 0; i < token_count; i++)
    {       
        if (((pTok[i].end) - (pTok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + pTok[i].start),"profile_no",APP_COMPARE_LENGTH("profile_no")))
            {
                i++;              
                profile_no = strtol((char *)(pBuf  + pTok[i].start), NULL, 10);

                if(profile_no > CONFIG_USER_PROFILE_MAX_NO)
                {
                    TRACE_WARN("%s () USER PROFILE DOESN'T EXIST (%d  > %d) \n",__FUNCTION__,profile_no,CONFIG_USER_PROFILE_MAX_NO);
                    return;
                }
            }
        }
    }

    CONFIG_EraseIndividualUserProfile(profile_no);

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
U32 App_fill_common_json_header(U8* in_rsp, APP_MQTT_CMD_TYPE cmd)
{
    U8* rsp ;

    rsp = in_rsp;

    rsp +=sprintf((char *)rsp,"{\"%s\":\"%s\",", "type", "mqtt");
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "device_name", App.DevConfig.device_name);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "device_id", App.DevConfig.device_id);
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "farm_name", App.DevConfig.firm_name);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "greenhouse_id", App.DevConfig.greenhouse_id);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "mobile_app_id", App.DevConfig.mobile_app_id);
    rsp +=sprintf((char *)rsp,"\"%s\":\"%s\",", "ver", APP_EC_PROTOCOL_VERSION);
    rsp +=sprintf((char *)rsp,"\"%s\":%d,", "flag", 0);
    rsp +=sprintf((char *)rsp,"\"%s\":{\"%s\":%d,\"%s\":[", "res", "cmd",cmd,"fields");

    return rsp - in_rsp;
}

                                           
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

#if 0
/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_grow_info(U8* in_rsp,GrowConfig_GrowCycleInfo*   pInfo)
{
    U8* rsp ;

    rsp = in_rsp;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "grow_info");

    rsp +=sprintf((char *)rsp,"\"%s\":%d", "rpt_lo_tmp",pInfo->to_report_low_temp_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "lo_tmp_chk_val",pInfo->low_temp_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_hi_tmp",pInfo->to_report_high_temp_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "hi_tmp_chk_val",pInfo->high_temp_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_critical_tmp",pInfo->to_report_critical_temp_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "critical_tmp_chk_val",pInfo->critical_temp_threshold_value);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_lo_hmdy",pInfo->to_report_low_humidity_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "lo_hmdy_chk_val",pInfo->low_humidity_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_hi_hmdy",pInfo->to_report_high_humidity_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "hi_hmdy_chk_val",pInfo->high_humidity_threshold_value);


    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_lo_co2",pInfo->to_report_low_co2_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "lo_co2_chk_val",pInfo->low_co2_threshold_value);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_hi_co2",pInfo->to_report_high_co2_threshold);
    rsp +=sprintf((char *)rsp,",\"%s\":%0.2f", "hi_co2_chk_val",pInfo->high_co2_threshold_value);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_battery_flt",pInfo->to_report_battery_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_frequency_flt",pInfo->to_report_ac_power_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_emr_fuse_flt",pInfo->to_report_emr_fuse_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_sensor_flt",pInfo->to_report_sensor_fault);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "rpt_rs485_fuse_flt",pInfo->to_report_rs485_fuse_fault);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "light_restrike_tmr",pInfo->is_light_restrike_timer_present);

    rsp +=sprintf((char *)rsp,",\"%s\":\"", "light_restrike_tmr_dur");
    rsp +=UTILITY_convert_time_duration_to_str(pInfo->light_restrike_timer_duration , rsp);

    rsp +=sprintf((char *)rsp,"\"}}");


    return rsp - in_rsp;
}
#endif

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_dev_light_info(U8* in_rsp, GrowConfig_DevLightInfo*   pInfo)
{
    U8* rsp ;

    rsp = in_rsp;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "dev_light");
    rsp +=sprintf((char *)rsp,"\"%s\":%d", "state",pInfo->DevState);
    rsp +=sprintf((char *)rsp,",\"%s\":%d","light_cycle",pInfo->LightCycle);
    rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","light_on_time",pInfo->Light_OnTime.HH,pInfo->Light_OnTime.MM,pInfo->Light_OnTime.SS);
    rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"}}","light_off_time",pInfo->Light_OffTime.HH,pInfo->Light_OffTime.MM,pInfo->Light_OffTime.SS);


    return rsp - in_rsp;
}


/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_dev_gen_info(U8* in_rsp,
                                      GrowConfig_DevGenInfo*   pInfo,
                                      DEVICE_TYPE  device)
{
    U8* rsp ;


    rsp = in_rsp;

    switch (device)
    {
        case DEVICE_HOOD_VENT:
            rsp +=sprintf((char *)rsp,",{\"%s\":{", "dev_hood_vent");
            break;

        case DEVICE_CIRC:
            rsp +=sprintf((char *)rsp,",{\"%s\":{", "dev_circ");
            break;

        case DEVICE_FXP1:
            rsp +=sprintf((char *)rsp,",{\"%s\":{", "dev_fxp1");
            break;

        case DEVICE_FXP2:
            rsp +=sprintf((char *)rsp,",{\"%s\":{", "dev_fxp2");
             break;
    }

    rsp +=sprintf((char *)rsp,"\"%s\":%d", "state",pInfo->DevState);
    rsp +=sprintf((char *)rsp,",\"%s\":%d", "op_mode",pInfo->DevOpState);

    if(pInfo->DevOpState == FLEXI)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%d", "flexi_mode",pInfo->DevOpMode);
        rsp +=sprintf((char *)rsp,",\"%s\":%d","vent_lockout",pInfo->is_vent_lockout);
        rsp +=sprintf((char *)rsp,",\"%s\":%d","lc_on",pInfo->is_lc_on);  
        rsp +=sprintf((char *)rsp,",\"%s\":%d","dc_on",pInfo->is_dc_on);   

        if(pInfo->DevOpMode == RPTTIMER)
        {
            rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","lc_rpt_tmr_start_delay",pInfo->lc_start_delay.HH,pInfo->lc_start_delay.MM,pInfo->lc_start_delay.SS);
            rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","lc_rpt_tmr_on_time",pInfo->lc_on_time.HH,pInfo->lc_on_time.MM,pInfo->lc_on_time.SS);          
            rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","lc_rpt_tmr_off_time",pInfo->lc_off_time.HH,pInfo->lc_off_time.MM,pInfo->lc_off_time.SS);

            rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","dc_rpt_tmr_start_delay",pInfo->dc_start_delay.HH,pInfo->dc_start_delay.MM,pInfo->dc_start_delay.SS);
            rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","dc_rpt_tmr_on_time",pInfo->dc_on_time.HH,pInfo->dc_on_time.MM,pInfo->dc_on_time.SS);          
            rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","dc_rpt_tmr_off_time",pInfo->dc_off_time.HH,pInfo->dc_off_time.MM,pInfo->dc_off_time.SS);
      
        }
        else if((pInfo->DevOpMode == HEATER) || (pInfo->DevOpMode == COOLER)) 
        {
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_tmp_up_val",pInfo->lc_upper_threshold); 
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_tmp_lo_val",pInfo->lc_lower_threshold);  
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_tmp_up_val",pInfo->dc_upper_threshold); 
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_tmp_lo_val",pInfo->dc_lower_threshold);      
        }
        else if((pInfo->DevOpMode == HUMIDIFIER) || (pInfo->DevOpMode == DEHUMIDIFIER)) 
        {
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_hmdy_up_val",pInfo->lc_upper_threshold); 
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_hmdy_lo_val",pInfo->lc_lower_threshold);  
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_hmdy_up_val",pInfo->dc_upper_threshold); 
            rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_hmdy_lo_val",pInfo->dc_lower_threshold);      
        }
        else  
        {
            rsp +=sprintf((char *)rsp,",\"%s\":%d","co2_source",pInfo->co2_src); 
            if( pInfo->co2_src == CYLINDER)
            {
                rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","co2_cyl_set_val",pInfo->lc_upper_threshold);
            }
            else
            { 
                rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","co2_gen_on_val",pInfo->lc_upper_threshold);  
                rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","co2_gen_off_val",pInfo->lc_lower_threshold); 
            }
        }        
    }
    else if((pInfo->DevOpState == FOLLOW) || (pInfo->DevOpState == FLIP))
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%d", "follow_dev",pInfo->ToFollowDevice);
        rsp +=sprintf((char *)rsp,",\"%s\":%d","lc_on",pInfo->is_lc_on);  
        rsp +=sprintf((char *)rsp,",\"%s\":%d","dc_on",pInfo->is_dc_on); 
    }
    else
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%d", "nt_mode_lc",pInfo->is_lc_on); 

        if( device == DEVICE_HOOD_VENT)
        {
            //  rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","nt_finish_delay",pInfo->FinishDelay.HH,pInfo->FinishDelay.MM,pInfo->FinishDelay.SS);
          
        }
        else if (device == DEVICE_CIRC)
        {
			rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","nt_lc_start_delay",pInfo->lc_start_delay.HH,pInfo->lc_start_delay.MM,pInfo->lc_start_delay.SS);     
            rsp +=sprintf((char *)rsp,",\"%s\":%d", "nt_mode_dc",pInfo->is_dc_on);
            rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","nt_dc_start_delay",pInfo->dc_start_delay.HH,pInfo->dc_start_delay.MM,pInfo->dc_start_delay.SS);
        }
    }

    rsp +=sprintf((char *)rsp,"}}");

    return rsp - in_rsp;
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static U32 app_fill_json_dev_vent_info(U8* in_rsp, GrowConfig_DevVentInfo*   pInfo)
{
    U8* rsp ;

    rsp = in_rsp;

    rsp +=sprintf((char *)rsp,",{\"%s\":{", "dev_vent");
    rsp +=sprintf((char *)rsp,"\"%s\":%d", "state",pInfo->DevState);

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "lc_rpt_tmr",pInfo->is_rpttimer_enabled_in_lc);
    if(pInfo->is_rpttimer_enabled_in_lc)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","lc_rpt_start_delay",pInfo->lc_start_delay.HH,pInfo->lc_start_delay.MM,pInfo->lc_start_delay.SS);
        rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","lc_rpt_on_time",pInfo->lc_on_time.HH,pInfo->lc_on_time.MM,pInfo->lc_on_time.SS);
        rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","lc_rpt_off_time",pInfo->lc_off_time.HH,pInfo->lc_off_time.MM,pInfo->lc_off_time.SS);     
    }

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "dc_rpt_tmr",pInfo->is_rpttimer_enabled_in_dc);
    if(pInfo->is_rpttimer_enabled_in_dc)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","dc_rpt_start_delay",pInfo->dc_start_delay.HH,pInfo->dc_start_delay.MM,pInfo->dc_start_delay.SS);
        rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","dc_rpt_on_time",pInfo->dc_on_time.HH,pInfo->dc_on_time.MM,pInfo->dc_on_time.SS);
        rsp +=sprintf((char *)rsp,",\"%s\":\"%d:%d:%d\"","dc_rpt_off_time",pInfo->dc_off_time.HH,pInfo->dc_off_time.MM,pInfo->dc_off_time.SS);     
    }


    rsp +=sprintf((char *)rsp,",\"%s\":%d", "lc_chk_tmp",pInfo->is_temp_check_needed_in_lc);
    if(pInfo->is_temp_check_needed_in_lc)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_tmp_on_val",pInfo->lc_on_temp_threshold); 
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_tmp_off_val",pInfo->lc_off_temp_threshold);
    }

    rsp +=sprintf((char *)rsp,",\"%s\":%d", "dc_chk_tmp",pInfo->is_temp_check_needed_in_dc);
    if(pInfo->is_temp_check_needed_in_dc)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_tmp_on_val",pInfo->dc_on_temp_threshold); 
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_tmp_off_val",pInfo->dc_off_temp_threshold);
    }



    rsp +=sprintf((char *)rsp,",\"%s\":%d", "lc_chk_hmdy",pInfo->is_humidity_check_needed_in_lc);
    if(pInfo->is_humidity_check_needed_in_lc)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_hmdy_on_val",pInfo->lc_on_humidity_threshold); 
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","lc_hmdy_off_val",pInfo->lc_off_humidity_threshold); 
    }


    rsp +=sprintf((char *)rsp,",\"%s\":%d", "dc_chk_hmdy",pInfo->is_humidity_check_needed_in_dc);


    if(pInfo->is_humidity_check_needed_in_dc)
    {
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_hmdy_on_val",pInfo->dc_on_humidity_threshold);
        rsp +=sprintf((char *)rsp,",\"%s\":%0.2f","dc_hmdy_off_val",pInfo->dc_off_humidity_threshold);
    }


    rsp +=sprintf((char *)rsp,"}}");

    return rsp - in_rsp;
}




/*******************************************************************************
* Name       :
* Description: 
* Remarks    : 
*******************************************************************************/
static void app_apply_profile_action_timer_handler(void* Arg)
{
    U8 time[35];

    U32 ct;

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    OSA_StopTimer(&App.apply_profile_timer);
    OSA_DeInitTimer(&App.apply_profile_timer);
    
    ct = RTC_GetTime(time);

    if(App.apply_profile_st <= ct)
    {
		TRACE_INFO("%s Applying Profile  \n", __FUNCTION__);
         /* Time to apply the new profile */
        memcpy(&App.WorkingProfile,App.pNewProfile,sizeof(App.WorkingProfile));
        free(App.pNewProfile);
        App.pNewProfile = NULL;
        App.apply_new_profile = FALSE;
        memset(&App.apply_profile_date,0x00,sizeof(App.apply_profile_date));
        memset(&App.apply_profile_time,0x00,sizeof(App.apply_profile_time));
        App.apply_profile_st = 0x00;
        App.gc_profile = App.apply_profile ;
        App.gc_profile_no = App.apply_profile_no;
        App_apply_new_profile();
    }
    else
    {
		TRACE_INFO("Time not ready , Restarting timer (%d)  \n",App.apply_profile_st-ct);
        OSA_InitTimer(&App.apply_profile_timer,(App.apply_profile_st-ct)*1000, FALSE, app_apply_profile_action_timer_handler, NULL);
        OSA_StartTimer(&App.apply_profile_timer);
    }

	TRACE_INFO("%s Exit \n", __FUNCTION__);
	
}

#if 0
/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_parse_json_grow_info(U8*   pBuf, 
                                     U32   len,
                                     GrowConfig_GrowCycleInfo* pInfo)
{
    jsmn_parser j;

    jsmntok_t tok[APP_PROFILE_DEV_MAX_TOKEN];

    S32 token_count; 

	U32 i;

	U8* p;

	U32 val;

	mTIME time;

	/* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROFILE_DEV_MAX_TOKEN);

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

	//memset(pInfo,0x00,sizeof(GrowConfig_GrowCycleInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
            if (!memcmp((char*)(pBuf + tok[i].start),"rpt_lo_tmp",APP_COMPARE_LENGTH("rpt_lo_tmp")))
            {
                i++;              
                pInfo->to_report_low_temp_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"lo_tmp_chk_val",APP_COMPARE_LENGTH("lo_tmp_chk_val")))
            {
                i++;        
                pInfo->low_temp_threshold_value = strtof((char *)(pBuf  + tok[i].start),NULL);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_hi_tmp",APP_COMPARE_LENGTH("rpt_hi_tmp")))
            {
                i++;              
                pInfo->to_report_high_temp_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"hi_tmp_chk_val",APP_COMPARE_LENGTH("hi_tmp_chk_val")))
            {
                i++;              
                pInfo->high_temp_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_critical_tmp",APP_COMPARE_LENGTH("rpt_critical_tmp")))
            {
                i++;              
                pInfo->to_report_critical_temp_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"critical_tmp_chk_val",APP_COMPARE_LENGTH("critical_tmp_chk_val")))
            {
                i++;              
                pInfo->critical_temp_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_lo_hmdy",APP_COMPARE_LENGTH("rpt_lo_hmdy")))
            {
                i++;              
                pInfo->to_report_low_humidity_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"lo_hmdy_chk_val",APP_COMPARE_LENGTH("lo_hmdy_chk_val")))
            {
                i++;              
                pInfo->low_humidity_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_hi_hmdy",APP_COMPARE_LENGTH("rpt_hi_hmdy")))
            {
                i++;              
                pInfo->to_report_high_humidity_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"hi_hmdy_chk_val",APP_COMPARE_LENGTH("hi_hmdy_chk_val")))
            {
                i++;              
                pInfo->high_humidity_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_lo_co2",APP_COMPARE_LENGTH("rpt_lo_co2")))
            {
                i++;              
                pInfo->to_report_low_co2_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"lo_co2_chk_val",APP_COMPARE_LENGTH("lo_co2_chk_val")))
            {
                i++;              
                pInfo->low_co2_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_hi_co2",APP_COMPARE_LENGTH("rpt_hi_co2")))
            {
                i++;              
                pInfo->to_report_high_co2_threshold = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"hi_co2_chk_val",APP_COMPARE_LENGTH("hi_co2_chk_val")))
            {
                i++;              
                pInfo->high_co2_threshold_value = strtof((char *)(pBuf  + tok[i].start), NULL);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_battery_flt",APP_COMPARE_LENGTH("rpt_battery_flt")))
            {
                i++;              
                pInfo->to_report_battery_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_frequency_flt",APP_COMPARE_LENGTH("rpt_battery_flt")))
            {
                i++;              
                pInfo->to_report_ac_power_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_emr_fuse_flt",APP_COMPARE_LENGTH("rpt_emr_fuse_flt")))
            {
                i++;              
                pInfo->to_report_emr_fuse_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_sensor_flt",APP_COMPARE_LENGTH("rpt_sensor_flt")))
            {
                i++;              
                pInfo->to_report_sensor_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"rpt_rs485_fuse_flt",APP_COMPARE_LENGTH("rpt_485_fuse_flt")))
            {
                i++;              
                pInfo->to_report_rs485_fuse_fault = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            } 
			else if (!memcmp((char*)(pBuf + tok[i].start),"light_restrike_tmr",APP_COMPARE_LENGTH("light_restrike_tmr")))
            {
                i++;              
                pInfo->is_light_restrike_timer_present = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"light_restrike_dur",APP_COMPARE_LENGTH("light_restrike_dur")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &time);
				pInfo->light_restrike_timer_duration = (time.HH*3600) + (time.MM*60) + time.SS;
            }
        }
    }
	//UTILITY_PrintBuffer(pBuf,len,1);
}
#endif
/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_parse_json_dev_light_info(U8*   pBuf, 
                                          U32   len,
                                          GrowConfig_DevLightInfo* pInfo)
{
	jsmn_parser j;

    jsmntok_t tok[APP_PROFILE_DEV_MAX_TOKEN];

    S32 token_count; 

	U32 i;

	U8* p;

	U32 val;

	/* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROFILE_DEV_MAX_TOKEN);

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

	//memset(pInfo,0x00,sizeof(GrowConfig_DevLightInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
			if (!memcmp((char*)(pBuf + tok[i].start),"state",APP_COMPARE_LENGTH("state")))
            {
                i++;              
                pInfo->DevState = strtol((char *)(pBuf  + tok[i].start), NULL, 10);

            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"light_cycle",APP_COMPARE_LENGTH("light_cycle")))
            {
                i++;              
                pInfo->LightCycle = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"light_on_time",APP_COMPARE_LENGTH("light_on_time")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->Light_OnTime);
            } 
			else if (!memcmp((char*)(pBuf + tok[i].start),"light_off_timer",APP_COMPARE_LENGTH("light_off_time")))
            {
                i++;
                p = (char*)(pBuf + tok[i].start);
                *(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->Light_OffTime);
            } 
		}	
	}

//	UTILITY_PrintBuffer(pBuf,len,1);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_parse_json_dev_gen_info(U8*   pBuf, 
                                        U32   len,
                                        GrowConfig_DevGenInfo* pInfo,
									    DEVICE_TYPE  device_name )
{
	jsmn_parser j;

    jsmntok_t tok[APP_PROFILE_DEV_MAX_TOKEN];

    S32 token_count; 

	U32 i;

	U8* p;

	U32 val;

	/* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROFILE_DEV_MAX_TOKEN);

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

	//memset(pInfo,0x00,sizeof(GrowConfig_DevGenInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
			if (!memcmp((char*)(pBuf + tok[i].start),"state",APP_COMPARE_LENGTH("state")))
            {
                i++;              
                pInfo->DevState = strtol((char *)(pBuf  + tok[i].start), NULL, 10);

            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"op_mode",APP_COMPARE_LENGTH("op_mode")))
            {
                i++;              
                pInfo->DevOpState = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }	
		}	
	}  
    
	if(pInfo->DevOpState == FLEXI)
	{
		for (i = 0; i < token_count; i++)
		{       
        	if (((tok[i].end) - (tok[i].start)) > 0 )
        	{
				if (!memcmp((char*)(pBuf + tok[i].start),"flexi_mode",APP_COMPARE_LENGTH("flexi_mode")))
           		{
              		i++;              
               		pInfo->DevOpMode = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
           		}
                else if (!memcmp((char*)(pBuf + tok[i].start),"vent_lockout",APP_COMPARE_LENGTH("vent_lockout")))
                {
                    i++;              
                    pInfo->is_vent_lockout = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                }
                else if (!memcmp((char*)(pBuf + tok[i].start),"lc_on",APP_COMPARE_LENGTH("lc_on")))
                {
                    i++;              
                    pInfo->is_lc_on = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                }
                else if (!memcmp((char*)(pBuf + tok[i].start),"dc_on",APP_COMPARE_LENGTH("dc_on")))
                {
                    i++;              
                    pInfo->is_dc_on = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                }

			}	
		}

		if(pInfo->DevOpMode == RPTTIMER)
		{
			for (i = 0; i < token_count; i++)
			{       
        		if (((tok[i].end) - (tok[i].start)) > 0 )
        		{
					if (!memcmp((char*)(pBuf + tok[i].start),"lc_rpt_tmr_start_delay",APP_COMPARE_LENGTH("lc_rpt_tmr_start_delay")))
           			{
              			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->lc_start_delay);
           			}
					else if (!memcmp((char*)(pBuf + tok[i].start),"lc_rpt_tmr_on_time",APP_COMPARE_LENGTH("lc_rpt_tmr_on_time")))
           			{
              			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->lc_on_time);
           			}
					else if (!memcmp((char*)(pBuf + tok[i].start),"lc_rpt_tmr_off_time",APP_COMPARE_LENGTH("lc_rpt_tmr_off_time")))
           			{
              			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->lc_off_time);
           			}
					else if (!memcmp((char*)(pBuf + tok[i].start),"dc_rpt_tmr_start_delay",APP_COMPARE_LENGTH("dc_rpt_tmr_start_delay")))
           			{
              			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->dc_start_delay);
           			}
					else if (!memcmp((char*)(pBuf + tok[i].start),"dc_rpt_tmr_on_time",APP_COMPARE_LENGTH("dc_rpt_tmr_on_time")))
           			{
              			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->dc_on_time);
           			}
					else if (!memcmp((char*)(pBuf + tok[i].start),"dc_rpt_tmr_off_time",APP_COMPARE_LENGTH("dc_rpt_tmr_off_time")))
           			{
              			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->dc_off_time);
           			}
				}	
			}
		}
		else if ((pInfo->DevOpMode == HEATER) || (pInfo->DevOpMode == COOLER))
		{
			for (i = 0; i < token_count; i++)
			{       
        		if (((tok[i].end) - (tok[i].start)) > 0 )
        		{
					if (!memcmp((char*)(pBuf + tok[i].start),"lc_tmp_up_val",APP_COMPARE_LENGTH("lc_tmp_up_val")))
           			{
              			i++;              
               			pInfo->lc_upper_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
                    else if (!memcmp((char*)(pBuf + tok[i].start),"lc_tmp_lo_val",APP_COMPARE_LENGTH("lc_tmp_lo_val")))
           			{
              			i++;              
               			pInfo->lc_lower_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
					if (!memcmp((char*)(pBuf + tok[i].start),"dc_tmp_up_val",APP_COMPARE_LENGTH("dc_tmp_up_val")))
           			{
              			i++;              
               			pInfo->dc_upper_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
                    else if (!memcmp((char*)(pBuf + tok[i].start),"dc_tmp_lo_val",APP_COMPARE_LENGTH("dc_tmp_lo_val")))
           			{
              			i++;              
               			pInfo->dc_lower_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
				}	
			}				
		}
		else if ((pInfo->DevOpMode == HUMIDIFIER) || (pInfo->DevOpMode == DEHUMIDIFIER))
		{
			for (i = 0; i < token_count; i++)
			{       
        		if (((tok[i].end) - (tok[i].start)) > 0 )
        		{
					if (!memcmp((char*)(pBuf + tok[i].start),"lc_hmdy_up_val",APP_COMPARE_LENGTH("lc_hmdy_up_val")))
           			{
              			i++;              
               			pInfo->lc_upper_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
                    else if (!memcmp((char*)(pBuf + tok[i].start),"lc_hmdy_lo_val",APP_COMPARE_LENGTH("lc_hmdy_lo_val")))
           			{
              			i++;              
               			pInfo->lc_lower_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
					if (!memcmp((char*)(pBuf + tok[i].start),"dc_hmdy_up_val",APP_COMPARE_LENGTH("dc_hmdy_up_val")))
           			{
              			i++;              
               			pInfo->dc_upper_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
                    else if (!memcmp((char*)(pBuf + tok[i].start),"dc_hmdy_lo_val",APP_COMPARE_LENGTH("dc_hmdy_lo_val")))
           			{
              			i++;              
               			pInfo->dc_lower_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
				}	
			}
				
		}
		else 
		{
			for (i = 0; i < token_count; i++)
			{       
        		if (((tok[i].end) - (tok[i].start)) > 0 )
        		{
                    if (!memcmp((char*)(pBuf + tok[i].start),"co2_source",APP_COMPARE_LENGTH("co2_source")))
                    {
                        i++;              
                        pInfo->co2_src = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                    }
					else if (!memcmp((char*)(pBuf + tok[i].start),"co2_cyl_set_val",APP_COMPARE_LENGTH("co2_cyl_set_val")))
           			{
              			i++;              
               			pInfo->lc_upper_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
					else if (!memcmp((char*)(pBuf + tok[i].start),"co2_gen_on_val",APP_COMPARE_LENGTH("co2_gen_on_val")))
           			{
              			i++;              
               			pInfo->lc_upper_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           			}
                    else if (!memcmp((char*)(pBuf + tok[i].start),"co2_gen_off_val",APP_COMPARE_LENGTH("co2_gen_off_val")))
                    {
                        i++;              
                        pInfo->lc_lower_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
                    }
                   
				}	
			}				
		}
	}
	else if((pInfo->DevOpState == FOLLOW) || (pInfo->DevOpState == FLIP))
	{
		for (i = 0; i < token_count; i++)
		{       
        	if (((tok[i].end) - (tok[i].start)) > 0 )
        	{
				if (!memcmp((char*)(pBuf + tok[i].start),"follow_dev",APP_COMPARE_LENGTH("follow_dev")))
           		{
              		i++;              
               		pInfo->ToFollowDevice = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
           		}
                else if (!memcmp((char*)(pBuf + tok[i].start),"lc_on",APP_COMPARE_LENGTH("lc_on")))
                {
                    i++;              
                    pInfo->is_lc_on = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                }
                else if (!memcmp((char*)(pBuf + tok[i].start),"dc_on",APP_COMPARE_LENGTH("dc_on")))
                {
                    i++;              
                    pInfo->is_dc_on = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
                }
			}	
		}
	}
	else
	{
	    if (device_name == DEVICE_HOOD_VENT)
		{
			for (i = 0; i < token_count; i++)
			{       
        		if (((tok[i].end) - (tok[i].start)) > 0 )
        		{
					if (!memcmp((char*)(pBuf + tok[i].start),"nt_mode_lc",APP_COMPARE_LENGTH("nt_mode_lc")))
           			{
              			i++;              
               			pInfo->is_lc_on = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
           			}
#if 0
           			else if (!memcmp((char*)(pBuf + tok[i].start),"nt_finish_delay",APP_COMPARE_LENGTH("nt_finish_delay")))
           			{
               			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->FinishDelay);
           			}
#endif
				}	
			}
		}
		else if(device_name == DEVICE_CIRC)
		{
			for (i = 0; i < token_count; i++)
			{       
        		if (((tok[i].end) - (tok[i].start)) > 0 )
        		{
					if (!memcmp((char*)(pBuf + tok[i].start),"nt_mode_lc",APP_COMPARE_LENGTH("nt_mode_lc")))
           			{
              			i++;              
               			pInfo->is_lc_on = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
           			}
           			else if (!memcmp((char*)(pBuf + tok[i].start),"nt_lc_start_delay",APP_COMPARE_LENGTH("nt_lc_start_delay")))
           			{
               			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->lc_start_delay);
           			}
					else if (!memcmp((char*)(pBuf + tok[i].start),"nt_mode_dc",APP_COMPARE_LENGTH("nt_mode_dc")))
           			{
              			i++;              
               			pInfo->is_dc_on = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
           			}
           			else if (!memcmp((char*)(pBuf + tok[i].start),"nt_dc_start_delay",APP_COMPARE_LENGTH("nt_dc_start_delay")))
           			{
               			i++;
               			p = (char*)(pBuf + tok[i].start);
               			*(p + tok[i].end - tok[i].start) = '\0';
						UTILITY_convert_time_str(p, &pInfo->dc_start_delay);
           			}	
				}	
			}
		}
	}	

//	UTILITY_PrintBuffer(pBuf,len,1);
}

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
static void app_parse_json_dev_vent_info(U8*   pBuf, 
                                          U32   len,
                                          GrowConfig_DevVentInfo* pInfo)
{
    jsmn_parser j;

    jsmntok_t tok[APP_PROFILE_DEV_MAX_TOKEN];

    S32 token_count; 

	U32 i;

	U8* p;

	U32 val;

	/* process the grow info JSON object message */
    jsmn_init(&j);
    token_count = jsmn_parse(&j, (const char*)pBuf,len, tok, APP_PROFILE_DEV_MAX_TOKEN);

	TRACE_INFO("%s Entry \n", __FUNCTION__);

    TRACE_DBG("TOKEN COUNT = %d\n",token_count);

	//memset(pInfo,0x00,sizeof(GrowConfig_DevVentInfo));

    for (i = 0; i < token_count; i++)
    {       
        if (((tok[i].end) - (tok[i].start)) > 0 )
        {
			if (!memcmp((char*)(pBuf + tok[i].start),"state",APP_COMPARE_LENGTH("state")))
            {
                i++;              
                pInfo->DevState = strtol((char *)(pBuf  + tok[i].start), NULL, 10);

            }
            else if (!memcmp((char*)(pBuf + tok[i].start),"lc_rpt_tmr",APP_COMPARE_LENGTH("lc_rpt_tmr")))
            {
                i++;              
                pInfo->is_rpttimer_enabled_in_lc = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_rpt_tmr",APP_COMPARE_LENGTH("dc_rpt_tmr")))
            {
                i++;              
                pInfo->is_rpttimer_enabled_in_dc = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_chk_tmp",APP_COMPARE_LENGTH("lc_chk_tmp")))
            {
                i++;              
                pInfo->is_temp_check_needed_in_lc = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_chk_tmp",APP_COMPARE_LENGTH("dc_chk_tmp")))
            {
                i++;              
                pInfo->is_temp_check_needed_in_dc = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_chk_hmdy",APP_COMPARE_LENGTH("lc_chk_hmdy")))
            {
                i++;              
                pInfo->is_humidity_check_needed_in_lc = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_chk_hmdy",APP_COMPARE_LENGTH("dc_chk_hmdy")))
            {
                i++;              
                pInfo->is_humidity_check_needed_in_dc = strtol((char *)(pBuf  + tok[i].start), NULL, 10);
            }
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_rpt_start_delay",APP_COMPARE_LENGTH("lc_rpt_start_delay")))
           	{
               	i++;
            	p = (char*)(pBuf + tok[i].start);
           		*(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->lc_start_delay);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_rpt_on_time",APP_COMPARE_LENGTH("lc_rpt_on_time")))
           	{
               	i++;
            	p = (char*)(pBuf + tok[i].start);
           		*(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->lc_on_time);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_rpt_off_time",APP_COMPARE_LENGTH("lc_rpt_off_time")))
           	{
               	i++;
            	p = (char*)(pBuf + tok[i].start);
           		*(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->lc_off_time);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_rpt_start_delay",APP_COMPARE_LENGTH("dc_rpt_start_delay")))
           	{
               	i++;
            	p = (char*)(pBuf + tok[i].start);
           		*(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->dc_start_delay);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_rpt_on_time",APP_COMPARE_LENGTH("dc_rpt_on_time")))
           	{
               	i++;
            	p = (char*)(pBuf + tok[i].start);
           		*(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->dc_on_time);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_rpt_off_time",APP_COMPARE_LENGTH("dc_rpt_off_time")))
           	{
               	i++;
            	p = (char*)(pBuf + tok[i].start);
           		*(p + tok[i].end - tok[i].start) = '\0';
				UTILITY_convert_time_str(p, &pInfo->dc_off_time);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_tmp_on_val",APP_COMPARE_LENGTH("lc_tmp_on_val")))
           	{
            	i++;              
            	pInfo->lc_on_temp_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_tmp_off_val",APP_COMPARE_LENGTH("lc_tmp_off_val")))
           	{
            	i++;              
            	pInfo->lc_off_temp_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_tmp_on_val",APP_COMPARE_LENGTH("dc_tmp_on_val")))
           	{
            	i++;              
            	pInfo->dc_on_temp_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_tmp_off_val",APP_COMPARE_LENGTH("dc_tmp_off_val")))
           	{
            	i++;              
            	pInfo->dc_off_temp_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_hmdy_on_val",APP_COMPARE_LENGTH("lc_hmdy_on_val")))
           	{
            	i++;              
            	pInfo->lc_on_humidity_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"lc_hmdy_off_val",APP_COMPARE_LENGTH("lc_hmdy_off_val")))
           	{
            	i++;              
            	pInfo->lc_off_humidity_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_hmdy_on_val",APP_COMPARE_LENGTH("dc_hmdy_on_val")))
           	{
            	i++;              
            	pInfo->dc_on_humidity_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}
			else if (!memcmp((char*)(pBuf + tok[i].start),"dc_hmdy_off_val",APP_COMPARE_LENGTH("dc_hmdy_off_val")))
           	{
            	i++;              
            	pInfo->dc_off_humidity_threshold = strtof((char *)(pBuf  + tok[i].start), NULL);
           	}	
		}	
	}  
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
