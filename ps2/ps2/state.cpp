#include "pgen.h"

#include "generator.h"
#include "vdp.h"
#include "cpuz80.h"
#include "ym2612.h"
#include "psg.h"

#undef HAVE_UNISTD_H
#include "zlib.h"

pgenEmuState::pgenEmuState(const char *romName, unsigned short checksum)
{
	memset(&state, 0, sizeof(t_pgenState));
	empty = 1;

	strncpy(gameName, romName, 64);
	gameName[63] = '\0';

	unsigned short romnameChecksum = 0;
	for(int i = 0; i < (int)strlen(gameName); i++)
		romnameChecksum += (unsigned char)~gameName[i];

	sprintf(stateFilename, "%04X-%04X.pgs", romnameChecksum, checksum);
	sprintf(sramFilename, "%04X-%04X.pgr", romnameChecksum, checksum);
}

void pgenEmuState::save()
{
	empty = 0;

	memcpy(state.vdp_vram, vdp_vram, 64*1024);
	memcpy(state.vdp_cram, vdp_cram, 128);
	memcpy(state.vdp_vsram, vdp_vsram, 80);
	memcpy(state.vdp_reg, vdp_reg, 25);
	state.gen_region = gen_region;
	state.vdp_ctrlflag = vdp_ctrlflag;
	state.vdp_code = (uint8)vdp_code;
	state.vdp_first = vdp_first;
	state.vdp_second = vdp_second;
	state.vdp_dmabytes = vdp_dmabytes;
	state.vdp_address = vdp_address;
	memcpy(state.cpu68k_ram, cpu68k_ram, 0x10000);
	memcpy(&state.regs, &regs, sizeof(t_regs));
	memcpy(state.cpuz80_ram, cpuz80_ram, 0x2000);
	state.cpuz80_active = cpuz80_active;
	state.cpuz80_resetting = cpuz80_resetting;
	state.cpuz80_bank = cpuz80_bank;
	memcpy(&state.cpuz80_regs, Z80_Context, sizeof(Z80_Regs));
	state.cpuz80_after_EI = after_EI;

	YM2612_Save(state.YM2612_Regs);

	PSG_Save_State();
	memcpy(state.PSG_Regs, PSG_Save, 32);	
}

int pgenEmuState::load()
{
	if(empty)
		return -1;

	memcpy(vdp_vram, state.vdp_vram, 64*1024);
	memcpy(vdp_cram, state.vdp_cram, 128);
	memcpy(vdp_vsram, state.vdp_vsram, 80);
	memcpy(vdp_reg, state.vdp_reg, 25);
	gen_region = state.gen_region;
	vdp_code = (enum t_code)state.vdp_code;
	vdp_first = state.vdp_first;
	vdp_second = state.vdp_second;
	vdp_dmabytes = state.vdp_dmabytes;
	vdp_address = state.vdp_address;
	memcpy(cpu68k_ram, state.cpu68k_ram, 0x10000);
	memcpy(&regs, &state.regs, sizeof(t_regs));
	memcpy(cpuz80_ram, state.cpuz80_ram, 0x2000);
	cpuz80_active = state.cpuz80_active;
	cpuz80_resetting = state.cpuz80_resetting;
	cpuz80_bank = state.cpuz80_bank;
	memcpy(Z80_Context, &state.cpuz80_regs, sizeof(Z80_Regs));
	after_EI = state.cpuz80_after_EI;

	YM2612_Restore(state.YM2612_Regs);

	memcpy(PSG_Save, state.PSG_Regs, 32);
	PSG_Restore_State();
	return 0;
}



int pgenEmuState::saveState()
{
	int rv;
	t_pgenSaveBuffer saveBuffer;

	if(!currentSaver->checkSaveExist())
	{
		if(currentSaver->createSave() < 0)
			return STATE_ERROR_CREATE_SAVE;
	}

	save();

	// Fill in header
	saveBuffer.header.magic = PGEN_SAVE_MAGIC;
	saveBuffer.header.version = PGEN_VERSION;
	saveBuffer.header.type = SAVE_TYPE_STATE;
	strcpy(saveBuffer.header.gameName, gameName);
	saveBuffer.header.origSize = sizeof(t_pgenState);

	// Compress state
	int compressedSize = sizeof(saveBuffer.buffer);

	rv = compress2(saveBuffer.buffer, (uLongf *)&compressedSize, (u8 *)&state, sizeof(t_pgenState), 9);
	if(rv != Z_OK)
		return STATE_ERROR_SAVE_STATE;

	// Write file
	char openFilename[256];
	if(!strcmp(currentSaver->savePath, "/"))
		sprintf(openFilename, "/%s", stateFilename);
	else
		sprintf(openFilename, "%s/%s", currentSaver->savePath, stateFilename);

	int outSize = compressedSize + sizeof(saveBuffer.header);
	int fd = currentSaver->saverAIO->open(openFilename, O_WRONLY | O_RDWR | O_CREAT | O_TRUNC);
	if(fd < 0)
		return STATE_ERROR_SAVE_OPEN;

	rv = currentSaver->saverAIO->write(fd, (u8 *)&saveBuffer, outSize);
	if(rv != outSize)
	{
		currentSaver->saverAIO->close(fd);
		return STATE_ERROR_SAVE_WRITE;
	}

	currentSaver->saverAIO->close(fd);
	return 0;
}

