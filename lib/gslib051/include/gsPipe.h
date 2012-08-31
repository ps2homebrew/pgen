/*******************************
*                              *
*       GSLIB by Hiryu         *
*                              *
* gsPipe module:               *
* Based on GFX-Pipe by Vzzrzzn *
* (and modifications by Sjeep) *
*                              *
*******************************/

#ifndef _GSPIPE_H_
#define _GSPIPE_H_

#include "gsDefs.h"

class gsPipe
{
// Allow gsDriver to access the protected member functions
// primarily setZBuffer, and setDrawFrame
friend class gsDriver;

public:

	gsPipe(unsigned int size=0x20000); // buffer size must be less than 1MB
	~gsPipe();

	// Copy constructor
	gsPipe(const gsPipe& copyPipe);

	// assignment operator
	gsPipe& operator = (const gsPipe& copyPipe);

	
	// ReInit this pipe (re-setup alpha/zbuffer states etc) after using a different gsPipe
	void ReInit(void);

	unsigned int getPipeSize(void);

	/***********************
	* Pipe Flush Functions *
	***********************/

	void FlushCheck(void);
	void Flush(void);
	void FlushInt(void);

	/***********************************
	* Texture Upload & Setup Functions *
	***********************************/


	// send a byte-packed texture from RDRAM to VRAM
	// TBP = VRAM_address
	// TBW = buffer_width_in_pixels  -- dependent on pxlfmt
	// xofs, yofs in units of pixels
	// pxlfmt = 0x00 (32-bit), 0x02 (16-bit), 0x13 (8-bit), 0x14 (4-bit)
	// wpxls, hpxls = width, height in units of pixels

	void TextureUpload(unsigned int TBP, int TBW, int xofs, int yofs, int pxlfmt, const unsigned char* tex, int wpxls, int hpxls);

	// send a byte-packed texture from VRAM to RDRAM
	// TBP = VRAM_address
	// TBW = buffer_width_in_pixels  -- dependent on pxlfmt
	// xofs, yofs in units of pixels
	// pxlfmt = 0x00 (32-bit), 0x02 (16-bit), 0x13 (8-bit), 0x14 (4-bit)
	// wpxls, hpxls = width, height in units of pixels

	void TextureDownload(unsigned int TBP, int TBW, int xofs, int yofs, int pxlfmt, unsigned char* tex, int wpxls, int hpxls);

	void TextureSet(unsigned int tbp, int tbw, enum gsTexSize texwidth, enum gsTexSize texheight, u32 tpsm, u32 cbp, u32 csm, u32 cbw, u32 cpsm);
	void TextureFlush(void);
	void setFilterMethod(int FilterMethod);
	

	/***********************************
	* ZBuffer Enable/Disable Functions *
	***********************************/

	void setZTestEnable(int enable);
	void setAlphaEnable(int enable);

	void setScissorRect(long x1, long y1, long x2, long y2);
	void setOrigin(int x, int y);

	void setDither(unsigned long enable);
	void setColClamp(unsigned long enable);
	void setPrModeCont(unsigned long enable);


	/******************************************
	* Here are all the Point & line functions *
	******************************************/

	void Point(
		int x, int y,
		unsigned z, unsigned colour);

	void Line(
		int x1, int y1,
		int x2, int y2,
		unsigned z, unsigned colour);

	/**************************************
	* Here are all the triangle functions *
	**************************************/

	void TriangleLine(
		int x1, int y1, unsigned z1, unsigned c1,
		int x2, int y2, unsigned z2, unsigned c2,
		int x3, int y3, unsigned z3, unsigned c3);

	void TriangleFlat(
		int x1, int y1, unsigned z1,
		int x2, int y2, unsigned z2,
		int x3, int y3, unsigned z3,
		unsigned colour);

	void TriangleGouraud(
		int x1, int y1, unsigned z1, unsigned c1,
		int x2, int y2, unsigned z2, unsigned c2,
		int x3, int y3, unsigned z3, unsigned c3);


	void TriangleTexture(
		int x1, int y1, unsigned z1, unsigned u1, unsigned v1,
		int x2, int y2, unsigned z2, unsigned u2, unsigned v2,
		int x3, int y3, unsigned z3, unsigned u3, unsigned v3,
		unsigned colour);

	/***************************************
	* Here are all the rectangle functions *
	***************************************/

	void RectFlat(
		int x1, int y1,
		int x2, int y2,
		unsigned z, unsigned colour);

	void RectLine(
		int x1, int y1,
		int x2, int y2,
		unsigned z, unsigned colour);

	void RectTexture(
		int x1, int y1, u32 u1, u32 v1,
		int x2, int y2, u32 u2, u32 v2,
		u32 z, u32 colour);


	void RectGouraud(
		int x1, int y1, unsigned c1,
		int x2, int y2, unsigned c2,
		unsigned z);


	/**************************************
	* Here are all the TriStrip functions *
	**************************************/

	void TriStripGouraud(
		int x1, int y1, unsigned z1, unsigned c1,
		int x2, int y2, unsigned z2, unsigned c2,
		int x3, int y3, unsigned z3, unsigned c3,
		int x4, int y4, unsigned z4, unsigned c4);

	void TriStripGouraudTexture(
		int x1, int y1, unsigned z1, unsigned u1, unsigned v1, unsigned c1,
		int x2, int y2, unsigned z2, unsigned u2, unsigned v2, unsigned c2,
		int x3, int y3, unsigned z3, unsigned u3, unsigned v3, unsigned c3,
		int x4, int y4, unsigned z4, unsigned u4, unsigned v4, unsigned c4);


protected:
	// These 2 member functions should only be accessed by the gsDriver class
	void setZBuffer(unsigned long base, int psm, unsigned long mask);
	void setDrawFrame(unsigned long base, unsigned long width , int psm, unsigned long mask);

private:
	unsigned int getBytesLeft(void);
	void InitPipe(unsigned long *dmatadr);


    unsigned long* m_DmaPipe1;    // 'pipe 1' ... base of allotted pipeline memory
    unsigned long* m_DmaPipe2;    // 'pipe 2' ... DmaPipe1 + (MemSize / 2)
    unsigned int m_MemSize;       // # of bytes allotted to the pipelines (total)

    unsigned long* m_CurrentPipe;     // pointer to current 'pipe' .. may only be equal to
                                // either DmaPipe1 or DmaPipe2
    unsigned long* m_CurrentDmaAddr;  // pointer to the the dma block currently being added to
    unsigned long* m_CurrentGifTag;   // pointer to current "block" we can add prims to

	unsigned long* m_Buffer;

	/*************************************************************************
	* Local state variables                                                  *
	* Used for ReInit()'ing our gsPipe, to reset hardware to preffered state *
	*************************************************************************/

	// Even if the hardware alpha is enabled, we wont set the PABE bit in the prim reg
	// if our local gsPipe Alpha state is disabled
	int m_AlphaEnabled;

	int m_ZBufferEnabled;
	int m_ZTestEnabled;

	int m_OriginX;	// X drawing origin
	int m_OriginY;	// Y drawing origin

	int m_FilterMethod;

	/*******************************************************************************
	* Here are the static global state variables, which reflect the hardware state *
	* These are accessible by all gsPipe's, and can be used to update the hardware *
	* to match the individual gsPipe's state if the two dont match                 *
	*******************************************************************************/
/*
	int hw_AlphaEnabled;
	int hw_ZBufferEnabled;
	int hw_ZTestEnabled;

	int hw_OriginX;
	int hw_OriginY;
*/
};


#endif
