#ifndef _SAVE_H
#define _SAVE_H

class pgenSaver
{
	public:

		virtual int checkSaveExist() = 0;
		virtual int createSave() = 0;

		abstractIO *saverAIO;
		char savePath[256];
};

// Only supports memory card in port 0, slot 0
class mcSaver : public pgenSaver
{
	public:

		mcSaver(const char *savePath, abstractIO *saverAIO);

		int checkSaveExist();
		int createSave();	

};

class hddSaver : public pgenSaver
{
	public:

		hddSaver(const char *savePath, abstractIO *saverAIO);

		int checkSaveExist();
		int createSave();	

		int exists;
};

#endif /* _SAVE_H */
