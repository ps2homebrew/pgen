#include "pgen.h"
#include "vdp.h"

//
// NEW RENDERING CODE
//

unsigned char sprite_order[0x101], *sprite_base;
unsigned sprite_count, Bpp, Bpp_times8;
unsigned char *dest;

// Macros, to route draw_tile and draw_tile_solid to the right handler
#define draw_tile(which, line, where) \
	    draw_tile1((which),(line),(where));


#define draw_tile_solid(which, line, where) \
	    draw_tile1_solid((which),(line),(where));

static inline int get_word(unsigned char *where)
  { return (where[0] << 8) | where[1]; }

#  define PIXEL0 (0x000000f0)
#  define PIXEL1 (0x0000000f)
#  define PIXEL2 (0x0000f000)
#  define PIXEL3 (0x00000f00)
#  define PIXEL4 (0x00f00000)
#  define PIXEL5 (0x000f0000)
#  define PIXEL6 (0xf0000000)
#  define PIXEL7 (0x0f000000)
#  define SHIFT0 ( 4)
#  define SHIFT1 ( 0)
#  define SHIFT2 (12)
#  define SHIFT3 ( 8)
#  define SHIFT4 (20)
#  define SHIFT5 (16)
#  define SHIFT6 (28)
#  define SHIFT7 (24)

// Blit tile solidly, for 1 byte-per-pixel
inline void draw_tile1_solid(int which, int line, unsigned char *where)
{
  unsigned tile, pal;

  pal = (which >> 9 & 0x30); // Determine which 16-color palette

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(vdp_reg[12] & 2) // interlace
  // vram + tile_addr + line * 8 (skips odd lines for interlace)
    tile = *(unsigned*)(vdp_vram + ((which&0x7ff) << 6) + (line << 3));
  else
  // vram + tile_addr + line * 4 (4 bytes per line)
    tile = *(unsigned*)(vdp_vram + ((which&0x7ff) << 5) + (line << 2));

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      *(where  ) = ((tile & PIXEL7)>>SHIFT7) | pal;
      *(where+1) = ((tile & PIXEL6)>>SHIFT6) | pal;
      *(where+2) = ((tile & PIXEL5)>>SHIFT5) | pal;
      *(where+3) = ((tile & PIXEL4)>>SHIFT4) | pal;
      *(where+4) = ((tile & PIXEL3)>>SHIFT3) | pal;
      *(where+5) = ((tile & PIXEL2)>>SHIFT2) | pal;
      *(where+6) = ((tile & PIXEL1)>>SHIFT1) | pal;
      *(where+7) = ((tile & PIXEL0)>>SHIFT0) | pal;
    } else {
      *(where  ) = ((tile & PIXEL0)>>SHIFT0) | pal;
      *(where+1) = ((tile & PIXEL1)>>SHIFT1) | pal;
      *(where+2) = ((tile & PIXEL2)>>SHIFT2) | pal;
      *(where+3) = ((tile & PIXEL3)>>SHIFT3) | pal;
      *(where+4) = ((tile & PIXEL4)>>SHIFT4) | pal;
      *(where+5) = ((tile & PIXEL5)>>SHIFT5) | pal;
      *(where+6) = ((tile & PIXEL6)>>SHIFT6) | pal;
      *(where+7) = ((tile & PIXEL7)>>SHIFT7) | pal;
    }
}

