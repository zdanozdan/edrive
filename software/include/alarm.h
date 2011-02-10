/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/alarm.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: alarm.h,v $
**  Revision 1.1  2004/01/12 22:29:45  tomasz
**  Alarm handling routines
**
**
**  $Id: alarm.h,v 1.1 2004/01/12 22:29:45 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#ifndef __ALARM_H__
#define __ALARM_H__

typedef struct _AlarmDefs {
      unsigned const char type;
      unsigned const char *description;
} AlarmDefs;

typedef enum _AlarmTypes {
   NO_ALARM,
   FOIL_ROLL_ALARM,
   HOLO_SEARCH_ALARM
} AlarmTypes;

void alarm_on(unsigned char type);
void alarm_off(void);

#endif


