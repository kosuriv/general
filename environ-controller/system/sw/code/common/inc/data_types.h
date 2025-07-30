/*******************************************************************************
* (C) Copyright 2020;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : data_types.h
*
*  Description         : It contains generic data declarations
*
*  Change history      :
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri  2nd Jul 2020      1.1               Initial Creation
*  
*******************************************************************************/
#ifndef DATA_TYPES_H
#define DATA_TYPES_H

/*******************************************************************************
*                          Include Files
*******************************************************************************/

/*******************************************************************************
*                          C++ Declaration Wrapper
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
*                          Type & Macro Declarations
*******************************************************************************/

#define PVOID          void*

#define UINT32         unsigned long

#define INT32          long

#define UINT16         unsigned short

#define INT16          short

#define UINT8          unsigned char

#define INT8           char

#define ERRORCODE      UINT32

#define CHAR           char

#define UCHAR          unsigned char

#define PCHAR          CHAR*

#define PUCHAR         UCHAR*

#define BYTE           UCHAR

#define BOOLEAN        UCHAR

#define TRUE   1

#define FALSE  0

//#define SUCCESS  1

//#define FAILURE  0

    /* Bit positions */
#define BIT_0       0x0001
#define BIT_1       0x0002
#define BIT_2       0x0004
#define BIT_3       0x0008
#define BIT_4       0x0010
#define BIT_5       0x0020
#define BIT_6       0x0040
#define BIT_7       0x0080
#define BIT_8       0x0100
#define BIT_9       0x0200
#define BIT_10      0x0400
#define BIT_11      0x0800
#define BIT_12      0x1000
#define BIT_13      0x2000
#define BIT_14      0x4000
#define BIT_15      0x8000
#define BIT_24      0x01000000
#define BIT_28      0x10000000
#define BIT_29      0x20000000
#define BIT_30      0x40000000
#define BIT_31      0x80000000

#define SWAP_BYTES(z)   ( (z<<8) | ( (z>>8)&0xff ) )    /* Swap Bytes */
/* Move Byte 0->Byte 3 , Byte 1->Byte 2 , Byte 2->Byte 1 , Byte 3->Byte 0 */

#define SWAP_BYTES_LONG(z) ( (z<<24) | ((z<<8)&0xff0000) | ((z>>8)&0xff00) | ((z>>24)&0xff) )


#define BYTE_16_LITTLE_ENDIAN_TO_INT(string ,number)\
                                    ( number = string[0]+ string[1]*0x100 )

#define BYTE_32_LITTLE_ENDIAN_TO_INT(string ,number)\
                                    ( number = string[0]+string[1]*0x100+ string[2]*0x10000+ string[3]*0x1000000)

#define INT_TO_BYTE_16_LITLE_ENDIAN(number,string)\
                                    ( string[0]=number%0x100,\
                                      string[1]=number/0x100)

#define INT_TO_BYTE_32_LITLE_ENDIAN(number,string)\
                                    ( INT_TO_BYTE_16_LITLE_ENDIAN(number%0x10000,string),\
                                      INT_TO_BYTE_16_LITLE_ENDIAN(number/0x10000,(string+2)))


#define BYTE_16_BIG_ENDIAN_TO_INT(string ,number)\
                                    ( number = string[1]+ string[0]*0x100 )

#define BYTE_32_BIG_ENDIAN_TO_INT(string ,number)\
                                    ( number = string[3]+string[2]*0x100+ string[1]*0x10000+ string[0]*0x1000000)

#define INT_TO_BYTE_16_BIG_ENDIAN(number,string)\
                                    ( string[1]=number%0x100,\
                                      string[0]=number/0x100)

#define INT_TO_BYTE_32_BIG_ENDIAN(number,string)\
                                    ( INT_TO_BYTE_16_BIG_ENDIAN(number%0x10000,(string+2)),\
                                      INT_TO_BYTE_16_BIG_ENDIAN(number/0x10000,string))

#ifndef FALSE
    #define FALSE    0
#elif (FALSE != 0)
    #error FALSE != 0
#endif

#ifndef TRUE
    #define TRUE     1
#elif (TRUE != 1)
    #error TRUE != 1
#endif


#ifndef NULL
#define NULL       ((void*)0)
#endif


#define U8 unsigned char    

#define S8 signed char    

#define U16  unsigned short 

#define S16  signed short   

#define U32  unsigned long   

#define S32    signed long   

#define  CO_BOOLEAN U8   

#define FLOAT float

typedef struct U64
{
    U32 l;
    U32 h;
} U64;

typedef struct CO_DELAY
{
    U64 Value;
    CO_BOOLEAN Negative;
} CO_DELAY;


typedef struct BUFFER8
{
    UINT32  size;
    UINT8  *pBuf;
} BUFFER8;

/* Definition of a linked list with byte bufferss. */
typedef struct BUFFER8LISTELEMENT
{
    BUFFER8                    buffer;
    struct BUFFER8LISTELEMENT *pNextListItem;
} BUFFER8LISTELEMENT;

/* Definition of a halfword buffer. */
typedef struct BUFFER16
{
    UINT32  size;
    UINT16 *pData;
} BUFFER16;

/* Definition of a linked list with halfword bufferss. */
typedef struct BUFFER16LISTELEMENT
{
    BUFFER16                    buffer;
    struct BUFFER16LISTELEMENT *pNextListItem;
} BUFFER16LISTELEMENT;

/* Definition of a byte buffer. */
typedef struct BUFFER32
{
    UINT32  size;
    UINT32 *pData;
} BUFFER32;

/* Definition of a linked list with byte bufferss. */
typedef struct BUFFER32LISTELEMENT
{
    BUFFER32                   buffer;
    struct BUFFER8LISTELEMENT *pNextListItem;
} BUFFER32LISTELEMENT;



typedef void (*CALLBACK_FCT)(void *pThis, const void *pParam) ;

typedef struct CALLBACK
{
    void *pArg;
    CALLBACK_FCT pFunction;
} CALLBACK;


typedef void (*CALLBACK32_FCT)(void *pThis, UINT32 value) ;

/* Callback class using a 32 bit return parameter */
typedef struct CALLBACK32
{
    void *pArg;
    CALLBACK32_FCT pFunction;
} CALLBACK32;


typedef void (*FUNC_PTR)(void);

typedef void (*Callback) (void);

typedef void (*Callback_1_Arg) (void *arg);

typedef void (*FUNC_PTR2)(U32);



#define STATUS_OK  (U32)0x1

#define STATUS_ERR (U32)0x0

/*******************************************************************************
*                          Extern Data Declarations
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Prototypes
*******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
*                          End of File
*******************************************************************************/
