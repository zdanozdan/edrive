/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/menu.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: menu.h,v $
**  Revision 1.6  2004/11/19 20:33:36  tomasz
**  latest changes incoroprated
**
**  Revision 1.5  2004/01/12 22:34:38  tomasz
**  Fixed problem with EXIT menu
**
**  Revision 1.4  2003/12/28 14:56:56  tomasz
**  refresh_count_display() added
**
**  Revision 1.3  2003/12/22 10:53:50  tomasz
**  new features
**
**  Revision 1.2  2003/11/14 18:10:27  tomasz
**  ported to compile with sdcc c51 compiler
**
**  Revision 1.1  2003/11/08 12:00:19  tomasz
**  first release
**
**  $Id: menu.h,v 1.6 2004/11/19 20:33:36 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/
#ifndef __MENU_H__
#define __MENU_H__

#include <time.h>

#define MENU_ITEMS 7
#define MAX_VALUE 15
#define CLEAR_CANCEL_FUNCTION  -1
#define ON_OFF_FUNCTION        -2
#define NUMERIC_FUNCTION       -3

#define BLINKING_ON  blinking = TRUE
#define BLINKING_OFF blinking = FALSE;

void menu_init_blinking(void);
void main_program_workloop(void);
void refresh_count_display(void);
void menu_previous_position(void);

typedef struct _Blink {
      char position;
      char *first_pattern_on;
      char *first_pattern_off;
      char *second_pattern_on;
      char *second_pattern_off;
      char lines;
} Blink_params;

typedef enum DisplayStatesIds {
   MAIN_STATE_ID        = 0,
   FOIL_PULL_ID         = 1,
   SPEED_ADJUST_ID      = 2,
   FOIL_ALARM_ID        = 3,
   COUNT_SETUP_ID       = 4,
   HOLOGRAM_SETUP_ID    = 5,
   NEXT_MENU_ID         = 6,
   PULL_INPUT_ID        = 7,
   STEP_INPUT_ID        = 8,
   CLEAR_INPUT_ID       = 9,
   PULL_SPEED_INPUT_ID  = 10,
   CLEAR_SPEED_INPUT_ID = 11,
   INCH_SPEED_INPUT_ID  = 12,
   ENABLE_INPUT_ID      = 13,
   ROLL_SIZE_INPUT_ID   = 14,
   IMPRESS_ID           = 15,
   FOIL_USED_ID         = 16,
   ROLL_ID              = 17,
   HOLO_ACTIVE_ID       = 18,
   HOLO_OFFSET_ID       = 19,
   HOLO_OVERRUN_ID      = 20,
   HOLO_SPEED_ID        = 21,
   TIME_ID              = 22,
   DATE_ID              = 23,
   SYSTEM_ID            = 24,
   EXIT_POS_ID          = 0xFE,
   EMPTY                = 0xFF
};

#endif


