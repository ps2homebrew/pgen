#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (lib)
#

include ../Makefile.include

CFLAGS		= -D_EE -fshort-double -O2 -G0 -c -Wall -I$(PS2LIB)/common/include -I$(PS2LIB)/ee/include
OBJECTS		= amigamod_rpc.o cdvd_rpc.o sjpcm_rpc.o dlmalloc.o

all: $(OBJECTS)
	cp -f $(OBJECTS) ../build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
