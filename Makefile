WATCOM=../watcom

all:	dumpcis.exe

dumpcis.exe:	dumpcis.o printcis.o readcis.o pci16a.obj
	wlink OP q D w a SYS dos N dumpcis.exe F dumpcis.o F printcis.o F readcis.o F pci16a.obj

readcis.o:	readcis.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul readcis.c
printcis.o:	printcis.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul printcis.c
dumpcis.o:	dumpcis.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul dumpcis.c
pci16a.obj:	pci16a.asm
	nasm  -f obj pci16a.asm

clean:
	rm -rf *.o *.obj *.err *.exe
