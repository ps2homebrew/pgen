/****************************************
*                                       *
*            GSLIB by Hiryu             *
*                                       *
* gsDriver module:                      *
* Provides functions for initialisation *
* and configuration of the PS2's GS,    *
* aswell as support for GS management   *
* and multiple frame-buffers.           *
*                                       *
****************************************/

#ifndef _GSDRIVER_H_
#define _GSDRIVER_H_

#include "gsDefs.h"
#include "gsPipe.h"

class gsDriver
{
public:
	// Default constructor, should setup default display environment
	gsDriver();
	~gsDriver();

	// Setup Display for specified size, mode, and number of buffers
	void setDisplayMode(unsigned int width, unsigned int height,
		unsigned int xpos, unsigned int ypos,
		unsigned int psm, unsigned int num_bufs,
		unsigned int TVmode, unsigned int TVinterlace,
		unsigned int zbuffer, unsigned int zpsm);

	void clearScreen(void);

	void setDisplayPosition(unsigned int xpos, unsigned int ypos);

	// Get the location of the frame buffer or texture buffer
	unsigned int getFrameBufferBase(unsigned int fb_num);
	unsigned int getTextureBufferBase(void);

	// Get the number of the current display or draw buffers
	unsigned int getCurrentDisplayBuffer(void);
	unsigned int getCurrentDrawBuffer(void);

	static unsigned int getBytesPerPixel(unsigned int psm)
	{
		switch (psm)
		{
		case GS_PSMCT32:
			return 4;

		case GS_PSMCT24:
		case GS_PSGPU24:
			return 3;

		case GS_PSMCT16:
		case GS_PSMCT16S:
			return 2;

		default:
			return 1;
		}
	}

	static gsTexSize getTexSizeFromInt(int texsize)
	{
		// Get the size in 2^X of texsize
		int pow2 = 0x0400; // exact power of 2
		int i;

		// special case for 0
		if (texsize == 0)
			return (gsTexSize)0;

		for (i=10; i>=0; i--)
		{
			// If texsize = exact 2^X
			if (texsize == pow2)
			{
				return (gsTexSize)i;
			}

			// check if texsize is bigger than the next lower power of 2
			// (also handles the case of a texsize > 1024)
			if (texsize > (pow2>>1))
			{
				return (gsTexSize)(i);
			}

			pow2 = pow2 >> 1;
		}

		return (gsTexSize)0;
	}

	static void WaitForVSync(void)
	{
		GS_CSR &= 8; // generate
		while(!(GS_CSR & 8)); // wait until its generated
	}


	//VSync Interrupt Handler Routines 
	unsigned int AddVSyncCallback(void (*func_ptr)());
	void RemoveVSyncCallback(unsigned int RemoveID);
	void EnableVSyncCallbacks(void);
	void DisableVSyncCallbacks(void);


	// Only to be used for traditional double-buffer display
	// (Use other funcs for multi-buffer displays)
	void swapBuffers(void);


	/************************
	* Multiple-Buffer funcs *
	************************/


	// Check if there is free buffer for Drawing to
	bool isDrawBufferAvailable();

	// Check if there is a completed buffer available for displaying
	bool isDisplayBufferAvailable();

	// Set the draw environment to the next free buffer
	// If no free buffer is available to draw then do nothing
	// (continuing drawing will draw over existing frame, so
	// to avoid this poll isDrawBufferAvailable until TRUE)
	void setNextDrawBuffer();

	// Call this function when you have finished drawing to a frame
	// to indicate that it is available for display
	void DrawBufferComplete();	
	
	// Call this on vsync to display the next frame (if complete)
	// and free-up another buffer for drawing.
	// no need to call DisplayBufferFree() or setNextDisplayBuffer()
	void DisplayNextFrame();
	
	// The default gsPipe used by the gsDriver for init, and available for users
	gsPipe drawPipe;

private:

	// Set the display to display the next complete drawn buffer
	// If no complete drawn buffer is available the do nothing
	//void setNextDisplayBuffer();		

	// Call this function after setNextDisplayBuffer()
	// to indicate that another buffer is available for drawing
	//void DisplayBufferFree();

	// Set the display or draw buffer
	// Note: These should not be used by the end user
	// (used for internal functionality only)
	void setDisplayBuffer(unsigned int buf_num);
	void setDrawBuffer(unsigned int buf_num);



	unsigned int m_FrameWidth;			// Width of frame buffers
	unsigned int m_FrameHeight;			// Height of frame buffers
	unsigned int m_FrameXpos;			// X positiion of frame
	unsigned int m_FrameYpos;			// Y positiion of frame
	unsigned int m_FramePSM;			// PSM of frame buffers
	unsigned int m_ZBuffer;				// ZBuffer used ? 0:1
	unsigned int m_ZBufferPSM;			// PSM of ZBuffer

	unsigned int m_CurrentDisplayBuffer;// Current Display Buffer
	unsigned int m_CurrentDrawBuffer;	// Current Draw Buffer

	unsigned int m_NumFrameBuffers;			// Number of frame buffers
//	unsigned int m_NextFreeDrawBuffer;		// The next available frame to draw to
//	unsigned int m_NextCompleteDrawBuffer;	// The next available frame for displaying
	unsigned int m_FreeBuffersAvailable;	// Number of buffers available for drawing to
	unsigned int m_CompleteBuffersAvailable;// Number of buffers available for displaying


	unsigned int m_FrameSize;			// Pre-calculated size of one frame buffer
	unsigned int m_ZBufferBase;			// Base of ZBuffer GS Mem (0 if no zbuffer allocated)
	unsigned int m_ZBufferSize;			// Size of ZBuffer (0 if no zbuffer allocated)
	unsigned int m_TextureBufferBase;	// Base of Texture Memory

	//struct gsDrawEnv m_DrawEnv __attribute__((aligned(16)));
};

#endif /* _HW1_H_ */
