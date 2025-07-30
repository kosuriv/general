/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : flash.c
*
*  Description         : It contains flash driver functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  29th July 2020    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <string.h>

#include "flash.h"

#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "stm32f4xx_hal_flash.h"
#endif

#ifdef ESP32_S2_MINI
#include "esp_spi_flash.h"
#include "esp_flash.h"
#endif

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
extern void *APP_intr_arg_list[];
#endif

/*******************************************************************************
*                          Extern Function Declarations
*******************************************************************************/

/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
#define FLASH_RDWR_TYPE_SIZE_DWORD      8
#define FLASH_RDWR_TYPE_SIZE_WORD       4
#define FLASH_RDWR_TYPE_SIZE_HWORD      2
#define FLASH_RDWR_TYPE_SIZE_BYTE       1


/* Bank 1 sectors */
#define FLASH_SECTOR_0_ADDRESS      0x08000000   //16k
#define FLASH_SECTOR_1_ADDRESS      0x08004000   //16k
#define FLASH_SECTOR_2_ADDRESS      0x08008000   //16k
#define FLASH_SECTOR_3_ADDRESS      0x0800C000   //16k
#define FLASH_SECTOR_4_ADDRESS      0x08010000   //64k
#define FLASH_SECTOR_5_ADDRESS      0x08020000   //128k
#define FLASH_SECTOR_6_ADDRESS      0x08040000   //128k
#define FLASH_SECTOR_7_ADDRESS      0x08060000   //128k
#define FLASH_SECTOR_8_ADDRESS      0x08080000   //128k
#define FLASH_SECTOR_9_ADDRESS      0x080A0000   //128k
#define FLASH_SECTOR_10_ADDRESS     0x080C0000   //128k
#define FLASH_SECTOR_11_ADDRESS     0x080E0000   //128k

/* Bank 2 sectors */
#define FLASH_SECTOR_12_ADDRESS     0x08100000   //16k
#define FLASH_SECTOR_13_ADDRESS     0x08104000   //16k
#define FLASH_SECTOR_14_ADDRESS     0x08108000   //16k
#define FLASH_SECTOR_15_ADDRESS     0x0810C000   //16k
#define FLASH_SECTOR_16_ADDRESS     0x08110000   //64k
#define FLASH_SECTOR_17_ADDRESS     0x08120000   //128k
#define FLASH_SECTOR_18_ADDRESS     0x08140000   //128k
#define FLASH_SECTOR_19_ADDRESS     0x08160000   //128k
#define FLASH_SECTOR_20_ADDRESS     0x08180000   //128k
#define FLASH_SECTOR_21_ADDRESS     0x081A0000   //128k
#define FLASH_SECTOR_22_ADDRESS     0x081C0000   //128k
#define FLASH_SECTOR_23_ADDRESS     0x081E0000   //128k

#endif
/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
static U32 flash_EraseInit(U32 sector_address, FLASH_EraseInitTypeDef * f);
#endif

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
void FLASH_Init(FLASH_T *flash)
{

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
FLASH_Status FLASH_Erase(FLASH_T *flash, U32 sector_address)
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
	U32 sector_error;

	FLASH_Status erase_status = FLASH_OK;

	FLASH_EraseInitTypeDef eraseInit;

	flash_EraseInit(sector_address, &eraseInit);

	HAL_FLASH_Unlock();

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	erase_status = HAL_FLASHEx_Erase(&eraseInit, &sector_error);

	HAL_FLASH_Lock();

	return erase_status;
#endif

#ifdef ESP32_S2_MINI
   printf("%s Entry()  sector_address = %x\n", __FUNCTION__,sector_address);
   return  spi_flash_erase_sector(sector_address);
#endif


}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
FLASH_Status FLASH_Write(FLASH_T *flash, U32 addr, const U8 *Wbuf, U32 size)
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
	U32 index = 0;
	FLASH_Status FLASH_Write_Status = FLASH_OK;
	uint64_t flash_write_word = 0xFFFFFFFF;

	U32 word_count = size / FLASH_RDWR_TYPE_SIZE_WORD;
	U32 residue_bytes = size % FLASH_RDWR_TYPE_SIZE_WORD;

	HAL_FLASH_Unlock();

	while(word_count) {
		memcpy(&flash_write_word, &Wbuf[index], FLASH_RDWR_TYPE_SIZE_WORD);

		FLASH_Write_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr+index, flash_write_word);
		index += FLASH_RDWR_TYPE_SIZE_WORD;
		word_count--;
		if(FLASH_Write_Status != FLASH_OK) {
			break;
		}
	}

	while(residue_bytes) {
		FLASH_Write_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr+index, Wbuf[index]);
		index++;
		residue_bytes--;
		if(FLASH_Write_Status != FLASH_OK) {
			break;
		}
	}
	HAL_FLASH_Lock();

	return FLASH_Write_Status;
