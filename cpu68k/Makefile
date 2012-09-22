#
# PGEN
# Copyright (C) 2002 James Ponder, Nick Van Veen
#
# Subdir Makefile (cpu68k)
#

include ../Makefile.include

OBJECTS	=	cpu68k-0.o cpu68k-1.o cpu68k-2.o cpu68k-3.o \
			cpu68k-4.o cpu68k-5.o cpu68k-6.o cpu68k-7.o \
			cpu68k-8.o cpu68k-9.o cpu68k-a.o cpu68k-b.o \
			cpu68k-c.o cpu68k-d.o cpu68k-e.o cpu68k-f.o

BUILT_SOURCES = def68k-iibs.h def68k-funcs.h def68k-proto.h \
                cpu68k-0.c cpu68k-1.c cpu68k-2.c cpu68k-3.c \
                cpu68k-4.c cpu68k-5.c cpu68k-6.c cpu68k-7.c \
                cpu68k-8.c cpu68k-9.c cpu68k-a.c cpu68k-b.c \
                cpu68k-c.c cpu68k-d.c cpu68k-e.c cpu68k-f.c

all: $(BUILT_SOURCES) $(OBJECTS)
	cp -f $(OBJECTS) ../ps2/build/

clean:
	$(RM) *.o *.map *.lst core *.dis *.elf gen68k def68k cpu68k-* def68k-*

###########################
## Generate sources      ##
###########################

gen68k.o:
	gcc -c gen68k.c -I../hdr -I.
tab68k.o:
	gcc -c tab68k.c -I../hdr -I.
def68k.o:
	gcc -c def68k.c -I../hdr -I.


gen68k_OBJECTS = gen68k.o tab68k.o
gen68k_SOURCES = gen68k.c tab68k.c
gen68k_DEPENDENCIES = def68k-iibs.h

gen68k: $(gen68k_OBJECTS) $(gen68k_DEPENDENCIES)
	rm -f gen68k
	gcc -o gen68k $(gen68k_OBJECTS)



def68k_OBJECTS = def68k.o tab68k.o
def68k_SOURCES = def68k.c tab68k.c

def68k: $(def68k_OBJECTS) $(def68k_DEPENDENCIES)
	rm -f def68k
	gcc -o def68k $(def68k_OBJECTS)



def68k-iibs.h:	def68k def68k.def
	./def68k
def68k-funcs.h:	def68k def68k.def
	./def68k
def68k-proto.h:	def68k def68k.def
	./def68k

cpu68k-0.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-1.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-2.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-3.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-4.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-5.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-6.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-7.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-8.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-9.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-a.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-b.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-c.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-d.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-e.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
cpu68k-f.c:	gen68k$(EXEEXT)
	./gen68k$(EXEEXT)
