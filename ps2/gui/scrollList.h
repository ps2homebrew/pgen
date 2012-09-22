#ifndef _SCROLL_LIST_H
#define _SCROLL_LIST_H

class scrollList
{
	public:

		scrollList(int x, int y, int w, int h, int itemsPage, int itemHeight);

		void drawScrollList();
		void refreshScrollList(int numItems);
		void updateScrollList(u32 padRepeat, u32 padNoRepeat);
		int getSelection();

	protected:

		virtual void drawItem(int item, int selected, int x, int w, int y) = 0;
		virtual void drawScrollBar(float scrollBarScale, float scrollBarPos) = 0;

		int itemsPage, itemHeight;

	private:

		int x, y, w, h;
		int selection, framePosition, numItems;
		int padCountdown, padHeldDown;
};

#endif /* _SCROLL_LIST_H */
