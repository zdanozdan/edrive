#ifndef _TRACE_H_
#define _TRACE_H_

////////////////////////////////////////////////////////////////////////////////
//
//                          Trace module definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifdef _TRACE_ON_

#define _TRACE_INIT_    trace_init()
#define _TRACE_ENABLE_  trace_enable
#define _TRACE_DISABLE_ trace_disable 
#define _TRACE_PRINT_   trace_print

#else

#define _TRACE_INIT_
#define _TRACE_ENABLE_
#define _TRACE_DISABLE_
#define _TRACE_PRINT_

#endif

int  trace_init ( void );

void trace_enable ( const unsigned int trace_id, char* level, const int timestamp );

void trace_disable ( const unsigned int trace_id );

void trace_print ( const unsigned int trace_id, const unsigned int level, char *buffer, ... );

#endif


