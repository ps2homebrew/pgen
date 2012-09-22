#
# PGEN
# Copyright (C) 2007 Mega Man, bootsector
#

IOP_LDFLAGS += -m mipsirx 

IOP_BIN = ../../static-data/cdvd.irx

PS2LIB	= $(PS2DEV)/ps2sdk

#IOP_OBJS = cdvd_iop.o iop_cdvdman.o
IOP_OBJS = cdvd_iop.o


all: $(IOP_BIN)

clean:
	rm -f $(IOP_BIN) $(IOP_OBJS)

include $(PS2LIB)/samples/Makefile.pref
include $(PS2LIB)/samples/Makefile.iopglobal
