#ifndef _OPTIONS_H
#define _OPTIONS_H

class guiOptionsMenu : public guiComponentIf
{
		public:

			guiOptionsMenu();
			void draw();
			void update(u32 padRepeat, u32 padNoRepeat);

		private:

			void updateOptionString();

			char optionString[9][64];
			int selection;

};

#endif
