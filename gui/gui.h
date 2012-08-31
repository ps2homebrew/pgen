#ifndef _GUI_H
#define _GUI_H

#define GUI_STAT_RUNNING		0x01
#define GUI_STAT_LOAD_ROM		0x02
#define GUI_STAT_EXEC_ROMLIST	0x04
#define GUI_STAT_EXEC_OPTIONS	0x08
#define GUI_STAT_EXEC_SAVEMAN	0x10
#define GUI_STAT_EXEC_CREDITS	0x20
#define GUI_STAT_EXEC_INGAME	0x40

#define GUI_FLAG_ANIM_OPEN		1
#define GUI_FLAG_ANIM_CLOSE		2

#define Z_BG		1
#define Z_BG_MASK	3		
#define Z_BOX1		4
#define Z_BOX2		5
#define Z_BOX3		6
#define Z_LIST		7
#define Z_SCROLL	8
#define Z_SCROLL_M	9
#define Z_SCROLL_M2	10
#define Z_SCROLL_M3	11
#define Z_FADE		12

class guiAnimateBox
{
	public:

		// Direction = GUI_FLAG_ANIM_*
		guiAnimateBox(int x1, int y1, int x2, int y2, u32 color, int direction);

		// Returns 0 while still drawing, 1 when finished
		int drawStep();

	private:

		int xOff, yOff;
		int xDest, yDest;

		int x1, y1, x2, y2;
		u32 color;
		int direction;

		int finished;
};


// Draws a dialog box. 
class guiDialogBox
{
	public:

		guiDialogBox(int x, int y, int w, int h, const char *title, const char *body);
		void draw();

	private:

		int x, y, w, h;
		int bodyY;
		const char *title, *body;
};

class guiComponentIf
{
	public:

		virtual void draw() = 0;
		virtual void update(u32 padRepeat, u32 padNoRepeat) = 0;

		u32 getStatus()
		{
			return status;
		}
		void reset()
		{
			status = GUI_STAT_RUNNING;
			flag = GUI_FLAG_ANIM_OPEN;
		}
		guiComponentIf()
		{
			boxAnim = NULL;
			status = GUI_STAT_RUNNING;
			flag = GUI_FLAG_ANIM_OPEN;
			alpha = 0x80;
			alphaDiff = -8;
			direction = 0;
		}

		virtual ~guiComponentIf() { };

	protected:

		void updateAlpha()
		{
			alpha += alphaDiff;
			if(alpha <= 0)
				alphaDiff = 8;
			else if(alpha >= 0x80)
				alphaDiff = -8;
		}

		u32 status, flag;
		int alpha, alphaDiff, direction;

		guiAnimateBox *boxAnim;
};

class guiMainMenu : public guiComponentIf
{
		public:

			guiMainMenu();

			void draw();
			void update(u32 padRepeat, u32 padNoRepeat);

		private:

			int selection;
};

pgenRom *pgenMenu(int fadeIn);
void guiDisplayLoading();
void guiDisplayLoadingEnd();

void guiDoDisplayCalibrate();
void guiUpdateDisplayPosition();

void guiDoIngameMenu();
void guiInit();

void guiFadeOutIngame();
void guiFadeInGui();
void guiUploadTextures();
void guiDrawBackground();
void guiDrawScroll();
void guiUpdateScreen();
void guiUpdateInput(u32 *padRepeat, u32 *padNoRepeat);
void guiError(const char *message);
void guiFatalError(const char *message);

#endif 
