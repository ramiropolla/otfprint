#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_kern.h"
#include "otf_cmap.h"
#include "otf_cff.h"
#include "otf_vm.h"
#include "otf_read.h"

struct otf_header {
	uint32_t version;
	uint16_t numTables;
	uint16_t searchRange;
	uint16_t entrySelector;
	uint16_t rangeShift;
};
struct otf_table {
	uint32_t tag;
	uint32_t checkSum;
	uint32_t offset;
	uint32_t length;
};

static struct otf_header *otf_read_header(uint8_t **pp)
{
	struct otf_header *h = malloc(sizeof(struct otf_header));
	if (!h) {
		fprintf(stderr, "%s no mem (line: %d)\n", __func__, __LINE__);
		exit(-1);
	}
	h->version       = get_u32(pp);
	h->numTables     = get_u16(pp);
	h->searchRange   = get_u16(pp);
	h->entrySelector = get_u16(pp);
	h->rangeShift    = get_u16(pp);
	return h;
}

int main(int argc, char *argv[])
{
	struct kern *kern = NULL;
	struct cmap *cmap = NULL;
	struct cff *cff = NULL;
	struct otf_header *h = NULL;
	struct otf_table *t = NULL;
	uint8_t *data = NULL;
	uint8_t *p;
	long int fsize;
	FILE *fp = NULL;
	int i;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <font>\n", argv[0]);
		return -1;
	}

	fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "could not open '%s'\n", argv[1]);
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data = malloc(fsize);
	if (!data) {
		fprintf(stderr, "could not alloc %ld bytes\n", fsize);
		goto the_end;
	}

	if (!fread(data, fsize, 1, fp)) {
		fprintf(stderr, "could not read %ld bytes\n", fsize);
		goto the_end;
	}

	p = data;

	h = otf_read_header(&p);
	if (!h) {
		fprintf(stderr, "could not malloc header\n");
		goto the_end;
	}

	printf("*** HEADER\n");
	printf("version      : %08x\n", h->version);
	printf("numTables    : %04x\n", h->numTables);
	printf("searchRange  : %04x\n", h->searchRange);
	printf("entrySelector: %04x\n", h->entrySelector);
	printf("rangeShift   : %04x\n", h->rangeShift);

	t = malloc(sizeof(struct otf_table) * h->numTables);
	if (!t) {
		fprintf(stderr, "could not malloc tables\n");
		goto the_end;
	}

	for (i = 0; i < h->numTables; i++) {
		t[i].tag      = get_u32(&p);
		t[i].checkSum = get_u32(&p);
		t[i].offset   = get_u32(&p);
		t[i].length   = get_u32(&p);
#if 1
		{
			uint8_t s[5];
			s[0] = (t[i].tag & 0xff000000) >> 24;
			s[1] = (t[i].tag & 0x00ff0000) >> 16;
			s[2] = (t[i].tag & 0x0000ff00) >>  8;
			s[3] = (t[i].tag & 0x000000ff) >>  0;
			s[4] = 0;
			printf("*** TABLE %d\n", i);
			printf("\ttag     : %s [%08x]\n", s, t[i].tag);
			printf("\tcheckSum: %08x\n", t[i].checkSum);
			printf("\toffset  : %08x\n", t[i].offset);
			printf("\tlength  : %d\n", t[i].length);
		}
#endif
	}

	for (i = 0; i < h->numTables; i++) {
		if        (t[i].tag == 0x6b65726e) {
			kern = kern_parse(data+t[i].offset);
			kern_debug(kern);
		} else if (t[i].tag == 0x636d6170) {
			cmap = cmap_parse(data+t[i].offset);
			cmap_debug(cmap);
		} else if (t[i].tag == 0x43464620) {
			cff = cff_parse(data+t[i].offset);
			cff_debug(cff);
		}
	}

	{
		int idx  = cmap_get_glyph_index(cmap, 3, 1, 't');
		int cset = cff_get_charset(cff->font[0].charset_data, idx);
		printf("idx %d %d\n", idx, cset);
		exit(-1);
	}

	otf_vm(cff, &cff->font[0], 'A');

the_end:
	if (kern)
		kern_free(kern);
	if (h)
		free(h);
	if (t)
		free(t);
	if (data)
		free(data);
	if (fp)
		fclose(fp);

	return 0;
}
