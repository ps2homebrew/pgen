#include "pgen.h"

#include "aio.h"
#include "gui.h"
#include "ingame.h"

guiIngameMenu::guiIngameMenu()
{
	updateIngameStrings();
	selection = 0;
}

void guiIngameMenu::draw()
{
	int yPos = 48;

	// Draw ingame display, then dim it
	gfxUpdateIngameDisplay(0);
	drawPipe->RectFlat(0, 0, 320, 240, Z_BG_MASK, GS_SET_RGBA(0, 0, 0, 0x40));

	switch(flag)
	{
		case GUI_FLAG_ANIM_OPEN:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(72, 30, 250, 214, GS_SET_RGBA(166, 170, 255, 115), GUI_FLAG_ANIM_OPEN);

			if(boxAnim->drawStep())
			{
				flag = 0;
				delete(boxAnim);
				boxAnim = NULL;
				updateIngameStrings();
				selection = 0;
			}

			break;

		case GUI_FLAG_ANIM_CLOSE:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(72, 30, 250, 214, GS_SET_RGBA(166, 170, 255, 115), GUI_FLAG_ANIM_CLOSE);

			if(boxAnim->drawStep())
			{
				if(mode)
					pgenRunning = 0;

				flag = 0;
				status = 0;
				delete(boxAnim);
				boxAnim = NULL;
				mode = 0;
			}

			break;

		case INGAME_FLAG_ERROR:

			drawPipe->RectFlat(53, 1, 270, 26, Z_BOX1, GS_SET_RGBA(166, 170, 255, 115));
			drawPipe->RectLine(52, 0, 270, 26, Z_BOX2, GS_SET_RGBA(111, 114, 171, 115));
			ocraFont.Print(63, 270, 5, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_CENTRE, currentError);

		default:

			drawPipe->RectFlat(72, 30, 250, 46, Z_BOX1, GS_SET_RGBA(111, 114, 171, 115));
			drawPipe->RectFlat(72, 46, 250, 214, Z_BOX1, GS_SET_RGBA(166, 170, 255, 115));

			zerohourFont.Print(0, 320, 30, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80),
				GSFONT_ALIGN_CENTRE, "In-game Menu");

			updateAlpha();

			for(int i = 0; i < 10; i++)
			{
				u32 color;

				if(i == selection)
					color = GS_SET_RGBA(0x80, 0x80, 0x80, alpha);
				else
					color = GS_SET_RGBA(0x80, 0x80, 0x80, 180);

				ocraFont.Print(20, 300, yPos, Z_LIST, color, 
						GSFONT_ALIGN_CENTRE, &ingameString[i][0]);

				yPos += 13;
				if((i == 0) || (i == 4) || (i == 7))
					yPos += 10;
				
			}
	}
}

void guiIngameMenu::update(u32 padRepeat, u32 padNoRepeat)
{
	int rv;

	switch(flag)
	{

		case INGAME_FLAG_ERROR:

			if(padNoRepeat & PAD_CROSS)
			{
				flag = GUI_FLAG_ANIM_CLOSE;
				mode = 0;
			}

			break;

		default:

			if(padNoRepeat & PAD_UP)
			{
				selection--;
				if(selection < 0)
					selection = 9;
			}
			else if(padNoRepeat & PAD_DOWN)
			{
				selection++;
				if(selection > 9)
					selection = 0;
			}
			else if(padNoRepeat & PAD_CROSS)
			{
				u32 num;

				flag = GUI_FLAG_ANIM_CLOSE;
				mode = 0;

				switch(selection)
				{
					case 0:	// return to game
						break;

					case 1: // quick save state
				
						currentRom->state->save();
						break;

					case 2: // quick load state

						currentRom->state->load();
						break;

					case 3: // mc/hdd save state

						rv = currentRom->state->saveState();
						if(rv < 0)
						{
							switch(rv)
							{
								case STATE_ERROR_CREATE_SAVE:
									currentError = "Failed to create saved state";
									break;

								case STATE_ERROR_SAVE_OPEN:
									currentError = "Failed to open saved state";
									break;

								case STATE_ERROR_SAVE_WRITE:
									currentError = "Failed to write saved state";
									break;
							}

							flag = INGAME_FLAG_ERROR;
						
							//char* buf;
							//sprintf(buf,"%d",rv);
							//currentError = buf;
							//flag = INGAME_FLAG_ERROR;
						}

						break;

					case 4:	// mc/hdd load state

						rv = currentRom->state->loadState();
						if(rv < 0)
						{
							switch(rv)
							{
								case STATE_ERROR_LOAD_STATE:
									currentError = "Failed to load saved state";
									break;

								case STATE_ERROR_STATE_NOT_FOUND:
									currentError = "Saved state not found";
									break;

								case STATE_ERROR_VERSION:
									currentError = "Incompatible saved state";
									break;
							}

							flag = INGAME_FLAG_ERROR;
						}

						break;

					case 5:	// reposition display

						guiDoDisplayCalibrate();
						flag = 0;
						pgenOptionsChanged = 1;

						break;

					case 6:	// renderer

						num =	(pgenRuntimeSetting.settings.currentRenderer |
									(pgenRuntimeSetting.settings.renderFilter << 1)) & 0x03;

						num++;

						pgenRuntimeSetting.settings.currentRenderer = num & 0x01;
						pgenRuntimeSetting.settings.renderFilter = (num >> 1) & 0x01;

						// We dont want to close the menu box yet
						flag = 0;

						pgenOptionsChanged = 1;

						break;

					case 7:	// region

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
				
						pgenSetRegion();

						flag = 0;

						pgenOptionsChanged = 1;

						break;

					case 8:	// soft reset

						gen_reset();
						break;

					case 9:	// return to main menu

						mode = 1;
						break;
				}

				updateIngameStrings();
			}
	}
}


void guiIngameMenu::updateIngameStrings()
{
	char device[16];

	strcpy(&ingameString[0][0], "Return to Game");

	strcpy(&ingameString[1][0], "Quick Save State");
	strcpy(&ingameString[2][0], "Quick Load State");

	if(pgenRuntimeSetting.settings.saveDevice == 0)
		strcpy(device, "MC");
	else
		strcpy(device, "HDD");

	sprintf(&ingameString[3][0], "%s Save State", device);
	sprintf(&ingameString[4][0], "%s Load State", device);

	strcpy(&ingameString[5][0], "Reposition Screen");

	if(pgenRuntimeSetting.settings.currentRenderer == 0)
		strcpy(&ingameString[6][0], "Renderer: Line");
	else
		strcpy(&ingameString[6][0], "Renderer: Cell");

	if(pgenRuntimeSetting.settings.renderFilter)
		strcat(&ingameString[6][0], ", Filter");

	strcpy(&ingameString[7][0], "Region: ");
	if(pgenRuntimeSetting.settings.regionAutoDetect)
		strcat(&ingameString[7][0], "Auto");
	else
	{
		switch(pgenRuntimeSetting.settings.currentRegion)
		{
			case 0:
				strcat(&ingameString[7][0], "JAP");
				break;
			case 2:
				strcat(&ingameString[7][0], "USA");
				break;
			case 3:
				strcat(&ingameString[7][0], "Europe");
				break;
		}
	}

	strcpy(&ingameString[8][0], "Soft Reset");
	strcpy(&ingameString[9][0], "Return to Main Menu");
}
