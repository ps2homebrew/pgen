#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (lib/zlib)
#

include ../Makefile.include

OBJECTS		=	adler32.o compress.o crc32.o deflate.o gzio.o infblock.o infcodes.o inffast.o \
			inflate.o inftrees.o infutil.o uncompr.o zutil.o trees.o

all: $(OBJECTS)
	cp -f $(OBJECTS) ../build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
