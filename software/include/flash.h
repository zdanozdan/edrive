/**************************************************************************
**                                                                        
**  FILE : $Source: /CVS/cvsrepo/projects/edrive/software/include/flash.h,v $
**                                                                        
**  $Author: tomasz $                                                     
**  $Log: flash.h,v $
**  Revision 1.1  2003/12/22 10:56:07  tomasz
**  first release
**
**
**  $Id: flash.h,v 1.1 2003/12/22 10:56:07 tomasz Exp $       
**  
**  COPYRIGHT   :  2003 Easy Devices                                
**************************************************************************/

#ifndef __FLASH_H__
#define __FLASH_H__

unsigned char flash_write_byte(unsigned char value,unsigned int address);
unsigned char flash_write_page(unsigned char count, unsigned int xram, unsigned int flash);
unsigned char read_manufacturer(void);
#endif
