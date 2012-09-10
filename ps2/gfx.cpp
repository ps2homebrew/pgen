#include "pgen.h"
#include "vdp.h"

void paletteUpdate();
void gfxUpdateIngameDisplay();
void gfxVsyncCb();
void gfxUploadTextures();
void gfxInitFonts();
void gfxAllocTextures();
void gfxSetPal();
void gfxSetNtsc();

t_pgenTexEnv pgenTexEnv;

u8 padding1[2048];
u8 displayBuffer[512*256] __attribute__((aligned(64)));
u8 padding2[2048];
u16 paletteBuffer[256] __attribute__((aligned(64)));

gsFont ocraFont, zerohourFont;
gsPipe *drawPipe;
gsDriver* dispDriver;

volatile int gfxSkipNextFrame = 0;
int gfxRenderFrame = 1;
volatile int framesPerSecond;

void gfxAllocTextures()
{
	u32 texOffset = dispDriver->getTextureBufferBase();

//	printf("texOffset = 0x%X\n", texOffset);

	pgenTexEnv.pgenBG.texBufPtr = texOffset;
	pgenTexEnv.pgenBG.texBufWidth = 512 /* 320 */;
	pgenTexEnv.pgenBG.psm = pgenbgIIF.psm;
	pgenTexEnv.pgenBG.width = pgenbgIIF.width;
	pgenTexEnv.pgenBG.height = pgenbgIIF.height;
	pgenTexEnv.pgenBG.texPtr = (const unsigned char *)&pgenbgIIF.pixel_data;

	texOffset += pgenTexEnv.pgenBG.texBufWidth * /* pgenTexEnv.pgenBG.height */ 256
					* dispDriver->getBytesPerPixel(pgenTexEnv.pgenBG.psm);
	while(texOffset % 256) texOffset++;

//	printf("texOffset = 0x%X\n", texOffset);

	pgenTexEnv.ocraFont.texBufPtr = texOffset;
	pgenTexEnv.ocraFont.texBufWidth = 256 /* 192 */;
	pgenTexEnv.ocraFont.psm = ocraFnt.PSM;
	pgenTexEnv.ocraFont.width = ocraFnt.TexWidth;
	pgenTexEnv.ocraFont.height = ocraFnt.TexHeight;
	pgenTexEnv.ocraFont.texPtr = (const unsigned char *)&ocraFnt.PixelData;

	texOffset += pgenTexEnv.ocraFont.texBufWidth * pgenTexEnv.ocraFont.height
					* dispDriver->getBytesPerPixel(pgenTexEnv.ocraFont.psm);
	while(texOffset % 256) texOffset++;

//	printf("texOffset = 0x%X\n", texOffset);

	pgenTexEnv.zeroHourFont.texBufPtr = texOffset;
	pgenTexEnv.zeroHourFont.texBufWidth = 256;
	pgenTexEnv.zeroHourFont.psm = zerohourFnt.PSM;
	pgenTexEnv.zeroHourFont.width = zerohourFnt.TexWidth;
	pgenTexEnv.zeroHourFont.height = zerohourFnt.TexHeight;
	pgenTexEnv.zeroHourFont.texPtr = (const unsigned char *)&zerohourFnt.PixelData;

	texOffset += pgenTexEnv.zeroHourFont.texBufWidth * pgenTexEnv.zeroHourFont.height
					* dispDriver->getBytesPerPixel(pgenTexEnv.zeroHourFont.psm);
	while(texOffset % 256) texOffset++;

//	printf("texOffset = 0x%X\n", texOffset);

	pgenTexEnv.pgenFolder.texBufPtr = texOffset;
	pgenTexEnv.pgenFolder.texBufWidth = 16;
	pgenTexEnv.pgenFolder.psm = folderIIF.psm;
	pgenTexEnv.pgenFolder.width = folderIIF.width;
	pgenTexEnv.pgenFolder.height = folderIIF.height;
	pgenTexEnv.pgenFolder.texPtr = (const unsigned char *)&folderIIF.pixel_data;

	texOffset += pgenTexEnv.pgenFolder.texBufWidth * pgenTexEnv.pgenFolder.height
					* dispDriver->getBytesPerPixel(pgenTexEnv.pgenFolder.psm);
	while(texOffset % 256) texOffset++;
	
//	printf("texOffset = 0x%X\n", texOffset);

	pgenTexEnv.emuDisplayTex.texBufPtr = texOffset;
	pgenTexEnv.emuDisplayTex.texBufWidth = 512;
	pgenTexEnv.emuDisplayTex.psm = GS_PSMT8;
	pgenTexEnv.emuDisplayTex.width = 512;
	pgenTexEnv.emuDisplayTex.height = 256;
	pgenTexEnv.emuDisplayTex.texPtr = (const unsigned char *)displayBuffer;

	texOffset += pgenTexEnv.emuDisplayTex.texBufWidth * pgenTexEnv.emuDisplayTex.height
					* dispDriver->getBytesPerPixel(pgenTexEnv.emuDisplayTex.psm);
	while(texOffset % 256) texOffset++;

//	printf("texOffset = 0x%X\n", texOffset);

	pgenTexEnv.emuDisplayClut.texBufPtr = texOffset;
	pgenTexEnv.emuDisplayClut.texBufWidth = 256;
	pgenTexEnv.emuDisplayClut.psm = GS_PSMCT16;
	pgenTexEnv.emuDisplayClut.width = 256;
	pgenTexEnv.emuDisplayClut.height = 1;
	pgenTexEnv.emuDisplayClut.texPtr = (const unsigned char *)paletteBuffer;
}

