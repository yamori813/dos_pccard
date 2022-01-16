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

#if defined(__TURBOC__)
/* nothing */

#elif defined(__WATCOMC__)
#if defined(__386__)
#error This is a 16-bit program. Compile with WCC
#endif

#else
#error Unsupported compiler
#endif

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

int cdecl legacy_read_mem(unsigned offset, unsigned char *val);

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
				printf("Cardbus controller detected\n");
				err = pci_write_config_dword(&pci, 0x44, 0x3e1);
				err = pci_read_config_dword(&pci, 0x04, &reg);
				reg |= 1;
				err = pci_write_config_dword(&pci, 0x04, reg);
/* http://oswiki.osask.jp/?PCIC */

				legacy_index(0x00);
				legacy_read_data(&data);
				printf("ExCA 0x00  %02x\n", data);
				legacy_index(0x06);
				legacy_write_data(0x20);
				legacy_index(0x04);
				legacy_write_data(0x40);
				legacy_index(0x01);
				legacy_read_data(&data);
				printf("ExCA 0x01  %02x\n", data);
				if ((data & 0x0c) == 0x0c) {
					printf("Card inserted\n");
					printf("Power On\n");
					legacy_index(0x02);
					legacy_write_data(0x10);
					sleep(1);
					legacy_index(0x01);
					legacy_read_data(&data);
					printf("ExCA 0x01  %02x\n", data);
					printf("Reset Card\n");
					legacy_index(0x03);
					legacy_write_data(0x00);
					sleep(1);
					legacy_index(0x03);
					legacy_write_data(0x40);
					sleep(1);

					/* 0xd0000～0xd0fff */
					legacy_index(0x10);
					legacy_write_data(0xd0);
					legacy_index(0x11);
					legacy_write_data(0xc0);
					legacy_index(0x12);
					legacy_write_data(0xd0);
					legacy_index(0x13);
					legacy_write_data(0x00);
					legacy_index(0x14);
					legacy_write_data(0x30);
					legacy_index(0x15);
					legacy_write_data(0x7f);

					printf("Enable Memory\n");
					legacy_index(0x06);
					legacy_write_data(0x21);

					for (i = 0; i < 64; ++i) {
						legacy_read_mem(i, &data);
						printf("%02x ", data);
					}

					printf("\nPower Off\n");
					legacy_index(0x02);
					legacy_write_data(0x00);
					sleep(1);
					legacy_index(0x01);
					legacy_read_data(&data);
					printf("ExCA 0x01  %02x\n", data);
				}
			}
		}
	} while(!pci_iterate(&pci));
	return 0;
}