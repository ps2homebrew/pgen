#include "pgen.h"

#include "aio.h"
#include "gui.h"
#include "credits.h"
#include "options.h"
#include "scrollList.h"
#include "romlist.h"
#include "saveman.h"
#include "ingame.h"

guiSaveManager *saveMan;
guiOptionsMenu *optionsMenu;
guiRomlist *romlist;
guiMainMenu *mainMenu;
guiCredits *credits;
guiIngameMenu *ingameMenu;

guiComponentIf *currentGui;

void guiInit()
{
	saveMan = new guiSaveManager();
	optionsMenu = new guiOptionsMenu();
	romlist = new guiRomlist(16, 80, 280, 136);
	mainMenu = new guiMainMenu();
	credits = new guiCredits();
	ingameMenu = new guiIngameMenu();

	currentGui = mainMenu;
}

pgenRom *pgenMenu(int fadeIn)
{
	u32 status = GUI_STAT_RUNNING;
	u32 padRepeat = 0, padNoRepeat = 0;

	gfxUploadTextures();

	if(fadeIn)
		guiFadeInGui();

	// Loop until we are to load a rom
	while(!(status & GUI_STAT_LOAD_ROM))
	{
		guiDrawBackground();
		guiDrawScroll();

		currentGui->draw();

		guiUpdateInput(&padRepeat, &padNoRepeat);
		currentGui->update(padRepeat, padNoRepeat);
		if((padRepeat &	(PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2)) ==
						(PAD_L1 | PAD_L2 | PAD_R1 | PAD_R2))
		{
			if(padRepeat & PAD_SELECT)
				gfxChangeDefaultVideoMode(GS_TV_PAL);
			else if(padRepeat & PAD_START)
				gfxChangeDefaultVideoMode(GS_TV_NTSC);
		}

		status = currentGui->getStatus();

		guiUpdateScreen();

		if(status != GUI_STAT_RUNNING)
		{
			currentGui->reset();

			if(status & GUI_STAT_LOAD_ROM)
				break;

			switch(status)
			{
				case GUI_STAT_EXEC_ROMLIST:
					currentGui = romlist;
					break;

				case GUI_STAT_EXEC_OPTIONS:
					currentGui = optionsMenu;
					break;

				case GUI_STAT_EXEC_SAVEMAN:
					currentGui = saveMan;
					break;

				case GUI_STAT_EXEC_CREDITS:
					currentGui = credits;
					break;

				default:
					currentGui = mainMenu;
			}
		}
	}

	guiDisplayLoading();
	
	// Load rom
	return new pgenRom(romlist->getRomAIO(), romlist->getRomFilename());
}

