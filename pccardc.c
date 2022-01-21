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

#include "readcis.h"

#if defined(__TURBOC__)
/* nothing */

#elif defined(__WATCOMC__)
#if defined(__386__)
#error This is a 16-bit program. Compile with WCC
#endif

#else
#error Unsupported compiler
#endif

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
