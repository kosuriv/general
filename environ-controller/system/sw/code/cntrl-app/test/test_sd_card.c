/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : test_sd_card.c
*
*  Description         : This file is  to test file system in SD card 
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  23rd Dec 2020     1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "trace.h"
#include "sd_card.h"
#include "fat_filelib.h"



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
static SPI Spi;
static SD_CARD SdCard;

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
void TEST_FileSystem()
{
    FL_FILE *file;

    int i =0;

    unsigned char c;

    int size ;

    TRACE_INFO("%s() Entry \n", __FUNCTION__);

    SPI_Init(&Spi,SPI2_HW_INDEX,SPI_MODE0);

    SD_CARD_Init(&SdCard,SPI_IFACE, &Spi );


#if 0

    // Read File Test 
    file = fl_fopen("/test.txt", "r");

    if (file)
    {
        fl_fseek(file, 0L, SEEK_END); 
  
    // calculating the size of the file 
         size = fl_ftell(file);  


        TRACE_INFO("SIZE  = %d \n",size ); 

        fl_fseek(file, 0L, SEEK_SET); 


    for (i =0 ; i< size; i++)
    {
         c=fl_fgetc(file);
         TRACE_INFO(" SIZE = %d ,CHAR = %c \n",size ,c ); 
    }  
      
#if 0
        while((c=fl_fgetc(file))!= EOF)              
            TRACE_INFO("CHAR = %c \n",c );
#endif 
             
    }
    else
    {
        TRACE_INFO("FILE NOT FOUND\n" );
    }

    // Close file
    fl_fclose(file);

return;

#endif


    // List root directory
 fl_listdirectory("/");

//return;

    // Write  File Test 
    file = fl_fopen("/file9.txt", "w");
    if (file)
    {
        // Write some data
        unsigned char data[] = { 'F', 'I', 'L', 'E','9','W','E','L','C','O','M','E' };
        if (fl_fwrite(data, 1, sizeof(data), file) != sizeof(data))
            printf("ERROR: Write file failed\n");
    }
    else
        printf("ERROR: Create file failed\n");



    // Close file
    fl_fclose(file);

#if 0
    // Delete File Test 
    if (fl_remove("/file.txt") < 0)
        printf("ERROR: Delete file failed\n");
#endif

    // List root directory
    fl_listdirectory("/");  

    //fl_shutdown(); 
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
