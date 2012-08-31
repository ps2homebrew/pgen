/*******************************
*                              *
*       GSLIB by Hiryu         *
*                              *
* General GS Definitions       *
*                              *
*******************************/

#ifndef _GSDEFS_H_
#define _GSDEFS_H_

// GS Privileged registers.
#define GS_PMODE	*((volatile unsigned long int*)0x12000000)
#define GS_SMODE2	*((volatile unsigned long int*)0x12000020)
#define GS_DISPFB1	*((volatile unsigned long int*)0x12000070)
#define GS_DISPLAY1	*((volatile unsigned long int*)0x12000080)
#define GS_BGCOLOUR	*((volatile unsigned long int*)0x120000E0)
#define GS_CSR      *((volatile unsigned long int*)0x12001000)
#define GS_BUSDIR   *((volatile unsigned long int*)0x12001040)


// GIF registers
#define GIF_CTRL	*((volatile unsigned long int*)0x12001000)
#define GIF_STAT    *((volatile unsigned long int*)0x10003020)


// GS Regs
#define GS_REG_PRIM			0x00
#define GS_REG_RGBAQ		0x01
#define GS_REG_UV			0x03
#define GS_REG_XYZ2			0x05
#define GS_REG_TEX0_1		0x06
#define GS_REG_TEX0_2		0x07
#define GS_REG_CLAMP_1		0x08
#define GS_REG_TEX1_1		0x14
#define GS_REG_XYOFFSET_1	0x18
#define GS_REG_XYOFFSET_2	0x19
#define GS_REG_PRMODECONT	0x1A
#define GS_REG_TEXCLUT		0x1C
#define GS_REG_TEXA			0x3B
#define GS_REG_TEXFLUSH		0x3F
#define GS_REG_SCISSOR_1	0x40
#define GS_REG_SCISSOR_2	0x41
#define GS_REG_ALPHA_1		0x42
#define GS_REG_ALPHA_2		0x43
#define GS_REG_DTHE			0x45
#define GS_REG_COLCLAMP		0x46
#define GS_REG_TEST_1		0x47
#define GS_REG_TEST_2		0x48
#define GS_REG_PABE			0x49	//;
#define GS_REG_FRAME_1		0x4C
#define GS_REG_FRAME_2		0x4D
#define GS_REG_ZBUF_1		0x4E
#define GS_REG_ZBUF_2		0x4F
#define GS_REG_BITBLTBUF	0x50
#define GS_REG_TRXPOS		0x51
#define GS_REG_TRXREG		0x52
#define GS_REG_TRXDIR		0x53


// GS PRIM TYPES
#define GS_PRIM_POINT			0x0
#define GS_PRIM_LINE			0x1
#define GS_PRIM_LINESTRIP		0x2
#define GS_PRIM_TRIANGLE		0x3
#define GS_PRIM_TRIANGLE_STRIP	0x4
#define GS_PRIM_TRIANGLE_FAN	0x5
#define GS_PRIM_SPRITE			0x6

#define GS_ALPHA_SOURCE 0x00
#define GS_ALPHA_FRAME  0x01
#define GS_ALPHA_FIXED  0x02


// Generic Definitions
#define GS_ENABLE			1
#define GS_DISABLE			0

// GS Display Mode Settings
#define GS_TV_INTERLACE		1
#define GS_TV_NONINTERLACE	0
#define GS_TV_PAL			3
#define GS_TV_NTSC			2
#define GS_TV_AUTO			((*((char*)0x1FC7FF52))=='E')+2

// GS PSM Settings
#define GS_PSMCT32		0x00
#define GS_PSMCT24		0x01
#define GS_PSMCT16		0x02
#define GS_PSMCT16S		0x0A
#define GS_PSGPU24		0x12

#define GS_PSMT8		0x13
#define GS_PSMT4		0x14
#define GS_PSMT8H		0x1B
#define GS_PSMT4HL		0x24
#define GS_PSMT4HH		0x2C

#define GS_PSMZ32		0x00
#define GS_PSMZ24		0x01
#define GS_PSMZ16		0x02
#define GS_PSMZ16S		0x0A

#define GS_CSM1			0
#define GS_CSM2			1

#ifndef NULL
#define NULL	0
#endif

//typedef unsigned int u32;

#define GS_FILTER_NEAREST	0x00
#define GS_FILTER_LINEAR	0x01

// Definitions for texture sizes
enum gsTexSize {
	GS_TEX_SIZE_2 = 1,
	GS_TEX_SIZE_4,
	GS_TEX_SIZE_8,
	GS_TEX_SIZE_16,
	GS_TEX_SIZE_32,
	GS_TEX_SIZE_64,
	GS_TEX_SIZE_128,
	GS_TEX_SIZE_256,
	GS_TEX_SIZE_512,
	GS_TEX_SIZE_1024
};

// IIF file format header

typedef struct{
	u32 identifier; // 'IIF1' - IIFv (v = version of iif)
	u32 width;
	u32 height;
	u32 psm;
	char pixel_data;
} IIF;


//
// Macro's used to set the contents of GS registers
//

