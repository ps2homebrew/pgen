#ifndef _INGAME_H
#define _INGAME_H

#define INGAME_FLAG_ERROR		10

class guiIngameMenu : public guiComponentIf
{
	public:

		guiIngameMenu();
		void draw();
		void update(u32 padRepeat, u32 padNoRepeat);

	private:

		void updateIngameStrings();
		char ingameString[10][64];
		int selection;
		int mode;	// 0 = return to game, 1 = exit to main menu

		char *currentError;
};

#endif
