/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/pull.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: pull.c,v $
**  Revision 1.10  2007/01/12 15:38:26  tomasz
**  modifications to make sdcc 2.6.0 compiler happy about warnings. Doesn't solve new compiler problem
**
**  Revision 1.9  2006/07/03 23:32:23  tomasz
**  fixed displaying total count error, fixed setting pull length with hologram offset, fixed enabling/disablin pull input when holo alarm goes off
**
**  Revision 1.8  2005/06/22 20:04:55  tomasz
**  global changes to fix the problem with slow LCD/Keypad reaction when motor in motion
**
**  Revision 1.7  2004/11/19 19:33:57  tomasz
**  Aggregate commit after many changes
**
**  Revision 1.6  2004/08/05 12:40:02  tomasz
**  Changed for 3M and 3,3Kohm
**
**  Revision 1.5  2004/01/28 09:51:15  tomasz
**  Added RTC data backup, small changes due to B&H visit
**
**  Revision 1.4  2004/01/19 13:48:16  tomasz
**  changed speed settings for newly assembled board
**
**  Revision 1.3  2004/01/12 22:30:30  tomasz
**  Added holograms handling routines
**
**  Revision 1.2  2003/12/28 14:52:01  tomasz
**  Fixed problem with pull accuracy and acceleration
**
**  Revision 1.1  2003/12/22 10:54:57  tomasz
**  first release
**
**
**  $Id: pull.c,v 1.10 2007/01/12 15:38:26 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#include <stdio.h>
#include "89c51rd2.h"
#include "global.h"
#include "pwm.h"
#include "keypad.h"
#include "lcd.h"
#include "menu.h"
#include "alarm.h"
#include "flash.h"
#include "pull.h"
#include "rtc.h"

#define COUNT_SIZE 65536
#define STEPPER_RATIO_4_TO_1       10000
#define STEPPER_RATIO_3_TO_1       7500
#define MAX_FOIL_USED_LEN          9999
#define MAX_IMPRESSIONS            9999999
#define MAX_IMPRESSIONS_TOTAL      9999999

//#define MAGIC_NUMBER_1 3201
//#define MAGIC_NUMBER_2 9999

typedef enum _holo_states {
   HOLO_SEARCH_PRESSED,
   HOLO_INITIAL,
   HOLO_SEARCHING,
   HOLO_SENSOR_FOUND,
   HOLO_RUN_OFFSET
} Holo_States;

unsigned char xdata current_steps = 0;
volatile unsigned char accelerate = TRUE;
volatile unsigned int steps = 0;
volatile unsigned int change_point = 0;
volatile unsigned int pull_len = 0;
volatile unsigned char counter = 0;
volatile unsigned char ramp = 0;
volatile unsigned char speed = 0;
volatile BOOL change_now = FALSE;
volatile unsigned char hologram_states = HOLO_INITIAL;
volatile unsigned int pca_steps_counter = 0;
volatile unsigned char pull_sensor_state = PULL_SENSOR_INITIAL;
volatile unsigned char pull_offset_added = FALSE;

extern SystemData xdata system_data;
extern code SystemData flash_data;
extern volatile unsigned int delay_counter;
extern volatile AlarmDefs *system_alarm;
extern unsigned char xdata update_flash;
unsigned char xdata impress_update_counter = 0;

const unsigned char code pull_speed_table[] = { 255, 196, 192, 188, 184, 180, 176, 172, 168, 164, 160 };
const unsigned char code ramp_table[] =  { 60, 57, 54, 51, 48, 45, 42, 39, 36, 33, 30, 27, 24, 21, 18, 15, 12, 9, 6, 3, 0 };
const unsigned char code inch_speed_table[] = { 210, 205, 200, 195, 190, 185, 180, 175, 170, 165, 160 };
const unsigned char code holo_speed_table[] =  { 80, 60, 40, 20, 10, 8, 5, 3, 2, 1, 0 };

