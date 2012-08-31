#include "pgen.h"

#include "aio.h"
#include "gui.h"
#include "scrollList.h"
#include "romlist.h"

//#define	SHOW_ALL_FILES
#define	LIST_ALL_FS

guiRomlist::guiRomlist(int x, int y, int w, int h) : scrollList(x + 3, y + 2, w - 3, h - 2, 10, 13)
{
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	scrollWidth = 8;

	strcpy(defaultMemcardDir, "/BRDATA-SYSTEM");
	u8 romver[16];
	int fd = fioOpen("rom0:ROMVER", O_RDONLY);
	fioRead(fd, romver, sizeof romver);
	fioClose(fd);
	defaultMemcardDir[2] = (romver[4] == 'E' ? 'E' :
			(romver[4] == 'J' ? 'I' : 'A'));

	virtualRomlist = (t_romlistEntry *)malloc(sizeof(t_romlistEntry) * 128);
	romlist = (t_romlistEntry *)malloc(sizeof(t_romlistEntry) * ROMLIST_MAX_ENTRIES);
	sortedList = (t_romlistEntry **)malloc(sizeof(t_romlistEntry *) * ROMLIST_MAX_ENTRIES);
	if(!romlist || !virtualRomlist || !sortedList)
		guiFatalError("Failed to allocate memory!");

	dirLevel = 0;
	strcpy(path, "/");

	fillVirtualDir();
	setList(virtualRomlist, numVirtual);
}

void guiRomlist::draw()
{
	switch(flag)
	{
		case GUI_FLAG_ANIM_OPEN:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(x, y, x + w + scrollWidth, y + h, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_OPEN);

			if(boxAnim->drawStep())
			{
				flag = 0;
				delete(boxAnim);
				boxAnim = NULL;
			}

			break;

		case GUI_FLAG_ANIM_CLOSE:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(x, y, x + w + scrollWidth, y + h, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_CLOSE);

			if(boxAnim->drawStep())
			{
				flag = 0;
				delete(boxAnim);
				boxAnim = NULL;

				// If mode = 0, then simply close rombox
				if(mode == 0)
					status = 0;
				// If mode = 1, exec game
				else
					status = GUI_STAT_LOAD_ROM;
			}

			break;

		default:

			drawPipe->RectFlat(x, y, x + w, y + h, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));			
			updateAlpha();
			drawScrollList();

	}
}

void guiRomlist::update(u32 padRepeat, u32 padNoRepeat)
{
	if(padNoRepeat & PAD_CROSS)
	{
		int selection = getSelection();

		// If a directory was selected..
		if(sortedList[selection]->flags & ROMLIST_FLAG_DIRECTORY)
		{
			if(!strcmp(sortedList[selection]->gameName, ".."))
			{
				// If dirLevel = 1 and selection = "..", we are returning to the virtual
				// directory. Unmount current aio etc
				if(dirLevel == 1)
				{
					if (currentAIO != saverHddAIO)
						delete(currentAIO);
					currentAIO = NULL;

					dirLevel--;
					setList(virtualRomlist, numVirtual);
					refreshScrollList(numVirtual);
				}
				// Otherwise, we are changing to a parent directory in the current AIO.
				// Update pwd and re-fetch romlist.
				else
				{
					char *ptr = strrchr(path, '/');
					int idx = (int)ptr - (int)path;

					if(!ptr)
						guiFatalError("Failed to change to parent");

					path[idx] = '\0';

					// Special case: we moved into the root dir, and just terminated the root '/'
					// character. Gotta fix that.
					if(!strlen(path))
						strcpy(path, "/");

					dirLevel--;
					fillRomlist();
				}
			}
			else
			{
				// If dirLevel = 0 and selection != "..", we have to mount a new AIO
				if(dirLevel == 0)
				{
					// Trying to mount cdrom
					if(strstr(sortedList[selection]->fileName, "cdfs:"))
						currentAIO = new cdvdIO(2048);
					// Trying to mount HDD partition
					else if (strcmp(sortedList[selection]->fileName, "hdd0:PP.GEN") == 0)
						currentAIO = saverHddAIO;
					else if(strstr(sortedList[selection]->fileName, "hdd0:"))
						currentAIO = new hddIO(sortedList[selection]->fileName);
					// Trying to mount from memory cards
					else if(strstr(sortedList[selection]->fileName, "mc0:"))
						currentAIO = new mcIO(0, 2048);
					else if(strstr(sortedList[selection]->fileName, "mc1:"))
						currentAIO = new mcIO(1, 2048);
					else if(strstr(sortedList[selection]->fileName, "mass:"))
						currentAIO = new usbIO(2048);
					else
						guiFatalError("Try to mount unknown AIO!");

					if(currentAIO->getstatus() & AIO_STATE_ERROR)
					{
						if (currentAIO != saverHddAIO)
							delete(currentAIO);
						currentAIO = NULL;
						guiError("Failed to mount device");
					}
					else
					{
						// If entering memcard, try and start in BRDATA-SYSTEM directory
						if(	strstr(sortedList[selection]->fileName, "mc0:") ||
							strstr(sortedList[selection]->fileName, "mc1:"))
						{
							t_aioDent dent;

							if(currentAIO->getstat(defaultMemcardDir, &dent) == 0)
							{
								strcpy(path, defaultMemcardDir);
								dirLevel++;
							}
						}
						else
							strcpy(path, "/");

						dirLevel++;
						fillRomlist();					
					}

				}
				// Otherwise we are trying to move to a subdir in the current AIO
				else
				{						
					strcpy(path, sortedList[selection]->fileName);
					dirLevel++;
					fillRomlist();					
				}
			}
		}
		// Otherwise a rom was selected
		else
		{
			strcpy(currentFileName, sortedList[selection]->fileName);		
			mode = 1;
			flag = GUI_FLAG_ANIM_CLOSE;
		}
	}

	updateScrollList(padRepeat, padNoRepeat);

	if(padRepeat & PAD_TRIANGLE)
	{
		mode = 0;
		flag = GUI_FLAG_ANIM_CLOSE;
	}
}

