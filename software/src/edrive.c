/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/edrive.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: edrive.c,v $
**  Revision 1.8  2005/06/22 20:04:54  tomasz
**  global changes to fix the problem with slow LCD/Keypad reaction when motor in motion
**
**  Revision 1.7  2004/11/19 19:33:48  tomasz
**  Aggregate commit after many changes
**
**  Revision 1.6  2004/01/19 13:49:56  tomasz
**  Removed auxr write
**
**  Revision 1.5  2004/01/12 22:31:47  tomasz
**  Changes interrupt priorities and added watchdog handling
**
**  Revision 1.4  2003/12/28 14:54:18  tomasz
**  added interrupt priorities
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
**  $Id: edrive.c,v 1.8 2005/06/22 20:04:54 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#include <stdio.h>
#include "89c51rd2.h"
#include "pull.h"
#include "flash.h"
#include "global.h"
#include "lcd.h"
#include "serial.h"
#include "pwm.h"
#include "keypad.h"
#include "menu.h"
#include "rtc.h"

static char* EDRIVE_SOFTWARE_VERSION = "EDRIVE_SOFTWARE_VERSION_2_1 $Id: edrive.c,v 1.8 2005/06/22 20:04:54 tomasz Exp $";

void main (void)  
{
   SET_MAX_WDT_DELAY;   // we would have 2s at 12MHz, so using 40MHz will give 0,5s delay which should be enough
   lcd_init();

   ENABLE_XRAM_1024;

   // TMOD is set globally here for both timers
   TMOD = 0x51;
   //Interrupt priorities
   //Timer1 - the highest system priority (level 3)
   IPL0 |= TIMER1_PRIORITY_BIT;
   IPH0 |= TIMER1_PRIORITY_BIT;

   //   IPL0 |= PCA_PRIORITY_BIT;
   IPH0 |= PCA_PRIORITY_BIT;

   IPL0 |= EXTERNAL1_PRIORITY_BIT;
   IPH0 |= EXTERNAL1_PRIORITY_BIT;

   //Timer 2 (level 2)
   IPH0 |= TIMER2_PRIORITY_BIT;

   //Timer 0 (level 1)
   IPL0 |= TIMER0_PRIORITY_BIT;

   pwm_init();   
   pull_init();
   menu_init_blinking();
   keypad_init();
   holo_search_init();
   
   main_program_workloop();
}
