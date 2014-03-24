#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_mem.h"
#include "otf_hhea.h"
#include "otf_read.h"

struct hhea *hhea_parse(uint8_t *p)
{
	struct hhea *hhea = calloc(1, sizeof(struct hhea));

	hhea->version = get_u32(&p);

	hhea->Ascender            = get_s16(&p);
	hhea->Descender           = get_s16(&p);
	hhea->LineGap             = get_s16(&p);
	hhea->advanceWidthMax     = get_u16(&p);
	hhea->minLeftSideBearing  = get_s16(&p);
	hhea->minRightSideBearing = get_s16(&p);
	hhea->xMaxExtent          = get_s16(&p);

	hhea->caretSlopeRise   = get_s16(&p);
	hhea->caretSlopeRun    = get_s16(&p);
	hhea->caretOffset      = get_s16(&p);
	hhea->reserved0        = get_s16(&p);
	hhea->reserved1        = get_s16(&p);
	hhea->reserved2        = get_s16(&p);
	hhea->reserved3        = get_s16(&p);
	hhea->metricDataFormat = get_s16(&p);
	hhea->numberOfHMetrics = get_u16(&p);

	return hhea;
}
void hhea_debug(struct hhea *hhea)
{
	printf("*** HHEA\n");
	printf("\tversion %08x\n", hhea->version);

	printf("\tAscender:            %d\n", hhea->Ascender);
	printf("\tDescender:           %d\n", hhea->Descender);
	printf("\tLineGap:             %d\n", hhea->LineGap);
	printf("\tadvanceWidthMax:     %d\n", hhea->advanceWidthMax);
	printf("\tminLeftSideBearing:  %d\n", hhea->minLeftSideBearing);
	printf("\tminRightSideBearing: %d\n", hhea->minRightSideBearing);
	printf("\txMaxExtent:          %d\n", hhea->xMaxExtent);

	/* (U)SHORT */
	printf("\tcaretSlopeRise:   %d\n", hhea->caretSlopeRise);
	printf("\tcaretSlopeRun:    %d\n", hhea->caretSlopeRun);
	printf("\tcaretOffset:      %d\n", hhea->caretOffset);
	printf("\treserved0:        %d\n", hhea->reserved0);
	printf("\treserved1:        %d\n", hhea->reserved1);
	printf("\treserved2:        %d\n", hhea->reserved2);
	printf("\treserved3:        %d\n", hhea->reserved3);
	printf("\tmetricDataFormat: %d\n", hhea->metricDataFormat);
	printf("\tnumberOfHMetrics: %d\n", hhea->numberOfHMetrics);
}
void hhea_free(struct hhea *hhea)
{
	free(hhea);
}
