/*******************************************************************************
* (C) Copyright 2022;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : sensor.c
*
*  Description         : It contains sensor functionality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   9th Sep 2022    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "sensor2.h"
#include "trace.h"
#include "scd4x_i2c.h"

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
 static SENSOR_T* pSensor;

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
void SENSOR_Init(SENSOR_T*  pThis)
{
    U16 serial_0;
    U16 serial_1;
    U16 serial_2;
    S16 error = 0;
    uint16_t sensor_altitude = 1;
    TRACE_INFO("%s Entry  \n", __FUNCTION__);

    pSensor = pThis;  

    memset(pSensor ,0x00,sizeof(SENSOR_T));

    sensirion_i2c_hal_init();

    // Clean up potential SCD40 states
    scd4x_wake_up();

    scd4x_stop_periodic_measurement();

   // scd4x_reinit();

    error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
    if (error) {
        TRACE_INFO("Error executing scd4x_get_serial_number(): %i\n", error);
    } else {
        TRACE_INFO("Serial Number: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
    }
    error = scd4x_get_sensor_altitude(&sensor_altitude);

    if (error) {
        TRACE_INFO("Error executing scd4x_get_sensor_altitude(): %i\n", error);
    } else {
        TRACE_INFO("Sensor Altitude: %x\n", sensor_altitude);
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SENSOR_Read(U16* co2, S32* temperature_m_deg_c, S32* humidity_m_percent_rh)
{
    S16 error = 0;
    bool data_ready_flag=0 ;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    if((pSensor->is_altitude_set) && (!pSensor->is_sensor_started))
    {
        /* start periodic measurement */
        error =   scd4x_start_periodic_measurement();

        if (error)
        {
            TRACE_INFO("Error executing scd4x_start_periodic_measurement(): %i\n", error);
        } 
        else
        {
            TRACE_INFO("CMD to start periodic measurement sent  SUCCESSFULLY \n");
        }
        pSensor->is_sensor_started = TRUE;
    }
    else if (!pSensor->is_sensor_started) 
    {

*co2 =1000;

*temperature_m_deg_c = 30;

*humidity_m_percent_rh = 70;

        TRACE_WARN("Sensor Altitude Not SET yet \n");
        return;
    }

    /* check for data ready flag */
    do
    {
        error = scd4x_get_data_ready_flag(&data_ready_flag);

        if (error)
        {
            TRACE_INFO("Error executing scd4x_get_data_ready_flag(): %x \n", error);
        }
        else
        {
           // TRACE_INFO("data_ready_flag : %x\n", data_ready_flag);
        }
        //OSA_Sleep(1);
    } while(!data_ready_flag);

    /* Get the sensor values */
    error = scd4x_read_measurement( co2, temperature_m_deg_c, humidity_m_percent_rh);

    if (error) 
    {
        TRACE_INFO("Error executing scd4x_read_measurement(): %i\n", error);
    }
    else
    {
        TRACE_INFO("Measured Successfully \n");
    }

   // scd4x_stop_periodic_measurement();
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SENSOR_SetHeight(U16 height)
{
    S16 error = 0;

    error = scd4x_set_sensor_altitude(height);

    if (error) 
    {
        TRACE_INFO("Error executing scd4x_set_sensor_altitude(): %i\n", error);
    } 
    else 
    {
        pSensor->is_altitude_set = TRUE;
        TRACE_INFO("Sensor Altitude Set \n");
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


/*******************************************************************************
*                          End of File
*******************************************************************************/
