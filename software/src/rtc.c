/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/rtc.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: rtc.c,v $
**  Revision 1.5  2007/01/12 15:38:26  tomasz
**  modifications to make sdcc 2.6.0 compiler happy about warnings. Doesn't solve new compiler problem
**
**  Revision 1.4  2004/11/19 19:33:58  tomasz
**  Aggregate commit after many changes
**
**  Revision 1.3  2004/01/28 09:50:10  tomasz
**  change the way the year is displayed
**
**  Revision 1.2  2004/01/19 13:47:34  tomasz
**  Library rewritten according to dallas example
**
**  Revision 1.1  2004/01/12 22:28:46  tomasz
**  library for use 8051 and dallas RTC ds1203
**
**
**  $Id: rtc.c,v 1.5 2007/01/12 15:38:26 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "89c51rd2.h"
#include "rtc.h"

#define RTC_SEC    0x80    // second register
#define RTC_MIN    0x82    // minutes register
#define RTC_HOUR   0x84    // hour register
#define RTC_DATE   0x86    // date register
#define RTC_MONTH  0x88    // month register
#define RTC_DOW    0x8A    // day of week register
#define RTC_YEAR   0x8C    // year register

#define RTC_CTRL   0x8E    // control register
#define RTC_TC     0x90    // trickle charger register
#define RTC_BURST  0xBE    // clock burst address
#define RAM_BURST  0xFE    // memory burst


#define SET_RST P1_7 = 1;
#define RESET_RST P1_7 = 0;

#define SET_CLK P1_6 = 1;
#define RESET_CLK P1_6 = 0;

#define SET_IO( value ) P1_5 = value;
#define READ_IO P1_5;

#define IO   P1_5
#define SCLK P1_6
#define RSTB P1_7

typedef unsigned char uchar;

code char * code month[] = {"Jan ","Feb ","Mar ","Apr ","May ","Jun ","Jul ","Aug ","Sep ","Oct ","Nov ","Dec "};
code char * code day[]   = {"Sun ","Mon ","Tue ","Wed ","Thu ","Fri ","Sat "};
code char monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31};

extern struct tm xdata system_time;

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

/* ----------------------------------------------------------------------- */
void reset_3w()
{
   SCLK = 0;
   RSTB = 0;
   RSTB = 1;
}
/* ----------------------------------------------------------------------- */
void wbyte_3w(uchar W_Byte)
{
   uchar i;
   uchar temp;

   temp = W_Byte;
   
   for(i = 0; i < 8; ++i)
   {
      IO = 0;
      if(temp & 0x01)
      {
         IO = 1;	/* set port pin high to read data */
      }
      SCLK = 0;
      SCLK = 1;
      temp >>= 1;
   }
}
/* ----------------------------------------------------------------------- */
uchar rbyte_3w()
{
   uchar i;
   uchar R_Byte;
   uchar TmpByte;
   
   R_Byte = 0x00;
   IO = 1;
   for(i=0; i<8; ++i)
   {
      SCLK = 1;
      SCLK = 0;
      TmpByte = (uchar)IO;
      TmpByte <<= 7;
      R_Byte >>= 1;
      R_Byte |= TmpByte; 
   }
   return R_Byte;
}

/*  struct tm */
/*  { */
/*    unsigned char tm_sec;                    Seconds.     [0-60]      */
/*    unsigned char tm_min;                    Minutes.     [0-59]      */
/*    unsigned char tm_hour;                   Hours.       [0-23]      */
/*    unsigned char tm_mday;                   Day.         [1-31]      */
/*    unsigned char tm_mon;                    Month.       [0-11]      */
/*    int tm_year;                             Year since 1900          */
/*    unsigned char tm_wday;                   Day of week. [0-6]       */
/*    int tm_yday;                             Days in year.[0-365]     */
/*    unsigned char tm_isdst;                  Daylight saving time     */
/*    unsigned char tm_hundredth;              not standard 1/100th sec */
/*  }; */

