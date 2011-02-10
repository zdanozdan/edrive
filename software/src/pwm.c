/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/src/pwm.c,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: pwm.c,v $
**  Revision 1.4  2003/12/28 14:50:29  tomasz
**  timer frequency and initial speed changed.
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
**  $Id: pwm.c,v 1.4 2003/12/28 14:50:29 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#include "89c51rd2.h"
#include <stdio.h>        
#include "global.h"
#include "pwm.h"

void pwm_init(void)
{
   //   CMOD = 0x00; //PCA timer frequency Fclk/6
   CMOD = 0x2; //PCA timer frequency Fclk/2
   CCAP0H = PWM_INITIAL_SPEED;
   CCAPM0 = CCAPMn_PCA_PWM_ENABLE | CCAPMn_PCA_ENABLE_COMPARATOR;
   CR = 1; //PCA timer start
}

/*  void pwm_start(void) */
/*  { */
/*     CR = 1; */
/*  } */

/*  void pwm_stop(void) */
/*  { */
/*     CR = 0; */
/*  } */

/*  void pwm_update(const char value) */

/*  { */
/*     CCAP0H = value; */
/*  } */