void start_pull(void)
{
   if (IS_STEPPER_DISABLED)
   {
      if (system_data.foil_pull.steps > 0 )
      {
         if (current_steps < system_data.foil_pull.steps) 
         {
            current_steps++;
            pull_len = system_data.foil_pull.pull;
            speed = pull_speed_table[system_data.speed_adjust.pull] - (unsigned char)system_data.offset.pull_offset;
         }
         else
         {
            current_steps = 0;
            pull_len = system_data.foil_pull.clear;
            speed = pull_speed_table[system_data.speed_adjust.clear] - (unsigned char)system_data.offset.pull_offset;
         }
      }
      else
      {
         current_steps = 0;
         pull_len = system_data.foil_pull.pull;
         speed = pull_speed_table[system_data.speed_adjust.pull] - (unsigned char)system_data.offset.pull_offset;
      }

      /**************************************************************************************************/
      /* We need to substract hologram offset from newly set pull length just not to miss the holo mark */
      if (system_data.hologram.active != 0)
      {
         if (pull_offset_added == TRUE)
            pull_len -= system_data.hologram.offset;
      }
      pull_offset_added = FALSE;

      /**************************************************************/
      /* Here we need to add calculated foil lenght to foil counter */
      if (system_data.count.foilused >= MAX_FOIL_USED_LEN)
      {
         system_data.count.foilused = 0;
         system_data.count.foilused_milimeters = 0;
      }
      if (system_data.count.impress >= MAX_IMPRESSIONS)
      {
         system_data.count.impress = 0;
      }
      if (system_data.count.impress_total >= MAX_IMPRESSIONS_TOTAL)
      {
         system_data.count.impress_total = 0;
      }

      system_data.count.foilused_milimeters += pull_len;
      system_data.count.foilused = system_data.count.foilused_milimeters / MILIMETERS_PER_METER;

      /**************************************************************************************/
      /* Another point is to check if we should trigger foil alarm using count->roll        */
      system_data.count.roll_milimeters += pull_len;
      system_data.foil_alarm.roll_counter = system_data.count.roll_milimeters / MILIMETERS_PER_METER;
      
      if (system_alarm->type == NO_ALARM)
      {
         if (system_data.foil_alarm.enable != 0)
         {
            if (system_data.foil_alarm.roll_counter >= (system_data.foil_alarm.roll - system_data.foil_alarm.enable))
            {
               system_data.count.roll_milimeters = 0;
               system_data.foil_alarm.roll_counter = 0;
               alarm_on(FOIL_ROLL_ALARM);
            }
         }
      }

      /*******************************************/
      /* Just make sure new values are displayed */
      refresh_count_display();
      
      /**************************************************************************************/
      /* Need to check if we have ratio 4:1 or 3:1. If 3:1 we need to update pull by 3/4    */
      if (system_data.system.ratio != 0)
      {
         pull_len = pull_len + (pull_len << 1);  /* pull_len*3 */
         pull_len >>= 2;                         /* pull_len/4 */
      }

      ds1302_write_memory(RTC_ALARM_ROLL_COUNT_LSB,(unsigned char)system_data.foil_alarm.roll_counter&0xff);
      ds1302_write_memory(RTC_ALARM_ROLL_COUNT_MSB,(unsigned char)(system_data.foil_alarm.roll_counter>>8)&0xff);
      ds1302_write_memory(RTC_COUNT_FOILUSED_LSB,(unsigned char)system_data.count.foilused&0xff);
      ds1302_write_memory(RTC_COUNT_FOILUSED_MSB,(unsigned char)(system_data.count.foilused>>8)&0xff);
      ds1302_write_memory(RTC_IMPRESS_LSB,(unsigned char)system_data.count.impress&0xff);
      ds1302_write_memory(RTC_IMPRESS_LSB_1,(unsigned char)(system_data.count.impress>>8)&0xff);
      ds1302_write_memory(RTC_IMPRESS_LSB_2,(unsigned char)(system_data.count.impress>>16)&0xff);
      ds1302_write_memory(RTC_IMPRESS_MSB,(unsigned char)(system_data.count.impress>>24)&0xff);
      ds1302_write_memory(RTC_IMPRESS_TOTAL_LSB,(unsigned char)system_data.count.impress_total&0xff);
      ds1302_write_memory(RTC_IMPRESS_TOTAL_LSB_1,(unsigned char)(system_data.count.impress_total>>8)&0xff);
      ds1302_write_memory(RTC_IMPRESS_TOTAL_LSB_2,(unsigned char)(system_data.count.impress_total>>16)&0xff);
      ds1302_write_memory(RTC_IMPRESS_TOTAL_MSB,(unsigned char)(system_data.count.impress_total>>24)&0xff);
      // Here we store our OK signal within battery backed memory
      ds1302_write_memory(RTC_TEST_BYTE,0x55);

      /***********************************************/
      /* Flash must be updated with new values       */
      if (update_flash == true)
      {
         DISABLE_INTERRUPTS;
         flash_write_page(SYSTEM_DATA_SIZE,(unsigned int)&system_data,(unsigned int)FLASH_START_POINT);
         update_flash = false;
         ENABLE_INTERRUPTS;
      }
      else
      {
         if (impress_update_counter++ >= 0x99)
         {
            DISABLE_INTERRUPTS;
            flash_write_page(COUNT_STRUCT_SIZE,(unsigned int)&system_data.count,(unsigned int)FLASH_START_POINT+SETTINGS_SIZE);
            impress_update_counter = 0;
            ENABLE_INTERRUPTS;
            ds1302_write_memory(RTC_TEST_BYTE,0x0);
         }
      }

      change_point = COUNT_SIZE - (pull_len>>1);
      pull_len = COUNT_SIZE - pull_len;
     
      TL1 = (unsigned char)pull_len&0xff;
      TH1 = (unsigned char)(pull_len >> 8)&0xff;
      
      counter=0;

      ramp = ramp_table[system_data.speed_adjust.ramp] + (unsigned char)system_data.offset.ramp_offset;

      accelerate = TRUE;
      change_now = FALSE;
      //      CCAP0H = PWM_INITIAL_SPEED;
      CCAP0H = (unsigned char)system_data.offset.initial_speed;

      // make sure holo sensor is disabled as well as PCA interrupt before pull is finished
      DISABLE_HOLO_INTERRUPT;
      DISABLE_PCA_INTERRUPT;
      
      ENABLE_T1_INTERRUPT;
      TIMER_1_RUN;
      TIMER_2_RUN;
      ENABLE_STEPPER;
   }
}

