#include "pgen.h"

#undef HAVE_UNISTD_H
#include "unzip.h"

pgenRom::pgenRom(abstractIO *aio, const char *filename)
{
	romData = NULL;
	state = NULL;
	romSize = 0;
	loaded = 0;

	char *extension = strrchr(filename, '.');
	if(!extension)
		return;

	if(!strcasecmp(extension, ".zip"))
		readZippedRom(aio, filename);
	else
		readRom(aio, filename);

	// If load failed, bail
	if(romData == NULL)
		return;

	if(loadRom() < 0)
		return;

	char romName[64];
	char *romNameStart = strrchr(filename, '/');
	strncpy(romName, ++romNameStart, 64);
	romName[63] = '\0';
	extension = strrchr(romName, '.');
	if(extension)
		*extension = '\0';

	state = new pgenEmuState(romName, gen_cartinfo.checksum);
}

pgenRom::~pgenRom()
{
	if(romData)
		free(romData);
	if(state)
		delete(state);
}

int pgenRom::loadRom()
{
	int imagetype;
	unsigned int blocks, x, i;
	u8 *newBuf;

	cpu68k_rom = romData;
	cpu68k_romlen = romSize;

	if(cpu68k_romlen < 0x200)
	{
		guiError("File is too small");
		return -1;
	}

	imagetype = 1;                // BIN file by default

	// SMD file format check - Richard Bannister
	if((cpu68k_rom[8] == 0xAA) && (cpu68k_rom[9] == 0xBB) &&
		cpu68k_rom[10] == 0x06) {
			imagetype = 2;              // SMD file
	}

	// check for interleaved 'SEGA'
	if(cpu68k_rom[0x280] == 'E' && cpu68k_rom[0x281] == 'A' &&
		cpu68k_rom[0x2280] == 'S' && cpu68k_rom[0x2281] == 'G') {
			imagetype = 2;              // SMD file
	}

	// convert to standard BIN file format
	switch (imagetype) {

		case 1:                      // BIN
			break;
		case 2:                      // SMD

			blocks = (cpu68k_romlen - 512) / 16384;
			if(blocks * 16384 + 512 != cpu68k_romlen) {
				guiError("Image is corrupt.");
				return -1;
			}

			if((newBuf = (unsigned char *)malloc(cpu68k_romlen - 512)) == NULL) {
				cpu68k_rom = NULL;
				cpu68k_romlen = 0;
				guiError("Out of memory!");
				return -1;
			}

			for(i = 0; i < blocks; i++) {
				for(x = 0; x < 8192; x++) {
					newBuf[i * 16384 + x * 2 + 0] = cpu68k_rom[512 + i * 16384 + 8192 + x];
					newBuf[i * 16384 + x * 2 + 1] = cpu68k_rom[512 + i * 16384 + x];
				}
			}

			free(romData);
			romData = newBuf;
			romSize -= 512;
			cpu68k_rom = newBuf;
			cpu68k_romlen -= 512;

			break;
		default:
			guiError("Unknown image type");
			return -1;
	}

	gen_setupcartinfo();

	// Setup defaults for IO devices
	gen_sixcont = 0;
	gen_multitap = 0;

	// scan IO info section in rom header
	for(i = 0x190; i < 0x1A0; i++)
	{
		if(cpu68k_rom[i] == '6') gen_sixcont = 1;
		if((cpu68k_rom[i] == '4') && pgenRuntimeSetting.multiTapConnected) gen_multitap = 1;
	}

	pgenSetRegion();

	// If "multi-mode tv" is enabled, change video mode accoridng to region of game
	if(pgenRuntimeSetting.settings.mutliModeTv)
	{
		switch(gen_region)
		{
			case 0:
			case 2:
				pgenRuntimeSetting.gameVideoMode = NTSC;
				gfxUpdateIngameVideoMode();
				break;
			case 3:
				pgenRuntimeSetting.gameVideoMode = PAL;
				gfxUpdateIngameVideoMode();
				break;
		}
	}

	// reset system
	gen_reset();

	// fix checksum, if broken
	if(gen_cartinfo.checksum != (cpu68k_rom[0x18e] << 8 | cpu68k_rom[0x18f])) {
		cpu68k_rom[0x18e] = gen_cartinfo.checksum >> 8;
		cpu68k_rom[0x18f] = gen_cartinfo.checksum & 0xff;
	}

	loaded = 1;

	return 0;
}

void pgenRom::readRom(abstractIO *aio, const char *filename)
{
	int fd = aio->open(filename, O_RDONLY);
	if(fd < 0)
		return;

	int fdSize = aio->lseek(fd, 0, SEEK_END);
	aio->lseek(fd, 0, SEEK_SET);

	romData = (u8 *)memalign(64, fdSize);
	if(!romData)
	{
		printf("Failed to allocate memory for rom!\n");
		return;
	}

	if(aio->read(fd, romData, fdSize) != fdSize)
		goto loadRom_error;

	aio->close(fd);

	romSize = fdSize;
	return;

loadRom_error:

	free(romData);
	romData = NULL;
	return;	
}

// readZippedRom is based off code by Hiryu
void pgenRom::readZippedRom(abstractIO *aio, const char *zipname)
{
	aioSetCurrent(aio);

	unzFile file = unzOpen(zipname);
	if(file == NULL)
		return;

	char filename[144];
	int port = unzGoToFirstFile(file);
	unz_file_info info;
	int readSize;

	while(port == UNZ_OK)
	{
		unzGetCurrentFileInfo(file, &info, filename, 128, NULL,0, NULL,0);

		const char *extension = strrchr(filename, '.');

		if(extension != NULL)
		{
			if(!strcasecmp(extension, ".bin"))
				break;

			if(!strcasecmp(extension, ".gen"))
				break;

			if(!strcasecmp(extension, ".smd"))
				break;
		}
 
		port = unzGoToNextFile(file);
    }

    if(port != UNZ_OK)
		goto readZippedRom_error;

	romSize = info.uncompressed_size;
	romData = (u8 *)memalign(64, romSize);
	if(!romData)
		goto readZippedRom_error;

    unzLocateFile(file, filename, 1);
    unzGetCurrentFileInfo(file, &info, filename,128, NULL,0, NULL,0);
    
    if( unzOpenCurrentFile(file) != UNZ_OK )
		goto readZippedRom_error;

	readSize = unzReadCurrentFile(file, romData, romSize);
	if(unzCloseCurrentFile(file) == UNZ_CRCERROR)
		goto readZippedRom_error;
		
	if(readSize != romSize)
		goto readZippedRom_error;

    unzClose(file);

	aioSetCurrent(NULL);
    return;	

readZippedRom_error:

	if(romData)
		free(romData);
	romData = NULL;
	unzClose(file);
	aioSetCurrent(NULL);
	return;
}

int pgenRom::isRomLoaded()
{
	return loaded;
}
