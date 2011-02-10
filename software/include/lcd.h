/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/lcd.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: lcd.h,v $
**  Revision 1.2  2003/11/14 18:10:27  tomasz
**  ported to compile with sdcc c51 compiler
**
**  Revision 1.1  2003/11/08 12:00:19  tomasz
**  first release
**
**  $Id: lcd.h,v 1.2 2003/11/14 18:10:27 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/
#ifndef __LCD_H__
#define __LCD_H__

#include "global.h"

void lcd_put ( char value, BOOL command ) reentrant;
void lcd_goto_xy (const char x, const char y) reentrant;
void lcd_clear ( void );
void lcd_home ( void );
void lcd_init ( void );
void lcd_cursor_on ( void );
void lcd_cursor_off ( void );
void lcd_put_string(const char *value);

#endif
