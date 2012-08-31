#ifndef _MACHINE_H
#define _MACHINE_H

/*****************************************************************************/
/*     Generator - Sega Genesis emulation - (c) James Ponder 1997-1998       */
/*****************************************************************************/
/*                                                                           */
/* machine.h                                                                 */
/*                                                                           */
/*****************************************************************************/

#include "config.h"

/* get correct sizes for uint8, uint16 and uint32 */

#ifndef NULL
#define NULL	(void *)0
#endif

typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;

/*
#if (SIZEOF_UNSIGNED_CHAR == 1)
typedef unsigned char uint8;
typedef signed char sint8;
#elif (SIZEOF_UNSIGNED_SHORT == 1)
typedef unsigned short uint8;
typedef signed short sint8;
#elif (SIZEOF_UNSIGNED_INT == 1)
typedef unsigned int uint8;
typedef signed int sint8;
#elif (SIZEOF_UNSIGNED_LONG == 1)
typedef unsigned long uint8;
typedef signed long sint8;
#elif (SIZEOF_UNSIGNED_LONG_LONG == 1)
typedef unsigned long long uint8;
typedef signed long long sint8;
#else
typedef unsigned char uint8;
typedef signed char sint8;
#warning Unable to work out the 8-bit integer names
#endif

#if (SIZEOF_UNSIGNED_CHAR == 2)
typedef unsigned char uint16;
typedef signed char sint16;
#elif (SIZEOF_UNSIGNED_SHORT == 2)
typedef unsigned short uint16;
typedef signed short sint16;
#elif (SIZEOF_UNSIGNED_INT == 2)
typedef unsigned int uint16;
typedef signed int sint16;
#elif (SIZEOF_UNSIGNED_LONG == 2)
typedef unsigned long uint16;
typedef signed long sint16;
#elif (SIZEOF_UNSIGNED_LONG_LONG == 2)
typedef unsigned long long uint16;
typedef signed long long sint16;
#else
typedef unsigned short uint16;
typedef signed short sint16;
#warning Unable to work out the 16-bit integer names
#endif

#if (SIZEOF_UNSIGNED_CHAR == 4)
typedef unsigned char uint32;
typedef signed char sint32;
#elif (SIZEOF_UNSIGNED_SHORT == 4)
typedef unsigned short uint32;
typedef signed short sint32;
#elif (SIZEOF_UNSIGNED_INT == 4)
typedef unsigned int uint32;
typedef signed int sint32;
#elif (SIZEOF_UNSIGNED_LONG == 4)
typedef unsigned long uint32;
typedef signed long sint32;
#elif (SIZEOF_UNSIGNED_LONG_LONG == 4)
typedef unsigned long long uint32;
typedef signed long long sint32;
#else
#warning Unable to work out the 32-bit integer names
typedef unsigned int uint32;
typedef signed int sint32;
#endif
*/

#endif
