#ifndef _UIP_H
#define _UIP_H

/* this structure is used to pass information from the uip (ui platform
   dependent) bit to the main ui-console bit, it is passed to uip in the
   uip_init() function, but it is setup in uip_vgamode() function */

typedef struct {
  uint8 redshift;                  /* bit position for 5 bits of red */
  uint8 greenshift;                /* bit position for 5 bits of green */
  uint8 blueshift;                 /* bit position for 5 bits of blue */
  uint32 linewidth;                /* line width of mode in bytes */
  uint8 *screenmem0;        /* start of screen memory bank 0 */
  uint8 *screenmem1;        /* start of screen memory bank 1 */
  uint8 *screenmem_w;       /* start of screen bank for writing */
} t_uipinfo;

int uip_init(t_uipinfo *uipinfo);
int uip_initjoysticks(void);
int uip_vgamode(void);
void uip_displaybank(int bank);
void uip_clearscreen(void);
void uip_textmode(void);
int uip_checkkeyboard(void);
void uip_vsync(void);
unsigned int uip_whichbank(void);
void uip_singlebank(void);
void uip_doublebank(void);
uint8 uip_getchar(void);
void uip_clearmiddle(void);
int uip_setcolourbits(int red, int green, int blue);

#endif
