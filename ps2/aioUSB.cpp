/* Copyright 2006 by Mega Man */

#include "pgen.h"

#include "fileXio_rpc.h"


//#define AIOMC_DEBUG


usbIO::usbIO(int maxEntries)
{
#ifdef AIOMC_DEBUG
	printf("usbIO constructor.\n");
#endif

	fatEntries = NULL;

	fatEntries = (fat_dir *)memalign(64, sizeof(fat_dir) * maxEntries);
	if(!fatEntries)
		guiFatalError("Failed to allocate memory!");

	// TODO: remove when not used anymore
	strcpy(devname, "mass:");
	status = 0;
}

usbIO::~usbIO()
{
#ifdef AIOMC_DEBUG
	printf("usbIO destructor.\n");
#endif

	if(fatEntries)
		free(fatEntries);
}

int usbIO::open(const char *name, int flags)
{
	char temp[1024];

#ifdef AIOMC_DEBUG
	printf("usbIO open %s\n", name);
#endif
	snprintf(temp, 1024, "%s%s", devname, name);
	temp[1023] = '\0';

	return fioOpen(temp, flags);
}

int usbIO::close(int fd)
{
#ifdef AIOMC_DEBUG
	printf("usbIO close\n");
#endif

	return fioClose(fd);
}

int usbIO::read(int fd, unsigned char *buffer, int size)
{
#ifdef AIOMC_DEBUG
	printf("usbIO read %d\n", size);
#endif

	return fioRead(fd, buffer, size);
}

int usbIO::write(int fd, const unsigned char *buffer, int size)
{
#ifdef AIOMC_DEBUG
	printf("usbIO write %d\n", size);
#endif

	return fioWrite(fd, buffer, size);
}

int usbIO::lseek(int fd, int offset, int whence)
{
#ifdef AIOMC_DEBUG
	printf("usbIO lseek\n");
#endif

	return fioLseek(fd, offset, whence);
}

int usbIO::remove(const char *name)
{
	int rv;
	char removeName[1024];

#ifdef AIOMC_DEBUG
	printf("usbIO remove\n");
#endif

	snprintf(removeName, 1024, "%s%s", devname, name);
	removeName[1023] = '\0';

	rv = fioRemove(removeName);
	if(!rv)
		rv = fioRmdir(removeName);

	return rv;
}

int usbIO::rename(const char *old, const char *newname)
{
	char oldString[1024];
	char newString[1024];
#ifdef AIOHDD_DEBUG
	printf("usbIO rename\n");
#endif

	snprintf(oldString, 1024, "%s%s", devname, old);
	snprintf(newString, 1024, "%s%s", devname, newname);

	return fileXioRename(oldString, newString);
}

int usbIO::mkdir(const char *name)
{
	char temp[1024];

#ifdef AIOMC_DEBUG
	printf("usbIO mkdir %s\n", name);
#endif
	snprintf(temp, 1024, "%s%s", devname, name);
	temp[1023] = '\0';

	return fioMkdir(temp);
}

int usbIO::rmdir(const char *name)
{
	char removeName[1024];

#ifdef AIOMC_DEBUG
	printf("usbIO rmdir\n");
#endif

	snprintf(removeName, 1024, "%s%s", devname, name);
	removeName[1023] = '\0';

	return fioRmdir(removeName);
}

// Hacked for PGEN - add a ".." to the top of the list if its not there already
int usbIO::getdir(const char *name, const char *extensions, t_aioDent dentBuf[], int maxEnt)
{
	iox_dirent_t thisDir;
	int count = 0, dirFd, rv;
	char openString[1024];

	snprintf(openString, 1024, "%s%s", devname, name);

#ifdef AIOHDD_DEBUG
	printf("usbIO getdir. openString = %s\n", openString);
#endif

	dirFd = fileXioDopen(openString);
	if(dirFd < 0)
		return dirFd;

	if(!strcmp(name, "/"))
	{
		strcpy(dentBuf[0].name, "..");
		dentBuf[0].attrib = AIO_ATTRIB_DIR;
		dentBuf[0].size = 512;
		count++;
	}

	rv = fileXioDread(dirFd, &thisDir);
	while((rv > 0) && (count < maxEnt))
	{
		if(strcmp(thisDir.name, "."))
		{
			if(	(tocEntryCompare(thisDir.name, extensions)) ||
				(FIO_S_ISDIR(thisDir.stat.mode)))
			{

				strncpy(dentBuf[count].name, thisDir.name, 256);
				dentBuf[count].name[255] = '\0'; // for safety
				dentBuf[count].attrib = 0;
				if(FIO_S_ISDIR(thisDir.stat.mode))
					dentBuf[count].attrib |= AIO_ATTRIB_DIR;
				if(FIO_S_ISLNK(thisDir.stat.mode))
					dentBuf[count].attrib |= AIO_ATTRIB_SYMLINK;
				dentBuf[count].size = thisDir.stat.size;

				count++;
			}
		}
		rv = fileXioDread(dirFd, &thisDir);
	}

	fileXioDclose(dirFd);

#ifdef AIOHDD_DEBUG
	printf("usbIO getdir returning.. count = %d\n", count);
#endif

	return count;
}

int usbIO::getstat(const char *name, t_aioDent *dent)
{
	iox_stat_t thisStat;
	char filename[1024];
	int rv;

#ifdef AIOHDD_DEBUG
	printf("usbIO getstat\n");
#endif

	// Special case: name = "/"
	if(!strcmp(name, "/"))
	{
		strcpy(dent->name, "/");
		dent->attrib = AIO_ATTRIB_DIR;
		dent->size = 512;
	}
	else
	{
		snprintf(filename, 1024, "%s%s", devname, name);

		rv = fileXioGetStat(filename, &thisStat);
		if(rv < 0)
			return rv;

		strcpy(dent->name, name);
		dent->attrib = 0;
		if(FIO_S_ISDIR(thisStat.mode))
			dent->attrib |= AIO_ATTRIB_DIR;
		if(FIO_S_ISLNK(thisStat.mode))
			dent->attrib |= AIO_ATTRIB_SYMLINK;
		dent->size = thisStat.size;
	}

	return 0;
}

u32 usbIO::getstatus()
{
#ifdef AIOMC_DEBUG
	printf("usbIO getstatus\n");
#endif

	return status;
}

void usbIO::clearstatus(u32 bits)
{
#ifdef AIOMC_DEBUG
	printf("usbIO clearstatus\n");
#endif

	status &= ~bits;
}

int usbIO::freespace()
{
#ifdef AIOHDD_DEBUG
	printf("usbIO freespace\n");
#endif

	int zoneFree = fileXioDevctl(devname, PFSCTL_GET_ZONE_FREE, NULL, 0, NULL, 0);
	int zoneSize = fileXioDevctl(devname, PFSCTL_GET_ZONE_SIZE, NULL, 0, NULL, 0);

	// Return value as kb's
	return zoneFree * zoneSize / 1024;

}

int usbIO::tocEntryCompare(char* filename, const char* extensions)
{
	static char ext_list[129];
	char* token;
	char* ext_point;

	if(!extensions)
		return 1;
	
	strncpy(ext_list,extensions,128);
	ext_list[128]=0;

	token = strtok( ext_list, " ," );
	while( token != NULL )
	{
		// if 'token' matches extension of 'filename'
		// then return a match
		ext_point = strrchr(filename,'.');
		if(ext_point == NULL) return 0;

		if(strcasecmp(ext_point, token) == 0)
			return 1;
		
		/* Get next token: */
		token = strtok( NULL, " ," );
	}
	
	// If not match found then return FALSE
	return 0;	
}
