/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

#include "generator.h"
#include "registers.h"

#include <string.h>
#include <setjmp.h>

#include "reg68k.h"
#include "cpu68k.h"
#include "mem68k.h"
#include "diss68k.h"
#include "cpuz80.h"
#include "vdp.h"
#include "ui.h"
#include "gensound.h"

/*** global variables ***/
/*
uint32 reg68k_pc;
uint32 *reg68k_regs;
t_sr reg68k_sr;
*/
/*** forward references ***/

/*** reg68k_external_step - execute one instruction ***/

unsigned int reg68k_external_step(void)
{
  static t_ipc ipc;
  static t_iib *piib;
  jmp_buf jb;
  static unsigned int clks;

  /* !!! entering global register usage area !!! */

  if (!setjmp(jb)) {
    /* move PC and register block into global processor register variables */
    reg68k_pc = regs.pc;
    reg68k_regs = regs.regs;
    reg68k_sr = regs.sr;

    if (regs.pending && ((reg68k_sr.sr_int >> 8) & 7) < regs.pending)
      reg68k_internal_autovector(regs.pending);

    if (!(piib = cpu68k_iibtable[fetchword(reg68k_pc)]))
      ui_err("Invalid instruction @ %08X [%04X]\n", reg68k_pc,
             fetchword(reg68k_pc));

    cpu68k_ipc(reg68k_pc,
               mem68k_memptr[(reg68k_pc >> 12) & 0xfff] (reg68k_pc &
                                                         0xFFFFFF), piib,
               &ipc);
    cpu68k_functable[fetchword(reg68k_pc) * 2 + 1] (&ipc);
    clks = piib->clocks;
    /* restore global registers back to permanent storage */
    regs.pc = reg68k_pc;
    regs.sr = reg68k_sr;
    longjmp(jb, 1);
  }
  cpu68k_clocks += clks;
  return clks;                  /* number of clocks done */
}

/*** reg68k_external_execute - execute at least given number of clocks,
     and return number of clocks executed too much ***/

unsigned int reg68k_external_execute(unsigned int clocks)
{
  unsigned int index;
  t_ipclist *list;
  t_ipc *ipc;
  uint32 pc24;
  jmp_buf jb;
  static t_ipc step_ipc;
  static t_iib *step_piib;
  static int clks;

  clks = clocks;

  if (!setjmp(jb)) {
    /* move PC and register block into global variables */
    reg68k_pc = regs.pc;
    reg68k_regs = regs.regs;
    reg68k_sr = regs.sr;

    if (regs.pending && ((reg68k_sr.sr_int >> 8) & 7) < regs.pending)
      reg68k_internal_autovector(regs.pending);

    do {
      pc24 = reg68k_pc & 0xffffff;
      if ((pc24 & 0xff0000) == 0xff0000) {
        /* executing code from RAM, do not use compiled information */
        do {
          step_piib = cpu68k_iibtable[fetchword(reg68k_pc)];
          if (!step_piib)
		  {
            ui_err("Invalid instruction (iib assert) @ %08X\n", reg68k_pc);
			gen_quit = 1;
			goto end;
		  }
          cpu68k_ipc(reg68k_pc,
                     mem68k_memptr[(reg68k_pc >> 12) &
                                   0xfff] (reg68k_pc & 0xFFFFFF),
                     step_piib, &step_ipc);
          cpu68k_functable[fetchword(reg68k_pc) * 2 + 1] (&step_ipc);
          clks -= step_piib->clocks;
          cpu68k_clocks += step_piib->clocks;
        }
        while (!step_piib->flags.endblk);
        list = NULL;            /* stop compiler warning ;(  */
      } else {
        index = (pc24 >> 1) & (LEN_IPCLISTTABLE - 1);
        list = ipclist[index];
        while (list && (list->pc != pc24)) {
          list = list->next;
        }
#ifdef PROCESSOR_ARM
        if (!list) {
          list = cpu68k_makeipclist(pc24);
		  if(gen_quit)
			  goto end;
          list->next = ipclist[index];
          ipclist[index] = list;
          list->compiled = compile_make(list);
        }
        list->compiled((t_ipc *) (list + 1));
#else
        if (!list) {
          /* LOG_USER(("Making IPC list @ %08x", pc24)); */
          list = cpu68k_makeipclist(pc24);
		  if(gen_quit)
			  goto end;
          list->next = ipclist[index];
          ipclist[index] = list;
        }
        ipc = (t_ipc *) (list + 1);
        do {
          ipc->function(ipc);
          ipc++;
        }
        while (*(int *)ipc);
#endif
        do {
          clks -= list->clocks;
          cpu68k_clocks += list->clocks;
        } while (list->norepeat && clks > 0);
      }
    }
    while (clks > 0);

end:
    /* restore global registers back to permanent storage */
    regs.pc = reg68k_pc;
    regs.sr = reg68k_sr;
    longjmp(jb, 1);
  }
  return -clks;                 /* i.e. number of clocks done too much */
}

