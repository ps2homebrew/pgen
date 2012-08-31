#include "machine.h"
#include "gensound.h"
#include "pgen.h"

s16 ps2_soundbuf[2][960];

void up_samples(uint16 *sleft, uint16 *sright, uint16 *dleft, uint16 *dright, int dest_samples, int orig_samples) {

	int i;
	int count = 0;
	int ratio = dest_samples/orig_samples;

	for(i=0;i<orig_samples*ratio;i++) {

		dleft[i] = sleft[count];
		dright[i] = sright[count];

		if(!(i%ratio) && i) count++;

	}
}

extern "C" int soundp_start(void) {

	return 0;
}

extern "C" void soundp_stop(void) {

}

extern "C" void soundp_output(uint16 *left, uint16 *right, unsigned int samples) {

//	printf("samples = %d\n", samples);

	up_samples(left, right, (u16 *)ps2_soundbuf[0], (u16 *)ps2_soundbuf[1], sound_normalsamples, samples);
	SjPCM_Enqueue(ps2_soundbuf[0], ps2_soundbuf[1], sound_normalsamples, /*1*/ 0);

}
