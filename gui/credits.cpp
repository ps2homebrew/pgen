#include "pgen.h"

#include "aio.h"
#include "gui.h"
#include "credits.h"


static char creditText[] = {	"\n\n\n\n\n\n\n\n\n\n"
							"PGEN v1.5.1 beta readapted by AKuHAK\n"
							"Sega Genesis/Megadrive emulator\n"
							"for the PS2\n"
							"\n"
							"https://bitbucket.org/AKuHAK/pgen/src\n"
							"\n\n"
							"Credits\n"
							"\n"
							"PS2 specific code, as well as many\n"
							"additions to the emulation core by\n"
							"Nicholas Van Veen (aka Sjeep)\n"
							"\n"
							"Generator, the emulator which PGEN is\n"
							"based on, was written by James Ponder\n"
							"\n"
							"PSG and FM emulation code by\n"
							"Stephane, author of the Gens emulator\n"
							"\n"
							"SjPCM sound output library by Sjeep\n"
							"\n"
							"libMtap multitap library by Sjeep\n"
							"\n"
							"libhdd HDD utility library by Sjeep\n"
							"\n"
							"HDD drivers by Sjeep, mrbrown,\n"
							"Vector and Florin Sasu\n"
							"\n"
							"libCDVD PS2 CDVD library written by\n"
							"Sjeep and Hiryu\n"
							"\n"
							"USB Mass Storage support by\n"
							"Mega Man\n"
							"\n"
							"CPU68K ABCD instruction fix by\n"
							"bootsector\n"
							"\n"
							"PS2Lib PS2 kernel library by Sjeep,\n"
							"Gustavo Scotti, Hiryu, mrbrown,\n"
							"Oobles, Pukko and others\n"
							"\n"
							"gsLib PS2 graphics library by Hiryu\n"
							"\n"
							"AmigaMod mod file player by Vzzrzzn\n"
							"\n"
							"The memory card icon was created by\n"
							"Nikorasu\n"
							"\n\n"
							"Thanks to:\n"
							"\n"
							"Hiryu for gsLib and libCDVD, for his\n"
							"continual help and support\n"
							"\n"
							"The Unknown artists who composed the\n"
							"excellent MOD files used in the menu\n"
							"\n"
							"[vEX], Bgnome and MoRpHiUs for\n"
							"creating tutorials, documents,\n"
							"websites etc related to PGEN\n"
							"\n"
							"Fat Mike, for lending me his spare\n"
							"HDD to assist in adding HDD support\n"
							"to PGEN\n"
							"\n"
							"luckess, for the exclusive version 1.5\n"
							"background picture\n"
							"\n"
							"Barry, for pointing the ABCD\n"
							"instruction bug\n"
							"\n"
							"The BETA testers:\n"
							"Drakonite, emukid, Emulord, Hiryu,\n"
							"Jide, Jimmi, Mark, Rob6021\n"
							"\n\n"
							"Greetz\n"
							"\n"
							"adk, adresd, blackdroid, drakonite,\n"
							"dreamtime, duke, guichi, herben,\n"
							"hiryu, jenova, jules, karmix,\n"
							"longchair, loser, mrbrown, nagra,\n"
							"nikorasu, norecess, oobles, oopo,\n"
							"pukko, rce, [ro]man, runtime, sg2,\n"
							"tyranid, warren, vzzrzzn and anybody\n"
							"else that I forgot to mention :)\n"
							"\n\n\n\n"
};


guiCredits::guiCredits()
{
	creditLines = 0;
	for(int i = 0; i < (int)strlen(creditText) + 1; i++)
		if((creditText[i] == '\n') || (creditText[i] == '\0'))
			creditLines++;

	// HACK bugfix
	creditLines -= 4;
	scrollY = 96;
	scrollHeight = creditLines * ocraFnt.CharGridHeight;
	scrollDelay = 2;
}

void guiCredits::draw()
{
	switch(flag)
	{
		case GUI_FLAG_ANIM_OPEN:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(20, 80, 300, 216, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_OPEN);

			if(boxAnim->drawStep())
			{
				flag = 0;
				delete(boxAnim);
				boxAnim = NULL;
			}

			break;

		case GUI_FLAG_ANIM_CLOSE:

			if(boxAnim == NULL)
				boxAnim = new guiAnimateBox(20, 80, 300, 216, GS_SET_RGBA(166, 170, 255, 86), GUI_FLAG_ANIM_CLOSE);

			if(boxAnim->drawStep())
			{
				flag = 0;
				status = 0;
				delete(boxAnim);
				boxAnim = NULL;
			}

			break;

		default:

			drawPipe->RectFlat(20, 80, 300, 96, Z_SCROLL_M2, GS_SET_RGBA(111, 114, 171, 86));
			drawPipe->RectFlat(20, 96, 300, 200, Z_BOX1, GS_SET_RGBA(166, 170, 255, 86));
			drawPipe->RectFlat(20, 200, 300, 216, Z_SCROLL_M2, GS_SET_RGBA(111, 114, 171, 86));
		
			zerohourFont.Print(20, 300, 80, Z_SCROLL_M3, GS_SET_RGBA(0x80, 0x80, 0x80, 0x80), 
				GSFONT_ALIGN_CENTRE, "Credits");

			drawPipe->setScissorRect(20, 96, 300, 200);
				ocraFont.Print(20, 300, scrollY, Z_LIST, GS_SET_RGBA(0x80, 0x80, 0x80, 180),
				GSFONT_ALIGN_CENTRE, creditText);

			drawPipe->setScissorRect(0, 0, 320, 240);

			if(--scrollDelay < 0)
			{
				scrollY--;
				if(scrollY < (-scrollHeight + 96))
					scrollY = 96;

				scrollDelay = 2;
			}

			// Ok, this is very weird - if this is not here, then the credits screen
			// will dissapear just after opening. Its as though status is somehow set to 0
			// when it shouldnt be. Hrm..
			status = GUI_STAT_RUNNING;
	}
}

void guiCredits::update(u32 padRepeat, u32 padNoRepeat)
{
	if(padNoRepeat & PAD_TRIANGLE)
		flag = GUI_FLAG_ANIM_CLOSE;
}