abstractIO *guiRomlist::getRomAIO()
{
	return currentAIO;
}

const char *guiRomlist::getRomFilename()
{
	return (const char *)currentFileName;
}

void guiRomlist::drawItem(int item, int selected, int x, int w, int y)
{
	int thisAlpha;

	if(selected) thisAlpha = alpha;
	else thisAlpha = 180;

	if(sortedList[item]->flags & ROMLIST_FLAG_DIRECTORY)
	{
		drawPipe->TextureSet(pgenTexEnv.pgenFolder.texBufPtr, pgenTexEnv.pgenFolder.texBufWidth,
			dispDriver->getTexSizeFromInt(pgenTexEnv.pgenFolder.width), 
			dispDriver->getTexSizeFromInt(pgenTexEnv.pgenFolder.height), 
			pgenTexEnv.pgenFolder.psm, 0, 0, 0, 0);

		drawPipe->RectTexture(x, y, 0, 0, x + pgenTexEnv.pgenFolder.width, y + pgenTexEnv.pgenFolder.height,
			pgenTexEnv.pgenFolder.width, pgenTexEnv.pgenFolder.height, Z_SCROLL, 
			GS_SET_RGBA(0x80, 0x80, 0x80, 0x80));

		x += pgenTexEnv.pgenFolder.width + 2; // 2 pixels space
	}

	// TODO: remove need for hack below
	// HACK - to prevent text from running into next line
	drawPipe->setScissorRect(x, y, x + w, y + 12);
	ocraFont.Print(x, x + w, y, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, thisAlpha), 
				GSFONT_ALIGN_LEFT, sortedList[item]->gameName);
	drawPipe->setScissorRect(0, 0, 320, 240);
}

void guiRomlist::fillVirtualDir()
{
	strcpy(virtualRomlist[0].gameName, "CD-ROM");
	strcpy(virtualRomlist[0].fileName, "cdfs:");
	virtualRomlist[0].flags = ROMLIST_FLAG_DIRECTORY;

	strcpy(virtualRomlist[1].gameName, "Memory Card 1");
	strcpy(virtualRomlist[1].fileName, "mc0:");
	virtualRomlist[1].flags = ROMLIST_FLAG_DIRECTORY;

	strcpy(virtualRomlist[2].gameName, "Memory Card 2");
	strcpy(virtualRomlist[2].fileName, "mc1:");
	virtualRomlist[2].flags = ROMLIST_FLAG_DIRECTORY;

	strcpy(virtualRomlist[3].gameName, "USB Stick");
	strcpy(virtualRomlist[3].fileName, "mass:");
	virtualRomlist[3].flags = ROMLIST_FLAG_DIRECTORY;

	numVirtual = 4;

	if(pgenRuntimeSetting.HDDFormatted)
	{
		t_hddFilesystem *hddFs = (t_hddFilesystem *)memalign(64, sizeof(t_hddFilesystem) * 127);
		if(!hddFs)
			guiFatalError("Failed to allocate memory!");

		int numFs = hddGetFilesystemList(hddFs, 127);
		for(int i = 0; i < numFs; i++)
		{
#ifndef LIST_ALL_FS
			if(hddFs[i].fileSystemGroup == FS_GROUP_COMMON)
			{
#endif
				if (hddFs[i].formatted)
				{
					strcpy(virtualRomlist[numVirtual].gameName, hddFs[i].name);
					strcpy(virtualRomlist[numVirtual].fileName, hddFs[i].filename);
					virtualRomlist[numVirtual].flags = ROMLIST_FLAG_DIRECTORY;

					numVirtual++;
				}
#ifndef LIST_ALL_FS
			}
#endif
		}

		free(hddFs);
	}

	refreshScrollList(numVirtual);
}

