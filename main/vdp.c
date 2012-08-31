/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

/* video display processor emulation */

/* for dma 'bytes per line' stuff we use 'memory to vram' figures as a baseline
   and double the number of real bytes for 'vram copy' in order to adjust to
   our model; also there is a subtraction of one for vram fill to compensate
   there too - probably being far too picky and it isn't exactly perfect
   anyway */

/* to do: fix F flag in status register */

#include <string.h>

#include "generator.h"
#include "vdp.h"
#include "cpu68k.h"
#include "ui.h"
#include "event.h"

#undef DEBUG_VDP
#undef DEBUG_VDPDMA
#undef DEBUG_VDPDATA
#undef DEBUG_VDPCRAM

/*** variables externed ***/

unsigned int vdp_event;
unsigned int vdp_vislines;
unsigned int vdp_visstartline;
unsigned int vdp_visendline;
unsigned int vdp_totlines;
unsigned int vdp_framerate;
unsigned int vdp_clock;
unsigned int vdp_68kclock;
unsigned int vdp_clksperline_68k;
unsigned int vdp_line = 0;      /* current line number */
uint8 vdp_oddframe = 0;  /* odd/even frame */
uint8 vdp_vblank = 0;    /* set during vertical blanking */
uint8 vdp_hblank = 0;    /* set during horizontal blanking */
uint8 vdp_vsync = 0;     /* a vsync just happened */
uint8 vdp_dmabusy = 0;   /* dma busy flag */
uint8 vdp_layerB = 1;    /* flag */
uint8 vdp_layerBp = 1;   /* flag */
uint8 vdp_layerA = 1;    /* flag */
uint8 vdp_layerAp = 1;   /* flag */
uint8 vdp_layerW = 1;    /* flag */
uint8 vdp_layerWp = 1;   /* flag */
uint8 vdp_layerH = 1;    /* flag */
uint8 vdp_layerS = 1;    /* flag */
uint8 vdp_layerSp = 1;   /* flag */
uint8 vdp_cram[LEN_CRAM];
uint8 vdp_vsram[LEN_VSRAM];
uint8 vdp_vram[LEN_VRAM];
uint8 vdp_cramf[LEN_CRAM / 2];
unsigned int vdp_event_start;
unsigned int vdp_event_vint;
unsigned int vdp_event_hint;
unsigned int vdp_event_hdisplay;
unsigned int vdp_event_end;
signed int vdp_nextevent = 0;
sint32 vdp_dmabytes = 0;        /* bytes left in DMA - must be fixed size */
signed int vdp_hskip_countdown = 0;     /* actual countdown */
uint16 vdp_address;             /* address for data/dma transfers */
t_code vdp_code;                /* code number for data/dma transfers CD3-CD0 */
uint8 vdp_ctrlflag;             /* set inbetween ctrl writes */
uint16 vdp_first;               /* first word of address set */
uint16 vdp_second;              /* second word of address set */

/*** global variables ***/

uint8 vdp_reg[25];
static int vdp_collision;       /* set during a sprite collision */
static int vdp_overflow;        /* set when too many sprites in one line */
static int vdp_fifofull;        /* set when write fifo full */
static int vdp_fifoempty;       /* set when write fifo empty */
static int vdp_complex;         /* set when simple routines can't cope */

/*** forward references ***/

void vdp_ramcopy_vram(int type);
void vdp_dma_vramcopy(void);
void vdp_dma_fill(uint8 data);
void vdp_showregs(void);
void vdp_describe(void);
void vdp_eventinit(void);
void vdp_layer_simple(unsigned int layer, unsigned int priority,
                      uint8 *fielddata, unsigned int lineoffset);
inline void vdp_plotcell(uint8 *patloc, uint8 palette, uint8 flags,
                         uint8 *cellloc, unsigned int lineoffset);
void vdp_sprites(unsigned int line, uint8 *pridata, uint8 *outdata);
int vdp_sprite_simple(unsigned int priority, uint8 *framedata,
                      unsigned int lineoffset, unsigned int number,
                      uint8 *spritelist, uint8 *sprite);
void vdp_sprites_simple(unsigned int priority, uint8 *framedata,
                        unsigned int lineoffset);
void vdp_shadow_simple(uint8 *framedata, unsigned int lineoffset);
void vdp_newlayer(unsigned int line, uint8 *pridata, uint8 *outdata,
                  unsigned int layer);
void vdp_newwindow(unsigned int line, uint8 *pridata, uint8 *outdata);

#define PRIBIT_LAYERB 0
#define PRIBIT_LAYERA 1
#define PRIBIT_SPRITE 2

/*** vdp_init - initialise this sub-unit ***/

int vdp_init(void)
{
  vdp_reset();
  return 0;
}

/*** vdp_setupvideo - setup parameters dependant on vdp_pal ***/

void vdp_setupvideo(void)
{
  int v30 = (vdp_reg[1] & 1 << 3) ? 1 : 0;

  if (!vdp_pal && v30)
    ui_err("Impossible VDP mode - vertical 30 cell NTSC");

  /* What speed is the PAL clock? */
  vdp_clock = vdp_pal ? 53200000 : 53693100;
  vdp_68kclock = vdp_clock / 7;
  vdp_vislines = v30 ? 240 : 224;
  vdp_visstartline = v30 ? 46 : (vdp_pal ? 54 : 19);
  vdp_visendline = vdp_visstartline + vdp_vislines;
  vdp_totlines = vdp_pal ? 312 : 262;
  vdp_framerate = vdp_pal ? 50 : 60;
  vdp_clksperline_68k = (vdp_68kclock / vdp_framerate / vdp_totlines);
}

/*** vdp_softreset - soft reset ***/

void vdp_softreset(void)
{
  /* a soft reset involves resetting the cpu so we need to reset the
     vdp event timers */
  vdp_eventinit();
}

/*** vdp_reset - reset vdp sub-unit ***/

void vdp_reset(void)
{
  int i;

  /* clear registers */

  for (i = 0; i < 25; i++)
    vdp_reg[i] = 0;

  /* set PAL/NTSC variables */

  vdp_setupvideo();
  vdp_vblank = 0;
  vdp_hblank = 0;
  vdp_oddframe = 0;
  vdp_collision = 0;
  vdp_overflow = 0;
  vdp_vsync = 0;
  vdp_fifofull = 0;
  vdp_fifoempty = 1;	// BUGFIX: Set FIFO empty bit to always be 1, since some games wait for FIFO empty
  vdp_ctrlflag = 0;
  vdp_first = 0;
  vdp_second = 0;
  vdp_dmabytes = 0;
  vdp_address = 0;

  memset(vdp_cram, 0, LEN_CRAM);
  memset(vdp_vsram, 0, LEN_VSRAM);
  memset(vdp_vram, 0, LEN_VRAM);

  /* clear CRAM */

  for (i = 0; i < 64; i++) {
    (vdp_cram + i * 2)[0] = (i & 7) << 1;
    (vdp_cram + i * 2)[1] = (i & 7) << 5 | (i & 7) << 1;
    vdp_cramf[i] = 1;
  }
  vdp_eventinit();
  LOG_VERBOSE(("VDP: totlines = %d (%s)", vdp_totlines, vdp_pal ?
               "PAL" : "NTSC"));
}

uint16 vdp_status(void)
{
  uint16 ret;

  /* bit      meaning (when set)
   * 0        0:ntsc 1:pal
   * 1        dma busy
   * 2        during h blanking
   * 3        during v blanking
   * 4        frame in interlace mode - 0:even 1:odd
   * 5        collision happened between non-zero pixels in two sprites
   * 6        too many sprites in one line
   * 7        v interrupt has happened
   * 8        write fifo full
   * 9        write fifo empty
   * 10-15 are next word on bus, i.e. next word in ROM - CM
   */
  ret = vdp_pal | vdp_dmabusy << 1 | vdp_hblank << 2 | vdp_vblank << 3 |
    vdp_oddframe << 4 | vdp_collision << 5 | vdp_overflow << 6 | vdp_vsync <<
    7 | vdp_fifofull << 8 | vdp_fifoempty << 9;

#ifdef DEBUG_VDP
  if (vdp_collision)
    LOG_VERBOSE(("%08X Collision read %d", regs.pc, vdp_collision));
#endif

  vdp_vsync = vdp_collision = vdp_overflow = 0;
  vdp_ctrlflag = 0;             /* Charles MacDonald - so he claims ;) */

  LOG_DEBUG1(("%08X STATUS READ %02X", regs.pc, ret));
  LOG_VERBOSE(("%08X STATUS READ %02X", regs.pc, ret));

  return (ret);
}

void vdp_storectrl(uint16 data)
{
  uint8 reg;
  uint8 regdata;

#ifdef DEBUG_VDP
  LOG_VERBOSE(("%08X [VDP] Ctrl write of %04X (vdp_ctrlflag before=%d)",
               regs.pc, data, vdp_ctrlflag));
#endif

  if (!vdp_ctrlflag) {
    if ((data & 0xE000) == 0x8000) {
      /* register set */
      reg = (data >> 8) & 31;
      regdata = data & 255;
      if (reg > 24) {
        LOG_NORMAL(("%08X [VDP] Invalid register (%d)", regs.pc,
                    ((data >> 8) & 31)));
        return;
      }
      vdp_reg[reg] = regdata;
      vdp_code = 0;
#ifdef DEBUG_VDP
      LOG_VERBOSE(("%08X [VDP] Register %d set to %04X", regs.pc,
                   reg, regdata));
#endif
      return;
    } else {
      vdp_ctrlflag = 1;
      vdp_first = data;
      return;
    }
  } else {
    vdp_second = data;
    vdp_ctrlflag = 0;
    vdp_code = ((vdp_first >> 14) & 3) | ((data >> 2) & (3 << 2));
    vdp_address = (vdp_first & 0x3FFF) | (data << 14 & 0xC000);

#ifdef DEBUG_VDP
    LOG_VERBOSE(("%08X [VDP] Ctrl: %08X; code=%d address=%X", regs.pc,
                 (vdp_first << 16) | vdp_second, vdp_code, vdp_address));
#endif

    if ((data & 1 << 7) && (vdp_reg[1] & 1 << 4)) {     /* CD5 - DMA ? */
      if (vdp_dmabusy) {
        vdp_dmabusy = 1;        /* null statement to avoid gcc warnings */
        LOG_DEBUG1(("DMA initiation during DMA!"));
      }
      /* CD4 - not read - need to verify */
      vdp_dmabusy = 1;
      switch ((vdp_reg[23] >> 6) & 3) {
      case 0:
      case 1:                  /* ram to vram */
        switch (vdp_code) {
        case 1:                /* ram copy to vram */
          vdp_ramcopy_vram(0);
          break;
        case 3:                /* ram copy to cram */
          vdp_ramcopy_vram(1);
          break;
        case 5:                /* ram copy to vsram */
          vdp_ramcopy_vram(2);
          break;
        default:               /* undefined */
          LOG_NORMAL(("%08X [VDP] start of type %d to address %X",
                      regs.pc, vdp_code, vdp_address));
          break;
        }
        vdp_dmabusy = 0;        /* 68k was frozen */
        break;
      case 2:                  /* VRAM fill */
        /* later folks */
        break;
      case 3:                  /* VRAM copy */
        vdp_dma_vramcopy();
        /* vdp_dmabusy is cleared when vdp_dmabytes is empty */
        break;
      }
    }
  }
}

