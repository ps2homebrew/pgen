#include "pgen.h"
#include "options.h"


guiOptionsMenu::guiOptionsMenu()
{
	selection = 0;
	updateOptionString();
}

void guiOptionsMenu::updateOptionString()
{
	strcpy(&optionString[0][0], "Region: ");
	if(pgenRuntimeSetting.settings.regionAutoDetect)
		strcat(&optionString[0][0], "Auto");
	else
	{
		switch(pgenRuntimeSetting.settings.currentRegion)
		{
			case 0:
				strcat(&optionString[0][0], "JAP");
				break;
			case 2:
				strcat(&optionString[0][0], "USA");
				break;
			case 3:
				strcat(&optionString[0][0], "Europe");
				break;
		}
	}

	strcpy(&optionString[1][0], "Default Region: ");
	switch(pgenRuntimeSetting.settings.defaultRegion)
	{
			case 0:
				strcat(&optionString[1][0], "JAP");
				break;
			case 2:
				strcat(&optionString[1][0], "USA");
				break;
			case 3:
				strcat(&optionString[1][0], "Europe");
				break;
	}

	strcpy(&optionString[2][0], "Renderer: ");
	if(pgenRuntimeSetting.settings.currentRenderer == 0)
		strcat(&optionString[2][0], "Line");
	else
		strcat(&optionString[2][0], "Cell");

	strcpy(&optionString[3][0], "Video Filter: ");
	if(pgenRuntimeSetting.settings.renderFilter == 0)
		strcat(&optionString[3][0], "Off");
	else
		strcat(&optionString[3][0], "On");

	strcpy(&optionString[4][0], "Sound: ");
	if(pgenRuntimeSetting.settings.soundOn == 0)
		strcat(&optionString[4][0], "Off");
	else
		strcat(&optionString[4][0], "On");

	strcpy(&optionString[5][0], "MultiMode TV: ");
	if(pgenRuntimeSetting.settings.mutliModeTv == 0)
		strcat(&optionString[5][0], "No");
	else
		strcat(&optionString[5][0], "Yes");
		
	strcpy(&optionString[6][0], "Reposition Screen");

	strcpy(&optionString[7][0], "Frame Counter: ");
	if(pgenRuntimeSetting.settings.displayFps == 0)
		strcat(&optionString[7][0], "Off");
	else
		strcat(&optionString[7][0], "On");

	strcpy(&optionString[8][0], "Video Mode: ");
	switch(pgenRuntimeSetting.guiVideoMode)
	{
			case NTSC:
				strcat(&optionString[8][0], "NTSC");
				break;
			case PAL:
				strcat(&optionString[8][0], "PAL");
				break;
	}

	strcpy(&optionString[9][0], "Button remapping: ");
	switch(pgenRuntimeSetting.input.pgenRuntimeSetting.xpressed)
	{
			case 0:
				strcat(&optionString[9][0], "a");
				break;
			case 1:
				strcat(&optionString[9][0], "b");
				break;			
			case 2:
				strcat(&optionString[9][0], "c");
				break;			
			case 3:
				strcat(&optionString[9][0], "x");
				break;			
			case 4:
				strcat(&optionString[9][0], "y");
				break;			
			case 5:
				strcat(&optionString[9][0], "z");
				break;			
			case 6:
				strcat(&optionString[9][0], "start");
				break;			
			case 7:
				strcat(&optionString[9][0], "mode");
				break;			
			case 8:
				strcat(&optionString[9][0], "pause");
				break;	
	}

	strcpy(&optionString[10][0], "Save device: ");
	if(pgenRuntimeSetting.settings.saveDevice == 0)
		strcat(&optionString[10][0], "Memory Card");
	else
		strcat(&optionString[10][0], "HDD");
}

