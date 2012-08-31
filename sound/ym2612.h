#ifndef _YM2612_H_
#define _YM2612_H_

#ifdef __cplusplus
extern "C" {
#endif

// Change it if you need to do long update
#define	MAX_UPDATE_LENGTH   2000

// Gens always uses 16 bits sound (in 32 bits buffer) and do the convertion later if needed.
#define OUTPUT_BITS         16

// VC++ inline
#ifndef INLINE
#define INLINE              __inline
#endif

int YM2612_Init(int clock, int rate, int interpolation);
int YM2612_Reset(void);
void YM2612_Update(int **buf, int length);
int YM2612_Write(unsigned char adr, unsigned char data);
int YM2612_Read(void);
int YM2612_Save(unsigned char SAVE[0x200]);
int YM2612_Restore(unsigned char SAVE[0x200]);
int YM2612_End(void);
/* Gens */

void YM2612_DacAndTimers_Update(int **buffer, int length);
void YM2612_Special_Update(void);

/* end */

// used for foward...
void Update_Chan_Algo0(channel_ *CH, int **buf, int length);
void Update_Chan_Algo1(channel_ *CH, int **buf, int length);
void Update_Chan_Algo2(channel_ *CH, int **buf, int length);
void Update_Chan_Algo3(channel_ *CH, int **buf, int length);
void Update_Chan_Algo4(channel_ *CH, int **buf, int length);
void Update_Chan_Algo5(channel_ *CH, int **buf, int length);
void Update_Chan_Algo6(channel_ *CH, int **buf, int length);
void Update_Chan_Algo7(channel_ *CH, int **buf, int length);

void Update_Chan_Algo0_LFO(channel_ *CH, int **buf, int length);
void Update_Chan_Algo1_LFO(channel_ *CH, int **buf, int length);
void Update_Chan_Algo2_LFO(channel_ *CH, int **buf, int length);
void Update_Chan_Algo3_LFO(channel_ *CH, int **buf, int length);
void Update_Chan_Algo4_LFO(channel_ *CH, int **buf, int length);
void Update_Chan_Algo5_LFO(channel_ *CH, int **buf, int length);
void Update_Chan_Algo6_LFO(channel_ *CH, int **buf, int length);
void Update_Chan_Algo7_LFO(channel_ *CH, int **buf, int length);

void Update_Chan_Algo0_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo1_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo2_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo3_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo4_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo5_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo6_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo7_Int(channel_ *CH, int **buf, int length);

void Update_Chan_Algo0_LFO_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo1_LFO_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo2_LFO_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo3_LFO_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo4_LFO_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo5_LFO_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo6_LFO_Int(channel_ *CH, int **buf, int length);
void Update_Chan_Algo7_LFO_Int(channel_ *CH, int **buf, int length);

// used for foward...
void Env_Attack_Next(slot_ *SL);
void Env_Decay_Next(slot_ *SL);
void Env_Substain_Next(slot_ *SL);
void Env_Release_Next(slot_ *SL);
void Env_NULL_Next(slot_ *SL);

#ifdef __cplusplus
};
#endif

#endif
extern int YM2612_Enable;
extern int YM2612_Improv;
extern int DAC_Enable;
extern int *YM_Buf[2];
extern int YM_Len;

/* end */