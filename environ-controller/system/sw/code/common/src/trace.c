
/*******************************************************************************
* (C) Copyright 2019;  WDB Systems India Pvt Ltd, Bangalore
* The attached material and the information contained therein is proprietary
* to WDB Systems India Pvt Ltd and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of WDB Systems 
* India Pvt Ltd.It must be returned to WDB Systems India Pvt Ltd upon its first
* request.
*
*  File Name           : trace.c
*
*  Description         : It contains trace functions
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
#include "trace.h"

#ifdef NUCLEO_F439ZI_EVK_BOARD
#include "main.h"
#endif

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
#ifdef NUCLEO_F439ZI_EVK_BOARD
static UART_HandleTypeDef *trace_huart;
#endif

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
#ifdef NUCLEO_F439ZI_EVK_BOARD
void TRACE_Init(UART_HandleTypeDef *huart)
{
	trace_huart = huart;
}
#endif

/*******************************************************************************
* Name       :
* Description:
* Remarks    :
*******************************************************************************/
#ifdef NUCLEO_F439ZI_EVK_BOARD
int __io_putchar(int ch) 
{
  HAL_UART_Transmit(trace_huart, (U8 *)&ch, 1, 0xFFFF);
  return ch;
}
#endif

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void TRACE_printf_array(U8 *array, U32 size)
{
    U8 *p = array;

    while ((int)size > 0) {
        U8 *q = p; 
        U32 i, c;

        TRACE_PRINTF("%p: ", p);

        for (i = 0;  i < 16;  i++) {
            if (i < (int)size) {
                TRACE_PRINTF("%02X ", *q++ & 0xFF);
            } else {
                TRACE_PRINTF("   ");
            }
            if (i == 7) {  TRACE_PRINTF(" ");  }
        }
        
        TRACE_PRINTF(" |");
        q = p;
        for (i = 0;  i < 16;  i++) {
            if (i < (int)size) {
                c = *q++ & 0xFF;
                if ((c < 0x20) || (c >= 0x7F)) c = '.';
            } else {
                c = ' ';
            }
            TRACE_PRINTF("%d", c);
        }
        TRACE_PRINTF("|\n");
        
        size -= 16;
        p += 16;
    }
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
