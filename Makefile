WATCOM=../watcom
OBJS=pccardc.o printcis.o readcis.o pci16a.obj

all:	pccardc.exe

pccardc.exe:	$(OBJS)
	wlink OP q D w a SYS dos N pccardc.exe F pccardc.o F printcis.o F readcis.o F pci16a.obj

.c.o:
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul $<

pci16a.obj:	pci16a.asm
	nasm  -f obj pci16a.asm

clean:
	rm -rf *.o *.obj *.err *.exe
