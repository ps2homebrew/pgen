/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

#include "generator.h"

#include "vdp.h"
#include "cpu68k.h"
#include "cpuz80.h"
#include "reg68k.h"
#include "ui.h"
#include "gensound.h"

/* due to DMA transfers, event_nextevent can be called during an instruction
   cycle (reg68k_external_execute -> instruction -> vdp write -> dma ->
   event_freeze -> event_nextevent).  Be careful */

/* time for next event - update vdp_event - return when to call again */

inline void event_nextevent(void)
{
  /* call this when it *is* time for the next event as dictated by vdp_event,
     so we switch on it and update vdp_event at the same time */

  switch (vdp_event++) {
  case 0:
  EVENT_NEWLINE:
    LOG_DEBUG1(("%08X due %08X, %d A: %d (cd=%d)",
                cpu68k_clocks, vdp_event_start,
                vdp_line - vdp_visstartline, vdp_reg[10],
                vdp_hskip_countdown));
    if (vdp_line == 0)
      sound_startfield();
    if (vdp_line == (vdp_visstartline - 1)) {
      vdp_vblank = 0;
      vdp_hskip_countdown = vdp_reg[10];
    }
    if ((vdp_nextevent = vdp_event_vint - cpu68k_clocks) > 0)
      break;
    vdp_event++;
  case 1:
    LOG_DEBUG1(("%08X due %08X, %d B: %d (cd=%d)",
                cpu68k_clocks, vdp_event_vint,
                vdp_line - vdp_visstartline, vdp_reg[10],
                vdp_hskip_countdown));
    if (vdp_line == vdp_visendline) {
      vdp_vblank = 1;
      vdp_vsync = 1;
      if (vdp_reg[1] & 1 << 5)
        reg68k_external_autovector(6);  /* vertical interrupt */
    }
    if ((vdp_nextevent = vdp_event_hint - cpu68k_clocks) > 0)
      break;
    vdp_event++;
  case 2:
    LOG_DEBUG1(("%08X due %08X, %d C: %d (cd=%d)",
                cpu68k_clocks, vdp_event_hint,
                vdp_line - vdp_visstartline, vdp_reg[10],
                vdp_hskip_countdown));
    if (vdp_line >= vdp_visstartline && vdp_line < vdp_visendline)
      vdp_hblank = 1;
    if (vdp_line == (vdp_visstartline - 1) || (vdp_line > vdp_visendline)) {
      vdp_hskip_countdown = vdp_reg[10];
      LOG_DEBUG1(("H counter reset to %d", vdp_hskip_countdown));
    }
    if (vdp_reg[0] & 1 << 4) {
      LOG_DEBUG1(("pre = %d", vdp_hskip_countdown));
      if (vdp_hskip_countdown-- == 0) {
        LOG_DEBUG1(("in = %d", vdp_hskip_countdown));
        /* re-initialise counter */
        vdp_hskip_countdown = vdp_reg[10];
        LOG_DEBUG1(("H counter looped to %d", vdp_hskip_countdown));
        if (vdp_line >= vdp_visstartline - 1 && vdp_line < vdp_visendline - 1)
          reg68k_external_autovector(4);        /* horizontal interrupt */
        /* since this game is obviously timing sensitive, we sacrifice
           executing the right number of 68k clocks this frame in order
           to accurately do the moments following H-Int */
        cpu68k_clocks = vdp_event_hint;
      }
      LOG_DEBUG1(("post = %d", vdp_hskip_countdown));
    }
    /* the 68k is frozen for 68k ram to vram copies, see event_freeze */
    if (vdp_dmabytes) {         /* blank mode ? */
      vdp_dmabytes -= (vdp_vblank || !(vdp_reg[1] & 1 << 6))
        ? ((vdp_reg[12] & 1) ? 205 : 167) : ((vdp_reg[12] & 1) ? 18 : 16);
      if (vdp_dmabytes <= 0) {
        vdp_dmabytes = 0;
        vdp_dmabusy = 0;
      }
    }
    if ((vdp_nextevent = vdp_event_hdisplay - cpu68k_clocks) > 0)
      break;
    vdp_event++;
  case 3:
    LOG_DEBUG1(("%08X due %08X, %d D: %d (cd=%d)",
                cpu68k_clocks, vdp_event_hdisplay,
                vdp_line - vdp_visstartline, vdp_reg[10],
                vdp_hskip_countdown));
    ui_line(vdp_line - vdp_visstartline + 1);
    if ((vdp_nextevent = vdp_event_end - cpu68k_clocks) > 0)
      break;
    /* vdp_event++; - not required, we set vdp_event to 0 below */
  case 4:
    /* end of line, do sound, platform stuff */
    LOG_DEBUG1(("%08X due %08X, %d E: %d (cd=%d)",
                cpu68k_clocks, vdp_event_end,
                vdp_line - vdp_visstartline, vdp_reg[10],
                vdp_hskip_countdown));
    if (vdp_line >= vdp_visstartline && vdp_line < vdp_visendline)
      vdp_hblank = 0;
    cpuz80_sync();
    sound_line();
    vdp_line++;
    if (vdp_line == vdp_visendline)
      cpuz80_interrupt();
    if (vdp_line == vdp_totlines) {
      /* the order of these is important */
      sound_endfield(); /* must be before ui_endfield for GYM log */
      ui_endfield();
      vdp_endfield(); /* must be after ui_endfield as it alters state */
      cpuz80_endfield();
      cpu68k_endfield();
      cpu68k_frames++;
    }
    vdp_event_start += vdp_clksperline_68k;
    vdp_event_vint += vdp_clksperline_68k;
    vdp_event_hint += vdp_clksperline_68k;
    vdp_event_hdisplay += vdp_clksperline_68k;
    vdp_event_end += vdp_clksperline_68k;
    vdp_event = 1;
    goto EVENT_NEWLINE;
  }                             /* switch */
}

