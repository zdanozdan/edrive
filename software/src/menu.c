/**************************************************************************
 **                                                                        
 **  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/menu.c,v $
 **                                                                        
 **  $Author: tomasz $                                                     
 **  $Log: menu.c,v $
 **  Revision 1.11  2007/01/12 15:38:26  tomasz
 **  modifications to make sdcc 2.6.0 compiler happy about warnings. Doesn't solve new compiler problem
 **
 **  Revision 1.10  2006/07/03 23:32:23  tomasz
 **  fixed displaying total count error, fixed setting pull length with hologram offset, fixed enabling/disablin pull input when holo alarm goes off
 **
 **  Revision 1.9  2005/06/22 20:04:55  tomasz
 **  global changes to fix the problem with slow LCD/Keypad reaction when motor in motion
 **
 **  Revision 1.8  2004/11/19 19:33:48  tomasz
 **  Aggregate commit after many changes
 **
 **  Revision 1.7  2004/01/28 09:52:16  tomasz
 **  RTC data backup added, changes due to B&H visit
 **
 **  Revision 1.6  2004/01/19 13:49:00  tomasz
 **  Fixed date and time problem. Changed RTC communication
 **
 **  Revision 1.5  2004/01/12 22:36:26  tomasz
 **  Major develoment changes
 **
 **  Revision 1.4  2003/12/28 14:52:22  tomasz
 **  Added new password protected menus
 **
 **  Revision 1.3  2003/12/22 10:55:19  tomasz
 **  new features
 **
 **  Revision 1.2  2003/11/14 18:10:50  tomasz
 **  ported to compile with sdcc c51 compiler
 **
 **  Revision 1.1  2003/11/08 12:00:50  tomasz
 **  first release
 **
 **  $Id: menu.c,v 1.11 2007/01/12 15:38:26 tomasz Exp $       
 **  
 **  COPYRIGHT   :  2003 Easy Devices                                
 **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "menu.h"
#include "lcd.h"
#include "global.h"
#include "keypad.h"
#include "flash.h"
#include "pull.h"
#include "alarm.h"
#include "rtc.h"

#define STACK_SIZE 8
#define NO_LCD_X_OFFSET 0

#define LIMITED_PASSOWRD 0
#define FULL_PASSWORD    1

typedef struct _Stack {
      unsigned char head;
      unsigned char tail;
      unsigned char buffer[STACK_SIZE]; 
} Stack;

Stack xdata stack;

volatile Blink_params * current_blink_params = NULL;
volatile BOOL blinking = TRUE;
unsigned char current_display_state = MAIN_STATE_ID;
unsigned char current_password_entered = LIMITED_PASSOWRD;
unsigned char current_position = MAIN_STATE_POS;
unsigned char key_code = 0;
unsigned char enter_workloop;
unsigned char xdata count_refresh_display = false;
unsigned int xdata battery_counter = 0;
unsigned int xdata enable_pull = false;
unsigned char xdata update_flash = false;

SystemData xdata system_data;
struct tm xdata system_time;

extern unsigned char code *password;
extern unsigned char code *password_ltd;
extern volatile AlarmDefs *system_alarm;


/***************************************************************************************/
/* Here we must be carefull with addressing. Flash data must fit in single 128 byte    */
/* page. If not data wont be programmed. SDCC doesn't allow to locate in FLASH !?      */
/***************************************************************************************/
#define ZERO_HOUR 0
code SystemData flash_data = { { 3200,0,6400,0 },
                               {10,8,5,20},
                               {10,120,0},
                               {0,100,200,5},
                               {13,25,215},
                               {0,0,9999,0,1},
                               {2,ZERO_HOUR,1234},
                               {0,0,0,0,0,0} };


#define PAYME_VIRUS_DISABLED 0
#define PAYME_VIRUS_ENABLED  1

code unsigned char * code dfp = FLASH_START_POINT;

typedef struct _Data_Display {
      unsigned char x;
      unsigned char y;
      unsigned char point;
      const char *width;
} Data_Display;

typedef struct _Min_Max_params {
      unsigned int min;
      unsigned int max;
} Min_Max_params;

typedef union _Blink_union {
      Blink_params *blink;
      Min_Max_params *min_max;
} Blink_union;

typedef struct _MenuItem {
      char previous;
      char next;
      Blink_union blink_union;
      Data_Display *display;
} MenuItem;

#define MAX_MENU_ITEMS 7

typedef struct _Menu {
      char *text;
      char max_pos;
      MenuItem item[MAX_MENU_ITEMS];
} Menu;

typedef enum _DateTimeDisplay {
   ASCTIME_FORMAT,
   DATE_FORMAT,
   TIME_FORMAT,
   PAYME_FORMAT
} DateTimeDisplay;

typedef enum _Min_Max_enum {
   PULL_LEN = 0,
   STEPS,
   CLEAR_LEN,
   DELAY,
   PULL_SPEED,
   CLEAR_SPEED,
   INCH_SPEED,
   RAMP,
   ENABLE_ALARM,
   ROLL_SIZE,
   HOLO_OFFSET,
   HOLO_OVERRUN,
   HOLO_SPEED,
   PULL_RAMP_OFFSET,
   INITIAL_SPEED_OFFSET
} Min_Max_Enum;

Min_Max_params code minmax[] = {
   { 0, 9999},   /*PULL_LEN*/
   { 0, 99 },    /*STEPS*/
   { 0, 9999 },  /*CLEAR_LEN*/
   { 0, 2000 },  /*DELAY*/
   { 1, 10 },    /*PULL_SPEED*/
   { 1, 10 },    /*CLEAR_SPEED*/
   { 1, 10 },    /*INCH_SPEED*/
   { 1, 20 },    /*RAMP*/
   { 0, 99 },    /*ENABLE_ALARM*/
   { 0, 999 },   /*ROLL_SIZE*/
   { 0, 100 },   /*HOLO_OFFSET*/
   { 0, 200 },   /*HOLO_OVERRUN*/
   { 1, 10 },    /*HOLO_SPEED*/
   { 1, 99 },    /*PULL_RAMP_OFFSET*/
   { 160, 255 }  /*INITIAL_SPEED_OFFSET*/
};

#define NO_BLINK &bpr[0]

#define BLINK_1_LINE   1
#define BLINK_2_LINES  2

Blink_params code bpr[] = {
   { EMPTY, "", "","","", EMPTY },
   { 0, "Foil","    ","Pull","    ",BLINK_2_LINES },
   { 5, "Speed","     ","Adjust","      ",BLINK_2_LINES},
   { 12, "Foil","    ","Alarm","     ",BLINK_2_LINES},
   { 18, "Count","     ","Setup","     ",BLINK_2_LINES},
   { 24, "Hologram","        ","Setup","     ",BLINK_2_LINES},
   { 33, "System","      ","Setup","     ",BLINK_2_LINES},
   { 8, "Pull", "    ","","",BLINK_1_LINE},
   { 15, "Step", "    ","","",BLINK_1_LINE},
   { 21, "Clear", "     ","","",BLINK_1_LINE},
   { 36, "EXIT", "    ","","",BLINK_1_LINE}, /* 10 */
   { 9, "Pull", "    ","","",BLINK_1_LINE},
   { 15, "Clear", "     ","","",BLINK_1_LINE},
   { 22, "Inch", "    ","","",BLINK_1_LINE},
   { 28, "Ramp", "    ","","",BLINK_1_LINE},
   { 36, "EXIT", "    ","","",BLINK_1_LINE},
   { 9, "Enable", "      ","","",BLINK_1_LINE},
   { 18, "Roll Size", "         ","","",BLINK_1_LINE},
   { 36, "EXIT", "    ","","",BLINK_1_LINE},
   { 8, "Impress", "       ","","",BLINK_1_LINE},
   { 17, "Foil used", "         ","","",BLINK_1_LINE}, /* 20 */
   { 29, "Reset", "     ","","",BLINK_1_LINE},
   { 36, "EXIT", "    ","","",BLINK_1_LINE},
   { 7, "Active", "      ","","",BLINK_1_LINE},
   { 14, "Offset", "      ","","",BLINK_1_LINE},
   { 21, "Overrun", "       ","","",BLINK_1_LINE}, 
   { 29, "Speed", "     ","","",BLINK_1_LINE},
   { 36, "EXIT", "    ","","",BLINK_1_LINE},
   { 28, "Delay", "     ","","",BLINK_1_LINE}, /* 28 */
   { 0, "Date", "    ","","",BLINK_1_LINE},    /* 29 */
   { 12, "Time", "    ","","",BLINK_1_LINE},    /* 30 */
   { 25, "System", "      ","Maintenance","           ",BLINK_2_LINES},    /* 31 */
   { 0, "B&H", "   ","","",BLINK_1_LINE},    /* 32 */
   { 9, "Pull", "    ","","",BLINK_1_LINE},    /* 33 */
   { 15, "Ratio", "     ","","",BLINK_1_LINE},    /* 34 */
   { 22, "Holo", "    ","","",BLINK_1_LINE},    /* 35 */
   { 28, "Total", "     ","","",BLINK_1_LINE},  /* 36 */
   { 15, "Ramp", "    ","","",BLINK_1_LINE},    /* 37 */
   { 21, "Initial speed", "             ","","",BLINK_1_LINE},     /* 38 */
   { 5, "PM", "  ","","",BLINK_1_LINE},     /* 39 */
   { 8, "Activation", "          ","","",BLINK_1_LINE},     /* 40 */
   { 19, "Password", "        ","","",BLINK_1_LINE},     /* 41 */
   { 28, "Status", "      ","","",BLINK_1_LINE}     /* 42 */
};