int pgenEmuState::loadState()
{
	int rv;
	t_pgenSaveBuffer saveBuffer;

	// Read in file
	char openFilename[256];
	if(!strcmp(currentSaver->savePath, "/"))
		sprintf(openFilename, "/%s", stateFilename);
	else
		sprintf(openFilename, "%s/%s", currentSaver->savePath, stateFilename);

	int fd = currentSaver->saverAIO->open(openFilename, O_RDONLY);
	if(fd < 0)
		return STATE_ERROR_STATE_NOT_FOUND;

	int fdSize = currentSaver->saverAIO->lseek(fd, 0, SEEK_END);
	currentSaver->saverAIO->lseek(fd, 0, SEEK_SET);
	int compressedSize = fdSize - sizeof(saveBuffer.header);
	if(currentSaver->saverAIO->read(fd, (u8 *)&saveBuffer, fdSize) != fdSize)
	{
		currentSaver->saverAIO->close(fd);
		return STATE_ERROR_LOAD_STATE;
	}
		
	currentSaver->saverAIO->close(fd);

	// Check header
	if((saveBuffer.header.version < PGEN_COMPAT_VER) || (saveBuffer.header.magic != PGEN_SAVE_MAGIC))
		return STATE_ERROR_VERSION;

	// Uncompress state
	int destLen = sizeof(saveBuffer.buffer);
	rv = uncompress((u8 *)&state, (uLongf *)&destLen, saveBuffer.buffer, compressedSize);
	if(rv != Z_OK)
		return STATE_ERROR_LOAD_STATE;

	// Load state
	empty = 0;
	load();
	return 0;
}

int pgenEmuState::saveSram()
{
	int rv;
	t_pgenSaveBuffer saveBuffer;

	// Only attempt to save if ROM uses SRAM
	if(!cpu68k_sramactive)
		return 0;

	if(!currentSaver->checkSaveExist())
	{
		if(currentSaver->createSave() < 0)
			return STATE_ERROR_CREATE_SAVE;
	}

	// Fill in header
	saveBuffer.header.magic = PGEN_SAVE_MAGIC;
	saveBuffer.header.version = PGEN_VERSION;
	saveBuffer.header.type = SAVE_TYPE_SRAM;
	strcpy(saveBuffer.header.gameName, gameName);
	saveBuffer.header.origSize = cpu68k_sramlen;

	// Compress SRAM
	int compressedSize;

	rv = compress2(saveBuffer.buffer, (uLongf *)&compressedSize, (u8 *)cpu68k_srambuff, cpu68k_sramlen, 9);
	if(rv != Z_OK)
		return STATE_ERROR_SAVE_STATE;

	// Write file
	char openFilename[256];
	if(!strcmp(currentSaver->savePath, "/"))
		sprintf(openFilename, "/%s", sramFilename);
	else
		sprintf(openFilename, "%s/%s", currentSaver->savePath, sramFilename);

	int outSize = compressedSize + sizeof(saveBuffer.header);
	int fd = currentSaver->saverAIO->open(openFilename, O_RDWR | O_CREAT | O_TRUNC);
	if(fd < 0)
		return STATE_ERROR_SAVE_STATE;

	rv = currentSaver->saverAIO->write(fd, (u8 *)&saveBuffer, outSize);
	if(rv != outSize)
	{
		currentSaver->saverAIO->close(fd);
		return STATE_ERROR_SAVE_STATE;
	}

	currentSaver->saverAIO->close(fd);

	return 0;
}

int pgenEmuState::loadSram()
{
	int rv;
	t_pgenSaveBuffer saveBuffer;

	// Only attempt to load if ROM uses SRAM
	if(!cpu68k_sramactive)
		return 0;

	// Read in file
	char openFilename[256];
	if(!strcmp(currentSaver->savePath, "/"))
		sprintf(openFilename, "/%s", sramFilename);
	else
		sprintf(openFilename, "%s/%s", currentSaver->savePath, sramFilename);

	// Read in file
	int fd = currentSaver->saverAIO->open(openFilename, O_RDONLY);
	if(fd < 0)
		return STATE_ERROR_LOAD_STATE;

	int fdSize = currentSaver->saverAIO->lseek(fd, 0, SEEK_END);
	currentSaver->saverAIO->lseek(fd, 0, SEEK_SET);
	int compressedSize = fdSize - sizeof(saveBuffer.header);

	if(currentSaver->saverAIO->read(fd, (u8 *)&saveBuffer, fdSize) != fdSize)
	{
		currentSaver->saverAIO->close(fd);
		return STATE_ERROR_LOAD_STATE;
	}
	
	currentSaver->saverAIO->close(fd);

	// Check header
	if((saveBuffer.header.version < PGEN_COMPAT_VER) || (saveBuffer.header.magic != PGEN_SAVE_MAGIC))
		return STATE_ERROR_VERSION;
	
	// Uncompress SRAM
	int destLen;
	rv = uncompress((u8 *)cpu68k_srambuff, (uLongf *)&destLen, saveBuffer.buffer, compressedSize);
	if(rv != Z_OK)
		return STATE_ERROR_LOAD_STATE;

	return 0;
}
