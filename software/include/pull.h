/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/pull.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: pull.h,v $
**  Revision 1.5  2005/06/22 20:04:54  tomasz
**  global changes to fix the problem with slow LCD/Keypad reaction when motor in motion
**
**  Revision 1.4  2004/01/28 09:53:09  tomasz
**  Pull on sensor release added
**
**  Revision 1.3  2004/01/12 22:30:49  tomasz
**  Added holograms handling routines
**
**  Revision 1.2  2003/12/28 14:56:25  tomasz
**  new interrrupts definitions added
**
**  Revision 1.1  2003/12/22 10:53:17  tomasz
**  first release
**
**
**  $Id: pull.h,v 1.5 2005/06/22 20:04:54 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#ifndef __PULL_H__
#define __PULL_H__

#define PULL_INPUT_PIN P3_2
#define MILIMETERS_PER_METER 10000

typedef enum _pull_states {
   PULL_SENSOR_INITIAL,
   PULL_SENSOR_ACTIVATED,
   PULL_SENSOR_WAIT_NOISE_READY,
   PULL_SENSOR_START_DELAYED
} Pull_States;

void timer2_interrupt(void) interrupt 5 using 0;
void pca_timer_interrupt(void) interrupt 6 using 0;
void int0_pull_interrupt(void) interrupt 0 using 0;
void timer1_interrupt(void) interrupt 3 using 0;
void int1_holo_interrupt(void) interrupt 2 using 0;
void pull_init(void);
void start_pull(void);
void holo_search_start(void);
void holo_search_init(void);
void inch_mode();

#endif
