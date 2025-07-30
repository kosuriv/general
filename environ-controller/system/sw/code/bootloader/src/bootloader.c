/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : bootloader.c
*
*  Description         : It contains boot loader main functionality 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  16th Jun 2020     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "bootloader.h"
#include "memory_layout.h"
#include "trace.h"
#include "board.h"
#include "flash.h"
#include "fat_filelib.h"
#include "common.h"

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/
void *APP_intr_arg_list[BOARD_INTR_MAX_IRQn];  //keep the same name as APP

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define BOOTLOADER_FIRMWARE_FILE "/FwUg/cntrl_app.bin"

#define BOOTLOADER_MAX_PKT_SIZE  512 //SD card read /write unit size
 
/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void bootloader_CheckFirmwareUpgrade();

static void bootloader_StartApplication();

static void bootloader_Upgrade();


/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static BOOTLOADER_T Boot;

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart1;
/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/


/*******************************************************************************
* Name       : BOOTLOADER_Start
* Description: Entry function for boot loader functionality 
* Remarks    :
*******************************************************************************/
void BOOTLOADER_Start()
{
    memset((U8*)&Boot, 0x00, sizeof(BOOTLOADER_T));  

#ifdef NUCLEO_F439ZI_EVK_BOARD
    TRACE_Init(&huart3);
#else
    TRACE_Init(&huart1);
#endif

	printf("%s() Entry\n",__FUNCTION__ );

	TRACE_INFO( "CSR status register = %x  Value = %x\n", PERIPH_BASE+RCC_CSR_OFFSET  , *((U32*)(PERIPH_BASE+RCC_CSR_OFFSET)));



 //   NVIC_EnableIRQ( SVCall_IRQn |  PendSV_IRQn | SysTick_IRQn | DMA2_Stream0_IRQn | DMA2_Stream3_IRQn );
    NVIC_EnableIRQ(  DMA2_Stream0_IRQn | DMA2_Stream3_IRQn );
#if 1
    /* Check for firmware update , if so upgrade the firmware  */
    bootloader_CheckFirmwareUpgrade();

    /* Relinquish the control to application  firmware */
    bootloader_StartApplication();

    while(1)
    {

    }
#endif
    
}
/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : bootloader_CheckFirmwareUpgrade
* Description: It checks the need to upgrade firmware , if so upgrade the
               firmware
* Remarks    : 
*******************************************************************************/
static void bootloader_CheckFirmwareUpgrade()
{  
    U8 file_name[50];

	U8* file_list[1];

	U8* rsp;

    TRACE_INFO("%s() Entry \n", __FUNCTION__);

    /* Initialize SD card & file system */
    SPI_Init(&Boot.Spi1,SPI1_HW_INDEX,SPI_MODE0);
    SPI_DMAInit(&Boot.Spi1);

    SD_CARD_Init(&Boot.SdCard,SPI_IFACE, &Boot.Spi1 );

    TRACE_DBG("Init SD Card COMPLETED\n"); 

 	if(SD_CARD_is_sd_card_present())
    {
#if 0
        if(SD_CARD_is_file_present(BOOTLOADER_FIRMWARE_FILE))
        {	
			TRACE_INFO("NEED TO DO FIRMWARE UPGRADE \n");

			bootloader_Upgrade(BOOTLOADER_FIRMWARE_FILE);
		}
#else
        if(SD_CARD_is_dir_present(COMMON_FWUG_DIR_NAME))
        {
			rsp = file_name;

			rsp +=sprintf(file_name ,"%s/",COMMON_FWUG_DIR_NAME);
			
			file_list[0] = rsp;

            if(SD_CARD_get_filelist_from_dir(COMMON_FWUG_DIR_NAME, file_list, 1))
            {
                TRACE_INFO("NEED TO DO FIRMWARE UPGRADE = %s \n",file_name);
				bootloader_Upgrade(file_name);
            }
        }
#endif
	}
    TRACE_INFO("%s() Exit \n", __FUNCTION__);
}

/*******************************************************************************
* Name       : bootloader_StartApplication
* Description: It releases control to application 
* Remarks    : 
*******************************************************************************/
static void bootloader_StartApplication()
{
    FUNC_PTR  *AppStart;

    /* disable interrupts */

    /* disable stack monitor */

    /* disable any timers ticks if started */

    /* clear all interrupts */

    AppStart = (FUNC_PTR *) (APP_ENTRY);

    /* Set new vector table pointer */

    /* Set new stack pointer  */

    /* Jump to application */


#if 1
    (*AppStart)();
#else
    printf("BOOT LOADER INFINTE LOOP STARTING \n");
    while(1)
    {
    	//printf("BOOT LOADER INFINTE LOOP\n");
    }
#endif
}

/*******************************************************************************
* Name       : 
* Description: It does copying firmware to FLASH
* Remarks    : 
*******************************************************************************/
static void bootloader_Upgrade(U8* file_name)
{
	FL_FILE *pFile;

    U32 file_size;

	U8 buf[BOOTLOADER_MAX_PKT_SIZE];

	U32 i;
	U32 k = 0x00;
	U32 next_len;
	U32 to_read_size;

	U32 addr;

    TRACE_INFO("%s() Entry \n", __FUNCTION__);

    /* Open file */
	pFile = fl_fopen(file_name , "r" );

    /* obtain file size */
    fl_fseek(pFile , 0 , SEEK_END);

    file_size = fl_ftell(pFile);

    TRACE_INFO("File Size: %d\n",file_size);

	/* Do erase flash */
    FLASH_EraseSize(&Boot.FlashDriver, APP_START_ADDRESS , file_size);

	fl_fseek (pFile , 0 , SEEK_SET);

	addr = APP_START_ADDRESS;

    for(i=0; i<file_size;)
    {
        k++;
        next_len = i + BOOTLOADER_MAX_PKT_SIZE ;
        if (next_len > file_size)
        {
            next_len = file_size;
        }
        to_read_size = next_len - i;
        
		i = next_len;

        TRACE_DBG("To READ SIZE (k = %d) = %d \n",k,to_read_size);

        fl_fread(buf,1,to_read_size,pFile);

		FLASH_Write(&Boot.FlashDriver, addr, buf, to_read_size);

		addr +=to_read_size;   
    }

	fl_fclose(pFile);

//	SD_CARD_delete_file(file_name);
	SD_CARD_delete_dir(COMMON_FWUG_DIR_NAME);
}

/*******************************************************************************
*                          End of File
*******************************************************************************/