void guiRomlist::fillRomlist()
{

	romlistDent = (t_aioDent *)memalign(64, sizeof(t_aioDent) * ROMLIST_MAX_ENTRIES);
	if(!romlistDent)
		guiFatalError("Failed to allocate memory!");

#ifdef SHOW_ALL_FILES
	numRoms = currentAIO->getdir(path, NULL, romlistDent, ROMLIST_MAX_ENTRIES);
#else
	numRoms = currentAIO->getdir(path, ".smd, .bin, .zip, .gen", romlistDent, ROMLIST_MAX_ENTRIES);
#endif

	if(numRoms < 0)
	{
		if (currentAIO != saverHddAIO)
			delete(currentAIO);
		currentAIO = NULL;
		free(romlistDent);

		dirLevel = 0;
		setList(virtualRomlist, numVirtual);
		refreshScrollList(numVirtual);

		guiError("Failed to read from device");

		return;
	}

	for(int i = 0; i < numRoms; i++)
	{
		if(!strcmp(path, "/"))
			snprintf(romlist[i].fileName, 128, "/%s", romlistDent[i].name);
		else
			snprintf(romlist[i].fileName, 128, "%s/%s", path, romlistDent[i].name);
		
		if(romlistDent[i].attrib & AIO_ATTRIB_DIR)
			romlist[i].flags = ROMLIST_FLAG_DIRECTORY;
		else
			romlist[i].flags = 0;

		// strlen - 4, to get rid of the extension.
		if(romlist[i].flags & ROMLIST_FLAG_DIRECTORY)
			strcpy(romlist[i].gameName, romlistDent[i].name);
		else
		{
			strncpy(romlist[i].gameName, romlistDent[i].name, strlen(romlistDent[i].name) - 4);
			romlist[i].gameName[strlen(romlistDent[i].name) - 4] = '\0';
		}
	}

	free(romlistDent);

	sortList(romlist, numRoms);
	refreshScrollList(numRoms);
}

void guiRomlist::drawScrollBar(float scrollBarScale, float scrollBarPos)
{
	int scrollHeight = (int)((float)h * scrollBarScale);
	int scrollY = y + (int)(scrollBarPos * (float)(h - scrollHeight));

	drawPipe->RectFlat(x + w, y, x + w + scrollWidth, y + h, Z_BOX2, GS_SET_RGBA(111, 114, 171, 86));

	if(numRoms > itemsPage)
		drawPipe->RectFlat(x + w + 1, scrollY, x + w + scrollWidth - 2, scrollY + scrollHeight, Z_BOX3,
			GS_SET_RGBA(166, 170, 255, 86));
}

int sortCompare(const void *l, const void *r)
{
	t_romlistEntry **left = (t_romlistEntry **)l;
	t_romlistEntry **right = (t_romlistEntry **)r;

	if(!strcmp((*left)->gameName, ".."))
		return -1;

	if(((*left)->flags & ROMLIST_FLAG_DIRECTORY) && !((*right)->flags & ROMLIST_FLAG_DIRECTORY))
		return -1;
	else if(((*right)->flags & ROMLIST_FLAG_DIRECTORY) && !((*left)->flags & ROMLIST_FLAG_DIRECTORY))
		return 1;

	return strcmp((*left)->gameName, (*right)->gameName);
}

extern "C" void my_qsort(char *a, int n, int es, int (*cmp)(const void *, const void *));

void guiRomlist::sortList(t_romlistEntry *currentList, int size)
{
	setList(currentList, size);
	my_qsort((char *)sortedList, size, sizeof(t_romlistEntry *), sortCompare);
}

void guiRomlist::setList(t_romlistEntry *currentList, int size)
{
	for(int i = 0; i < size; i++)
		sortedList[i] = &currentList[i];
}
