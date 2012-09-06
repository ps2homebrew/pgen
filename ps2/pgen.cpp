
#include "pgen.h"
#include "zlib.h"
#include "unzip.h"
#include "amigamod.h"


t_pgenRuntimeSetting pgenRuntimeSetting = {
	0, 0, 0, 0, 0, 0,
	{
		PGEN_VERSION,
		3,		// Current region = Europe
		3,		// Default region = Europe
		1,		// Region autodetect = Yes
		0,		// Renderer: Line render
		0,		// Filter: Nearest
		1,		// Sound: On
		0,		// Mutlimode TV: No
		0,		// FPS counter: Off
		0,		// Save device: Memory card

		680, 37,	// PAL x/y offset
	},
	0,
	{
		,		//a = square
		,		//b = X
		,		//c = Circle
		,		//x = L1
		,		//y = R1
		,		//z = R2
		,		//start = Start
		,		//L2 = Mode
		,		//Select = Pause
	}
};

int pgenRunning;
volatile int pgenState = PGEN_STATE_GUI;
int pgenOptionsChanged = 0;

pgenRom *currentRom = NULL;

pgenSaver *currentSaver = NULL;
hddIO *saverHddAIO = NULL;
mcIO *saverMcAIO = NULL;
mcSaver *saverMC = NULL;
hddSaver *saverHDD = NULL;

#ifdef LIBCDVD_HOST
u8 cdvdIrx2[102400];
#endif

extern "C" void _init();
void initialise();
void loadModules();
void loadHddModules();
void postEmulationProcess();
void preEmulationProcess();
void powerOffCallback(void *arg);
int loadModuleBuffer(u8 *buffer, int size, int argc, char *argv);
void initMods();
void playNextMod();
void stopMod();

int main(int argc, char *argv[])
{
	// Init global objects (HACK!)
//	_init();

	int fadeIn = 0;

	initialise();
	gen_init();


	pgenOptionsLoad();
	guiUpdateDisplayPosition();

	while(1)
	{
		currentRom = pgenMenu(fadeIn);
		if(!currentRom->isRomLoaded())
		{
			guiError("Failed to load rom!");
			delete(currentRom);
			fadeIn = 0;
			continue;
		}

		CDVD_Stop();
		preEmulationProcess();

		while(pgenRunning)
		{
			gfxSync();
			updateIngameInput();
			event_doframe();
		}

		postEmulationProcess();
		delete(currentRom);
		fadeIn = 1;

	}

	return 0;
}