// Blit tile, leaving color zero transparent, for 1 byte per pixel
inline void draw_tile1(int which, int line, unsigned char *where)
{
  unsigned tile, pal;

  pal = (which >> 9 & 0x30); // Determine which 16-color palette

  if(which & 0x1000) // y flipped
    line ^= 7; // take from the bottom, instead of the top

  if(vdp_reg[12] & 2) // interlace
    tile = *(unsigned*)(vdp_vram + ((which&0x7ff) << 6) + (line << 3));
  else
    tile = *(unsigned*)(vdp_vram + ((which&0x7ff) << 5) + (line << 2));
  // If the tile is all 0's, why waste the time?
  if(!tile) return;

  // Blit the tile!
  if(which & 0x800) // x flipped
    {
      if(tile & PIXEL7) *(where  ) = ((tile & PIXEL7)>>SHIFT7) | pal;
      if(tile & PIXEL6) *(where+1) = ((tile & PIXEL6)>>SHIFT6) | pal;
      if(tile & PIXEL5) *(where+2) = ((tile & PIXEL5)>>SHIFT5) | pal;
      if(tile & PIXEL4) *(where+3) = ((tile & PIXEL4)>>SHIFT4) | pal;
      if(tile & PIXEL3) *(where+4) = ((tile & PIXEL3)>>SHIFT3) | pal;
      if(tile & PIXEL2) *(where+5) = ((tile & PIXEL2)>>SHIFT2) | pal;
      if(tile & PIXEL1) *(where+6) = ((tile & PIXEL1)>>SHIFT1) | pal;
      if(tile & PIXEL0) *(where+7) = ((tile & PIXEL0)>>SHIFT0) | pal;
    } else {
      if(tile & PIXEL0) *(where  ) = ((tile & PIXEL0)>>SHIFT0) | pal;
      if(tile & PIXEL1) *(where+1) = ((tile & PIXEL1)>>SHIFT1) | pal;
      if(tile & PIXEL2) *(where+2) = ((tile & PIXEL2)>>SHIFT2) | pal;
      if(tile & PIXEL3) *(where+3) = ((tile & PIXEL3)>>SHIFT3) | pal;
      if(tile & PIXEL4) *(where+4) = ((tile & PIXEL4)>>SHIFT4) | pal;
      if(tile & PIXEL5) *(where+5) = ((tile & PIXEL5)>>SHIFT5) | pal;
      if(tile & PIXEL6) *(where+6) = ((tile & PIXEL6)>>SHIFT6) | pal;
      if(tile & PIXEL7) *(where+7) = ((tile & PIXEL7)>>SHIFT7) | pal;
    }
}

// Draw the window (front or back)
void draw_window(int line, int front)
{
  int size;
  int x, y, w, start;
  int pl, add;
  int total_window;
  unsigned char *where;
  int which;
  // Set everything up
  y = line >> 3;
  total_window = (y < (vdp_reg[18]&0x1f)) ^ (vdp_reg[18] >> 7);

  // Wide or narrow
  size = (vdp_reg[12] & 1)? 64 : 32;

  pl = (vdp_reg[3] << 10) + ((y&0x3f)*size*2);

  // Wide(320) or narrow(256)?
  if(vdp_reg[12] & 1)
    {
      w = 40;
      start = -8;
    } else {
      w = 32;
      start = 24;
    }
  add = -2;
  where = dest + (start * Bpp);
  for(x=-1; x<w; ++x)
    {
      if(!total_window)
		{
			if(vdp_reg[17] & 0x80)
	  		{
	    		if(x < ((vdp_reg[17]&0x1f) << 1)) goto skip;
	  		} else {
	    		if(x >= ((vdp_reg[17]&0x1f) << 1)) goto skip;
	  		}
		}
      which = get_word(((unsigned char*)vdp_vram) + (pl+(add&((size-1)<<1))));
      if((which>>15) == front)
	  draw_tile(which, line&7, where);
skip:
      add += 2; where += Bpp_times8;
    }
}

