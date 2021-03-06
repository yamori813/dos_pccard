/*
 * Copyright (c) 1995 Andrew McRae.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: readcis.h,v 1.1.1.1.8.1.2.1 1999/03/17 12:52:45 toshi Exp $
 */


typedef unsigned char u_char;
typedef unsigned long u_int;
typedef unsigned off_t;

struct tuple {
//	struct tuple *next;
	unsigned char code;
	int     length;
	unsigned char *data;
};

struct tuple_list {
	struct tuple_list *next;
	struct tuple *tuples;
	off_t   offs;
	int     flags;
};

struct tuple_info {
	char   *name;
	unsigned char code;
	unsigned char length;		/* 255 means variable length */
};

#if 0
/*
 *	Memory device descriptor.
 */
struct dev_mem {
	unsigned char valid;
	unsigned char type;
	unsigned char speed;
	unsigned char wps;
	unsigned char addr;
	unsigned char units;
};

/*
 *	One I/O structure describing a possible I/O map
 *	of the card.
 */
struct cis_ioblk {
	struct cis_ioblk *next;
	unsigned int addr;
	unsigned int size;
};

/*
 *	A structure storing a memory map for the card.
 */
struct cis_memblk {
	struct cis_memblk *next;
	unsigned int address;
	unsigned int length;
	unsigned int host_address;
};

/*
 *	One configuration entry for the card.
 */
struct cis_config {
	struct cis_config *next;
	unsigned int pwr:1;		/* Which values are defined. */
	unsigned int timing:1;
	unsigned int iospace:1;
	unsigned int irq:1;
	unsigned int memspace:1;
	unsigned int misc_valid:1;
	unsigned char id;
	unsigned char io_blks;
	unsigned char io_addr;
	unsigned char io_bus;
	struct cis_ioblk *io;
	unsigned char irqlevel;
	unsigned char irq_flags;
	unsigned irq_mask;
	unsigned char memwins;
	struct cis_memblk *mem;
	unsigned char misc;
};

/*
 *	Structure holding all data retrieved from the
 *	CIS block on the card.
 *	The default configuration contains interface defaults
 *	not listed in each separate configuration.
 */
struct cis {
	struct tuple_list *tlist;
	char    *manuf;
	char    *vers;
	char    *add_info1;
	char    *add_info2;
	unsigned char maj_v, min_v;
	unsigned char last_config;
	unsigned char ccrs;
	unsigned long reg_addr;
	unsigned char func_id1, func_id2;
	struct dev_mem attr_mem;
	struct dev_mem common_mem;
	struct cis_config *def_config;
	struct cis_config *conf;
};
#endif

#define	tpl32(tp)	(((long)*((tp) + 3) << 24) | \
			 ((long)*((tp) + 2) << 16) | \
			 ((long)*((tp) + 1) << 8)  | *(tp))
#define	tpl24(tp)	(((long)*((tp) + 2) << 16) | \
			 ((long)*((tp) + 1) << 8)  | *(tp))
#define	tpl16(tp)	((*((tp) + 1) << 8)  | *(tp))

#if 0
void   *xmalloc(int);
void    dump(unsigned char *, int);
void    dumpcis(struct cis *);
void    freecis(struct cis *);
struct cis *readcis(int);

char   *tuple_name(unsigned char);
#endif
u_int   parse_num(int, u_char *, u_char **, int);

#define EXCAOFFSET	0x800
#define MEMWINOFFSET	0x1000

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

int cdecl io_write_data(unsigned addr, unsigned char val);
int cdecl io_read_data(unsigned addr, unsigned char *val);

int cdecl cb_read_mem(unsigned offset, unsigned char *val);
int cdecl cb_write_mem(unsigned offset, unsigned char val);