void vdp_ramcopy_vram(int type)
{
  uint16 length = vdp_reg[19] | vdp_reg[20] << 8;
  uint8 srcbank = vdp_reg[23];
  uint16 srcoffset = vdp_reg[21] | vdp_reg[22] << 8;
  uint8 increment = vdp_reg[15];
  uint16 *srcmemory;
  uint16 srcmask;
  uint16 data;
  unsigned int i;

#ifdef DEBUG_VDP
  LOG_VERBOSE(("%08X [VDP] VRAM copy from source %08X "
               "vdpaddr=%08X length=%d (%s)",
               regs.pc, (srcbank * 0x10000 + srcoffset) * 2, vdp_address,
               length,
               type == 0 ? "vram" :
               type == 1 ? "cram" : type == 2 ? "vsram" : "??"));
#endif

  if (srcbank & 1 << 6) {
    srcmemory = (uint16 *)cpu68k_ram;
    srcmask = 0x7fff;           /* 32k words = 64k */
  } else {
    srcmemory = (uint16 *)(cpu68k_rom + srcbank * 0x20000);
    srcmask = 0xffff;           /* 64k words = 128k */
  }
  for (i = 0; i < length; i++) {
    data = LOCENDIAN16(srcmemory[srcoffset & srcmask]);
    switch (type) {
    case 0:                    /* VRAM */
      vdp_vram[vdp_address] = data >> 8;
      vdp_vram[vdp_address ^ 1] = data & 0xff;
      break;
    case 1:                    /* CRAM */
      vdp_cram[vdp_address & 0x7e] = data >> 8;
      vdp_cram[(vdp_address & 0x7e) | 1] = data & 0xff;
      vdp_cramf[(vdp_address & 0x7e) >> 1] = 1;
#ifdef DEBUG_VDPCRAM
      LOG_VERBOSE(("%08X CRAM %X = %04X", regs.pc, vdp_address >> 1, data));
#endif
      break;
    case 2:                    /* VSRAM */
      if ((vdp_address & 0x7e) < LEN_VSRAM) {
        vdp_vsram[vdp_address & 0x7e] = data >> 8;
        vdp_vsram[(vdp_address & 0x7e) | 1] = data & 0xff;
      }
      break;
    }
    srcoffset += 1;
    vdp_address += increment;
  }
  vdp_reg[19] = 0;
  vdp_reg[20] = 0;
  vdp_reg[22] = (srcoffset >> 8) & 0xff;
  vdp_reg[21] = srcoffset & 0xff;
  /* vram sends bytes, cram/vsram send words so are twice as efficient */
  event_freeze(type == 0 ? length * 2 : length);
}

void vdp_dma_vramcopy()
{
  uint32 length = vdp_reg[19] | vdp_reg[20] << 8;
  uint8 increment = vdp_reg[15];
  uint16 srcaddr = vdp_reg[21] | vdp_reg[22] << 8;
  unsigned int i;

  if (length == 0) {
    LOG_NORMAL(("%08X [VDP] Warning - length of 0 used in vram copy"));
    length = 0x10000;           /* could be 0xffff */
  }
#ifdef DEBUG_VDPDMA
  LOG_VERBOSE(("%08X [VDP] COPY length %04X dstaddr %08X inc %02X "
               "srcaddr %04X", regs.pc, length, vdp_address, increment,
               srcaddr));
#endif

  for (i = 0; i < length; i++) {
    vdp_vram[vdp_address] = vdp_vram[srcaddr++];
    vdp_address += increment;
  }

  vdp_reg[19] = 0;
  vdp_reg[20] = 0;
  vdp_reg[22] = (srcaddr >> 8) & 0xff;
  vdp_reg[21] = srcaddr & 0xff;
  vdp_dmabytes = length * 2;    /* factor of 2 vram copy to vram fill (p36) */
}

/*** vdp_dma_fill - implement the DMA part of the fill operation - note
     that the low byte of the 16 bit word has already been written in the
     non-dma stage ***/

void vdp_dma_fill(uint8 data)
{
  uint16 length = vdp_reg[19] | vdp_reg[20] << 8;
  uint8 increment = vdp_reg[15];
  unsigned int i;

  if (increment != 1 && increment != 2 && increment != 4)
    LOG_NORMAL(("VDP fill used with strange increment %d", increment));

  for (i = 0; i < length; i++) {
    vdp_vram[vdp_address ^ 1] = data;
    vdp_address += increment;   /* 16 bit wrap */
  }
  vdp_reg[19] = 0;
  vdp_reg[20] = 0;
  vdp_dmabytes = length + 1;    /* extra byte used (see p36) */
}

void vdp_storedata(uint16 data)
{
  uint16 address;
  uint16 sdata;

  if (vdp_ctrlflag) {
    LOG_NORMAL(("%08X [VDP] Unterminated ctrl setting %04X/%04X", regs.pc,
                vdp_first, vdp_second));
    vdp_storectrl(vdp_second);
  }
#ifdef DEBUG_VDPDATA
  LOG_NORMAL(("%08X [VDP] code=%d (%s) data=%04X addr=%X inc=%d",
              regs.pc, vdp_code,
              vdp_code == cd_vram_store ? "vram" :
              vdp_code == cd_cram_store ? "cram" :
              vdp_code == cd_vsram_store ? "vsram" : "??",
              data, vdp_address, vdp_reg[15]));
#endif
  switch (vdp_code) {
  case cd_vram_store:
    sdata = (vdp_address & 1) ? SWAP16(data) : data;    /* only for VRAM */
    *(uint16 *)(vdp_vram + (vdp_address & 0xfffe)) = LOCENDIAN16(sdata);
    break;
  case cd_cram_store:
    address = vdp_address & 0x7e;       /* address lines used */
    *(uint16 *)(vdp_cram + address) = LOCENDIAN16(data);
    vdp_cramf[address >> 1] = 1;
    break;
  case cd_vsram_store:
    address = vdp_address & 0x7e;       /* address lines used */
    if (address < LEN_VSRAM)
      *(uint16 *)(vdp_vsram + address) = LOCENDIAN16(data);
    break;
  default:                     /* undefined */
    LOG_NORMAL(("%08X [VDP] Bad word store to %08X of type %d data = %04X",
                regs.pc, vdp_address, vdp_code, data));
    break;
  }
  vdp_address += vdp_reg[15];   /* 16 bit wrap */

  /* note fall-through from normal write to DMA initiation - this is
     correct operation */

  if (vdp_dmabusy) {
    if (vdp_code == 1)
      /* other vdp_codes consume time but don't result in fills */
      /* vdp_dmabusy is cleared when vdp_dmabytes is empty */
      vdp_dma_fill((data >> 8) & 0xff);
  }
}

uint16 vdp_fetchdata(void)
{
  uint16 address;
  uint16 data;

  if (vdp_ctrlflag) {
    LOG_NORMAL(("%08X [VDP] Unterminated ctrl setting %04X/%04X", regs.pc,
                vdp_first, vdp_second));
    vdp_storectrl(vdp_second);
  }

  switch (vdp_code) {
  case cd_vram_fetch:
    data = LOCENDIAN16(*(uint16 *)(vdp_vram + (vdp_address & 0xfffe)));
    break;
  case cd_cram_fetch:
    address = vdp_address & 0x7e;       /* address lines used */
    data = LOCENDIAN16(*(uint16 *)(vdp_cram + address));
    break;
  case cd_vsram_fetch:
    address = vdp_address & 0x7e;       /* address lines used */
    if (address < LEN_VSRAM)
      data = LOCENDIAN16(*(uint16 *)(vdp_vsram + address));
    else
      data = 0;                 /* tests show this range appears random
                                   (although I'm sure it isn't) */
    break;
  default:                     /* undefined */
    /* reading in write mode suspends 68000 on a real machine */
    LOG_NORMAL(("%08X [VDP] Bad word fetch of %08X of type %d",
                regs.pc, vdp_address, vdp_code));
    data = 0;
    break;
  }
  vdp_address += vdp_reg[15];   /* 16 bit wrap */
  return data;
}

#define LINEDATA(offset, value, palette, priority) \
if (value) { \
  linedata[offset] = (palette)*16 + value; \
} else if (priority) { \
  linedata[offset] = (linedata[offset] & 63); \
}

#define LINEDATASPR(offset, value, palette, priority) \
if (value) { \
  if (outdata[offset] < 62) \
    vdp_collision = 1; \
  if (priority) \
    pridata[offset]|= 1<<PRIBIT_SPRITE; \
  else \
    pridata[offset]&= ~(1<<PRIBIT_SPRITE); \
  outdata[offset] = (palette)*16 + value; \
}

