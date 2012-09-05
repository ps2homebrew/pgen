#ifndef _PGEN_VARS_H
#define _PGEN_VARS_H

enum bootMode { BOOT_CDVD = 1, BOOT_DMS3 = 2, BOOT_HDD = 3 };

// Structure will be used directly in the options save/load code.
typedef struct
{
	u16 version;
	u8 currentRegion;			// 0 = JAP, 1 = Invalid, 2 = USA, 3 = Europe
	u8 defaultRegion;			// As above
	u8 regionAutoDetect;		// 0 = No, 1 = Yes
	u8 currentRenderer;			// 0 = Line, 1 = Cell
	u8 renderFilter;			// 0 = Nearest, 1 = Linear
	u8 soundOn;					// 0 = Sound off, 1 = Sound on
	u8 mutliModeTv;				// 0 = No, 1 = Yes
	u8 displayFps;				// 0 = Off, 1 = On
	u8 saveDevice;				// 0 = Memory Card, 1 = HDD
	
	u16 dispX, dispY;		// X & Y offset parameters for both PAL and NTSC display modes.

} t_pgenSettings;

typedef struct
{
	int HDDAvailable, HDDFormatted;

	int gameVideoMode, guiVideoMode;
	int maxFrameSec;

	int multiTapConnected;

	t_pgenSettings settings;

} t_pgenRuntimeSetting;

extern t_pgenRuntimeSetting pgenRuntimeSetting;
extern int pgenRunning;
extern int pgenOptionsChanged;
extern volatile int pgenState;

// Hack, so core C code doesnt complain
#ifdef __cplusplus
extern pgenRom *currentRom;

extern pgenSaver *currentSaver;
extern mcSaver *saverMC;
extern hddSaver *saverHDD;
extern hddIO *saverHddAIO;
extern mcIO *saverMcAIO;
#endif

#endif /* _PGEN_VARS_H */
