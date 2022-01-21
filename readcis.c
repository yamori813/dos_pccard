/*****************************************************************************
PCI demo for Turbo C or 16-bit Watcom C, using 16-bit PCI BIOS

Chris Giese     <geezer@execpc.com>     http://my.execpc.com/~geezer
Release date: Feb 23, 2005
This code is public domain (no copyright).
You can do whatever you want with it.
*****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <dos.h>

#include "i82365reg.h"

#if defined(__TURBOC__)
/* nothing */

#elif defined(__WATCOMC__)
#if defined(__386__)
#error This is a 16-bit program. Compile with WCC
#endif

#else
#error Unsupported compiler
#endif

#define EXCAOFFSET	0x800

typedef struct
{
	unsigned char bus, dev, fn;
} pci_t;

/* IMPORTS
from PCI16A.ASM */
int cdecl pci_detect(void);
int cdecl pci_read_config_byte(pci_t *pci, unsigned reg, unsigned char *val);
int cdecl pci_read_config_word(pci_t *pci, unsigned reg, unsigned short *val);
int cdecl pci_read_config_dword(pci_t *pci, unsigned reg, unsigned long *val);
int cdecl pci_write_config_byte(pci_t *pci, unsigned reg, unsigned char val);
int cdecl pci_write_config_word(pci_t *pci, unsigned reg, unsigned short val);
int cdecl pci_write_config_dword(pci_t *pci, unsigned reg, unsigned long val);

int cdecl legacy_index(unsigned char val);
int cdecl legacy_write_data(unsigned char val);
int cdecl legacy_read_data(unsigned char *val);

int cdecl cb_read_mem(unsigned offset, unsigned char *val);
int cdecl cb_write_mem(unsigned offset, unsigned char val);

unsigned g_last_pci_bus;
/*****************************************************************************
*****************************************************************************/
static int pci_iterate(pci_t *pci)
{
	unsigned char hdr_type = 0x80;

/* if first function of this device, check if multi-function device
(otherwise fn==0 is the _only_ function of this device) */
	if(pci->fn == 0)
	{
		if(pci_read_config_byte(pci, 0x0E, &hdr_type))
			return -1;	/* error */
	}
/* increment iterators
fn (function) is the least significant, bus is the most significant */
	pci->fn++;
	if(pci->fn >= 8 || (hdr_type & 0x80) == 0)
	{
		pci->fn = 0;
		pci->dev++;
		if(pci->dev >= 32)
		{
			pci->dev = 0;
			pci->bus++;
			if(pci->bus > g_last_pci_bus)
				return 1; /* done */
		}
	}
	return 0;
}

static void read_cis()
{
	int i, pos;
	unsigned char data;
	unsigned char type, len;

	printf("Power On\n");
	cb_write_mem(EXCAOFFSET + PCIC_PWRCTL,
	    PCIC_PWRCTL_PWR_ENABLE | PCIC_PWRCTL_OE);
	sleep(1);
	cb_read_mem(EXCAOFFSET + 0x01, &data);
	printf("ExCA 0x01  %02x\n", data);
	printf("Reset Card\n");
	cb_write_mem(EXCAOFFSET + 0x03, 0x00);
	sleep(1);
	cb_write_mem(EXCAOFFSET + 0x03, 0x40);
	sleep(1);

	/*
	   set memory windows to 0xb1000 - 0xb1fff (4Kbyte)
	*/
	cb_write_mem(EXCAOFFSET + 0x10, 0xb1);
	cb_write_mem(EXCAOFFSET + 0x11, 0xc0);
	cb_write_mem(EXCAOFFSET + 0x12, 0xb1);
	cb_write_mem(EXCAOFFSET + 0x13, 0x00);
	cb_write_mem(EXCAOFFSET + 0x14, 0x4f);
	cb_write_mem(EXCAOFFSET + 0x15, 0x7f);
	cb_write_mem(EXCAOFFSET + 0x40, 0x00);
	cb_write_mem(EXCAOFFSET + PCIC_ADDRWIN_ENABLE, 0x01);

	sleep(1);
	pos = 0;
	while (1) {
		cb_read_mem(pos+0x1000, &type);
		printf("%02x ", type);
		pos += 2;
		if (type == 0xff)
			break;
		cb_read_mem(pos+0x1000, &len);
		printf("%02x ", len);
		pos += 2;
		/* check this block data length and next type,len over 4k */
		if ((len + 2) * 2 + pos > 0x1000) {
			printf("CIS is over 4K\n");
			break;
		}
		for (i = 0; i < len; ++i) {
			cb_read_mem(pos+0x1000, &data);
			printf("%02x ", data);
			pos += 2;
		}
		printf("\n");
	}
	sleep(1);
	printf("\nPower Off\n");
	cb_write_mem(EXCAOFFSET + PCIC_PWRCTL, 0x00);
}
/*****************************************************************************
*****************************************************************************/
int main(void)
{
	pci_t pci;
	int err;

/* check for PCI BIOS */
	if(pci_detect())
		return 1;
/* display numeric ID of all PCI devices detected */
	memset(&pci, 0, sizeof(pci));
	do
	{
		unsigned long id;

/* 00=PCI_VENDOR_ID */
		err = pci_read_config_dword(&pci, 0x00, &id);
		if(err)
ERR:		{
			printf("Error 0x%02X reading PCI config\n", err);
			return 1;
		}
/* anything there? */
		if(id != 0xFFFFFFFFL)
		{
			printf("bus %u, device %2u, function %u: "
				"device=%04lX:%04lX\n", pci.bus,
				pci.dev, pci.fn, id & 0xFFFF, id >> 16);
		}
	} while(!pci_iterate(&pci));
/* find a USB controller */
	memset(&pci, 0, sizeof(pci));
	do
	{
		unsigned char major, minor;

/* 0B=class */
		err = pci_read_config_byte(&pci, 0x0B, &major);
		if(err)
			goto ERR;
/* 0A=sub-class */
		err = pci_read_config_byte(&pci, 0x0A, &minor);
		if(err)
			goto ERR;
/* anything there? */
		if(major != 0xFF || minor != 0xFF)
		{
printf("detected device of class %u.%u\n", major, minor);
			if(major == 6 && minor == 7)
			{
				int i;
				unsigned long reg;
				unsigned char data;
				/*
				   set CardBus Socket/ExCA Base Address
				   0xb0000 - 0xb0fff (4Kbyte)
				*/
				err = pci_write_config_dword(&pci, 0x10,
				    0xb0000);

				err = pci_read_config_dword(&pci, 0x04, &reg);
				reg |= 7;
				err = pci_write_config_dword(&pci, 0x04, reg);
				for (i = 0; i < 64; i += 2) {
					cb_read_mem(i+0x00, &data);
					printf("%02x ", data);
				}
				printf("\n");

/* http://oswiki.osask.jp/?PCIC */
				cb_read_mem(EXCAOFFSET + PCIC_IDENT, &data);
				printf("ExCA PCIC_IDENT %02x\n", data);

				cb_write_mem(EXCAOFFSET + 0x06, 0x20);
				cb_write_mem(EXCAOFFSET + 0x03, 0x40);

				cb_read_mem(EXCAOFFSET + PCIC_IF_STATUS, &data);
				printf("ExCA PCIC_IF_STATUS %02x\n", data);
				if ((data & PCIC_IF_STATUS_CARDDETECT_MASK) ==
				     PCIC_IF_STATUS_CARDDETECT_PRESENT) {
					printf("Card inserted\n");
					read_cis();
				}
			}
		}
	} while(!pci_iterate(&pci));
	return 0;
}