#define LINEDATALAYER(offset, value, palette, priority) \
  if (priority) \
    pridata[offset]|= layer ? 1<<PRIBIT_LAYERB : 1<<PRIBIT_LAYERA; \
  if (value) \
    outdata[offset] = (palette)*16 + value; \
  else \
    outdata[offset] = 0;

void vdp_sprites(unsigned int line, uint8 *pridata, uint8 *outdata)
{
  uint8 interlace = (vdp_reg[12] >> 1) & 3;
  uint8 *spritelist = vdp_vram + ((vdp_reg[5] & 0x7F) << 9);
  t_spriteinfo si[128];         /* 128 - max sprites supported per line */
  unsigned int sprites;
  unsigned int idx;
  int i;
  uint8 link;
  uint8 *sprite = spritelist;
  int plotter;                  /* flag */
  unsigned int screencells = (vdp_reg[12] & 1) ? 40 : 32;
  unsigned int maxspl = (vdp_reg[12] & 1) ? 20 : 16;    /* max sprs/line */
  unsigned int cells;
  uint8 loops = 0;

  for (idx = 0; loops < 255 && idx < maxspl; loops++) {
    link = sprite[3] & 0x7F;
    si[idx].sprite = sprite;
    if (interlace == 3)
      si[idx].vpos = (LOCENDIAN16(*(uint16 *)(sprite)) & 0x3FF) - 0x100;
    else
      si[idx].vpos = (LOCENDIAN16(*(uint16 *)(sprite)) & 0x1FF) - 0x080;
    if ((signed int)line < si[idx].vpos)
      goto next;
    si[idx].vsize = 1 + (sprite[2] & 3);
    if (interlace == 3)
      si[idx].vsize <<= 1;
    si[idx].vmax = si[idx].vpos + si[idx].vsize * 8;
    if ((signed int)line >= si[idx].vmax)
      goto next;
    si[idx].hpos = (LOCENDIAN16(*(uint16 *)(sprite + 6)) & 0x1FF) - 0x80;
    si[idx].hsize = 1 + ((sprite[2] >> 2) & 3);
    si[idx].hplot = si[idx].hsize;
    si[idx].hmax = si[idx].hpos + si[idx].hsize * 8;
    idx++;
  next:
    if (!link)
      break;
    sprite = spritelist + (link << 3);

  }
  if (idx < 1)
    return;
  sprites = idx;
  plotter = 1;
  cells = (vdp_reg[12] & 1) ? 40 : 32;  /* 320 or 256 pixels */
  /* loop masking */
  for (i = 0; i < (signed int)sprites; i++) {
    if (plotter == 0) {
      sprites = i;
      break;
    }
    if (si[i].hpos == -128) {
      /* mask sprite - but does it? */
      if (i > 0) {
        /* is there a higher priority sprite? */
        if (si[i - 1].vpos <= (signed int)line &&
            si[i - 1].vmax > (signed int)line) {
          /* match, mask time */
          plotter = 0;
        }
      } else {
        /* higest priority sprite, so mask */
        plotter = 0;
      }
      si[i].hplot = 0;
    }
    if (si[i].hpos == -127) {
      LOG_VERBOSE(("Warning: Use of hpos = 1 in plotter"));
      plotter = 1;
    }
    if (cells >= si[i].hplot) {
      cells -= si[i].hplot;
    } else {
      si[i].hplot = cells;      /* only room for this many */
      cells = 0;
    }
  }

  {
    sint16 hpos, vpos;
    uint16 hsize, vsize, hplot;
    sint16 hmax, vmax;
    uint16 cellinfo;
    uint16 pattern;
    uint8 palette;
    uint8 *cellline;
    uint32 data;
    uint8 pixel;
    uint8 priority;
    int j, k;

    /* loop around sprites until end of list marker or no more */
    for (i = sprites - 1; i >= 0; i--) {
      hpos = si[i].hpos;
      vpos = si[i].vpos;
      vsize = si[i].vsize;      /* doubled when in interlace mode */
      hsize = si[i].hsize;
      hmax = si[i].hmax;
      vmax = si[i].vmax;
      hplot = si[i].hplot;      /* number of cells to plot for this sprite */
      cellinfo = LOCENDIAN16(*(uint16 *)(si[i].sprite + 4));
      pattern = cellinfo & 0x7FF;
      palette = (cellinfo >> 13) & 3;
      priority = (cellinfo >> 15) & 1;

      cellline =
        vdp_vram + ((interlace == 3) ? (pattern << 6) : (pattern << 5));
      if (cellinfo & 1 << 12)   /* vertical flip */
        cellline += (vmax - line - 1) * 4;
      else
        cellline += (line - vpos) * 4;
      for (k = 0; k < hsize && hplot--; k++) {
        if (hpos > -8 && hpos < 0) {
          if (cellinfo & 1 << 11) {
            /* horizontal flip */
            data = LOCENDIAN32(*(uint32 *)(cellline +
                                           (hsize - k * 2 -
                                            1) * (vsize << 5)));
            data >>= (-hpos) * 4;       /* get first pixel in bottom 4 bits */
            for (j = 0; j < 8 + hpos; j++) {
              pixel = data & 15;
              LINEDATASPR(j, pixel, palette, priority);
              data >>= 4;
            }
          } else {
            data = LOCENDIAN32(*(uint32 *)cellline);
            data <<= (-hpos) * 4;       /* get first pixel in top 4 bits */
            for (j = 0; j < 8 + hpos; j++) {
              pixel = (data >> 28) & 15;
              LINEDATASPR(j, pixel, palette, priority);
              data <<= 4;
            }
          }
        } else if (hpos >= 0 && hpos <= (signed int)(screencells - 1) * 8) {
          if (cellinfo & 1 << 11) {
            /* horizontal flip */
            data = LOCENDIAN32(*(uint32 *)(cellline +
                                           (hsize - k * 2 -
                                            1) * (vsize << 5)));
            for (j = 0; j < 8; j++) {
              pixel = data & 15;
              LINEDATASPR(hpos + j, pixel, palette, priority);
              data >>= 4;
            }
          } else {
            data = LOCENDIAN32(*(uint32 *)cellline);
            for (j = 0; j < 8; j++) {
              pixel = (data >> 28) & 15;
              LINEDATASPR(hpos + j, pixel, palette, priority);
              data <<= 4;
            }
          }
        } else if (hpos > (signed int)(screencells - 1) * 8 &&
                   hpos < (signed int)screencells * 8) {
          if (cellinfo & 1 << 11) {
            /* horizontal flip */
            data = LOCENDIAN32(*(uint32 *)(cellline +
                                           (hsize - k * 2 -
                                            1) * (vsize << 5)));
            for (j = 0; j < (signed int)(screencells * 8 - hpos); j++) {
              pixel = data & 15;
              LINEDATASPR(hpos + j, pixel, palette, priority);
              data >>= 4;
            }
          } else {
            data = LOCENDIAN32(*(uint32 *)cellline);
            for (j = 0; j < (signed int)(screencells * 8 - hpos); j++) {
              pixel = (data >> 28) & 15;
              LINEDATASPR(hpos + j, pixel, palette, priority);
              data <<= 4;
            }
          }
        }
        cellline += vsize << 5; /* 32 bytes per cell (note vsize is doubled
                                   when interlaced) */
        hpos += 8;
      }
    }
  }
}

/* layer 0 = A (top), layer 1 = B (bottom) */