/*** event_doframe - execute until the end of the current frame ***/

void event_doframe(void)
{
  unsigned int startframe = cpu68k_frames;

  do {
    while (vdp_nextevent > 0)
	{
      vdp_nextevent = -reg68k_external_execute(vdp_nextevent);
	  if(gen_quit)
		  return;
	}
    event_nextevent();
  }
  while (startframe == cpu68k_frames);

}

/*** event_step - execute one instruction with no caching ***/

void event_dostep(void)
{
  /* execute one instruction and subtract from vdp_nextevent those cycles */
  vdp_nextevent -= reg68k_external_step();
  /* if negative or 0, i.e. we have done all the cycles we need to this event,
     call event_nextevent! */
  while (vdp_nextevent <= 0)
    event_nextevent();
}

/*** event_freeze_clocks - freeze 68k for given clock cycles ***/

/* NB: this routine is called by event_freeze which is called by
   vdp routines.  This could all happen IN THE MIDDLE OF A BLOCK at which
   point cpu68k_clocks will have been updated but vdp_nextevent will not! 
   therefore we must at this point work out what vdp_nextevent should be
   at the current moment in time, add on the clocks, wind forward time and
   then subtract the clocks that will be added later */

void event_freeze_clocks(unsigned int clocks)
{
  int missed = 0;

  /* first - fix vdp_nextevent to be correct for right now, due to block
     marking delay */

  /* find out how many clocks vdp_nextevent has missed */
  /* modify vdp_nextevent to reflect the real state as of now */

  switch (vdp_event) {
  case 0:
    missed = vdp_nextevent - (vdp_event_start - cpu68k_clocks);
    vdp_nextevent -= missed;
    break;
  case 1:
    missed = vdp_nextevent - (vdp_event_vint - cpu68k_clocks);
    vdp_nextevent -= missed;
    break;
  case 2:
    missed = vdp_nextevent - (vdp_event_hint - cpu68k_clocks);
    vdp_nextevent -= missed;
    break;
  case 3:
    missed = vdp_nextevent - (vdp_event_hdisplay - cpu68k_clocks);
    vdp_nextevent -= missed;
    break;
  case 4:
    missed = vdp_nextevent - (vdp_event_end - cpu68k_clocks);
    vdp_nextevent -= missed;
    break;
  default:
//    printf("assertion failed: bad vdp_event in event_freeze_clocks: %d\n",
//           vdp_event);
  }

  /* move cpu68k_clocks and vdp_nextevent forward in time */

  cpu68k_clocks += clocks;
  vdp_nextevent -= clocks;

  /* now catch up events */

  while (vdp_nextevent <= 0)
    event_nextevent();

  /* and then un-adjust vdp_nextevent, as the block marking code will update
     this later */

  vdp_nextevent += missed;
}

/*** event_freeze - freeze 68k for given VDP byte transfer ***/

void event_freeze(unsigned int bytes)
{
  int wide = vdp_reg[12] & 1;
  int clocks, possible;
  double percent_possible;
  int togo = (int)bytes;

  cpu68k_frozen = 1;            /* prohibit interrupts since PC is not known in the
                                   middle of a 68k block due to register mappings */

  while (togo > 0) {
    /* clocks will be negative if we're in the middle of a cpu block */
    clocks = vdp_event_end - cpu68k_clocks;
    if (clocks < 0)
      clocks = 0;
    percent_possible = clocks / vdp_clksperline_68k;
    if (vdp_reg[1] & 1 << 6 && !vdp_vblank) {
      /* vdp active */
      possible = (unsigned int)(percent_possible * (wide ? 18 : 16));
    } else {
      /* vdp inactive */
      possible = (unsigned int)(percent_possible * (wide ? 205 : 167));
    }
    if (togo >= possible) {
      event_freeze_clocks(clocks);
      togo -= possible;
    } else {
      event_freeze_clocks(((double)togo / possible) * clocks);
      togo = 0;
    }
  }
  cpu68k_frozen = 0;
}
