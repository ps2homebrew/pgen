// amigamod_rpc.cpp

#include <tamtypes.h>
#include <kernel.h>
#include <sifcmd.h>
#include <sifrpc.h>
#include <iopheap.h>
#include <string.h>

#include "amigamod.h"

static struct t_SifRpcClientData amodCd __attribute__((aligned(64)));
static unsigned sbuff[64] __attribute__((aligned (64)));

static int ammodi = 0;
static void *iopmodimg = 0;


int amigaModInit(int nosdinit)
{
    if (ammodi)
        return -1;

    if (SifBindRpc( &amodCd, VZMOD, 0) < 0)
        return -1;

    if (!nosdinit)
    {
        char hi[16] = "amigaModInit !";
        memcpy((char *)sbuff, hi, 16);
        SifCallRpc( &amodCd, MOD_INIT, 0, (void *)(&sbuff[0]), 16, (void *)(&sbuff[0]), 64, 0, 0);
    }

   ammodi = 1;
    return 0;
}


// must only be called when playback is paused
int amigaModLoad( void *moddata, int size )
{
    int i;
    struct t_SifDmaTransfer sdt;

	if (!ammodi)
        return -1;

    if (SifInitIopHeap() != 0)
        return -1;

    if (iopmodimg)
        SifFreeIopHeap(iopmodimg);
    iopmodimg = SifAllocIopHeap(size);

    // transfer the moddata via dma into spu RAM
    sdt.src = (void*)moddata;
    sdt.dest = iopmodimg;
    sdt.size = size;
    sdt.attr = 0;
    i = SifSetDma(&sdt, 1);
    while (SifDmaStat(i) >= 0); // potential for infinite loop ...

    sbuff[0] = (int)iopmodimg;
    SifCallRpc( &amodCd, MOD_LOAD, 0, (void *)(&sbuff[0]), 4, (void *)(&sbuff[0]), 64, 0, 0);

    return sbuff[0];
}


int amigaModPlay(unsigned linear)
{
	if (!ammodi)
        return -1;

    sbuff[0] = linear;
    SifCallRpc( &amodCd, MOD_PLAY, 0, (void *)(&sbuff[0]), 4, (void *)(&sbuff[0]), 16, 0, 0);
    return sbuff[0];
    //return 0;
}


// won't pause right away; best to wait one VSync after this is called, before another amigaMod* call
int amigaModPause()
{
	if (!ammodi)
        return -1;

    SifCallRpc( &amodCd, MOD_PAUSE, 1, (void *)(&sbuff[0]), 4, (void *)(&sbuff[0]), 0, 0, 0);
    //return sbuff[0];
    return 0;
}

// volume between 0 and 0x3fff
int amigaModSetVolume( unsigned short volume)
{
	if (!ammodi)
        return -1;

    sbuff[0] = volume & 0x3fff;
    SifCallRpc( &amodCd, MOD_SETVOL, 0, (void *)(&sbuff[0]), 4, (void *)(&sbuff[0]), 0, 0, 0);
    return 0;
}


int amigaModGetInfo( ModInfoStruct *info)
{
	if (!ammodi)
        return -1;

    SifCallRpc( &amodCd, MOD_GETINFO, 0, (void *)(&sbuff[0]), 4, (void *)(&sbuff[0]), 64, 0, 0);
    info->curorder = sbuff[1];
    info->currow = sbuff[2];
    info->numchannels = sbuff[3];
    info->bpm = sbuff[4];
    //return sbuff[0];
    return 0;
}


int amigaModQuit()
{
	if (!ammodi)
        return -1;

    SifCallRpc( &amodCd, MOD_QUIT, 1, (void *)(&sbuff[0]), 4, (void *)(&sbuff[0]), 0, 0, 0);
	ammodi = 0;

    return 0;
}


void ModPuts(char *s)
{
	if (!ammodi)
        return;

    memcpy((char *)sbuff, s, 252);
    SifCallRpc( &amodCd, MOD_PUTS, 1, (void *)(&sbuff[0]), 252, (void *)(&sbuff[0]), 0, 0, 0);
}

