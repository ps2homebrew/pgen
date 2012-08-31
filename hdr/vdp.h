#ifndef _VDP_H
#define _VDP_H

typedef enum {
  cd_vram_fetch, cd_vram_store,
  cd_2, cd_cram_store,
  cd_vsram_fetch, cd_vsram_store,
  cd_6, cd_7,
  cd_cram_fetch
} t_code;

extern unsigned int vdp_event;
extern unsigned int vdp_vislines;
extern unsigned int vdp_visstartline;
extern unsigned int vdp_visendline;
extern unsigned int vdp_totlines;
extern unsigned int vdp_framerate;
extern unsigned int vdp_clock;
extern unsigned int vdp_68kclock;
extern unsigned int vdp_clksperline_68k;
extern unsigned int vdp_line;
extern uint8 vdp_oddframe;
extern uint8 vdp_vblank;
extern uint8 vdp_hblank;
extern uint8 vdp_vsync;
extern uint8 vdp_dmabusy;
extern uint8 vdp_overseas;
extern uint8 vdp_layerB;
extern uint8 vdp_layerBp;
extern uint8 vdp_layerA;
extern uint8 vdp_layerAp;
extern uint8 vdp_layerW;
extern uint8 vdp_layerWp;
extern uint8 vdp_layerH;
extern uint8 vdp_layerS;
extern uint8 vdp_layerSp;
extern uint8 vdp_cram[];
extern uint8 vdp_vsram[];
extern uint8 vdp_vram[];
extern unsigned int vdp_cramchange;
extern uint8 vdp_cramf[];
extern unsigned int vdp_event_start;
extern unsigned int vdp_event_vint;
extern unsigned int vdp_event_hint;
extern unsigned int vdp_event_hdisplay;
extern unsigned int vdp_event_end;
extern signed int vdp_nextevent;
extern signed int vdp_dmabytes;
extern signed int vdp_hskip_countdown;
extern uint16 vdp_address;
extern t_code vdp_code;
extern uint8 vdp_ctrlflag;
extern uint16 vdp_first;
extern uint16 vdp_second;

void vdp_reset(void);
int vdp_init(void);
uint16 vdp_status(void);
void vdp_storectrl(uint16 data);
void vdp_storedata(uint16 data);
uint16 vdp_fetchdata(void);
void vdp_renderline(unsigned int line, uint8 *linedata, unsigned int odd);
void vdp_renderline_interlace2(unsigned int line, uint8 *linedata);
void vdp_showregs(void);
void vdp_describe(void);
void vdp_spritelist(void);
void vdp_endfield(void);

#ifdef __cplusplus
extern "C" void vdp_renderframe(uint8 *framedata, unsigned int lineoffset);
#else
void vdp_renderframe(uint8 *framedata, unsigned int lineoffset);
#endif

void vdp_setupvideo(void);
uint8 vdp_gethpos(void);

#define LEN_CRAM 128
#define LEN_VSRAM 80
#define LEN_VRAM 64*1024

/* an estimate of the total cell width including HBLANK, for calculations */
#define TOTAL_CELLWIDTH 64

extern uint8 vdp_reg[];

#endif
