extern uint32 uiplot_palcache[192];
extern uint32 uiplot_palcache_yuv[192];

#ifdef WORDS_BIGENDIAN

#define YUY2_SINGLE(v,l,h) do { \
  l = ((v) << 8) | (((v) & 0xff00) >> 8); \
  h = ((v) << 8) | (((v) >> 16) & 0xff); \
} while (0)

#define YVYU_SINGLE(v,l,h) do { \
  l = ((v) << 8) | (((v) >> 16) & 0xff); \
  h = ((v) << 8) | (((v) & 0xff00) >> 8); \
} while (0)

#define UYVY_SINGLE(v,l,h) do { \
  l = ((v) & 0xff00) | (((v) >> 8) & 0xff); \
  h = (((v) >> 8) & 0xff00) | ((v) & 0xff); \
} while (0)

#else /* !WORDS_BIGENDIAN */

#define YUY2_SINGLE(v,l,h) do { \
  l = ((v) & 0xff) | ((v) & 0xff00); \
  h = ((v) & 0xff) | (((v) >> 8) & 0xff00); \
} while (0)

#define YVYU_SINGLE(v,l,h) do { \
  l = ((v) & 0xff) | (((v) >> 8) & 0xff00); \
  h = ((v) & 0xff) | ((v) & 0xff00); \
} while (0)

#define UYVY_SINGLE(v,l,h) do { \
  l = ((((v) >> 8) & 0xff) | ((v) << 8)) & 0xffff; \
  h = (((v) >> 16) | ((v) << 8)) & 0xffff; \
} while (0)

#endif /* WORDS_BIGENDIAN */


void uiplot_setshifts(int redshift, int greenshift, int blueshift);
void uiplot_checkpalcache(int flag);
void uiplot_convertdata_yuy2(uint8 *indata, uint16 *outdata, unsigned int pixels);
void uiplot_convertdata_yvyu(uint8 *indata, uint16 *outdata, unsigned int pixels);
void uiplot_convertdata_uyvy(uint8 *indata, uint16 *outdata, unsigned int pixels);
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