void vdp_newlayer(unsigned int line, uint8 *pridata, uint8 *outdata,
                  unsigned int layer)
{
  int i, j;
  uint8 hsize = vdp_reg[16] & 3;
  uint8 vsize = (vdp_reg[16] >> 4) & 3;
  uint8 hmode = vdp_reg[11] & 3;
  uint8 vmode = (vdp_reg[11] >> 2) & 1;
  uint16 vramoffset = (layer ? ((vdp_reg[4] & 7) << 13) :
                       ((vdp_reg[2] & (7 << 3)) << 10));
  uint16 *patterndata = (uint16 *)(vdp_vram + vramoffset);
  uint16 *hscrolldata = (uint16 *)(((vdp_reg[13] & 63) << 10)
                                   + vdp_vram + layer * 2);
  uint8 screencells = (vdp_reg[12] & 1) ? 40 : 32;
  uint16 hwidth, vwidth, vmask, hoffset, voffset;
  uint16 cellinfo;
  uint8 *pattern;
  uint32 data;
  uint8 palette;
  uint8 pixel;
  uint8 priority;
  int interlace = (((vdp_reg[12] >> 1) & 3) == 3) ? 1 : 0;
  int realline = interlace ? line >> 1 : line;
  int column;

  /* window stuff */
  int vcell = realline >> 3;
  int topbottom = vdp_reg[18] & 0x80;
  int winvpos = vdp_reg[18] & 0x1f;
  int leftright = vdp_reg[17] & 0x80;
  int winhpos = vdp_reg[17] & 0x1f;
  int corrupted = 0;

  if (layer == 0) {

    /* sometimes the window covers the whole line, so we skip layer A for
       that bit - for lines where layer A appears even for just one cell we
       plot the entire line - would it be worth improving this? */

    if (topbottom ? (vcell >= winvpos) : (vcell < winvpos))
      return;                   /* window is on the whole line */

    /* it could be that the vdp is in such a state that there is no whole line
       part to the window, but reg[17] is either 00 or 1F, meaning that
       infact there is (this is one way of turning on/off window) in which
       case we really should optimise this case otherwise we could be
       drawing layer A pointlessly as window will just clobber all our work) */

    if (leftright ? (winhpos == 0) : (winhpos >= (screencells >> 1)))
      return;                   /* window is on the whole line */
  }

  /* select horizontal scrolling offset based on mode */

  switch (hmode) {
  case 0:                      /* full screen */
    hoffset = (0x400 - LOCENDIAN16(hscrolldata[0])) & 0x3FF;
    break;
  case 1:                      /* line scroll with first 8 lines */
    hoffset = (0x400 - LOCENDIAN16(hscrolldata[2 * (realline & 7)])) & 0x3FF;
    break;
  case 2:                      /* cell scroll */
    hoffset = (0x400 - LOCENDIAN16(hscrolldata[2 * (realline & ~7)])) & 0x3FF;
    break;
  case 3:                      /* line scroll */
    hoffset = (0x400 - LOCENDIAN16(hscrolldata[2 * realline])) & 0x3FF;
    break;
  default:
    hoffset = 0;
  }

  /* vsram works in two-column lumps, so if hoffset & 15 is 1-8 then
     the first of our columns is going to use a fixed hoffset of 0,
     if it's 9-15 then the first two of our columns are going to use
     a fixed hoffset of 0 - pengo, air diver */

  /* what column are we going to do first?  -2, -1 or 0 */

  column = (hoffset & 15) == 0 ? 0 : ((hoffset & 15) > 8 ? -2 : -1);

  /* to implement the bug in the VDP's window code, we check to see if
     the window is on the left (leftright is 0), that there really is a
     window (winhpos not 0, winhpos < max is checked earlier) and that
     there is a non-aligned horizontal offset - then we adjust the number
     of cells to plot (screencells), skip over the data we're not going
     to plot (outdata, pridata) and temporarily increase the hoffset */

  if (layer == 0 && !leftright && winhpos && (hoffset & 15)) {
    screencells -= winhpos;
    hoffset += 16;              /* this is the corruption */
    hoffset += winhpos * 16;
    outdata += winhpos * 16;
    pridata += winhpos * 16;
    corrupted = (hoffset & 15) >= 8 ? 1 : 2;    /* one lump or two? */
  }
  vwidth = ((vsize + 1) << 5) * (interlace + 1);
  vmask = (vwidth << 3) - 1;    /* to put offsets into range */
  hwidth = (hsize + 1) << 5;
  /* if 2-cell mode and vsram not valid yet, use 0 as the offset */
  voffset = ((vmode && column < 0) ? 0 :
             LOCENDIAN16(((uint16 *)vdp_vsram)[layer]));
  voffset = (voffset + line) & vmask;
  hoffset &= (hwidth << 3) - 1; /* put offset in range */
  if (hsize == 2) {
    /* hsize=2 is special - only use top row in table */
    voffset &= 7;
    hwidth = 32;
  }
  cellinfo = LOCENDIAN16(patterndata[(hoffset >> 3) +
                                     hwidth *
                                     ((voffset >> 3) >> interlace)]);
  /* 32 bytes per pattern or 64 in interlace mode 2 */
  pattern = vdp_vram + (((cellinfo & 2047) << 5) << interlace );
  /* now get correct line from pattern data */
  if (interlace) {
    /* interlace - double height cells */
    if (cellinfo & 1 << 12) {
      /* vertical flip */
      pattern += 4 * (15 - (voffset & 15));
    } else {
      /* no vertical flip */
      pattern += 4 * (voffset & 15);
    }
  } else {
    /* no interlace */
    if (cellinfo & 1 << 12) {
      /* vertical flip */
      pattern += 4 * (7 - (voffset & 7));
    } else {
      /* no vertical flip */
      pattern += 4 * (voffset & 7);
    }
  }
  priority = (cellinfo >> 15) & 1;
  palette = (cellinfo >> 13) & 3;
  data = LOCENDIAN32(*(uint32 *)pattern);
  if (cellinfo & 1 << 11) {
    /* horizontal flip */
    data >>= (hoffset & 7) * 4; /* get first pixel in bottom 4 bits */
    for (i = 0; i < 8 - (hoffset & 7); i++) {
      pixel = data & 15;
      LINEDATALAYER(i, pixel, palette, priority);
      data >>= 4;
    }
  } else {
    data <<= (hoffset & 7) * 4; /* get first pixel in top 4 bits */
    for (i = 0; i < 8 - (hoffset & 7); i++) {
      pixel = (data >> 28) & 15;
      LINEDATALAYER(i, pixel, palette, priority);
      data <<= 4;
    }
  }
  outdata += 8 - (hoffset & 7);
  pridata += 8 - (hoffset & 7);
  hoffset += 8;
  if (--corrupted == 0)
    hoffset -= 16;              /* turn off corruption */
  hoffset &= (hwidth << 3) - 1; /* put offset in range */
  column++;
  for (j = 1; j < screencells; j++) {
    if (vmode) {
      /* 2-cell scroll */
      if (column >= 0)
        voffset = LOCENDIAN16(((uint16 *)vdp_vsram)[(column & ~1) + layer]);
      else
        voffset = 0;
    } else {
      /* full screen */
      voffset = LOCENDIAN16(((uint16 *)vdp_vsram)[layer]);
    }
    voffset = (voffset + line) & vmask;
    if (hsize == 2)
      /* hsize=2 is special - only use top row in table */
      voffset &= 7;
    cellinfo = LOCENDIAN16(patterndata[(hoffset >> 3) +
                                       hwidth *
                                       ((voffset >> 3) >> interlace)]);
    priority = (cellinfo >> 15) & 1;
    /* printf("hoff: %04X voff: %04X hwid: %04X cell: %08X info: %04X\n",
       hoffset, voffset, hwidth, (hoffset>>3)+hwidth*(voffset>>3),
       cellinfo); */
    /* printf("loc %08X cellinfo %08X\n",
       (hoffset>>3)+hwidth*(voffset>>3),
       cellinfo); */
    palette = (cellinfo >> 13) & 3;
    /* 32 bytes per pattern or 64 in interlace mode 2 */
    pattern = vdp_vram + (((cellinfo & 2047) << 5) << interlace );
    /* now get correct line from pattern data */
    if (interlace) {
      /* interlace - double height cells */
      if (cellinfo & 1 << 12) {
        /* vertical flip */
        pattern += 4 * (15 - (voffset & 15));
      } else {
        /* no vertical flip */
        pattern += 4 * (voffset & 15);
      }
    } else {
      /* no interlace */
      if (cellinfo & 1 << 12) {
        /* vertical flip */
        pattern += 4 * (7 - (voffset & 7));
      } else {
        /* no vertical flip */
        pattern += 4 * (voffset & 7);
      }
    }
    data = LOCENDIAN32(*(uint32 *)pattern);
    if (cellinfo & 1 << 11) {
      /* horizontal flip */
      for (i = 0; i < 8; i++) {
        pixel = data & 15;
        LINEDATALAYER(i, pixel, palette, priority);
        data >>= 4;
      }
    } else {
      for (i = 0; i < 8; i++) {
        pixel = (data >> 28) & 15;
        LINEDATALAYER(i, pixel, palette, priority);
        data <<= 4;
      }
    }
    outdata += 8;
    pridata += 8;
    hoffset += 8;
    if (--corrupted == 0)
      hoffset -= 16;            /* turn off corruption */
    hoffset &= (hwidth << 3) - 1;       /* put offset in range */
    column++;
  }
  if (hoffset & 7) {
    if (vmode) {
      /* 2-cell scroll */
      if (column >= 0)
        voffset = LOCENDIAN16(((uint16 *)vdp_vsram)[(column & ~1) + layer]);
      else
        voffset = 0;
    } else {
      /* full screen */
      voffset = LOCENDIAN16(((uint16 *)vdp_vsram)[layer]);
    }
    voffset = (voffset + line) & vmask;
    if (hsize == 2)
      /* hsize=2 is special - only use top row in table */
      voffset &= 7;   
    cellinfo = LOCENDIAN16(patterndata[(hoffset >> 3) +
                                       hwidth *
                                       ((voffset >> 3) >> interlace)]);
    priority = (cellinfo >> 15) & 1;
    palette = (cellinfo >> 13) & 3;
    /* 32 bytes per pattern or 64 in interlace mode 2 */
    pattern = vdp_vram + (((cellinfo & 2047) << 5) << interlace);
    /* now get correct line from pattern data */
    if (interlace) {
      /* interlace - double height cells */
      if (cellinfo & 1 << 12) {
        /* vertical flip */
        pattern += 4 * (15 - (voffset & 15));
      } else {
        /* no vertical flip */
        pattern += 4 * (voffset & 15);
      }
    } else {
      /* no interlace */
      if (cellinfo & 1 << 12) {
        /* vertical flip */
        pattern += 4 * (7 - (voffset & 7));
      } else {
        /* no vertical flip */
        pattern += 4 * (voffset & 7);
      }
    }
    data = LOCENDIAN32(*(uint32 *)pattern);
    if (cellinfo & 1 << 11) {
      /* horizontal flip */
      for (i = 0; i < (hoffset & 7); i++) {
        pixel = data & 15;
        LINEDATALAYER(i, pixel, palette, priority);
        data >>= 4;
      }
    } else {
      for (i = 0; i < (hoffset & 7); i++) {
        pixel = (data >> 28) & 15;
        LINEDATALAYER(i, pixel, palette, priority);
        data <<= 4;
      }
    }
  }
}

/* this function updates outdata/pridata which came from the layerA generation
   routines */

