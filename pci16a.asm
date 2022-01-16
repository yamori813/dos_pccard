;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; PCI demo for Turbo C or 16-bit Watcom C, using 16-bit PCI BIOS
;
; Chris Giese     <geezer@execpc.com>     http://my.execpc.com/~geezer
; Release date: Feb 23, 2005
; This code is public domain (no copyright).
; You can do whatever you want with it.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SEGMENT _TEXT PUBLIC CLASS=CODE

PCI_INSTALL_CHECK	equ 0B101h

PCI_READ_CONFIG_BYTE	equ 0B108h
PCI_READ_CONFIG_WORD	equ 0B109h
PCI_READ_CONFIG_DWORD	equ 0B10Ah

PCI_WRITE_CONFIG_BYTE	equ 0B10Bh
PCI_WRITE_CONFIG_WORD	equ 0B10Ch
PCI_WRITE_CONFIG_DWORD	equ 0B10Dh

%macro	EXP	1
	GLOBAL $_%1	; leading underscores
	$_%1:
%endmacro

%macro	IMP	1
	EXTERN $_%1
	%define %1 _%1
%endmacro

; IMPORTS
; from PCIBIO16.C:
IMP g_last_pci_bus

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			pci_detect
; action:		checks for 16-bit PCI BIOS
; in:			(nothing)
; out (error):		return value != 0
; out (success):	return value == 0, sets g_last_pci_bus
; modifies:		EAX, high 16 bits of EBX, ECX, EDX, EDI
; minimum CPU:		'386
; notes:		C prototype:
;			int pci_detect(void);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP pci_detect
	push bp
		mov bp,sp
		push di
		push si
		push dx
		push cx
		push bx
			xor edi,edi
			mov ax,PCI_INSTALL_CHECK
			int 1Ah
			cmp edx,20494350h ; " ICP"
			je ok
			mov ax,0FFFFh
			jmp short no
ok:
			xor ch,ch
			mov [g_last_pci_bus],cx
			xor ax,ax
;;
;; printf("version %u.%u detected, ", regs.x.bx >> 8, regs.x.bx & 0xFF);
;;
no:
		pop bx
		pop cx
		pop dx
		pop si
		pop di
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			pci_read_config_byte
; action:		uses 16-bit PCI BIOS to read PCI config byte
; in:			arguments on stack per C prototype
; out (error):		return value != 0
; out (success):	return value == 0
; modifies:		EAX, *val, high 16 bits of EBX, ECX, EDX
; minimum CPU:		'386
; notes:		C prototype:
;			int pci_read_config_byte(pci_t *pci,
;				unsigned reg, unsigned char *val);
; typedef struct
; {
;	unsigned char bus, dev, fn;
; } pci_t;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP pci_read_config_byte
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov ax,PCI_READ_CONFIG_BYTE
			mov di,[bp + 4]
			mov bh,[di + 0]	; bus
			mov bl,[di + 1]	; dev
			shl bl,3
			or  bl,[di + 2] ; fn
			mov di,[bp + 6]	; reg
			int 1Ah
			mov bx,[bp + 8]	; val
			mov [bx],cl	; *val
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			pci_read_config_word
; action:		uses 16-bit PCI BIOS to read PCI config word
; in:			arguments on stack per C prototype
; out (error):		return value != 0
; out (success):	return value == 0
; modifies:		EAX, *val, high 16 bits of EBX, ECX, EDX
; minimum CPU:		'386
; notes:		C prototype:
;			int pci_read_config_word(pci_t *pci,
;				unsigned reg, unsigned short *val);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP pci_read_config_word
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov ax,PCI_READ_CONFIG_WORD
			mov di,[bp + 4]
			mov bh,[di + 0]	; bus
			mov bl,[di + 1]	; dev
			shl bl,3
			or  bl,[di + 2] ; fn
			mov di,[bp + 6] ; reg
			int 1Ah
			mov bx,[bp + 8] ; val
			mov [bx],cx	; *val
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			pci_read_config_dword
; action:		uses 16-bit PCI BIOS to read PCI config dword
; in:			arguments on stack per C prototype
; out (error):		return value != 0
; out (success):	return value == 0
; modifies:		EAX, *val, high 16 bits of EBX, ECX, EDX
; minimum CPU:		'386
; notes:		C prototype:
;			int pci_read_config_dword(pci_t *pci,
;				unsigned reg, unsigned long *val);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