void initialise()
{
	SifInitRpc(0);

#ifdef LIBCDVD_HOST
	{
		int fd, fdSize;

		fd = fioOpen("host:libcdvd.irx", O_RDONLY);
		if(fd < 0)
		{
			printf("Failed to open libcdvd.irx\n");
			SleepThread();
		}

		fdSize = fioLseek(fd, 0, SEEK_END);
		fioLseek(fd, 0, SEEK_SET);

		if(fioRead(fd, cdvdIrx2, fdSize) != fdSize)
		{
			printf("Failed to read libcdvd.irx\n");
			SleepThread();
		}

		fioClose(fd);
	}
#endif

	SifIopReset("rom0:UDNL rom0:EELOADCNF",0);
       	while(!SifIopSync());
       	fioExit();
       	SifExitIopHeap();
       	SifLoadFileExit();
       	SifExitRpc();
       	SifExitCmd();

       	SifInitRpc(0);
       	FlushCache(0);
       	FlushCache(2);
	
	initGFX();
	guiDisplayLoading();
	sbv_patch_enable_lmb();
	sbv_patch_disable_prefix_check();
	loadModules();


	mtapInit();

	padInit(0);
	initPads();

	mcInit(MC_TYPE_XMC);

	CDVD_Init();
	initMods();
	playNextMod();

	loadHddModules();
	poweroffInit();
	poweroffSetCallback(&powerOffCallback, NULL);

	// Setup default region settings (will be over-written if a config file is found)
	if(((*((char*)0x1FC7FF52))=='E')+2 == NTSC)
		pgenRuntimeSetting.settings.defaultRegion = 
			pgenRuntimeSetting.settings.currentRegion = 2; // USA

	if(hddCheckPresent() < 0)
		pgenRuntimeSetting.HDDAvailable = 0;
	else
		pgenRuntimeSetting.HDDAvailable = 1;

	if(hddCheckFormatted() < 0)
		pgenRuntimeSetting.HDDFormatted = 0;
	else
		pgenRuntimeSetting.HDDFormatted = 1;

	guiInit();

	// Setup savers
	saverMcAIO = new mcIO(0, 2048);
	saverMC = new mcSaver("/PGEN11", saverMcAIO);
	currentSaver = saverMC;

	if(pgenRuntimeSetting.HDDFormatted)
	{
		// Cant create AIO yet, as then partition will be busy
		// when we try to check if it exists
		saverHDD = new hddSaver("/Saves", NULL);

		if(!saverHDD->checkSaveExist())
		{
			printf("hdd save doesnt exist!\n");
			saverHDD->createSave();
		}

		if(saverHDD->checkSaveExist())
		{
			printf("hdd save exists\n");
			saverHddAIO = new hddIO("hdd0:PP.P-GEN");
			saverHDD->saverAIO = saverHddAIO;
			currentSaver = saverHDD;
			pgenRuntimeSetting.settings.saveDevice = 1;
		}
	}

	// If we're not using the HDD, check if save exists on
	// memcard and create if required
	if(!currentSaver->checkSaveExist())
		currentSaver->createSave();

	guiDisplayLoadingEnd();
}


void loadModules()
{
//	char ifConfig[] = "192.168.0.23\000255.255.255.0\000192.168.0.1\000";

	loadModuleBuffer(freesio2Irx, 102400, 0, NULL);	
	loadModuleBuffer(freemtapIrx, 102400, 0, NULL);	
	loadModuleBuffer(freepadIrx, 102400, 0, NULL);
	loadModuleBuffer(xmcmanIrx, 102400, 0, NULL);
	loadModuleBuffer(xmcservIrx, 102400, 0, NULL);

	loadModuleBuffer(freesdIrx, 102400, 0, NULL);
//	SifLoadModule("rom0:LIBSD", 0, NULL);
#ifndef LIBCDVD_HOST
	loadModuleBuffer(cdvdIrx, 102400, 0, NULL);
#else
	loadModuleBuffer(cdvdIrx2, 102400, 0, NULL);
#endif
	loadModuleBuffer(sjpcmIrx, 102400, 0, NULL);
	loadModuleBuffer(amigamodIrx, 102400, 0, NULL);
	loadModuleBuffer(poweroffIrx, 102400 , 0, NULL);
	loadModuleBuffer(iomanXIrx, 102400, 0, NULL);
	loadModuleBuffer(fileXioIrx, 102400, 0, NULL);

	loadModuleBuffer(usbdIrx, size_usbdIrx, 0, NULL);
	loadModuleBuffer(usb_massIrx, size_usb_massIrx, 0, NULL);

}

void loadHddModules()
{
	// BUG: For an unknown reason, if these are not static then GCC will load the 
	//      strings onto the stack at the start of the function, then before the 3rd
	//      and 4th loadModuleBuffer calls (which get inlined). The stack area holding
	//      the strings gets trashed. Should investigate..
	static char hddarg[] = "-o" "\0" "4" "\0" "-n" "\0" "20";
	static char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "40";

	loadModuleBuffer(ps2dev9Irx, 102400, 0, NULL);
	if(loadModuleBuffer(ps2atadIrx, 102400, 0, NULL) >= 0)
	{
		loadModuleBuffer(ps2hddIrx, 102400, sizeof(hddarg), hddarg);
		loadModuleBuffer(ps2fsIrx, 102400, sizeof(pfsarg), pfsarg);
	}
}

