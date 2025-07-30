/*******************************************************************************
* (C) Copyright 2019;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : nvds.c
*
*  Description         : This has driver functions to read/write functions for 
                         profile data to/from flash or serial flash or 
                         sdcard
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  6th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "nvds.h"
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
#define NVDS_END_MARKER_TAG             (U32)0xFFFFFFFF

#define NVDS_MAGIC_WORD_LENGTH          sizeof(NVDS_PROFILE_MAGIC_WORD)

/*******************************************************************************
*                          Static Function Prototypes
*******************************************************************************/
static void nvds_create_tag(NVDS *nvds,
                            U32 tag,
                            NVDS_TagStatus status,
                            U32 len,
                            U8 *buffer,
                            U32 addr);

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
void NVDS_Init(void *this,
               U32 (*nread)(void *, U32,  U8 *,U32),
               U32 (*nwrite)(void *, U32, U8 *,U32),
               U32 (*nerase)(void *, U32, U32),
               void *dev,
               U32   size,
               U32   startLocation)
{
    NVDS *nvds = (NVDS*)this;
    nvds->nread = nread;
    nvds->nwrite = nwrite;
    nvds->nerase = nerase;
    nvds->dev = dev;
    nvds->size = size;
    nvds->startLocation = startLocation;
    nvds->nextWriteLocation = startLocation;
    nvds->endLocation = startLocation + size;

    // search for the next write location
    if (NVDS_IsPresent(nvds))
    {
        Nvds_TagHeader tagHeader;
        
        startLocation += NVDS_MAGIC_WORD_LENGTH;
        nvds->nread(nvds->dev, startLocation,
                (U8 *)(&tagHeader),sizeof(Nvds_TagHeader));

        // Search till the end of existing tags
        while (tagHeader.tag != NVDS_END_MARKER_TAG)
        {
            startLocation += sizeof(Nvds_TagHeader) + tagHeader.length;
            nvds->nread(nvds->dev, startLocation,
                        (U8 *)(&tagHeader),sizeof(Nvds_TagHeader));
        };
        
        nvds->nextWriteLocation = startLocation;
    }
  //  TRACE_DBG("NVDS Next Location = %x\n" ,nvds->nextWriteLocation);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 NVDS_InitMemory(void *this)
{
    NVDS *nvds = (NVDS*)this;
    return nvds->nerase(nvds->dev, nvds->startLocation, nvds->size);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 NVDS_WriteTag(void *this, U32 tag, U32 len, U8 *buf)
{
    U32 location;
    U32 magicWord = NVDS_PROFILE_MAGIC_WORD;
    Nvds_TagHeader tagHeader;
    NVDS *nvds = (NVDS *)this;

    location  = nvds->startLocation;
    if (!NVDS_IsPresent(nvds))
    {
         // First write, hence write magic word
        nvds->nwrite(nvds->dev, location, 
                    (U8 *)(&magicWord),NVDS_MAGIC_WORD_LENGTH);
        nvds->nextWriteLocation += NVDS_MAGIC_WORD_LENGTH;

        // Write Tag
        nvds_create_tag(nvds, tag, NVDS_TAG_STATUS_VALID,
                       (U32)len, buf, nvds->nextWriteLocation);
        nvds->nextWriteLocation += len + sizeof(Nvds_TagHeader);
    }
    else 
    {
        if (len + sizeof(Nvds_TagHeader) 
            < (nvds->endLocation - nvds->nextWriteLocation))
        {
            // Size is suffecient to append at the end
            location += NVDS_MAGIC_WORD_LENGTH;
            do {
                nvds->nread(nvds->dev, location,
                         (U8 *)(&tagHeader),sizeof(Nvds_TagHeader));
                if (tagHeader.tag == tag)
                {
                    if (tagHeader.status != NVDS_TAG_STATUS_VALID ) 
                    {
                        // Use this location to go to next valid tag
                        location = tagHeader.status;
                    } 
                    else
                    {
                        // This is valid tag , make it point to next stored location
                        uint32_t newStatus = nvds->nextWriteLocation;
                        nvds->nwrite(nvds->dev, location + offsetof(Nvds_TagHeader, status),
                                    (U8 *)(&newStatus),sizeof(newStatus));
                        break;
                    }          
                }
                else
                {
                    location += tagHeader.length + sizeof(Nvds_TagHeader);
                }          
            } while (tagHeader.tag != NVDS_END_MARKER_TAG);

            // Write  the tag
            location = nvds->nextWriteLocation;
            nvds_create_tag(nvds, tag, NVDS_TAG_STATUS_VALID,
                           (uint16_t)len, buf, nvds->nextWriteLocation);

            // Update next write location
            nvds->nextWriteLocation += len + sizeof(Nvds_TagHeader);
        } 
        else
        {
             // Space is not enough , return media full
             return STATUS_ERR;
        }        
    }

    return STATUS_OK;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U32 NVDS_ReadTag(void *this, U32 tag, U8 *buf)
{
    U32 location;
    Nvds_TagHeader tagHeader;
    NVDS *nvds = (NVDS *)this;
   
    if (!NVDS_IsPresent(nvds))
    {
         return STATUS_ERR;
    }
        
    location  = nvds->startLocation + NVDS_MAGIC_WORD_LENGTH;
    do {
        nvds->nread(nvds->dev, location, 
                (uint8_t *)(&tagHeader),sizeof(Nvds_TagHeader));

        if (tagHeader.tag == tag )
        {
            if (tagHeader.status != NVDS_TAG_STATUS_VALID) 
            {
                // Use this location to go to next valid tag
                location =  tagHeader.status;
            }
            else 
            {
                // This is valid tag , read & send it */
                location += sizeof(Nvds_TagHeader);
                return nvds->nread(nvds->dev, location,buf,tagHeader.length );
            }
        } 
        else 
        {
            location += sizeof(Nvds_TagHeader) + tagHeader.length;
        }
    } while (tagHeader.tag != NVDS_END_MARKER_TAG);

    // Tag not found
    return STATUS_ERR;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
BOOLEAN NVDS_IsPresent(void *this)
{
    U32 location;
    U32 magicWord;
    NVDS *nvds = (NVDS *)this;
   
    location  = nvds->startLocation;
   // TRACE_DBG("NVDS READ MAGIC Location = %x\n" ,location);
    nvds->nread(nvds->dev, location, 
             (U8 *)(&magicWord),NVDS_MAGIC_WORD_LENGTH);
    return magicWord == NVDS_PROFILE_MAGIC_WORD;
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
U8* NVDS_ReadTagAddr(void *this, U32 tag)
{
    uint32_t location;
    Nvds_TagHeader tagHeader;
    NVDS *nvds = (NVDS *)this;

    if (!NVDS_IsPresent(nvds)) {
         return NULL;
    }

    location  = nvds->startLocation + NVDS_MAGIC_WORD_LENGTH;
    do {
        nvds->nread(nvds->dev, location,
                (uint8_t *)(&tagHeader),sizeof(Nvds_TagHeader));

        if (tagHeader.tag == tag ) {
            if (tagHeader.status != NVDS_TAG_STATUS_VALID) {
                // Use this location to go to next valid tag
                location =  tagHeader.status;
            } else {
                // This is valid tag , read & send it */
                location += sizeof(Nvds_TagHeader);
                return (uint8_t*)location;
            }
        } else {
            location += sizeof(Nvds_TagHeader) + tagHeader.length;
        }
    } while (tagHeader.tag != NVDS_END_MARKER_TAG);

    // Tag not found
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
static void nvds_create_tag(NVDS *nvds,
                            U32 tag,
                            NVDS_TagStatus status,
                            U32 len,
                            U8 *buffer,
                            U32 addr)
{
    Nvds_TagHeader tagHeader;

    tagHeader.tag = tag ;
    tagHeader.status = status;
    tagHeader.length = len;
    nvds->nwrite(nvds->dev, addr,(uint8_t *)&tagHeader,sizeof(tagHeader) );
    addr += sizeof(tagHeader);
    nvds->nwrite(nvds->dev, addr, buffer, len);
}


/*******************************************************************************
*                          End of File
*******************************************************************************/
