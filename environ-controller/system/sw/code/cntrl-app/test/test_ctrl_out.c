/*******************************************************************************
* (C) Copyright 2021;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : test_cntrl_out.c
*
*  Description         : This file contains functions to test device outlets  GPIOs 
                         for ESP board   
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*    Linta,Usha  7th Sep 2021      1.1               Initial Creation
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
void TEST_CTRL_OUT_ActivateAll()
{
    TRACE_DBG("%s() Entry",__FUNCTION__);

    CTRL_OUT_Init();

    ActivateDevice(DEVICE_LIGHT);
    ActivateDevice(DEVICE_HOOD_VENT);
    ActivateDevice(DEVICE_CIRC);
    ActivateDevice(DEVICE_VENT);
    ActivateDevice(DEVICE_FXP1);
    ActivateDevice(DEVICE_FXP2);

    TRACE_DBG("%s() Exit",__FUNCTION__); 
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TEST_CTRL_OUT_DeActivateAll()
{
    TRACE_DBG("%s() Entry",__FUNCTION__);

    DeActivateDevice(DEVICE_LIGHT);
    DeActivateDevice(DEVICE_HOOD_VENT);
    DeActivateDevice(DEVICE_CIRC);
    DeActivateDevice(DEVICE_VENT);
    DeActivateDevice(DEVICE_FXP1);
    DeActivateDevice(DEVICE_FXP2);

    TRACE_DBG("%s() Exit",__FUNCTION__); 
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