void draw_sprites(int line, int front)
{
  unsigned which;
  int tx, ty, x, y, xend, ysize, yoff, i;
  unsigned char *where, *sprite;
  // Sprites have to be in reverse order :P
  for(i = sprite_count - 1; i >= 0; --i)
    {
      sprite = sprite_base + (sprite_order[i] << 3);
      // Get the first tile
      which = get_word(sprite + 4);
      // Only do it if it's on the right priority
      if((which >> 15) == (unsigned int)front)
	{
	  // Get the sprite's location
	  y = get_word(sprite);
	  x = get_word(sprite + 6) & 0x1ff;

	  // Interlace?
	  if(vdp_reg[12] & 2)
	    y = (y & 0x3fe) >> 1;
	  else
	    y &= 0x1ff;

	  x -= 0x80;
	  y -= 0x80;
	  yoff = (line - y);

	  // Narrow mode?
	  if(!(vdp_reg[12] & 1)) x += 32;

	  xend = ((sprite[2] << 1) & 0x18) + x;
	  ysize = sprite[2] & 0x3;

	  // Render if this sprite's on this line
	  if(xend > -8 && x < 320 && yoff >= 0 && yoff <= (ysize<<3)+7)
	    {
	      ty = yoff & 7;
	      // y flipped?
	      if(which & 0x1000)
		which += ysize - (yoff >> 3);
	      else
		which += (yoff >> 3);
	      ++ysize;
	      // x flipped?
	      if(which & 0x800)
		{
		  where = dest + (xend * Bpp);
		  for(tx = xend; tx >= x; tx -= 8)
		    {
		      if(tx > -8 && tx < 320)
			draw_tile(which, ty, where);
		      which += ysize;
		      where -= Bpp_times8;
		    }
	        } else {
		  where = dest + (x * Bpp);
		  for(tx = x; tx <= xend; tx += 8)
		    {
		      if(tx > -8 && tx < 320)
			draw_tile(which, ty, where);
		      which += ysize;
		      where += Bpp_times8;
		    }
		}
	    }
	}
    }
}

// The body for the next few functions is in an extraneous header file.
// Phil, I hope I left enough in this file for GLOBAL to hack it right. ;)
// Thanks to John Stiles for this trick :)

void draw_plane_back0(int line)
{
#define FRONT 0
#define PLANE 0
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

void draw_plane_back1(int line)
{
#define FRONT 0
#define PLANE 1
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

void draw_plane_front0(int line)
{
#define FRONT 1
#define PLANE 0
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

void draw_plane_front1(int line)
{
#define FRONT 1
#define PLANE 1
#include "ras-drawplane.h"
#undef PLANE
#undef FRONT
}

// The main interface function, to generate a scanline
void draw_scanline(char* bitmap, int pitch, int line)
{
  unsigned i;
  unsigned next = 0;
  // Set the destination in the bmap
  dest = (unsigned char *)bitmap + (pitch * (line + 8) + 16);

  // If bytes per pixel hasn't yet been set, do it
  // HACK - FIXED 8-BIT
  if(!Bpp)
    {
    Bpp = 1;
    Bpp_times8 = Bpp << 3; // used for tile blitting
    }

  // Render the screen if it's turned on
  if(vdp_reg[1] & 0x40)
    {
/*
	// fill in background
  	background = vdp_reg[7] & 63;
	for(i=0;i<320;i++) dest[i] = background;
*/
	  // Find the sprite base in VRAM
	  sprite_base = vdp_vram + (vdp_reg[5]<<9);
	  // Order the sprites
	  sprite_count = sprite_order[0] = 0;
	  do {
	    next = sprite_base[(next << 3) + 3];
	    sprite_order[++sprite_count] = next;
	  // No more than 256 sprites/line, a reasonable limit ;)
	  } while (next && sprite_count < 0x100);
	  // Clean up the dirt
	  //dirt[0x30] &= ~0x20; dirt[0x34] &= ~1;

      // Draw, from the bottom up
      // Low priority
      draw_plane_back1(line);
      draw_plane_back0(line);
      draw_window(line, 0);
      draw_sprites(line, 0);
      // High priority
      draw_plane_front1(line);
      draw_plane_front0(line);
      draw_window(line, 1);
      draw_sprites(line, 1);
    } else {
      // The display is off, paint it black
      // Do it a dword at a time
      unsigned *destl = (unsigned*)dest;
      for(i = 0; i < (80 * Bpp); ++i) destl[i] = 0;
    }
}