#define GS_SET_ALPHA(a,b,c,d,fix) \
	((u64) (a) | ((u64)(b)<<2) | ((u64)(c)<<4) | ((u64)(d)<<6) | ((u64)(fix)<<32))

#define GS_SET_RGBAQ(r, g, b, a, q) \
	((u64)(r)        | ((u64)(g) << 8) | ((u64)(b) << 16) | \
	((u64)(a) << 24) | ((u64)(q) << 32))

#define GS_SET_RGBA(r, g, b, a) \
	((u64)(r)        | ((u64)(g) << 8) | ((u64)(b) << 16) | ((u64)(a) << 24))

#define GS_SET_XYZ(x, y, z) \
	((u64)(x<<4) | ((u64)(y<<4) << 16) | ((u64)(z) << 32))

#define GS_SET_UV(u, v) ((u64)(u<<4) | ((u64)(v<<4) << 16))

#define GS_SET_COLQ(c) (0x3f80000000000000 | c)

#define GS_SET_FRAME(fbp, fbw, psm, fbmask) \
	( (u64)((fbp)>>13) | (u64)((u64)((fbw)/64)<< 16) | (u64)((u64)(psm)<< 24) | (u64)((u64)(fbmask) << 32) )

#define GS_SET_XYOFFSET(ofx, ofy) ((u64)(ofx<<4) | ((u64)(ofy<<4) << 32))

#define GS_SET_ZBUF(zbp, psm, zmsk) \
	( (u64)((zbp)>>13) | ((u64)(psm) << 24) | ((u64)(zmsk) << 32) )

#define GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
	( (u64)(ate)         | ((u64)(atst) << 1) | ((u64)(aref) << 4)  | ((u64)(afail) << 12) | \
	((u64)(date) << 14) | ((u64)(datm) << 15) | ((u64)(zte) << 16)  | ((u64)(ztst) << 17) )

#define GS_SET_SCISSOR(scax0, scax1, scay0, scay1) \
	( (u64)(scax0) | ((u64)(scax1) << 16) | ((u64)(scay0) << 32) | ((u64)(scay1) << 48) )

#define GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
	((u64)(prim)      | ((u64)(iip) << 3)  | ((u64)(tme) << 4) | \
	((u64)(fge) << 5) | ((u64)(abe) << 6)  | ((u64)(aa1) << 7) | \
	((u64)(fst) << 8) | ((u64)(ctxt) << 9) | ((u64)(fix) << 10))

#define GS_SET_DISPFB(fbp, fbw, psm, dbx, dby) \
	((u64)((fbp)>>13) | ((u64)(fbw/64) << 9) | ((u64)(psm) << 15) | ((u64)(dbx) << 32) | ((u64)(dby) << 43))

#define GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm) \
	((u64)(sbp)         | ((u64)(sbw) << 16) | \
	((u64)(spsm) << 24) | ((u64)(dbp) << 32) | \
	((u64)(dbw) << 48)  | ((u64)(dpsm) << 56))

#define GS_SET_TRXDIR(xdr) ((u64)(xdr))

#define GS_SET_TRXPOS(ssax, ssay, dsax, dsay, dir) \
	((u64)(ssax)        | ((u64)(ssay) << 16) | \
	((u64)(dsax) << 32) | ((u64)(dsay) << 48) | \
	((u64)(dir) << 59))

#define GS_SET_TRXREG(rrw, rrh) \
	((u64)(rrw) | ((u64)(rrh) << 32))

#define GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, cbp, cpsm, csm, csa, cld) \
	((u64)(tbp)         | ((u64)(tbw) << 14) | ((u64)(psm) << 20)  | ((u64)(tw) << 26) | \
	((u64)(th) << 30)   | ((u64)(tcc) << 34) | ((u64)(tfx) << 35)  | ((u64)(cbp) << 37) | \
	((u64)(cpsm) << 51) | ((u64)(csm) << 55) | ((u64)(csa) << 56)  | ((u64)(cld) << 61))

#define GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
	((u64)(lcm) | ((u64)(mxl) << 2)  | ((u64)(mmag) << 5) | ((u64)(mmin) << 6) | \
	((u64)(mtba) << 9) | ((u64)(l) << 19) | ((u64)(k) << 32))

#define GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv) \
	((u64)(wms)         | ((u64)(wmt) << 2) | ((u64)(minu) << 4)  | ((u64)(maxu) << 14) | \
	((u64)(minv) << 24) | ((u64)(maxv) << 34))

#define GS_SET_TEXA(ta0, aem, ta1) \
	((u64)(ta0) | ((u64)(aem) << 15) | ((u64)(ta1) << 32))

#define GS_SET_DISPLAY(width,height,xpos,ypos) \
	(((u64)(height-1)<<44) | ((u64)0x9FF<<32) | \
	((((2560+(width-1))/width)-1)<<23) | \
	(ypos<<12) | (xpos*(2560/width)))


// Misc macro's

#define GIF_SET_TAG(nloop, eop, pre, prim, flg, nreg) \
	( (u64)(nloop) | ((u64)(eop)<<15) | ((u64)(pre) << 46) | \
	((u64)(prim)<<47) | ((u64)(flg)<<58) | ((u64)(nreg)<<60) )


#endif /* _GSDEFS_H_ */