void guiDrawBackground()
{
	drawPipe->setFilterMethod(GS_FILTER_LINEAR);
	drawPipe->TextureSet(pgenTexEnv.pgenBG.texBufPtr, pgenTexEnv.pgenBG.texBufWidth, 
		dispDriver->getTexSizeFromInt(pgenTexEnv.pgenBG.width), 
		dispDriver->getTexSizeFromInt(pgenTexEnv.pgenBG.height), 
		pgenTexEnv.pgenBG.psm, 0, 0, 0, 0);

	drawPipe->RectTexture(0, 0, 0, 0, 320, 240, 320, 240, Z_BG, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
	drawPipe->setFilterMethod(GS_FILTER_NEAREST);
}

void guiDrawScroll()
{
	// Only display scroller for PAL, since it doesnt fully fit on a NTSC screen
	// (sorry, when I first designed the PGEN gui I thought NTSC could display 240 vertical
	// lines - I later found out it was only 224. Dont wanna re-design the whole thing just
	// because of this small issue ;)
	int dif=0;
	if(pgenRuntimeSetting.guiVideoMode != GS_TV_PAL)
		dif=0;
//		return;

	// HACK: scrollX = 310 instead of 320, as it seems to fix weird bug
	// (during first ~1 second of scroller, text would fux0r)
	static int scrollX = 310, scrollDelay = 1, inited = 0, scrollLength = 0;
	static char scrollerString[] = "    PGEN 1.5.1s by Sjeep and others (see credits)"
	"readapted by AKuHAK";

	if(!inited)
	{
		for(unsigned int i = 0; i < strlen(scrollerString); i++)
			scrollLength += zerohourFnt.CharWidth[(int)scrollerString[i]];

		inited = 1;
	}

	zerohourFont.Print(scrollX, 336, 224-dif, Z_SCROLL, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80), 
		GSFONT_ALIGN_LEFT, scrollerString);

	if(--scrollDelay < 0)
	{
		scrollX--;
		
		if(scrollX < -scrollLength)
			scrollX = 310;

		scrollDelay = 1;
	}

	drawPipe->TextureSet(pgenTexEnv.pgenBG.texBufPtr, pgenTexEnv.pgenBG.texBufWidth, 
		dispDriver->getTexSizeFromInt(pgenTexEnv.pgenBG.width), 
		dispDriver->getTexSizeFromInt(pgenTexEnv.pgenBG.height), 
		pgenTexEnv.pgenBG.psm, 0, 0, 0, 0);

	drawPipe->RectTexture(0, 220-dif, 0, 220-dif, 10, 240-dif, 10, 240-dif, Z_SCROLL_M, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
	drawPipe->TriStripGouraudTexture(	10, 220-dif, Z_SCROLL_M, 10, 220-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80),
										40, 220-dif, Z_SCROLL_M, 40, 220-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x00),
										10, 240-dif, Z_SCROLL_M, 10, 240-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80),
										40, 240-dif, Z_SCROLL_M, 40, 240-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x00));

	drawPipe->RectTexture(310, 220-dif, 310, 220-dif, 320, 240-dif, 320, 240-dif, Z_SCROLL_M, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
	drawPipe->TriStripGouraudTexture(	280, 220-dif, Z_SCROLL_M, 280, 220-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x00),
										310, 220-dif, Z_SCROLL_M, 310, 220-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80),
										280, 240-dif, Z_SCROLL_M, 280, 240-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x00),
										310, 240-dif, Z_SCROLL_M, 310, 240-dif, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));
}

void guiUpdateScreen()
{
	drawPipe->Flush();
	dispDriver->DrawBufferComplete();

	dispDriver->WaitForVSync();

	dispDriver->setNextDrawBuffer();
	dispDriver->DisplayNextFrame();

	// Clear the screen (with ZBuffer Disabled)
	drawPipe->setZTestEnable(GS_DISABLE);
	drawPipe->RectFlat(0,0,320,240,0,GS_SET_RGBA(0,0,0,0x80));
	drawPipe->setZTestEnable(GS_ENABLE);
}

void guiUpdateInput(u32 *padRepeat, u32 *padNoRepeat)
{
	static int oldPad = 0;
	int padData = 0;

	padData = guiPad->updateInput();

	if(padRepeat)
		*padRepeat = padData;
	if(padNoRepeat)
		*padNoRepeat = padData & ~oldPad;
	oldPad = padData;
}


void guiDisplayLoading()
{
	guiAnimateBox *boxAnim = new guiAnimateBox(40, 130, 280, 166, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_OPEN);
	u32 stat = 0;


	do {

		guiDrawBackground();
		guiDrawScroll();
		stat = boxAnim->drawStep();
		guiUpdateScreen();


	} while(!stat);
	free(boxAnim);

	for(int i = 0; i < 2; i++)
	{
		guiDrawBackground();
		guiDrawScroll();

		drawPipe->RectFlat(40, 130, 280, 166, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));
		zerohourFont.Print(20, 300, 138, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80), 
			GSFONT_ALIGN_CENTRE, "Loading...");

		guiUpdateScreen();
	}
}

void guiDisplayLoadingEnd()
{
	guiAnimateBox *boxAnim = new guiAnimateBox(40, 130, 279, 166, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_CLOSE);
	u32 stat;

	do {

		guiDrawBackground();
		guiDrawScroll();
		stat = boxAnim->drawStep();
		guiUpdateScreen();


	} while(!stat);
	free(boxAnim);
}

