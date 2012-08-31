/***************************************************************/
/*                                                             */
/* PSG.C : SN76489 emulator                                    */
/*                                                             */
/* Noise define constantes taken from MAME                     */
/*                                                             */
/* This source is a part of Gens project (gens@consolemul.com) */
/* Copyright (c) 2002 by Stéphane Dallongeville                */
/*                                                             */
/***************************************************************/

#include <math.h>
#include <string.h>
#include "psg.h"


/* Defines */

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Change MAX_OUTPUT to change PSG volume (default = 0x7FFF)

#define MAX_OUTPUT 0x5FFF

#define W_NOISE 0x12000
#define P_NOISE 0x08000
#define NOISE_DEF 0x0f35


/* Variables */

unsigned int PSG_SIN_Table[16][512];
unsigned int PSG_Step_Table[1024];
unsigned int PSG_Volume_Table[16];
unsigned int PSG_Noise_Step_Table[4];
unsigned int PSG_Save[8];

struct _psg PSG;


/* Functions */

void PSG_Write(int data)
{
	if (data & 0x80)
	{
		PSG.Current_Register = (data & 0x70) >> 4;
		PSG.Current_Channel = PSG.Current_Register >> 1;

		data &= 0x0F;
		
		PSG.Register[PSG.Current_Register] = (PSG.Register[PSG.Current_Register] & 0x3F0) | data;

		if (PSG.Current_Register & 1)
		{
			// Volume

			PSG.Volume[PSG.Current_Channel] = PSG_Volume_Table[data];
		}
		else
		{
			// Frequency

			if (PSG.Current_Channel != 3)
			{
				// Normal channel

				PSG.CntStep[PSG.Current_Channel] = PSG_Step_Table[PSG.Register[PSG.Current_Register]];

				if ((PSG.Current_Channel == 2) && ((PSG.Register[6] & 3) == 3))
				{
					PSG.CntStep[3] = PSG.CntStep[2] >> 1;
				}
			}
			else
			{
				// Noise channel

				PSG.Noise = NOISE_DEF;
				PSG_Noise_Step_Table[3] = PSG.CntStep[2] >> 1;
				PSG.CntStep[3] = PSG_Noise_Step_Table[data & 3];

				if (data & 4) PSG.Noise_Type = W_NOISE;
				else PSG.Noise_Type = P_NOISE;
			}
		}
	}
	else
	{
		if (!(PSG.Current_Register & 1))
		{
			// Frequency 

			if (PSG.Current_Channel != 3)
			{
				PSG.Register[PSG.Current_Register] = (PSG.Register[PSG.Current_Register] & 0x0F) | ((data & 0x3F) << 4);

				PSG.CntStep[PSG.Current_Channel] = PSG_Step_Table[PSG.Register[PSG.Current_Register]];

				if ((PSG.Current_Channel == 2) && ((PSG.Register[6] & 3) == 3))
				{
					PSG.CntStep[3] = PSG.CntStep[2] >> 1;
				}
			}
		}
	}
}


void PSG_Update_SIN(int **buffer, int length)
{
	int i, j, out;
	int cur_cnt, cur_step, cur_vol;
	unsigned int *sin_t;

	for(j = 2; j >= 0; j--)
	{
		if (PSG.Volume[j])
		{
			cur_cnt = PSG.Counter[j];
			cur_step = PSG.CntStep[j];
			sin_t = PSG_SIN_Table[PSG.Register[(j << 1) + 1]];

			for(i = 0; i < length; i++)
			{
				out = sin_t[(cur_cnt = (cur_cnt + cur_step) & 0x1FFFF) >> 8];

				buffer[0][i] += out;
				buffer[1][i] += out;
			}

			PSG.Counter[j] = cur_cnt;
		}
		else PSG.Counter[j] += PSG.CntStep[j] * length;
	}


	// Channel 3 - Noise

	if (cur_vol = PSG.Volume[3])
	{
		cur_cnt = PSG.Counter[3];
		cur_step = PSG.CntStep[3];

		for(i = 0; i < length; i++)
		{
			cur_cnt += cur_step;

			if (PSG.Noise & 1)
			{
				buffer[0][i] += cur_vol;
				buffer[1][i] += cur_vol;

				if (cur_cnt & 0x10000)
				{
					cur_cnt &= 0xFFFF;
					PSG.Noise = (PSG.Noise ^ PSG.Noise_Type) >> 1;
				}
			}
			else if (cur_cnt & 0x10000)
			{
				cur_cnt &= 0xFFFF;
				PSG.Noise >>= 1;
			}
		}

		PSG.Counter[3] = cur_cnt;
	}
	else PSG.Counter[3] += PSG.CntStep[3] * length;
}


