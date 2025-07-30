/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : sd_card.c
*
*  Description         : It contains sd card functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  14th Dec 2020     1.0               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "sd_card.h"
#include "trace.h"
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

#define SD_CMD_GO_IDLE_STATE                       ((U8)0)
#define SD_CMD_SEND_OP_COND                        ((U8)1)
#define SD_CMD_ALL_SEND_CID                        ((U8)2)
#define SD_CMD_SET_REL_ADDR                        ((U8)3) /*!< SDIO_SEND_REL_ADDR for SD Card */
#define SD_CMD_SET_DSR                             ((U8)4)
#define SD_CMD_SDIO_SEN_OP_COND                    ((U8)5)
#define SD_CMD_HS_SWITCH                           ((U8)6)
#define SD_CMD_SEL_DESEL_CARD                      ((U8)7)
#define SD_CMD_HS_SEND_EXT_CSD                     ((U8)8)
#define SD_CMD_SEND_CSD                            ((U8)9)
#define SD_CMD_SEND_CID                            ((U8)10)
#define SD_CMD_READ_DAT_UNTIL_STOP                 ((U8)11) /*!< SD Card doesn't support it */
#define SD_CMD_STOP_TRANSMISSION                   ((U8)12)
#define SD_CMD_SEND_STATUS                         ((U8)13)
#define SD_CMD_HS_BUSTEST_READ                     ((U8)14)
#define SD_CMD_GO_INACTIVE_STATE                   ((U8)15)
#define SD_CMD_SET_BLOCKLEN                        ((U8)16)
#define SD_CMD_READ_SINGLE_BLOCK                   ((U8)17)
#define SD_CMD_READ_MULT_BLOCK                     ((U8)18)
#define SD_CMD_HS_BUSTEST_WRITE                    ((U8)19)
#define SD_CMD_WRITE_DAT_UNTIL_STOP                ((U8)20) /*!< SD Card doesn't support it */
#define SD_CMD_SET_BLOCK_COUNT                     ((U8)23) /*!< SD Card doesn't support it */
#define SD_CMD_WRITE_SINGLE_BLOCK                  ((U8)24)
#define SD_CMD_WRITE_MULT_BLOCK                    ((U8)25)
#define SD_CMD_PROG_CID                            ((U8)26) /*!< reserved for manufacturers */
#define SD_CMD_PROG_CSD                            ((U8)27)
#define SD_CMD_SET_WRITE_PROT                      ((U8)28)
#define SD_CMD_CLR_WRITE_PROT                      ((U8)29)
#define SD_CMD_SEND_WRITE_PROT                     ((U8)30)
#define SD_CMD_SD_ERASE_GRP_START                  ((U8)32) /*!< To set the address of the first write
                                                                  block to be erased. (For SD card only) */
#define SD_CMD_SD_ERASE_GRP_END                    ((U8)33) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For SD card only) */
#define SD_CMD_ERASE_GRP_START                     ((U8)35) /*!< To set the address of the first write block to be erased.
                                                                  (For MMC card only spec 3.31) */

#define SD_CMD_ERASE_GRP_END                       ((U8)36) /*!< To set the address of the last write block of the
                                                                  continuous range to be erased. (For MMC card only spec 3.31) */

#define SD_CMD_ERASE                               ((U8)38)
#define SD_CMD_FAST_IO                             ((U8)39) /*!< SD Card doesn't support it */
#define SD_CMD_GO_IRQ_STATE                        ((U8)40) /*!< SD Card doesn't support it */
#define SD_CMD_LOCK_UNLOCK                         ((U8)42)
#define SD_CMD_APP_CMD                             ((U8)55)
#define SD_CMD_GEN_CMD                             ((U8)56)
#define SD_CMD_NO_CMD                              ((U8)64)


#define CMD8       ((U8)8)
#define CMD8_ARG    0x0000001AA

#define CMD8_ARG_BE    0xAA010000
#define CMD8_CRC    0x86 //(1000011 << 1)

#define ACMD41       ((U8)41)

#define ACMD41_ARG    0x40000000

#define ACMD41_ARG_BE  0x00000040  //0xC0100000


#define CMD58       ((U8)58)


#define SD_DUMMY_BYTE                         ((U8)0xFF)

#define SD_SPI_NUMBER                   ((U8)64)  /* needed for SPI interface on SD cards */