void vdp_newwindow(unsigned int line, uint8 *pridata, uint8 *outdata)
{
  int interlace = (((vdp_reg[12] >> 1) & 3) == 3) ? 1 : 0;
  int realline = line >> interlace;
  int voffset = line;
  uint16 *patterndata = (uint16 *)(vdp_vram + ((vdp_reg[3] & 0x3e) << 10));
  uint8 winhpos = vdp_reg[17] & 0x1f;
  uint8 winvpos = vdp_reg[18] & 0x1f;
  uint8 vcell = realline / 8;
  uint8 hcell;
  uint8 screencells = (vdp_reg[12] & 1) ? 40 : 32;
  uint8 topbottom = vdp_reg[18] & 0x80;
  uint8 leftright = vdp_reg[17] & 0x80;
  uint8 patternshift = (vdp_reg[12] & 1) ? 6 : 5;
  uint16 cellinfo;
  uint8 *pattern;
  uint32 data;
  uint8 palette;
  uint8 pixel;
  unsigned int i;
  unsigned int wholeline = 0;
  uint8 priority;
  int layer = 0;                /* for LINEDATALAYER macro */

  /* if topbottom is set then the wholeline part of the window is to the
     bottom, if it is clear then it is to the top
     in the other part of the window, if leftright is set then the window
     is on the right in this part of the screen
     this section below is repeated in the main renderline code, so that
     layerA is not unnecessarily calculated */

  if (topbottom ? (vcell >= winvpos) : (vcell < winvpos))
    wholeline = 1;

  if (!wholeline) {
    if (winhpos >= 20)
      winhpos = 20;             /* 0x1F might be special? */
    if (leftright) {
      /* clear out priority bits on right of line (winhpos units of 16 pix */
      for (i = winhpos * 4; i < 320 / 4; i++)   /* winhpos units of 16 pix */
        ((uint32 *)pridata)[i] &=
          (0xffffffff - (0x01010101 << PRIBIT_LAYERA));
      memset(outdata + winhpos * 16, 0, 320 - (winhpos * 16));
    } else {
      /* clear out priority bits on left of line (winhpos units of 16 pix) */
      for (i = 0; i < (unsigned int)(winhpos / 4); i++)
        ((uint32 *)pridata)[i] &=
          (0xffffffff - (0x01010101 << PRIBIT_LAYERA));
      memset(outdata, 0, winhpos * 16);
    }
  }
  for (hcell = 0; hcell < screencells; hcell++, outdata += 8, pridata += 8) {
    if (!wholeline) {
      if (leftright) {
        if ((hcell >> 1) < winhpos)
          continue;
      } else {
        if ((hcell >> 1) >= winhpos)
          continue;
      }
    }
    cellinfo = LOCENDIAN16(patterndata[vcell << patternshift | hcell]);
    priority = (cellinfo >> 15) & 1;
    palette = (cellinfo >> 13) & 3;
    /* 32 bytes per pattern */
    pattern = vdp_vram + (((cellinfo & 2047) << 5) << interlace);
    /* now get correct line from pattern data */
    if (interlace) {
      /* interlace - double height cells */
      if (cellinfo & 1 << 12) {
        /* vertical flip */
        pattern += 4 * (15 - (voffset & 15));
      } else {
        /* no vertical flip */
        pattern += 4 * (voffset & 15);
      }
    } else {
      /* no interlace */
      if (cellinfo & 1 << 12) {
        /* vertical flip */
        pattern += 4 * (7 - (voffset & 7));
      } else {
        /* no vertical flip */
        pattern += 4 * (voffset & 7);
      }
    }
    data = LOCENDIAN32(*(uint32 *)pattern);
    if (cellinfo & 1 << 11) {
      /* horizontal flip */
      for (i = 0; i < 8; i++) {
        pixel = data & 15;
        LINEDATALAYER(i, pixel, palette, priority);
        data >>= 4;
      }
    } else {
      for (i = 0; i < 8; i++) {
        pixel = (data >> 28) & 15;
        LINEDATALAYER(i, pixel, palette, priority);
        data <<= 4;
      }
    }
  }                             /* hcell */
}

/*** vdp_renderline - render a line of a field ***/

/* line = field line (0 to 223)
   linedata = buffer to put the output data (console colours: 0-191)
   odd = whether this is an odd field or not (fields are 0 or 1, therefore
                                              odd is the second one in a pair)
   call with odd=0 at all times when not in interlace mode 2
 */

void vdp_renderline(unsigned int line, uint8 *linedata, unsigned int odd)
{
  int i;
  uint8 datablock[320 * 4];
  uint8 *data_sprite = datablock;
  uint8 *data_layerA = datablock + 320;
  uint8 *data_layerB = datablock + 320 * 2;
  uint8 *priorities = datablock + 320 * 3;
  uint8 bg = vdp_reg[7] & 63;
  unsigned int interlace = (((vdp_reg[12] >> 1) & 3) == 3) ? 1 : 0;

  memset(datablock, 0, sizeof(datablock));

  if ((vdp_reg[1] & 1 << 6) == 0) {
    /* screen is disabled */
    for (i = 0; i < 320; i++)
      linedata[i] = bg;
    return;
  }

  if (vdp_layerS || vdp_layerSp)
    vdp_sprites(interlace ? (line * 2 + odd) : line, priorities, data_sprite);
  if (vdp_layerA || vdp_layerAp) {
    vdp_newlayer(interlace ? (line * 2 + odd) : line, priorities,
                 data_layerA, 0);
    if (vdp_layerW || vdp_layerWp)
      vdp_newwindow(interlace ? (line * 2 + odd) : line, priorities,
                    data_layerA);
  }
  if (vdp_layerB || vdp_layerBp)
    vdp_newlayer(interlace ? (line * 2 + odd) : line, priorities,
                 data_layerB, 1);

  for (i = 0; i < 320; i++) {
    switch (priorities[i] | (vdp_reg[12] & 1 << 3)) {
    case 0:                    /* s/ten=0, B=0, A=0, S=0 */
      if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else
        linedata[i] = bg;
      break;
    case 1:                    /* s/ten=0, B=1, A=0, S=0 */
      if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else
        linedata[i] = bg;
      break;
    case 2:                    /* s/ten=0, B=0, A=1, S=0 */
      if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else
        linedata[i] = bg;
      break;
    case 3:                    /* s/ten=0, B=1, A=1, S=0 */
      if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else
        linedata[i] = bg;
      break;
    case 4:                    /* s/ten=0, B=0, A=0, S=1 */
      if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else
        linedata[i] = bg;
      break;
    case 5:                    /* s/ten=0, B=1, A=0, S=1 */
      if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else
        linedata[i] = bg;
      break;
    case 6:                    /* s/ten=0, B=0, A=1, S=1 */
      if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else
        linedata[i] = bg;
      break;
    case 7:                    /* s/ten=0, B=1, A=1, S=1 */
      if (data_sprite[i])
        linedata[i] = data_sprite[i];
      else if (data_layerA[i])
        linedata[i] = data_layerA[i];
      else if (data_layerB[i])
        linedata[i] = data_layerB[i];
      else
        linedata[i] = bg;
      break;
    case 8:                    /* s/ten=1, B=0, A=0, S=0 */
      if (data_sprite[i]) {
        if (data_sprite[i] == 63) {     /* shadow operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i] | 128; /* shadow */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i] | 128; /* shadow */
          else
            linedata[i] = bg | 128;     /* shadow */
        } else if (data_sprite[i] == 62) {      /* highlight operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i];       /* normal */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i];       /* normal */
          else
            linedata[i] = bg;   /* normal */
        } else {
          linedata[i] = data_sprite[i] | 128;   /* shadow */
        }
      } else {
        if (data_layerA[i])
          linedata[i] = data_layerA[i] | 128;   /* shadow */
        else if (data_layerB[i])
          linedata[i] = data_layerB[i] | 128;   /* shadow */
        else
          linedata[i] = bg | 128;       /* shadow */
      }
      break;
    case 9:                    /* s/ten=1, B=1, A=0, S=0 */
      if (data_layerB[i]) {
        linedata[i] = data_layerB[i];   /* normal */
      } else {
        if (data_sprite[i]) {
          if (data_sprite[i] == 63) {   /* shadow operator */
            if (data_layerA[i])
              linedata[i] = data_layerA[i] | 128;       /* shadow */
            else
              linedata[i] = bg | 128;   /* shadow */
          } else if (data_sprite[i] == 62) {    /* highlight operator */
            if (data_layerA[i])
              linedata[i] = data_layerA[i] | 64;        /* highlight */
            else
              linedata[i] = bg | 64;    /* highlight */
          } else {
            linedata[i] = data_sprite[i];       /* normal */
          }
        } else {
          if (data_layerA[i])
            linedata[i] = data_layerA[i];       /* normal */
          else
            linedata[i] = bg;   /* normal */
        }
      }
      break;
    case 10:                   /* s/ten=1, B=0, A=1, S=0 */
      if (data_layerA[i]) {
        linedata[i] = data_layerA[i];   /* normal */
      } else {
        if (data_sprite[i]) {
          if (data_sprite[i] == 63) {   /* shadow operator */
            if (data_layerB[i])
              linedata[i] = data_layerB[i] | 128;       /* shadow */
            else
              linedata[i] = bg | 128;   /* shadow */
          } else if (data_sprite[i] == 62) {    /* highlight operator */
            if (data_layerB[i])
              linedata[i] = data_layerB[i] | 64;        /* highlight */
            else
              linedata[i] = bg | 64;    /* highlight */
          } else {
            linedata[i] = data_sprite[i];       /* normal */
          }
        } else {
          if (data_layerB[i])
            linedata[i] = data_layerB[i];       /* normal */
          else
            linedata[i] = bg;   /* normal */
        }
      }
      break;
    case 11:                   /* s/ten=1, B=1, A=1, S=0 */
      if (data_layerA[i]) {
        linedata[i] = data_layerA[i];   /* normal */
      } else if (data_layerB[i]) {
        linedata[i] = data_layerB[i];   /* normal */
      } else if (data_sprite[i]) {
        if (data_sprite[i] == 63)       /* shadow operator */
          linedata[i] = bg | 128;       /* shadow */
        else if (data_sprite[i] == 62)  /* highlight operator */
          linedata[i] = bg | 64;        /* highlight */
        else
          linedata[i] = data_sprite[i]; /* normal */
      } else {
        linedata[i] = bg;       /* normal */
      }
      break;
    case 12:                   /* s/ten=0, B=0, A=0, S=1 */
      if (data_sprite[i]) {
        if (data_sprite[i] == 63) {     /* shadow operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i] | 128; /* shadow */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i] | 128; /* shadow */
          else
            linedata[i] = bg | 128;     /* shadow */
        } else if (data_sprite[i] == 62) {      /* highlight operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i];       /* normal */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i];       /* normal */
          else
            linedata[i] = bg;   /* normal */
        } else {
          linedata[i] = data_sprite[i]; /* normal */
        }
      } else if (data_layerA[i]) {
        linedata[i] = data_layerA[i] | 128;     /* shadow */
      } else if (data_layerB[i]) {
        linedata[i] = data_layerB[i] | 128;     /* shadow */
      } else {
        linedata[i] = bg | 128; /* shadow */
      }
      break;
    case 13:                   /* s/ten=1, B=1, A=0, S=1 */
      if (data_sprite[i]) {
        if (data_sprite[i] == 63) {     /* shadow operator */
          if (data_layerB[i])
            linedata[i] = data_layerB[i] | 128; /* shadow */
          else if (data_layerA[i])
            linedata[i] = data_layerA[i] | 128; /* shadow */
          else
            linedata[i] = bg | 128;     /* shadow */
        } else if (data_sprite[i] == 62) {      /* highlight operator */
          if (data_layerB[i])
            linedata[i] = data_layerB[i] | 64;  /* highlight */
          else if (data_layerA[i])
            linedata[i] = data_layerA[i] | 64;  /* highlight */
          else
            linedata[i] = bg | 64;      /* highlight */
        } else {
          linedata[i] = data_sprite[i]; /* normal */
        }
      } else if (data_layerB[i]) {
        linedata[i] = data_layerB[i];   /* normal */
      } else if (data_layerA[i]) {
        linedata[i] = data_layerA[i];   /* normal */
      }
      break;
    case 14:                   /* s/ten=1, B=0, A=1, S=1 */
      if (data_sprite[i]) {
        if (data_sprite[i] == 63) {     /* shadow operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i] | 128; /* shadow */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i] | 128; /* shadow */
          else
            linedata[i] = bg | 128;     /* shadow */
        } else if (data_sprite[i] == 62) {      /* highlight operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i] | 64;  /* highlight */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i] | 64;  /* highlight */
          else
            linedata[i] = bg | 64;      /* highlight */
        } else {
          linedata[i] = data_sprite[i]; /* normal */
        }
      } else if (data_layerA[i]) {
        linedata[i] = data_layerA[i];   /* normal */
      } else if (data_layerB[i]) {
        linedata[i] = data_layerB[i];   /* normal */
      }
      break;
    case 15:                   /* s/ten=1, B=1, A=1, S=1 */
      if (data_sprite[i]) {
        if (data_sprite[i] == 63) {     /* shadow operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i] | 128; /* shadow */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i] | 128; /* shadow */
          else
            linedata[i] = bg | 128;     /* shadow */
        } else if (data_sprite[i] == 62) {      /* highlight operator */
          if (data_layerA[i])
            linedata[i] = data_layerA[i] | 64;  /* highlight */
          else if (data_layerB[i])
            linedata[i] = data_layerB[i] | 64;  /* highlight */
          else
            linedata[i] = bg | 64;      /* highlight */
        } else {
          linedata[i] = data_sprite[i]; /* normal */
        }
      } else if (data_layerA[i]) {
        linedata[i] = data_layerA[i];   /* normal */
      } else if (data_layerB[i]) {
        linedata[i] = data_layerB[i];   /* normal */
      }
      break;
    }
  }
}

