/*****************************
*                            *
*      GSLIB by Hiryu        *
*                            *
* gsFont module:             *
* Screen Font print function *
* using gsLib's gsPipe       *
*                            *
*****************************/

#ifndef _GSFONT_H_
#define _GSFONT_H_

#include "gsDefs.h"
#include "gsPipe.h"

enum gsFontAlign {
	GSFONT_ALIGN_LEFT = 1,
	GSFONT_ALIGN_CENTRE,
	GSFONT_ALIGN_RIGHT,
};


// all bitmap characters should be left-aligned in PixelData (if kerning at least)

typedef struct{
	
	// overall texture size is in pixels, since there is no reason
	// why overall texture size has to be in GS_TEX units (ie: a power of 2)
	char ID[4];	//"BFNT"

	unsigned int TexWidth;	// overall width of texture - in pixels
	unsigned int TexHeight;	// overall width of texture - in pixels

	unsigned int PSM;	// Colour Depth of font Texture (in GS_PSM)

	unsigned int NumXChars;	// number of characters horizontally in grid
	unsigned int NumYChars;	// number of characters vertically in grid

	unsigned int CharGridWidth;	// width of one char grid - in pixels 
	unsigned int CharGridHeight; // height of one char grid - in pixels

	// 256 byte entry for width of individual characters
	char CharWidth[256];

	// actual bitmap pixel data
	char PixelData;
} gsFontTex;



class gsFont
{
public:
	gsFont(gsPipe* fontPipe = (gsPipe*)NULL) { m_pFontPipe = fontPipe;};

	// Assign a pipe to the font class
	void assignPipe(gsPipe* fontPipe) { m_pFontPipe = fontPipe;};

	void uploadFont(gsFontTex* pFontTex, unsigned int TBbase, int TBwidth, int TBxpos, int TBypos );

	void Print(int x, int Xend, int y, int z, unsigned long colour, gsFontAlign alignment, const char* string);


private:

	// Get the length of the current line (upto max_length, or \n, or end of string)
	void GetCurrLineLength(const char* string, int max_length, int& pix_length, int& char_length);

	// Print just the current line
	void PrintLine(int x, int y, int z, unsigned long colour, int length, const char* string);

	gsPipe* m_pFontPipe;

	unsigned int m_TBbase;
	int m_TBwidth;
	int m_TBxpos;
	int m_TBypos;

	unsigned int m_TexWidth;	// overall width of texture - in pixels (from gsFontTex)
	unsigned int m_TexHeight;	// overall width of texture - in pixels (from gsFontTex)

	unsigned int m_NumXChars;	// number of characters horizontally in grid
	unsigned int m_NumYChars;	// number of characters vertically in grid

	unsigned int m_CharGridWidth;	// width of one char - in GS units (from gsFontTex)
	unsigned int m_CharGridHeight;// height of one char - in GS units (from gsFontTex)

	unsigned int m_PSM;	// Colour Depth of font Texture (GS_PSM) (from gsFontTex)

	bool m_Bold;
	bool m_Underline;

	char m_CharWidth[256];
};

#endif //_GSFONT_H_