void pull_init(void) 
{
   ENABLE_T2_INTERRUPT;
   RCAP2L = 0xf4;
   RCAP2H = 0xfb;
}

void inch_mode()
{
   if (IS_STEPPER_DISABLED)
   {
      if (IS_PCA_DISABLED)
      {
         CCAP0H = inch_speed_table[system_data.speed_adjust.inch];
         ENABLE_STEPPER;
         while ( keypad_get_last_code() == INCH || INCH_BUTTON_PRESSED ) 
         {
            SERVICE_WATCHDOG;
         }
         DISABLE_STEPPER;
         CCAP0H = (unsigned char)system_data.offset.initial_speed;
         //         CCAP0H = PWM_INITIAL_SPEED;
      }
   }
}

void holo_search_init(void)
{
   CCAP1L = 0x10;
   CCAP1H = 0x27;
   CCAPM1 = CCAPMn_PCA_ENABLE_COMPARATOR | CCAPMn_PCA_MATCH | CCAPMn_PCA_INTERRUPT_ENABLE;
}

void holo_search_start(void)
{
   if (system_data.hologram.active != 0)
   {
      hologram_states = HOLO_SEARCH_PRESSED;
      pca_steps_counter = 0;
      HOLO_NEGATIVE_TRIGGER;
      ENABLE_HOLO_INTERRUPT;
      ENABLE_PCA_INTERRUPT;
   }
}

void set_holo_len(unsigned int len)
{
   pull_len = len;
   if (system_data.system.ratio != 0)
   {
      pull_len = pull_len + (pull_len << 1);  /* pull_len*3 */
      pull_len >>= 2;                         /* pull_len/4 */
   }
   pull_len <<= 1;
}