void guiOptionsMenu::draw()
{
	int yPos = 72;

	switch(flag)
	{
		case GUI_FLAG_ANIM_OPEN:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(70, yPos-18, 250, 218, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_OPEN);

			if(boxAnim->drawStep())
			{
				flag = 0;
				delete(boxAnim);
				boxAnim = NULL;
				updateOptionString();
			}

			break;

		case GUI_FLAG_ANIM_CLOSE:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(70, yPos-18, 250, 218, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_CLOSE);

			if(boxAnim->drawStep())
			{
				flag = 0;
				status = 0;
				delete(boxAnim);
				boxAnim = NULL;
			}

			break;

		default:

			drawPipe->RectFlat(70, yPos-18, 250, yPos-2, Z_BOX1, GS_SET_RGBA(111, 114, 171, 86));
			drawPipe->RectFlat(70, yPos-2, 250, 218, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));

			zerohourFont.Print(20, 300, yPos-18, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80), 
				GSFONT_ALIGN_CENTRE, "Options");

			updateAlpha();

			for(int i = 0; i < 10; i++)
			{
				u32 color;

				if((i == 9) && (!pgenRuntimeSetting.HDDFormatted))
					color = GS_SET_RGBA(0x55, 0x55, 0x55, 180);
				else if(i == selection)
					color = GS_SET_RGBA(0x80, 0x80, 0x80, alpha);
				else
					color = GS_SET_RGBA(0x80, 0x80, 0x80, 180);

				ocraFont.Print(70, 250, yPos, Z_LIST, color, 
						GSFONT_ALIGN_CENTRE, &optionString[i][0]);

				yPos += 13;
			}

	}
}

void guiOptionsMenu::update(u32 padRepeat, u32 padNoRepeat)
{
	int pgenRuntimeSetting.xpressed=0;
	if((padNoRepeat & PAD_UP) && (selection > 0))
	{
		selection--;
	}
	else if(padNoRepeat & PAD_DOWN)
	{
		if(	((pgenRuntimeSetting.HDDFormatted) && (selection < 10)) ||
			((!pgenRuntimeSetting.HDDFormatted) && (selection < 9)) )
				selection++;
	}
	else if(padRepeat & PAD_TRIANGLE)
	{
		pgenOptionsSave();
		flag = GUI_FLAG_ANIM_CLOSE;
	}
	else if(padNoRepeat & PAD_CROSS)
	{
		switch(selection)
		{
			case 0:	// Region

				if(pgenRuntimeSetting.settings.regionAutoDetect) {
					pgenRuntimeSetting.settings.regionAutoDetect = 0;
					pgenRuntimeSetting.settings.currentRegion = 0;
				}
				else if(pgenRuntimeSetting.settings.currentRegion < 3)
				{
					pgenRuntimeSetting.settings.currentRegion++;

					if(pgenRuntimeSetting.settings.currentRegion == 1) 
						pgenRuntimeSetting.settings.currentRegion++; // PAL/JAP is invalid
				} else
					pgenRuntimeSetting.settings.regionAutoDetect = 1;

				break;
				
			case 1:	// Default region

				pgenRuntimeSetting.settings.defaultRegion++;
				if(pgenRuntimeSetting.settings.defaultRegion == 1) 
					pgenRuntimeSetting.settings.defaultRegion++; // PAL/JAP is invalid
				if(pgenRuntimeSetting.settings.defaultRegion > 3) 
					pgenRuntimeSetting.settings.defaultRegion = 0;
				
				break;

			case 2:	// Renderer
				
				pgenRuntimeSetting.settings.currentRenderer ^= 1;
				break;

			case 3: // Filter

				pgenRuntimeSetting.settings.renderFilter ^= 1;
				break;

			case 4:	// Sound

				pgenRuntimeSetting.settings.soundOn ^= 1;
				break;

			case 5:	// MultiMode TV

				pgenRuntimeSetting.settings.mutliModeTv ^= 1;
				break;

			case 6:

				guiDoDisplayCalibrate();

				break;

			case 7:	// Frame counter

				pgenRuntimeSetting.settings.displayFps ^= 1;
				break;
			
			case 8:	// Video system

				pgenRuntimeSetting.guiVideoMode++;
				if(pgenRuntimeSetting.guiVideoMode > 3) 
					pgenRuntimeSetting.guiVideoMode = 2;
				gfxChangeDefaultVideoMode(pgenRuntimeSetting.guiVideoMode); 

				break;

			case 9:	// Save device

				pgenRuntimeSetting.xpressed++;
				if(pgenRuntimeSetting.xpressed > 8)
					pgenRuntimeSetting.xpressed=0;
				//TODO: updateInput();
				break;

			case 10:	// Save device

				pgenRuntimeSetting.settings.saveDevice ^= 1;

				if(pgenRuntimeSetting.settings.saveDevice == 0)
					currentSaver = saverMC;
				else
					currentSaver = saverHDD;

				break;
		}

		updateOptionString();
	}
}
