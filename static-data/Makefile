#
# PGEN
# Copyright (C) 2002 - 2006 James Ponder, Nick Van Veen, Mega Man
#
# Subdir Makefile (static-data)
#
# NOTE: Datafiles must be converted into .s files with bin2s first!
#

include ../Makefile.include

OBJECTS	=	ocra-fnt.o zerohour-fnt.o pgenbg.o folder.o sonic.o \
		amigamodIrx.o cdvdIrx.o fileXioIrx.o ps2hddIrx.o iomanXIrx.o ps2fsIrx.o poweroffIrx.o \
		ps2atadIrx.o ps2dev9Irx.o sjpcmIrx.o musicZip.o usbdIrx.o usb_massIrx.o freesdIrx.o freesio2Irx.o \
		freepadIrx.o xmcmanIrx.o xmcservIrx.o freemtapIrx.o

ZIPFILES = *.mod
ZIPSOURCES = $(addprefix ../data/,$(ZIPFILES))

all: $(OBJECTS)
	cp -f $(OBJECTS) ../build/

clean:
	$(RM) *.o *.map *.lst core *.dis music.zip

%Irx.s: %.irx
	bin2s $< $@ $(*)Irx

%-fnt.s: %.fnt
	bin2s $< $@ $(*)Fnt

%.s: %.iif
	bin2s $< $@ $(*)IIF

%.s: %.icn
	bin2s $< $@ $(*)Icn

%.irx: $(PS2SDK)/iop/irx/%.irx
	cp $< $@

%Zip.s: %.zip
	bin2s $< $@ $(*)Zip

music.zip: $(ZIPSOURCES)
	cd ../data && zip -0 ../static-data/$@ $(ZIPFILES)

