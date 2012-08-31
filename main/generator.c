/* Generator is (c) James Ponder, 1997-2001 http://www.squish.net/generator/ */

#include <tamtypes.h>
#include <string.h>
#include <malloc.h>
#include "generator.h"
#include "ctype.h"

#include "ui.h"
#include "memz80.h"
#include "cpu68k.h"
#include "mem68k.h"
#include "cpuz80.h"
#include "vdp.h"
#include "gensound.h"

/*** variables externed in generator.h ***/

unsigned int gen_sixcont = 0; // 6-button controller enable
unsigned int gen_multitap = 0; // mutlitap enable

unsigned int gen_region = 0;
unsigned int gen_quit = 0;
unsigned int gen_debugmode = 0;
unsigned int gen_loglevel = 0;  /* 2 = NORMAL, 1 = CRITICAL */
unsigned int gen_autodetect = 1; /* 0 = no, 1 = yes */
unsigned int gen_musiclog = 0; /* 0 = no, 1 = GYM, 2 = GNM */
t_cartinfo gen_cartinfo;
char gen_leafname[128];

//static int gen_freerom = 0;
static int gen_freesram = 0;

/*** forward references ***/

void gen_nicetext(char *out, char *in, unsigned int size);
uint16 gen_checksum(uint8 *start, unsigned int length);
void gen_setupcartinfo(void);

/*** Program entry point ***/

int gen_init()
{
  int retval;

  /* initialise 68k memory system */
  if ((retval = mem68k_init()))
    ui_err("Failed to initialise mem68k module (%d)", retval);

  /* initialise z80 memory system */
  if ((retval = memz80_init()))
    ui_err("Failed to initialise memz80 module (%d)", retval);

  /* initialise vdp system */
  if ((retval = vdp_init()))
    ui_err("Failed to initialise vdp module (%d)", retval);

  /* initialise cpu system */
  if ((retval = cpu68k_init()))
    ui_err("Failed to initialise cpu68k module (%d)", retval);

  /* initialise z80 cpu system */
  if ((retval = cpuz80_init()))
    ui_err("Failed to initialise cpuz80 module (%d)", retval);

  /* initialise sound system */
  if ((retval = sound_init()))
    ui_err("Failed to initialise sound module (%d)", retval);

}

/*** gen_reset - reset system ***/

extern int isModePressed();

void gen_reset(void)
{
  gen_quit = 0;

  mem68k_init(); // needed to disable sram if game does not support it
  vdp_reset();
  cpu68k_reset();
  cpuz80_reset();
  if (sound_reset()) {
    ui_err("sound failure");
  }

  // turn off six button controller if MODE button pressed
  if(isModePressed()) gen_sixcont = 0;
}

/*** gen_softreset - reset system ***/

void gen_softreset(void)
{
  cpu68k_reset();
}

/* setup to run from ROM in memory */

/*
void gen_loadmemrom(const char *rom, int romlen)
{
  cpu68k_rom = (char *)rom; // I won't alter it, promise
  cpu68k_romlen = romlen;
  gen_freerom = 0;
  gen_setupcartinfo();
  gen_reset();
}
*/

/* setup gen_cartinfo from current loaded rom */

