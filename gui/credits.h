#ifndef _CREDITS_H
#define _CREDITS_H

class guiCredits : public guiComponentIf
{
		public:

			guiCredits();
			void draw();
			void update(u32 padRepeat, u32 padNoRepeat);

		private:

			int creditLines;
			int scrollY;
			int scrollHeight;
			int scrollDelay;
};

#endif