void ds1302_read_rtc ( struct tm *tm_time )
{
   reset_3w();
   wbyte_3w(RTC_BURST|0x01);
   tm_time->tm_sec = bcd2bin(rbyte_3w()&0x7F);
   tm_time->tm_min = bcd2bin(rbyte_3w()&0x7F);
   tm_time->tm_hour = bcd2bin(rbyte_3w()&0x3F);
   tm_time->tm_mday = bcd2bin(rbyte_3w()&0x3F);
   tm_time->tm_mon = bcd2bin(rbyte_3w()&0x1F);
   tm_time->tm_wday = bcd2bin(rbyte_3w()&0x7);
   tm_time->tm_year = bcd2bin(rbyte_3w());
   // According to spec we need to do 8 consecutive reads in clock burst mode
   rbyte_3w();
   reset_3w();
}

void ds1302_read_rtc_bcd ( struct tm *tm_time )
{
   reset_3w();
   wbyte_3w(RTC_BURST|0x01);
   tm_time->tm_sec =  rbyte_3w()&0x7F;
   tm_time->tm_min =  rbyte_3w()&0x7F;
   tm_time->tm_hour = rbyte_3w()&0x3F;
   tm_time->tm_mday = rbyte_3w()&0x3F;
   tm_time->tm_mon =  rbyte_3w()&0x1F;
   tm_time->tm_wday = rbyte_3w()&0x7;
   tm_time->tm_year = rbyte_3w();
   // According to spec we need to do 8 consecutive reads in clock burst mode
   rbyte_3w();
   reset_3w();
}

void ds1302_write_rtc ( const struct tm *tm_time )
{
   reset_3w();
   wbyte_3w(RTC_CTRL);    // "Write Protect" flag reset:
   wbyte_3w(0x00);
   reset_3w();
   wbyte_3w(RTC_BURST);  // Writing data in burst mode:
   wbyte_3w( tm_time->tm_sec );    // writing seconds:
   wbyte_3w( tm_time->tm_min );    // writing minutes:
   wbyte_3w( tm_time->tm_hour );   // writing hours:
   wbyte_3w( tm_time->tm_mday );   // writing day of the month:
   wbyte_3w( tm_time->tm_mon );    // writing month:
   wbyte_3w( tm_time->tm_wday );   // writing day of week:
   wbyte_3w( (unsigned char)(tm_time->tm_year&0xff) );   // writing year:
   wbyte_3w(0x80);                 // "Write Protect" flag set:
   reset_3w();
}

unsigned char bcd2bin(unsigned char bcd)
{
   unsigned char retval;
   retval = bcd>>4 & 0x0f;
   retval *= 10;
   retval += bcd & 0x0f;
   return retval;
}

void check_time(struct tm *timeptr) 
{
    // we could do some normalization here, e.g.
    // change 40 october to 9 november
    if (timeptr->tm_sec<=0) timeptr->tm_sec=0;
    else if (timeptr->tm_sec>59) timeptr->tm_sec=59;
    if (timeptr->tm_min<=0) timeptr->tm_min=0;
    else if (timeptr->tm_min>59) timeptr->tm_min=59;
    if (timeptr->tm_hour<=0) timeptr->tm_hour=0;
    else if (timeptr->tm_hour>23) timeptr->tm_hour=23;
    if (timeptr->tm_wday<=0) timeptr->tm_wday=0;
    else if (timeptr->tm_wday>6) timeptr->tm_wday=6;
    if (timeptr->tm_mday<=1) timeptr->tm_mday=1;
    else if (timeptr->tm_mday>31) timeptr->tm_mday=31;
    if (timeptr->tm_mon<=0) timeptr->tm_mon=0;
    else if (timeptr->tm_mon>11) timeptr->tm_mon=11;
    if (timeptr->tm_year<=0) timeptr->tm_year=0;
}

#define LEAP_YEAR(year) ((year%4)==0)

