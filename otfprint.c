#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_hmtx.h"
#include "otf_hhea.h"
#include "otf_kern.h"
#include "otf_cmap.h"
#include "otf_gpos.h"
#include "otf_cff.h"
#include "otf_vm.h"
#include "otf_sid.h"
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
	struct gpos *gpos = NULL;
	struct hhea *hhea = NULL;
	struct hmtx *hmtx = NULL;
	struct sid *strings = NULL;
	struct cff *cff = NULL;
	struct otf_header *h = NULL;
	struct otf_table *t = NULL;
	struct glyph *glyph_list[0x100] = { 0 };
	uint8_t *data = NULL;
	int x_offset = 0;
	int last_glyph = 0;
	char *pp;
	uint8_t *p;
	long int fsize;
	FILE *fp = NULL;
	int i;

	if (argc < 3) {
		fprintf(stderr, "usage: %s <font> <text>\n", argv[0]);
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

#if 0
	printf("*** HEADER\n");
	printf("version      : %08x\n", h->version);
	printf("numTables    : %04x\n", h->numTables);
	printf("searchRange  : %04x\n", h->searchRange);
	printf("entrySelector: %04x\n", h->entrySelector);
	printf("rangeShift   : %04x\n", h->rangeShift);
#endif

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
#if 0
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
//			kern_debug(kern);
		} else if (t[i].tag == 0x636d6170) {
			cmap = cmap_parse(data+t[i].offset);
//			cmap_debug(cmap);
		} else if (t[i].tag == 0x43464620) {
			cff = cff_parse(data+t[i].offset);
			strings = sid_new(cff->string_idx);
//			cff_debug(cff, strings);
		} else if (t[i].tag == 0x47504f53) {
			gpos = gpos_parse(data+t[i].offset);
//			gpos_debug(gpos);
		} else if (t[i].tag == 0x68686561) {
			hhea = hhea_parse(data+t[i].offset);
//			hhea_debug(hhea);
		} else if (t[i].tag == 0x686d7478) {
			hmtx = hmtx_parse(data+t[i].offset, hhea);
//			hmtx_debug(hmtx);
		}
	}

	/* print svg header */
	printf("<?xml version=\"1.0\" standalone=\"no\"?>\n");
	printf("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
	printf("<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" version=\"1.1\">\n");

	/* populate glyph_list with glyphs */
	for (pp = argv[2]; *pp; pp++) {
		if (!glyph_list[(int)*pp]) {
			int gid = cmap_get_gid(cmap, 3, 1, *pp);
			glyph_list[(int)*pp] = otf_vm(cff, &cff->font[0], gid);
		}
	}

	/* write glyphs as defs */
	printf("<defs>\n");
	for (i = 0; i < 0x100; i++)
		if (glyph_list[i] && i != ' ')
			printf(" <path id=\"%c\" d=\"%s\" />\n", i, glyph_list[i]->path);
	printf("</defs>\n");

	/* get full text width */
	for (pp = argv[2]; *pp; pp++) {
		struct glyph *glyph = glyph_list[(int)*pp];
		int gid = cmap_get_gid(cmap, 3, 1, *pp) + 1;
		if (last_glyph)
			x_offset += kern_get(kern, last_glyph, gid);
		x_offset += glyph->width;
		last_glyph = gid;
	}

	/* print group */
	printf("<g transform=\"translate(%g, %d)\">\n", x_offset/2., hhea->Ascender);
	printf("<g transform=\"translate(-%g) scale(1,-1)\">\n", x_offset/2.);
	x_offset = 0;
	last_glyph = 0;
	for (pp = argv[2]; *pp; pp++) {
		struct glyph *glyph = glyph_list[(int)*pp];
		int xtra = 0;
		int gid = cmap_get_gid(cmap, 3, 1, *pp) + 1;
		if (last_glyph)
			xtra = kern_get(kern, last_glyph, gid);
//		printf("kern %d lsb %d\n", xtra, hmtx_get_lsb(hmtx, gid));
		x_offset += xtra;
		if (*pp != ' ')
			printf(" <use x=\"%d\" xlink:href=\"#%c\" />\n", x_offset, *pp);
		x_offset += glyph->width;
		last_glyph = gid;
	}
	printf("</g>\n");
	printf("</g>\n");

	/* print svg trailer */
	printf("</svg>\n");

the_end:
	if (hmtx)
		hmtx_free(hmtx);
	if (hhea)
		hhea_free(hhea);
	if (gpos)
		gpos_free(gpos);
	if (kern)
		kern_free(kern);
	if (strings)
		sid_free(strings);
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
