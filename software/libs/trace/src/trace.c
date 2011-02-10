
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "rtos.h"


////////////////////////////////////////////////////////////////////////////////
//
//                       Internal module's definitions
//
////////////////////////////////////////////////////////////////////////////////

#define TRACE_MAX_ID  64

#define TRACE_LEVEL_NONE               0x00

#define TRACE_LEVEL_ERROR              0x01
#define TRACE_LEVEL_WARNING            0x02
#define TRACE_LEVEL_INFO               0x03
#define TRACE_LEVEL_DEBUG              0x04

#define TRACE_LEVEL_ERROR_TIMESTAMP    0x81
#define TRACE_LEVEL_WARNING_TIMESTAMP  0x82
#define TRACE_LEVEL_INFO_TIMESTAMP     0x83
#define TRACE_LEVEL_DEBUG_TIMESTAMP    0x84


typedef struct
{
      char *level;
      unsigned char mask;
      
} TraceLevelPattern;


////////////////////////////////////////////////////////////////////////////////
//
//                         Internal module's variables
//
////////////////////////////////////////////////////////////////////////////////

static unsigned char trace_mask[TRACE_MAX_ID];

static int trace_sem;

static long trace_sem_timeout=0L;

static const char* levels[] = { "NONE: ","ERROR: ","WARNING: ", "INFO: ", "DEBUG: " };


////////////////////////////////////////////////////////////////////////////////
//
//                         Public module's functions
//
////////////////////////////////////////////////////////////////////////////////


int trace_init ( void )
{
   return RTX_Create_Sem( &trace_sem, "TRC", 1 );
}





void trace_enable( const unsigned int trace_id, char *level, const int timestamp )
{   
   const TraceLevelPattern patterns_table1[]=
      {
         { "ERROR",   TRACE_LEVEL_ERROR   },
         { "WARNING", TRACE_LEVEL_WARNING },
         { "INFO",    TRACE_LEVEL_INFO    },
         { "DEBUG",   TRACE_LEVEL_DEBUG   },
         {  NULL,     0                   }
      };

   const TraceLevelPattern patterns_table2[]=
      {
         { "ERROR",   TRACE_LEVEL_ERROR_TIMESTAMP   },
         { "WARNING", TRACE_LEVEL_WARNING_TIMESTAMP },
         { "INFO",    TRACE_LEVEL_INFO_TIMESTAMP    },
         { "DEBUG",   TRACE_LEVEL_DEBUG_TIMESTAMP   },
         {  NULL,     0                             }
      };

   const TraceLevelPattern *trace_patterns;
   char *ptr;

   if ( timestamp==0 ) trace_patterns=patterns_table1;
   else trace_patterns=patterns_table2;

   if ( trace_id < TRACE_MAX_ID )
   {
      ptr=strupr(level);

      for ( int i=0; trace_patterns[i].level!=NULL; i++ )
      {
         if ( strcmp ( trace_patterns[i].level, ptr )==0 )
         {
            trace_mask[trace_id] = trace_patterns[i].mask;
            printf( "Trace enabled: %u with level: %s\n", trace_id, trace_patterns[i].level );
            return;
         }
      }//end for (...)
   }//end if ( trace_id < TRACE_MAX_ID )

   trace_mask[trace_id] = TRACE_LEVEL_ERROR;
   printf( "Trace enabled: %u with level: %s\n", trace_id, "ERROR" );
}





void trace_disable( const unsigned int trace_id )
{
   if ( trace_id<TRACE_MAX_ID )
   {
      trace_mask[trace_id]=TRACE_LEVEL_NONE;
   }
}





void trace_print( const unsigned int trace_id, const unsigned int level, char *buffer, ... )
{
   unsigned char tmp;
   unsigned long ticks;
   va_list args;

   va_start( args, buffer );
   if ( trace_id<TRACE_MAX_ID )
   {
      tmp=trace_mask[trace_id];
      if ( (tmp&0x80)==0 )
      {
         if ( level<=tmp )
         {
            if ( RTX_Wait_Sem( trace_sem, &trace_sem_timeout )==0 )
            {
               printf(levels[level]);
               vprintf ( buffer, args );
               RTX_Signal_Sem( trace_sem );
            }
         }//end if ( level<=tmp )
      }//end if ( (tmp&0x80)==0 )
      else
      {
         tmp&=0x7F;
         if ( level<=tmp )
         {
            if ( RTX_Wait_Sem( trace_sem, &trace_sem_timeout )==0 )
            {
               printf(levels[level]);
               RTX_Get_System_Ticks( &ticks );
               printf  ( "[ %08lu ] ", ticks );
               vprintf ( buffer, args );
               RTX_Signal_Sem( trace_sem );
            }
         }//end if ( level<=tmp )
      }//end if ( (tmp&0x80)==0 ) else
   }//end if ( trace_id<TRACE_MAX_ID )
   va_end( args );
}











