/*
    ----------------------------------------------------------------------
    sjpcm_irx.c - SjPCM IOP-side code. (c) Nick Van Veen (aka Sjeep), 2002
                                       (c) Mega Man, 2006
    ----------------------------------------------------------------------

    This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "irx_imports.h"

// LIBSD defines

#define SD_CORE_1			1
#define SD_P_BVOLL			((0x0F<<8)+(0x01<<7))
#define SD_P_BVOLR			((0x10<<8)+(0x01<<7))
#define SD_P_MVOLL			((0x09<<8)+(0x01<<7))
#define SD_P_MVOLR			((0x0A<<8)+(0x01<<7))

#define SD_INIT_COLD		0
#define SD_C_NOISE_CLK		(4<<1)

#define SD_BLOCK_ONESHOT	(0<<4)
#define SD_BLOCK_LOOP		(1<<4)

#define SD_TRANS_MODE_STOP  2

////////////////

#define	SJPCM_IRX		0xB0110C5
#define SJPCM_PUTS		0x01
#define	SJPCM_INIT		0x02
#define SJPCM_PLAY		0x03
#define SJPCM_PAUSE		0x04
#define SJPCM_SETVOL	0x05
#define SJPCM_ENQUEUE	0x06
#define SJPCM_CLEARBUFF	0x07
#define SJPCM_QUIT		0x08
#define SJPCM_GETAVAIL  0x09
#define SJPCM_GETBUFFD  0x10

#define TH_C		0x02000000

SifRpcDataQueue_t qd;
SifRpcServerData_t sd0;

void SjPCM_Thread(void* param);
void SjPCM_PlayThread(void* param);
static int SjPCM_TransCallback(void* param);

void* SjPCM_rpc_server(unsigned int fno, void *data, int size);
void* SjPCM_Puts(char* s);
void* SjPCM_Init(unsigned int* sbuff);
void* SjPCM_Enqueue(unsigned int* sbuff);
void* SjPCM_Play();
void* SjPCM_Pause();
void* SjPCM_Setvol(unsigned int* sbuff);
void* SjPCM_Clearbuff();
void* SjPCM_Available(unsigned int* sbuff);
void* SjPCM_Buffered(unsigned int* sbuff);
void* SjPCM_Quit();

extern void wmemcpy(void *dest, void *src, int numwords);

static unsigned int buffer[0x80];

int memoryAllocated = 0;
char *pcmbufl = NULL;
char *pcmbufr = NULL;
char *spubuf = NULL;

int readpos = 0;
int writepos = 0;

int volume = 0x3fff;

int transfer_sema = 0;
int play_tid = 0;

int intr_state;

int SyncFlag;

int _start ()
{
  iop_thread_t param;
  int th;

  FlushDcache();

  CpuEnableIntr(0);
  EnableIntr(36);	// Enables SPU DMA (channel 0) interrupt.
  EnableIntr(40);	// Enables SPU DMA (channel 1) interrupt.
  EnableIntr(9);	// Enables SPU IRQ interrupt.

  param.attr         = TH_C;
  param.thread       = SjPCM_Thread;
  param.priority     = 40;
  param.stacksize    = 0x800;
  param.option       = 0;
  th = CreateThread(&param);
  if (th > 0) {
  	StartThread(th,0);
	return 0;
  }
  else return 1;

}

void SjPCM_Thread(void* param)
{
  printf("SjPCM v2.0 - by Sjeep\n");

  printf("SjPCM: RPC Initialize\n");
  SifInitRpc(0);

  SifSetRpcQueue(&qd, GetThreadId());
  SifRegisterRpc(&sd0, SJPCM_IRX, (void *)SjPCM_rpc_server,(void *) &buffer[0],0,0,&qd);
  SifRpcLoop(&qd);
}

void* SjPCM_rpc_server(unsigned int fno, void *data, int size)
{

	switch(fno) {
		case SJPCM_INIT:
			return SjPCM_Init((unsigned*)data);
		case SJPCM_PUTS:
			return SjPCM_Puts((char*)data);
		case SJPCM_ENQUEUE:
			return SjPCM_Enqueue((unsigned*)data);
		case SJPCM_PLAY:
			return SjPCM_Play();
		case SJPCM_PAUSE:
			return SjPCM_Pause();
		case SJPCM_SETVOL:
			return SjPCM_Setvol((unsigned*)data);
		case SJPCM_CLEARBUFF:
			return SjPCM_Clearbuff();
		case SJPCM_QUIT:
			return SjPCM_Quit();
		case SJPCM_GETAVAIL:
			return SjPCM_Available((unsigned*)data);
		case SJPCM_GETBUFFD:
			return SjPCM_Buffered((unsigned*)data);
	}

	return NULL;
}

void* SjPCM_Clearbuff()
{
	CpuSuspendIntr(&intr_state);

	memset(spubuf,0,0x800);
	memset(pcmbufl,0,960*2*20);
	memset(pcmbufr,0,960*2*20);

	CpuResumeIntr(intr_state);
	
	return NULL;
}

void* SjPCM_Play()
{
	sceSdSetParam(SD_CORE_1|SD_P_BVOLL,volume);
	sceSdSetParam(SD_CORE_1|SD_P_BVOLR,volume);

	return NULL;
}

void* SjPCM_Pause()
{
	sceSdSetParam(SD_CORE_1|SD_P_BVOLL,0);
	sceSdSetParam(SD_CORE_1|SD_P_BVOLR,0);

	return NULL;
}

void* SjPCM_Setvol(unsigned int* sbuff)
{
	volume = sbuff[5];

	sceSdSetParam(SD_CORE_1|SD_P_BVOLL,volume);
	sceSdSetParam(SD_CORE_1|SD_P_BVOLR,volume);

	return NULL;
}

void* SjPCM_Puts(char* s)
{
	printf("SjPCM: %s",s);

	return NULL;
}

void* SjPCM_Init(unsigned int* sbuff)
{
	iop_sema_t sema;
	iop_thread_t play_thread;

	SyncFlag = sbuff[0];		

	sema.attr = 0; // XXX: SA_THFIFO;
	sema.initial = 0;
	sema.max = 1;
	transfer_sema= CreateSema(&sema);
	if(transfer_sema <= 0) {
		printf("SjPCM: Failed to create semaphore!\n");
		ExitDeleteThread();
	}

	// Allocate memory
	if(!memoryAllocated)
	{
		pcmbufl = AllocSysMemory(0,960*2*20,NULL);
		if(pcmbufl == NULL) {
			printf("SjPCM: Failed to allocate memory for sound buffer!\n");
			ExitDeleteThread();
		}
		pcmbufr = AllocSysMemory(0,960*2*20,NULL);
		if(pcmbufr == NULL) {
			printf("SjPCM: Failed to allocate memory for sound buffer!\n");
			ExitDeleteThread();
		}
		spubuf = AllocSysMemory(0,0x800,NULL);
		if(spubuf == NULL) {
			printf("SjPCM: Failed to allocate memory for sound buffer!\n");
			ExitDeleteThread();
		}

		printf("SjPCM: Memory Allocated. %d bytes left.\n",QueryTotalFreeMemSize());

		memoryAllocated = 1;
	}

	memset(pcmbufl,0,960*2*20);
	memset(pcmbufr,0,960*2*20);
	memset(spubuf,0,0x800);

	printf("SjPCM: Sound buffers cleared\n");

	// Initialise SPU
	if(sceSdInit(SD_INIT_COLD) < 0) {
		printf("SjPCM: Failed to initialise libsd!\n");
		ExitDeleteThread();
	}
	else printf("SjPCM: libsd initialised!\n");

	sceSdSetCoreAttr(SD_CORE_1|SD_C_NOISE_CLK,0);
	sceSdSetParam(SD_CORE_1|SD_P_MVOLL,0x3fff);
	sceSdSetParam(SD_CORE_1|SD_P_MVOLR,0x3fff);
	sceSdSetParam(SD_CORE_1|SD_P_BVOLL,volume);
	sceSdSetParam(SD_CORE_1|SD_P_BVOLR,volume);

	sceSdSetTransCallback(1, (void *)SjPCM_TransCallback);

	// Start audio streaming
	sceSdBlockTrans(1,SD_BLOCK_LOOP,spubuf, 0x800);

	printf("SjPCM: Setting up playing thread\n");

	// Start playing thread
	play_thread.attr         = TH_C;
  	play_thread.thread       = SjPCM_PlayThread;
  	play_thread.priority 	 = 39;
  	play_thread.stacksize    = 0x800;
  	play_thread.option       = 0;
  	play_tid = CreateThread(&play_thread);
	if (play_tid > 0) StartThread(play_tid,0);
	else {
		printf("SjPCM: Failed to start playing thread!\n");
		ExitDeleteThread();
	}

	// Return data
	sbuff[1] = (unsigned)pcmbufl;
	sbuff[2] = (unsigned)pcmbufr;
	sbuff[3] = writepos;

	printf("SjPCM: Entering playing thread.\n");

	return sbuff;
}

void SjPCM_PlayThread(void* param)
{
	int which;

	while(1) {

		WaitSema(transfer_sema);

		// Interrupts are suspended, instead of using semaphores.
		CpuSuspendIntr(&intr_state);

		which = 1 - (sceSdBlockTransStatus(1, 0 )>>24);

		wmemcpy(spubuf+(1024*which),pcmbufl+readpos,512);		// left
		wmemcpy(spubuf+(1024*which)+512,pcmbufr+readpos,512);	// right

		readpos += 512;
		if(readpos >= (960*2*20)) readpos = 0;

		CpuResumeIntr(intr_state);

	}
}

void* SjPCM_Enqueue(unsigned int* sbuff)
{
	writepos += sbuff[0]*2;
	if(writepos >= (960*2*20)) writepos = 0;

	if(SyncFlag)
		if(writepos == (960*2*10)) readpos = 0x2400;

	sbuff[3] = writepos;

	return sbuff;
}

static int SjPCM_TransCallback(void* param)
{
	iSignalSema(transfer_sema);

	return 1;
}

void* SjPCM_Available(unsigned int* sbuff)
{
  unsigned int rp = readpos, wp = writepos;
  if (wp<rp) wp+=960*2*20;
  sbuff[3] = (960*2*20-(wp-rp))/4;
  return sbuff;
}

void* SjPCM_Buffered(unsigned int* sbuff)
{
  unsigned int rp = readpos, wp = writepos;
  if (wp<rp) wp+=960*2*20;
  sbuff[3] = (wp-rp)/4;
  return sbuff;
}

void* SjPCM_Quit(unsigned int* sbuff)
{
	sceSdSetTransCallback(1,NULL);
	sceSdBlockTrans(1,SD_TRANS_MODE_STOP,0,0);

	TerminateThread(play_tid);
	DeleteThread(play_tid);

	DeleteSema(transfer_sema);

	return sbuff;

}