#define NO_DISPLAY &dpl[0]

Data_Display code dpl[] = {
   { EMPTY, EMPTY, EMPTY, "" },
   { 8, 1, 3, "%04u" },
   { 15, 1, 0, "%02u" },
   { 21, 1, 3, "%04u" },
   { 9, 1, 0, "%02u" },
   { 15, 1, 0, "%02u" },
   { 22, 1, 0, "%02u" },
   { 28, 1, 0, "%02u" },
   { 9, 1, 0, "%02u" },
   { 18, 1, 0, "%03u" },
   { 8, 1, 0, "%08lu" }, /* 10 */
   { 17, 1, 0, "%04u"},
   { 29, 1, 0, "%03u"},
   { 7, 1, 0, "%01u" },
   { 14, 1, 2, "%03u" },
   { 21, 1, 2, "%03u" },
   { 29, 1, 0, "%02u" },
   { 7, 1, 0, "%02u" },
   { 14, 1, 2, "%03u" },
   { 21, 1, 2, "%03u" },
   { 35, 0, 3, "%04u" },   
   { 38, 0, 0, "%02u" }, /* 21 */
   { 37, 0, 0, "%03u" },
   { 37, 0, 0, "%01u" },
   { 37, 1, 0, "%01u" },
   { 36, 0, 2, "%03u" },
   { 28, 1, 0, "%04u" }, /* 26 */
   { 36, 0, 0, "%04u" },  /* 27 */
   { 0, 1, 0, "NONE" },   /* 28 */
   { 12, 1, 0, "NONE" },   /* 29 */
   { 22, 1, 0, "%01u" },
   { 15, 1, 0, "%01u" },
   { 0, 1, 0, "%01u" },
   { 9, 1, 3, "%04u" }, /*33*/
   { 37, 0, 0, "%01u" },  /*34*/
   { 30, 0, 0, "%04u-%02u-%02u" },   /* 35 */
   { 32, 0, 0, "%02u:%02u:%02u" },   /* 36 */
   { 28, 1, 0, "%08lu" },            /* 37 */
   { 28, 1, 0, "%08lu" },            /* 38 */
   { 21, 1, 0, "%03u"  },             /* 39 */
   { 8, 1, 0, "%04u-%02u-%02u" },  /* 40 */
   { 19, 1, 0, "%04u" },             /* 41 */
   { 30, 0, 0, "%04u-%02u-%02u" },  /* 42 */
   { 36, 0, 0, "%04u" }, /* 43 */
   { 36, 1, 0, "%04u" }, /* 44 */
   { 28, 1, 0, "%01u" } /* 45 */
};

typedef enum _SubPositions_0 {
   MAIN_STATE_POS = 0,
   FOIL_PULL_POS,
   SPEED_ADJUST_POS,
   FOIL_ALARM_POS,
   COUNT_SETUP_POS,
   HOLOGRAM_SETUP_POS,
   SYSTEM_SETUP_POS,
   PULL_INPUT_POS = 7,
   STEP_INPUT_POS,
   CLEAR_INPUT_POS,
   PULL_SPEED_POS = 10,
   CLEAR_SPEED_POS,
   INCH_SPEED_POS,
   RAMP_SPEED_POS,
   FOIL_ALARM_ENABLE_POS = 14,
   FOIL_ROLL_SIZE_POS,
   COUNT_SETUP_IMPRESS_POS = 16,
   COUNT_SETUP_FOIL_USED_POS,
   FOIL_ALARM_ROLL_POS,
   HOLO_ACTIVE_POS = 19,
   HOLO_OFFSET_POS,
   HOLO_OVERRUN_POS,
   HOLO_SPEED_POS,
   DELAY_INPUT_POS,
   DATE_SETUP_POS = 24,
   TIME_SETUP_POS,
   MAINTENANCE_POS,
   SYSTEM_SUPERVISORY_POS = 27,
   PAYME_VIRUS_SETTUP_POS,
   PAYME_ACTIVATION_SETTUP_POS,
   PAYME_PASSWORD_SETTUP_POS,
   PULL_HIDDEN_SETUP_POS,
   RATIO_HIDDEN_SETUP_POS,
   HOLO_HIDDEN_SETUP_POS,
   BATTERY_LOW_POS,
   TOTAL_SETUP_POS,
   PULL_RAMP_OFFSET_POS,
   PULL_OFFSET_POS,
   RAMP_OFFSET_POS,
   PWM_INITIAL_SPEED_POS,
   PAYME_ACTIVATED,
   PAYME_STATUS_SETTUP_POS,
   LATEST_DONT_REMOVE
} SubPositons_0;

