extern uint32 uiplot_palcache[192];

void uiplot_setshifts(int redshift, int greenshift, int blueshift);
void uiplot_checkpalcache(int flag);
void uiplot_convertdata16(uint8 *indata, uint16 *outdata, unsigned int pixels);
void uiplot_convertdata32(uint8 *indata, uint32 *outdata, unsigned int pixels);
void uiplot_render16_x1(uint16 *linedata, uint16 *olddata, uint8 *screen,
                        unsigned int pixels);
void uiplot_render32_x1(uint32 *linedata, uint32 *olddata, uint8 *screen,
                        unsigned int pixels);
void uiplot_render16_x2(uint16 *linedata, uint16 *olddata, uint8 *screen,
                        unsigned int linewidth, unsigned int pixels);
void uiplot_render32_x2(uint32 *linedata, uint32 *olddata, uint8 *screen,
                        unsigned int linewidth, unsigned int pixels);
void uiplot_render16_x2h(uint16 *linedata, uint16 *olddata, uint8 *screen,
                         unsigned int pixels);
void uiplot_render32_x2h(uint32 *linedata, uint32 *olddata, uint8 *screen,
                         unsigned int pixels);
void uiplot_irender16_weavefilter(uint16 *evendata, uint16 *odddata,
                                  uint8 *screen, unsigned int pixels);
void uiplot_irender32_weavefilter(uint32 *evendata, uint32 *odddata,
                                  uint8 *screen, unsigned int pixels);
