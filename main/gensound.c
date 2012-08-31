/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

#include "generator.h"
#include "gensound.h"
#include "gensoundp.h"
#include "vdp.h"
#include "ui.h"
#include "psg.h"
#include "ym2612.h"

#include <tamtypes.h>
#include "pgen_vars.h"

/*
#include "sn76496.h"

#ifdef JFM
#  include "jfm.h"
#else
#  include "support.h"
#  include "fm.h"
#endif
*/
/*** variables externed ***/

unsigned int sound_speed = 24000;    /* sample rate */

int sound_debug = 0;            /* debug mode */
int sound_feedback = 0;         /* -1, running out of sound
                                   +0, lots of sound, do something */
unsigned int sound_minfields = 5;       /* min fields to try to buffer */
unsigned int sound_maxfields = 10;      /* max fields before blocking */
unsigned int sound_sampsperfield;       /* samples per field */
unsigned int sound_normalsamples;
unsigned int sound_threshold;   /* samples in buffer aiming for */
uint8 sound_regs1[256];
uint8 sound_regs2[256];
uint8 sound_address1 = 0;
uint8 sound_address2 = 0;
uint8 sound_keys[8];
int sound_logsample = 0;        /* sample to log or -1 if none */
unsigned int sound_on = 1;      /* sound enabled */
unsigned int sound_psg = 1;     /* psg enabled */
unsigned int sound_fm = 1;

//960 is biggest that will ever be needed (48Khz PAL)
sint32 sound_soundbuf[2][960];
sint16 end_soundbuf[2][960];

/*** forward references ***/

static void sound_process(void);

/*** file scoped variables ***/

static int sound_active = 0;

#ifdef JFM
static t_jfm_ctx *sound_ctx;
#endif

/*** sound_init - initialise this sub-unit ***/

int sound_init(void)
{

  sound_sampsperfield = sound_speed / pgenRuntimeSetting.maxFrameSec;
  sound_normalsamples = 48000 / pgenRuntimeSetting.maxFrameSec;
  sound_threshold = sound_minfields * sound_sampsperfield;

  sound_start();
    if(YM2612_Init(vdp_clock / 7,sound_speed,0)) {
    LOG_VERBOSE(("YM2612 failed init"));
    sound_stop();
    return 1;
  }

  PSG_Init(vdp_clock / 15,sound_speed);

  LOG_VERBOSE(("YM2612 Initialised @ sample rate %d", sound_speed));
  return 0;
}

/*** sound_final - finalise this sub-unit ***/

void sound_final(void)
{
  sound_stop();
//  YM2612Shutdown();
  YM2612_End();
}

/*** sound_start - start sound ***/

int sound_start(void)
{
  if (sound_active)
    sound_stop();
  sound_sampsperfield = sound_speed / vdp_framerate;
  LOG_VERBOSE(("Starting sound..."));
  if (soundp_start() == -1) {
    LOG_CRITICAL(("Failed to start sound hardware"));
    sound_active = 0;
    return 1;
  }
  sound_active = 1;
  LOG_VERBOSE(("Started sound."));
  return 0;
}

/*** sound_stop - stop sound ***/

void sound_stop(void)
{
  if (!sound_active)
    return;
  LOG_VERBOSE(("Stopping sound..."));
  soundp_stop();
  sound_active = 0;
  LOG_VERBOSE(("Stopped sound."));
}

/*** sound_reset - reset sound sub-unit ***/

int sound_reset(void)
{
  sound_final();
  return sound_init();
}

/*** sound_startfield - start of frame ***/

void sound_startfield(void)
{
	int i;

	for(i=0;i<sound_sampsperfield;i++) {
		sound_soundbuf[0][i] = 0;
		sound_soundbuf[1][i] = 0;
	}
}
/*** sound_endfield - end frame and output sound ***/

void sound_endfield(void)
{
  if (!sound_active) {
    /* sound is turned off - let generator continue */
    sound_feedback = 0;
    return;
  }

  sound_process();
  soundp_output(end_soundbuf[0], end_soundbuf[1], sound_sampsperfield);
}

/*** sound_ym2612fetch - fetch byte from ym2612 chip ***/

uint8 sound_ym2612fetch(uint8 addr)
{
//  return YM2612Read(0, addr);
  return YM2612_Read();
}

/*** sound_ym2612store - store a byte to the ym2612 chip ***/

void sound_ym2612store(uint8 addr, uint8 data)
{
//  YM2612Write(0, addr, data);
  YM2612_Write(addr,data);
}

/*** sound_sn76496store - store a byte to the sn76496 chip ***/

void sound_sn76496store(uint8 data)
{
//  SN76496Write(0, data);
  PSG_Write(data);
}

/*** sound_genreset - reset genesis sound ***/

void sound_genreset(void)
{
//  YM2612ResetChip(0);
  YM2612_Reset();
}

/*** sound_line - called at end of line ***/

void sound_line(void)
{
	int s1 = (sound_sampsperfield * (vdp_line)) / vdp_totlines;
	int s2 = (sound_sampsperfield * (vdp_line + 1)) / vdp_totlines;
	unsigned int samples = s2 - s1;
	static sint32 *tbuf[2];

	if(!sound_on) return;

	tbuf[0] = sound_soundbuf[0] + s1;
	tbuf[1] = sound_soundbuf[1] + s1;

	if (s2 > s1) YM2612_DacAndTimers_Update(tbuf,samples);

}

/*** sound_process - process sound ***/

static void sound_process(void)
{
	static sint32 *tbuf[2];
	unsigned int i;

	tbuf[0] = sound_soundbuf[0];
	tbuf[1] = sound_soundbuf[1];

	if (sound_psg)
		PSG_Update(tbuf,sound_sampsperfield);

	if (sound_fm)
		YM2612_Update(tbuf,sound_sampsperfield);

	for(i=0;i<sound_sampsperfield;i++) {
		if(sound_soundbuf[0][i] > 0x7FFF) sound_soundbuf[0][i] = 0x7FFF;
		else if(sound_soundbuf[0][i] < -0x8000) sound_soundbuf[0][i] = -0x8000;

		if(sound_soundbuf[1][i] > 0x7FFF) sound_soundbuf[1][i] = 0x7FFF;
		else if(sound_soundbuf[1][i] < -0x8000) sound_soundbuf[1][i] = -0x8000;
	}

    	for(i=0;i<sound_sampsperfield;i++) {
    		end_soundbuf[0][i] = (short)sound_soundbuf[0][i];
			end_soundbuf[1][i] = (short)sound_soundbuf[1][i];
    	}
}
