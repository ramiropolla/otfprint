#ifndef OTF_HHEA_H
#define OTF_HHEA_H

#include <inttypes.h>

struct hhea {
	uint32_t version;

	/* (U) FWORD */
	int16_t  Ascender;
	int16_t  Descender;
	int16_t  LineGap;
	uint16_t advanceWidthMax;
	int16_t  minLeftSideBearing;
	int16_t  minRightSideBearing;
	int16_t  xMaxExtent;

	/* (U)SHORT */
	int16_t  caretSlopeRise;
	int16_t  caretSlopeRun;
	int16_t  caretOffset;
	int16_t  reserved0;
	int16_t  reserved1;
	int16_t  reserved2;
	int16_t  reserved3;
	int16_t  metricDataFormat;
	uint16_t numberOfHMetrics;
};
struct hhea *hhea_parse(uint8_t *p);
void         hhea_debug(struct hhea *hhea);
void         hhea_free(struct hhea *hhea);

#endif /* OTF_HHEA_H */
