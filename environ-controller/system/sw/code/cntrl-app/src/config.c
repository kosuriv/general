/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : config.c
*
*  Description         : This is main controller application code 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  7th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "trace.h"
#include "profile.h"


/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define CONFIG_PROFILE0_INIT(profile)                                          \
{                                                                              \
    PROFILE_AddTag(profile, CONFIG_SYSTEM_INFO_TAG, sizeof(Config_SystemInfo));\
    PROFILE_AddTag(profile, CONFIG_DEVICE_INFO_TAG, sizeof(Config_DeviceInfo));\
    PROFILE_AddTag(profile, CONFIG_WIFI_INFO_TAG, sizeof(Config_WiFiInfo));    \
    PROFILE_AddTag(profile, CONFIG_MQTT_INFO_TAG, sizeof(Config_MQTTInfo));    \
    PROFILE_AddTag(profile, CONFIG_SMTP_INFO_TAG, sizeof(Config_SmtpInfo));    \
    PROFILE_AddTag(profile, CONFIG_GROW_CYCLE_INFO_TAG, sizeof(GrowConfig_GrowCycleInfo));\
} 


#define CONFIG_PROFILE_INIT(profile)                                                          \
{                                                                                             \
    PROFILE_AddTag(profile, CONFIG_DEV_LIGHT_INFO_TAG, sizeof(GrowConfig_DevLightInfo));      \
    PROFILE_AddTag(profile, CONFIG_DEV_HOOD_VENT_INFO_TAG, sizeof(GrowConfig_DevGenInfo));    \
    PROFILE_AddTag(profile, CONFIG_DEV_CIRC_INFO_TAG, sizeof(GrowConfig_DevGenInfo));         \
    PROFILE_AddTag(profile, CONFIG_DEV_FXP1_INFO_TAG, sizeof(GrowConfig_DevGenInfo));         \
    PROFILE_AddTag(profile, CONFIG_DEV_FXP2_INFO_TAG, sizeof(GrowConfig_DevGenInfo));         \
    PROFILE_AddTag(profile, CONFIG_DEV_VENT_INFO_TAG, sizeof(GrowConfig_DevVentInfo));       \
}


/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void config_CheckCreateFactoryProfileSet(CONFIG *config);

static void config_CreateSpecialProfile(PROFILE *profile );

static void config_CreateProfile(PROFILE *profile, U32 size, U32 StartAddress);