#endif

#ifdef ESP32_S2_MINI
    return spi_flash_write(addr,Wbuf,size);
#endif

}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void FLASH_Read(FLASH_T *flash,U32 addr, U8 *Rbuf, U32 size)
{
#ifdef NUCLEO_F439ZI_EVK_BOARD
	U32 index = 0;
	U32 flash_read_word;

	U32 word_count = size / FLASH_RDWR_TYPE_SIZE_WORD;
	U32 residue_bytes = size % FLASH_RDWR_TYPE_SIZE_WORD;

	while(word_count){
		flash_read_word = *(uint32_t*)(addr + index);
		Rbuf[index] = (uint8_t)(flash_read_word);
		Rbuf[index+1] = (uint8_t)(flash_read_word >> 8);
		Rbuf[index+2] = (uint8_t)(flash_read_word >> 16);
		Rbuf[index+3] = (uint8_t)(flash_read_word >> 24);
		word_count--;
		index += FLASH_RDWR_TYPE_SIZE_WORD;
	}

	while(residue_bytes) {
		flash_read_word = *(uint32_t*)(addr + index);
		Rbuf[index] = (uint8_t)(flash_read_word);
		index++;
		residue_bytes--;
	}
#endif

#ifdef ESP32_S2_MINI
    return spi_flash_read(addr, Rbuf, size);
#endif


}

#ifdef NUCLEO_F439ZI_EVK_BOARD
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void FLASH_ISR(IRQn_Type Irq_No) 
{
	FLASH_T *pFlash = (FLASH_T*) APP_intr_arg_list[Irq_No];

	if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP) != RESET) { // If end of operation
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);

	} else if(__HAL_FLASH_GET_FLAG(FLASH_FLAG_OPERR) != RESET) { // If operation error
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR);

	} else {

	}
}
#endif

/*******************************************************************************
* Name       : 
* Description: it erases those many sectors whose size is equal or greater than 
                the desired size
* Remarks    : address should be starting sctor address 
*******************************************************************************/
void FLASH_EraseSize(FLASH_T *flash, U32 addr , U32 size)
{

#ifdef NUCLEO_F439ZI_EVK_BOARD
    FLASH_EraseInitTypeDef eraseInit;

    U32 sector_size;

    U32 sector_address;

    U32 total_size;

    total_size = 0x00;

    sector_address = addr;

ERASE_AGAIN:
    sector_size = flash_EraseInit(sector_address, &eraseInit);

    FLASH_Erase(flash, sector_address);

    total_size += sector_size;

    if(total_size < size)
    {
        sector_address +=sector_size;
        goto ERASE_AGAIN;
    }
#endif

#ifdef ESP32_S2_MINI
    
    U32 sector_size = 0x1000;  //4KB sector size
    
    U32 start_sector = addr / 0x1000;
    
    U32 status;
   // TRACE_DBG("%s Entry() addr = %x\tsize = %x\tstart_sector = %x\tstatus = %d\n", __FUNCTION__,addr,size,start_sector,status);
    do 
    {
        status = FLASH_Erase(flash, start_sector);
        //TRACE_DBG("%s Entry()  status = %d\n", __FUNCTION__,status);
        size -=sector_size;
        start_sector++;  
        //TRACE_DBG("size remaining  = %x\n",size);      
    } while( size != 0);
    return status;
#endif

}


