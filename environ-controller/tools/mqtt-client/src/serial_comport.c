/*******************************************************************************
* (C) Copyright 2018;  WDBSystems, Bangalore
* The attached material and the information contained therein is proprietary
* to WDBSystems and is issued only under strict confidentiality arrangements.
* It shall not be used, reproduced, copied in whole or in part, adapted,
* modified, or disseminated without a written license of WDBSystems.           
* It must be returned to WDBSystems upon its first request.
*
*  File Name           : serial_comport.c
*
*  Description         : It contains routines to communicate with UART/RS232 
*                        interface  
*
*  Change history      : 
*
*     Author        Date           Ver                 Description
*  ------------    --------        ---   --------------------------------------
*   venu kosuri   27th Aug 2018    1.1               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>


#include "serial_comport.h"

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
static void serial_port_comm_init(U32 baud_rate, U8* ComPortName);

/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
static int serial_app_ComHandle;  /* serial communication handle to the device */ 

/*******************************************************************************
*                          Extern Data Definitions
*******************************************************************************/

/*******************************************************************************
*                          Extern Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description: comPort == "/dev/ttyS1" or "/dev/ttyS0" etc ... baudrate = 9600 or  
115200 etc ...
* Remarks    : 
*******************************************************************************/
void SERIAL_COMPORT_Init(U32 baud_rate, U8* ComPortName)
{  
    unsigned int baud = 0; 
    serial_port_comm_init(baud_rate,ComPortName);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SERIAL_COMPORT_DeInit()
{
    close(serial_app_ComHandle);
}
/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SERIAL_COMPORT_Read(U8* buf, U32 length)
{
    read(serial_app_ComHandle, buf, length);
}

/*******************************************************************************
* Name       : 
* Description: 
* Remarks    : 
*******************************************************************************/
void SERIAL_COMPORT_Write(U8* buf, U32 length)
{
    write(serial_app_ComHandle , buf, length);
}


 
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/

/*******************************************************************************
* Name       : 
* Description:  
* Remarks    : 
*******************************************************************************/
static void serial_port_comm_init(U32 baud_rate, U8* ComPortName)
{
    int cmdLen =0, rdLen =0, i =0,wrLen=0,respLen=0;
    unsigned int baud =0;
    struct termios oldio,newtio;
    char ch;
    unsigned char ComPort[50];

    printf("app_s2w_comm_init() = %s\n",ComPortName);

    if (strcmp(ComPortName,"COM1") == 0)
    {
#if 1
        strcpy(ComPort,"/dev/ttyUSB0");  ///it is for raspberry zero board
#else
     //  strcpy(ComPort,"/dev/serial0");
        strcpy(ComPort,"/dev/ttyAMA0"); //it is for raspberry 2 board
#endif
        printf("ComPort = %s\n",ComPort);
    }
    else if (strcmp(ComPortName,"COM2") == 0)
    {
        strcpy(ComPort,"/dev/ttyS1");
        printf("ComPort = %s\n",ComPort);
    }

    else if (strcmp(ComPortName,"COM3") == 0)
    {
        strcpy(ComPort,"/dev/ttyS2");
        printf("ComPort = %s\n",ComPort);
    }

    else if (strcmp(ComPortName,"COM4")== 0)
    {
        strcpy(ComPort,"/dev/ttyS3");
        printf("ComPort = %s\n",ComPort);
    }

    else if (strcmp(ComPortName,"COM5")== 0)
    {
        strcpy(ComPort,"/dev/ttyS4");
        printf("ComPort = %s\n",ComPort);
    }   
    else if (strcmp(ComPortName,"COM6")== 0)
    {
        strcpy(ComPort,"/dev/ttyS5");
        printf("ComPort = %s\n",ComPort);
    }
    else if (strcmp(ComPortName,"COM7")== 0)
    {
        strcpy(ComPort,"/dev/ttyS6");
        printf("ComPort = %s\n",ComPort);
    }
    else if (strcmp(ComPortName,"COM8") == 0)
    {
        strcpy(ComPort,"/dev/ttyS7");
        printf("ComPort = %s\n",ComPort);
    }
    else
    {
        printf("NO COM PORT SELECTED \n");
        exit(0);
    }    


    printf("Inside s2w_pc_init() = %s\n",ComPort);

   // baud_usr = baud_rate;
    serial_app_ComHandle = open(ComPort, O_RDWR | O_NOCTTY | O_NDELAY );

    if(serial_app_ComHandle > 0)
    {
        printf(" %s:Serial port opened successfully \n",ComPort);
    }
    else
    {
        printf("...Not able to open Serial port :%s %d \n ",ComPort, serial_app_ComHandle);
        perror("...Not able to open Serial port --   ");
        exit (0);
    }

    fcntl(serial_app_ComHandle, F_SETFL, 0);

    switch(baud_rate)
    {
    case 9600:
        baud = B9600;
        break;
    case 19200:
        baud = B19200;
        break;
    case 38400:
        baud = B38400;
        break;
    case 57600:
        baud = B57600;
        break;
    case 115200:
        baud = B115200;
        break;
    case 230400:
        baud = B230400;
        break;
    case 460800:
        baud = B460800;
        break;
    case 921600:
        baud = B921600;
        break;
    default:
        baud = B9600;
        break;
    }

    newtio.c_cflag |= CLOCAL | CREAD;
    newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    newtio.c_cflag &= ~PARENB;
    newtio.c_cflag &= ~CSTOPB;
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;

    cfsetospeed(&newtio, baud);

    cfsetispeed(&newtio, baud);



    newtio.c_iflag |= IGNPAR;
    newtio.c_iflag &= ~(IXON | IXOFF | IXANY);

    newtio.c_cc[VMIN] |= 0;
    newtio.c_cc[VTIME] |= 1000;


    /* set the port settings  */
    tcsetattr(serial_app_ComHandle,TCSANOW, &newtio);

}
 
/*******************************************************************************
*                          End of File
*******************************************************************************/