void PSG_Update(int **buffer, int length)
{
	int i, j;
	int cur_cnt, cur_step, cur_vol;

	for(j = 2; j >= 0; j--)
	{
		if ((cur_vol = PSG.Volume[j]) && ((cur_step = PSG.CntStep[j]) < 0x10000))
		{
			cur_cnt = PSG.Counter[j];

			for(i = 0; i < length; i++)
			{
				if ((cur_cnt += cur_step) & 0x10000)
				{
					buffer[0][i] += cur_vol;
					buffer[1][i] += cur_vol;
				}
			}

			PSG.Counter[j] = cur_cnt;
		}
		else
		{
			PSG.Counter[j] += PSG.CntStep[j] * length;
		}
	}

	// Channel 3 - Noise

	if (cur_vol = PSG.Volume[3])
	{
		cur_cnt = PSG.Counter[3];
		cur_step = PSG.CntStep[3];

		for(i = 0; i < length; i++)
		{
			cur_cnt += cur_step;

			if (PSG.Noise & 1)
			{
				buffer[0][i] += cur_vol;
				buffer[1][i] += cur_vol;

				if (cur_cnt & 0x10000)
				{
					cur_cnt &= 0xFFFF;
					PSG.Noise = (PSG.Noise ^ PSG.Noise_Type) >> 1;
				}
			}
			else if (cur_cnt & 0x10000)
			{
				cur_cnt &= 0xFFFF;
				PSG.Noise >>= 1;
			}
		}

		PSG.Counter[3] = cur_cnt;
	}
	else PSG.Counter[3] += PSG.CntStep[3] * length;
}


void PSG_Init(int clock, int rate)
{
	int i, j;
	float out;

	for(i = 1; i < 1024; i++)
	{
		// Step calculation

		out = (float) (clock) / (float) (i << 4);		// out = frequency
		out /= (float) (rate);
		out *= 65536.0;

		PSG_Step_Table[i] = (unsigned int) out;
	}

	PSG_Step_Table[0] = PSG_Step_Table[1];

	for(i = 0; i < 3; i++)
	{
		out = (float) (clock) / (float) (1 << (9 + i));
		out /= (float) (rate);
		out *= 65536.0;

		PSG_Noise_Step_Table[i] = (unsigned int) out;
	}

	PSG_Noise_Step_Table[3] = 0;

	out = (float) MAX_OUTPUT / 3.0;

	for (i = 0; i < 15; i++)
	{
		PSG_Volume_Table[i] = (unsigned int) out;
		out /= 1.258925412;		// = 10 ^ (2/20) = 2dB
	}

	PSG_Volume_Table[15] = 0;

	for(i = 0; i < 512; i++)
	{
		out = sinf((2.0 * PI) * ((float) (i) / 512));

		for(j = 0; j < 16; j++)
		{
			PSG_SIN_Table[j][i] = (unsigned int) (out * (float) PSG_Volume_Table[j]);
		}
	}

	PSG.Current_Register = 0;
	PSG.Current_Channel = 0;
	PSG.Noise = 0;
	PSG.Noise_Type = 0;

	for (i = 0; i < 4; i++)
	{
		PSG.Volume[i] = 0;
		PSG.Counter[i] = 0;
		PSG.CntStep[i] = 0;
	}

	for (i = 0; i < 8; i += 2)
	{
		PSG_Save[i] = 0;
		PSG_Save[i + 1] = 0x0F;			// volume = OFF
	}

	PSG_Restore_State();				// Reset
}


void PSG_Save_State(void)
{
	int i;

	for(i = 0; i < 8; i++) PSG_Save[i] = PSG.Register[i];
}


void PSG_Restore_State(void)
{
	int i;

	for(i = 0; i < 8; i++)
	{
		PSG_Write(0x80 | (i << 4) | (PSG_Save[i] & 0xF));
		PSG_Write((PSG_Save[i] >> 4) & 0x3F);
	}
}
