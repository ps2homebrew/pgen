#include "pgen.h"
#include "scrollList.h"
#include "saveman.h"

guiSaveManager::guiSaveManager() : scrollList(18, 95, 240, 108, 8, 13)
{
	freeSpace = 0;
	pgenUsed = 0;
	numEntries = 0;

	saveEntries = (saveManEntry *)malloc(2048 * sizeof(saveManEntry));
	if(!saveEntries)
		guiFatalError("Failed to allocate memory!");

	
}
void guiSaveManager::drawBody()
{
	drawPipe->RectFlat(16, 80, 304, 94, Z_BOX1, GS_SET_RGBA(111, 114, 171, 86));

	drawPipe->RectFlat(16, 94, 258, 202, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));
	drawPipe->RectFlat(258, 94, 260, 202, Z_BOX1, GS_SET_RGBA(111, 114, 171, 86));
	drawPipe->RectFlat(260, 94, 296, 202, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));

	drawPipe->RectFlat(16, 202, 304, 216, Z_BOX1, GS_SET_RGBA(111, 114, 171, 86));
	
	ocraFont.Print(16, 304, 80, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
		GSFONT_ALIGN_LEFT, "Game Name");
	ocraFont.Print(16, 304, 80, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
		GSFONT_ALIGN_RIGHT, "Size (kb)");
}

void guiSaveManager::draw()
{
	switch(flag)
	{
		case GUI_FLAG_ANIM_OPEN:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(16, 80, 304, 216, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_OPEN);

			if(boxAnim->drawStep())
			{
				flag = SAVEMAN_FLAG_POPULATING;
				delete(boxAnim);
				boxAnim = NULL;
			}

			break;

		case GUI_FLAG_ANIM_CLOSE:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(16, 80, 304, 216, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_CLOSE);

			if(boxAnim->drawStep())
			{
				flag = 0;
				status = 0;
				delete(boxAnim);
				boxAnim = NULL;
			}

			break;

		case SAVEMAN_FLAG_POPULATING:

			if(counter++ == 2)
			{
				counter = 0;

				if(updateAndFill() < 0)
				{
					flag = SAVEMAN_FLAG_ERROR;
					errorFatal = 1;
					if(pgenRuntimeSetting.settings.saveDevice == 0)
						currentError = "Failed to read from memory card!";
					else
						currentError = "Failed to read from HDD!";
				}
				else
					flag = 0;
			}

			drawBody();
			drawScrollBar(1.0f, 0.0f);

			ocraFont.Print(16, 258, 142, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_CENTRE, "Populating list...");

			break;

		case SAVEMAN_FLAG_CONFIRM_DELETE:

			drawBody();
			drawScrollBar(1.0f, 0.0f);

			ocraFont.Print(16, 258, 116, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_CENTRE, "Really delete saved state?");
			ocraFont.Print(16, 258, 142, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_CENTRE, deleteEntry->name);

			updateAlpha();

			if(deleteSelection == 0) // "Yes" selected
			{
				ocraFont.Print(16, 121, 168, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, alpha), 
					GSFONT_ALIGN_RIGHT, "Yes");
				ocraFont.Print(153, 258, 168, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
					GSFONT_ALIGN_LEFT, "No");
			}
			else // "No" selected
			{
				ocraFont.Print(16, 121, 168, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
					GSFONT_ALIGN_RIGHT, "Yes");
				ocraFont.Print(153, 258, 168, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, alpha), 
					GSFONT_ALIGN_LEFT, "No");
			}

			break;

		case SAVEMAN_FLAG_DELETE:

			if(counter++ == 2)
			{
				counter = 0;

				int rv = deleteSave();
				if(rv < 0)
				{
					currentError = "Error deleting save!";
					flag = SAVEMAN_FLAG_ERROR;
					errorFatal = 0;
				}
				else
					flag = SAVEMAN_FLAG_POPULATING;
			}

			drawBody();
			drawScrollBar(1.0f, 0.0f);

			ocraFont.Print(16, 258, 142, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_CENTRE, "Deleting...");					

			break;

		case SAVEMAN_FLAG_ERROR:

			drawBody();
			drawScrollBar(1.0f, 0.0f);

			ocraFont.Print(16, 258, 128, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_CENTRE, currentError);
			ocraFont.Print(16, 258, 154, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_CENTRE, "Press X to continue");				

			break;

		default:

			drawBody();

			char string[64];

			if(pgenUsed > 1024)
				sprintf(string, "PGEN Usage: %d mb", pgenUsed / 1024);
			else
				sprintf(string, "PGEN Usage: %d kb", pgenUsed);
				
			ocraFont.Print(20, 300, 202, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_LEFT, string);

			if(freeSpace > 1024)
				sprintf(string, "Total Free: %d mb", freeSpace / 1024);
			else
				sprintf(string, "Total Free: %d kb", freeSpace);

			ocraFont.Print(20, 300, 202, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180), 
				GSFONT_ALIGN_RIGHT, string);

			updateAlpha();
			drawScrollList();
	}
}

