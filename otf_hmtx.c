#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_mem.h"
#include "otf_hmtx.h"
#include "otf_read.h"

struct hmtx *hmtx_parse(uint8_t *p, struct hhea *hhea)
{
	struct hmtx *hmtx = calloc(1, sizeof(struct hmtx));
	int i;

	hmtx->longHorMetric = x_calloc(hhea->numberOfHMetrics, sizeof(struct longHorMetric));
	for (i = 0; i < hhea->numberOfHMetrics; i++) {
		hmtx->longHorMetric[i].advanceWidth = get_u16(&p);
		hmtx->longHorMetric[i].lsb          = get_s16(&p);
	}

	hmtx->hhea = hhea;

	return hmtx;
}
void hmtx_debug(struct hmtx *hmtx)
{
	struct hhea *hhea = hmtx->hhea;
	int i;

	printf("*** HMTX\n");
	for (i = 0; i < hhea->numberOfHMetrics; i++)
		printf("\t[%3d] %d, %d\n", i, hmtx->longHorMetric[i].advanceWidth, hmtx->longHorMetric[i].lsb);
}
void hmtx_free(struct hmtx *hmtx)
{
	free(hmtx);
}
int hmtx_get_width(struct hmtx *hmtx, int gid)
{
	return hmtx->longHorMetric[gid].advanceWidth;
}
int hmtx_get_lsb(struct hmtx *hmtx, int gid)
{
	return hmtx->longHorMetric[gid].lsb;
}
