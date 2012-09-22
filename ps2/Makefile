#
# PGEN
# Copyright (C) 2002 - 2007 James Ponder, Nick Van Veen, Mega Man, bootsector
#
# Root Makefile
#

PROJECT		= pgen
OBJECTS		= build/*.o

include ./Makefile.include

all: $(PROJECT).elf

$(PROJECT).elf: clean-build make-dirs
	$(CXX) $(LDFLAGS) -o $(PROJECT).elf -T$(PS2SDK)/ee/startup/linkfile $(OBJECTS) $(LIBS)
	ee-strip --strip-all $(PROJECT).elf
#	sjcrunch $(PROJECT).elf $(PROJECT)-crunched.elf
	ps2-packer $(PROJECT).elf $(PROJECT)-packed.elf > /dev/null

clean-build:
	rm -f build/*.o *.elf

make-dirs:
	$(MAKE) -C ../z80
	$(MAKE) -C ../cpu68k
	$(MAKE) -C ../main
	$(MAKE) -C ps2
	$(MAKE) -C ../sound
	$(MAKE) -C iop
	$(MAKE) -C static-data
	$(MAKE) -C gui
	$(MAKE) -C lib
	$(MAKE) -C zlib
	$(MAKE) -C unzip

clean: clean-build
	$(MAKE) -C ../z80 clean
	$(MAKE) -C ../cpu68k clean
	$(MAKE) -C ../main clean
	$(MAKE) -C ps2 clean
	$(MAKE) -C ../sound clean
	$(MAKE) -C iop clean
	$(MAKE) -C static-data clean	
	$(MAKE) -C gui clean
	$(MAKE) -C lib clean
	$(MAKE) -C zlib clean
	$(MAKE) -C unzip clean
