/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : profile.h
*
*  Description         : This has all declarations for profile module
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   Venu Kosuri  5th Aug 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef PROFILE_H
#define PROFILE_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include "data_types.h"
#include "nvds.h"


/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/
#define PROFILE_MAX_TAG  7  // No of max tags in any profile


#define PROFILE_MAX_TAG_LENGTH                  (1024)


typedef enum PROFILE_TYPE
{
    PROFILE_FLASH,              /*  Profile in PROGRAM FLASH */
    PROFILE_SERIAL_FLASH,       /*  Profile in SERIAL_FLASH */
    PROFILE_SD_CARD,            /*  Profile in SDRAM */
    PROFILE_SRAM,               /*  Profile in SRAM */
    PROFILE_BKP_SRAM            /*  Profile in BACK UP SRAM */

} PROFILE_TYPE;

typedef struct PROFILE_TAG_INFO
{
    U32 tagId;             /* Unique tag ID. */
    U32 tagSize;           /* Tag size in bytes. */
} PROFILE_TAG_INFO;

/* Prototype for profile init function handler */
typedef void (*PROFILE_INIT) (void *media,
                              U32 (*nread)(void *,  U32, U32, U8 *),
                              U32 (*nwrite)(void *, U32, U32, U8 *),
                              U32 (*nerase)(void *, U32, U32),
                              void *dev,
                              U32 size,
                              U32 startLocation);

/*  Prototype for profile erase function handler */
typedef U32 (*PROFILE_ERASE) (void *media);

/* Prototype for profile tag write function handler.*/
typedef U32 (*PROFILE_WRITE) (void *media, U32 tag, U32 length, U8 *buf);

/* Prototype for profile tag read function handler.*/
typedef U32 (*PROFILE_READ) (void *media, U32 tag, U8 *buf);

/*Prototype for profile validation function handler.*/
typedef BOOLEAN (*PROFILE_IS_VALID) (void *media);

/* Prototype for reading address of tag in the profile */
typedef U8* (*PROFILE_READ_ADDRESS) (void *media, U32 tag);


typedef struct PROFILE_STORAGE
{
    PROFILE_INIT pinit;         /* Function handler to init a profile. */
    PROFILE_ERASE perase;       /* Function handler to erase a profile. */
    PROFILE_WRITE pwrite;       /* Function handler to write a tag in a profile. */
    PROFILE_READ pread;         /* Function handler to read a tag from a profile. */
    PROFILE_READ_ADDRESS pread_addr;           /* Function handler to read a tag from a profile. */
    PROFILE_IS_VALID isValid;    /* Function handler to check if the profile has a valid signature. */
    union
    {
        NVDS pFlash;    /* Using Program Flash as stroage media */
        NVDS pSerilFlash;        /* Using Serial Flash as storage media */
        NVDS pSdCard;        /* Using Serial Flash as storage media */
        void *pSRAM;              /* Using SRAM as storage media TBD */
        void *pBKPSRAM;              /* Using BACK UP SRAM as storage media TBD*/

    } media;
} PROFILE_STORAGE;


typedef struct PROFILE
{
    PROFILE_TAG_INFO tags[PROFILE_MAX_TAG];     
    U32              noOfTags;        /* Number of actual tags in a profile */
    PROFILE_TYPE     type;                
    PROFILE_STORAGE  storage; 
} PROFILE;

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/
void PROFILE_Init(PROFILE           *profile,
                  PROFILE_TYPE      type,
                  U32               size,
                  U32               StartAddress);

void PROFILE_Copy(PROFILE *dest, PROFILE *src);

U32  PROFILE_WriteTag(PROFILE *profile, U32 tag, U32 length, U8 *buffer);

U32  PROFILE_ReadTag(PROFILE *profile, U32 tag, U8 *buffer);

void PROFILE_AddTag(PROFILE *profile, U32 tag, U16 length);

U32  PROFILE_Erase(PROFILE *profile);

BOOLEAN  PROFILE_is_valid(PROFILE *profile);

U8* PROFILE_ReadTagAddr(PROFILE *profile, U32 tag);

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
