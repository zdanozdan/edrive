/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/global.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: global.h,v $
**  Revision 1.10  2005/06/22 20:04:54  tomasz
**  global changes to fix the problem with slow LCD/Keypad reaction when motor in motion
**
**  Revision 1.9  2004/11/19 20:33:36  tomasz
**  latest changes incoroprated
**
**  Revision 1.8  2004/08/05 12:40:15  tomasz
**  no message
**
**  Revision 1.7  2004/01/28 09:54:02  tomasz
**  Interrupt level on EX port added
**
**  Revision 1.6  2004/01/19 13:46:47  tomasz
**  changed size of structs defs
**
**  Revision 1.5  2004/01/12 22:34:05  tomasz
**  New global defs added
**
**  Revision 1.4  2003/12/28 14:57:59  tomasz
**  interrupts global defines added
**
**  Revision 1.3  2003/12/22 10:54:00  tomasz
**  new features
**
**  Revision 1.2  2003/11/14 18:10:27  tomasz
**  ported to compile with sdcc c51 compiler
**
**  Revision 1.1  2003/11/08 12:00:19  tomasz
**  first release
**
**  $Id: global.h,v 1.10 2005/06/22 20:04:54 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "89c51rd2.h"

#ifndef NULL
#define NULL  0
#endif

#define TRUE  1
#define FALSE 0
#define true  1
#define false 0

#define ASCII_OFFSET          0x30

#define ENABLE_XRAM_1024      AUXR |= 0xC0;

//
//Define this if X2 mode bit is set in the core
//
#define X2_MODE

typedef unsigned char BOOL;
typedef unsigned char bool;
typedef unsigned char boolean;

#define ENABLE_INTERRUPTS      EA=1
#define DISABLE_INTERRUPTS     EA=0
#define ENABLE_PCA_INTERRUPT   EC=1
#define DISABLE_PCA_INTERRUPT  EC=0
#define IS_PCA_ENABLED         EC==1
#define IS_PCA_DISABLED        EC==0
#define ENABLE_EXT_INTERRUPT   EX1=1
#define DISABLE_EXT_INTERRUPT  EX1=0
#define ENABLE_T1_INTERRUPT    ET1=1
#define DISABLE_T1_INTERRUPT   ET1=0
#define ENABLE_T0_INTERRUPT    ET0=1
#define DISABLE_T0_INTERRUPT   ET0=0
#define ENABLE_T2_INTERRUPT    ET2=1
#define DISABLE_T2_INTERRUPT   ET2=0
#define ENABLE_PULL_INTERRUPT  EX0=1
#define DISABLE_PULL_INTERRUPT EX0=0
#define ENABLE_HOLO_INTERRUPT  EX1=1
#define DISABLE_HOLO_INTERRUPT EX1=0
#define HOLO_NEGATIVE_TRIGGER  IT1=1
#define HOLO_LOW_LEVEL_TRIGGER IT1=0
#define IS_HOLO_ENABLED        EX1==1
#define IS_HOLO_DISABLED       EX1==0

#define ENABLE_RAMP_INTERRUPT  CCF1=1
#define DISABLE_RAMP_INTERRUPT CCF1=0

#define ENABLE_STEPPER         P1_2=0
#define DISABLE_STEPPER        P1_2=1
#define IS_STEPPER_ENABLED     P1_2==0
#define IS_STEPPER_DISABLED    P1_2==1

#define INCH_BUTTON_PRESSED    P1_0==0
#define INCH_BUTTON_RELEASED   P1_0==1

#define PCA_PRIORITY_BIT       0x40
#define TIMER2_PRIORITY_BIT    0x20
#define SERIAL_PRIRITY_BIT     0x10
#define TIMER1_PRIORITY_BIT    0x8
#define EXTERNAL1_PRIORITY_BIT 0x4
#define TIMER0_PRIORITY_BIT    0x2
#define EXTERNAL0_PRIORITY_BIT 0x1

#define TIMER_0_RUN            TR0=1
#define TIMER_0_STOP           TR0=0
#define TIMER_1_RUN            TR1=1
#define TIMER_1_STOP           TR1=0
#define TIMER_2_RUN            TR2=1
#define TIMER_2_STOP           TR2=0

//  #define BUZZER_ON              P1_4=1
//  #define BUZZER_OFF             P1_4=0

#define BUZZER_ON              P3_6=0
#define BUZZER_OFF             P3_6=1

#define SERVICE_WATCHDOG       WDTRST=0x1e; WDTRST=0xe1;

//  #define HOLO_SPEED_PORT        P3_6
#define HOLO_SPEED_PORT        P1_4

#define BATT_TEST_PORT         P2_7
#define BATT_TEST_BIT_MASK     0x00 /* P2_7 */
#define BATT_FAIL_PORT         P1_1

#define SET_MAX_WDT_DELAY      WDTPRG=0x7

#define FLASH_START_POINT      0x7d00

typedef struct _FoilPull 
{
      unsigned int pull;
      unsigned int steps;
      unsigned int clear;
      unsigned int delay;
} FoilPull;

typedef struct _SpeedAdjust 
{
      unsigned int pull;
      unsigned int clear;
      unsigned int inch;
      unsigned int ramp;
} SpeedAdjust;

typedef struct _FoilAlarm 
{
      unsigned int enable;
      unsigned int roll;
      unsigned int roll_counter;
} FoilAlarm;

typedef struct _Hologram 
{
      unsigned int active;
      unsigned int offset;
      unsigned int overrun;
      unsigned int speed;
} Hologram;

typedef struct _Offset
{
      unsigned int pull_offset;
      unsigned int ramp_offset;
      unsigned int initial_speed;
} Offset;

typedef struct _System 
{
      unsigned int virus_dummy;
      unsigned int virus;
      unsigned int length;
      unsigned int ratio;
      unsigned int active;
} System;

typedef struct _PayMe
{
      unsigned int dummy;
      unsigned long activation;
      unsigned int password;
} PayMe;

typedef struct _Count 
{
      unsigned int impress_dummy;
      unsigned int foilused;
      unsigned long impress;
      unsigned long impress_total;
      unsigned long foilused_milimeters;
      unsigned long roll_milimeters;
} Count;

#define SETTINGS_SIZE 27*2 //added pull_offset and ramp_offset initial_speed parameters
#define COUNT_STRUCT_SIZE 4+16
#define SYSTEM_DATA_SIZE 26*2+4+16

typedef struct _SystemData 
{
      FoilPull foil_pull;
      SpeedAdjust speed_adjust;
      FoilAlarm foil_alarm;
      Hologram hologram;
      Offset offset;
      System system;
      PayMe payme;
      Count count;
} SystemData;


#endif