void pca_timer_interrupt(void) interrupt 6 using 0
{
   static unsigned char pca_counter = 0;

   if (pca_counter++ == holo_speed_table[system_data.hologram.speed])
   {
      pca_steps_counter++;
      if (HOLO_SPEED_PORT == 0)
      {
         HOLO_SPEED_PORT = 1;
      }
      else
      {
         HOLO_SPEED_PORT = 0;
      }
      pca_counter = 0;

      switch(hologram_states)
      {
         // Holo search goes forever until STOP button is pressed
         case HOLO_SEARCH_PRESSED:
            break;

         case HOLO_INITIAL:
            hologram_states = HOLO_SEARCHING;
            set_holo_len(system_data.hologram.overrun);
            break;
            
         case HOLO_SEARCHING:
            if (pca_steps_counter >= pull_len)
            {
               hologram_states = HOLO_INITIAL;
               DISABLE_PCA_INTERRUPT;
               DISABLE_HOLO_INTERRUPT;
               //               ENABLE_PULL_INTERRUPT;
               //we need to disable pull input until the alarm is cleared.
               DISABLE_PULL_INTERRUPT;
               HOLO_SPEED_PORT = 0;
               alarm_on(HOLO_SEARCH_ALARM);
            }
            break;
            
         case HOLO_RUN_OFFSET:
            if (pca_steps_counter >= pull_len)
            {
               hologram_states = HOLO_INITIAL;
               pull_offset_added = TRUE;
               DISABLE_PCA_INTERRUPT;
               DISABLE_HOLO_INTERRUPT;
               ENABLE_PULL_INTERRUPT;
               HOLO_SPEED_PORT = 0;
            }
            break;         
      }
   }
      
   CL = 0;
   CH = 0;
   CCF1 = 0;
}

void timer2_interrupt(void) interrupt 5 using 0
{
   steps = (unsigned int)TH1;
   steps = (unsigned int)((steps << 8) | TL1);

   if (steps > change_point)
   {
      if (change_now == FALSE) 
      {
         counter = ramp - counter;
         change_now = TRUE;
      }

      //      if (CCAP0H <= PWM_INITIAL_SPEED)
      if (CCAP0H <= (unsigned char)system_data.offset.initial_speed);
      {
         if (counter++ >= ramp)
         {
            counter = 0;
            CCAP0H++;
         }
      }
   }

   if (accelerate == TRUE && steps <= change_point)
   {
      if (CCAP0H > speed) 
      {
         if (counter++ >= ramp)
         {
            counter = 0;
            CCAP0H--;
         }
      }
      else 
      {
         change_point = COUNT_SIZE - steps + pull_len;
         accelerate = FALSE;
      }
   }

   TF2 = 0;
}

//
// This timer counts incomming motor pulses so in fact it counts number of steps done
//
void timer1_interrupt(void) interrupt 3 using 0
{
   DISABLE_STEPPER;
   TIMER_1_STOP;
   TIMER_2_STOP;

   if (system_data.hologram.active != 0)
   {
      hologram_states = HOLO_INITIAL;
      pca_steps_counter = 0;
      //      HOLO_NEGATIVE_TRIGGER;
      HOLO_LOW_LEVEL_TRIGGER;
      ENABLE_HOLO_INTERRUPT;
      ENABLE_PCA_INTERRUPT;
   }
   else 
   {
      ENABLE_PULL_INTERRUPT;
   }
}

#define SENSOR_READ_DELAY 5000

void int0_pull_interrupt(void) interrupt 0 using 0
{
   //   unsigned int counter;
   DISABLE_PULL_INTERRUPT;
   system_data.count.impress++;
   system_data.count.impress_total++;
   //
   // We need to mark that sensor has been activated ('0' at pull input pin)
   //
   pull_sensor_state = PULL_SENSOR_ACTIVATED; 
   delay_counter = 0;
   //
   // Do some delay first to avoid reading sensor noise
   //
//    for ( counter = 0; counter < SENSOR_READ_DELAY; counter++ )
//    {
//       SERVICE_WATCHDOG;
//    }
   //
   // We need to start pulling when pull input is released.
   // Do some busy waiting inside this loop and wait for pull sensor to release.
   //
//    while (PULL_INPUT_PIN == 0)
//    {
//       SERVICE_WATCHDOG;
//    }

//    if (system_data.foil_pull.delay > 0)
//    {
//       delay_counter = 0;
//       while (delay_counter < system_data.foil_pull.delay)
//       {
//          SERVICE_WATCHDOG;
//       }
//    }
   //start_pull();
}

void int1_holo_interrupt(void) interrupt 2 using 0
{
   DISABLE_HOLO_INTERRUPT;
   hologram_states = HOLO_RUN_OFFSET;
   set_holo_len(system_data.hologram.offset);
   system_data.count.foilused_milimeters += pull_len >> 1;
   system_data.count.roll_milimeters += pull_len >> 1;
   pca_steps_counter = 0;
}