void gen_setupcartinfo(void)
{
  unsigned int i;
  char *p;

  memset(&gen_cartinfo, 0, sizeof(gen_cartinfo));
  gen_nicetext(gen_cartinfo.console, (char *)(cpu68k_rom + 0x100), 16);
  gen_nicetext(gen_cartinfo.copyright, (char *)(cpu68k_rom + 0x110), 16);
  gen_nicetext(gen_cartinfo.name_domestic, (char *)(cpu68k_rom + 0x120), 48);
  gen_nicetext(gen_cartinfo.name_overseas, (char *)(cpu68k_rom + 0x150), 48);
  if (cpu68k_rom[0x180] == 'G' && cpu68k_rom[0x181] == 'M') {
    gen_cartinfo.prodtype = pt_game;
  } else if (cpu68k_rom[0x180] == 'A' && cpu68k_rom[0x181] == 'I') {
    gen_cartinfo.prodtype = pt_education;
  } else {
    gen_cartinfo.prodtype = pt_unknown;
  }
  gen_nicetext(gen_cartinfo.version, (char *)(cpu68k_rom + 0x182), 12);
  gen_cartinfo.checksum = gen_checksum(((uint8 *)cpu68k_rom) + 0x200,
                                       cpu68k_romlen - 0x200);
  gen_nicetext(gen_cartinfo.memo, (char *)(cpu68k_rom + 0x1C8), 28);
  for (i = 0x1f0; i < 0x1ff; i++) {
    if (cpu68k_rom[i] == 'J')
      gen_cartinfo.flag_japan = 1;
    if (cpu68k_rom[i] == 'U')
      gen_cartinfo.flag_usa = 1;
    if (cpu68k_rom[i] == 'E')
      gen_cartinfo.flag_europe = 1;
	if (cpu68k_rom[i] == '1')
	  gen_cartinfo.flag_japan = 1;
	if (cpu68k_rom[i] == '4')
	  gen_cartinfo.flag_usa = 1;
	if (cpu68k_rom[i] == '5')
	  gen_cartinfo.flag_usa = 1;
	if (cpu68k_rom[i] == '8')
	  gen_cartinfo.flag_europe = 1;
  }
  if (cpu68k_rom[0x1f0] >= '1' && cpu68k_rom[0x1f0] <= '9') {
    gen_cartinfo.hardware = cpu68k_rom[0x1f0] - '0';
  } else if (cpu68k_rom[0x1f0] >= 'A' && cpu68k_rom[0x1f0] <= 'F') {
    gen_cartinfo.hardware = cpu68k_rom[0x1f0] - 'A' + 10;
  }
  p = gen_cartinfo.country;
  for (i = 0x1f0; i < 0x200; i++) {
    if (cpu68k_rom[i] != 0 && cpu68k_rom[i] != 32)
      *p++ = cpu68k_rom[i];
  }
  *p = '\0';

  if(cpu68k_srambuff) {
  	if(gen_freesram) free(cpu68k_srambuff);
	cpu68k_sram = NULL;
	cpu68k_srambuff = NULL;
  }

  // Setup SRAM - Added by Nick Van Veen. Based on code from DGen.
  if(cpu68k_rom[0x1b1] == 'A' && cpu68k_rom[0x1b0] == 'R' && 1) {
  	 cpu68k_sramstart =	cpu68k_rom[0x1b4] << 24 | cpu68k_rom[0x1b5] << 16 |
				cpu68k_rom[0x1b6] << 8  | cpu68k_rom[0x1b7];
     	 cpu68k_sramend = 	cpu68k_rom[0x1b8] << 24 | cpu68k_rom[0x1b9] << 16 |
  	         		cpu68k_rom[0x1ba] << 8  | cpu68k_rom[0x1bb];
	// Make sure start is even, end is odd, for alignment
	// A ROM that I came across had the start and end bytes of
	// the save ram the same and wouldn't work.  Fix this as seen
	// fit, I know it could probably use some work. [PKH]
 	if(cpu68k_sramstart != cpu68k_sramend) {
        	if(cpu68k_sramstart & 1) --cpu68k_sramstart;
        	if(!(cpu68k_sramend & 1)) ++cpu68k_sramend;

		cpu68k_sramend++;

        	cpu68k_sramlen = cpu68k_sramend - cpu68k_sramstart;
			cpu68k_srambuff = (uint8 *)memalign(16,cpu68k_sramlen+64); // first 64 bytes is game name
        	cpu68k_sram = cpu68k_srambuff + 64;

		gen_freesram = 1;

		// If save RAM does not overlap main ROM, set it active by default since
		// a few games can't manage to properly switch it on/off.
		if(cpu68k_sramstart >= cpu68k_romlen)
	  		cpu68k_sramactive = 1;
	} else {
        	cpu68k_sramstart = cpu68k_sramlen = cpu68k_sramend = 0;
        	cpu68k_sram = NULL;
			cpu68k_srambuff = NULL;
		cpu68k_sramactive = 0;
	}
  }
  else
  {
 	cpu68k_sramstart = cpu68k_sramlen = cpu68k_sramend = 0;
	cpu68k_sram = NULL;
	cpu68k_srambuff = NULL;
	cpu68k_sramactive = 0;
  }

}