void guiSaveManager::update(u32 padRepeat, u32 padNoRepeat)
{
	switch(flag)
	{
		case SAVEMAN_FLAG_CONFIRM_DELETE:

			if(padNoRepeat & PAD_LEFT)
				deleteSelection = 0;
			else if(padNoRepeat & PAD_RIGHT)
				deleteSelection = 1;

			if(padNoRepeat & PAD_CROSS)
			{
				if(deleteSelection == 0)
				{
					counter = 0;
					flag = SAVEMAN_FLAG_DELETE;
				}
				else
					flag = 0;
			}

			break;

		case SAVEMAN_FLAG_ERROR:
	
			if(padNoRepeat & PAD_CROSS)
			{
				if(errorFatal)
					flag = GUI_FLAG_ANIM_CLOSE;
				else
					flag = 0;
			}

			break;

		default:

			updateScrollList(padRepeat, padNoRepeat);

			if(padRepeat & PAD_TRIANGLE)
				flag = GUI_FLAG_ANIM_CLOSE;
			else if((padNoRepeat & PAD_CROSS) && (numEntries > 0))
			{
				flag = SAVEMAN_FLAG_CONFIRM_DELETE;
				deleteEntry = &saveEntries[getSelection()];
				deleteSelection = 1;
			}
	}
}

void guiSaveManager::drawItem(int item, int selected, int x, int w, int y)
{
	int thisAlpha;

	if(selected) thisAlpha = alpha;
	else thisAlpha = 180;

	// TODO: remove need for hack below
	// HACK - to prevent text from running into next line
	drawPipe->setScissorRect(x, y, x + w, y + 12);
	ocraFont.Print(x, x + w, y, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, thisAlpha), 
				GSFONT_ALIGN_LEFT, saveEntries[item].name);
	drawPipe->setScissorRect(0, 0, 320, 240);

	char sizeString[16];
	sprintf(sizeString, "%d", saveEntries[item].size);
	ocraFont.Print(x + w + 3, x + w + 3 + 36, y, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, thisAlpha), 
				GSFONT_ALIGN_LEFT, sizeString);
}

void guiSaveManager::drawScrollBar(float scrollBarScale, float scrollBarPos)
{
	int scrollHeight = (int)((float)108 * scrollBarScale);
	int scrollY = 94 + (int)(scrollBarPos * (float)(108 - scrollHeight));

	drawPipe->RectFlat(296, 94, 304, 202, Z_BOX2, GS_SET_RGBA(111, 114, 171, 86));
	drawPipe->RectFlat(297, scrollY, 302, scrollY + scrollHeight, Z_BOX3,
		GS_SET_RGBA(166, 170, 255, 86));
}

int guiSaveManager::updateAndFill()
{
	t_aioDent *dirEnts;
	t_pgenSaveBuffer *saveBuffer;
	int count = 0;

	freeSpace = currentSaver->saverAIO->freespace();
	pgenUsed = 0;

	dirEnts = (t_aioDent *)memalign(64, 2048 * sizeof(t_aioDent));
	saveBuffer = (t_pgenSaveBuffer *)memalign(64, sizeof(t_pgenSaveBuffer));
	if(!dirEnts || !saveBuffer)
		guiFatalError("Failed to allocate memory!");

	numEntries = currentSaver->saverAIO->getdir(currentSaver->savePath, ".pgs, .pgr", dirEnts, 2048);
	if(numEntries < 0)
	{
		free(dirEnts);
		free(saveBuffer);
		return -1;
	}

	for(int i = 0; i < numEntries; i++)
	{
		if(!strcmp(dirEnts[i].name, ".."))
			continue;

		char openName[128];

		if(!strcmp(currentSaver->savePath, "/"))
			sprintf(openName, "/%s", dirEnts[i].name);
		else
			sprintf(openName, "%s/%s", currentSaver->savePath, dirEnts[i].name);

		strcpy(saveEntries[count].filename, openName);

		int fd = currentSaver->saverAIO->open(openName, O_RDONLY);
		if(fd < 0)
		{
			free(dirEnts);
			free(saveBuffer);
			return -1;
		}

		currentSaver->saverAIO->read(fd, (u8 *)&saveBuffer->header, sizeof(saveBuffer->header));
		currentSaver->saverAIO->close(fd);

		char *extension = strrchr(dirEnts[i].name, '.');
		if(!strcmp(extension, ".pgr"))
			sprintf(saveEntries[count].name, "(SR) %s", saveBuffer->header.gameName);
		else
			strcpy(saveEntries[count].name, saveBuffer->header.gameName);

		saveEntries[count].size = dirEnts[i].size / 1024;
		if(saveEntries[count].size == 0) saveEntries[count].size = 1;

		pgenUsed += dirEnts[i].size / 1024;
	
		count++;
	}

	free(dirEnts);
	free(saveBuffer);
	numEntries = count;
	refreshScrollList(numEntries);
}

int guiSaveManager::deleteSave()
{
	return currentSaver->saverAIO->remove(deleteEntry->filename);
}
