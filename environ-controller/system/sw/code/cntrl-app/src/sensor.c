/*******************************************************************************
* (C) Copyright 2022;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : sensor.c
*
*  Description         : It contains sensor wrapper  functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  18th Nov 2022     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "sensor.h"
#include "trace.h"
#include "modbus_master.h"
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
FLOAT SENSOR_Get(U32 sensor_index , SENSOR_TYPE name)
{
    FLOAT value = 0;
    BOOLEAN status = FALSE;
    U16 sensor_value2[3];
    TRACE_INFO("%s Entry \n", __FUNCTION__);

    /* get sensor values from sensor module */
    do 
    {
        memset((U8*)sensor_value2,0x00,sizeof(sensor_value2));
        status = MODBUS_GetSetParam(0x2a ,FC_GET_SENSOR_DATA , (void*)sensor_value2);

        if(!status)
        {   
           // OSA_Sleep(1); 
            OSA_Usleep(500000);
        } 
    }while (!status);

    switch ( name)
    {
        case SENSOR_TEMPERATURE:
            value = (FLOAT)sensor_value2[1];
            break;

        case SENSOR_HUMIDITY:
            value = (FLOAT)sensor_value2[2];
            break;
            
        case SENSOR_CO2:
            value = (FLOAT)sensor_value2[0];
            break;
    }
    TRACE_INFO("%s Exit = %.2f \n", __FUNCTION__, value);
    return value;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : use this when modbus is disabled
*******************************************************************************/
FLOAT SENSOR_Get2(U32 sensor_index , SENSOR_TYPE name)
{
    FLOAT value ;

    static U8 i = 0;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    switch ( name)
    {
        case SENSOR_TEMPERATURE:
            if( i == 1)
                value = 35;
            else
                value = 25;
            break;

        case SENSOR_HUMIDITY:
            if( i == 1)
                value = 70;
            else
                value = 60;
            break;
            
        case SENSOR_CO2:
            if( i == 1)
                value = 1300;
            else
                value = 1000;
            break;
    }

    if( i == 1)
    {
        i=0;
    }
    else
        i++;

    TRACE_INFO("%s Exit = %.2f \n", __FUNCTION__, value);
    return value;


}
/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/


/*******************************************************************************
*                          End of File
*******************************************************************************/
