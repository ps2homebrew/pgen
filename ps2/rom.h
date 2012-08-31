#ifndef _ROM_H
#define _ROM_H

class pgenRom
{
	public:

		pgenRom(abstractIO *aio, const char *filename);
		~pgenRom();

		int isRomLoaded();

		pgenEmuState *state;

	private:

		int loadRom();
		void readRom(abstractIO *aio, const char *filename);
		void readZippedRom(abstractIO *aio, const char *zipname);

		u8 *romData;
		int romSize;
		int loaded;
};

#endif /* _ROM_H */
