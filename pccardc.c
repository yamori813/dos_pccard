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

void enable_pccard()
{
	int i, ioaddr, confaddr, confidx;
	unsigned char data;
	unsigned char scpinit[] = {0xfe, 0x80, 0x80, 0x00, 0x00, 0x01, 0xcc, 0x80, 0x03, 0x08, 0x01, 0x01, 0x22};

	printf("Power On\n");
	cb_write_mem(EXCAOFFSET + PCIC_PWRCTL,
//	    PCIC_PWRCTL_VPP1_VCC | PCIC_PWRCTL_PWR_ENABLE | PCIC_PWRCTL_OE);
	    PCIC_PWRCTL_PWR_ENABLE | PCIC_PWRCTL_OE);
	cb_read_mem(EXCAOFFSET + 0x02, &data);
	printf("ExCA 0x02  %02x\n", data);
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
	cb_write_mem(EXCAOFFSET + PCIC_ADDRWIN_ENABLE, PCIC_ADDRWIN_ENABLE_MEM0);

	confaddr = 0x400;
	confidx = 1;
	for (i = 0; i < 8; ++i) {
		cb_read_mem(MEMWINOFFSET + confaddr + i, &data);
		printf("%02x ", data);
	}
	printf("\n");
	/* reset card by CCOR */
	cb_write_mem(MEMWINOFFSET + confaddr, 1 << 7);
	sleep(1);
	cb_write_mem(MEMWINOFFSET + confaddr, 0);
	sleep(1);
	/* 8bit access +/
	cb_write_mem(MEMWINOFFSET + confaddr + 2, 1 << 5);
	/* set configuration */
	cb_write_mem(MEMWINOFFSET + confaddr, confidx);
	for (i = 0; i < 8; ++i) {
		cb_read_mem(MEMWINOFFSET + confaddr + i, &data);
		printf("%02x ", data);
	}
	printf("\n");

	cb_write_mem(EXCAOFFSET + PCIC_PWRCTL,
	    PCIC_PWRCTL_VPP1_VCC | PCIC_PWRCTL_VPP2_VCC | PCIC_PWRCTL_PWR_ENABLE | PCIC_PWRCTL_OE);
	cb_read_mem(EXCAOFFSET + 0x02, &data);
	printf("ExCA 0x02  %02x\n", data);
	ioaddr = 0x330;
	ioaddr = 0x300;
	/* start */
	cb_write_mem(EXCAOFFSET + PCIC_IOADDR0_START_LSB, ioaddr & 0xff);
	cb_write_mem(EXCAOFFSET + PCIC_IOADDR0_START_MSB, ioaddr >> 8);
	/* end */
	cb_write_mem(EXCAOFFSET + PCIC_IOADDR0_STOP_LSB, (ioaddr & 0xff) + 0xc);
	cb_write_mem(EXCAOFFSET + PCIC_IOADDR0_STOP_MSB, ioaddr >> 8);
	cb_write_mem(EXCAOFFSET + PCIC_ADDRWIN_ENABLE, PCIC_ADDRWIN_ENABLE_IO0 | 1);
	cb_write_mem(EXCAOFFSET + PCIC_IOCTL, 0);
	cb_write_mem(EXCAOFFSET + PCIC_IOCTL, 2);
//	cb_write_mem(EXCAOFFSET + 0x03, 0x20);
	for (i = 0; i < 0x0c; ++i) {
		io_read_data(ioaddr + i, &data);
		printf("%02x ", data);
	}
	printf("\n");

	sleep(1);
	for (i = 0; i < 0x0c; ++i) {
		io_write_data(ioaddr + i, scpinit[i]);
	}
// mpu uart mode
//	io_write_data(0x331, 0x3f);

	for (i = 0; i < 0x0c; ++i) {
		io_read_data(ioaddr + i, &data);
		printf("%02x ", data);
	}
	printf("\n");

	sleep(1);
	printf("Power Off\n");
	cb_write_mem(EXCAOFFSET + PCIC_PWRCTL, 0x00);
}
/*****************************************************************************
*****************************************************************************/
int main( int argc, char *argv[] )
{
	pci_t pci;
	int err;
	int dump;

	dump = 0;
	if (argc > 1) {
		if(strcmp("/D", argv[1]) == 0) {
			dump = 1;
		}
	}
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
					if (dump)
						read_cis();
					else
						enable_pccard();
				}
			}
		}
	} while(!pci_iterate(&pci));
	return 0;
}