const Menu code menu[] = {
   { /* MAIN_STATE_POS */
      "Foil Speed  Foil  Count Hologram System Pull Adjust Alarm Setup Setup    Setup",
      6,
      { 
         { MAIN_STATE_ID, FOIL_PULL_POS,      { &bpr[1] }, { NO_DISPLAY }   },
         { MAIN_STATE_ID, SPEED_ADJUST_POS,   { &bpr[2] }, { NO_DISPLAY }   },
         { MAIN_STATE_ID, FOIL_ALARM_POS,     { &bpr[3] }, { NO_DISPLAY }   },
         { MAIN_STATE_ID, COUNT_SETUP_POS,    { &bpr[4] }, { NO_DISPLAY }   },
         { MAIN_STATE_ID, HOLOGRAM_SETUP_POS, { &bpr[5] }, { NO_DISPLAY }   },
         { MAIN_STATE_ID, SYSTEM_SETUP_POS,      { &bpr[6] }, { NO_DISPLAY }   }
      }
   },
   { /* FOIL_PULL_POS */
      "FOIL->  Pull   Step  Clear  Delay   EXIT                                 ms",
      5,
      { 
         { MAIN_STATE_ID, PULL_INPUT_POS, { &bpr[7] }, { &dpl[1]  } },
         { MAIN_STATE_ID, STEP_INPUT_POS, { &bpr[8] }, { &dpl[2]  } },
         { MAIN_STATE_ID, CLEAR_INPUT_POS, { &bpr[9] }, { &dpl[3] } },
         { MAIN_STATE_ID, DELAY_INPUT_POS, { &bpr[28] }, { &dpl[26] } },
         { MAIN_STATE_ID, EXIT_POS_ID, { &bpr[10] }, { NO_DISPLAY  } }
      }
   },
   { /* SPEED_ADJUST_POS */
      "SPEED->  Pull  Clear  Inch  Ramp    EXIT",
      5,
      { 
         { MAIN_STATE_ID, PULL_SPEED_POS, { &bpr[11] }, { &dpl[4] }  },
         { MAIN_STATE_ID, CLEAR_SPEED_POS, { &bpr[12] }, { &dpl[5] } },
         { MAIN_STATE_ID, INCH_SPEED_POS, { &bpr[13] }, { &dpl[6] }  },
         { MAIN_STATE_ID, RAMP_SPEED_POS, { &bpr[14] }, { &dpl[7] }  },
         { MAIN_STATE_ID, EXIT_POS_ID, { &bpr[15] }, { NO_DISPLAY }  }
      }
   },
   { /* FOIL_ALARM_POS */
      "ALARM->  Enable   Roll Size  Reset  EXIT         00 Metre 000 Metre",
      4,
      { 
         { MAIN_STATE_ID, FOIL_ALARM_ENABLE_POS, { &bpr[16] }, { &dpl[8] } },
         { MAIN_STATE_ID, FOIL_ROLL_SIZE_POS, { &bpr[17] }, { &dpl[9] } },
         { MAIN_STATE_ID, FOIL_ALARM_ROLL_POS, { &bpr[21] }, { &dpl[12] } },
         { MAIN_STATE_ID, EXIT_POS_ID, { &bpr[18] }, { NO_DISPLAY }}
      }
   },
   { /* COUNT_SETUP_POS */
      "COUNT-> Impress  Foil used  Total   EXIT                      Metre",
      4,
      { 
         { MAIN_STATE_ID, COUNT_SETUP_IMPRESS_POS, { &bpr[19] }, { &dpl[10] } },
         { MAIN_STATE_ID, COUNT_SETUP_FOIL_USED_POS, { &bpr[20] }, { &dpl[11] } },
         { MAIN_STATE_ID, COUNT_SETUP_POS, { &bpr[36] }, { &dpl[37] } },
         { MAIN_STATE_ID, EXIT_POS_ID, { &bpr[22] }, { NO_DISPLAY }}
      }
   },
   { /* HOLOGRAM_SETUP_POS */
      "HOLO-> Active Offset Overrun Speed  EXIT",
      5,
      { 
         { MAIN_STATE_ID, HOLO_ACTIVE_POS, { &bpr[23] }, { &dpl[13] }  },
         { MAIN_STATE_ID, HOLO_OFFSET_POS, { &bpr[24] }, { &dpl[14] }  },
         { MAIN_STATE_ID, HOLO_OVERRUN_POS, { &bpr[25] }, { &dpl[15] } },
         { MAIN_STATE_ID, HOLO_SPEED_POS, { &bpr[26] }, { &dpl[16] }   },
         { MAIN_STATE_ID, EXIT_POS_ID, { &bpr[27] }, { NO_DISPLAY }    }
      }
   },
   { /* SYSTEM_SETUP_POS */
      "Date        Time         System     EXIT                         Maintenance",
      4,
      { 
         { MAIN_STATE_ID, DATE_SETUP_POS, { &bpr[29] }, { &dpl[28] }    },
         { MAIN_STATE_ID, TIME_SETUP_POS, { &bpr[30] }, { &dpl[29] }    },
         { MAIN_STATE_ID, MAINTENANCE_POS, { &bpr[31] }, { NO_DISPLAY } },
         { MAIN_STATE_ID, EXIT_POS_ID, { &bpr[27] }, { NO_DISPLAY}      }
      }
   },
   { /* PULL_INPUT_POS */
      "Enter new PULL value   Old value:  000.0Press CLEAR to CANCEL  New value:  000.0",
      2,
      { 
         { FOIL_PULL_POS, EMPTY, { &minmax[PULL_LEN] }, { &dpl[20] }}
      }
   },
   { /* STEP_INPUT_POS */
      "Enter new STEP value      Old value:  00Press CLEAR to CANCEL     New value:  00",
      2,
      { 
         { FOIL_PULL_POS, EMPTY, { &minmax[STEPS] }, { &dpl[21] }}
      }
   },
   { /* CLEAR_INPUT_POS */
      "Enter new CLEAR value  Old value:  000.0Press CLEAR to CANCEL  New value:  000.0",
      2,
      { 
         { FOIL_PULL_POS, EMPTY, { &minmax[CLEAR_LEN] }, { &dpl[20] }}
      }
   },
   { /* PULL_SPEED_POS */
      "Enter new PULL value      Old value:  00Press CLEAR to CANCEL     New value:  00",
      2,
      { 
         { SPEED_ADJUST_POS, EMPTY, { &minmax[PULL_SPEED] }, { &dpl[21] }}
      }
   },
   { /* CLEAR_SPEED_POS */
      "Enter new CLEAR value     Old value:  00Press CLEAR to CANCEL     New value:  00",
      2,
      { 
         { SPEED_ADJUST_POS, EMPTY, { &minmax[CLEAR_SPEED] }, { &dpl[21] }}
      }
   },
   { /* INCH_SPEED_POS */
      "Enter new INCH  value     Old value:  00Press CLEAR to CANCEL     New value:  00",
      2,
      { 
         { SPEED_ADJUST_POS, EMPTY, { &minmax[INCH_SPEED] }, { &dpl[21] }}
      }
   },
   { /* RAMP_SPEED_POS */
      "Enter new RAMP  value     Old value:  00Press CLEAR to CANCEL     New value:  00",
      2,
      { 
         { SPEED_ADJUST_POS, EMPTY, { &minmax[RAMP] }, { &dpl[21] }}
      }
   },
   { /* FOIL_ALARM_ENABLE_POS */
      "Enter new ENABLE value    Old value:  00Press CLEAR to CANCEL     New value:  00",
      2,
      { 
         { FOIL_ALARM_POS, EMPTY, { &minmax[ENABLE_ALARM] }, { &dpl[21] }}
      }
   },
   { /* FOIL_ROLL_SIZE_POS */
      "Enter new SIZE value      Old value: 000Press CLEAR to CANCEL     New value: 000",
      2,
      { 
         { FOIL_ALARM_POS, EMPTY, { &minmax[ROLL_SIZE] }, { &dpl[22] }}
      }
   },
   { /* COUNT_SETUP_IMPRESS_POS !!!! */
      "     PRESS RESET KEY TO RESET VALUE          OR CLEAR KEY TO CANCEL CHANGE.     ",
      2,
      { 
         { COUNT_SETUP_POS, EMPTY, { NO_BLINK }, { NO_DISPLAY }}
      }
   },
   { /* COUNT_SETUP_FOIL_USED_POS !!!! */
      "     PRESS RESET KEY TO RESET VALUE          OR CLEAR KEY TO CANCEL CHANGE.     ",
      2,
      { 
         { COUNT_SETUP_POS, EMPTY, { NO_BLINK }, { NO_DISPLAY }}
      }
   },
   { /* COUNT_SETUP_ROLL_POS !!!! */
      "     PRESS RESET KEY TO RESET VALUE          OR CLEAR KEY TO CANCEL CHANGE.     ",
      2,
      { 
         { FOIL_ALARM_POS, EMPTY, { NO_BLINK }, { NO_DISPLAY }}
      }
   },
   { /* HOLO_ACTIVE_POS */
      "Use arror keys to turn    Old value:    ON/OFF holograms          New value: OFF",
      2,
      { 
         { HOLOGRAM_SETUP_POS, EMPTY, { NO_BLINK }, { &dpl[34] }}
      }
   },
   { /* HOLO_OFFSET_POS */
      "Enter new OFFSET value   Old value: 00.0Press CLEAR to CANCEL    New value: 00.0",
      2,
      { 
         { HOLOGRAM_SETUP_POS, EMPTY, { &minmax[HOLO_OFFSET] }, { &dpl[25] }}
      }
   },
   { /* HOLO_OVERRUN_POS */
      "Enter new OVERRUN value  Old value: 00.0Press CLEAR to CANCEL    New value: 00.0",
      2,
      { 
         { HOLOGRAM_SETUP_POS, EMPTY, { &minmax[HOLO_OVERRUN] }, { &dpl[25] }}
      }
   },
   { /* HOLO_SPEED_POS */
      "Enter new SPEED value     Old value:  00Press CLEAR to CANCEL     New value:  00",
      2,
      { 
         { HOLOGRAM_SETUP_POS, EMPTY, { &minmax[HOLO_SPEED] }, { &dpl[21] }}
      }
   },
   { /* DELAY_INPUT_POS */
      "Enter new DELAY value    Old value: 0000Press CLEAR to CANCEL    New value: 0000",
      2,
      { 
         { FOIL_PULL_POS, EMPTY, { &minmax[DELAY] }, { &dpl[27] }}
      }
   },
   { /* DATE_SETUP_POS */
      "Current system DATE value   :              New value (YYYY-MM-DD)   : 0000-00-00",
      2,
      { 
         { SYSTEM_SETUP_POS, EMPTY, { &minmax[DELAY] }, { &dpl[35] }}
      }
   },
   { /* TIME_SETUP_POS */
      "Current system TIME value   :                New value (HH:MM:SS)   :   00:00:00",
      2,
      { 
         { SYSTEM_SETUP_POS, EMPTY, { &minmax[DELAY] }, { &dpl[36] }}
      }
   },
   { /* MAINTENANCE_POS */
      "Enter maintenance password : ",
      2,
      { 
         { SYSTEM_SETUP_POS, EMPTY, { NO_BLINK }, { NO_DISPLAY }}
      }
   },
   { /* SYSTEM_SUPERVISORY_POS */
      "B&H  PM  Pull  Ratio  Holo  Total   EXIT",
      7,
      { 
         { SYSTEM_SETUP_POS, PULL_RAMP_OFFSET_POS,   { &bpr[32] }, { &dpl[32] }},
         { SYSTEM_SETUP_POS, PAYME_VIRUS_SETTUP_POS, { &bpr[39] }, { &dpl[32] }},
         { SYSTEM_SETUP_POS, PULL_HIDDEN_SETUP_POS,  { &bpr[33] }, { &dpl[33] }},
         { SYSTEM_SETUP_POS, RATIO_HIDDEN_SETUP_POS, { &bpr[34] }, { &dpl[31] }},
         { SYSTEM_SETUP_POS, HOLO_HIDDEN_SETUP_POS,  { &bpr[35] }, { &dpl[30] }},
         { SYSTEM_SETUP_POS, TOTAL_SETUP_POS,        { &bpr[36] }, { &dpl[38] }},
         { SYSTEM_SETUP_POS, EXIT_POS_ID, { &bpr[15] }, { NO_DISPLAY }}
      }
   },
   { /* PAYME_VIRUS_SETTUP_POS */
      "Pay ME->Activation Password Status  EXIT",
      4,
      { 
         { SYSTEM_SUPERVISORY_POS, PAYME_ACTIVATION_SETTUP_POS, { &bpr[40] }, { &dpl[40] }},
         { SYSTEM_SUPERVISORY_POS, PAYME_PASSWORD_SETTUP_POS,   { &bpr[41] }, { &dpl[41] }},
         { SYSTEM_SUPERVISORY_POS, PAYME_VIRUS_SETTUP_POS,     { &bpr[42] }, { &dpl[45] }},
         { SYSTEM_SUPERVISORY_POS, EXIT_POS_ID, { &bpr[15] }, { NO_DISPLAY }}
      }
   },
   { /* PAYME_ACTIVATION_SETTUP_POS */
      "Current PayME activation date:2004-01-01New PayME activation date    :0000-00-00",
      2,
      { 
         { PAYME_VIRUS_SETTUP_POS, EMPTY, { &minmax[PULL_LEN] }, { &dpl[42] }}
      }
   },
   { /* PAYME_PASSWORD_SETTUP_POS */
      "Current PayME disable password :        New PayME disable password     :    0000",
      2,
      { 
         { PAYME_VIRUS_SETTUP_POS, EMPTY, { &minmax[PULL_LEN] }, { &dpl[43] }}
      }
   },
   { /* PULL_HIDDEN_SETUP_POS */
      "Enter new PULL value   Old value:  000.0Press CLEAR to CANCEL  New value:  000.0",
      2,
      { 
         { SYSTEM_SUPERVISORY_POS, EMPTY, { &minmax[PULL_LEN] }, { &dpl[20] }}
      }
   },
   { /* RATIO_HIDDEN_SETUP_POS */
      "Use arror keys to turn    Old value:    ratio settings            New value: 3:1",
      2,
      { 
         { SYSTEM_SUPERVISORY_POS, EMPTY, { NO_BLINK }, { &dpl[34] }}
      }
   },
   { /* HOLO_HIDDEN_SETUP_POS */
      "Use arror keys to turn    Old value:    ON/OFF holograms          New value: OFF",
      2,
      { 
         { SYSTEM_SUPERVISORY_POS, EMPTY, { NO_BLINK }, { &dpl[34] }}
      }
   },
   { /* BATTERY_LOW_POS */
      " BATTERY IS LOW !!!  REPLACE THE BATTERY         PRESS CLEAR TO CONTINUE !",
      2,
      { 
         { MAIN_STATE_ID, EMPTY, { NO_BLINK }, { &dpl[34] }}
      }
   },
//    { /* SYSTEM_SUPERVISORY_POS_LIMITED */
//       "B&H  PM  Pull  Ratio  Holo  Total   EXIT",
//       7,
//       { 
//          { SYSTEM_SETUP_POS, PULL_RAMP_OFFSET_POS, { &bpr[32] }, { &dpl[32] }},
//          { SYSTEM_SETUP_POS, SYSTEM_SUPERVISORY_POS_LIMITED, { &bpr[39] }, { &dpl[32] }},
//          { SYSTEM_SETUP_POS, PULL_HIDDEN_SETUP_POS,  { &bpr[33] }, { &dpl[33] }},
//          { SYSTEM_SETUP_POS, RATIO_HIDDEN_SETUP_POS, { &bpr[34] }, { &dpl[31] }},
//          { SYSTEM_SETUP_POS, SYSTEM_SUPERVISORY_POS_LIMITED,  { &bpr[35] }, { &dpl[30] }},
//          { SYSTEM_SETUP_POS, SYSTEM_SUPERVISORY_POS_LIMITED, { &bpr[36] }, { &dpl[38] }},
//          { SYSTEM_SETUP_POS, EXIT_POS_ID, { &bpr[15] }, { NO_DISPLAY }}
//       }
//    },
   { /* TOTAL_SETUP_POS !!!! */
      "     PRESS RESET KEY TO RESET VALUE          OR CLEAR KEY TO CANCEL CHANGE.     ",
      2,
      { 
         { SYSTEM_SUPERVISORY_POS, EMPTY, { NO_BLINK }, { NO_DISPLAY }}
      }
   },
   { /* PULL_RAMP_OFFSET_POS */
      "OFFSET-> Pull  Ramp  Initial speed  EXIT",
      4,
      { 
         { SYSTEM_SUPERVISORY_POS, PULL_OFFSET_POS, { &bpr[11] }, { &dpl[4] }  },
         { SYSTEM_SUPERVISORY_POS, RAMP_OFFSET_POS, { &bpr[37] }, { &dpl[5] }  },
         { SYSTEM_SUPERVISORY_POS, PWM_INITIAL_SPEED_POS, { &bpr[38] }, { &dpl[39] }  },
         { SYSTEM_SUPERVISORY_POS, EXIT_POS_ID, { &bpr[27] }, { NO_DISPLAY }    }
      }
   },
   { /* PULL_OFFSET_POS */
      "Enter PULL OFFSET value  Old value:   00Press CLEAR to CANCEL    New value:   00",
      2,
      { 
         { PULL_RAMP_OFFSET_POS, EMPTY, { &minmax[PULL_RAMP_OFFSET] }, { &dpl[21] }}
      }
   },
   { /* RAMP_OFFSET_POS */
      "Enter RAMP OFFSET value  Old value:   00Press CLEAR to CANCEL    New value:   00",
      2,
      { 
         { PULL_RAMP_OFFSET_POS, EMPTY, { &minmax[PULL_RAMP_OFFSET] }, { &dpl[21] }}
      }
   },
   { /* PWM_INITIAL_SPEED_POS */
      "Enter INITIAL SPEED value Old value: 000Press CLEAR to CANCEL     New value: 000",
      2,
      { 
         { PULL_RAMP_OFFSET_POS, EMPTY, { &minmax[INITIAL_SPEED_OFFSET] }, { &dpl[22] }}
      }
   },
   { /* PAYME_ACTIVATED */
      "Please insert correct user password or  contact us at +44 1480 474705     : ****",
      2,
      { 
         { MAIN_STATE_ID, EMPTY, { &minmax[PULL_LEN] }, { &dpl[44] }}
      }
   }
};

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