/*** get_nicetext - take a string, remove spaces and capitalise ***/

void gen_nicetext(char *out, char *in, unsigned int size)
{
  int flag, i;
  int c;
  char *start = out;

  flag = 0;                     /* set if within word, e.g. make lowercase */
  i = size;                     /* maximum number of chars in input */
  while ((c = *in++) && --i > 0) {
    if (isalpha(c)) {
      if (!flag) {
        /* make uppercase */
        flag = 1;
        if (islower(c))
          *out++ = c - 'z' + 'Z';
        else
          *out++ = c;
      } else {
        /* make lowercase */
        if (isupper(c))
          *out++ = (c) - 'Z' + 'z';
        else
          *out++ = c;
      }
    } else if (c == ' ') {
      if (flag)
        *out++ = c;
      flag = 0;
    } else if (isprint(c) && c != '\t') {
      flag = 0;
      *out++ = c;
    }
  }
  while (out > start && out[-1] == ' ')
    out--;
  *out++ = '\0';
}

/*** gen_checksum - get Genesis-style checksum of memory block ***/

uint16 gen_checksum(uint8 *start, unsigned int length)
{
  uint16 checksum = 0;

  if (length & 1) {
    length &= ~1;
    LOG_CRITICAL(("checksum routines given odd length (%d)", length));
  }

  for (; length; length -= 2, start += 2) {
    checksum += start[0] << 8;
    checksum += start[1];
  }
  return checksum;
}

//
// GEN_LOADIMAGE, PS2 SPECIFIC
//