void gfxInitFonts()
{
	ocraFont.assignPipe(drawPipe);
	zerohourFont.assignPipe(drawPipe);
}

void gfxUploadTextures()
{
	drawPipe->TextureUpload(pgenTexEnv.pgenBG.texBufPtr, pgenTexEnv.pgenBG.texBufWidth, 0, 0,
		pgenTexEnv.pgenBG.psm, pgenTexEnv.pgenBG.texPtr, pgenTexEnv.pgenBG.width, 
		pgenTexEnv.pgenBG.height);

	drawPipe->TextureUpload(pgenTexEnv.pgenFolder.texBufPtr, pgenTexEnv.pgenFolder.texBufWidth, 0, 0,
		pgenTexEnv.pgenFolder.psm, pgenTexEnv.pgenFolder.texPtr, pgenTexEnv.pgenFolder.width, 
		pgenTexEnv.pgenFolder.height);

	ocraFont.uploadFont(&ocraFnt, pgenTexEnv.ocraFont.texBufPtr, pgenTexEnv.ocraFont.texBufWidth, 0, 0);

	zerohourFont.uploadFont(&zerohourFnt, pgenTexEnv.zeroHourFont.texBufPtr, pgenTexEnv.zeroHourFont.texBufWidth, 0, 0);
}

void gfxVsyncCb()
{
	static int vrCount = 0;
	static int localFrameCount;

	if(pgenState != PGEN_STATE_EMULATION)
		goto gfxVsyncCb_end;

	if(dispDriver->isDisplayBufferAvailable())
	{
		dispDriver->DisplayNextFrame();

		localFrameCount++;
		gfxSkipNextFrame = 0;
	}
	else
		gfxSkipNextFrame = 1;

	vrCount++;
	switch(pgenRuntimeSetting.gameVideoMode)
	{
		case PAL:
			if(vrCount >= 50)
			{
				framesPerSecond = localFrameCount;
				localFrameCount = 0;
				vrCount = 0;
			}
			break;

		case NTSC:
		case VGA640_60:
			if(vrCount >= 60)
			{
				framesPerSecond = localFrameCount;
				localFrameCount = 0;
				vrCount = 0;
			}
			break;
	}

gfxVsyncCb_end:

	asm __volatile__ ("ei");
}

void initGFX()
{
	int defaultVidMode = ((*((char*)0x1FC7FF52))=='E')+2;

	dispDriver = new gsDriver(PAL);
	drawPipe = &dispDriver->drawPipe;

	pgenRuntimeSetting.gameVideoMode = defaultVidMode;
	pgenRuntimeSetting.guiVideoMode = defaultVidMode;	
	switch(defaultVidMode)
	{
		case PAL:

			pgenRuntimeSetting.maxFrameSec = 50;

			break;

		case NTSC:
		case VGA640_60:

			pgenRuntimeSetting.maxFrameSec = 60;

			break;
	}
	
	gfxUpdateGuiVideoMode();

	drawPipe->setAlphaEnable(GS_ENABLE);
	dispDriver->AddVSyncCallback(gfxVsyncCb);
	drawPipe->setFilterMethod(GS_FILTER_NEAREST);

	gfxAllocTextures();
	gfxInitFonts();
	gfxUploadTextures();
}