#define SD_CARD_STD_CAPACITY_SD_CARD_V1_1             0x00000000
#define SD_CARD_STD_CAPACITY_SD_CARD_V2_0             0x00000001
#define SD_CARD_HIGH_CAPACITY_SD_CARD                 0x00000002
#define SD_CARD_MULTIMEDIA_CARD                       0x00000003
#define SD_CARD_SECURE_DIGITAL_IO_CARD                0x00000004
#define SD_CARD_HIGH_SPEED_MULTIMEDIA_CARD            0x00000005
#define SD_CARD_SECURE_DIGITAL_IO_COMBO_CARD          0x00000006
#define SD_CARD_HIGH_CAPACITY_MMC_CARD                0x00000007

#define SD_CARD_HIGH_CAPACITY                0x40000000
#define SD_CARD_STD_CAPACITY                 0x00000000

#define SD_CARD_SECTOR_SIZE  512



#define SD_CARD_CMD_SET(pch , cmd)          \
do {                                        \
    *pch = (U8) (cmd + SD_SPI_NUMBER) ;     \
    pch++;                                  \
} while (0);

#define SD_CARD_INSERT_DUMMY_BYTES(pch, no)  \
do {                                          \
    memset(pch, 0xff,no);                      \
    pch += no;                                \
} while (0);


#define SD_CARD_GENERIC_TIMEOUT        1000


#define CHECK_SD_PRESENCE(r)                    \
do {                                            \
    if (SD_CARD_is_sd_card_present()!= TRUE) {            \
        TRACE_ERR("SD card not present\n");     \
        return r;                               \
    }                                           \
} while (0);

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static S32 sd_card_media_init();

static S32 sd_card_media_read(U32 sector, U8 *buffer, U32 sector_count);

static S32 sd_card_media_write(U32 sector, U8 *buffer, U32 sector_count);

static void sd_card_rx_complete(void *pThis, UINT32 value);

static U32 sd_card_send_cmd(U8 cmd_id, U32 Arg, U8 crc, U32 no_res_bytes, U8* RxBuf, U32* rx_len);

static U32 sd_card_read_sector(U32 Addr, U8* Buf);

static U32 sd_card_write_sector(U32 Addr, U8* Buf);

static void sd_card_wait();

