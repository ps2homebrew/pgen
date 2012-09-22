#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (lib/unzip)
#

include ../Makefile.include

OBJECTS		= explode.o unreduce.o unshrink.o unzip.o

all: $(OBJECTS)
	cp -f $(OBJECTS) ../build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
