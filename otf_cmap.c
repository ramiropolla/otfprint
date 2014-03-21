#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_cmap.h"
#include "otf_read.h"
#include "otf_mem.h"

/**
 ** main cmap
 **/
struct cmap *cmap_parse(uint8_t *p)
{
	struct cmap *cmap = x_calloc(1, sizeof(struct cmap));
	uint8_t *p0 = p;
	int i;

	cmap->version   = get_u16(&p);
	cmap->numTables = get_u16(&p);

	cmap->table = x_calloc(cmap->numTables, sizeof(struct cmap_table));

	for (i = 0; i < cmap->numTables; i++) {
		struct cmap_table *table = &cmap->table[i];

		table->platformID = get_u16(&p);
		table->encodingID = get_u16(&p);
		table->offset     = get_u32(&p);
	}

	for (i = 0; i < cmap->numTables; i++) {
		struct cmap_table *table = &cmap->table[i];
		uint8_t *dp = p0 + table->offset;
		int j;

		table->format = get_u16(&dp);

		if        (table->format == 4) {
			struct cmap_format_4 *fmt4 = x_calloc(1, sizeof(struct cmap_format_4));
			uint16_t segCount;
			int nGlyphs;

			fmt4->length        = get_u16(&dp);
			fmt4->language      = get_u16(&dp);
			fmt4->segCountX2    = get_u16(&dp);

			segCount = fmt4->segCountX2 / 2;

			fmt4->searchRange   = get_u16(&dp);
			fmt4->entrySelector = get_u16(&dp);
			fmt4->rangeShift    = get_u16(&dp);

			fmt4->endCount      = x_calloc(segCount, sizeof(uint16_t));
			fmt4->startCount    = x_calloc(segCount, sizeof(uint16_t));
			fmt4->idDelta       = x_calloc(segCount, sizeof(uint16_t));
			fmt4->idRangeOffset = x_calloc(segCount, sizeof(uint16_t));

			for (j = 0; j < segCount; j++) fmt4->endCount     [j] = get_u16(&dp);
			fmt4->reservedPad                                     = get_u16(&dp);
			for (j = 0; j < segCount; j++) fmt4->startCount   [j] = get_u16(&dp);
			for (j = 0; j < segCount; j++) fmt4->idDelta      [j] = get_s16(&dp);
			for (j = 0; j < segCount; j++) fmt4->idRangeOffset[j] = get_u16(&dp);

			nGlyphs = (fmt4->length - (4 * segCount + 8) * sizeof(uint16_t)) / sizeof(uint16_t);
			fmt4->glyphIdArray  = x_calloc(nGlyphs, sizeof(uint16_t));
			for (j = 0; j < nGlyphs; j++) fmt4->glyphIdArray [j] = get_u16(&dp);

			table->fmt4 = fmt4;
		} else if (table->format == 6) {
			struct cmap_format_6 *fmt6 = x_calloc(1, sizeof(struct cmap_format_6));

			fmt6->length     = get_u16(&dp);
			fmt6->language   = get_u16(&dp);
			fmt6->firstCode  = get_u16(&dp);
			fmt6->entryCount = get_u16(&dp);

			fmt6->glyphIdArray  = x_calloc(fmt6->entryCount, sizeof(uint16_t));
			for (j = 0; j < fmt6->entryCount; j++) fmt6->glyphIdArray [j] = get_u16(&dp);

			table->fmt6 = fmt6;
		} else {
			fprintf(stderr, "CMAP: (%s) only formats 4 and 6 are supported\n", __func__);
			exit(-1);
		}
	}

