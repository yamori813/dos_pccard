WATCOM=../watcom

all:	pccardc.exe

pccardc.exe:	pccardc.o printcis.o readcis.o pci16a.obj
	wlink OP q D w a SYS dos N pccardc.exe F pccardc.o F printcis.o F readcis.o F pci16a.obj

readcis.o:	readcis.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul readcis.c
printcis.o:	printcis.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul printcis.c
pccardc.o:	pccardc.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul pccardc.c

pci16a.obj:	pci16a.asm
	nasm  -f obj pci16a.asm

clean:
	rm -rf *.o *.obj *.err *.exe
