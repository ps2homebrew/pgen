#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (sound)
#

include ../Makefile.include

CFLAGS +=	-I$(NEWLIB)/include

OBJECTS		= psg.o ym2612.o

all: $(OBJECTS)
	cp -f $(OBJECTS) ../ps2/build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
