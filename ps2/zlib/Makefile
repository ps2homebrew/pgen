#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (lib/zlib)
#

include ../Makefile.include

OBJECTS		=	adler32.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o \
			trees.o zutil.o compress.o uncompr.o gzio.o

all: $(OBJECTS)
	cp -f $(OBJECTS) ../build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf
