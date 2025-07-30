/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : profile.c
*
*  Description         : This implements profile functions
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  5th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "profile.h"
#include "nvds.h"
#include "flash.h"
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
* Name       : PROFILE_Init
* Description: Initializes a profile with specified type, size, start address
               with appropriate storage media functions .

* Remarks    : Appropriate media driver functions should be provided
*******************************************************************************/
void PROFILE_Init(PROFILE           *profile,
                  PROFILE_TYPE      type,
                  U32               size,
                  U32               StartAddress)
{

    profile->type = type;
    profile->noOfTags = 0;

    if (profile->type == PROFILE_FLASH) {

        profile->storage.pinit = NVDS_Init;
        profile->storage.perase = NVDS_InitMemory;
        profile->storage.pwrite = NVDS_WriteTag;
        profile->storage.pread = NVDS_ReadTag;
        profile->storage.isValid = NVDS_IsPresent;
        profile->storage.pread_addr = NVDS_ReadTagAddr;

        profile->storage.pinit(&profile->storage.media,
                               FLASH_Read,
                               FLASH_Write,
                               FLASH_EraseSize,
                               NULL,
                               size,
                               StartAddress);
    }
    else if (profile->type == PROFILE_SERIAL_FLASH) {
        //TBD   
    }
    else if  (profile->type == PROFILE_SD_CARD) {
        //TBD   
    }
    else if (profile->type == PROFILE_SRAM) {
        //TBD   
    }
    else
    {
        //TBD  profile->type == PROFILE_BKP_SRAM
    }

}

/*******************************************************************************
* Name       : PROFILE_Copy
* Description: For every tag in dest profile, search if it exists in src profile.
               If yes, the tag in the src profile is copied into dest profile.
               Otherwise, the tag is ignored.
* Remarks    : 
*******************************************************************************/
void PROFILE_Copy(PROFILE *dest, PROFILE *src)
{
    U32 i;
    U8  *buf = malloc(PROFILE_MAX_TAG_LENGTH);

    if(buf == NULL){
        return;
    }

    /* Iterate through all tags in dest */
    for (i = 0; i < dest->noOfTags; i++)
    {  
        if (PROFILE_ReadTag(src, dest->tags[i].tagId,buf) == STATUS_OK) 
        {

            PROFILE_WriteTag(dest, dest->tags[i].tagId,dest->tags[i].tagSize, buf);
        }
    }
    free(buf);
}


/*******************************************************************************
* Name       : 
* Description: It write tag to the profile 
* Remarks    : 
*******************************************************************************/
U32  PROFILE_WriteTag(PROFILE *profile, U32 tag, U32 length, U8 *buffer)
{
    U32 i;

    for (i = 0; i < profile->noOfTags; i++) 
    {
        if (tag == profile->tags[i].tagId)
        {
            return profile->storage.pwrite(&profile->storage.media, tag,
                               length, buffer);
        }
    }

    return STATUS_ERR;
}

/*******************************************************************************
* Name       : PROFILE_ReadTag
* Description: It read tag from the profile 
* Remarks    : 
*******************************************************************************/
U32  PROFILE_ReadTag(PROFILE *profile, U32 tag, U8 *buffer)
{
    U32 i;
    for (i = 0; i < profile->noOfTags; i++) 
    {
        if (tag == profile->tags[i].tagId) 
        {
            return profile->storage.pread(&profile->storage.media, tag, buffer);
        }
    }

    return STATUS_ERR ;
}

/*******************************************************************************
* Name       : PROFILE_AddTag
* Description: It adds a tag to the profile data structure. The content of the
               tag needs be written to the profile by calling PROFILE_WriteTag
* Remarks    : 
*******************************************************************************/
void PROFILE_AddTag(PROFILE *profile, U32 tag, U16 length)
{
    U32 i;
    for (i = 0; i < profile->noOfTags; i++) 
    {
        if (tag == profile->tags[i].tagId) 
        {
            return;
        }
    }
    
    profile->tags[i].tagId = tag;
    profile->tags[i].tagSize = length;
    profile->noOfTags++;    
   // TRACE_DBG("TAG_ID = %d TAG_LENGTH = %d \n" ,tag , length);
}

/*******************************************************************************
* Name       : PROFILE_Erase
* Description: The underlying storage erase function handler is called to perform 
               the actual erase of the storage memory section.

* Remarks    : Using this function is tricky for FLASH stored  profiles, when multiple 
               profiles reside on the same section of the flash, meaning erasing one 
               profile implies the erasure of all profiles on the same section.
               this function, and restored afterwards.
*******************************************************************************/
U32  PROFILE_Erase(PROFILE *profile)
{
    return profile->storage.perase(&profile->storage.media);
}

/*******************************************************************************
* Name       : PROFILE_is_valid
* Description: Validates a profile by calling the validation function of the
               underlying storage.
* Remarks    : 
*******************************************************************************/
BOOLEAN  PROFILE_is_valid(PROFILE *profile)
{
    return profile->storage.isValid(&profile->storage.media);
}


/*******************************************************************************
* Name       : 
* Description: reads the address of the tag inside the profile.
* Remarks    : 
*******************************************************************************/
U8* PROFILE_ReadTagAddr(PROFILE *profile, U32 tag)
{
    U32 i;

    for (i = 0; i < profile->noOfTags; i++)
    {
        if (tag == profile->tags[i].tagId)
        {
            return profile->storage.pread_addr(&profile->storage.media, tag);
        }
    }
    return NULL;
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
