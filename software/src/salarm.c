/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/salarm.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: salarm.c,v $
**  Revision 1.1  2004/01/12 22:29:33  tomasz
**  Alarm handling routines
**
**
**  $Id: salarm.c,v 1.1 2004/01/12 22:29:33 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#include <stdio.h>
#include "89c51rd2.h"

typedef struct _AlarmDefs {
      unsigned const char type;
      unsigned const char *description;
} AlarmDefs;

typedef enum _AlarmTypes {
   NO_ALARM,
   FOIL_ROLL_ALARM,
   HOLO_SEARCH_ALARM
} AlarmTypes;

AlarmDefs code alarm_defs[] = {
   { NO_ALARM,"NO ALARM" },
   { FOIL_ROLL_ALARM,   "             FOIL ALARM !                     PRESS RESET TO CLEAR ALARM !      " },
   { HOLO_SEARCH_ALARM, "       HOLOGRAM OVERRUN ALARM !!!           PRESS HOLO SEARCH TO CONTINUE       " }
};

volatile AlarmDefs *system_alarm = &alarm_defs[NO_ALARM];

void alarm_on(unsigned char type)
{
   system_alarm = &alarm_defs[type];
}

void alarm_off(void)
{
   system_alarm = &alarm_defs[NO_ALARM];
}