static BOOLEAN sd_card_send_single_cmd_check_response(U8 cmd_id, U32 Arg, U8 crc, U8 expected_res);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static SD_CARD* pSdCard;

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
void SD_CARD_Init(SD_CARD* pThis,SD_CARD_INTERFACE iface, void* DriverHandle )
{
    TRACE_INFO("%s() Entry \n", __FUNCTION__);

    pSdCard = pThis;

     /* Initialize interface driver */
    pSdCard->Interface = iface;

    if (pSdCard->Interface == SPI_IFACE) {

    //   pSdCard->Driver.pwrite = SPI_Tx;
     //   pSdCard->Driver.pread = SPI_Rx;
        
        /* call driver Init function */
        pSdCard->Driver.handle.Spi = ( SPI*)DriverHandle;
    }
    else {
            //TBD  add if interface is SDIO accordingly  
    }
#ifndef BOOTLOADER
    OSA_InitSemaphore(&pSdCard->rx_sem,0,1);
#endif

    /* initialize SD card */
    if (!sd_card_media_init()) {
        TRACE_ERR("Failed to initialize SD card\n");
        return;
    };

    // Initialize File IO Library
    fl_init();
    TRACE_INFO("File IO Library initialized\n");

    // Initialize Locks, The calling thread takes care of it
    fl_attach_locks(NULL, NULL);

    // Attach media access functions to library
    if (fl_attach_media(sd_card_media_read, sd_card_media_write) != FAT_INIT_OK)
    {
        TRACE_WARN("ERROR: Media attach failed\n");

        return;
    }
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_is_sd_card_present(void)
{
    /* TBD */

    return TRUE;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_is_dir_present(U8* dir_name)
{
    FL_DIR dirstat;

    CHECK_SD_PRESENCE(FALSE);
    
    if (fl_opendir((char *)dir_name, &dirstat))
    {
        fl_closedir(&dirstat);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_create_dir(U8* dir_name)
{
    CHECK_SD_PRESENCE(FALSE);
    
    if (fl_createdirectory((char *)dir_name) == 1) {
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_delete_dir(U8 *dir_name)
{
    FL_DIR dirstat;
    int len;
    char buffer[50];

    CHECK_SD_PRESENCE(FALSE);
    
    TRACE_INFO("Directory %s\n", dir_name);

    if (fl_opendir((char *)dir_name, &dirstat))
    {
        struct fs_dir_ent dirent;

        while (fl_readdir(&dirstat, &dirent) == 0)
        {
            if (dirent.is_dir)
            {
                TRACE_INFO("%s <DIR>\r\n", dirent.filename);
            }
            else
            {
                TRACE_INFO("FILENAME=%s \n", dirent.filename);
                len = sprintf((char *)buffer, "%s/%s",dir_name,dirent.filename);
                (void)len;
                fl_remove(buffer);          
            }           
        }
        fl_closedir(&dirstat);
    }
    
    return TRUE;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 SD_CARD_get_filelist_from_dir(U8 *dir_name, U8 **buffer, U8 buf_len)
{
    FL_DIR dirstat;
    int i = 0;
   
    CHECK_SD_PRESENCE(0);
    
    TRACE_INFO("Check directory %s in %s\n", dir_name, __FUNCTION__);

    if (fl_opendir((char *)dir_name, &dirstat))
    {
        struct fs_dir_ent dirent;

        while (fl_readdir(&dirstat, &dirent) == 0)
        {
            if (dirent.is_dir)
            {
                TRACE_DBG("%s <DIR>\r\n", dirent.filename);
            }
            else
            {
                TRACE_DBG("File %s added in list\n", dirent.filename);
                strcpy((char *)buffer[i], dirent.filename); 
                i++;
                if (i >= buf_len) {     return i;  }
            }
        }

        fl_closedir(&dirstat);
    }   
    
    return i;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_create_file(U8* data , U32 data_len, U8 *file_name)
{
    FL_FILE *file;
    U32 len;
    U32 i=0;
    U8 *p;
    U8 *q;
    U8 dir[50];

    TRACE_INFO("%s() Entry \n", __FUNCTION__);

    CHECK_SD_PRESENCE(FALSE);
    
    /* check whether the given directories  present or not */ 
    len = strlen((char *)file_name);
    p = file_name;
    memset(dir,0x00,sizeof(dir));
    while( i < len)
    {
        q = p;
        q++;
        p = (U8 *)strchr((char *)q, '/');
        if(p)
        {
            memset(dir,0x00,sizeof(dir));
            memcpy(dir,file_name,p-file_name);
            i = p - file_name;
            if (!SD_CARD_is_dir_present(dir))
            {
                TRACE_INFO("Creating SD dir %s\n", dir);
                if (SD_CARD_create_dir(dir) == FALSE) 
				{
                  return FALSE;
                }
            }
        }
        else
        {
            break;
        }
    }

    file = fl_fopen((char *)file_name, "w+");

    if (file)
    {
        // Update the file with given data
        if (fl_fwrite(data, 1, data_len, file) != data_len)
        {
            TRACE_WARN("ERROR: Write file failed\n");
            return FALSE;
        }
    }
    else
    {
        TRACE_WARN("ERROR: Create file failed\n");
        return FALSE;
    }

    fl_fclose(file);

    TRACE_INFO("%s() Exit \n", __FUNCTION__);
    return TRUE;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_delete_file(U8 *file_name)
{
    CHECK_SD_PRESENCE(FALSE);
    
    return fl_remove((char *)file_name);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_move_file(U8 *file_name , U8* src_dir, U8* dst_dir)
{   
    FL_FILE *f1 ;
    FL_FILE *f2 ;

    U8 buffer [50];
    U8 c;
    U32 i;

    CHECK_SD_PRESENCE(FALSE);
    
    sprintf((char *)buffer, "%s/%s", dst_dir, file_name);
    f2 = fl_fopen((char *)buffer, "w");
    if (!f2) {
        TRACE_ERR("Failed to open file %s\n", buffer);
        return FALSE;
    }

    sprintf((char *)buffer, "%s/%s", src_dir, file_name);
    f1 = fl_fopen((char *)buffer, "r");

    if (f1)
    {
        /* copying the content of file */ 

        for (i = 0; i < f1->filelength; i++)        
        //while((c=fl_fgetc(f1))!= 0x0A)
        {   
            c=fl_fgetc(f1);           
            //TRACE_INFO("CHAR = %x \n",c ); 
            fl_fputc(c, f2);
        }            
    }
    else
    {
        TRACE_ERR("Failed to open file %s\n", buffer);
        fl_fclose(f2);
        return FALSE;
    }

    fl_fclose(f1);  
    fl_fclose(f2);
    fl_remove((char *)buffer);
    return TRUE;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
S32 SD_CARD_read_data_from_file(U8 *file_name, U8* data , U32 data_len)
{
    FL_FILE *file;
    int32_t i;

    CHECK_SD_PRESENCE(FALSE);
    
    // Read The File  
    file = fl_fopen((char *)file_name, "r");
    if (file)
    {
        //while((*data++=fl_fgetc(file))!= 0x0A);
        for (i = 0; i < file->filelength; i++) 
        {
            if (i < data_len)
            {
                *data++ = fl_fgetc(file);
            }
        }
    }
    else
    {
        TRACE_WARN("File %s not found\n", file_name);
        i = -1;
    }
        
    fl_fclose(file);

    return i;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_write_data_to_file(U8* data , U32 data_len, U8 *file_name)
{
    return sd_create_file(data, data_len, file_name);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_append_file(U8* buffer, U32 buf_len, U8 *file_name)
{
    FL_FILE *f1;
    
    CHECK_SD_PRESENCE(FALSE);

    TRACE_INFO("%s() Entry \n", __FUNCTION__);
        
    // Read The File
    f1 = fl_fopen((char *)file_name, "a+");

    fl_fseek(f1,0,SEEK_END);
    fl_fwrite(buffer, 1, buf_len, f1);
    fl_fclose(f1);

    TRACE_INFO("%s() Exit \n", __FUNCTION__);
    
    return TRUE;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN SD_CARD_is_file_present(U8* file_name)
{
    FL_FILE *file;
    uint32 file_size = 40;


    CHECK_SD_PRESENCE(FALSE);

    file = fl_fopen(file_name, "r");

    if(file)
    {
        fl_fclose(file);
        return TRUE;
    }
    else
    {
        //TRACE_INFO("FILE NOT FOUND\n" );
        return FALSE;
    }
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void* SD_CARD_get_file_ptr(U8* file_name)
{
    FL_FILE *file;
    U32 len;
    U32 i=0;
    U8 *p;
    U8 *q;
    U8 dir[50];

    CHECK_SD_PRESENCE(FALSE);
    
    /* check whether the given directories  present or not */ 
    len = strlen((char *)file_name);
    p = file_name;
    memset(dir,0x00,sizeof(dir));
    while( i < len)
    {
        q = p;
        q++;
        p = (U8 *)strchr((char *)q, '/');
        if(p)
        {
            memset(dir,0x00,sizeof(dir));
            memcpy(dir,file_name,p-file_name);
            i = p - file_name;
            if (!SD_CARD_is_dir_present(dir))
            {
                TRACE_INFO("Creating SD dir %s\n", dir);
                if (SD_CARD_create_dir(dir) == FALSE) 
                {
                  return NULL;
                }
            }
        }
        else
        {
            break;
        }
    }

    file = fl_fopen((char *)file_name, "a+");

    return file;
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SD_CARD_append_file2(U8* buffer, U32 buf_len,  void **fPtr)
{
    FL_FILE **f2 = (FL_FILE**)fPtr;
    FL_FILE *f1;
    
    CHECK_SD_PRESENCE(FALSE);

    f1 =*fPtr;
    fl_fwrite(buffer, 1, buf_len, f1);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SD_CARD_close_file( void **fPtr)
{
    FL_FILE **f2 = (FL_FILE**)fPtr;
    FL_FILE *f1;
    
    CHECK_SD_PRESENCE(FALSE);

    f1 =*fPtr;
    fl_fclose(f1);
}

/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static S32 sd_card_media_init() 
{
    U8 cmd[100];
    U8* req;
    U32 rx_size;
    U8 RxBuf[20];
    U8 flag = 0x01;
    U8 card_type;
	U32 tx_cmd_size;
	U32 val;
	U8* s;

    TRACE_INFO("%s() Entry \n", __FUNCTION__);

    req = cmd;
    memset(cmd, 0xff, sizeof(cmd));


START_AGAIN:

	/* send dummy bytes to synch up  */
    // TRACE_INFO("SENDING DUMMY BYTES \n"); 

	SPI_Tx(pSdCard->Driver.handle.Spi,cmd,80,sd_card_rx_complete,NULL,DMA_INTERRUPT);
    sd_card_wait();

    /* Send CMD0 to put in idle mode  expected R1   i.e  1 byte */
    sd_card_send_cmd(SD_CMD_GO_IDLE_STATE, 0x00, 0x95, 1,RxBuf, &rx_size);

    if( RxBuf[rx_size-1] == 0x01)
    {
        TRACE_INFO("SD Card Put in Idle mode\n");           

    	/* Send CMD8 to check SD card type, expected R7  i.e 5 bytes */
    	tx_cmd_size = sd_card_send_cmd(CMD8, CMD8_ARG, 0x86, 5,RxBuf,&rx_size);

		val = *( (U32 *) &RxBuf[tx_cmd_size+1]);

	    //TRACE_INFO("tx_cmd_size = %d  val = %x \n",tx_cmd_size,val );

		TRACE_INFO("CMD8 Response Val = %x \n",val );

		if((RxBuf[tx_cmd_size] == 0x01) && ( val == CMD8_ARG_BE))
    	{
        	TRACE_INFO("SD Card  Ver 2 \n");    
			pSdCard->card_type = SD_CARD_STD_CAPACITY_SD_CARD_V2_0;     
    	} 
		else if(RxBuf[tx_cmd_size] == 0x05)
		{
			TRACE_INFO("SD Card  Ver 1 \n");    
			pSdCard->card_type = SD_CARD_STD_CAPACITY_SD_CARD_V1_1; 
		}
		else
		{
			goto START_AGAIN;
		}

		/* Do Initialisation */
    	do {

        	/* Send CMD55,expected R1,i.e one byte if response 0x01 new card , 0x05 old card */
        	sd_card_send_cmd(SD_CMD_APP_CMD, 0x00, 0x65, 1,RxBuf,&rx_size);

        	card_type = RxBuf[rx_size-1];

        	/* Send ACMD41 , RX expected 1 byte, expected response  0x00 */
        	if( card_type == 0x01)
        	{
            	//TRACE_INFO("NEW SD card \n");
            	sd_card_send_cmd(ACMD41, ACMD41_ARG, 0x77, 1,RxBuf,&rx_size);
        	}
        	else
        	{
            	//TRACE_INFO("OLD  SD card \n");
            	sd_card_send_cmd(ACMD41, 0x00, 0xE5, 1,RxBuf,&rx_size);
        	}
        	flag = RxBuf[rx_size-1];

    	} while (flag) ;

		/* Send CMD58 to read capacity of SD card, response  expected R3 i.e 5 bytes  */
		tx_cmd_size =  sd_card_send_cmd(CMD58, 0x00, 0x00, 5,RxBuf,&rx_size);

		s = &RxBuf[tx_cmd_size+1];

		BYTE_32_BIG_ENDIAN_TO_INT(s,val);

		//TRACE_INFO("tx_cmd_size = %d  val = %x \n",tx_cmd_size,val );

		TRACE_INFO("CMD58 Response Val = %x \n",val );

		if(( val &  SD_CARD_HIGH_CAPACITY ) == SD_CARD_HIGH_CAPACITY)
		{
			TRACE_INFO("HIGH Capacity  SD card \n");
			pSdCard->card_capacity = SD_CARD_HIGH_CAPACITY;
		}
		else
		{
			TRACE_INFO("LOW Capacity  SD card \n");
			pSdCard->card_capacity = SD_CARD_STD_CAPACITY;
		}

	}
	else
	{
		TRACE_WARN("SD CARD Error\n");
		goto START_AGAIN;
	}

    return 1;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : interface function to file system
*******************************************************************************/
static S32 sd_card_media_read(U32 sector, U8 *buffer, U32 sector_count)
{
    U32 i;

  //  TRACE_INFO("%s() Entry \n", __FUNCTION__);

  //  TRACE_INFO("sector = %d , sector_count = %d   SIZE = %d  \n",sector, sector_count, sector_count*SD_CARD_SECTOR_SIZE);

    for (i = 0; i < sector_count; i++)
    {
		sd_card_read_sector(sector*SD_CARD_SECTOR_SIZE, buffer);
		sector ++;
        buffer += SD_CARD_SECTOR_SIZE;
    }

    return 1;

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : interface function to file system
*******************************************************************************/
static S32 sd_card_media_write(U32 sector, U8 *buffer, U32 sector_count)
{
    U32 i;

    //TRACE_INFO("%s() Entry \n", __FUNCTION__);

    //TRACE_INFO("sector = %d , sector_count = %d   SIZE = %d  \n",sector, sector_count, sector_count*SD_CARD_SECTOR_SIZE);

    for (i = 0; i < sector_count; i++)
    {
		sd_card_write_sector(sector*SD_CARD_SECTOR_SIZE, buffer);
		sector ++;
        buffer += SD_CARD_SECTOR_SIZE;
    }

    return 1;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : executes under SPI DMA ISR 
*******************************************************************************/
static void sd_card_rx_complete(void *pThis, UINT32 value)
{
#ifndef BOOTLOADER
	OSA_GiveSemaphoreIsr(&pSdCard->rx_sem);
#else
	pSdCard->rx_flag = TRUE;
#endif
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 sd_card_send_cmd(U8 cmd_id, U32 Arg, U8 crc, U32 no_res_bytes, U8* RxBuf ,U32* rx_len)
{
    U8 cmd[100];
    U8* req;
    U32 size;
    U32 time_out;
	U32 tx_cmd_size;

 //   TRACE_INFO("%s() Entry \n", __FUNCTION__);

    req = cmd;
    memset(cmd, 0x00, sizeof(cmd));

    SD_CARD_INSERT_DUMMY_BYTES(req,2);

    SD_CARD_CMD_SET (req,cmd_id);

   // *((U32*)req) = Arg ;  /* arguments for this cmd */ 

	INT_TO_BYTE_32_BIG_ENDIAN(Arg, req);
    req +=4;

    *req = crc ;  //CRC
    req++;

	tx_cmd_size = req-cmd;

    SD_CARD_INSERT_DUMMY_BYTES(req,1+no_res_bytes);

    size = req-cmd;

    memset(RxBuf , 0x00, size);

//	TRACE_INFO("TRANSMITTED COMMAND \n", __FUNCTION__);
//	UTILITY_PrintBuffer(cmd, size,0);
   
    if (pSdCard->Interface == SPI_IFACE)
    {
        SPI_TxRx(pSdCard->Driver.handle.Spi, cmd,size,RxBuf,sd_card_rx_complete,RxBuf,DMA_INTERRUPT);
    }
    sd_card_wait();

//	TRACE_INFO("RECEIVED RESPONSE \n", __FUNCTION__);

 //   UTILITY_PrintBuffer(RxBuf, size,0);

    *rx_len = size;
	
    return tx_cmd_size+1;
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 sd_card_read_sector(U32 Addr, U8* Buf)
{
	U8 cmd[512];

	U8 res[20];

	U32 rx_size ;
	
	U32 tx_cmd_size;

    U8* req;

    //TRACE_INFO("%s() Entry \n", __FUNCTION__);

	memset(cmd, 0xff, sizeof(cmd));

	/* Send CMD16 to set block size */
    if(sd_card_send_single_cmd_check_response(SD_CMD_SET_BLOCKLEN, SD_CARD_SECTOR_SIZE, 0x00, 0x00))
	{
		if(sd_card_send_single_cmd_check_response(SD_CMD_READ_SINGLE_BLOCK, Addr, 0x00, 0x00))
		{

		//	TRACE_INFO("READ CMD ACCEPTED \n");
READ_START_TOKEN:

            SPI_TxRx(pSdCard->Driver.handle.Spi, &cmd[0],1 ,res,sd_card_rx_complete,res,DMA_INTERRUPT);
			sd_card_wait();

		//	TRACE_INFO("Get Read Start Token  = %x \n",res[0]);

			if(res[0] != 0xfe)
			{
				goto READ_START_TOKEN;
			}
            //TRACE_INFO("Get Read Start Token  = %x \n",res[0]);

			//TRACE_INFO("Get Actual Data  \n");
       
            SPI_TxRx(pSdCard->Driver.handle.Spi,cmd,SD_CARD_SECTOR_SIZE ,Buf,sd_card_rx_complete,res,DMA_INTERRUPT);
    		sd_card_wait();


		//	UTILITY_PrintBuffer(Buf, SD_CARD_SECTOR_SIZE,0);

            //TRACE_INFO("Get CRC of Read Data \n");

            SPI_TxRx(pSdCard->Driver.handle.Spi, cmd,2 ,res,sd_card_rx_complete,res,DMA_INTERRUPT);
            sd_card_wait();
		}
    }
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static U32 sd_card_write_sector(U32 Addr, U8* Buf)
{
	U32 size ;

	U32 rx_size;
	
	U32 tx_cmd_size;

	U32 time_out;

	U8  res[30];

	U8  req[30];

    //TRACE_INFO("%s() Entry \n", __FUNCTION__);

    if(sd_card_send_single_cmd_check_response(SD_CMD_SET_BLOCKLEN, SD_CARD_SECTOR_SIZE, 0x00, 0x00))
	{
	    if(sd_card_send_single_cmd_check_response(SD_CMD_WRITE_SINGLE_BLOCK, Addr, 0x00, 0x00))
	    {

    	   // TRACE_INFO("WRITE CMD ACCEPTED \n");

		    req[0] = 0xFE;  /* start Token */

            if (pSdCard->Interface == SPI_IFACE)
            {
             //   TRACE_INFO("Send Write Start Token \n");
                SPI_Tx(pSdCard->Driver.handle.Spi,req,1,sd_card_rx_complete,NULL,DMA_INTERRUPT);
                sd_card_wait();

             //   TRACE_INFO("Send Actual Data \n");
                SPI_Tx(pSdCard->Driver.handle.Spi,Buf,SD_CARD_SECTOR_SIZE,sd_card_rx_complete,NULL,DMA_INTERRUPT);
                sd_card_wait();
            }

            /* Check for write status token 

             check for data accepted  xxx0_ _ _1 

                    010 data accepted
                    101 data rejected due to CRC
                    110 data rejected due to write error

                    if (0x1f == 0x5 ) */
WRITE_STATUS_TOKEN:
            req[0] =0xff;
            SPI_TxRx(pSdCard->Driver.handle.Spi, req,1 ,res,sd_card_rx_complete,res,DMA_INTERRUPT);
            sd_card_wait();

            //TRACE_INFO("Write Status Token  = %x \n",res[0]);

            if((res[0] & 0x1f ) != 0x5)
            {
                goto WRITE_STATUS_TOKEN;
            }
           // TRACE_INFO("Write Status Token  = %x \n",res[0]);

            /* wait for whole write operation to complete */
WRITE_COMPLETE:
            SPI_TxRx(pSdCard->Driver.handle.Spi, req,1 ,res,sd_card_rx_complete,res,DMA_INTERRUPT);
            sd_card_wait();

            //TRACE_INFO("Write Complete Token  = %x \n",res[0]);

            if(res[0] != 0xff)
            {
                goto WRITE_COMPLETE;
            }
            //TRACE_INFO("Write Complete Token  = %x \n",res[0]);

	    }
	}
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static void sd_card_wait()
{
#ifndef BOOTLOADER
	OSA_TakeSemaphore(&pSdCard->rx_sem);
#else
	//pSdCard->rx_flag = FALSE;
	while( !pSdCard->rx_flag );
	pSdCard->rx_flag = FALSE;
#endif
}


/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
static BOOLEAN sd_card_send_single_cmd_check_response(U8 cmd_id, U32 Arg, U8 crc, U8 expected_res)
{

	U8 cmd[20];
	U8* req;
	U8 res[2];

	//TRACE_INFO("%s() Entry \n", __FUNCTION__);

	/* Send CMD16 to set block size */
	req = cmd;

    SD_CARD_INSERT_DUMMY_BYTES(req,2);

    SD_CARD_CMD_SET (req,cmd_id);

    INT_TO_BYTE_32_BIG_ENDIAN(Arg, req);
    req +=4;

    *req = crc ; 
    req++;

    if (pSdCard->Interface == SPI_IFACE)
    {
        SPI_Tx(pSdCard->Driver.handle.Spi,cmd,req-cmd,sd_card_rx_complete,NULL,DMA_INTERRUPT);
        sd_card_wait();

AGAIN:
        SPI_TxRx(pSdCard->Driver.handle.Spi, &cmd[0],1,res,sd_card_rx_complete,res,DMA_INTERRUPT);
        sd_card_wait();

        if(res[0] != expected_res)
        {
            goto AGAIN;
        }
        //TRACE_INFO("Got CMD Response   = %x \n",res[0]);
	}
	return TRUE;

}





/*******************************************************************************
*                          End of File
*******************************************************************************/