void local_gmtime(time_t *timep) {

   unsigned long epoch=*timep;
   unsigned int year;
   unsigned char month, monthLength;
   unsigned long days;
  
   system_time.tm_sec=epoch%60;
   epoch/=60; // now it is minutes
   system_time.tm_min=epoch%60;
   epoch/=60; // now it is hours
   system_time.tm_hour=epoch%24;
   epoch/=24; // now it is days
   system_time.tm_wday=(epoch+4)%7;
  
   year=1970;
   days=0;
   while((days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch) {
      year++;
   }
   system_time.tm_year=year-2000;
  
   days -= LEAP_YEAR(year) ? 366 : 365;
   epoch -= days; // now it is days in this year, starting at 0
   system_time.tm_yday=epoch;
  
   days=0;
   month=0;
   monthLength=0;
   for (month=0; month<12; month++) {
      if (month==1) { // februari
         if (LEAP_YEAR(year)) {
            monthLength=29;
         } else {
            monthLength=28;
         }
      } else {
         monthLength = monthDays[month];
      }
    
      if (epoch>=monthLength) {
         epoch-=monthLength;
      } else {
         break;
      }
   }
   system_time.tm_mon=month;
   system_time.tm_mday=epoch+1;
  
   system_time.tm_isdst=0;
}

#define  MINUTE  60
#define  HOUR    60*MINUTE
#define  DAY     24L*HOUR
#define  YEAR    365*DAY

// convert broken time to calendar time (seconds since 1970)
time_t local_mktime(struct tm *timeptr, unsigned int add) {
   //   int year=timeptr->tm_year+1900;
   //   int year=timeptr->tm_year + 2000;
   int year=timeptr->tm_year + add;
   char month=timeptr->tm_mon;
   char mday = timeptr->tm_mday - 1;
   int i;
   long seconds;
   
   check_time(timeptr);
   
   // seconds from 1970 till 1 jan 00:00:00 this year
   seconds = (year-1970)*(YEAR);
   
   // add extra days for leap years
   for (i=1970; i<year; i++) 
   {
      if (LEAP_YEAR(i)) 
      {
         seconds+= DAY;
      }
   }
   
   // add days for this year
   for (i=0; i<month; i++) 
   {
      if (i==1 && LEAP_YEAR(year)) 
      { 
         seconds+= DAY*29;
      } 
      else 
      {
         seconds+= DAY*monthDays[i];
      }
    }
   
   seconds+= mday*DAY;
   return seconds;
}

// format the time into "Sat Feb 17 17:45:23 2001"
void printf_asctime(struct tm *timeptr) 
{
   check_time(timeptr);
   printf_fast("%04u ",(unsigned int)timeptr->tm_year+2000);
   printf_fast((char code*)month[bcd2bin(timeptr->tm_mon)]);
   printf_fast("%02u %02u:%02u:%02u", 
               (unsigned int)timeptr->tm_mday, 
               (unsigned int)timeptr->tm_hour, 
               (unsigned int)timeptr->tm_min, 
               (unsigned int)timeptr->tm_sec);
}

void printf_date(const struct tm *timeptr)
{
   printf_fast("%04u-%02u-%02u", 
               (unsigned int)timeptr->tm_year,
               (unsigned int)timeptr->tm_mon,
               (unsigned int)timeptr->tm_mday);
}

void printf_time(const struct tm *timeptr)
{
   printf_fast("%02u:%02u:%02u", 
               (unsigned int)timeptr->tm_hour,
               (unsigned int)timeptr->tm_min,
               (unsigned int)timeptr->tm_sec);
}




/* ----------------------------------------------------------------------- */

// void initialize_DS1302()
// {
//    reset_3w();
//    wbyte_3w(0x8e);	/* control register */
//    wbyte_3w(0);		/* disable write protect */
//    reset_3w();
//    wbyte_3w(RTC_SEC);
//    wbyte_3w(0);
//    //   wbyte_3w(0x90);	/* trickle charger register */
//    // wbyte_3w(0xab);	/* enable, 2 diodes, 8K resistor */
//    reset_3w();
//    wbyte_3w(0xbe);	/* clock burst write (eight registers) */
//    wbyte_3w(0);
//    wbyte_3w(0);
//    wbyte_3w(0x12);
//    wbyte_3w(0x15);
//    wbyte_3w(0x2);
//    wbyte_3w(0x2);
//    wbyte_3w(0x3);
//    wbyte_3w(0);		/* must write control register in burst mode */
//    reset_3w();
// }

void ds1302_write_memory(const unsigned char addr, const unsigned char number)
{
   unsigned char address;

   address = addr;

   reset_3w();
   wbyte_3w(RTC_CTRL);
   wbyte_3w(0x00);
   reset_3w();
   wbyte_3w((address<<1)|0xC0);
   wbyte_3w(number);
   reset_3w();
   wbyte_3w(RTC_CTRL);
   wbyte_3w(0x80);
   reset_3w();
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

unsigned char ds1302_read_memory( const unsigned char addr)
{
   unsigned char result;
   unsigned char address;

   address = addr;

   reset_3w();
   wbyte_3w((address<<1)|0xC1);
   result = rbyte_3w();
   reset_3w();
   return result;
}