/*******************************************************************************
*                         Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
static U32 flash_EraseInit(U32 sector_address, FLASH_EraseInitTypeDef * f)
{
    U32 sector_size  = 128*1024;


	switch(sector_address) {
		case FLASH_SECTOR_0_ADDRESS:
			f->Sector = FLASH_SECTOR_0;
			f->Banks = FLASH_BANK_1;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_1_ADDRESS:
			f->Sector = FLASH_SECTOR_1;
			f->Banks = FLASH_BANK_1;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_2_ADDRESS:
			f->Sector = FLASH_SECTOR_2;
			f->Banks = FLASH_BANK_1;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_3_ADDRESS:
			f->Sector = FLASH_SECTOR_3;
			f->Banks = FLASH_BANK_1;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_4_ADDRESS:
			f->Sector = FLASH_SECTOR_4;
			f->Banks = FLASH_BANK_1;
            sector_size  = 64*1024;
			break;
		case FLASH_SECTOR_5_ADDRESS:
			f->Sector = FLASH_SECTOR_5;
			f->Banks = FLASH_BANK_1;
			break;
		case FLASH_SECTOR_6_ADDRESS:
			f->Sector = FLASH_SECTOR_6;
			f->Banks = FLASH_BANK_1;
			break;
		case FLASH_SECTOR_7_ADDRESS:
			f->Sector = FLASH_SECTOR_7;
			f->Banks = FLASH_BANK_1;
			break;
		case FLASH_SECTOR_8_ADDRESS:
			f->Sector = FLASH_SECTOR_8;
			f->Banks = FLASH_BANK_1;
			break;
		case FLASH_SECTOR_9_ADDRESS:
			f->Sector = FLASH_SECTOR_9;
			f->Banks = FLASH_BANK_1;
			break;
		case FLASH_SECTOR_10_ADDRESS:
			f->Sector = FLASH_SECTOR_10;
			f->Banks = FLASH_BANK_1;
			break;
		case FLASH_SECTOR_11_ADDRESS:
			f->Sector = FLASH_SECTOR_11;
			f->Banks = FLASH_BANK_1;
			break;
		case FLASH_SECTOR_12_ADDRESS:
			f->Sector = FLASH_SECTOR_12;
			f->Banks = FLASH_BANK_2;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_13_ADDRESS:
			f->Sector = FLASH_SECTOR_13;
			f->Banks = FLASH_BANK_2;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_14_ADDRESS:
			f->Sector = FLASH_SECTOR_14;
			f->Banks = FLASH_BANK_2;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_15_ADDRESS:
			f->Sector = FLASH_SECTOR_15;
			f->Banks = FLASH_BANK_2;
            sector_size  = 16*1024;
			break;
		case FLASH_SECTOR_16_ADDRESS:
			f->Sector = FLASH_SECTOR_16;
			f->Banks = FLASH_BANK_2;
            sector_size  = 64*1024;
			break;
		case FLASH_SECTOR_17_ADDRESS:
			f->Sector = FLASH_SECTOR_17;
			f->Banks = FLASH_BANK_2;
			break;
		case FLASH_SECTOR_18_ADDRESS:
			f->Sector = FLASH_SECTOR_18;
			f->Banks = FLASH_BANK_2;
			break;
		case FLASH_SECTOR_19_ADDRESS:
			f->Sector = FLASH_SECTOR_19;
			f->Banks = FLASH_BANK_2;
			break;
		case FLASH_SECTOR_20_ADDRESS:
			f->Sector = FLASH_SECTOR_20;
			f->Banks = FLASH_BANK_2;
			break;
		case FLASH_SECTOR_21_ADDRESS:
			f->Sector = FLASH_SECTOR_21;
			f->Banks = FLASH_BANK_2;
			break;
		case FLASH_SECTOR_22_ADDRESS:
			f->Sector = FLASH_SECTOR_22;
			f->Banks = FLASH_BANK_2;
			break;
		case FLASH_SECTOR_23_ADDRESS:
			f->Sector = FLASH_SECTOR_23;
			f->Banks = FLASH_BANK_2;
			break;
		default:
			return;
			break;
	}

	f->VoltageRange = FLASH_VOLTAGE_RANGE_3;
	f->TypeErase = FLASH_TYPEERASE_SECTORS;
	f->NbSectors = 1; 						/* We will be erasing a single sector at a time */
    return sector_size;
}
#endif
/*******************************************************************************
*                          End of File
*******************************************************************************/
