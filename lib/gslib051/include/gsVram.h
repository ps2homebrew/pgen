/************************************
 *                                  *
 * gsVram module by ragnarok2040    *
 * provides functions for utilizing *
 * GS vram for textures and general *
 * usage                            *
 ***********************************/

#ifndef _GSVRAM_H_
#define _GSVRAM_H_

#include "gsDefs.h"
#include "gsDriver.h"

class gsVram
{
    public:

        gsVram(gsDriver& inDriver);

        // set pointer to location in vram
        void           setPointer(unsigned int inPointer);

        // get amount of vram available
        unsigned int   getAvailable();

        // get current vram used
        unsigned int   getCurrentPointer();

        // get total size of vram
        unsigned int   getTotal();

        // get pointer of unallocated vram space for a texture
        unsigned int   getTBP(int inTexWidth, int inTexHeight, int inPSM);

        // reset pointer to beginning of useable vram space
        void           Reset(gsDriver& inDriver);

    private:
	static const unsigned int kVramMax = 4194304;
        unsigned int m_vramPointer;
};

#endif /* _GSVRAM_H_ */