EXP pci_read_config_dword
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov ax,PCI_READ_CONFIG_DWORD
			mov di,[bp + 4]
			mov bh,[di + 0]	; bus
			mov bl,[di + 1]	; dev
			shl bl,3
			or  bl,[di + 2] ; fn
			mov di,[bp + 6] ; reg
			int 1Ah
			mov bx,[bp + 8] ; val
			mov [bx + 0],ecx ; *val
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			pci_write_config_byte
; action:		uses 16-bit PCI BIOS to write PCI config byte
; in:			arguments on stack per C prototype
; out (error):		return value != 0
; out (success):	return value == 0
; modifies:		EAX, high 16 bits of EBX, ECX, EDX
; minimum CPU:		'386
; notes:		C prototype:
;			int pci_write_config_byte(pci_t *pci,
;				unsigned reg, unsigned char val);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP pci_write_config_byte
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov ax,PCI_WRITE_CONFIG_BYTE
			mov di,[bp + 4]
			mov bh,[di + 0]	; bus
			mov bl,[di + 1]	; dev
			shl bl,3
			or  bl,[di + 2] ; fn
			mov di,[bp + 6] ; reg
			mov cl,[bp + 8] ; val
			int 1Ah
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			pci_write_config_word
; action:		uses 16-bit PCI BIOS to write PCI config word
; in:			arguments on stack per C prototype
; out (error):		return value != 0
; out (success):	return value == 0
; modifies:		EAX, high 16 bits of EBX, ECX, EDX
; minimum CPU:		'386
; notes:		C prototype:
;			int pci_write_config_word(pci_t *pci,
;				unsigned reg, unsigned short val);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP pci_write_config_word
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov ax,PCI_WRITE_CONFIG_WORD
			mov di,[bp + 4]
			mov bh,[di + 0]	; bus
			mov bl,[di + 1]	; dev
			shl bl,3
			or  bl,[di + 2] ; fn
			mov di,[bp + 6] ; reg
			mov cx,[bp + 8] ; val
			int 1Ah
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			pci_write_config_dword
; action:		uses 16-bit PCI BIOS to write PCI config dword
; in:			arguments on stack per C prototype
; out (error):		return value != 0
; out (success):	return value == 0
; modifies:		EAX, high 16 bits of EBX, ECX, EDX
; minimum CPU:		'386
; notes:		C prototype:
;			int pci_write_config_dword(pci_t *pci,
;				unsigned reg, unsigned long val);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

EXP pci_write_config_dword
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov ax,PCI_WRITE_CONFIG_DWORD
			mov di,[bp + 4]
			mov bh,[di + 0]	; bus
			mov bl,[di + 1]	; dev
			shl bl,3
			or  bl,[di + 2] ; fn
			mov di,[bp + 6] ; reg
			mov ecx,[bp + 8]; val
			int 1Ah
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

EXP legacy_index
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov al,[bp + 4] ; val
			mov dx,3e0h
			out dx,al
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

EXP legacy_write_data
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov al,[bp + 4] ; val
			mov dx,3e1h
			out dx,al
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

EXP legacy_read_data 
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov bx,[bp + 4] ; val
			mov dx,3e1h
			in  al, dx
			mov [bx + 0], al ; *val
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret

EXP legacy_read_mem
	push bp
	mov bp,sp
		push di
		push cx
		push bx
			mov di,[bp + 4] ; reg
			push ds
			mov ax, 0d000h
			mov ds, ax
			mov cl, [di]
			pop ds
			mov bx, [bp + 6] ; val
			mov [bx], cl	; *val
		pop bx
		pop cx
		pop di

		mov al,ah
		xor ah,ah
	pop bp
	ret
