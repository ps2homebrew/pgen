#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (ps2)
#

include ../Makefile.include

OBJECTS		=	pgen.o render.o gensoundp-ps2.o gfx.o input.o aioCdvd.o aioHdd.o aioMc.o aioUSB.o \
			aioDatafile.o state.o qsort.o aio-fio.o rom.o save.o # compress.o mc.o 

all: $(OBJECTS)
	cp -f $(OBJECTS) ../build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
