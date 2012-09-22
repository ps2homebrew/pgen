#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (main)
#

include ../Makefile.include

OBJECTS		= cpu68k.o cpuz80.o event.o generator.o gensound.o mem68k.o memz80.o reg68k.o vdp.o setjmp.o

all: $(OBJECTS)
	cp -f $(OBJECTS) ../ps2/build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
