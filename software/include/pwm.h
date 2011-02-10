/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/pwm.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: pwm.h,v $
**  Revision 1.4  2004/11/19 20:33:37  tomasz
**  latest changes incoroprated
**
**  Revision 1.3  2004/01/19 13:46:16  tomasz
**  changed PWM_INITIAL_SPEED to newly assembled board
**
**  Revision 1.2  2003/12/28 14:55:52  tomasz
**  initial speed changed
**
**  Revision 1.1  2003/11/08 12:00:19  tomasz
**  first release
**
**  $Id: pwm.h,v 1.4 2004/11/19 20:33:37 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#ifndef __PWM_H__
#define __PWM_H__

#define CCAPMn_PCA_INTERRUPT_ENABLE  0x1
#define CCAPMn_PCA_PWM_ENABLE        0x2
#define CCAPMn_PCA_TOGGLE            0x4
#define CCAPMn_PCA_MATCH             0x8
#define CCAPMn_PCA_CAPTURE_NEGATIVE  0x10
#define CCAPMn_PCA_CAPTURE_POSITIVE  0x20
#define CCAPMn_PCA_ENABLE_COMPARATOR 0x40

//#define PWM_INITIAL_SPEED            205
//#define PWM_INITIAL_SPEED            235
#define PWM_INITIAL_SPEED            220

void pwm_init(void);

#endif
