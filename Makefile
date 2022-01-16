WATCOM=../watcom

legacy.exe:	legacy.o pci16a.obj
	wlink OP q D w a SYS dos N legacy.exe F legacy.o F pci16a.obj

legacy.o:	legacy.c
	wcc  -I$(WATCOM)/h  -zq -3 -d2 -hw -ox -w=9 -zc -zp1 -ms -fr=nul legacy.c
pci16a.obj:	pci16a.asm
	nasm  -f obj pci16a.asm

clean:
	rm -rf *.o *.obj *.err *.exe
