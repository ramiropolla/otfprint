#ifndef OTF_HMTX_H
#define OTF_HMTX_H

#include <inttypes.h>

#include "otf_hhea.h"

struct longHorMetric {
	uint16_t advanceWidth;
	int16_t  lsb;
};

struct hmtx {
	struct longHorMetric *longHorMetric;
	struct hhea *hhea;
};
struct hmtx *hmtx_parse(uint8_t *p, struct hhea *hhea);
void         hmtx_debug(struct hmtx *hmtx);
void         hmtx_free(struct hmtx *hmtx);
int          hmtx_get_width(struct hmtx *hmtx, int gid);
int          hmtx_get_lsb(struct hmtx *hmtx, int gid);

#endif /* OTF_HMTX_H */
