#include <inttypes.h>

struct cmap_format_6 {
	uint16_t length;
	uint16_t language;
	uint16_t firstCode;
	uint16_t entryCount;
	uint16_t *glyphIdArray;
};

struct cmap_format_4 {
	uint16_t length;
	uint16_t language;
	uint16_t segCountX2;
	uint16_t searchRange;
	uint16_t entrySelector;
	uint16_t rangeShift;
	uint16_t *endCount;
	uint16_t reservedPad;
	uint16_t *startCount;
	int16_t  *idDelta;
	uint16_t *idRangeOffset;
	uint16_t *glyphIdArray;
};

struct cmap_table {
	uint16_t platformID;
	uint16_t encodingID;
	uint32_t offset;

	uint16_t format;
	struct cmap_format_4 *fmt4;
	struct cmap_format_6 *fmt6;
};

struct cmap {
	uint16_t version;
	uint16_t numTables;

	struct cmap_table *table;
};
struct cmap *cmap_parse(uint8_t *p);
void         cmap_debug(struct cmap *cmap);
void         cmap_free(struct cmap *cmap);
int          cmap_get_glyph_index(struct cmap *cmap, uint16_t platformID, uint16_t encodingID, uint16_t glyph);
