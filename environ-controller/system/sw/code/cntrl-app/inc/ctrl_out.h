/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : ctrl_out.h
*
*  Description         : This file defines hw specific GPIO pins for different 
*                        device outlets 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  29th July 2020    1.1               Initial Creation
*   Linta, Usha  17th Sep 2021     1.2    ESP board GPIO pins defined 
*                                         File modified  as per std template 
*  
*******************************************************************************/
#ifndef CTRL_OUT_H
#define CTRL_OUT_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>

#include "data_types.h"
#include "profile_data_types.h"


#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "stm32f4xx_hal.h"
#include "main.h"
#endif


#ifdef ESP32_S2_MINI
#include "gpio.h"
#endif

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

/* Relay Pins for device outlets */
#define RELAY_LIGHT_Pin GPIO_PIN_2
#define RELAY_LIGHT_GPIO_Port GPIOB
#define RELAY_HV_Pin GPIO_PIN_3
#define RELAY_HV_GPIO_Port GPIOB
#define RELAY_CIRC_Pin GPIO_PIN_4
#define RELAY_CIRC_GPIO_Port GPIOB
#define RELAY_VENT_Pin GPIO_PIN_5
#define RELAY_VENT_GPIO_Port GPIOB
#define RELAY_FXP1_Pin GPIO_PIN_6
#define RELAY_FXP1_GPIO_Port GPIOB
#define RELAY_FXP2_Pin GPIO_PIN_7
#define RELAY_FXP2_GPIO_Port GPIOB

/* LED pins for device outlets*/
#define LED_LIGHT_Pin GPIO_PIN_8
#define LED_LIGHT_GPIO_Port GPIOB
#define LED_HV_Pin GPIO_PIN_9
#define LED_HV_GPIO_Port GPIOB
#define LED_CIRC_Pin GPIO_PIN_12
#define LED_CIRC_GPIO_Port GPIOB
#define LED_VENT_Pin GPIO_PIN_13
#define LED_VENT_GPIO_Port GPIOB
#define LED_FXP1_Pin GPIO_PIN_14
#define LED_FXP1_GPIO_Port GPIOB
#define LED_FXP2_Pin GPIO_PIN_15
#define LED_FXP2_GPIO_Port GPIOB

#define LEAK_MODE_Pin  GPIO_PIN_11
#define LEAK_MODE_GPIO_Port GPIOA

#define PANIC_MODE_SWITCH_Pin  GPIO_PIN_12
#define PANIC_MODE_SWITCH_GPIO_Port GPIOA

#define AC_POWER_FAULT_Pin GPIO_PIN_13
#define AC_POWER_FAULT_GPIO_Port GPIOA

#define RS485_FUSE_FAULT_Pin GPIO_PIN_15
#define RS485_FUSE_FAULT_GPIO_Port GPIOA

#define HOT_RESTRIKE_FAULT_Pin GPIO_PIN_14
#define HOT_RESTRIKE_FAULT_GPIO_Port GPIOA

#define SENSOR_LED_Pin GPIO_PIN_8
#define SENSOR_LED_GPIO_Port GPIOA


#define BATTERY_TEST_PULSE_Pin GPIO_PIN_1
#define BATTERY_TEST_PULSE_GPIO_Port GPIOB


#endif



#ifdef ESP32_S3_MINI

/* Relay Pins for device outlets */
#define RELAY_LIGHT_Pin GPIO_NUM_42 

#define RELAY_HV_Pin GPIO_NUM_41

#define RELAY_CIRC_Pin GPIO_NUM_40

#define RELAY_VENT_Pin GPIO_NUM_48

#define RELAY_FXP1_Pin GPIO_NUM_47

#define RELAY_FXP2_Pin GPIO_NUM_21

/* LED pins for device outlets*/
#define LED_LIGHT_Pin GPIO_NUM_45

#define LED_HV_Pin GPIO_NUM_13

#define LED_CIRC_Pin GPIO_NUM_11

#define LED_VENT_Pin GPIO_NUM_46

#define LED_FXP1_Pin GPIO_NUM_20

#define LED_FXP2_Pin GPIO_NUM_3

#define HOT_RESTRIKE_FAULT_Pin GPIO_NUM_2
#define AC_POWER_FAULT_Pin GPIO_NUM_5             //TBD
#define PANIC_MODE_SWITCH_Pin  GPIO_NUM_14
#define SENSOR_LED_Pin GPIO_NUM_19
#define RS485_FUSE_FAULT_Pin GPIO_NUM_1

#define BATTERY_VOLTAGE_ADC_Pin GPIO_NUM_4
#define BATTERY_TEST_PULSE_Pin GPIO_NUM_5     //TBD

#define GPIO_PIN_LOW  0

#define GPIO_PIN_HIGH  1


#else
/* Relay Pins for device outlets */
#define RELAY_LIGHT_Pin GPIO_NUM_9 

#define RELAY_HV_Pin GPIO_NUM_8

#define RELAY_CIRC_Pin GPIO_NUM_7

#define RELAY_VENT_Pin GPIO_NUM_6

#define RELAY_FXP1_Pin GPIO_NUM_5

#define RELAY_FXP2_Pin GPIO_NUM_4

/* LED pins for device outlets*/
#define LED_LIGHT_Pin GPIO_NUM_21

#define LED_HV_Pin GPIO_NUM_26

#define LED_CIRC_Pin GPIO_NUM_33

#define LED_VENT_Pin GPIO_NUM_39

#define LED_FXP1_Pin GPIO_NUM_42

#define LED_FXP2_Pin GPIO_NUM_41

#define HOT_RESTRIKE_FAULT_Pin GPIO_NUM_3
#define AC_POWER_FAULT_Pin GPIO_NUM_10
#define PANIC_MODE_SWITCH_Pin  GPIO_NUM_14
#define SENSOR_LED_Pin GPIO_NUM_40
#define RS485_FUSE_FAULT_Pin GPIO_NUM_13

#define BATTERY_VOLTAGE_ADC_Pin GPIO_NUM_1
#define BATTERY_TEST_PULSE_Pin GPIO_NUM_2

#define GPIO_PIN_LOW  0

#define GPIO_PIN_HIGH  1


#endif


/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/

void CTRL_OUT_Init();

void ActivateDevice(DEVICE_TYPE Dev);

void DeActivateDevice(DEVICE_TYPE Dev);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/