void vdp_renderframe(uint8 *framedata, unsigned int lineoffset)
{
  unsigned int i, line;
  uint32 background;
  unsigned int vertcells = vdp_reg[1] & 1 << 3 ? 30 : 28;
  uint8 *linedata;

  /* fill in background */

  background = vdp_reg[7] & 63;
  background |= background << 8;
  background |= background << 16;

  for (line = 0; line < vertcells * 8; line++) {
    linedata = framedata + line * lineoffset;
    for (i = 0; i < (320 / 4); i++) {
      ((uint32 *)linedata)[i] = background;
    }
  }

  if (vdp_reg[1] & 1 << 6) {
    if (vdp_layerB)
      vdp_layer_simple(1, 0, framedata, lineoffset);
    if (vdp_layerA)
      vdp_layer_simple(0, 0, framedata, lineoffset);
    if (vdp_layerH && (vdp_reg[12] & 1 << 3))
      vdp_shadow_simple(framedata, lineoffset);
    if (vdp_layerS)
      vdp_sprites_simple(0, framedata, lineoffset);
    if (vdp_layerBp)
      vdp_layer_simple(1, 1, framedata, lineoffset);
    if (vdp_layerAp)
      vdp_layer_simple(0, 1, framedata, lineoffset);
    if (vdp_layerSp)
      vdp_sprites_simple(1, framedata, lineoffset);
  }

}

void vdp_showregs(void)
{
/*
  int i;

  for (i = 0; i < 25; i++) {
    printf("[%02d] %02X: ", i, vdp_reg[i]);
    switch (i) {
    case 0:
      printf("%s ", vdp_reg[0] & 1 << 1 ? "HV-stop" : "HV-enable");
      printf("%s ", vdp_reg[0] & 1 << 4 ? "HInt-enable" : "HInt-disable");
      break;
    case 1:
      printf("%s ", vdp_reg[1] & 1 << 3 ? "30-cell" : "28-cell");
      printf("%s ", vdp_reg[1] & 1 << 4 ? "DMA-enable" : "DMA-disable");
      printf("%s ", vdp_reg[1] & 1 << 5 ? "VInt-enable" : "VInt-disable");
      printf("%s ", vdp_reg[1] & 1 << 6 ? "Disp-enable" : "Disp-disable");
      break;
    case 2:
      printf("Scroll A @ %04X", (vdp_reg[2] & 0x38) << 10);
      break;
    case 3:
      printf("Window @ %04X", (vdp_reg[3] & 0x3E) << 10);
      break;
    case 4:
      printf("Scroll B @ %04X", (vdp_reg[4] & 7) << 13);
      break;
    case 5:
      printf("Sprites @ %04X", (vdp_reg[5] & 0x7F) << 9);
      break;
    case 7:
      printf("bgpal %d col %d", (vdp_reg[7] >> 4 & 3), (vdp_reg[7] & 15));
      break;
    case 10:
      printf("hintreg %04X", vdp_reg[10]);
      break;
    case 11:
      printf("V-mode %d H-mode %d ", (vdp_reg[11] >> 2) & 1,
             (vdp_reg[11] & 3));
      printf("%s",
             (vdp_reg[11] & 1 << 3) ? "ExtInt-enable" : "ExtInt-disable");
      break;
    case 12:
      printf("Interlace %d ", (vdp_reg[12] >> 1) & 3);
      printf("%s ", (vdp_reg[12] & 1 << 0) ? "40-cell" : "32-cell");
      printf("%s ",
             (vdp_reg[12] & 1 << 3) ? "Shadow-enable" : "Shadow-disable");
      break;
    case 13:
      printf("Scroll A @ %04X", (vdp_reg[13] & 0x3F) << 10);
      break;
    case 15:
      printf("Autoinc %d", vdp_reg[15]);
      break;
    case 16:
      printf("Vsize %d Hsize %d", (vdp_reg[16] >> 4) & 3, (vdp_reg[16] & 3));
      break;
    case 17:
      printf("Window H %s ", (vdp_reg[17] & 1 << 7) ? "right" : "left");
      printf("%d", vdp_reg[17] & 0x1F);
      break;
    case 18:
      printf("Window V %s ", (vdp_reg[18] & 1 << 7) ? "lower" : "upper");
      printf("%d", vdp_reg[18] & 0x1F);
      break;
    case 19:
      printf("DMA-length-low %02X", vdp_reg[19]);
      break;
    case 20:
      printf("DMA-length-high %02X", vdp_reg[20]);
      break;
    case 21:
      printf("DMA-source-low %02X", vdp_reg[21]);
      break;
    case 22:
      printf("DMA-source-mid %02X", vdp_reg[22]);
      break;
    case 23:
      printf("DMA-source-high %02X", vdp_reg[23]);
      break;
    }
    printf("\n");
  }
  printf("Cur hpos = %02X\n", vdp_gethpos());
  printf("Cur line = %02X (NB: could be +1 after h-int)\n", vdp_line);
  */
}

void vdp_spritelist(void)
{
  uint8 *spritelist = vdp_vram + ((vdp_reg[5] & 0x7F) << 9);
  uint8 *sprite;
  uint8 link = 0;
  uint16 pattern;
  uint8 palette;
  uint16 cellinfo;
  sint16 vpos, hpos, vmax;
  uint8 vsize, hsize;

  LOG_REQUEST(("SPRITE DUMP: (base=vram+%X)", (vdp_reg[5] & 0x7f) << 9));
  do {
    sprite = spritelist + (link << 3);
    hpos = (LOCENDIAN16(*(uint16 *)(sprite + 6)) & 0x1FF) - 0x80;
    vpos = (LOCENDIAN16(*(uint16 *)(sprite)) & 0x3FF) - 0x80;
    vsize = 1 + (sprite[2] & 3);
    hsize = 1 + ((sprite[2] >> 2) & 3);
    cellinfo = LOCENDIAN16(*(uint16 *)(sprite + 4));
    pattern = cellinfo & 0x7FF;
    palette = (cellinfo >> 13) & 3;
    vmax = vpos + vsize * 8;

    LOG_REQUEST(("Sprite %d @ %X", link,
                 (link << 3) | (vdp_reg[5] & 0x7f) << 9));
    LOG_REQUEST(("  Pos:  %d,%d", hpos, vpos));
    LOG_REQUEST(("  Size: %d,%d", hsize, vsize));
    LOG_REQUEST(("  Pri: %d, Pal: %d, Vflip: %d, Hflip: %d",
                 (cellinfo >> 15 & 1), (cellinfo >> 13 & 3),
                 (cellinfo >> 12 & 1), (cellinfo >> 11 & 1)));
    LOG_REQUEST(("  Pattern: %d (%x) @ vram+%X (%X if interlaced)",
                 (cellinfo & 0x7FF), (cellinfo & 0x7FF),
                 (cellinfo & 0x7FF) * 32, (cellinfo & 0x7FF) * 32));
    link = sprite[3] & 0x7F;
  }
  while (link);
}

