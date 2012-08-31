/* Generator is (c) James Ponder, 1997-2001 http://www.squ#include <stdlib.h> */
/* This file contains the interface to the Z80 emulator core, taken from MAME. */
/* Support for the MAME Z80 core added by Nick Van Veen */

//#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "generator.h"
#include "cpuz80.h"
#include "cpu68k.h"
#include "memz80.h"
#include "ui.h"

#include "z80.h"

int z80_irq_callback(int param);

/*** variables externed ***/

uint8 *cpuz80_ram = NULL;
uint32 cpuz80_bank = 0;
uint8 cpuz80_active = 0;
uint8 cpuz80_resetting = 0;
unsigned int cpuz80_pending = 0;
unsigned int cpuz80_on = 1;     /* z80 turned on? */

/*** global variables ***/

static unsigned int cpuz80_lastsync = 0;

int cpuz80_init(void)
{
  cpuz80_reset();
  return 0;
}

/*** cpuz80_reset - reset z80 sub-unit ***/

void cpuz80_reset(void)
{
  if (!cpuz80_ram) {
    if ((cpuz80_ram = malloc(LEN_SRAM)) == NULL) {
      ui_err("Out of memory\n");
    }
  }
  memset(cpuz80_ram, 0, LEN_SRAM);
  cpuz80_bank = 0;
//  cpuz80_bank = 0xff8000;
  cpuz80_active = 0;
  cpuz80_lastsync = 0;
  cpuz80_resetting = 1;
  cpuz80_pending = 0;
  z80_reset(0);
  z80_set_irq_callback(z80_irq_callback);
}

int z80_irq_callback(int param)
{
    return (0xFF);
}

/*** cpuz80_updatecontext - inform z80 processor of changed context ***/

void cpuz80_updatecontext(void)
{
  /* not required with raze */
}

/*** cpuz80_resetcpu - reset z80 cpu ***/

void cpuz80_resetcpu(void)
{
  cpuz80_sync();
  z80_reset(0);
  z80_set_irq_callback(z80_irq_callback);
  cpuz80_resetting = 1;         /* suspends execution */
}

/*** cpuz80_unresetcpu - unreset z80 cpu ***/

void cpuz80_unresetcpu(void)
{
  if (cpuz80_resetting)
    cpuz80_sync();
  cpuz80_resetting = 0;         /* un-suspends execution */
}

/*** cpuz80_bankwrite - data is being written to latch ***/

void cpuz80_bankwrite(uint8 data)
{
  cpuz80_bank = (((cpuz80_bank >> 1) | ((data & 1) << 23)) & 0xff8000);
}

/*** cpuz80_stop - stop the processor ***/

void cpuz80_stop(void)
{
  cpuz80_sync();
  cpuz80_active = 0;
}

/*** cpuz80_start - start the processor ***/

void cpuz80_start(void)
{
  if (!cpuz80_active)
    cpuz80_sync();
  cpuz80_active = 1;
}

/*** cpuz80_endfield - reset counters ***/

void cpuz80_endfield(void)
{
  cpuz80_lastsync = 0;
}

/*** cpuz80_sync - synchronise ***/

void cpuz80_sync(void)
{
  int cpu68k_wanted = cpu68k_clocks - cpuz80_lastsync;
  int wanted = (cpu68k_wanted < 0 ? 0 : cpu68k_wanted) * 7 / 15;
  int acheived;

  if (cpuz80_pending && Z80_Context->IFF1) {
    z80_set_irq_line(0, ASSERT_LINE);
    z80_set_irq_line(0, CLEAR_LINE);
    cpuz80_pending = 0;
  }
  if (cpuz80_on && cpuz80_active && !cpuz80_resetting) {
    /* ui_log(LOG_USER, "executing %d z80 clocks @ %X", wanted,
       cpuz80_z80.z80pc); */
    acheived = z80_execute(wanted);
    cpuz80_lastsync = cpuz80_lastsync + acheived * 15 / 7;
  } else {
    cpuz80_lastsync = cpu68k_clocks;
  }
}

/*** cpuz80_interrupt - cause an interrupt on the z80 */

void cpuz80_interrupt(void)
{
  if (!cpuz80_resetting) {
    if (Z80_Context->IFF1 == 0)
      cpuz80_pending = 1;
    z80_set_irq_line(0, ASSERT_LINE);
    z80_set_irq_line(0, CLEAR_LINE);
  }
}

void cpuz80_uninterrupt(void)
{
  z80_set_irq_line(0, CLEAR_LINE);
}

uint8 cpuz80_portread(uint8 port) { return 0; }
void cpuz80_portwrite(uint8 port, uint8 value) { }
