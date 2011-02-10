/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/keypad.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: keypad.c,v $
**  Revision 1.6  2005/06/22 20:04:54  tomasz
**  global changes to fix the problem with slow LCD/Keypad reaction when motor in motion
**
**  Revision 1.5  2004/01/12 22:33:10  tomasz
**  Added buzzer alarm handling
**
**  Revision 1.4  2003/12/28 14:53:08  tomasz
**  Timer0 int added 'using 0' qualifier
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
**  $Id: keypad.c,v 1.6 2005/06/22 20:04:54 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#include <stdio.h>
#include "89c51rd2.h"
#include "keypad.h"
#include "global.h"
#include "lcd.h"
#include "menu.h"
#include "alarm.h"
#include "pull.h"

extern SystemData xdata system_data;
extern volatile Blink_params * current_blink_params;
extern volatile BOOL blinking;
extern volatile AlarmDefs *system_alarm;
extern volatile unsigned char pull_sensor_state;

volatile static struct _Key xdata kb;
volatile unsigned char blink_counter = 0;
volatile unsigned int delay_counter = 0;

typedef struct _MappingTable {
      unsigned int scan;
      unsigned int normalized;
      char *description;
};

const struct _MappingTable code mappingTable[] = {
   { 0x37, 0, "0" },
   { 0x4b, 1, "1" },
   { 0x3b, 2, "2" },
   { 0x2b, 3, "3" },
   { 0x4d, 4, "4" },
   { 0x3d, 5, "5" },
   { 0x2d, 6, "6" },
   { 0x4e, 7, "7" },
   { 0x3e, 8, "8" },
   { 0x2e, 9, "9" },
   { 0x1d, LEFT, "LEFT" },
   { 0x1e, RIGHT, "RIGHT" },
   { 0xd,  HOLO, "HOLO SEARCH" },
   { 0xb,  TEST, "TEST PULL" },
   { 0x47, CLEAR, "CLEAR" },
   { 0x17, INCH, "INCH" },
   { 0xe,  STOP, "STOP" },
   { 0x1b, RESET, "RESET" },
   { 0x27, ENTER, "ENTER" },
   { 0 , 0 }
};

unsigned char find_mapping_code(const unsigned char scan_code)
{
   unsigned char result = 0;
   unsigned char stop = 0;
   unsigned char i;

   for (i=0; i<255, stop==0; i++)
   {
      if ( mappingTable[i].scan == scan_code )
      {
         result = mappingTable[i].normalized;
         stop = 1;
      }

      if ( (mappingTable[i].scan == 0) && 
           (mappingTable[i].normalized == 0) )
      {
         stop = 1;
         result = SCAN_ERROR;
      }
   }

   return result;
}

/**
 * Brief description.
 * We are using T0 as matrix keyboard decoder
 */

void keypad_init (void)
{
   kb.last_code = BUFFER_EMPTY;
   kb.status = BUFFER_EMPTY;

   //   TMOD &= 0xF1;
   ENABLE_T0_INTERRUPT;
   KEYPAD_START;
}

unsigned char keypad_pop_front(void)
{
   return kb.buffer;
}

unsigned char keypad_check_front(void)
{
   return kb.status;
}

void keypad_purge_buffer(void)
{
   kb.status = BUFFER_EMPTY;
}

unsigned char keypad_get_last_code(void)
{
   return kb.last_code;
}

// void reset_delay_counter()
// {
//    delay_counter = 0;
// }

// unsigned int get_delay_counter()
// {
//    return delay_counter;
// }

//
// Will give 100 ms for pull sensor to estabilish 
//
#define WAIT_100MS_DELAY 100

void check_pull_sensor(void) 
{
   switch(pull_sensor_state) 
   {
      case PULL_SENSOR_ACTIVATED:
         if (delay_counter >= WAIT_100MS_DELAY) 
         {
            pull_sensor_state = PULL_SENSOR_WAIT_NOISE_READY;
         }
         break;

      case PULL_SENSOR_WAIT_NOISE_READY:
         if (PULL_INPUT_PIN == 1)
         {
            if (system_data.foil_pull.delay > 0)
            {
               delay_counter = 0;
               pull_sensor_state = PULL_SENSOR_START_DELAYED;
            }
            else 
            {
               pull_sensor_state = PULL_SENSOR_INITIAL;
               start_pull();
            }
         }
         break;

      case PULL_SENSOR_START_DELAYED:
         if (delay_counter >= system_data.foil_pull.delay)
         {
            pull_sensor_state = PULL_SENSOR_INITIAL;
            start_pull();
         }
         break;
   }
}

/*
 * Brief description.
 * Assuming we are using X2 mode interrupt frequecny f=40MHz/12/2 = 6,66MHz
 * Timer expiration time in Mode 1 Exp = 1/6,66 * 65535 = 9,8 ms
 */

void timer0_interrupt(void) interrupt TF0_VECTOR using 0
{ 
   static unsigned char counter = 0;
   unsigned char input_value;
   unsigned char key_code;
   unsigned char empty;

   // check pull procedure
   check_pull_sensor();

   /* Let's assume we run interrupt at 20ms */
   delay_counter+=20;

   if (system_alarm->type != NO_ALARM)
   {
      blink_counter++;
      if (blink_counter == 30)
      {
         BUZZER_ON;
      }
      if (blink_counter == 60)
      {
         BUZZER_OFF;
         blink_counter = 0;
      }
   }
   else
   {
      BUZZER_OFF;

      if (blinking == TRUE)
      {
         blink_counter++;
         if (blink_counter == 30)
         {
            lcd_goto_xy(current_blink_params->position, 0);
            lcd_put_string(current_blink_params->first_pattern_off);
            
            if (current_blink_params->lines > 1)
            {
               lcd_goto_xy(current_blink_params->position, 2);
               lcd_put_string(current_blink_params->second_pattern_off);
            }
         }
         if (blink_counter == 60)
         {
            lcd_goto_xy(current_blink_params->position, 0);
            lcd_put_string(current_blink_params->first_pattern_on);

            if (current_blink_params->lines > 1)
            {
               lcd_goto_xy(current_blink_params->position, 1);
               lcd_put_string(current_blink_params->second_pattern_on);
            }
            
            blink_counter = 0;
         }
      }
   }

   if (counter++ >= 5)
   {
      counter = 0;
      empty = 0;

      KSC0 = 0;
      input_value = (KEYBOARD_PORT >> 1) & 0xF;
      if ( input_value != 0x0F ) 
      {
         empty++;
         
         key_code = find_mapping_code(input_value);
         
         if (key_code != SCAN_ERROR)
         {
            if (key_code != kb.last_code)
            {
               kb.buffer = key_code;
               kb.last_code = key_code;
               kb.status = DATA_AVAILABLE;
               if (system_alarm->type == NO_ALARM) BUZZER_ON;
            }
         }
      }
      KSC0 = 1;
      
      KSC1 = 0;
      input_value = (KEYBOARD_PORT >> 1) & 0xF;
      if ( input_value != 0x0F ) 
      {
         empty++;
         input_value |= 0x10;
         key_code = find_mapping_code(input_value);
         
         if (key_code != SCAN_ERROR)
         {
            if (key_code != kb.last_code)
            {
               kb.buffer = key_code;
               kb.last_code = key_code;
               kb.status = DATA_AVAILABLE;
               if (system_alarm->type == NO_ALARM) BUZZER_ON;
            }
         }
      }
      KSC1 = 1;
      
      KSC2 = 0;
      input_value = (KEYBOARD_PORT >> 1) & 0xF;
      if ( input_value != 0x0F ) 
      {
         empty++;
         
         input_value |= 0x20;
         key_code = find_mapping_code(input_value);
         
         if (key_code != SCAN_ERROR)
         {
            if (key_code != kb.last_code)
            {
               kb.buffer = key_code;
               kb.last_code = key_code;
               kb.status = DATA_AVAILABLE;
               if (system_alarm->type == NO_ALARM) BUZZER_ON;
            }
         }
      }
      KSC2 = 1;
      
      KSC3 = 0;
      input_value = (KEYBOARD_PORT >> 1) & 0xF;
      if ( input_value != 0x0F ) 
      {
         empty++;
         
         input_value |= 0x30;
         key_code = find_mapping_code(input_value);
         
         if (key_code != SCAN_ERROR)
         {
            if (key_code != kb.last_code)
            {
               kb.buffer = key_code;
               kb.last_code = key_code;
               kb.status = DATA_AVAILABLE;
               if (system_alarm->type == NO_ALARM) BUZZER_ON;
            }
         }
      }
      KSC3 = 1;
      
      KSC4 = 0;
      input_value = (KEYBOARD_PORT >> 1) & 0xF;
      if ( input_value != 0x0F ) 
      {
         empty++;
         
         input_value |= 0x40;
         key_code = find_mapping_code(input_value);
         
         if (key_code != SCAN_ERROR)
         {
            if (key_code != kb.last_code)
            {
               kb.buffer = key_code;
               kb.last_code = key_code;
               kb.status = DATA_AVAILABLE;
               if (system_alarm->type == NO_ALARM) BUZZER_ON;
            }
         }
      }
      KSC4 = 1;
      
      if (empty == 0)
      {
         kb.last_code = BUFFER_EMPTY;
      }
   }
}
