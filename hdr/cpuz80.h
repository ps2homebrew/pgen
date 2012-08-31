#ifndef _CPUZ80_H
#define _CPUZ80_H

#define LEN_SRAM 0x2000

extern uint8 *cpuz80_ram;
extern uint32 cpuz80_bank;
extern uint8 cpuz80_active;
extern uint8 cpuz80_resetting;
extern unsigned int cpuz80_on;

void cpuz80_reset(void);
void cpuz80_resetcpu(void);
void cpuz80_unresetcpu(void);
void cpuz80_bankwrite(uint8 data);
void cpuz80_stop(void);
void cpuz80_start(void);
void cpuz80_endfield(void);
void cpuz80_sync(void);
void cpuz80_interrupt(void);
void cpuz80_uninterrupt(void); /* debug */
uint8 cpuz80_portread(uint8 port);
void cpuz80_portwrite(uint8 port, uint8 value);
int cpuz80_init(void);
void cpuz80_updatecontext(void);

#endif
