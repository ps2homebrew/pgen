#ifndef _DATA_H
#define _DATA_H

extern IIF pgenbgIIF;
extern IIF folderIIF;

extern gsFontTex ocraFnt;
extern gsFontTex zerohourFnt;

#define SONIC_ICN_SIZE	75888
extern unsigned char sonicIcn[];

extern u8 musicZip[];
extern u32 size_musicZip;

// Embedded IRX's
extern u8 ps2fsIrx[];
extern u8 amigamodIrx[];
extern u8 cdvdIrx[];
extern u8 fileXioIrx[];
extern u8 ps2hddIrx[];
extern u8 iomanXIrx[];
extern u8 poweroffIrx[];
extern u8 ps2atadIrx[];
extern u8 ps2dev9Irx[];
extern u8 sjpcmIrx[];
extern u8 freesdIrx[];
extern u8 freesio2Irx[];
extern u8 freepadIrx[];
extern u8 freemtapIrx [];
extern u8 xmcmanIrx[];
extern u8 xmcservIrx[];
extern u8 usbdIrx[];
//extern u8 usb_massIrx[];
extern u8 usb_usbhdfsdIrx[];
extern u8 ps2ipIrx[];
extern u8 ps2smapIrx[];
extern u8 ps2linkIrx[];
extern u32 size_usbdIrx;
//extern u32 size_usb_massIrx;
extern u32 size_usbhdfsdIrx;
extern u32 size_ps2ipIrx;
extern u32 size_ps2linkIrx;
extern u32 size_ps2smapIrx;

#endif /* _DATA_H */
