#ifndef _STATE_H
#define _STATE_H

#include "z80.h"

#define	SAVE_TYPE_STATE		1
#define	SAVE_TYPE_SRAM		2


#define	STATE_ERROR_CREATE_SAVE			-1
#define	STATE_ERROR_SAVE_STATE			-2
#define	STATE_ERROR_LOAD_STATE			-3
#define	STATE_ERROR_SAVE_SRAM			-4
#define	STATE_ERROR_LOAD_SRAM			-5
#define	STATE_ERROR_STATE_NOT_FOUND		-6
#define STATE_ERROR_VERSION				-7

#define PGEN_SAVE_MAGIC		0x4E454750	// 'PGEN'

typedef struct
{
	u8 		vdp_vram[64*1024];
	u8 		vdp_cram[128];
	u8 		vdp_vsram[80];
	u8 		vdp_reg[25];
	u8		gen_region;
	u8 		vdp_ctrlflag;
	u8 		vdp_code;
	u16		vdp_first;
	u16		vdp_second;
	u32		vdp_dmabytes;
	u16		vdp_address;
	u8		cpu68k_ram[0x10000];
	t_regs		regs;
	u8		cpuz80_ram[0x2000];
	u8		cpuz80_active;
	u8		cpuz80_resetting;
	u32		cpuz80_bank;
	Z80_Regs	cpuz80_regs;
	u32		cpuz80_after_EI;
	u8		YM2612_Regs[0x200];
	u32		PSG_Regs[8];
} t_pgenState;

typedef struct 
{
	struct {
		u32 magic;
		u16 version;
		u8 type;
		u8 pad;
		char gameName[64];
		int origSize;
	} header;

	// Buffer to hold compressed state. Should be more than will ever be needed.
	u8 buffer[sizeof(t_pgenState) + 64];

} t_pgenSaveBuffer;

class pgenEmuState
{
	public:

		pgenEmuState(const char *romName, unsigned short checksum);

		void save();
		int load();

		int saveState();
		int loadState();

		int saveSram();
		int loadSram();

	private:

		t_pgenState state;
		int empty;

		char gameName[64];
		char stateFilename[32];
		char sramFilename[32];
};

#endif
