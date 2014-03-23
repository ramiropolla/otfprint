#ifndef OTF_KERN_H
#define OTF_KERN_H

#include <inttypes.h>


struct kern_format_0_pair {
	uint16_t left;
	uint16_t right;
	int16_t value; /* FWORD, int16_t in smallest em space whatever */
};
struct kern_format_0 {
	uint16_t nPairs;
	uint16_t searchRange;
	uint16_t entrySelector;
	uint16_t rangeShift;

	struct kern_format_0_pair *pair;
};

struct kern_table {
	uint16_t version;
	uint16_t length;
	uint16_t coverage;

	struct kern_format_0 *fmt0;
};

struct kern {
	uint16_t version;
	uint16_t nTables;

	struct kern_table *table;
};
struct kern *kern_parse(uint8_t *p);
int          kern_get(struct kern *kern, uint16_t l, uint16_t r);
void         kern_debug(struct kern *kern);
void         kern_free(struct kern *kern);

#endif /* OTF_KERN_H */