void guiDoIngameMenu()
{
	u32 status = GUI_STAT_RUNNING;
	u32 padRepeat = 0, padNoRepeat = 0;

	pgenState = PGEN_STATE_INGAME_MENU;
	if(pgenRuntimeSetting.settings.soundOn) SjPCM_Pause();
	gfxUploadTextures();

	while(1)
	{

		ingameMenu->draw();

		guiUpdateInput(&padRepeat, &padNoRepeat);
		ingameMenu->update(padRepeat, padNoRepeat);

		status = ingameMenu->getStatus();

		guiUpdateScreen();

		if(status != GUI_STAT_RUNNING)
		{
			ingameMenu->reset();
			break;
		}
	}

	if(pgenRuntimeSetting.settings.soundOn) SjPCM_Play();
	pgenState = PGEN_STATE_EMULATION;

	// XXX: jur, missing function dispDriver->ResetMBA();
}





guiMainMenu::guiMainMenu()
{
	selection = 0;
}

void guiMainMenu::draw()
{													  
	static char* strings[] = { "Rom List", "Options", "Save Manager", "Credits", "Exit" };
	int yPos = 128;
	int i;

	switch(flag)
	{
		case GUI_FLAG_ANIM_OPEN:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(86, 110, 234, 201, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_OPEN);

			if(boxAnim->drawStep())
			{
				flag = 0;
				delete(boxAnim);
				boxAnim = NULL;
			}

			break;

		case GUI_FLAG_ANIM_CLOSE:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(86, 110, 234, 201, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_CLOSE);

			if(boxAnim->drawStep())
			{
				flag = 0;
				status = 0;
				delete(boxAnim);
				boxAnim = NULL;


				switch(selection)
				{
					case 0:
						status = GUI_STAT_EXEC_ROMLIST;
						break;
					case 1:
						status = GUI_STAT_EXEC_OPTIONS;
						break;
					case 2:
						status = GUI_STAT_EXEC_SAVEMAN;
						break;
					case 3:
						status = GUI_STAT_EXEC_CREDITS;
						break;
					case 4:
						poweroffShutdown();
						break;
				}
				// The above will in effect disable the GUI_STAT_RUNNING bit.
			}

			break;

		default:

			drawPipe->RectFlat(86, 110, 234, 126, Z_BOX1, GS_SET_RGBA(111, 114, 171, 86));
			drawPipe->RectFlat(86, 126, 234, 201, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));

			updateAlpha();

			zerohourFont.Print(20, 300, 110, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80), 
				GSFONT_ALIGN_CENTRE, "Main Menu");

			for(i = 0; i < 5; i++) 
			{
				if(i == selection) 
					ocraFont.Print(20, 300, yPos, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, alpha), 
						GSFONT_ALIGN_CENTRE, strings[i]);
				else 
					ocraFont.Print(20, 300, yPos, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
						GSFONT_ALIGN_CENTRE, strings[i]);

				yPos += 13;
			}
	}
}

void guiMainMenu::update(u32 padRepeat, u32 padNoRepeat)
{
	if(padNoRepeat & PAD_UP)
	{
		selection--;
		if(selection < 0)
			selection = 4;
	}
	else if(padNoRepeat & PAD_DOWN)
	{
		selection++;
		if(selection > 4)
			selection = 0;
	}
	else if(padNoRepeat & PAD_CROSS)
		flag = GUI_FLAG_ANIM_CLOSE;
}






guiAnimateBox::guiAnimateBox(int x1, int y1, int x2, int y2, u32 color, int direction)
{
	this->x1 = x1;
	this->y1 = y1;
	this->x2 = x2;
	this->y2 = y2;
	this->color = color;
	this->direction = direction;
	finished = 0;

	switch(direction)
	{
		case GUI_FLAG_ANIM_OPEN:

			xDest = yDest = 0;
			xOff = ((x2 - x1) / 2) - 1;
			yOff = ((y2 - y1) / 2) - 1;
			break;

		case GUI_FLAG_ANIM_CLOSE:

			yOff = xOff = 0;
			xDest = ((x2 - x1) / 2) - 1;
			yDest = ((y2 - y1) / 2) - 1;
			break;
	}
}

