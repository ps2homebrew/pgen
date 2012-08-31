/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

#include "generator.h"
#include "cpu68k.h"
#include "mem68k.h"
#include "cpuz80.h"
#include "memz80.h"
#include "gensound.h"
#include "ui.h"

/*** forward references ***/

uint8 memz80_fetch_bad_byte(uint16 addr);
uint8 memz80_fetch_sram_byte(uint16 addr);
uint8 memz80_fetch_yam_byte(uint16 addr);
uint8 memz80_fetch_bank_byte(uint16 addr);
uint8 memz80_fetch_mem_byte(uint16 addr);
uint8 memz80_fetch_psg_byte(uint16 addr);

void memz80_store_bad_byte(uint16 addr, uint8 data);
void memz80_store_sram_byte(uint16 addr, uint8 data);
void memz80_store_yam_byte(uint16 addr, uint8 data);
void memz80_store_bank_byte(uint16 addr, uint8 data);
void memz80_store_mem_byte(uint16 addr, uint8 data);
void memz80_store_psg_byte(uint16 addr, uint8 data);

/*** file scope variables */

/*** memory map ***/

t_memz80_def memz80_def[] = {
  {0x00, 0x40, memz80_fetch_sram_byte, memz80_store_sram_byte},
  {0x40, 0x41, memz80_fetch_yam_byte, memz80_store_yam_byte},
  {0x41, 0x60, memz80_fetch_bad_byte, memz80_store_bad_byte},
  {0x60, 0x70, memz80_fetch_bank_byte, memz80_store_bank_byte},
  {0x70, 0x7F, memz80_fetch_bad_byte, memz80_store_bad_byte},
  {0x7F, 0x80, memz80_fetch_psg_byte, memz80_store_psg_byte},
  {0x80, 0x100, memz80_fetch_mem_byte, memz80_store_mem_byte},
  {0, 0, NULL, NULL}
};

uint8 (*memz80_fetch_byte[0x100]) (uint16 addr);
void (*memz80_store_byte[0x100]) (uint16 addr, uint8 data);

/*** initialise memory tables ***/

int memz80_init(void)
{
  int i = 0;
  int j;

  do {
    for (j = memz80_def[i].start; j < memz80_def[i].end; j++) {
      memz80_fetch_byte[j] = memz80_def[i].fetch_byte;
      memz80_store_byte[j] = memz80_def[i].store_byte;
    }
    i++;
  }
  while ((memz80_def[i].start != 0) || (memz80_def[i].end != 0));
  return 0;
}

/*** BAD fetch/store ***/

uint8 memz80_fetch_bad_byte(uint16 addr)
{
  LOG_CRITICAL(("[Z80] Invalid memory fetch (byte) 0x%X", addr));
  return 0;
}

void memz80_store_bad_byte(uint16 addr, uint8 data)
{
  LOG_CRITICAL(("[Z80] Invalid memory store (byte) 0x%X = %X", addr, data));
}

/*** SRAM fetch/store ***/

uint8 memz80_fetch_sram_byte(uint16 addr)
{
  return (*(uint8 *)(cpuz80_ram + (addr & 0x1fff)));
}

void memz80_store_sram_byte(uint16 addr, uint8 data)
{
  *(uint8 *)(cpuz80_ram + (addr & 0x1fff)) = data;
  return;
}

/*** YAM fetch/store ***/

uint8 memz80_fetch_yam_byte(uint16 addr)
{
  addr -= 0x4000;
  if (addr < 4) {
    return sound_ym2612fetch(addr);
  } else {
    LOG_CRITICAL(("[Z80] invalid YAM fetch (byte) 0x%X", addr));
    return 0;
  }
}

void memz80_store_yam_byte(uint16 addr, uint8 data)
{
  addr -= 0x4000;
  /* LOG_USER(("[YAM] (z80) store (byte) 0x%X (%d)", addr, data)); */
  if (addr < 4)
    sound_ym2612store(addr, data);
  else
    LOG_CRITICAL(("[Z80] invalid YAM store (byte) 0x%X", addr));
}

/*** BANK fetch/store ***/

uint8 memz80_fetch_bank_byte(uint16 addr)
{
  /* write only */
  LOG_CRITICAL(("[Z80] Bank fetch (Byte) 0x%X", addr));
  return 0;
}

void memz80_store_bank_byte(uint16 addr, uint8 data)
{
  (void)addr;
  /* set bank */
  cpuz80_bankwrite(data);
}

/*** MEM (banked) fetch/store ***/

uint8 memz80_fetch_mem_byte(uint16 addr)
{
  return (fetchbyte(cpuz80_bank | (addr - 0x8000)));
}

void memz80_store_mem_byte(uint16 addr, uint8 data)
{
  /* LOG_USER(("WRITE whilst bank = %08X (%08X)", cpuz80_bank,
     addr-0x8000)); */
  storebyte(cpuz80_bank | (addr - 0x8000), data);
}

/*** PSG fetch/store ***/

uint8 memz80_fetch_psg_byte(uint16 addr)
{
  /* write only */
  LOG_CRITICAL(("[Z80] Invalid memory read (byte) 0x%X", addr));
  return 0;
}

void memz80_store_psg_byte(uint16 addr, uint8 data)
{
  if (addr == 0x7f11)
    sound_sn76496store(data);
  else
    LOG_CRITICAL(("[Z80] Invalid memory store (byte) 0x%X", addr));
}