static void config_InitProfile(PROFILE *profile, U32 size, U32 StartAddress, U32 profile_no);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/

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
void CONFIG_Init(CONFIG *config)
{
    /* create factory default profiles if not present in FLASH */
    config_CheckCreateFactoryProfileSet(config); 
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : never use this to write special profile i.e PROFILE 0 
*******************************************************************************/
void CONFIG_WriteProfile(Config_Profile* pProf, U32 profile_no )
{
    PROFILE profile ;

    U32 start_addr;

    start_addr = CONFIG_USER_PROFILE_SET_START_ADDRESS + (profile_no * CONFIG_PROFILE_MAX_SIZE);

    config_InitProfile(&profile,CONFIG_PROFILE_MAX_SIZE,start_addr,profile_no);

    //PROFILE_WriteTag(&profile, CONFIG_GROW_CYCLE_INFO_TAG, sizeof(GrowConfig_GrowCycleInfo), &pProf->GrowInfo);
    PROFILE_WriteTag(&profile, CONFIG_DEV_LIGHT_INFO_TAG, sizeof(GrowConfig_DevLightInfo), &pProf->DevLightConfig);
    PROFILE_WriteTag(&profile, CONFIG_DEV_HOOD_VENT_INFO_TAG, sizeof(GrowConfig_DevGenInfo), &pProf->DevHoodVentConfig);
    PROFILE_WriteTag(&profile, CONFIG_DEV_CIRC_INFO_TAG, sizeof(GrowConfig_DevGenInfo), &pProf->DevCircConfig);
    PROFILE_WriteTag(&profile, CONFIG_DEV_FXP1_INFO_TAG, sizeof(GrowConfig_DevGenInfo), &pProf->DevFxp1Config);
    PROFILE_WriteTag(&profile, CONFIG_DEV_FXP2_INFO_TAG, sizeof(GrowConfig_DevGenInfo), &pProf->DevFxp2Config);
    PROFILE_WriteTag(&profile, CONFIG_DEV_VENT_INFO_TAG, sizeof(GrowConfig_DevVentInfo), &pProf->DevVentConfig);

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 CONFIG_WriteTag(U32 profile_no, U32 Tag, U8* pBuf, U32 len )
{
    PROFILE profile ;

    U32 start_addr; 

    start_addr = CONFIG_USER_PROFILE_SET_START_ADDRESS + (profile_no * CONFIG_PROFILE_MAX_SIZE);

    config_InitProfile(&profile,CONFIG_PROFILE_MAX_SIZE,start_addr,profile_no);

    return PROFILE_WriteTag(&profile, Tag, len, pBuf);
    
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : never use this to read special profile i.e PROFILE 0 
*******************************************************************************/
BOOLEAN CONFIG_ReadProfile(U32 profile_no, CONFIG_PROFILE_TYPE prof_type, Config_Profile* pProf)
{
    PROFILE profile ;

    U32 start_addr; 

    if(prof_type == CONFIG_FACTORY_PROFILE)
    {  
        start_addr = MEMORY_FACTORY_PROFILE_START_ADDRESS + (profile_no * CONFIG_PROFILE_MAX_SIZE);
    }
    else
    {
        start_addr = CONFIG_USER_PROFILE_SET_START_ADDRESS + (profile_no * CONFIG_PROFILE_MAX_SIZE);
    }

    config_InitProfile(&profile,CONFIG_PROFILE_MAX_SIZE,start_addr,profile_no);

    if(!PROFILE_is_valid(&profile))
    {
        TRACE_WARN("%s() PROFILE%d NOT AVAILABLE\n", __FUNCTION__,profile_no);
        return FALSE;
    }

   // PROFILE_ReadTag(&profile, CONFIG_GROW_CYCLE_INFO_TAG,     &pProf->GrowInfo);
    PROFILE_ReadTag(&profile, CONFIG_DEV_LIGHT_INFO_TAG,      &pProf->DevLightConfig);
    PROFILE_ReadTag(&profile, CONFIG_DEV_HOOD_VENT_INFO_TAG,  &pProf->DevHoodVentConfig);
    PROFILE_ReadTag(&profile, CONFIG_DEV_CIRC_INFO_TAG,       &pProf->DevCircConfig);
    PROFILE_ReadTag(&profile, CONFIG_DEV_FXP1_INFO_TAG,       &pProf->DevFxp1Config);
    PROFILE_ReadTag(&profile, CONFIG_DEV_FXP2_INFO_TAG,       &pProf->DevFxp2Config);
    PROFILE_ReadTag(&profile, CONFIG_DEV_VENT_INFO_TAG,       &pProf->DevVentConfig);

    /* read grow cycle info from profile0  i.e independent of any profile */
    CONFIG_ReadTag(0, prof_type,CONFIG_GROW_CYCLE_INFO_TAG, &pProf->GrowInfo);

    return TRUE;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void CONFIG_ReadTag(U32 profile_no, CONFIG_PROFILE_TYPE prof_type,U32 Tag, U8* pBuf )
{
    PROFILE profile ;

    U32 start_addr; 

    if(prof_type == CONFIG_FACTORY_PROFILE)
    {  
        start_addr = MEMORY_FACTORY_PROFILE_START_ADDRESS + (profile_no * CONFIG_PROFILE_MAX_SIZE);
    }
    else
    {
        start_addr = CONFIG_USER_PROFILE_SET_START_ADDRESS + (profile_no * CONFIG_PROFILE_MAX_SIZE);
    }

    config_InitProfile(&profile,CONFIG_PROFILE_MAX_SIZE,start_addr,profile_no);

    PROFILE_ReadTag(&profile, Tag, pBuf);
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN CONFIG_IsUserProvisonPresent()
{
    PROFILE profile ;

    PROFILE_Init(&profile,
                 PROFILE_FLASH,                
                 CONFIG_PROFILE_0_MAX_SIZE, 
                 CONFIG_USER_PROFILE_SET_START_ADDRESS);

    if(!PROFILE_is_valid(&profile))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
  
/*******************************************************************************
* Name       : 
* Description: This erases the complete user profile Area including
               provisionining  info such as wifi , mqtt details  
* Remarks    : . 
*******************************************************************************/  
void CONFIG_EraseUserProfile()
{
    PROFILE profile ;

    PROFILE_Init(&profile,
                 PROFILE_FLASH,                
                 CONFIG_PROFILE_0_MAX_SIZE * (CONFIG_USER_PROFILE_MAX_NO +1), 
                 CONFIG_USER_PROFILE_SET_START_ADDRESS);

    PROFILE_Erase(&profile);
    
}

/*******************************************************************************
* Name       : 
* Description: This erases the individual user profile
* Remarks    : In case of proviison , it is special profile zero  .
*******************************************************************************/  
void CONFIG_EraseIndividualUserProfile(U32 profile_no)
{
    PROFILE profile ;
    U32 start_addr; 

    start_addr = CONFIG_USER_PROFILE_SET_START_ADDRESS + (profile_no * CONFIG_PROFILE_MAX_SIZE);

    PROFILE_Init(&profile,
                 PROFILE_FLASH,                
                 CONFIG_PROFILE_MAX_SIZE, 
                 start_addr);

    PROFILE_Erase(&profile);    
}

/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void config_CheckCreateFactoryProfileSet(CONFIG *config)
{

    PROFILE profile ;
    U32 address;

    PROFILE_Init(&profile,
                 PROFILE_FLASH,                
                 CONFIG_PROFILE_0_MAX_SIZE, 
				 CONFIG_FACTORY_PROFILE_SET_START_ADDRESS);

    address = CONFIG_FACTORY_PROFILE_SET_START_ADDRESS;

    if(!PROFILE_is_valid(&profile))
    {
        config_CreateSpecialProfile(&profile);

        address +=CONFIG_PROFILE_0_MAX_SIZE;

        //for(i=1; i<=PROFILE_MAX_NO; i++) only one factory profile given 
        {
            config_CreateProfile(&profile,CONFIG_PROFILE_MAX_SIZE,address);
            address +=CONFIG_PROFILE_MAX_SIZE;
        }
    }
    else
    {
    	TRACE_DBG("FACTORY PROFILE SET ALREADY AVAILABLE \n");
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void config_CreateSpecialProfile(PROFILE *profile )
{
    U8* buf;
    S8 dev_name[22];
    S8 mac_addr[6]={0};

    buf = malloc(CONFIG_PROFILE_MAX_TAG_LENGTH);

    CONFIG_PROFILE0_INIT(profile);

    CONFIG_SYSTEM_INFO_INIT(&profile->storage,
                            buf,
                            CONFIG_SYSTEM_INFO_TAG,
                            "PulseSpectrum",  // manufacturer
                            "Env_Cntrlr",     // model type
                            "1.0.0");         // serial number
#ifdef ESP32_S2_MINI
    esp_efuse_mac_get_default(mac_addr);
    sprintf(dev_name,"EC_%x_%x_%x_%x_%x_%x", mac_addr[0]&255, mac_addr[1]&255, mac_addr[2]&255, mac_addr[3]&255, mac_addr[4]&255, mac_addr[5]&255);
#endif
    CONFIG_DEVICE_INFO_INIT(&profile->storage,
                            buf,
                            CONFIG_DEVICE_INFO_TAG,
#ifdef NUCLEO_F439ZI_EVK_BOARD 
                            "test_device",  // device name
#endif
#ifdef ESP32_S2_MINI
                            dev_name,
#endif
                            "pulse_firm",     // firm name
                            1,     // device id
						    1,     // green house id
                            1,    // app id
							GROW_CONTROLLER);    // controller type



    CONFIG_WIFI_INFO_INIT(&profile->storage,
    		   	   	   	  buf,
    					  CONFIG_WIFI_INFO_TAG,
    					  "wdbsystems",		// WiFi SSID
    					  "WDBNet123",		// WiFi Password
       	   	   	   	   	   9600,			// serail baud rate
#ifdef NUCLEO_F439ZI_EVK_BOARD 
    					   5);				// time zone  -11 to 13
#endif
#ifdef ESP32_S2_MINI
						   "UTC-05:30");    //time zone wrt UTC ,India time
#endif

    CONFIG_MQTT_INFO_INIT(&profile->storage,
    		   	   	   	  buf,
    					  CONFIG_MQTT_INFO_TAG,
                     //   "localveggy.com"
    				//	  "18.188.31.15",	// MQTT host name
                        "192.168.1.137",
                        "pulsespectrum",  // MQTT user name 
    				//	  "eciot",	// MQTT user name
                        "welcome",      // MQTT password
    				//	  "admin",	    // MQTT password
    					  "test",			// MQTT topic
    					  1883,				// HOST PORT: 1883
    					  1);				// QOS value: QoS 1


    CONFIG_SMTP_INFO_INIT(&profile->storage,
                          buf,
                          CONFIG_SMTP_INFO_TAG,
                          "xxx@gmail.com",      // email id
                          "password",             // password
                          "smtp.gmail.com",        // SMTP server name
                          465,                     // SMTP server port
                          1,                       // SMTP Authentication Required
                          "yyy@hotmail.com",      // to address
                          NULL,                    // CC address
                          NULL,                    // BCC address
                          NULL);                   // Signature

    CONFIG_GROW_CYCLE_INFO_INIT(&profile->storage,buf, CONFIG_GROW_CYCLE_INFO_TAG);

    free(buf);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void config_CreateProfile(PROFILE *profile, U32 size, U32 StartAddress)
{
    U8* buf;

    buf = malloc(CONFIG_PROFILE_MAX_TAG_LENGTH);

	config_InitProfile(profile, size, StartAddress,1);

   // CONFIG_GROW_CYCLE_INFO_INIT(&profile->storage,buf, CONFIG_GROW_CYCLE_INFO_TAG);
	CONFIG_DEV_LIGHT_INFO_INIT(&profile->storage, buf, CONFIG_DEV_LIGHT_INFO_TAG);
    CONFIG_DEV_HOOD_VENT_INFO_INIT(&profile->storage, buf, CONFIG_DEV_HOOD_VENT_INFO_TAG);
    CONFIG_DEV_CIRC_INFO_INIT(&profile->storage,  buf, CONFIG_DEV_CIRC_INFO_TAG);
    CONFIG_DEV_FXP1_INFO_INIT(&profile->storage, buf, CONFIG_DEV_FXP1_INFO_TAG);
    CONFIG_DEV_FXP2_INFO_INIT(&profile->storage, buf, CONFIG_DEV_FXP2_INFO_TAG);
    CONFIG_DEV_VENT_INFO_INIT(&profile->storage, buf, CONFIG_DEV_VENT_INFO_TAG);

    free(buf);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void config_InitProfile(PROFILE *profile, U32 size, U32 StartAddress, U32 profile_no)
{
    PROFILE_Init(profile,
                 PROFILE_FLASH,
                 size,
                 StartAddress);

	if(profile_no)
	{
    	CONFIG_PROFILE_INIT(profile);
	}
	else
	{
		CONFIG_PROFILE0_INIT(profile);
	}
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