void gfxChangeDefaultVideoMode(int vidMode)
{
	pgenRuntimeSetting.gameVideoMode = vidMode;
	pgenRuntimeSetting.guiVideoMode = vidMode;
	switch(vidMode)
	{
		case PAL:

			pgenRuntimeSetting.maxFrameSec = 50;

			break;

		case NTSC:
		case VGA640_60:

			pgenRuntimeSetting.maxFrameSec = 60;

			break;
	}

	gfxUpdateGuiVideoMode();
}

void gfxUpdateIngameDisplay(int flush)
{
	// Clear the screen (with ZBuffer Disabled)
	drawPipe->setZTestEnable(GS_DISABLE);
	drawPipe->RectFlat(0,0,320,240,0,GS_SET_RGBA(0,0,0,0x80));
	drawPipe->setZTestEnable(GS_ENABLE);

	// Upload Texture
	drawPipe->TextureUpload(pgenTexEnv.emuDisplayTex.texBufPtr, pgenTexEnv.emuDisplayTex.texBufWidth, 0, 0,
		pgenTexEnv.emuDisplayTex.psm, pgenTexEnv.emuDisplayTex.texPtr, pgenTexEnv.emuDisplayTex.width,
		pgenTexEnv.emuDisplayTex.height);

	// Upload Clut
	drawPipe->TextureUpload(pgenTexEnv.emuDisplayClut.texBufPtr, pgenTexEnv.emuDisplayClut.texBufWidth, 0, 0,
		pgenTexEnv.emuDisplayClut.psm, pgenTexEnv.emuDisplayClut.texPtr, pgenTexEnv.emuDisplayClut.width, 
		pgenTexEnv.emuDisplayClut.height);

	// Set the current texture to the emulated display tex + clut
	drawPipe->TextureFlush();
	gfxApplyFilterSetting();
	drawPipe->TextureSet(pgenTexEnv.emuDisplayTex.texBufPtr, 
		pgenTexEnv.emuDisplayTex.texBufWidth, 
		dispDriver->getTexSizeFromInt(pgenTexEnv.emuDisplayTex.width), 
		dispDriver->getTexSizeFromInt(pgenTexEnv.emuDisplayTex.height), 
		pgenTexEnv.emuDisplayTex.psm, pgenTexEnv.emuDisplayClut.texBufPtr, GS_CSM2, 
		pgenTexEnv.emuDisplayClut.texBufWidth, pgenTexEnv.emuDisplayClut.psm);

	drawPipe->setFilterMethod(GS_FILTER_NEAREST);

	// Draw the textured poly
	if(pgenRuntimeSetting.settings.currentRenderer) 
	{
		if(vdp_reg[12] & 1)
			drawPipe->RectTexture(0, 0, 8, 8, 320, vdp_vislines, 320 + 8, vdp_vislines + 8, 1, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
		else
			drawPipe->RectTexture(32, 0, 8, 8, 288, vdp_vislines, 256 + 8, vdp_vislines + 8, 1, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
	}
	else {
		if(vdp_reg[12] & 1)
			drawPipe->RectTexture(0, 0, 16, 8, 320, vdp_vislines, 320 + 16, vdp_vislines + 8, 1, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
		else
			drawPipe->RectTexture(32, 0, 48, 8, 288, vdp_vislines, 256 + 48, vdp_vislines + 8, 1, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
	}

	// Draw framecount
	if(pgenRuntimeSetting.settings.displayFps)
	{
		char fpsString[32];
		int maxFps = 0, fpsYPos = 0;

		switch(pgenRuntimeSetting.gameVideoMode)
		{
			case PAL:
				maxFps = 50;
				fpsYPos = 224;
				break;
			case NTSC:
				maxFps = 60;
				fpsYPos = 208;
				break;
			case VGA640_60:
				maxFps = 60;
				fpsYPos = 448;
				break;
		}

		sprintf(fpsString, "%d/%d", framesPerSecond, maxFps);
		ocraFont.Print(30, 320, fpsYPos, 2, GS_SET_RGBA(255, 255, 255, 180), GSFONT_ALIGN_LEFT, fpsString);
	}

	if(flush)
	{
		drawPipe->Flush();
		dispDriver->DrawBufferComplete();
		while(!(dispDriver->isDrawBufferAvailable()));
		dispDriver->setNextDrawBuffer();
//		dispDriver->WaitForVSync();
	}
}

extern "C" void ui_line(int line)
{

	if(pgenRuntimeSetting.settings.currentRenderer || !gfxRenderFrame)
	{
		return;
	}
	if(line < 0 || line >= (int)vdp_vislines)
	{
		return;
	}

	draw_scanline((char *)displayBuffer, 512, line);
}

void paletteUpdate()
{
#define REDSHIFT	0
#define BLUESHIFT	10
#define GREENSHIFT	5

  unsigned int col;
  uint8 *p;

  for (col = 0; col < 64; col++) {      /* the CRAM has 64 colours */

	vdp_cramf[col] = 0;
    p = (uint8 *)vdp_cram + (2 * col);    /* point p to the two-byte CRAM entry */
    paletteBuffer[col] =          /* normal */
      (p[0] & 0xE) << (BLUESHIFT + 1) |
      (p[1] & 0xE) << (REDSHIFT + 1) |
      ((p[1] & 0xE0) >> 4) << (GREENSHIFT + 1);
    paletteBuffer[col + 64] =     /* hilight */
      (p[0] & 0xE) << BLUESHIFT |
      (p[1] & 0xE) << REDSHIFT |
      ((p[1] & 0xE0) >> 4) << GREENSHIFT |
      (16 << BLUESHIFT) | (16 << REDSHIFT) |
      (16 << GREENSHIFT);
    paletteBuffer[col + 128] =    /* shadow */
      (p[0] & 0xE) << BLUESHIFT |
      (p[1] & 0xE) << REDSHIFT |
      ((p[1] & 0xE0) >> 4) << GREENSHIFT;
  }

  paletteBuffer[0x00] = paletteBuffer[vdp_reg[7]&0x3f];
  paletteBuffer[0x10] = paletteBuffer[vdp_reg[7]&0x3f];
  paletteBuffer[0x20] = paletteBuffer[vdp_reg[7]&0x3f];
  paletteBuffer[0x30] = paletteBuffer[vdp_reg[7]&0x3f];
}

extern "C" void ui_endfield(void)
{
	if(!gfxRenderFrame) return;

	if(pgenRuntimeSetting.settings.currentRenderer)
	{
		vdp_renderframe(displayBuffer + (8 * 512) + 8, 512);
	}

	paletteUpdate();
	gfxUpdateIngameDisplay(1);
}

void gfxSync()
{
	if(gfxSkipNextFrame)
	{
		gfxSkipNextFrame = 0;
		gfxRenderFrame = 0;
	}
	else
		gfxRenderFrame = 1;
}

void gfxApplyFilterSetting()
{
	if(pgenRuntimeSetting.settings.renderFilter == 0)
		drawPipe->setFilterMethod(GS_FILTER_NEAREST);
	else
		drawPipe->setFilterMethod(GS_FILTER_LINEAR);
}


void gfxSetPal()
{
	dispDriver->setDisplayMode(320, 240, PAL, NONINTERLACE, 
		GS_PSMCT16, GS_ENABLE, GS_PSMZ16, 4);
	dispDriver->setDisplayPosition(pgenRuntimeSetting.settings.dispX,
		pgenRuntimeSetting.settings.dispY);
}

void gfxSetNtsc()
{
	dispDriver->setDisplayMode(320, 240, NTSC, NONINTERLACE, 
		GS_PSMCT16, GS_ENABLE, GS_PSMZ16, 4);
	dispDriver->setDisplayPosition(pgenRuntimeSetting.settings.dispX,
		pgenRuntimeSetting.settings.dispY);
}

void gfxUpdateGuiVideoMode()
{
	switch(pgenRuntimeSetting.guiVideoMode)
	{
		case PAL:

			gfxSetPal();
			break;

		case NTSC:
		case VGA640_60:

			gfxSetNtsc();
			break;
	}
}

void gfxUpdateIngameVideoMode()
{
	switch(pgenRuntimeSetting.gameVideoMode)
	{
		case PAL:

			pgenRuntimeSetting.maxFrameSec = 50;
			gfxSetPal();
			break;

		case NTSC:
		case VGA640_60:

			pgenRuntimeSetting.maxFrameSec = 60;
			gfxSetNtsc();
			break;
	}
}
