#ifndef _GENSOUNDP_H
#define _GENSOUNDP_H

int soundp_start(void);
void soundp_stop(void);
int soundp_samplesbuffered(void);
int soundp_output(uint16 *left, uint16 *right, unsigned int samples);

#endif
