  int xsize, ysize;
  int x, scan, w, xstart;
  static int sizes[4] = {32, 64, 64, 128};
  unsigned which;
  unsigned char *where, *scrolls, *tiles, *tile_line;
  int xoff, yoff, xoff_mask;
  int xscroll, yscroll;

#if PLANE == 0
  // Plane 0 is only where the window isn't
  // This should make Herzog Zwei split screen work perfectly, and clean
  // up those little glitches on Sonic 3's level select.

  // vdp_reg[18] = Window V position

  if(vdp_reg[18] & 0x80) // Window goes down, plane 0 goes up! :)
    { // if DOWN = 1 (Window is from line specified to end of screen)
      if((line>>3) >= (vdp_reg[18]&0x1f)) return;
    } else { // Window goes up, plane 0 goes down (DOWN = 0 - Window is from line 0 to line specified)
      if((line>>3) < (vdp_reg[18]&0x1f)) return;
    }
#endif

  // get "xsize" value (number of horiz tiles)
  xsize = sizes[ vdp_reg[16]       & 3] << 1; // << 1 = * 2	
  // get "ysize" value
  ysize = sizes[(vdp_reg[16] >> 4) & 3];

#if PLANE == 0
  // get vertical scroll value
  yscroll = get_word(vdp_vsram);
  // scrolls = hscroll table address
  scrolls = vdp_vram + ((vdp_reg[13]<<10) & 0xfc00);
  // tiles = pattern name table address (A/B)
  tiles   = vdp_vram +  (vdp_reg[ 2]<<10);
#else // PLANE == 1
  yscroll = get_word(vdp_vsram+2);
  scrolls = vdp_vram + ((vdp_reg[13]<<10) & 0xfc00) + 2;
  tiles   = vdp_vram +  (vdp_reg[ 4]<<13);
#endif

  // Interlace?
  if(vdp_reg[12] & 2)
    yscroll >>= 1;

  // Offset for the line
  yscroll += line;

  // Wide or narrow?
  if(vdp_reg[12] & 1)
    {
      w = 40; xstart = -8;
    } else {
      w = 32; xstart = 24;
    }

  // How do we horizontal offset?
  switch(vdp_reg[11] & 3)
    {
    case 2: // per tile
      scrolls += (line & ~7) << 2;	// AND out line bits. << 2 (* 4) to skip 2 entries each line, 
									// since the entries for scroll A & B are interlaced
      break;
    case 3: // per line
      scrolls += line << 2;
      break;
    default:
      break;
    }

  // xcroll = the actual scroll value for this line
  xscroll = get_word(scrolls);

  // xoff_mask to wrap x value once its outside the screen
  xoff_mask = xsize - 1;

  yoff = (yscroll>>3) & (ysize - 1);	// yoff = y offset in TILES (not pixels)
  xoff = ((-(xscroll>>3) - 1)<<1) & xoff_mask; // wtf?
  tile_line = tiles + xsize * yoff;		// offset into vram for start line (nearest TILE)
										// xsize = width in tiles * 2 (since each nametable entry is a word)
  scan = yscroll & 7;					// the line within the pattern which is to be drawn

  where = dest + (xstart + (xscroll & 7)) * Bpp;	// 

  for(x = -1; x < w; ++x) // in units of tiles
    {
#if PLANE == 0
      if(vdp_reg[17] & 0x80) // Don't draw where the window will be
	{
	  if(x >= ((vdp_reg[17]&0x1f) << 1)) goto skip;
	} else {
	  // + 1 so scroll layers in Sonic look right
	  if((x + 1) < ((vdp_reg[17]&0x1f) << 1)) goto skip;
	}
#endif
      which = get_word(tile_line + xoff);

#if (FRONT == 0) && (PLANE == 1)
      draw_tile_solid(which, scan, where);
#elif FRONT == 1
      if(which >> 15) draw_tile(which, scan, where);
#else
      if(!(which >> 15)) draw_tile(which, scan, where);
#endif

#if PLANE == 0
skip:
#endif
      where += Bpp_times8;
      xoff = (xoff + 2) & xoff_mask;
    }