void vdp_describe(void)
{
  int layer;
  unsigned int line;
  uint32 o_patterndata, o_hscrolldata;
  uint16 *patterndata, *hscrolldata;
  uint8 hsize = vdp_reg[16] & 3;
  uint8 vsize = (vdp_reg[16] >> 4) & 3;
  uint8 hmode = vdp_reg[11] & 3;
  uint8 vmode = (vdp_reg[11] >> 2) & 1;
  uint16 hwidth, vwidth, hoffset, voffset, raw_hoffset;

  hwidth = 32 + hsize * 32;
  vwidth = 32 + vsize * 32;
  LOG_REQUEST(("VDP description:"));
  LOG_REQUEST(("  hsize = %d (ie. width=%d)", hsize, hwidth));
  LOG_REQUEST(("  vsize = %d (ie. width=%d)", vsize, vwidth));
  LOG_REQUEST(("  hmode = %d (0=full, 2=cell, 3=line)", hmode));
  LOG_REQUEST(("  vmode = %d (0=full, 1=2cell", vmode));

  for (layer = 0; layer < 2; layer++) {
    LOG_REQUEST(("  Layer %s:", layer == 0 ? "A" : "B"));
    o_patterndata = (layer == 0 ? ((vdp_reg[2] & 0x38) << 10) :
                     ((vdp_reg[4] & 7) << 13));
    o_hscrolldata = layer * 2 + ((vdp_reg[13] & 63) << 10);
    LOG_REQUEST(("    Pattern data @ vram+%08X", o_patterndata));
    LOG_REQUEST(("    Hscroll data @ vram+%08X", o_hscrolldata));
    patterndata = (uint16 *)(vdp_vram + o_patterndata);
    hscrolldata = (uint16 *)(vdp_vram + o_hscrolldata);
    for (line = 0; line < vdp_vislines; line++) {
      switch (hmode) {
      case 0:                  /* full screen */
        hoffset = (0x400 - LOCENDIAN16(hscrolldata[0])) & 0x3FF;
        break;
      case 1:                  /* line scroll with first 8 lines */
        hoffset = (0x400 - LOCENDIAN16(hscrolldata[2 * (line & 7)])) & 0x3FF;
        break;
      case 2:                  /* cell scroll */
        hoffset = (0x400 - LOCENDIAN16(hscrolldata[2 * (line & ~7)])) & 0x3FF;
        break;
      case 3:                  /* line scroll */
        hoffset = (0x400 - LOCENDIAN16(hscrolldata[2 * line])) & 0x3FF;
        break;
      default:
        hoffset = 0;
        break;
      }
      raw_hoffset = hoffset;
      hoffset &= (hwidth << 8) - 1;     /* put offset in range */
      voffset = (line + LOCENDIAN16(((uint16 *)vdp_vsram)[layer])) & 0x3FF;
      voffset &= (vwidth << 8) - 1;     /* put offset in range */
      LOG_REQUEST(("     line %d: hoffset=%d=%d, voffset=%d, "
                   "firstcell=vram+%08X", line, raw_hoffset, hoffset,
                   voffset,
                   o_patterndata + 2 * ((hoffset >> 3) +
                                        hwidth * (voffset >> 3))));
    }
  }
}

void vdp_eventinit(void)
{
  /* Facts from documentation:
     H-Blank is 73.7 clock cycles long.
     The VDP settings are aquired 36 clocks after start of H-Blank.
     The display period is 413.3 clocks in duration.
     V-Int occurs 14.7us after H-Int (which is 112 clock cycles)
     Facts from clock data:
     One line takes 488 clocks (vdp_clksperline_68k)
     Assumptions: (not sure if these are true anymore)
     We 'approximate' and make H-Int occur at the same time as H-Blank.
     V-Blank starts at V-Int and ends at the start of line 0.

     vdp_event_start    = start of line, end of v-blank
     vdp_event_vint     = v-int time on line 224 (or 240)
     (112 clocks after h-int)
     vdp_event_hint     = h-int time at end of each line
     vdp_event_hdisplay = settings are aquired and current line displayed
     vdp_event_end      = end of line, end of h-blank

     Note that if the program stays in H-Int 224 longer than 112 clocks, V-Int
     is not supposed to occur due to the processor acknowledging the wrong
     interrupt from the VDP, thus programs disable H-Ints on 223 to prevent
     this problem.  We don't worry about this.
   */
  vdp_event = 0;
  vdp_event_start = 0;
  vdp_event_vint = 112 - 74;
  vdp_event_hint = vdp_clksperline_68k - 74;
  vdp_event_hdisplay = vdp_event_hint + 36;
  vdp_event_end = vdp_clksperline_68k;
  vdp_nextevent = 0;
}

void vdp_endfield(void)
{
  vdp_line = 0;
  vdp_eventinit();
  vdp_oddframe ^= 1;            /* toggle */
  /* printf("(%d,%d,%d,%d,%d)\n", vdp_event_type,
     vdp_event_startline, vdp_event_hint, vdp_event_vdpplot,
     vdp_event_endline); */
}

inline void vdp_plotcell(uint8 *patloc, uint8 palette, uint8 flags,
                         uint8 *cellloc, unsigned int lineoffset)
{
  int y, x;
  uint8 value;
  uint32 data;

  switch (flags) {
  case 0:
    /* normal tile - no s/ten */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[y]);
      for (x = 0; x < 8; x++, data <<= 4) {
        value = data >> 28;
        if (value)
          cellloc[x] = palette * 16 + value;
      }
    }
    break;
  case 1:
    /* h flipped tile - no s/ten */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[y]);
      for (x = 0; x < 8; x++, data >>= 4) {
        value = data & 15;
        if (value)
          cellloc[x] = palette * 16 + value;
      }
    }
    break;
  case 2:
    /* v flipped tile - no s/ten */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[7 - y]);
      for (x = 0; x < 8; x++, data <<= 4) {
        value = data >> 28;
        if (value)
          cellloc[x] = palette * 16 + value;
      }
    }
    break;
  case 3:
    /* h and v flipped tile - no s/ten */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[7 - y]);
      for (x = 0; x < 8; x++, data >>= 4) {
        value = data & 15;
        if (value)
          cellloc[x] = palette * 16 + value;
      }
    }
    break;
  case 4:
    /* normal tile - s/ten enabled */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[y]);
      for (x = 0; x < 8; x++, data <<= 4) {
        value = data >> 28;
        if (value) {
          if (palette == 3 && value == 14) {
            cellloc[x] = (cellloc[x] & 63) + 64;
          } else if (palette == 3 && value == 15) {
            cellloc[x] = (cellloc[x] & 63) + 128;
          } else {
            cellloc[x] = palette * 16 + value;
          }
        }
      }
    }
    break;
  case 5:
    /* h flipped tile - s/ten */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[y]);
      for (x = 0; x < 8; x++, data >>= 4) {
        value = data & 15;
        if (value) {
          if (palette == 3 && value == 14) {
            cellloc[x] = (cellloc[x] & 63) + 64;
          } else if (palette == 3 && value == 15) {
            cellloc[x] = (cellloc[x] & 63) + 128;
          } else {
            cellloc[x] = palette * 16 + value;
          }
        }
      }
    }
    break;
  case 6:
    /* v flipped tile - s/ten enabled */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[7 - y]);
      for (x = 0; x < 8; x++, data <<= 4) {
        value = data >> 28;
        if (value) {
          if (palette == 3 && value == 14) {
            cellloc[x] = (cellloc[x] & 63) + 64;
          } else if (palette == 3 && value == 15) {
            cellloc[x] = (cellloc[x] & 63) + 128;
          } else {
            cellloc[x] = palette * 16 + value;
          }
        }
      }
    }
    break;
  case 7:
    /* h and v flipped tile - s/ten enabled */
    for (y = 0; y < 8; y++, cellloc += lineoffset) {
      data = LOCENDIAN32(((uint32 *)patloc)[7 - y]);
      for (x = 0; x < 8; x++, data >>= 4) {
        value = data & 15;
        if (value) {
          if (palette == 3 && value == 14) {
            cellloc[x] = (cellloc[x] & 63) + 64;
          } else if (palette == 3 && value == 15) {
            cellloc[x] = (cellloc[x] & 63) + 128;
          } else {
            cellloc[x] = palette * 16 + value;
          }
        }
      }
    }
    break;
  default:
    ui_err("Unknown plotcell flags");
  }
}

/* must be 8*lineoffset bytes scrap before and after fielddata and also
   8 bytes before each line and 8 bytes after each line */