void stack_push_back(const unsigned char push)
{
   stack.buffer[ stack.head++ ] = push;
   stack.head &= ( STACK_SIZE - 1 );
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


unsigned char stack_pop_front(void)
{
   unsigned char retval = 0;

   if (stack.head != stack.tail) 
   {
      retval = stack.buffer[ --stack.head ];
      stack.head &= ( STACK_SIZE - 1 );
   }

   return retval;
}

void stack_purge(void)
{
   stack.head = 0;
   stack.tail = 0;
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void menu_init_blinking(void)
{
   unsigned char *flash = (unsigned char*)dfp;
   unsigned char *xram = (unsigned char*)system_data;
   unsigned char count;

   if (*flash == 0xFF) // In the first run when flash memory is cleared we need to set default values
   {      
      flash = (unsigned char*)flash_data;
      for (count=0 ; count < SYSTEM_DATA_SIZE; count++)
      {
         *xram = *flash;
         xram++;
         flash++;
      }
      DISABLE_INTERRUPTS;
      flash_write_page(SYSTEM_DATA_SIZE,(unsigned int)&system_data,(unsigned int)FLASH_START_POINT);
      ENABLE_INTERRUPTS;
   }
   else
   {
      for (count=0 ; count < SYSTEM_DATA_SIZE; count++)
      {
         *xram = *flash;
         xram++;
         flash++;
      }
   }
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void set_new_blink(void)
{
   BLINKING_OFF;

   lcd_goto_xy(current_blink_params->position, 0);
   lcd_put_string(current_blink_params->first_pattern_on);
   
   if (current_blink_params->lines > 1)
   {
      lcd_goto_xy(current_blink_params->position, 1);
      lcd_put_string(current_blink_params->second_pattern_on);
   }
   current_blink_params = menu[current_display_state].item[current_position].blink_union.blink;

   BLINKING_ON;
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


void display_on_off(const unsigned int value, const char pos, const char *on_str, const char *off_str)
{
   Data_Display *display;

   display = menu[current_display_state].item[pos].display;
   lcd_goto_xy(display->x, display->y);

   if (value == 0)
      lcd_put_string(off_str);
   else
      lcd_put_string(on_str);
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void display_stored_long(const unsigned long value, const char pos)
{
   Data_Display *display;

   display = menu[current_display_state].item[pos].display; 
   lcd_goto_xy(display->x, display->y);
   printf_fast((char code*)display->width,value);
}

void display_stored_int(const unsigned int value, const char pos) 
{
   Data_Display *display;
   
   display = menu[current_display_state].item[pos].display;
   lcd_goto_xy(display->x, display->y);
   printf_fast((char code*)display->width,value);
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void display_stored_time(const char pos, const char method)
{
   unsigned char sec;
   Data_Display *display;
   sec = system_time.tm_sec;
   ds1302_read_rtc(&system_time);

   switch (method)
   {
      case PAYME_FORMAT:
         display = menu[current_display_state].item[pos].display;
         local_gmtime(&system_data.payme.activation);
         lcd_goto_xy(display->x, display->y);
         printf_fast("%04u-%02u-%02u", 
                     (unsigned int)system_time.tm_year + 2000,
                     (unsigned int)system_time.tm_mon + 1,
                     (unsigned int)system_time.tm_mday);
         break;

      case ASCTIME_FORMAT:
         if (sec != system_time.tm_sec)
         {
            BLINKING_OFF;
            display = menu[current_display_state].item[pos].display;
            lcd_goto_xy(display->x, display->y);
            printf_asctime(&system_time);
            BLINKING_ON;
         }
         break;

      case DATE_FORMAT:
         display = menu[current_display_state].item[current_position].display;
         lcd_goto_xy(display->x, display->y);
         printf_fast("%04u-%02u-%02u", 
                     (unsigned int)system_time.tm_year + 2000,
                     (unsigned int)system_time.tm_mon + 1,
                     (unsigned int)system_time.tm_mday);
         break;

      case TIME_FORMAT:
         display = menu[current_display_state].item[current_position].display;
         lcd_goto_xy(display->x, display->y);
         printf_fast("%02u:%02u:%02u", 
                     (unsigned int)system_time.tm_hour,
                     (unsigned int)system_time.tm_min,
                     (unsigned int)system_time.tm_sec);
         break;
   }
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void display_stored_data(unsigned int *value)
{
   unsigned int temp;
   unsigned char i;
   unsigned int j;
   Data_Display *display;

   for (i = 0; i< (menu[current_display_state].max_pos - 1); i++)
   {
      display = menu[current_display_state].item[i].display;

      temp = *value;
      lcd_goto_xy(display->x, display->y);
      printf_fast((char code*)display->width,temp);
      
      if (display->point > 0)
      {       
         temp = *value;
         j = (unsigned int)(temp % 10);
         lcd_goto_xy(display->x+display->point, display->y);
         printf_fast(".%d",j);
      }
      value++;
   }
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */
 
void stop_button_handler(void)
{
   unsigned char exit;
   exit = 0;

   DISABLE_STEPPER;
   DISABLE_PULL_INTERRUPT;
   DISABLE_PCA_INTERRUPT;
   DISABLE_HOLO_INTERRUPT;
   HOLO_SPEED_PORT = 0;
   TIMER_1_STOP;
   TIMER_2_STOP;
   BLINKING_OFF;
   lcd_clear();
   printf_fast("       MACHINE HAS BEEN STOPPED !              PRESS RESET TO CONTINUE          ");
   
   keypad_purge_buffer();
   while (exit == 0)
   {
      SERVICE_WATCHDOG;
      if (keypad_check_front() == DATA_AVAILABLE)
      {
         if (keypad_pop_front() == RESET)
         {
            exit = 1;
         }
      }
   }
   
   BLINKING_ON;
   ENABLE_PULL_INTERRUPT;
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

unsigned char check_external_events(void)
{
   unsigned char retval;
   unsigned char exit;
   unsigned char key_code;
   retval = 0;
   exit = 0;

   if (system_alarm->type != NO_ALARM)
   {
      BLINKING_OFF;
      lcd_clear();
      printf_fast((char code*)system_alarm->description);

      keypad_purge_buffer();

      while (exit == 0)
      {
         SERVICE_WATCHDOG;
         if (keypad_check_front() == DATA_AVAILABLE)
         {
            key_code = keypad_pop_front();
            
            if (key_code == STOP)
            {
               stop_button_handler();
            }
            if (key_code == RESET)
            {
               exit = 1;
            }
            if (system_alarm->type == HOLO_SEARCH_ALARM)
            {
               if (key_code == HOLO)
               {
                  holo_search_start();
                  exit = 1;
               }
            }
         }
      }

      // in case of hologram alarm pull input is disabled
      // when the alarm gets cleared we then need to enable them again
      ENABLE_PULL_INTERRUPT;

      alarm_off();
      retval = 1;
      BLINKING_ON;
   }

   if (count_refresh_display == true)
   {
      if (current_display_state == COUNT_SETUP_POS)
      {
         BLINKING_OFF;
         // display_stored_data replaced with display_stored_int due to some strange behaviour
         // display_stored_data((unsigned int*)system_data.count);
         display_stored_long(system_data.count.impress,0); 
         display_stored_int(system_data.count.foilused,1);
         display_stored_long(system_data.count.impress_total,2); 
         BLINKING_ON;
      }
      if (current_display_state == FOIL_ALARM_POS)
      {
         BLINKING_OFF;
         display_stored_data((unsigned int*)system_data.foil_alarm);
         BLINKING_ON;
      }
      count_refresh_display = false;
   }

   if (current_display_state == SYSTEM_SETUP_POS)
   {
      display_stored_time(0,ASCTIME_FORMAT);
   }

   if (update_flash == true)
   {
      if (IS_STEPPER_DISABLED) // We will disable interrupts during this call
      {
         DISABLE_INTERRUPTS;
         flash_write_page(SYSTEM_DATA_SIZE,(unsigned int)&system_data,(unsigned int)FLASH_START_POINT);
         update_flash = false;
         ENABLE_INTERRUPTS;
      }
   }

   if (INCH_BUTTON_PRESSED)
   {
      inch_mode();
   }

   return retval;
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


unsigned char check_extra_keys(unsigned char pressed)
{
   unsigned char retval;
   unsigned char exit;
   exit = 0;
   retval = 0;

   switch (pressed)
   {
      case TEST:
         start_pull();
         break;
         
      case STOP:
         stop_button_handler();
         retval = 1;
         break;

      case INCH:
         inch_mode();
         break;
         
      case HOLO:
         holo_search_start();
         break;
   }

   return retval;
}

void menu_previous_position()
{
    current_display_state = menu[current_display_state].item[current_position].previous;
    current_position = stack_pop_front();
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


void password_workloop()
{
   unsigned char exit = 0;
   unsigned char i = 0;
   unsigned char result;
   unsigned char retval = true;
   unsigned char retval_ltd = true;
   unsigned char extra = 0;
   
   BLINKING_OFF;

   while(exit == 0)
   {
      SERVICE_WATCHDOG;
      if (keypad_check_front() == DATA_AVAILABLE)
      {
         result = keypad_pop_front();
         extra = check_extra_keys(result);
         exit = extra;
         
         result += ASCII_OFFSET;
         printf_fast("*");
         
         if ( result != password[i] )
         {
            retval = false;
         }

         if ( result != password_ltd[i++] )
         {
            retval_ltd = false;
         }
         
         if (i==6)
            exit = 1;
            
         keypad_purge_buffer();
      }
   }
    
   if ( retval == true ) 
   {
      current_display_state = SYSTEM_SUPERVISORY_POS;
      current_password_entered = FULL_PASSWORD;
      current_position = 0;
   }
   if ( retval_ltd == true ) 
   {
      current_display_state = SYSTEM_SUPERVISORY_POS;
      current_password_entered = LIMITED_PASSOWRD;
      current_position = 0;
   }

   if (retval == false && retval_ltd == false)
   {
      if (extra == 0)
      {
         lcd_clear();
         printf_fast("         WRONG PASSWORD ENTERED !              PRESS ANY KEY TO CONTINUE");
         
         exit = 0;
         while(exit == 0)
         {
            SERVICE_WATCHDOG;
            if (keypad_check_front() == DATA_AVAILABLE)
            {
               result = keypad_pop_front();
               exit = 1;
            }
         }
      }
      
      current_display_state = MAIN_STATE_ID;
      current_position = 0;
      stack_purge();
      keypad_purge_buffer();
      ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);
   } 
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


unsigned long reset_workloop(const unsigned long value)
{
   unsigned char exit = 0;
   unsigned int result;
   
   BLINKING_OFF;
   result = value;

   while(exit == 0)
   {
      /******************************************************************************/
      /* We also need to check if alarm is on or inch button pressed and so on      */
      /* Putting this on the end of while() would be better but then all hangs ?!!! */
      SERVICE_WATCHDOG;
      exit = check_external_events();
      if (exit == 0)
      {
         if (keypad_check_front() == DATA_AVAILABLE)
         {
            key_code = keypad_pop_front();
            exit = check_extra_keys(key_code);

            switch (key_code)
            {
               case RESET:
                  exit = 1;
                  result = 0;
                  break;
                  
               case CLEAR:
                  result = value;
                  exit = 1;
                  break;
            }
            keypad_purge_buffer();
         }
      }
   }

   menu_previous_position();
   //   current_display_state = menu[current_display_state].item[current_position].previous;
   //current_position = stack_pop_front();
   ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);

   return result;
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


unsigned int on_off_workloop(const unsigned int value, const char* on_str, const char* off_str)
{
   Data_Display *display;
   unsigned char exit = 0;
   unsigned int result;
   
   BLINKING_OFF;
   result = value;

   display = menu[current_display_state].item[current_position].display;

   lcd_goto_xy(display->x, display->y+1);
   if (value != 0)
   {
      lcd_put_string(on_str);
   }
   else
   {
      lcd_put_string(off_str);
   }

   while(exit == 0)
   {
      SERVICE_WATCHDOG;
      /******************************************************************************/
      /* We also need to check if alarm is on or inch button pressed and so on      */
      /* Putting this on the end of while() would be better but then all hangs ?!!! */
      exit = check_external_events();
      if (exit == 0)
      {
         if (keypad_check_front() == DATA_AVAILABLE)
         {
            key_code = keypad_pop_front();
            exit = check_extra_keys(key_code);
            
            if (exit == 1)
            {
               result = value;
            }
            
            switch (key_code)
            {
               case ENTER:
                  exit = 1;
                  break;
                  
               case CLEAR:
                  result = value;
                  exit = 1;
                  break;
                  
               case LEFT:
                  if (result == 1)
                  {
                     result = 0;
                     lcd_goto_xy(display->x, display->y+1);
                     lcd_put_string(off_str);
                  }
                  else
                  {
                     result = 1;
                     lcd_goto_xy(display->x, display->y+1);
                     lcd_put_string(on_str);
                  }
                  break;
                  
               case RIGHT:
                  if (result == 1)
                  {
                     result = 0;
                     lcd_goto_xy(display->x, display->y+1);
                     lcd_put_string(off_str);
                  }
                  else
                  {
                     result = 1;
                     lcd_goto_xy(display->x, display->y+1);
                     lcd_put_string(on_str);
                  }
                  break;
            }
            keypad_purge_buffer();
         }
      }
   }

   menu_previous_position();
//    current_display_state = menu[current_display_state].item[current_position].previous;
//    current_position = stack_pop_front();
   ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);

   return result;
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


unsigned int read_keyboard_data_workloop(unsigned char size, const unsigned int value, const char point, const boolean isBCD)
{
   unsigned char count,j,exit;
   unsigned int result;

   count = 1;
   result = 0;

   while(count <= size)
   {
      SERVICE_WATCHDOG;
      /******************************************************************************/
      /* We also need to check if alarm is on or inch button pressed and so on      */
      /* Putting this on the end of while() would be better but then all hangs ?!!! */
      exit = check_external_events();
      if (exit == 0)
      {
         SERVICE_WATCHDOG;
         if (keypad_check_front() == DATA_AVAILABLE)
         {
            key_code = keypad_pop_front();
            lcd_cursor_off();
            if (check_extra_keys(key_code) == 1)
            {
               key_code = CLEAR;
            }
            lcd_cursor_on();
            
            //if (key_code < 10 && key_code >= 0 )
            if (key_code < 10)
            {
               if (count == (unsigned char)point)
               {
                  printf_fast("%d.",key_code);
               }
               else
               {
                  printf_fast("%d", key_code);
               }
            
               if (isBCD == false)
               {
                  result *= 10;
                  result += key_code;
               }
               else 
               {
                  result <<= 4;
                  result |= key_code;
               }
               
               count++;
            }
            else
            {
               switch (key_code)
               {
                  case ENTER:
                     for (j=0; j<=(size-count);j++)
                     {
                        if (isBCD == false)
                        {
                           result *= 10;
                        }
                        else
                        {
                           result <<= 4;
                        }
                     }
                     count = size+1;
                     break;
                     
                  case CLEAR:
                     count = size+1;
                     result = value;
                     break;
               }
            }
            keypad_purge_buffer();
         }
      }
      else
      {
         result = value;
         count = size+1;
      }
   }

   return result;
}

void time_workloop(void)
{
   unsigned int result;
   Data_Display *display;
   result = 0;

   BLINKING_OFF;

   display = menu[current_display_state].item[current_position].display;

   lcd_goto_xy(display->x, display->y+1);
   lcd_cursor_on();
   ds1302_read_rtc_bcd(&system_time);

   result = read_keyboard_data_workloop(2,system_time.tm_hour,0,true);
   if (result != system_time.tm_hour)
   {
      if (result <= 0x23)
      {
         system_time.tm_hour = (unsigned char)result&0xff;
      }
   }

   lcd_goto_xy(display->x+3, display->y+1);

   result = read_keyboard_data_workloop(2,system_time.tm_min,0,true);
   if (result != system_time.tm_min)
   {
      if (result <= 0x59)
      {
         system_time.tm_min = (unsigned char)result&0xff;
      }
   }

   lcd_goto_xy(display->x+6, display->y+1);

   result = read_keyboard_data_workloop(2,system_time.tm_sec,0,true);
   if (result != system_time.tm_sec)
   {
      if (result <= 0x59)
      {
         system_time.tm_sec = (unsigned char)result&0xff;
      }
   }

   ds1302_write_rtc(&system_time);

   lcd_cursor_off();
   menu_previous_position();
   //   current_display_state = menu[current_display_state].item[current_position].previous;
   //current_position = stack_pop_front();
   ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void date_workloop(const unsigned char payme_or_data, const unsigned char isBCD)
{
   unsigned int result;
   unsigned char temp;
   unsigned char ib;
   Data_Display *display;
   result = 0;
   ib = isBCD;

   BLINKING_OFF;

   display = menu[current_display_state].item[current_position].display;

   lcd_goto_xy(display->x, display->y+1);
   lcd_cursor_on();

   if (payme_or_data == PAYME_FORMAT) 
   {
      local_gmtime(&system_data.payme.activation);
   }
   else
   {
      ds1302_read_rtc_bcd(&system_time);
   }

   //   if (payme_or_data != PAYME_FORMAT && isBCD == true)
   if (payme_or_data != PAYME_FORMAT)
   {
      result = read_keyboard_data_workloop(4,0x2004,0,true);
      if (result >= 0x2004)
      {
         system_time.tm_year = (unsigned char)(result & 0xff);
      }
   }
   else // here we get non BCD number passed, just check the year range
   {
      result = read_keyboard_data_workloop(4,0x2004,0,true);
      if (result >= 0x2004)
      {
         temp = (unsigned char)result&0xff;
         temp = ((temp >> 4)&0xf)*10 + (unsigned char)result&0xf;
         system_time.tm_year = temp + 100;
      }
      else 
      {
         system_time.tm_year = 0;
      }
   }
   
   lcd_goto_xy(display->x+5, display->y+1);

   result = read_keyboard_data_workloop(2,system_time.tm_mon,0,false);
   //
   // We need to decrese month number to have this displayed successfully
   //
   if (result != system_time.tm_mon)
   {
      result--;
      system_time.tm_mon = (unsigned char)(result & 0xff);
   }

   lcd_goto_xy(display->x+8, display->y+1);

   result = read_keyboard_data_workloop(2,system_time.tm_mday,0,ib);
   if (result != system_time.tm_mday)
   {
      system_time.tm_mday = (unsigned char)(result & 0xff);
   }

   if (payme_or_data == PAYME_FORMAT) 
   {
      if (system_time.tm_year == 0) 
         system_data.payme.activation = 0;
      else
         system_data.payme.activation = (unsigned long)local_mktime(&system_time, 1900);
   }
   else
   {
      ds1302_write_rtc(&system_time);
   }

   lcd_cursor_off();
   menu_previous_position();
   ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */


unsigned int numeric_workloop(unsigned char size, const unsigned int value, const unsigned char offset)
{
   unsigned int result;
   Data_Display *display;
   Min_Max_params *minmax;

   BLINKING_OFF;

   display = menu[current_display_state].item[current_position].display;
   minmax = menu[current_display_state].item[current_position].blink_union.min_max;

   lcd_goto_xy(display->x+offset, display->y+1);
   lcd_cursor_on();

   result = read_keyboard_data_workloop(size,value,display->point,false);

   if (result < minmax->min)
   {
      result = minmax->min;
   }

   if (result > minmax->max)
   {
      result = minmax->max;
   }

   lcd_cursor_off();
   menu_previous_position();
   //   current_display_state = menu[current_display_state].item[current_position].previous;
   //current_position = stack_pop_front();
   ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);

   return result;
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void refresh_count_display(void)
{
   if (current_display_state == COUNT_SETUP_POS || current_display_state == FOIL_ALARM_POS)
   {
      count_refresh_display = true;
   }
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void workloop(void)
{
   unsigned char exit = 0;
   BLINKING_ON;

   while(exit == 0)
   {
      SERVICE_WATCHDOG;
      /******************************************************************************/
      /* We also need to check if alarm is on or inch button pressed and so on      */
      /* Putting this on the end of while() would be better but then all hangs ?!!! */
      exit = check_external_events();
      if (exit == 0)
      {
         if (keypad_check_front() == DATA_AVAILABLE)
         {
            key_code = keypad_pop_front();
            exit = check_extra_keys(key_code);
            
            switch (key_code)
            {
               case ENTER:
                  if ((unsigned char)menu[current_display_state].item[current_position].next == EXIT_POS_ID)
                  {
                     menu_previous_position();
                  }
                  else
                  {
                     current_display_state = menu[current_display_state].item[current_position].next;
                     stack_push_back(current_position);
                     current_position = 0;
                  }
                  ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);
                  exit = 1;
                  break;
                  
               case CLEAR:
                  menu_previous_position();
                  ds1302_write_memory(RTC_CURRENT_DISPLAY_STATE,current_display_state);
                  exit = 1;
                  break;
                  
               case RIGHT:
                  current_position++;
                  if (current_position == (unsigned char)menu[current_display_state].max_pos)
                  {
                     current_position = 0;
                  }
                  set_new_blink();
                  break;
                  
               case LEFT:
                  current_position--;
                  if (current_position == 0xFF)
                  {
                     current_position = menu[current_display_state].max_pos - 1;
                  }
                  set_new_blink();
                  break;
            }
            keypad_purge_buffer();
         }
      }
   }
}

/**
 * Brief description. Full description.
 * @param .
 * @param .
 * @return .
 * @see
 */

void battery_test(void)
{
   unsigned long temp_long;
   
   BATT_TEST_PORT = 0;

   for ( ; battery_counter < 2000; battery_counter++ )
   {
      SERVICE_WATCHDOG;
   }

   if (BATT_FAIL_PORT == 0)
   {
      current_display_state = BATTERY_LOW_POS;
   }
   else
   {
      current_display_state = ds1302_read_memory(RTC_CURRENT_DISPLAY_STATE);
      
      if (current_display_state >= LATEST_DONT_REMOVE)
      {
         current_display_state = 0;
      }

      // Let's test if out memory works OK
      if (ds1302_read_memory(RTC_TEST_BYTE) == 0x55)
      {
         system_data.foil_alarm.roll_counter = ds1302_read_memory(RTC_ALARM_ROLL_COUNT_MSB);
         system_data.foil_alarm.roll_counter <<= 8;
         system_data.foil_alarm.roll_counter &= 0xff00;
         system_data.foil_alarm.roll_counter |= ds1302_read_memory(RTC_ALARM_ROLL_COUNT_LSB);
         temp_long = system_data.foil_alarm.roll_counter;
         temp_long *= MILIMETERS_PER_METER;
         system_data.count.roll_milimeters = temp_long;
         
         system_data.count.foilused = ds1302_read_memory(RTC_COUNT_FOILUSED_MSB);
         system_data.count.foilused <<= 8;
         system_data.count.foilused &= 0xff00;
         system_data.count.foilused |= ds1302_read_memory(RTC_COUNT_FOILUSED_LSB);

         temp_long = system_data.count.foilused;
         temp_long *= MILIMETERS_PER_METER;
         system_data.count.foilused_milimeters = temp_long;

         system_data.count.impress = ds1302_read_memory(RTC_IMPRESS_MSB);
         system_data.count.impress <<= 8;
         system_data.count.impress |= ds1302_read_memory(RTC_IMPRESS_LSB_2);
         system_data.count.impress <<= 8;
         system_data.count.impress |= ds1302_read_memory(RTC_IMPRESS_LSB_1);
         system_data.count.impress <<= 8;
         system_data.count.impress |= ds1302_read_memory(RTC_IMPRESS_LSB);

         system_data.count.impress_total = ds1302_read_memory(RTC_IMPRESS_TOTAL_MSB);
         system_data.count.impress_total <<= 8;
         system_data.count.impress_total |= ds1302_read_memory(RTC_IMPRESS_TOTAL_LSB_2);
         system_data.count.impress_total <<= 8;
         system_data.count.impress_total |= ds1302_read_memory(RTC_IMPRESS_TOTAL_LSB_1);
         system_data.count.impress_total <<= 8;
         system_data.count.impress_total |= ds1302_read_memory(RTC_IMPRESS_TOTAL_LSB);
      }
   }

   BATT_TEST_PORT = 1; /* Also set by lcd driver before */
}


void main_program_workloop(void)
{
   battery_test();

   if (system_data.system.virus == PAYME_VIRUS_ENABLED) 
   {
      ds1302_read_rtc(&system_time);
      
      // First we check if battery was removed thus resulting in clock stop
      if (system_time.tm_year == 0 &&
          system_time.tm_mon == 1 &&
          system_time.tm_mday == 1 &&
          system_time.tm_hour == 0 &&
          system_time.tm_min == 0) 
      {
         current_display_state = PAYME_ACTIVATED;
      }
      // And then chcek if payme time has elapsed
      else 
      {
         if ( local_mktime(&system_time, 2000) >= system_data.payme.activation)
         {
            current_display_state = PAYME_ACTIVATED;
         }
      }
   }

   stack_purge();
   ENABLE_INTERRUPTS;
   ENABLE_PULL_INTERRUPT;

   HOLO_SPEED_PORT = 0;

   while (1)
   {
      enter_workloop = true;
      
      BLINKING_OFF;
      lcd_clear();
      lcd_put_string(menu[current_display_state].text);
      current_blink_params = menu[current_display_state].item[current_position].blink_union.blink;     
      SERVICE_WATCHDOG;

      switch (current_display_state)
      {
         case BATTERY_LOW_POS:
            reset_workloop(0);
            enter_workloop = false;
            break;

         case PAYME_ACTIVATED:
            DISABLE_STEPPER;
            DISABLE_PULL_INTERRUPT;
            DISABLE_PCA_INTERRUPT;
            if (system_data.payme.password ==  numeric_workloop(4, 0 ,NO_LCD_X_OFFSET))
            {
               system_data.payme.activation = 0;
               system_data.system.virus = PAYME_VIRUS_DISABLED;
               update_flash = true;
               ENABLE_STEPPER;
               ENABLE_PULL_INTERRUPT;
               ENABLE_PCA_INTERRUPT;
            }
            else 
            {
               current_display_state = PAYME_ACTIVATED;
            }

            enter_workloop = false;
            break;
            
            /****************************************************/
            /*  FOIL->   Pull    Step    Clear   Delay   EXIT   */
            /****************************************************/
         case FOIL_PULL_POS:
            display_stored_data((unsigned int*)system_data.foil_pull);
            break;

         case PULL_INPUT_POS:
            display_stored_data(&system_data.foil_pull.pull);
            system_data.foil_pull.pull = numeric_workloop(4, system_data.foil_pull.pull,NO_LCD_X_OFFSET);
            if (system_data.foil_pull.pull > system_data.system.length)
            {
               system_data.foil_pull.pull = system_data.system.length;
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case STEP_INPUT_POS:
            display_stored_data(&system_data.foil_pull.steps);
            system_data.foil_pull.steps = numeric_workloop(2, system_data.foil_pull.steps,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case CLEAR_INPUT_POS:
            display_stored_data(&system_data.foil_pull.clear);
            system_data.foil_pull.clear = numeric_workloop(4, system_data.foil_pull.clear,NO_LCD_X_OFFSET);
            if (system_data.foil_pull.clear > system_data.system.length)
            {
               system_data.foil_pull.clear = system_data.system.length;
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case DELAY_INPUT_POS:
            display_stored_data(&system_data.foil_pull.delay);
            system_data.foil_pull.delay = numeric_workloop(4, system_data.foil_pull.delay,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

            /*******************************************************/
            /*  SPEED->   Pull    Clear    Inch    Ramp     EXIT   */
            /*******************************************************/

         case SPEED_ADJUST_POS:
            display_stored_data((unsigned int*)system_data.speed_adjust);
            break;

         case PULL_SPEED_POS:
            display_stored_data(&system_data.speed_adjust.pull);
            system_data.speed_adjust.pull = numeric_workloop(2, system_data.speed_adjust.pull,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case CLEAR_SPEED_POS:
            display_stored_data(&system_data.speed_adjust.clear);
            system_data.speed_adjust.clear = numeric_workloop(2, system_data.speed_adjust.clear,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case INCH_SPEED_POS:
            display_stored_data(&system_data.speed_adjust.inch);
            system_data.speed_adjust.inch = numeric_workloop(2, system_data.speed_adjust.inch,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;
            
         case RAMP_SPEED_POS:
            display_stored_data(&system_data.speed_adjust.ramp);
            system_data.speed_adjust.ramp = numeric_workloop(2, system_data.speed_adjust.ramp,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

            /*******************************************************/
            /*  ALARM->   Enable   Foil Roll Size           EXIT   */
            /*******************************************************/
            
         case FOIL_ALARM_POS:
            display_stored_data((unsigned int*)system_data.foil_alarm);
            break;

         case FOIL_ALARM_ENABLE_POS:
            display_stored_data(&system_data.foil_alarm.enable);
            system_data.foil_alarm.enable = numeric_workloop(2, system_data.foil_alarm.enable,NO_LCD_X_OFFSET);
            if (system_data.foil_alarm.enable > system_data.foil_alarm.roll) 
            {
               system_data.foil_alarm.enable = system_data.foil_alarm.roll;
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case FOIL_ROLL_SIZE_POS:
            display_stored_data(&system_data.foil_alarm.roll);
            system_data.foil_alarm.roll = numeric_workloop(3, system_data.foil_alarm.roll,NO_LCD_X_OFFSET);
            if (system_data.foil_alarm.enable > system_data.foil_alarm.roll) 
            {
               system_data.foil_alarm.enable = system_data.foil_alarm.roll;
            }
            if (system_data.foil_alarm.roll == 0) 
            {
               system_data.foil_alarm.roll_counter = 0;
               ds1302_write_memory(RTC_ALARM_ROLL_COUNT_LSB,0);
               ds1302_write_memory(RTC_ALARM_ROLL_COUNT_MSB,0);
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case FOIL_ALARM_ROLL_POS:
            system_data.foil_alarm.roll_counter = (unsigned int)reset_workloop(system_data.foil_alarm.roll_counter);
            if (system_data.foil_alarm.roll_counter == 0)
            {
               system_data.count.roll_milimeters = 0;
               ds1302_write_memory(RTC_ALARM_ROLL_COUNT_LSB,0);
               ds1302_write_memory(RTC_ALARM_ROLL_COUNT_MSB,0);
            }
            update_flash = true;
            enter_workloop = false;
            break;

            /*******************************************************/
            /*  HOLO->   Active  Offset  Overrun  Speed     EXIT   */
            /*******************************************************/
            
         case HOLOGRAM_SETUP_POS:
            display_stored_data((unsigned int*)system_data.hologram);
            display_on_off(system_data.hologram.active,0,"ON ","OFF");
            break;

         case HOLO_ACTIVE_POS:
            if (system_data.system.active != 0)
            {
               display_on_off(system_data.hologram.active,0,"ON ","OFF");
               system_data.hologram.active = on_off_workloop(system_data.hologram.active,"ON ","OFF");
            }
            else
            {
               menu_previous_position();
//                current_display_state = menu[current_display_state].item[current_position].previous;
//                current_position = stack_pop_front();
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case HOLO_OFFSET_POS:
            display_stored_data(&system_data.hologram.offset);
            system_data.hologram.offset = numeric_workloop(3, system_data.hologram.offset,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case HOLO_OVERRUN_POS:
            display_stored_data(&system_data.hologram.overrun);
            system_data.hologram.overrun = numeric_workloop(3, system_data.hologram.overrun,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case HOLO_SPEED_POS:
            display_stored_data(&system_data.hologram.speed);
            system_data.hologram.speed = numeric_workloop(2, system_data.hologram.speed,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case COUNT_SETUP_POS:
            // display_stored_data replaced with display_stored_int due to some strange behaviour
            //            display_stored_data((unsigned int*)system_data.count);
            display_stored_long(system_data.count.impress,0);
            display_stored_int(system_data.count.foilused,1);
            display_stored_long(system_data.count.impress_total,2);
            break;

         case COUNT_SETUP_IMPRESS_POS:
            system_data.count.impress = reset_workloop(system_data.count.impress);
            if (system_data.count.impress == 0)
            {
               ds1302_write_memory(RTC_IMPRESS_LSB,0);
               ds1302_write_memory(RTC_IMPRESS_LSB_1,0);
               ds1302_write_memory(RTC_IMPRESS_LSB_2,0);
               ds1302_write_memory(RTC_IMPRESS_MSB,0);
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case COUNT_SETUP_FOIL_USED_POS:
            system_data.count.foilused = (unsigned int)reset_workloop(system_data.count.foilused);
            if (system_data.count.foilused == 0)
            {               
               system_data.count.foilused_milimeters = 0;
               ds1302_write_memory(RTC_COUNT_FOILUSED_LSB,0);
               ds1302_write_memory(RTC_COUNT_FOILUSED_MSB,0);
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case MAINTENANCE_POS:
            password_workloop();
            enter_workloop = false;
            break;

         case SYSTEM_SUPERVISORY_POS:
            display_stored_data((unsigned int*)system_data.system);
            display_on_off(system_data.system.virus,0,"     ","     ");
            display_on_off(system_data.system.ratio,3,"3:1","4:1");
            display_on_off(system_data.system.active,4,"ON ","OFF");
            display_stored_long(system_data.count.impress_total,5);
            break;

         case PAYME_VIRUS_SETTUP_POS: /* LIMITED FUNCTIONALITY */
            if (current_password_entered == FULL_PASSWORD) 
            {
               if (system_data.payme.activation == 0)
               {
                  display_on_off(0,0,"000-00-00 ","0000-00-00");
               }
               else 
               {
                  display_stored_time(0, PAYME_FORMAT);
               }
               display_stored_int(system_data.payme.password,1);
               display_on_off(system_data.system.virus,2,"ENABLED ","DISABLED");
            }
            else
            {
               menu_previous_position();
               enter_workloop = false;
            }
            break;

         case PAYME_ACTIVATION_SETTUP_POS:
            display_stored_time(0, PAYME_FORMAT);
            date_workloop(PAYME_FORMAT, false);
            ds1302_read_rtc(&system_time);
            
            if ( system_data.payme.activation >= local_mktime(&system_time, 2000)) 
            {
               system_data.system.virus = PAYME_VIRUS_ENABLED;
            }
            else 
            {
               system_data.system.virus = PAYME_VIRUS_DISABLED;
            }
            update_flash = true;
            enter_workloop = false;
            break;

         case PAYME_PASSWORD_SETTUP_POS:
            display_stored_int(system_data.payme.password,0);
            system_data.payme.password = numeric_workloop(4, system_data.payme.password,NO_LCD_X_OFFSET);
            update_flash=true;
            enter_workloop=false;
            break;

         case PULL_HIDDEN_SETUP_POS:
            display_stored_data(&system_data.system.length);
            system_data.system.length = numeric_workloop(4, system_data.system.length,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case RATIO_HIDDEN_SETUP_POS:
            display_on_off(system_data.system.ratio,0,"3:1","4:1");
            system_data.system.ratio = on_off_workloop(system_data.system.ratio,"3:1","4:1");
            update_flash = true;
            enter_workloop = false;
            break;

         case HOLO_HIDDEN_SETUP_POS: /*LIMITED FUNCTIONALITY*/
            if (current_password_entered == FULL_PASSWORD) 
            {
               display_on_off(system_data.system.active,0,"ON ","OFF");
               system_data.system.active = on_off_workloop(system_data.system.active,"ON ","OFF");
               //
               // Disabling holograms in hidden menu should result in disabling in disabling user menu
               //
               if (system_data.system.active == 0)
               {
                  system_data.hologram.active = 0;
               }
               update_flash = true;
            }
            else 
            {
               menu_previous_position();
            }
            enter_workloop = false;
            break;

         case DATE_SETUP_POS:
            if (system_data.system.virus == PAYME_VIRUS_DISABLED) 
            {
               display_stored_time(0, DATE_FORMAT);
               date_workloop(DATE_FORMAT, true);
            }
            else
            {
               menu_previous_position();
            }
            enter_workloop = false;
            break;

         case TIME_SETUP_POS:
            if (system_data.system.virus == PAYME_VIRUS_DISABLED) 
            {
               display_stored_time(0, TIME_FORMAT);
               time_workloop();
            }
            else 
            {
               menu_previous_position();
            }
            enter_workloop = false;
            break;

         case TOTAL_SETUP_POS: /* LIMITED FUNCTIONALITY */
            if (current_password_entered == FULL_PASSWORD) 
            {
               system_data.count.impress_total = reset_workloop(system_data.count.impress_total);
               if (system_data.count.impress_total == 0)
               {
                  ds1302_write_memory(RTC_IMPRESS_TOTAL_LSB,0);
                  ds1302_write_memory(RTC_IMPRESS_TOTAL_LSB_1,0);
                  ds1302_write_memory(RTC_IMPRESS_TOTAL_LSB_2,0);
                  ds1302_write_memory(RTC_IMPRESS_TOTAL_MSB,0);
               }
               update_flash = true;
            }
            else 
            {
               menu_previous_position();
            }
            enter_workloop = false;
            break;

            /****************************************************/
            /*  OFFSET->   Pull    Ramp                  EXIT   */
            /****************************************************/

         case PULL_RAMP_OFFSET_POS:
            display_stored_data((unsigned int*)system_data.offset);
            break;

         case PULL_OFFSET_POS:
            display_stored_data(&system_data.offset.pull_offset);
            system_data.offset.pull_offset = numeric_workloop(2, system_data.offset.pull_offset,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case RAMP_OFFSET_POS:
            display_stored_data(&system_data.offset.ramp_offset);
            system_data.offset.ramp_offset = numeric_workloop(2, system_data.offset.ramp_offset,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;

         case PWM_INITIAL_SPEED_POS:
            display_stored_data(&system_data.offset.initial_speed);
            system_data.offset.initial_speed = numeric_workloop(3, system_data.offset.initial_speed,NO_LCD_X_OFFSET);
            update_flash = true;
            enter_workloop = false;
            break;
      }

      if (enable_pull == false)
      {
         ENABLE_PULL_INTERRUPT;
         enable_pull = true;
      }

      if (enter_workloop == true)
         workloop();
   }
}

