#ifndef _GFX_H
#define _GFX_H

typedef struct
{
	u32 texBufPtr;
	int texBufWidth;
	int psm;
	int width, height;
	const unsigned char *texPtr;
} t_gfxTexture;

typedef struct
{
	t_gfxTexture pgenBG;			// 320x240x32bpp
	t_gfxTexture ocraFont;			// 192x256x32bpp
	t_gfxTexture zeroHourFont;		// 256x128x32bpp
	t_gfxTexture pgenFolder;		// 16x16x32bpp
	t_gfxTexture emuDisplayTex;		// 320x256x8bpp
	t_gfxTexture emuDisplayClut;	// 256x1x16bpp
} t_pgenTexEnv;

extern t_pgenTexEnv pgenTexEnv;

void initGFX();
void gfxSync();
void gfxUploadTextures();
void gfxApplyFilterSetting();
void gfxUpdateIngameDisplay(int flush);
void gfxUpdateGuiVideoMode();
void gfxUpdateIngameVideoMode();
void gfxChangeDefaultVideoMode(int vidMode);

extern gsFont ocraFont;
extern gsFont zerohourFont;

extern gsPipe *drawPipe;
extern gsDriver* dispDriver;

extern volatile int gfxSkipNextFrame;
extern int gfxRenderFrame;
extern int gfxRenderSimple;

#endif /* _GFX_H */
