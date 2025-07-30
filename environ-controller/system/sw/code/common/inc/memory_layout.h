/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : memory_layout.h
*
*  Description         : It describes the details of prtitioning of various 
                         sections of program flash memory 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  17th July 2020    1.1               Initial Creation
*  
*   Venu Kosuri  22nd  Sep 2021    1.2    Added details for  for ESP32_S2 board
*******************************************************************************/
#ifndef MEMORY_LAYOUT_H
#define MEMORY_LAYOUT_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD

/*
            -------------------------------------------------------------------  
    Section 0   |               64 KB               |  BootLoader (0x8000000 - 0x800FFFF)
                ---------------------------------------------------------------  
                --------------------------------------------------------------- 
                |                                    | (Profile Data, 
    Section 1   |               192 KB               |  Certificates etc ..)
                |                                    | (0x8010000 - 0x803FFFF)
                |                                    |  
                --------------------------------------------------------------- 
                ---------------------------------------------------------------  
                |                                   |
    Section 3   |               384KB               |     Application
                |                                   |  Starting at 0x08040000
                |                                   |   (0x8040000 - 0x809FFFF)
            -------------------------------------------------------------------

    Note :-  at present the given below section is not used , so ignore .
                ---------------------------------------------------------------  
                |                                   |
    Section 4   |               384KB               |    FW Download Image
                |                                   |  Starting at 0x080FA000  
                |                                   | (0x80A0000 - 0x80FFFFF)
            -------------------------------------------------------------------

*/

/* if there is change in this no pl change the following files also
 *  /cntrl_app/STM32F439ZITX_FLASH.ld   /cntrl_app/Core/Src/system_stm32f4xx.c  VECT_TAB_OFFSET  */
#define APP_START_ADDRESS 0x8040000

#define APP_ENTRY  (APP_START_ADDRESS+0x4)

#define MEMORY_FACTORY_PROFILE_START_ADDRESS 0x08010000  // sector 4  64KB block

#define MEMORY_USER_PROFILE_START_ADDRESS 0x08020000  // sector 5  128KB block

#define MEMORY_FLASH_PROFILE_SET_SIZE    0x1000  //TBD


#endif


#ifdef ESP32_S2_MINI


/*  external FLASH size 4MB , configured as given below 

            -------------------------------------------------------------------  
    Section 0   |         32KB        |  Bootloader.bin  (0x1000 - )
                ---------------------------------------------------------------

                ---------------------------------------------------------------
    Section 1   |         4 KB        | Partition table (0x8000 - ), type: data 
                ---------------------------------------------------------------

                ---------------------------------------------------------------
    Section 2   |          16 KB      |  Nvs (0x9000 - ), type: data 
                ---------------------------------------------------------------

                ---------------------------------------------------------------  
    Section 3   |          8 KB        | Ota Data  (0xd0000 - ), type: data 
                --------------------------------------------------------------- 
 
                ---------------------------------------------------------------  
    Section 4   |          4 KB        | phy_init  (0xf0000 - ), type: data 
                --------------------------------------------------------------- 

                ---------------------------------------------------------------
    Section 5   |          1MB      |  Factory APP (0x10000 - )  type: code
                ---------------------------------------------------------------

                ---------------------------------------------------------------  
    Section 6   |          1MB      | Ota_0 (0x130000 - )        type: code
                --------------------------------------------------------------- 
 
                ---------------------------------------------------------------  
    Section 7   |          1MB      | Ota_1  (0x250000 - )       type: code
                --------------------------------------------------------------- 

                ---------------------------------------------------------------  
    Section 8   | bottom  192 KB th position from last 1MB | factory profile + certificates   (0x3D0000 - ), type: data 
                ---------------------------------------------------------------
           
                ---------------------------------------------------------------  
    Section 9   |  bottom  160 KB th position from last 1MB | user profile    0x3d6000 ,type: data  
                ---------------------------------------------------------------
  
            -------------------------------------------------------------------

*/
#define APP_START_ADDRESS 0x10000

#define APP_ENTRY  (APP_START_ADDRESS+0x4)


#define MEMORY_FACTORY_PROFILE_START_ADDRESS  0x3D0000 //  earlier for s2 chip 0x3CE000,


#define MEMORY_FACTORY_PROFILE_SIZE  0x8000  //32 KB 

#define MEMORY_USER_PROFILE_START_ADDRESS  MEMORY_FACTORY_PROFILE_START_ADDRESS + MEMORY_FACTORY_PROFILE_SIZE 


/* store three certificates for WIFI as well as MQTT  at three 4 KB boundaries */

            /* enterprise  WIFI  certificates locations  */
#define MEMORY_WIFI_TLS_CERTIFICATES_START_ADDRESS  ( MEMORY_FACTORY_PROFILE_START_ADDRESS + 0x2000 )  // 8KB for factory profile
 
#define MEMORY_WIFI_TLS_CERTIFICATES_CA_CERT  MEMORY_WIFI_TLS_CERTIFICATES_START_ADDRESS  // 4KB for ca certificate

#define MEMORY_WIFI_TLS_CERTIFICATES_CLIENT_CERT  (MEMORY_WIFI_TLS_CERTIFICATES_START_ADDRESS+0x1000)  // 4KB for client certificate

#define MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT  (MEMORY_WIFI_TLS_CERTIFICATES_START_ADDRESS+0x2000)  // 4KB for key certificate


                                /* MQTT certificates locations  */
#define MEMORY_MQTT_TLS_CERTIFICATES_START_ADDRESS  ( MEMORY_WIFI_TLS_CERTIFICATES_KEY_CERT + 0x1000)

#define MEMORY_MQTT_TLS_CERTIFICATES_CA_CERT  MEMORY_MQTT_TLS_CERTIFICATES_START_ADDRESS  // 4KB for ca certificate

#define MEMORY_MQTT_TLS_CERTIFICATES_CLIENT_CERT  (MEMORY_MQTT_TLS_CERTIFICATES_START_ADDRESS+0x1000)  // 4KB for client certificate

#define MEMORY_MQTT_TLS_CERTIFICATES_KEY_CERT  (MEMORY_MQTT_TLS_CERTIFICATES_START_ADDRESS+0x2000)  // 4KB for key certificate

#endif


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
