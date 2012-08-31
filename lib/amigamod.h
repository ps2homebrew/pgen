// amigamod.h

#ifndef _AMIGAMOD_H_
#define _AMIGAMOD_H_

#define VZMOD       0##x##Bad##Ca11

#define MOD_INIT    0x0010
#define MOD_LOAD    0x0020
#define MOD_PLAY    0x0080
#define MOD_PAUSE   0x0090
#define MOD_SETVOL  0x0110
#define MOD_GETINFO 0x0210
#define MOD_QUIT    0x0810
#define MOD_PUTS    0x0900


typedef struct {
    int curorder;
    int currow;
    int numchannels;
    int bpm;
} ModInfoStruct;



#ifdef __cplusplus
extern "C" {
#endif

int amigaModInit(int nosdinit);
int amigaModLoad( void *moddata, int size );
int amigaModPlay(unsigned linear);
int amigaModPause();
int amigaModSetVolume( unsigned short volume);
int amigaModGetInfo( ModInfoStruct *info);
int amigaModQuit();
void ModPuts(char *s);



#ifdef __cplusplus
}
#endif


#endif