	return cmap;
}
void cmap_debug(struct cmap *cmap)
{
	int i;

	printf("*** cmap\n");
	printf("\tversion   %d\n", cmap->version);
	printf("\tnumTables %d\n", cmap->numTables);

	for (i = 0; i < cmap->numTables; i++) {
		struct cmap_table *table = &cmap->table[i];

		printf(" ** table[%d] decl\n", i);
		printf("\tplatformID %d\n", table->platformID);
		printf("\tencodingID %d\n", table->encodingID);
		printf("\toffset     %d\n", table->offset);
	}

	for (i = 0; i < cmap->numTables; i++) {
		struct cmap_table *table = &cmap->table[i];
		int j;

		printf(" ** table[%d] impl\n", i);

		if        (table->format == 4) {
			struct cmap_format_4 *fmt4 = table->fmt4;
			uint16_t segCount;
			int nGlyphs;

			segCount = fmt4->segCountX2 / 2;
			nGlyphs = (fmt4->length - (4 * segCount + 8) * sizeof(uint16_t)) / sizeof(uint16_t);

			printf("\tlength        %d\n", fmt4->length);
			printf("\tlanguage      %d\n", fmt4->language);
			printf("\tsegCountX2    %d\n", fmt4->segCountX2);
			printf("\tsearchRange   %d\n", fmt4->searchRange);
			printf("\tentrySelector %d\n", fmt4->entrySelector);
			printf("\trangeShift    %d\n", fmt4->rangeShift);
			printf("\treservedPad   %d\n", fmt4->reservedPad);
			for (j = 0; j < segCount; j++) {
				printf("\t[%3d] startCount(%04x) endCount(%04x) idDelta(% 5d)",
				       j, fmt4->startCount[j], fmt4->endCount[j], fmt4->idDelta[j]);
				printf(" idRangeOffset(%04x)", fmt4->idRangeOffset[j]);
				if (!fmt4->idRangeOffset[j])
					printf(" %d -> %d", fmt4->startCount[j] + fmt4->idDelta[j], fmt4->endCount[j] + fmt4->idDelta[j]);
				printf("\n");
			}
			for (j = 0; j < nGlyphs; j++)
				printf("\tglyphIdArray[%3d] %04x\n", j, fmt4->glyphIdArray[j]);
		} else if (table->format == 6) {
			struct cmap_format_6 *fmt6 = table->fmt6;

			printf("\tlength     %d\n", fmt6->length);
			printf("\tlanguage   %d\n", fmt6->language);
			printf("\tfirstCode  %d\n", fmt6->firstCode);
			printf("\tentryCount %d\n", fmt6->entryCount);
			for (j = 0; j < fmt6->entryCount; j++)
			     printf("\t[%3d] glyphIdArray %04x\n", j, fmt6->glyphIdArray[j]);
		}
	}
}
void cmap_free(struct cmap *cmap)
{
	free(cmap);
}
int cmap_get_gid(struct cmap *cmap, uint16_t platformID, uint16_t encodingID, uint16_t glyph)
{
	int i;

	for (i = 0; i < cmap->numTables; i++) {
		struct cmap_table *table = &cmap->table[i];
		int j;

		if (table->platformID != platformID || table->encodingID != encodingID)
			continue;

		if        (table->format == 4) {
			struct cmap_format_4 *fmt4 = table->fmt4;
			uint16_t segCount;

			segCount = fmt4->segCountX2 / 2;

			for (j = 0; j < segCount; j++) {
				if (glyph >= fmt4->startCount[j] && glyph <= fmt4->endCount[j]) {
					if (!fmt4->idRangeOffset[j]) {
						return fmt4->idDelta[j] + glyph - 1;
					} else {
						int u_off = glyph - fmt4->startCount[j];
						int idx = (fmt4->idRangeOffset[j]/2)-(segCount-j);
						return fmt4->glyphIdArray[idx+u_off]-1;
					}
				}
			}
		} else if (table->format == 6) {
			/* TODO */
			fprintf(stderr, "CMAP: (%s) format 6 not implemented\n", __func__);
			exit(-1);
		} else {
			fprintf(stderr, "CMAP: (%s) only format 4 is supported\n", __func__);
			exit(-1);
		}

		fprintf(stderr, "CMAP: (%s) glyph not found\n", __func__);
		return -1;
	}

	fprintf(stderr, "%s: bad platformID and encodingID\n", __func__);
	exit(-1);
	return 0;
}