char *gen_loadimage(char *filename)
{
/*
  int file, imagetype, bytes, bytesleft;
  const char *extension;
  uint8 *buffer;
  unsigned int blocks, x, i;
  uint8 *new;
  char *p;

  // Remove current file
  if (cpu68k_rom) {
    if (gen_freerom)
      free(cpu68k_rom);
    cpu68k_rom = NULL;
  }

	fioSetBlockMode(FIO_NOWAIT);
	fioOpen(filename, O_RDONLY);

	while(!fioSync(FIO_NOWAIT, &file))
		draw_loading_rom();

	if(file < 0) {
		cpu68k_rom = NULL;
	    cpu68k_romlen = 0;
		ui_err("Error loading rom!!");
	  }

  cpu68k_romlen = fioLseek(file,0,SEEK_END);
  fioLseek(file,0,SEEK_SET);

  if (cpu68k_romlen < 0x200) {
    ui_err("File is too small");
  }

  // allocate enough memory plus 16 bytes for disassembler to cope with the last instruction
  if ((cpu68k_rom = malloc(cpu68k_romlen + 16)) == NULL) {

    cpu68k_romlen = 0;
    ui_err("Out of memory!");
  }

  gen_freerom = 1;
  memset(cpu68k_rom, 0, cpu68k_romlen + 16);

  buffer = cpu68k_rom;
  bytesleft = cpu68k_romlen;
  
  do {

	fioRead(file, buffer, bytesleft);
	while(!fioSync(FIO_NOWAIT, &bytes))
		draw_loading_rom();

    if (bytes <= 0)
      break;
    buffer += bytes;
    bytesleft -= bytes;
  }
  while (bytesleft >= 0);

  fioClose(file);
  fioSetBlockMode(FIO_WAIT);

  if (bytes == -1)
    ui_err("bytes == -1");
  else if (bytes != 0)
    ui_err("invalid return code from read()");

  if (bytesleft) {
    LOG_CRITICAL(("%d bytes left to read?!", bytesleft));
    ui_err("Error whilst loading file");
  }

  imagetype = 1;                // BIN file by default

  // SMD file format check - Richard Bannister
  if ((cpu68k_rom[8] == 0xAA) && (cpu68k_rom[9] == 0xBB) &&
      cpu68k_rom[10] == 0x06) {
    imagetype = 2;              // SMD file
  }
  // check for interleaved 'SEGA'
  if (cpu68k_rom[0x280] == 'E' && cpu68k_rom[0x281] == 'A' &&
      cpu68k_rom[0x2280] == 'S' && cpu68k_rom[0x2281] == 'G') {
    imagetype = 2;              // SMD file
  }
  // Check extension is not wrong
  extension = filename + strlen(filename) - 3;
  if (extension > filename) {
    if (!strcasecmp(extension, "smd") && (imagetype != 2))
      LOG_REQUEST(("File extension (smd) does not match detected "
                   "type (bin)"));
    if (!strcasecmp(extension, "bin") && (imagetype != 1))
      LOG_REQUEST(("File extension (bin) does not match detected "
                   "type (smd)"));
  }

  // convert to standard BIN file format
  switch (imagetype) {
  case 1:                      // BIN
    break;
  case 2:                      // SMD
    blocks = (cpu68k_romlen - 512) / 16384;
    if (blocks * 16384 + 512 != cpu68k_romlen) {
      ui_err("Image is corrupt.");
    }

    if ((new = malloc(cpu68k_romlen - 512)) == NULL) {
	//if ((new = memalign(16,cpu68k_romlen - 512)) == NULL) {
      cpu68k_rom = NULL;
      cpu68k_romlen = 0;
      ui_err("Out of memory!");
    }

    for (i = 0; i < blocks; i++) {
      for (x = 0; x < 8192; x++) {
        new[i * 16384 + x * 2 + 0] = cpu68k_rom[512 + i * 16384 + 8192 + x];
        new[i * 16384 + x * 2 + 1] = cpu68k_rom[512 + i * 16384 + x];
      }
    }
    free(cpu68k_rom);
    cpu68k_rom = new;
    cpu68k_romlen -= 512;
    break;
  default:
    ui_err("Unknown image type");
    break;
  }

  // is this icky?
  if ((p = strrchr(filename, '/')) == NULL &&
      (p = strrchr(filename, '\\')) == NULL) {
    snprintf(gen_leafname, sizeof(gen_leafname), "%s", filename);
  } else {
    snprintf(gen_leafname, sizeof(gen_leafname), "%s", p + 1);
  }
  if ((p = strrchr(gen_leafname, '.')) != NULL) {
    if ((!strcasecmp(p, ".smd")) || (!strcasecmp(p, ".bin")))
      *p = '\0';
  }
  if (gen_leafname[0] == '\0')
    snprintf(gen_leafname, sizeof(gen_leafname), "rom");

  gen_setupcartinfo();

	// setup defaults for IO devices
	gen_sixcont = 0;
	gen_multitap = 0;

	// scan IO info section in rom header
	for(i = 0x190; i < 0x1A0; i++)
	{
		if(cpu68k_rom[i] == '6') gen_sixcont = 1;
		if((cpu68k_rom[i] == '4') && ps2_multitap_connected) gen_multitap = 1;
	}

  if (gen_autodetect) {

 	if((ps2_default_vidmode == 2) && (gen_cartinfo.flag_usa)) gen_region = 2;
	else if((ps2_default_vidmode == 3) && (gen_cartinfo.flag_europe)) gen_region = 3;
	else if((ps2_default_vidmode == 0) && (gen_cartinfo.flag_japan)) gen_region = 0;
	else if(gen_cartinfo.flag_usa) gen_region = 2;
	else if(gen_cartinfo.flag_japan) gen_region = 0;
	else if(gen_cartinfo.flag_europe) gen_region = 3;

  }

  update_videomode(); // for multi-mode tv mode (change to pal/ntsc accordingly)

  // reset system
  gen_reset();

  // fix checksum, if broken
  if (gen_cartinfo.checksum != (cpu68k_rom[0x18e] << 8 | cpu68k_rom[0x18f])) {
	cpu68k_rom[0x18e] = gen_cartinfo.checksum >> 8;
	cpu68k_rom[0x18f] = gen_cartinfo.checksum & 0xff;
  }

  LOG_REQUEST(("Loaded '%s'/'%s' (%s %04X %s)", gen_cartinfo.name_domestic,
               gen_cartinfo.name_overseas, gen_cartinfo.version,
               gen_cartinfo.checksum, gen_cartinfo.country));

*/
  return NULL;
}
