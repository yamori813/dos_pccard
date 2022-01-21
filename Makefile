WATCOM=../watcom

all:	readcis.exe

readcis.exe:	readcis.o printcis.o sub.o pci16a.obj
	wlink OP q D w a SYS dos N readcis.exe F readcis.o F printcis.o F sub.o F pci16a.obj

readcis.o:	readcis.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul readcis.c
printcis.o:	printcis.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul printcis.c
sub.o:	sub.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul sub.c
pci16a.obj:	pci16a.asm
	nasm  -f obj pci16a.asm

clean:
	rm -rf *.o *.obj *.err *.exe
