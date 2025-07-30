/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : grow_config.h
*
*  Description         : This has all declarations for all data needed for grow
                         cycle configuration
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  7th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef GROW_CONFIG_H
#define GROW_CONFIG_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "profile_data_types.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

                                /* --------------  */

// Grow Cycle Info

#define CONFIG_GROW_CYCLE_NAME_LEN        23    /* Maximum length for grow cycle or crop name */

#define CONFIG_GROW_CYCLE_TIME_LEN        23    /* Maximum length for grow cycle or crop name */



typedef struct __attribute__((aligned(4),packed)) GrowConfig_GrowCycleInfo
{

    mUNIT  measure_unit;

    mCLK  clk;

    mLANGUAGE       language;

    /* grow week calender parameters */

    FLOAT low_temp_threshold_value;
    FLOAT high_temp_threshold_value;
    FLOAT critical_temp_threshold_value;
    BOOLEAN to_report_low_temp_threshold;
    BOOLEAN to_report_high_temp_threshold;
    BOOLEAN to_report_critical_temp_threshold;

    FLOAT low_humidity_threshold_value;
    FLOAT high_humidity_threshold_value;
    BOOLEAN to_report_low_humidity_threshold;
    BOOLEAN to_report_high_humidity_threshold;

    FLOAT low_co2_threshold_value;
    FLOAT high_co2_threshold_value;
    BOOLEAN to_report_low_co2_threshold;
    BOOLEAN to_report_high_co2_threshold;

        /* System Faults */
    BOOLEAN to_report_battery_fault; // 
    BOOLEAN to_report_ac_power_fault;
    BOOLEAN to_report_emr_fuse_fault;
    BOOLEAN to_report_rs485_fuse_fault;
    BOOLEAN to_report_sensor_fault;
    BOOLEAN to_report_hot_restrike_fault;



    BOOLEAN is_light_restrike_timer_present;
    U32 light_restrike_timer_duration;

    /* CO2  TBD*/

    /* Profile independent & device dpecific parameters */

    mTIME DevHoodVent_FinishDelay; //Used for hood vent device  in Native mode 
    
    FLOAT backup_battery_voltage;

    FLOAT backup_battery_low_threshold_voltage;

    FLOAT backup_battery_high_threshold_voltage;

    mTIME co2_vent_delay_time;

    U32 sensor_height;

} GrowConfig_GrowCycleInfo;        


                                /* --------------  */     

// Grow Cycle Status Info

typedef struct __attribute__((aligned(4),packed)) GrowConfig_GrowCycleStatus
{
    /* grow week calender parameters */
   // U8 grow_cycle_name[CONFIG_GROW_CYCLE_NAME_LEN+1];
    U32 day_no;
    U32 week_no;
    U32 day_cur_week;
    U8  grow_time[CONFIG_GROW_CYCLE_TIME_LEN];  //HH:MM:SS format

    U8  rtc_time[CONFIG_GROW_CYCLE_TIME_LEN];  //RTC time 

    U8  grow_start_date_time[100];

    S32 days_elapsed;


    U32 profile_no;   // profile that is active now 
    //U32 transition_profile_no;

   /* light cycle status  */
    mCLD   cycle_type;
    U8 light_mode ;   //(Always ON:0/OFF:1/Normal:2 )
    U32 light_cycle_duration; 
    U32 elapsed_light_cycle_duration;
    U32 pending_light_cycle_duration;
    U32 dark_cycle_duration; 
    U32 elapsed_dark_cycle_duration;
    U32 pending_dark_cycle_duration;

    U8 lc_duration[12];  //* same as above only in the string format HH:MM:SS
    U8 elapsed_lc_duration[12];
    U8 pending_lc_duration[12];
    U8 dc_duration[12]; 
    U8 elapsed_dc_duration[12];
    U8 pending_dc_duration[12];


    /* Device Outlet Configured Status */
    U8 dev_light_config ; //(Suspended:- 0 /Forced:1 /Normal:2 )
    U8 dev_hood_vent_config;
    U8 dev_circ_config;
    U8 dev_vent_config; // (Repeatetive timer mode:0/Temp based:1 /Humidity based:2 )
    U8 dev_fxp1_config; // (Repeatetive timer mode:0/HeaterMode:1 /CoolerMode:2 /HumidifyMode:3 /DeHumidifyMode:4 /CO2:5 /Leakmode:6 )
    U8 dev_fxp2_config;

    /* Device Outlet Operation status */
    BOOLEAN dev_light_on ; // (On:1/Off:0 )
    BOOLEAN dev_hood_vent_on; 
    BOOLEAN dev_circ_on;
    BOOLEAN dev_vent_on;
    BOOLEAN dev_fxp1_on; 
    BOOLEAN dev_fxp2_on;

    /* Sensor data */
    FLOAT cur_temp;
    FLOAT cur_humidity;
    FLOAT cur_co2;
    FLOAT max_temp;
    FLOAT max_humidity;
    FLOAT max_co2;
    FLOAT min_temp;
    FLOAT min_humidity;
    FLOAT min_co2;
    BOOLEAN low_temp_state;
    BOOLEAN high_temp_state;
    BOOLEAN low_humidity_state;
    BOOLEAN high_humidity_state;
    BOOLEAN low_co2_state;
    BOOLEAN high_co2_state;

    /* System Faults */
    BOOLEAN backup_battery_fault; // ( Yes:1/No:0)
    BOOLEAN ac_power_fault;
    BOOLEAN hot_restrike_fault;
    BOOLEAN emr_fuse_fault;
    BOOLEAN rs485_fuse_fault;
    BOOLEAN sensor_fault;
    BOOLEAN network_fault;
    BOOLEAN critical_temp_fault;

    BOOLEAN panic_switch_on;

    U32 dev_light_counter ; // (Deactivation Counter )
    U32 dev_hood_vent_counter; 
    U32 dev_circ_counter;
    U32 dev_vent_counter;
    U32 dev_fxp1_counter; 
    U32 dev_fxp2_counter;

    FLOAT cur_battery_volatge;

}GrowConfig_GrowCycleStatus;

                                /* --------------  */

