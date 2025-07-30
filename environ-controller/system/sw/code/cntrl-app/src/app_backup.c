/*******************************************************************************
* (C) Copyright 2021;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : app_backup.c
*
*  Description         : It contains back up store/restore functionality
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   25th Jan 2021    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "app_backup.h"
#include "trace.h"
#include "board.h"

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/


/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
#ifdef ESP32_S2_MINI
    __NOINIT_ATTR U8 sram_buffer[1000];
    
    #define BOARD_BACKUP_SRAM_START_ADDRESS &sram_buffer
#endif

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
void App_backup_store(APP_SW_RESET_CAUSE reason)
{
    U32 addr = BOARD_BACKUP_SRAM_START_ADDRESS;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    BACKUP_SRAM_Write(addr, &reason,sizeof(reason));
    addr += sizeof(reason);

    BACKUP_SRAM_Write(addr, APP_EC_PROFILE_VERSION,sizeof(APP_EC_PROFILE_VERSION));

    addr += sizeof(APP_EC_PROFILE_VERSION);

    BACKUP_SRAM_Write(addr, &App.ec_state,sizeof(App.ec_state));

    addr += sizeof(App.ec_state);


    BACKUP_SRAM_Write(addr, App.gc_name,sizeof(App.gc_name));

    addr += sizeof(App.gc_name);

    BACKUP_SRAM_Write(addr, &App.gc_profile_no,sizeof(App.gc_profile_no));

    addr += sizeof(App.gc_profile_no);

    BACKUP_SRAM_Write(addr, &App.gc_reporting_period,sizeof(App.gc_reporting_period));

    addr += sizeof(App.gc_reporting_period);

    BACKUP_SRAM_Write(addr, &App.gc_profile,sizeof(App.gc_profile));

    addr += sizeof(App.gc_profile);

    BACKUP_SRAM_Write(addr, &App.gc_total_days,sizeof(App.gc_total_days));

    addr += sizeof(App.gc_total_days);

    BACKUP_SRAM_Write(addr, &App.WorkingProfile,sizeof(Config_Profile));

    addr += sizeof(Config_Profile);

    BACKUP_SRAM_Write(addr, &App.GrowStatus,sizeof(GrowConfig_GrowCycleStatus));

    addr += sizeof(GrowConfig_GrowCycleStatus);

    TRACE_INFO("%s Exit \n", __FUNCTION__);
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN App_backup_retreive(void)
{
    U32 addr = BOARD_BACKUP_SRAM_START_ADDRESS;

    U8 profile_ver[20];

    BOOLEAN status = TRUE;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    addr += sizeof(APP_SW_RESET_CAUSE);

    BACKUP_SRAM_Read(addr, profile_ver,sizeof(APP_EC_PROFILE_VERSION));

    if(!strcmp(profile_ver,APP_EC_PROFILE_VERSION))
    {
        addr += sizeof(APP_EC_PROFILE_VERSION);

        BACKUP_SRAM_Read(addr, &App.ec_state,sizeof(App.ec_state));

        addr += sizeof(App.ec_state);

        BACKUP_SRAM_Read(addr, App.gc_name,sizeof(App.gc_name));

        addr += sizeof(App.gc_name);

        BACKUP_SRAM_Read(addr, &App.gc_profile_no,sizeof(App.gc_profile_no));

        addr += sizeof(App.gc_profile_no);

        BACKUP_SRAM_Read(addr, &App.gc_reporting_period,sizeof(App.gc_reporting_period));

        addr += sizeof(App.gc_reporting_period);

        BACKUP_SRAM_Read(addr, &App.gc_profile,sizeof(App.gc_profile));

        addr += sizeof(App.gc_profile);

        BACKUP_SRAM_Read(addr, &App.gc_total_days,sizeof(App.gc_total_days));

        addr += sizeof(App.gc_total_days);

        BACKUP_SRAM_Read(addr, &App.WorkingProfile,sizeof(Config_Profile));

        addr += sizeof(Config_Profile);

        BACKUP_SRAM_Read(addr, &App.GrowStatus,sizeof(GrowConfig_GrowCycleStatus));

        addr += sizeof(GrowConfig_GrowCycleStatus);
    }
    else
    {
        status = FALSE;
    }
    TRACE_INFO("%s Exit \n", __FUNCTION__);
    return status;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
APP_SW_RESET_CAUSE App_backup_reset_reason(void)
{
    U32 addr = BOARD_BACKUP_SRAM_START_ADDRESS;

    APP_SW_RESET_CAUSE reason;

    TRACE_INFO("%s Entry \n", __FUNCTION__);

    BACKUP_SRAM_Read(addr, &reason,sizeof(reason));
    
    return reason;
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