/*** reg68k_external_autovector - for external use ***/

void reg68k_external_autovector(int avno)
{
  jmp_buf jb;

	if (cpu68k_frozen)
	{
		// RFB: Fixes crash
		regs.pending = avno;
		return;
	}

  if (!setjmp(jb)) {
    /* move PC and register block into global processor register variables */
    reg68k_pc = regs.pc;
    reg68k_regs = regs.regs;
    reg68k_sr = regs.sr;

    reg68k_internal_autovector(avno);

    /* restore global registers back to permanent storage */
    regs.pc = reg68k_pc;
    regs.sr = reg68k_sr;
    longjmp(jb, 1);
  }
}

/*** reg68k_internal_autovector - go to autovector - this call assumes global
     registers are already setup ***/

/* interrupts must not occur during cpu68k_frozen, as this flag indicates
   that we are catching up events due to a dma transfer.  Since the dma
   transfer is triggered by a memory write at which stage the current value
   of the PC is not written anywhere (due to being in the middle of a 68k
   block and it's in a local register), we MUST NOT use regs.pc - this
   routine uses reg68k_pc but this is loaded by reg68k_external_autovector,
   which is called by event_nextevent() and therefore will be a *WRONG*
   reg68k_pc! */

void reg68k_internal_autovector(int avno)
{
  int curlevel = (reg68k_sr.sr_int >> 8) & 7;
  uint32 tmpaddr;

  if ((curlevel < avno || avno == 7) && !cpu68k_frozen) {
    if (regs.stop) {
      LOG_DEBUG1(("stop finished"));
      /* autovector whilst in a STOP instruction */
      reg68k_pc += 4;
      regs.stop = 0;
    }
    if (!reg68k_sr.sr_struct.s) {
      reg68k_regs[15] ^= regs.sp;       /* swap A7 and SP */
      regs.sp ^= reg68k_regs[15];
      reg68k_regs[15] ^= regs.sp;
      reg68k_sr.sr_struct.s = 1;
    }
    reg68k_regs[15] -= 4;
    storelong(reg68k_regs[15], reg68k_pc);
    reg68k_regs[15] -= 2;
    storeword(reg68k_regs[15], reg68k_sr.sr_int);
    reg68k_sr.sr_struct.t = 0;
    reg68k_sr.sr_int &= ~0x0700;
    reg68k_sr.sr_int |= avno << 8;
    tmpaddr = reg68k_pc;
    reg68k_pc = fetchlong((V_AUTO + avno - 1) * 4);
    LOG_USER(("AUTOVECTOR %d: %X -> %X", avno, tmpaddr, reg68k_pc));
    regs.pending = 0;
  } else {
    LOG_USER(("%08X autovector %d pending", reg68k_pc, avno));
    // if (!regs.pending || regs.pending < avno) - not sure about this
    regs.pending = avno;
  }
}

/*** reg68k_internal_vector - go to vector - this call assumes global
     registers are already setup ***/

void reg68k_internal_vector(int vno, uint32 oldpc)
{
  if (!reg68k_sr.sr_struct.s) {
    reg68k_regs[15] ^= regs.sp; /* swap A7 and SP */
    regs.sp ^= reg68k_regs[15];
    reg68k_regs[15] ^= regs.sp;
    reg68k_sr.sr_struct.s = 1;
  }
  reg68k_regs[15] -= 4;
  storelong(reg68k_regs[15], oldpc);
  reg68k_regs[15] -= 2;
  storeword(reg68k_regs[15], reg68k_sr.sr_int);
  reg68k_pc = fetchlong(vno * 4);
  /* LOG_USER(("VECTOR %d: %X -> %X\n", vno, oldpc, reg68k_pc)); */
}