int guiAnimateBox::drawStep()
{
	switch(direction)
	{
		case GUI_FLAG_ANIM_OPEN:

			if(xOff)
			{
				xOff -= 7;
				if(xOff < 0)
					xOff = 0;
			}
			else if(yOff)
			{
				yOff -= 5;
				if(yOff < 0)
				{
					yOff = 0;
					finished = 1;
				}
			}

			break;

		case GUI_FLAG_ANIM_CLOSE:

			if(yOff < yDest)
			{
				yOff += 5;
				if(yOff > yDest)
					yOff = yDest;
			}
			else if(xOff < xDest)
			{
				xOff += 7;
				if(xOff > xDest)
				{
					xOff = xDest;
					finished = 1;
				}
			}

			break;

	}

	drawPipe->RectFlat(x1 + xOff, y1 + yOff, x2 - xOff, y2 - yOff, Z_BOX1, color);

	return finished;
}

void guiFadeOutIngame()
{
	int alpha = 0;

	while(alpha < 0x80)
	{
		gfxUpdateIngameDisplay(0);
		drawPipe->RectFlat(0, 0, 320, 240, Z_FADE, GS_SET_RGBA(255, 255, 255, alpha));
		guiUpdateScreen();

		alpha +=2;
	}

}

void guiFadeInGui()
{
	int alpha = 0x80;

	while(alpha > 0)
	{
		guiDrawBackground();
		guiDrawScroll();

		drawPipe->RectFlat(0, 0, 320, 240, Z_FADE, GS_SET_RGBA(255, 255, 255, alpha));
		guiUpdateScreen();

		alpha -=2;	
	}
}


#define DIALOG_BOX_TITLE_HEIGHT		16

guiDialogBox::guiDialogBox(int x, int y, int w, int h, const char *title, const char *body)
{
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->title = title;
	this->body = body;

	int numLines = 0;
	for(int i = 0; i < (int)strlen(body) + 1; i++)
		if((body[i] == '\n') || (body[i] == '\0'))
			numLines++;

	int bodyHeight = ocraFnt.CharGridHeight * numLines;
	bodyY = y + DIALOG_BOX_TITLE_HEIGHT + ( ((h - DIALOG_BOX_TITLE_HEIGHT) / 2) - (bodyHeight / 2)
			/* - (ocraFnt.CharGridHeight / 2)*/);
}

void guiDialogBox::draw()
{
	drawPipe->RectFlat(x, y, x + w, y + DIALOG_BOX_TITLE_HEIGHT, Z_BOX1, GS_SET_RGBA(111, 114, 171, 86));
	drawPipe->RectFlat(x, y + DIALOG_BOX_TITLE_HEIGHT, x + w, y + h, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));

	zerohourFont.Print(x, x + w, y, Z_BOX2, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80),
		GSFONT_ALIGN_CENTRE, title);
	ocraFont.Print(x, x + w, bodyY, Z_BOX2, GS_SET_RGBA(0x80, 0x80, 0x80, 180),
		GSFONT_ALIGN_CENTRE, body);
}




void guiFatalError(const char *msg) 
{ 
	guiDialogBox dialog(20, 100, 280, 76, "Fatal Error", msg);

	while(1)
	{
		guiDrawBackground();
		guiDrawScroll();
		
		dialog.draw();

		guiUpdateScreen();
	}
}

void guiError(const char *msg) 
{
	u32 padRepeat = 0, padNoRepeat = 0;
	char thisError[1024];
	snprintf(thisError, 1024, "%s\n\nPress START to continue", msg);
	guiDialogBox dialog(20, 100, 280, 76, "Error", thisError);

	while(!(padNoRepeat & PAD_START))
	{
		guiUpdateInput(&padRepeat, &padNoRepeat);

		guiDrawBackground();
		guiDrawScroll();
		
		dialog.draw();

		guiUpdateScreen();
	}
}


void guiUpdateDisplayPosition()
{
	int currentVideoMode;
	if(pgenState == PGEN_STATE_GUI)
		currentVideoMode = pgenRuntimeSetting.guiVideoMode;
	else
		currentVideoMode = pgenRuntimeSetting.gameVideoMode;

	switch(currentVideoMode)
	{
		case GS_TV_PAL:
				
			dispDriver->setDisplayPosition(pgenRuntimeSetting.settings.dispXPAL,
				pgenRuntimeSetting.settings.dispYPAL);

			break;

		case GS_TV_NTSC:

			dispDriver->setDisplayPosition(pgenRuntimeSetting.settings.dispXNTSC,
				pgenRuntimeSetting.settings.dispYNTSC);

			break;
	}
}

