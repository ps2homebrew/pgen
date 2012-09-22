#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (gui)
#

include ../Makefile.include

OBJECTS		= gui.o ingame.o saveman.o options.o romlist.o credits.o scrollList.o

all: $(OBJECTS)
	cp -f $(OBJECTS) ../build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
