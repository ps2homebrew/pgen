#ifndef _SAVEMAN_H
#define _SAVEMAN_H

#define SAVEMAN_FLAG_POPULATING			10
#define SAVEMAN_FLAG_CONFIRM_DELETE		11
#define SAVEMAN_FLAG_DELETE				12
#define SAVEMAN_FLAG_ERROR				13

// SAVE_TYPE_STATE etc defined in state.h

typedef struct
{
	char name[64];
	char filename[64];
	int size;
} saveManEntry;

class guiSaveManager : public scrollList, public guiComponentIf
{
		public:

			guiSaveManager();
			void draw();
			void update(u32 padRepeat, u32 padNoRepeat);

		private:

			// Deletes the current selection, returns negative on error
			int deleteSave();

			// Updates free/used variables and fills dirEntries
			int updateAndFill();

			void drawItem(int item, int selected, int x, int w, int y);
			void drawScrollBar(float scrollBarScale, float scrollBarPos);

			void drawBody();

			saveManEntry *saveEntries;
			int numEntries;

			int freeSpace, pgenUsed;
			int counter;

			int deleteSelection;
			saveManEntry *deleteEntry;

			char *currentError;
			int errorFatal;
};

#endif /* _SAVEMAN_H */
