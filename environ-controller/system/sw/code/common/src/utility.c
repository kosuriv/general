/*******************************************************************************
* (C) Copyright 2014;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : utility.c
*
*  Description         : It contains utility functions
*
*  Change history      : $Id$
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   20th Jan 2014    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "utility.h"

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
* Name       : UTILITY_PrintBuffer
* Description: This function prints the content of provided buffer either in 
               HEX mode or Chracter mode
* Remarks    : 
*******************************************************************************/
void UTILITY_PrintBuffer(U8* buf,U32 len, U8 mode)
{
    int  i = 0x00 ;

    printf(" Buffer Length To Print = %d\n  ",len);

    for( i=0; i<len; i++)
    {
        if ( mode == UTILITY_PRINT_HEX_MODE )
        {
            printf("%x\t", buf[i]);
        }
        else
        {
             printf("%c\t", buf[i]);
        }

        if( ((i+1) % 16) == 0x00)
        {
            printf("\n");
        }
    }
    printf("\n ");
}



/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
U32  UTILITY_convert_time_duration_to_str(U32 time_in_sec , U8* pStr)
{
    mTIME t;

    U32  value_left;

    t.HH = time_in_sec / 3600 ;

    value_left = time_in_sec  % 3600;

    t.MM = value_left / 60;
 
    t.SS = value_left %  60 ;

    return sprintf((char*)pStr,"%d:%d:%d", t.HH, t.MM ,t.SS);
}



/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
S32 UTILITY_date_cmp(mDATE* d1, mDATE* d2)
{
	S32 days;
	U32 month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31,30, 31, 30, 31 };
	U32 i;


    if((d1->DD == d2->DD) && (d1->MM == d2->MM) && (d1->YY == d2->YY))
    {
       return 0;
    }
    else if ((d1->YY > d2->YY) ||
             ((d1->YY == d2->YY) && (d1->MM > d2->MM)) ||
             ((d1->YY == d2->YY) && (d1->MM == d2->MM) && (d1->DD > d2->DD)))
    {
        return -1;
    }
    else 
    {
    	if((d1->YY == d2->YY) && (d1->MM == d2->MM))
    	{
    		days = d2->DD-d1->DD;
    		printf(" No Of Days = %d\n  ",days);
    	}
    	else if ((d1->YY == d2->YY) && (d1->MM != d2->MM))
    	{
    		/* year same , not months */
    		if (!(d1->YY %4))
    		{
    			month[2] = 29;
    		}
    		/* calculate pending days for the current month */
    		days = month[d1->MM]-d1->DD ;
    		d1->MM++;

    		/* calculate days till the months matched*/
    	    for( i=d1->MM ; i < d2->MM; i++)
    	    {
    	    	days +=month[i];
    	    }
    	    /* add balance days of the last month */
    	    days +=d2->DD;
    	}
    	else
    	{
    		/* year, months not same */
    		if (!(d1->YY %4))
    		{
    			month[2] = 29;
    		}
    		/* calculate pending days for the current year */
    		days = month[d1->MM]-d1->DD ;
    		d1->MM++;

    		while(d1->MM  <= 12 )
    		{
    			days += month[d1->MM];
    			d1->MM++;
    		}
    		d1->YY++;

    		/* calculate days till the years matched*/
    		while(d1->YY != d2->YY)
    		{
    			days +=365;
    			if(!(d1->YY %4 ))
    			{
    				days++;
    			}
    			d1->YY++;
    		}
            if(!(d1->YY %4 ))
            {
            	month[2]=29;
            }
            /* calculate pending days for the last year */
            for( i=0; i < d2->MM; i++)
            {
            	days +=month[i];
            }
            days +=d2->DD;
    	}
    	return days;
    }
}


\
/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
void UTILITY_convert_time_str(U8* time_str, mTIME* time)
{
    U8* p;
    U8* q;

    if( time_str)
    {
        /* Hour */
        p = time_str;
        q = (U8*)strchr(p,':');
        *q =0x00;
        time->HH = atoi(p); 
        *q = ':';

        /* minute */
        q++;
        p = q;
        q = (U8*)strchr(p,':');
        *q =0x00;
        time->MM = atoi(p); 
        *q = ':';

        /* second */
        q++;
        p = q;
        time->SS = atoi(p); 
    }   

}

/*******************************************************************************
* Name       :
* Description:
* Remarks    : 
*******************************************************************************/
void UTILITY_convert_date_str(U8* date_str, mDATE* date)
{
    U8* p;
    U8* q;

    if( date_str)
    {
        /* Hour */
        p = date_str;
        q = (U8*)strchr(p,'/');
        *q =0x00;
        date->DD = atoi(p); 
        *q = '/';

        /* minute */
        q++;
        p = q;
        q = (U8*)strchr(p,'/');
        *q =0x00;
        date->MM = atoi(p); 
        *q = '/';

        /* second */
        q++;
        p = q;
        date->YY = atoi(p); 
    }   
}

 
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/
 
/*******************************************************************************
*                          End of File
*******************************************************************************/
