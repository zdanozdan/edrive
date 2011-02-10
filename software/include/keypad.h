/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/keypad.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: keypad.h,v $
**  Revision 1.4  2004/01/12 22:33:42  tomasz
**  Added delay functions
**
**  Revision 1.3  2003/12/28 14:57:21  tomasz
**  'using 0' added to timer0 interrupt
**
**  Revision 1.2  2003/11/14 18:10:27  tomasz
**  ported to compile with sdcc c51 compiler
**
**  Revision 1.1  2003/11/08 12:00:19  tomasz
**  first release
**
**  $Id: keypad.h,v 1.4 2004/01/12 22:33:42 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#ifndef	__KEYPAD_H__
#define	__KEYPAD_H__

void timer0_interrupt(void) interrupt TF0_VECTOR using 0;

#define KEYBOARD_BUFFER_SIZE      1
#define KEYBOARD_PORT             P0
#define KEYBOARD_AUX_PIN          P3_7

#define KSC0                      P0_0
#define KSC1                      P0_5
#define KSC2                      P0_6
#define KSC3                      P0_7
#define KSC4                      KEYBOARD_AUX_PIN

#define KRL0                      P0_1
#define KRL1                      P0_2
#define KRL2                      P0_3
#define KRL3                      P0_4


#define KEYPAD_START              TR0 = 1;
#define KEYPAD_STOP               TR0 = 0;

typedef struct _Key {
      unsigned char last_code;
      unsigned char status;
      unsigned char buffer; 
};

//Enum for buffer states definition
typedef enum {
   BUFFER_EMPTY = 0x90,
   DATA_AVAILABLE = 0x91,
   SCAN_ERROR = 0x92
} Buffer_status;

typedef enum _KeyCodes {
   LEFT = 0x10,
   RIGHT = 0x11,
   HOLO = 0x12,
   TEST = 0x13,
   CLEAR = 0x14,
   INCH = 0x15,
   STOP = 0x16,
   RESET = 0x17,
   ENTER = 0x18
} KeyCodes;

void keypad_init(void);
unsigned char keypad_pop_front(void);
void keypad_purge_buffer(void);
unsigned char keypad_check_front(void);
unsigned char keypad_get_last_code(void);
void reset_delay_counter(void);
unsigned int get_delay_counter(void);

#endif /* __KEYPAD_H__ */










