#
# PGEN
# Copyright (C) 2007 Mega Man, bootsector
#

IOP_LDFLAGS += -m mipsirx

IOP_BIN = ../../static-data/sjpcm.irx

IOP_OBJS = sjpcm_irx.o hw.o imports.o

PS2LIB	= $(PS2DEV)/ps2sdk


all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

include $(PS2LIB)/samples/Makefile.pref
include $(PS2LIB)/samples/Makefile.iopglobal

# A rule to build imports.lst.
%.o : %.lst
	echo "#include \"irx_imports.h\"" > build-imports.c
	cat $< >> build-imports.c
	$(IOP_CC) $(IOP_CFLAGS) -c build-imports.c -o $@
		-rm -f build-imports.c

