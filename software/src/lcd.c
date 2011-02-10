/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/lcd.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: lcd.c,v $
**  Revision 1.5  2005/06/22 20:04:55  tomasz
**  global changes to fix the problem with slow LCD/Keypad reaction when motor in motion
**
**  $Id: lcd.c,v 1.5 2005/06/22 20:04:55 tomasz Exp $
**************************************************************************/

#include "89c51rd2.h"
#include <stdio.h>
#include <string.h>
#include "lcd.h"

#define LCD_CTRL_PORT       P2
#define LCD_BUSY_BIT        P2_3
#define LCD_E_BIT           P2_4
#define LCD_RW_BIT          P2_5
#define LCD_RS_BIT          P2_6
#define LCD_RS_BIT_MASK     0x40 //P2_6 = 0x40

#define LCD_FUNCTION_SET    0x2 //Turns 4-bit mode

//Display on/off control
#define LCD_BLINK_ON        0x1
#define LCD_CURSOR_ON       0x2
#define LCD_DISPLAY_ON      0x4
#define LCD_DISPLAY_OFF     0x0
#define LCD_CONTROL_ON      0x8

// From http://www.myke.com/lcd.htm
// In describing how the LCD should be initialized in four bit mode, I will specify writing to the LCD in terms of nybbles. 
// This is because initially, just single nybbles are sent (and not two, which make up a byte and a full instruction). 
// As I mentioned above, when a byte is sent, the high nybble is sent before the low nybble and the "E" pin is toggled each 
// time four bits is sent to the LCD. To initialize in four bit mode:

//   1. Wait more than 15 msecs after power is applied.
//   2. Write 0x03 to LCD and wait 5 msecs for the instruction to complete
//   3. Write 0x03 to LCD and wait 160 usecs for instruction to complete
//   4. Write 0x03 AGAIN to LCD and wait 160 usecs (or poll the Busy Flag)
//   5. Set the Operating Characteristics of the LCD
//          * Write 0x02 to the LCD to Enable Four Bit Mode

#define LCD_INIT_FOR_4_BITS 0x3
#define LCD_INIT_FOR_8_BITS 0x30

//Clear display
#define LCD_CLEAR           0x1

//Go home
#define LCD_HOME            0x2

#ifdef X2_MODE
#define ENABLE_DELAY        300
#define ENABLE_DELAY_LONG   3000
#else
#define ENABLE_DELAY        150
#define ENABLE_DELAY_LONG   1500
#endif

#define lcd_put_command( command ) lcd_put( command, TRUE )
#define lcd_put_char( character ) lcd_put( character, FALSE )

void lcd_delay ( int delay ) 
{
    int counter = 0;

    for ( ; counter < delay; counter++ )
    {
       SERVICE_WATCHDOG;
    }
}

void enable(void) 
{
   LCD_E_BIT = 1;
   lcd_delay(ENABLE_DELAY);
   LCD_E_BIT = 0;
   lcd_delay(ENABLE_DELAY);
}

void lcd_goto_xy (const char x, const char y) reentrant
{
   if (y == 0)
   {
      lcd_put_command(x+0x80);
   }
   else
   {
      lcd_put_command(x+0x80+0x40);
   }
}

void lcd_put ( char value, BOOL command ) reentrant
{
   if ( command ) 
   {
      LCD_CTRL_PORT = (( value >> 4 ) & 0x0f ) | LCD_CTRL_PORT&0x80;
      enable();
      LCD_CTRL_PORT = ( value & 0x0f ) | LCD_CTRL_PORT&0x80;
      enable();
   } 
   else 
   {
      LCD_CTRL_PORT = ( ( value >> 4 ) & 0x0f ) | LCD_RS_BIT_MASK | LCD_CTRL_PORT&0x80;
      enable();
      LCD_CTRL_PORT = ( value & 0x0f ) | LCD_RS_BIT_MASK | LCD_CTRL_PORT&0x80;
      enable();
      LCD_RS_BIT = 0;
   }
}

/* lcd_clear
 *
 * Purpose:     - Clears display and returns cursor to home position (upper-left).
 *
 */

void lcd_clear ( void ) 
{
   lcd_put_command( LCD_CLEAR );
}

/* lcd_home
 *
 * Purpose:     - Returns cursor to home position.
 *              - Returns display to original position (when shifted).
 *
 */

void lcd_home ( void ) 
{
   lcd_put_command( LCD_HOME );
}

void putchar(char c) 
{
   lcd_put_char(c);
}

void lcd_put_string(const char *value) reentrant
{
   unsigned char i;

   for(i=0; i<strlen(value); i++)
   {
      SERVICE_WATCHDOG;
      lcd_put_char(value[i]);
   }
}

void lcd_cursor_on(void)
{
   lcd_put_command( LCD_CONTROL_ON | LCD_DISPLAY_ON | LCD_CURSOR_ON | LCD_BLINK_ON );
}

void lcd_cursor_off(void)
{
   lcd_put_command( LCD_CONTROL_ON | LCD_DISPLAY_ON );
}


/**
 * Brief description. Full description.
 *   1. Wait more than 15 msecs after power is applied.
 *   2. Write 0x03 to LCD and wait 5 msecs for the instruction to complete
 *   3. Write 0x03 to LCD and wait 160 usecs for instruction to complete
 *   4. Write 0x03 AGAIN to LCD and wait 160 usecs (or poll the Busy Flag)
 *   5. Set the Operating Characteristics of the LCD
 *           Write 0x02 to the LCD to Enable Four Bit Mode
 *
 * @see http://www.myke.com/lcd.htm
 */
void lcd_init ( void ) 
{
   lcd_delay(ENABLE_DELAY_LONG);
   LCD_CTRL_PORT = LCD_INIT_FOR_4_BITS;
   enable();

   lcd_delay(ENABLE_DELAY_LONG);
   LCD_CTRL_PORT = LCD_INIT_FOR_4_BITS;
   enable();

   lcd_delay(ENABLE_DELAY_LONG);
   LCD_CTRL_PORT = LCD_INIT_FOR_4_BITS;
   enable();

   lcd_delay(ENABLE_DELAY_LONG);
   LCD_CTRL_PORT = LCD_FUNCTION_SET | LCD_CTRL_PORT&0x80; 
   enable();

   lcd_delay(ENABLE_DELAY_LONG);

   lcd_put_command( LCD_CONTROL_ON | LCD_DISPLAY_ON );
   //   lcd_delay(ENABLE_DELAY_LONG);
   lcd_clear();
   //lcd_delay(ENABLE_DELAY_LONG);
   lcd_home();
   //lcd_delay(ENABLE_DELAY_LONG);
   lcd_cursor_off();

   //lcd_delay(ENABLE_DELAY_LONG);
}
