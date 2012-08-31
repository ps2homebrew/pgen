#ifndef _PSG_H
#define _PSG_H

#ifdef __cplusplus
extern "C" {
#endif


extern unsigned int PSG_Save[8];

struct _psg
{
	int Current_Channel;
	int Current_Register;
	int Register[8];
	unsigned int Counter[4];
	unsigned int CntStep[4];
	int Volume[4];
	unsigned int Noise_Type;
	unsigned int Noise;
};

extern struct _psg PSG;


void PSG_Write(int data);
void PSG_Update_SIN(int **buffer, int length);
void PSG_Update(int **buffer, int length);
void PSG_Init(int clock, int rate);
void PSG_Save_State(void);
void PSG_Restore_State(void);


#ifdef __cplusplus
};
#endif

#endif
