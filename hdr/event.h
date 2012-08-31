#ifndef _EVENT_H
#define _EVENT_H

/*****************************************************************************/
/*     Generator - Sega Genesis emulation - (c) James Ponder 1997-2001       */
/*****************************************************************************/
/*                                                                           */
/* event.h                                                                   */
/*                                                                           */
/*****************************************************************************/

#ifdef __cplusplus
extern "C" void event_doframe(void);
#else
void event_doframe(void);
#endif
void event_dostep(void);
void event_freeze_clocks(unsigned int clocks);
void event_freeze(unsigned int bytes);

#endif