void postEmulationProcess()
{
	pgenState = PGEN_STATE_GUI;

	if(pgenRuntimeSetting.settings.soundOn)
	{
		SjPCM_Setvol(0x0000);
		SjPCM_Quit();
		playNextMod();
	}

	guiFadeOutIngame();

	// This will save SRAM if necessary
	currentRom->state->saveSram();

	drawPipe->setFilterMethod(GS_FILTER_NEAREST);

	if(pgenOptionsChanged)
	{
		if(pgenOptionsSave() == 0)
			pgenOptionsChanged = 0;
	}

	if(pgenRuntimeSetting.settings.mutliModeTv)
	{
		gfxUpdateGuiVideoMode();

		// In case multi-mode is turned off
		pgenRuntimeSetting.gameVideoMode = pgenRuntimeSetting.guiVideoMode;
	}
}


void preEmulationProcess()
{
	if(pgenRuntimeSetting.settings.soundOn)
	{
		stopMod();
		SjPCM_Init(1);
		SjPCM_Setvol(0x3fff);
	}

	pgenRunning = 1;
	gfxRenderFrame = 1;

	// This will load SRAM if the ROM uses it, and a save is present
	// on the save device
	currentRom->state->loadSram();

	pgenState = PGEN_STATE_EMULATION;
}


void pgenSetRegion()
{
	if (pgenRuntimeSetting.settings.regionAutoDetect) {

		if((pgenRuntimeSetting.settings.defaultRegion == 2) && (gen_cartinfo.flag_usa)) gen_region = 2;
		else if((pgenRuntimeSetting.settings.defaultRegion == 3) && (gen_cartinfo.flag_europe)) gen_region = 3;
		else if((pgenRuntimeSetting.settings.defaultRegion == 0) && (gen_cartinfo.flag_japan)) gen_region = 0;
		else if(gen_cartinfo.flag_usa) gen_region = 2;
		else if(gen_cartinfo.flag_japan) gen_region = 0;
		else if(gen_cartinfo.flag_europe) gen_region = 3;
	}
	else
		gen_region = pgenRuntimeSetting.settings.currentRegion;
}

int loadModuleBuffer(u8 *buffer, int size, int argc, char *argv)
{
	void *iopMem;
	SifDmaTransfer_t dmaTrans;
	int i;
	int rv;

	iopMem = SifAllocIopHeap(size);

	dmaTrans.src = buffer;
	dmaTrans.dest = iopMem;
	dmaTrans.size = size;
	dmaTrans.attr = 0;
	i = SifSetDma(&dmaTrans, 1);
	while(SifDmaStat(i) >= 0);

	rv = SifLoadModuleBuffer(iopMem, argc, argv);

	SifFreeIopHeap(iopMem);

	return rv;
}

void powerOffCallback(void *arg)
{
	pgenState = PGEN_STATE_GUI;
	//poweroffShutdown();
}


int pgenOptionsSave()
{
	if(!currentSaver->checkSaveExist())
	{
		if(currentSaver->createSave() < 0)
			return -1;
	}

	char openFilename[256];
	if(!strcmp(currentSaver->savePath, "/"))
		strcpy(openFilename, "/settings.cfg");
	else
		sprintf(openFilename, "%s/settings.cfg", currentSaver->savePath);



	int fd = currentSaver->saverAIO->open(openFilename, O_WRONLY | O_RDWR | O_CREAT | O_TRUNC);
	if(fd < 0)
		return -1;

	if(currentSaver->saverAIO->write(fd, (u8 *)&pgenRuntimeSetting.settings, sizeof(t_pgenSettings)) 
		!= sizeof(t_pgenSettings))
	{
		currentSaver->saverAIO->close(fd);
		return -1;
	}

	currentSaver->saverAIO->close(fd);

	return 0;
}

