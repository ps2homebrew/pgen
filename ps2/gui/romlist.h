#ifndef _ROMLIST_H
#define _ROMLIST_H

#define ROMLIST_MAX_ENTRIES			2048
#define ROMLIST_FLAG_DIRECTORY		0x01

typedef struct {
	char gameName[256];
	char fileName[256];
	u32 flags;
} t_romlistEntry;

class guiRomlist : public scrollList, public guiComponentIf
{
		public:

			guiRomlist(int x, int y, int w, int h);

			void draw();
			void update(u32 padRepeat, u32 padNoRepeat);

			abstractIO *getRomAIO();
			const char *getRomFilename();

		private:

			void drawItem(int item, int selected, int x, int w, int y);
			void drawScrollBar(float scrollBarScale, float scrollBarPos);

			void fillVirtualDir();
			void fillRomlist();
//			int sortCompare(const void *l, const void *r);
			void sortList(t_romlistEntry *currentList, int size);
			void setList(t_romlistEntry *currentList, int size);
			
			int dirLevel;						// 0 = virtual dir, >0 = mounted AIO
			char path[1024];
			char currentFileName[1024];
			char defaultMemcardDir[64];

			abstractIO *currentAIO;
			t_aioDent *romlistDent;				// allocated len = 2048
			t_romlistEntry *virtualRomlist;		// allocated len = 128
			t_romlistEntry *romlist;			// allocated len = 2048
			t_romlistEntry **sortedList;

			int numVirtual;
			int numRoms;
			int x, y, w, h;
			int scrollWidth;

			int mode;							// 0 = close rombox, 1 = exec game
};

#endif
