/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : test_gpio.c
*
*  Description         : This file contains functions to test device GPIOs 
                         for ESP board   
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*    Linta. Usha   17th Sep 2021       1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "ctrl_out.h"
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

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void ctrl_out_ActivateLight(void);

static void ctrl_out_DeActivateLight(void);

static void ctrl_out_ActivateHoodVent(void);

static void ctrl_out_DeActivateHoodVent(void);  

static void ctrl_out_ActivateCirc(void);

static void ctrl_out_DeActivateCirc(void);  

static void ctrl_out_ActivateVent(void);

static void ctrl_out_DeActivateVent(void); 

static void ctrl_out_ActivateFxp1(void);

static void ctrl_out_DeActivateFxp1(void); 

static void ctrl_out_ActivateFxp2(void);

static void ctrl_out_DeActivateFxp2(void); 

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
void CTRL_OUT_Init()
{
     TRACE_DBG("%s Entry \n", __FUNCTION__);

#ifdef ESP32_S2_MINI

    gpio_reset_pin(GPIO_NUM_6);
    gpio_set_direction(GPIO_NUM_6, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_6, 0);

    gpio_reset_pin(RELAY_LIGHT_Pin);
    gpio_reset_pin(RELAY_HV_Pin);
    gpio_reset_pin(RELAY_CIRC_Pin);
    gpio_reset_pin(RELAY_VENT_Pin);
    gpio_reset_pin(RELAY_FXP1_Pin);
    gpio_reset_pin(RELAY_FXP2_Pin);

    gpio_set_direction(RELAY_LIGHT_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_HV_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_CIRC_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_VENT_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_FXP1_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(RELAY_FXP2_Pin, GPIO_MODE_OUTPUT);

    gpio_reset_pin(LED_LIGHT_Pin);
    gpio_reset_pin(LED_HV_Pin);
    gpio_reset_pin(LED_CIRC_Pin);
    gpio_reset_pin(LED_VENT_Pin);
    gpio_reset_pin(LED_FXP1_Pin);
    gpio_reset_pin(LED_FXP2_Pin);

    gpio_set_direction(LED_LIGHT_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_HV_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_CIRC_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_VENT_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_FXP1_Pin, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_FXP2_Pin, GPIO_MODE_OUTPUT);

#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void ActivateDevice(DEVICE_TYPE Dev)
{
    TRACE_DBG("%s Entry = %d \n", __FUNCTION__,Dev);

    switch(Dev)
    {
        case DEVICE_LIGHT:
            ctrl_out_ActivateLight();
            break;

        case DEVICE_HOOD_VENT:
            ctrl_out_ActivateHoodVent();
            break;

        case DEVICE_CIRC:
            ctrl_out_ActivateCirc();
            break;

        case DEVICE_VENT:
            ctrl_out_ActivateVent();
            break;

        case DEVICE_FXP1:
            ctrl_out_ActivateFxp1();
            break;

        case DEVICE_FXP2:
            ctrl_out_ActivateFxp2();
            break;
        default:
            break;
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void DeActivateDevice(DEVICE_TYPE Dev)
{
    TRACE_DBG("%s Entry = %d \n", __FUNCTION__,Dev);

    switch(Dev)
    {
        case DEVICE_LIGHT:
            ctrl_out_DeActivateLight();
            break;

        case DEVICE_HOOD_VENT:
            ctrl_out_DeActivateHoodVent();
            break;

        case DEVICE_CIRC:
            ctrl_out_DeActivateCirc();
            break;

        case DEVICE_VENT:
            ctrl_out_DeActivateVent();
            break;

        case DEVICE_FXP1:
            ctrl_out_DeActivateFxp1();
            break;

        case DEVICE_FXP2:
            ctrl_out_DeActivateFxp2();
            break;
        default:
            break;
    }
}



/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void ctrl_out_ActivateLight(void)
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_LIGHT_GPIO_Port, RELAY_LIGHT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_LIGHT_GPIO_Port, LED_LIGHT_Pin, GPIO_PIN_SET);
#endif

#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_LIGHT_Pin, GPIO_PIN_HIGH);
    gpio_set_level(LED_LIGHT_Pin, GPIO_PIN_HIGH);
#endif
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void ctrl_out_DeActivateLight(void)
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_LIGHT_GPIO_Port, RELAY_LIGHT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_LIGHT_GPIO_Port, LED_LIGHT_Pin, GPIO_PIN_RESET);
#endif

#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_LIGHT_Pin, GPIO_PIN_LOW);
    gpio_set_level(LED_LIGHT_Pin, GPIO_PIN_LOW);
#endif
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void ctrl_out_ActivateHoodVent(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_HV_GPIO_Port, RELAY_HV_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_HV_GPIO_Port, LED_HV_Pin, GPIO_PIN_SET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_HV_Pin, GPIO_PIN_HIGH);
    gpio_set_level(LED_HV_Pin, GPIO_PIN_HIGH);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void ctrl_out_DeActivateHoodVent(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_HV_GPIO_Port, RELAY_HV_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_HV_GPIO_Port, LED_HV_Pin, GPIO_PIN_RESET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_HV_Pin, GPIO_PIN_LOW);
    gpio_set_level(LED_HV_Pin, GPIO_PIN_LOW);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/

static void ctrl_out_ActivateCirc(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_CIRC_GPIO_Port, RELAY_CIRC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_HV_GPIO_Port, LED_CIRC_Pin, GPIO_PIN_SET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_CIRC_Pin, GPIO_PIN_HIGH);
    gpio_set_level(LED_CIRC_Pin, GPIO_PIN_HIGH);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/

static void ctrl_out_DeActivateCirc(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_CIRC_GPIO_Port, RELAY_CIRC_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_HV_GPIO_Port, LED_CIRC_Pin, GPIO_PIN_RESET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_CIRC_Pin, GPIO_PIN_LOW);
    gpio_set_level(LED_CIRC_Pin, GPIO_PIN_LOW);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/

static void ctrl_out_ActivateVent(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_VENT_GPIO_Port, RELAY_VENT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_VENT_GPIO_Port, LED_VENT_Pin, GPIO_PIN_SET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_VENT_Pin, GPIO_PIN_HIGH);
    gpio_set_level(LED_VENT_Pin, GPIO_PIN_HIGH);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/

static void ctrl_out_DeActivateVent(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_VENT_GPIO_Port, RELAY_VENT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_VENT_GPIO_Port, LED_VENT_Pin, GPIO_PIN_RESET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_VENT_Pin, GPIO_PIN_LOW);
    gpio_set_level(LED_VENT_Pin, GPIO_PIN_LOW);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/

static void ctrl_out_ActivateFxp1(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_FXP1_GPIO_Port, RELAY_FXP1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_FXP1_GPIO_Port, LED_FXP1_Pin, GPIO_PIN_SET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_FXP1_Pin, GPIO_PIN_HIGH);
    gpio_set_level(LED_FXP1_Pin, GPIO_PIN_HIGH);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void ctrl_out_DeActivateFxp1(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_FXP1_GPIO_Port, RELAY_FXP1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_FXP1_GPIO_Port, LED_FXP1_Pin, GPIO_PIN_RESET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_FXP1_Pin, GPIO_PIN_LOW);
    gpio_set_level(LED_FXP1_Pin, GPIO_PIN_LOW);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void ctrl_out_ActivateFxp2(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_FXP2_GPIO_Port, RELAY_FXP2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_FXP2_GPIO_Port, LED_FXP2_Pin, GPIO_PIN_SET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_FXP2_Pin, GPIO_PIN_HIGH);
    gpio_set_level(LED_FXP2_Pin, GPIO_PIN_HIGH);
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/

static void ctrl_out_DeActivateFxp2(void) 
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
    HAL_GPIO_WritePin(RELAY_FXP2_GPIO_Port, RELAY_FXP2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_FXP2_GPIO_Port, LED_FXP2_Pin, GPIO_PIN_RESET);
#endif
    
#ifdef ESP32_S2_MINI
    gpio_set_level(RELAY_FXP2_Pin, GPIO_PIN_LOW);
    gpio_set_level(LED_FXP2_Pin, GPIO_PIN_LOW);
#endif
}


/*******************************************************************************
*                          End of File
*******************************************************************************/

