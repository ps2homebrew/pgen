#include "pgen.h"
#include "fileXio_rpc.h"
#include "sys/fcntl.h"

mcSaver::mcSaver(const char *savePath, abstractIO *saverAIO)
{
	strcpy(this->savePath, savePath);
	this->saverAIO = saverAIO;
}

int mcSaver::checkSaveExist()
{
	mcTable mcdir;	// Will be aligned
	int ret;
	
	mcGetDir(0, 0, savePath, 0, 1, &mcdir);
	mcSync(MC_WAIT, NULL, &ret);

	if(ret <= 0) return 0;
	else return 1;
}

int mcSaver::createSave()
{
	int mcFd, ret, iconFd, iconSize;
	mcIcon iconSys;

	static iconIVECTOR bgcolor[4] = {
		{  68,  23, 116,  0 }, // top left
		{ 255, 255, 255,  0 }, // top right
		{ 255, 255, 255,  0 }, // bottom left
		{  68,  23, 116,  0 }, // bottom right
	};
	static iconFVECTOR lightdir[3] = {
		{ 0.5, 0.5, 0.5, 0.0 },
		{ 0.0,-0.4,-0.1, 0.0 },
		{-0.5,-0.5, 0.5, 0.0 },
	};
	static iconFVECTOR lightcol[3] = {
		{ 0.3, 0.3, 0.3, 0.00 },
		{ 0.4, 0.4, 0.4, 0.00 },
		{ 0.5, 0.5, 0.5, 0.00 },
	};
	static iconFVECTOR ambient = { 0.50, 0.50, 0.50, 0.00 };

	char dirName[128];
	sprintf(dirName, "mc0:%s", savePath);
	if((ret = fioMkdir(dirName)) < 0) 
		return -1;

	// Setup icon.sys
	memset(&iconSys, 0, sizeof(mcIcon));
	strcpy((char *)iconSys.head, "PS2D");
	strcpy_sjis((short *)&iconSys.title, "PGEN Saves");
	iconSys.nlOffset = 13;
	iconSys.trans = 0x60;
	memcpy(iconSys.bgCol, bgcolor, sizeof(bgcolor));
	memcpy(iconSys.lightDir, lightdir, sizeof(lightdir));
	memcpy(iconSys.lightCol, lightcol, sizeof(lightcol));
	memcpy(iconSys.lightAmbient, ambient, sizeof(ambient));
	strcpy((char *)iconSys.view, "sonic.ico");
	strcpy((char *)iconSys.copy, "sonic.ico");
	strcpy((char *)iconSys.del, "sonic.ico");

	// Write icon.sys
	char fileName[128];
	sprintf(fileName, "%s/icon.sys", dirName);
	mcFd = fioOpen(fileName,O_WRONLY | O_CREAT);
	if(mcFd < 0) return -1;

	fioWrite(mcFd, &iconSys, sizeof(iconSys));
	fioClose(mcFd);

	// Write icon file
	iconSize = 75888;	// TODO: define
	sprintf(fileName, "%s/sonic.ico", dirName);
	iconFd = fioOpen(fileName,O_WRONLY | O_CREAT);
	if(iconFd < 0) return -1;

	fioWrite(iconFd, sonicIcn, iconSize);
	fioClose(iconFd);

	return 0;
}



hddSaver::hddSaver(const char *savePath, abstractIO *saverAIO)
{
	strcpy(this->savePath, savePath);
	this->saverAIO = saverAIO;
	exists = 0;
}

int hddSaver::checkSaveExist()
{
	if(!exists)
	{
		int fd = fileXioOpen("hdd0:PP.GEN", O_RDONLY, 0);
		if(fd < 0)
			return 0;
		else
		{
			fileXioClose(fd);
			exists = 1;
			return 1;
		}
	}
	else
		return 1;
}

int hddSaver::createSave()
{
	int fd = fileXioOpen("hdd0:PP.GEN,128M", O_WRONLY | O_RDWR | O_CREAT | O_TRUNC, 0);
	if(fd < 0)
		return -1;

	fileXioClose(fd);

	int zoneSize = 8192;
	int rv = fileXioFormat("pfs0:", "hdd0:PP.GEN", (const char*)&zoneSize, sizeof(int));
	if(rv < 0)
		return -1;

	rv = fileXioMount("pfs2:", "hdd0:PP.GEN", FIO_MT_RDWR);
	if(rv < 0)
		return -1;

	char dirName[128];
	sprintf(dirName, "pfs2:%s", savePath);

	int fileMode =	FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP |
						FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH;

	rv = fileXioMkdir(dirName, fileMode);
	if(rv < 0)
		goto hddCreateSave_error;

	fileXioUmount("pfs2:");

	delete(saverAIO);
	saverAIO = new hddIO("hdd0:PP.GEN");
	exists = 1;

	return 0;

hddCreateSave_error:

	fileXioUmount("pfs2:");
	return -1;
}