typedef struct __attribute__((aligned(4),packed)) GrowConfig_DevLightInfo
{
    DEVICE_STATE  DevState;

    LIGHT_CYCLE_STATE LightCycle;
    mTIME Light_OnTime;
    mTIME Light_OffTime; 

}GrowConfig_DevLightInfo;

                                /* --------------  */

/* generic device is used for all four devices i.e hood vent , circ  , fxp1, fxp2 */
typedef struct __attribute__((aligned(4),packed)) GrowConfig_DevGenInfo
{
    DEVICE_STATE  DevState;      /* Suspended , Forced , Usual */
        
    DEVICE_OP_STATE DevOpState;  /* Native , Folow, Flip or Flex */

    DEVICE_OP_MODE DevOpMode;   /* In Flex mode , RPTTimer , heater , cooler, humidifier , dehumidifier , co2 */

    DEVICE_TYPE    ToFollowDevice  ;  //device to follow

    mTIME lc_start_delay;    //used by Circ device in native mode , used by all devices in  RPTTimer mode 

    mTIME dc_start_delay;    //used by Circ device in native mode , used by all devices in  RPTTimer mode 

    BOOLEAN is_lc_on ;  //Used by Hood Vent or Circ  in Native mode ,used by all devices for fleximode /flipmode/follow mode

    BOOLEAN is_dc_on ;  //Used by Circ in Native mode , used by all devices for fleximode /flipmode/follow mode

   // mTIME FinishDelay;   //Used for hood vent device  in Native mode 

    mTIME lc_on_time;  //in RPT Timer mode it is duration,Used to store on time in other modes, 

    mTIME lc_off_time; 

    FLOAT lc_upper_threshold;

    FLOAT lc_lower_threshold;

    mTIME dc_on_time;  //in RPT Timer mode it is duration,Used to store on time in other modes, 

    mTIME dc_off_time; 

    FLOAT dc_upper_threshold;

    FLOAT dc_lower_threshold;

    CO2_SOURCE  co2_src;

    BOOLEAN is_vent_lockout;  //to start flexi mode  check flag for vent outlet or not 


}GrowConfig_DevGenInfo;



typedef struct __attribute__((aligned(4),packed)) GrowConfig_DevVentInfo
{
    DEVICE_STATE  DevState;

    BOOLEAN is_rpttimer_enabled_in_lc;        
    mTIME lc_start_delay;
    mTIME lc_on_time;  
    mTIME lc_off_time;  


    BOOLEAN is_temp_check_needed_in_lc;
    FLOAT lc_on_temp_threshold;
    FLOAT lc_off_temp_threshold;


    BOOLEAN is_humidity_check_needed_in_lc;
    FLOAT lc_on_humidity_threshold;
    FLOAT lc_off_humidity_threshold;


    BOOLEAN is_rpttimer_enabled_in_dc;
    mTIME dc_start_delay;
    mTIME dc_on_time; 
    mTIME dc_off_time; 


    BOOLEAN is_temp_check_needed_in_dc;
    FLOAT dc_on_temp_threshold;
    FLOAT dc_off_temp_threshold;


    BOOLEAN is_humidity_check_needed_in_dc;
    FLOAT dc_on_humidity_threshold;
    FLOAT dc_off_humidity_threshold;

}GrowConfig_DevVentInfo;






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