void guiDoDisplayCalibrate()
{
	int currentVideoMode;
	u32 padRepeat = 0, padNoRepeat = 0;

	if(pgenState == PGEN_STATE_GUI)
		currentVideoMode = pgenRuntimeSetting.guiVideoMode;
	else
		currentVideoMode = pgenRuntimeSetting.gameVideoMode;

	while(1)
	{
		drawPipe->Line(0, 0, 40, 0, 10, GS_SET_RGBA(255, 255, 255, 0x80)); 
		drawPipe->Line(0, 0, 0, 80, 10, GS_SET_RGBA(255, 255, 255, 0x80));
		drawPipe->Line(0, 40, 40, 40, 10, GS_SET_RGBA(255, 255, 255, 0x80));

		drawPipe->Line(280, 0, 320, 0, 10, GS_SET_RGBA(255, 255, 255, 0x80));
		drawPipe->Line(319, 0, 319, 80, 10, GS_SET_RGBA(255, 255, 255, 0x80));
		drawPipe->Line(280, 40, 320, 40, 10, GS_SET_RGBA(255, 255, 255, 0x80));

		drawPipe->Line(0, 144, 0, 224, 10, GS_SET_RGBA(255, 255, 255, 0x80));
		drawPipe->Line(0, 184, 40, 184, 10, GS_SET_RGBA(255, 255, 255, 0x80));
		drawPipe->Line(0, 224, 40, 224, 10, GS_SET_RGBA(255, 255, 255, 0x80));

		drawPipe->Line(280, 224, 319, 224, 10, GS_SET_RGBA(255, 255, 255, 0x80));
		drawPipe->Line(319, 144, 319, 224, 10, GS_SET_RGBA(255, 255, 255, 0x80));
		drawPipe->Line(280, 184, 319, 184, 10, GS_SET_RGBA(255, 255, 255, 0x80));

		ocraFont.Print(0, 320, 96, 15, GS_SET_RGBA(0x80, 0x80, 0x80, 180),
		GSFONT_ALIGN_CENTRE, "Use the arrow buttons to re-position\n"
							 "the screen. Press X when done.");

		guiUpdateInput(&padRepeat, &padNoRepeat);

		int diffx = 0, diffy = 0;
		if(padRepeat & PAD_LEFT)
			diffx = -1;
		if(padRepeat & PAD_RIGHT)
			diffx = 1;
		if(padRepeat & PAD_UP)
			diffy = -1;
		if(padRepeat & PAD_DOWN)
			diffy = 1;
		
		if(padNoRepeat & PAD_CROSS)
			break;

		if(diffx || diffy)
		{
			switch(currentVideoMode)
			{
				case GS_TV_PAL:
				
					pgenRuntimeSetting.settings.dispXPAL += diffx;
					pgenRuntimeSetting.settings.dispYPAL += diffy;

					if(pgenRuntimeSetting.settings.dispXPAL < 0)
						pgenRuntimeSetting.settings.dispXPAL = 0;
					if(pgenRuntimeSetting.settings.dispYPAL < 0)
						pgenRuntimeSetting.settings.dispYPAL = 0;

					dispDriver->setDisplayPosition(pgenRuntimeSetting.settings.dispXPAL,
						pgenRuntimeSetting.settings.dispYPAL);

					break;

				case GS_TV_NTSC:

					pgenRuntimeSetting.settings.dispXNTSC += diffx;
					pgenRuntimeSetting.settings.dispYNTSC += diffy;

					if(pgenRuntimeSetting.settings.dispXNTSC < 0)
						pgenRuntimeSetting.settings.dispXNTSC = 0;
					if(pgenRuntimeSetting.settings.dispYNTSC < 0)
						pgenRuntimeSetting.settings.dispYNTSC = 0;

					dispDriver->setDisplayPosition(pgenRuntimeSetting.settings.dispXNTSC,
						pgenRuntimeSetting.settings.dispYNTSC);

					break;
			}
		}

		guiUpdateScreen();
	}
}
