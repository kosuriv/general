/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : nvds.h
*
*  Description         : This has driver functions to read/write functions for 
                         profile data to/from flash or serial flash or 
                         sdcard
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  5th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef NVDS_H
#define NVDS_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
#define NVDS_PROFILE_MAGIC_WORD         0xABCD1234

typedef enum NVDS_TagStatus
{
    NVDS_TAG_STATUS_VALID    = (0x1FFFFFFF), /*Indicating latest tag content. */
    NVDS_TAG_STATUS_INVALID  = (0x00000000)  /* Invalid tag. */
} NVDS_TagStatus;


typedef struct Nvds_TagHeader
{
    U32         tag;           /* Unique tag id. */
    U32         length;        /* Length of the tag content. */
    NVDS_TagStatus   status;   /* Status field of this tag, for internal use. */
} Nvds_TagHeader;


typedef struct NVDS
{
     /* Read function handler for flash or serial flash or sd card */
    U32 (*nread)(void *dev, U32 addr,  U8 *buf ,U32 length ); 

    /* Write function handler for flash or serial flash or sd card */
    U32 (*nwrite)(void *dev, U32 addr, U8 *buf, U32 length );  

    /* Erase function handler for flash. */
    U32 (*nerase)(void *dev, U32 addr, U32 size);                

    void *dev;  /* Pointer to flash device or serial flash device or sd card*/

    U32 size;   /* Total size allocated for profile */

    U32 startLocation; /* Starting location of NVDS in memory. */

    U32 endLocation;   /* Ending location of NVDS in memory. */

    U32 nextWriteLocation; /* Location in memory to write the next tag. */

} NVDS;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void NVDS_Init(void *this,
               U32 (*nread)(void *,  U32, U8*, U32),
               U32 (*nwrite)(void *, U32, U8*, U32),
               U32 (*nerase)(void *, U32, U32),
               void *dev,
               U32   size,
               U32   startLocation);

U32 NVDS_InitMemory(void *this);

U32 NVDS_WriteTag(void *this, U32 tag, U32 len, U8 *buf);

U32 NVDS_ReadTag(void *this, U32 tag, U8 *buf);

BOOLEAN NVDS_IsPresent(void *this);

U8* NVDS_ReadTagAddr(void *this, U32 tag);


#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
