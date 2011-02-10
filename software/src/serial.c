#include <89c51rd2.h>
#include <string.h> 
#include "include/serial.h"

#define WAIT_TR_END() while ( TI == 0 )
#define XTAL 24000000
#define BAUDRATE 9600
#define XON  0x11
#define XOFF 0x13

void rs_init(void)
{   
   // Disable serial interface
   // interrupt temporary
   ES=0;

   // Interrupt on level 0
   PS=0;

   PCON |= 0x80;
   PCON &= 0xBF;

   // Timer1 configured as baudrate
   // generator:
   TMOD = 0x20;

   // Baudrate = 9600
   TH1 = 0xF3;

   // Serial port in mode 1
   SM1=1;
   SM0=0;
   
   // Receiver disabled
   REN=1;
   TI=0;
   RI=0;

   //T1 started
   TR1=1;

   // Enable serial interface
   // interrupt
   //   ES=1;
}

void put_serial(const char *message)
{
   unsigned char i;

   for (i=0; i<=strlen(message);i++)
   {
      SBUF = message[i];
      WAIT_TR_END();
      TI = 0;
   }      
}

