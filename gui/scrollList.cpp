#include "pgen.h"

#include "aio.h"
#include "gui.h"
#include "scrollList.h"


scrollList::scrollList(int x, int y, int w, int h, int itemsPage, int itemHeight)
{
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	framePosition = selection = numItems = 0;
	padCountdown = 10;
	padHeldDown = 0;
	this->itemsPage = itemsPage;
	this->itemHeight = itemHeight;
}

void scrollList::drawScrollList()
{
	int thisY = y;

	for(int i = framePosition; i < (framePosition + itemsPage); i++)
	{
		if(i >= numItems)
			break;

		drawItem(i, i == selection, x, w, thisY);

		thisY += itemHeight;
	}

	float scale = (float)itemsPage / (float)numItems;
	if(scale > 1.0f) scale = 1.0f;
	else if(scale < 0.1f) scale = 0.1f;

	int thisItems = numItems - 1;
	if(thisItems <= 0)
		thisItems = 1;

	drawScrollBar(scale, (float)selection / (float)thisItems);
}

void scrollList::refreshScrollList(int numItems)
{
	selection = 0;
	framePosition = 0;
	this->numItems = numItems;
}

void scrollList::updateScrollList(u32 padRepeat, u32 padNoRepeat)
{
	if(padCountdown) padCountdown--;

	// Scroll UP
	if((padRepeat & PAD_UP) && !padCountdown)
	{
		if(padHeldDown++ < 4) padCountdown = 10;
		else padCountdown = 2;

		selection--;
		if(selection < framePosition)
			framePosition--;

		if(selection < 0)
		{
			selection = numItems - 1;

			if(numItems > itemsPage)
				framePosition = numItems - itemsPage;
			else
				framePosition = 0;
		}
	}
	// Scroll DOWN
	else if((padRepeat & PAD_DOWN) && !padCountdown)
	{
		if(padHeldDown++ < 4) padCountdown = 10;
		else padCountdown = 2;

		selection++;
		if(selection >= (framePosition + itemsPage))
			framePosition++;

		if(selection > (numItems - 1))
		{
			selection = 0;
			framePosition = 0;
		}
	}
	// Scroll to top
	else if(padRepeat & PAD_L1)
	{
		selection = 0;
		framePosition = 0;
	}
	// Scroll to bottom
	else if(padRepeat & PAD_L2)
	{
		selection = numItems - 1;

		if(numItems > itemsPage)
			framePosition = numItems - itemsPage;
		else
			framePosition = 0;
	}
	// Scroll page up
	else if((padRepeat & PAD_LEFT) && !padCountdown && (framePosition >= itemsPage))
	{
		padCountdown = 10;

		selection -= itemsPage;
		framePosition -= itemsPage;

	}
	// Scroll page down
	else if((padRepeat & PAD_RIGHT) && !padCountdown && (framePosition <= (numItems - 2 * itemsPage)))
	{
		padCountdown = 10;

		selection += itemsPage;
		framePosition += itemsPage;
	}

	if(!(padRepeat & (PAD_UP | PAD_DOWN)))
		padHeldDown = 0;
}

int scrollList::getSelection()
{
	return selection;
}