void vdp_layer_simple(unsigned int layer, unsigned int priority,
                      uint8 *framedata, unsigned int lineoffset)
{
  uint8 hsize = vdp_reg[16] & 3;
  uint8 vsize = (vdp_reg[16] >> 4) & 3;
  uint8 hmode = vdp_reg[11] & 3;
  uint8 vmode = (vdp_reg[11] >> 2) & 1;
  uint16 vramoffset = (layer ? ((vdp_reg[4] & 7) << 13) :
                       ((vdp_reg[2] & (7 << 3)) << 10));
  uint16 *patterndata = (uint16 *)(vdp_vram + vramoffset);
  uint16 *hscrolldata = (uint16 *)(((vdp_reg[13] & 63) << 10)
                                   + vdp_vram + layer * 2);
  uint8 screencells = (vdp_reg[12] & 1) ? 40 : 32;
  uint16 hwidth = 32 + hsize * 32;
  uint16 vwidth = 32 + vsize * 32;
  uint16 hoffset, voffset;
  uint16 cellinfo;
  uint8 *pattern;
  uint8 palette;
  unsigned int xcell, ycell;
  uint8 *toploc, *cellloc;
  uint8 flags;
  uint32 hscroll, vscroll;

  if (layer == 0 &&
      ((vdp_reg[18] == 0x9F && vdp_reg[17] == 0x9F) ||
       (vdp_reg[18] == 0x9F && vdp_reg[17] == 0x80) ||
       (vdp_reg[18] == 0x80) ||
       (vdp_reg[18] == 0x00 && vdp_reg[17] > (screencells << 1))))
    /* quick hack to remove layer A when it definitely shouldn't be plotted */
    return;

  for (xcell = 0; xcell <= screencells; xcell++) {
    if (vmode) {
      /* 2-cell scroll */
      vscroll = ((xcell >= screencells ? xcell - 2 : xcell) & ~1) + layer;
    } else {
      /* full screen */
      vscroll = layer;
    }
    voffset = LOCENDIAN16(((uint16 *)vdp_vsram)[vscroll]) & 0x3FF;
    toploc = framedata - lineoffset * (voffset & 7);
    for (ycell = 0; ycell <= 28;
         ycell++, voffset += 8, toploc += lineoffset * 8) {
      switch (hmode) {
      case 0:                  /* full screen */
      case 1:                  /* line scroll (first 8 lines) - approximation */
        hscroll = 0;
        break;
      case 2:                  /* cell scroll */
        hscroll = 2 * (ycell >= 28 ? ycell - 2 : ycell) * 8;
        break;
      case 3:                  /* line scroll - approximation */
        hscroll = 2 * (ycell >= 28 ? ycell - 2 : ycell) * 8;
        vdp_complex = 1;
        break;
      default:
        hscroll = 0;
        break;
      }
      voffset &= (vwidth * 8) - 1;
      hoffset = (0x400 - LOCENDIAN16(hscrolldata[hscroll])) & 0x3FF;
      hoffset = (hoffset + xcell * 8) & ((hwidth * 8) - 1);
      cellinfo =
        LOCENDIAN16(patterndata[(hoffset >> 3) + hwidth * (voffset >> 3)]);
      if (((uint8)((cellinfo & 1 << 15) ? 1 : 0)) == priority) {
        /* plot cell */
        palette = (cellinfo >> 13) & 3;
        pattern = vdp_vram + ((cellinfo & 2047) << 5);
        flags = (cellinfo >> 11) & 3;   /* bit0=H flip, bit1=V flip */
        cellloc = toploc - (hoffset & 7) + xcell * 8;
        vdp_plotcell(pattern, palette, flags, cellloc, lineoffset);
      }
    }                           /* ycell */
  }                             /* xcell */
}

void vdp_shadow_simple(uint8 *framedata, unsigned int lineoffset)
{
  unsigned int vertcells = vdp_reg[1] & 1 << 3 ? 30 : 28;
  uint8 *linedata;
  unsigned int line;
  int i;

  for (line = 0; line < vertcells * 8; line++) {
    linedata = framedata + line * lineoffset;
    /* this could be done 4 bytes at a time */
    for (i = 0; i < 320; i++)
      linedata[i] = (linedata[i] & 63) + 128;
  }
}

void vdp_sprites_simple(unsigned int priority, uint8 *framedata,
                        unsigned int lineoffset)
{
  uint8 *spritelist = vdp_vram + ((vdp_reg[5] & 0x7F) << 9);

  vdp_sprite_simple(priority, framedata, lineoffset, 1,
                    spritelist, spritelist);
}

int vdp_sprite_simple(unsigned int priority, uint8 *framedata,
                      unsigned int lineoffset, unsigned int number,
                      uint8 *spritelist, uint8 *sprite)
{
  int plotted = 1;
  uint8 link;
  uint16 pattern;
  uint8 palette;
  uint16 cellinfo;
  sint16 vpos, hpos, vmax;
  uint16 xcell, ycell;
  uint8 vsize, hsize;
  uint8 *cellloc;
  uint8 flags;
  uint8 *patloc;

  if (number > 80) {
    LOG_VERBOSE(("%08X [VDP] Maximum of 80 sprites exceeded", regs.pc));
    return 0;
  }

  link = sprite[3] & 0x7F;
  hpos = (LOCENDIAN16(*(uint16 *)(sprite + 6)) & 0x1FF) - 0x80;
  vpos = (LOCENDIAN16(*(uint16 *)(sprite)) & 0x3FF) - 0x80;
  vsize = 1 + (sprite[2] & 3);
  hsize = 1 + ((sprite[2] >> 2) & 3);
  cellinfo = LOCENDIAN16(*(uint16 *)(sprite + 4));
  pattern = cellinfo & 0x7FF;
  palette = (cellinfo >> 13) & 3;
  vmax = vpos + vsize * 8;

  if (link) {
    if (hpos == -128)
      /* we do not support 'masking' in simple mode */
      vdp_complex = 1;
    plotted =
      vdp_sprite_simple(priority, framedata, lineoffset, number + 1,
                        spritelist, spritelist + (link << 3));
    plotted++;
  }

  if (((uint8)((cellinfo & 1 << 15) ? 1 : 0)) != priority)
    return plotted;
  if (vpos >= 240 || hpos >= 320 || vpos + vsize * 8 <= 0
      || hpos + hsize * 8 <= 0)
    /* sprite is not on screen */
    return plotted;
  flags = (cellinfo >> 11) & 3; /* bit0=H flip, bit1=V flip */
  if (vdp_reg[12] & 1 << 3)     /* s/ten enabled? */
    flags |= 1 << 2;
  switch (flags) {
  case 0:
  case 4:
    /* normal orientation */
    for (ycell = 0; ycell < vsize; ycell++) {
      if (ycell * 8 + vpos < -7 || ycell * 8 + vpos >= 240)
        /* cell out of plotting area (remember scrap area) */
        continue;
      patloc = vdp_vram + (pattern << 5) + ycell * 32;
      for (xcell = 0; xcell < hsize; xcell++, patloc += vsize * 32) {
        if (xcell * 8 + hpos < -7 || xcell * 8 + hpos >= 320)
          /* cell out of plotting area */
          continue;
        cellloc = framedata + ((vpos + ycell * 8) * lineoffset +
                               (hpos + xcell * 8));
        vdp_plotcell(patloc, palette, flags, cellloc, lineoffset);
      }
    }
    break;
  case 1:
  case 5:
    /* H flip */
    for (ycell = 0; ycell < vsize; ycell++) {
      if (ycell * 8 + vpos < -7 || ycell * 8 + vpos >= 240)
        /* cell out of plotting area (remember scrap area) */
        continue;
      patloc =
        vdp_vram + (pattern << 5) + ycell * 32 + vsize * 32 * (hsize - 1);
      for (xcell = 0; xcell < hsize; xcell++, patloc -= vsize * 32) {
        if (xcell * 8 + hpos < -7 || xcell * 8 + hpos >= 320)
          /* cell out of plotting area */
          continue;
        cellloc = framedata + ((vpos + ycell * 8) * lineoffset +
                               (hpos + xcell * 8));
        vdp_plotcell(patloc, palette, flags, cellloc, lineoffset);
      }
    }
    break;
  case 2:
  case 6:
    /* V flip */
    for (ycell = 0; ycell < vsize; ycell++) {
      if (ycell * 8 + vpos < -7 || ycell * 8 + vpos >= 240)
        /* cell out of plotting area (remember scrap area) */
        continue;
      patloc = vdp_vram + (pattern << 5) + (vsize - ycell - 1) * 32;
      for (xcell = 0; xcell < hsize; xcell++, patloc += vsize * 32) {
        if (xcell * 8 + hpos < -7 || xcell * 8 + hpos >= 320)
          /* cell out of plotting area */
          continue;
        cellloc = framedata + ((vpos + ycell * 8) * lineoffset +
                               (hpos + xcell * 8));
        vdp_plotcell(patloc, palette, flags, cellloc, lineoffset);
      }
    }
    break;
  case 3:
  case 7:
    /* H and V flip */
    for (ycell = 0; ycell < vsize; ycell++) {
      if (ycell * 8 + vpos < -7 || ycell * 8 + vpos >= 240)
        /* cell out of plotting area (remember scrap area) */
        continue;
      patloc = vdp_vram + (pattern << 5) + ((vsize - ycell - 1) * 32 +
                                            vsize * 32 * (hsize - 1));
      for (xcell = 0; xcell < hsize; xcell++, patloc -= vsize * 32) {
        if (xcell * 8 + hpos < -7 || xcell * 8 + hpos >= 320)
          /* cell out of plotting area */
          continue;
        cellloc = framedata + ((vpos + ycell * 8) * lineoffset +
                               (hpos + xcell * 8));
        vdp_plotcell(patloc, palette, flags, cellloc, lineoffset);
      }
    }
    break;
  }
  return plotted;
}


uint8 vdp_gethpos(void)
{
  float percent;

  // vdp_event = 0/1/2 -> beginning of line until 74 clocks before end
  // 3     -> between hint and hdisplay (36 clocks)
  // 4     -> between hdisplay and end  (38 clocks)
  // This routine goes from 0 to the maximum number allowed not within
  // H-blank, and then goes slightly beyond up to hdisplay.  Then
  // between hdisplay and end we go negative.  I'm not sure how negative
  // it is supposed to be, this goes from -38 to 0.
  //
  // 40 horizontal cells, H goes from $00 to $B6, $E4 to $FF
  // 32 horizontal cells, H goes from $00 to $93, $E8 to $FF
  //
  // this is such a bodge - any changes, check '3 Ninjas kick back'
  
  LOG_DEBUG1(("gethpos %X: clocks=%X : startofline=%X : hint=%X : "
              "end=%X", vdp_event, cpu68k_clocks,
              vdp_event_start, vdp_event_hint, vdp_event_end));
  if (vdp_event < 3) {
    percent = ((float)(cpu68k_clocks - vdp_event_start) /
               (float)(vdp_event_hint - vdp_event_start));
    if (vdp_reg[12] & 1)
      return (cpu68k_clocks > vdp_event_hint) ? 0xE4 : percent * 0xB6;
    else
      return (cpu68k_clocks > vdp_event_hint) ? 0xE8 : percent * 0x93;
  } else {
    percent = ((float)(cpu68k_clocks - vdp_event_hint) /
               (float)(vdp_event_end - vdp_event_hint));
    if (vdp_reg[12] & 1)
      return ((cpu68k_clocks > vdp_event_end) ? 0 :
              ((uint8)(0xE4 + percent * 28) & 0xff));
    else
      return ((cpu68k_clocks > vdp_event_end) ? 0 :
              ((uint8)(0xE8 + percent * 24) & 0xff));
  }
}
