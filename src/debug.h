#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "dbgLevels.h"
#ifndef DEBUG_LEVEL 
#define DEBUG_LEVEL WARN_L
#endif

#if DEBUG_LEVEL <= FAIL_L
#define FAIL(format,...) dbg(FAIL_L,format,##__VA_ARGS__)
#else
#define FAIL
#endif

#if DEBUG_LEVEL <= ERROR_L
#define ERR(format,...) dbg(ERROR_L,format,##__VA_ARGS__)
#else
#define ERR
#endif

#if DEBUG_LEVEL <= WARN_L
#define WARN(format,...) dbg(WARN_L,format,##__VA_ARGS__)
#else
#define WARN
#endif

#if DEBUG_LEVEL <= INFO_L
#define INFO(format,...) dbg(INFO_L,format,##__VA_ARGS__)
#else
#define INFO
#endif

#if DEBUG_LEVEL <= DBG_L
#define DBG(format,...) dbg(DBG_L,format,##__VA_ARGS__)
#else
#define DBG
#endif

#if DEBUG_LEVEL <= VERBOSE_L
#define VERBOSE(format,...) dbg(VERBOSE_L,format,##__VA_ARGS__)
#else
#define VERBOSE
#endif

// yea 16 bit int is overkill but unpacking and storing a uint16_t is faster and takes less code
extern uint16_t  DbgLevel;
void dbg(uint16_t level,const char* format,...);
void setDbgLvl(uint16_t level);

#endif

