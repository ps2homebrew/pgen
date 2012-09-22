#ifndef _PGEN_H
#define _PGEN_H

#include <stdio.h>
#include <tamtypes.h>
#include <stdarg.h>
#include <string.h>
#include <kernel.h>
#include <sifrpc.h>
#include <malloc.h>
#include <fileio.h>
#include <iopcontrol.h>
#include <loadfile.h>
#include <libpad.h>
#include <libmc.h>
#include <iopheap.h>

#include "gsDefs.h"
#include "gsDriver.h"
#include "gsPipe.h"
#include "gsFont.h"

#include "generator.h"
#include "cpu68k.h"
#include "mem68k.h"
#include "event.h"

#include "cdvd_rpc.h"
#include "libmtap.h"
#include "sjpcm.h"

#include "libhdd.h"
#include "sbv_patches.h"

#include "aio.h"
#include "data.h"
#include "gfx.h"
#include "input.h"
#include "render.h"
#include "state.h"
#include "save.h"
#include "rom.h"
#include "gui.h"

#include "pgen_vars.h"

#define PGEN_VERSION		2000

// Saves from PGEN versions >= the COMPAT_* macro are compatible with the current release
#define PGEN_COMPAT_VER		2000


#define PGEN_STATE_GUI				1
#define	PGEN_STATE_EMULATION		2
#define	PGEN_STATE_INGAME_MENU		3

void pgenSetRegion();

int pgenOptionsSave();
int pgenOptionsLoad();

#endif // _PGEN_H
