/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/rtc.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: rtc.h,v $
**  Revision 1.4  2004/11/19 20:33:37  tomasz
**  latest changes incoroprated
**
**  Revision 1.3  2004/01/28 09:52:40  tomasz
**  RTC data backup added
**
**  Revision 1.2  2004/01/19 13:45:45  tomasz
**  added ds1302_read_rtc_bcd()
**
**  Revision 1.1  2004/01/12 22:29:12  tomasz
**  library for use 8051 and dallas RTC ds1203
**
**
**  $Id: rtc.h,v 1.4 2004/11/19 20:33:37 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#ifndef _DS1302_H_
#define	_DS1302_H_

#include <time.h>

typedef enum _RTC_BACKUP {
   RTC_CURRENT_DISPLAY_STATE,
   RTC_ALARM_ROLL_COUNT_LSB,
   RTC_ALARM_ROLL_COUNT_MSB,
   RTC_COUNT_FOILUSED_LSB,
   RTC_COUNT_FOILUSED_MSB,
   RTC_IMPRESS_LSB,
   RTC_IMPRESS_LSB_1,
   RTC_IMPRESS_LSB_2,
   RTC_IMPRESS_MSB,
   RTC_IMPRESS_TOTAL,
   RTC_IMPRESS_TOTAL_LSB,
   RTC_IMPRESS_TOTAL_LSB_1,
   RTC_IMPRESS_TOTAL_LSB_2,
   RTC_IMPRESS_TOTAL_MSB,
   RTC_TEST_BYTE
} RTC_BACKUP;

void ds1302_init_rtc  ( void );
void ds1302_write_rtc ( const struct tm *tm_time );
void ds1302_read_rtc  ( struct tm *tm_time );
void ds1302_read_rtc_bcd ( struct tm *tm_time );
void ds1302_write_memory(const unsigned char addr, const unsigned char number);
unsigned char ds1302_read_memory( const unsigned char addr);
void ds1302_write_memory_burst ( const unsigned char* data, const unsigned int bytes );
void ds1302_read_memory_burst ( unsigned char* data, const unsigned int bytes );
unsigned char bcd2bin(unsigned char bcd);

void local_gmtime(time_t *time);
time_t local_mktime(struct tm *timeptr, unsigned int);
void printf_date(const struct tm *timeptr);
void printf_time(const struct tm *timeptr);
void printf_asctime(struct tm *timeptr);
void initialize_DS1302(void);

#endif