int pgenOptionsLoad()
{
	t_pgenSettings localSettings;

	char openFilename[256];
	if(!strcmp(currentSaver->savePath, "/"))
		strcpy(openFilename, "/settings.cfg");
	else
		sprintf(openFilename, "%s/settings.cfg", currentSaver->savePath);

	int fd = currentSaver->saverAIO->open(openFilename, O_RDONLY);
	if(fd < 0)
		return -1;

	int fdSize = currentSaver->saverAIO->lseek(fd, 0, SEEK_END);
	currentSaver->saverAIO->lseek(fd, 0, SEEK_SET);
	if(fdSize != sizeof(t_pgenSettings))
		return -1;

	if(currentSaver->saverAIO->read(fd, (u8 *)&localSettings, sizeof(t_pgenSettings)) != 
		sizeof(t_pgenSettings))
	{
		currentSaver->saverAIO->close(fd);
		return -1;
	}

	currentSaver->saverAIO->close(fd);

	if(localSettings.version < PGEN_COMPAT_VER)
		return -1;

	memcpy((void *)&pgenRuntimeSetting.settings, (void *)&localSettings, sizeof(t_pgenSettings));

	return 0;
}


//datafileIO modZipAIO(musicZip, MUSIC_ZIP_SIZE);
datafileIO *modZipAIO;
unzFile modZip;

void initMods()
{
	modZipAIO = new datafileIO(musicZip, size_musicZip);
	aioSetCurrent(modZipAIO);

	modZip = unzOpen("");
	unzGoToFirstFile(modZip);
	aioSetCurrent(NULL);
}

void playNextMod()
{
	aioSetCurrent(modZipAIO);

	char filename[144];
	unz_file_info info;

	unzGetCurrentFileInfo(modZip, &info, filename, 128, NULL,0, NULL,0);
	unzOpenCurrentFile(modZip);

	int modSize = info.uncompressed_size;
	u8 *modBuffer = (u8 *)memalign(64, modSize);
	if(!modBuffer)
		guiFatalError("Failed to allocate memory!");

	unzReadCurrentFile(modZip, modBuffer, modSize);
	unzCloseCurrentFile(modZip);

	amigaModInit(0);
	amigaModPause();
	amigaModLoad(modBuffer, modSize);
	amigaModPlay(1);
	amigaModSetVolume(0x3fff);	

	if(unzGoToNextFile(modZip) == UNZ_END_OF_LIST_OF_FILE)
		unzGoToFirstFile(modZip);

	aioSetCurrent(NULL);
	free(modBuffer);
}

void stopMod()
{
	amigaModSetVolume(0x0000);
	amigaModQuit();
}

// Make the Generator core happy..

extern "C" void ui_err(const char *text, ...) { 

	static char buff[4096];
    va_list args;

	va_start(args, text);
	vsnprintf(buff, 4096, text, args);

	guiError(buff);

	pgenRunning = 0;
}

extern "C" void ui_log_debug3(const char *text, ...) { }
extern "C" void ui_log_debug2(const char *text, ...) { }
extern "C" void ui_log_debug1(const char *text, ...) { }
extern "C" void ui_log_user(const char *text, ...) { }
extern "C" void ui_log_verbose(const char *text, ...) { }
extern "C" void ui_log_normal(const char *text, ...) { }
extern "C" void ui_log_critical(const char *text, ...) { }
extern "C" void ui_log_request(const char *text, ...) { }
extern "C" void ui_musiclog(uint8 *data, unsigned int length) { }

// C++ hacks

//extern "C" void __cxa_pure_virtual() {}
//extern "C" void _impure_ptr() {}
//extern "C" void fwrite() {}

//void * __builtin_new(size_t size) { return malloc(size); }
//void __builtin_delete(void *ptr) { free(ptr); }
void abort() {}
