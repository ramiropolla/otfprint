#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_mem.h"
#include "otf_kern.h"
#include "otf_read.h"

/**
 ** main kern
 **/
struct kern *kern_parse(uint8_t *p)
{
	struct kern *kern = calloc(1, sizeof(struct kern));
	int i;

	kern->version = get_u16(&p);
	kern->nTables = get_u16(&p);

	kern->table = x_calloc(kern->nTables, sizeof(struct kern_table));

	for (i = 0; i < kern->nTables; i++) {
		struct kern_table *table = &kern->table[i];
		struct kern_format_0 *fmt0 = NULL;
		int j;

		table->version  = get_u16(&p);
		table->length   = get_u16(&p);
		table->coverage = get_u16(&p);

		if (table->version != 0) {
			fprintf(stderr, "KERN: (%s) only format 0 is supported\n", __func__);
			exit(-1);
		}

		fmt0 = x_malloc(sizeof(struct kern_format_0));
		table->fmt0 = fmt0;

		fmt0->nPairs        = get_u16(&p);
		fmt0->searchRange   = get_u16(&p);
		fmt0->entrySelector = get_u16(&p);
		fmt0->rangeShift    = get_u16(&p);

		fmt0->pair = x_calloc(fmt0->nPairs, sizeof(struct kern_format_0_pair));

		for (j = 0; j < fmt0->nPairs; j++) {
			fmt0->pair[j].left  = get_u16(&p);
			fmt0->pair[j].right = get_u16(&p);
			fmt0->pair[j].value = get_s16(&p);
		}
	}

	return kern;
}
int kern_get(struct kern *kern, uint16_t l, uint16_t r)
{
	int i;

	for (i = 0; i < kern->nTables; i++) {
		struct kern_table *table = &kern->table[i];
		struct kern_format_0 *fmt0 = table->fmt0;
		int j;

		for (j = 0; j < fmt0->nPairs; j++)
			if (l == fmt0->pair[j].left && r == fmt0->pair[j].right)
				return fmt0->pair[j].value;
	}

	return 0;
}
void kern_debug(struct kern *kern)
{
	int i;

	printf("*** KERN\n");
	printf("\tversion %d\n", kern->version);
	printf("\tnTables %d\n", kern->nTables);

	for (i = 0; i < kern->nTables; i++) {
		struct kern_table *table = &kern->table[i];
		struct kern_format_0 *fmt0 = table->fmt0;
		int j;

		printf(" ** table[%d]\n", i);
		printf("\tversion  %d\n", table->version);
		printf("\tlength   %d\n", table->length);
		printf("\tcoverage %d\n", table->coverage);

		/* only version 0 supported */

		printf("\tnPairs        %d\n", fmt0->nPairs);
		printf("\tsearchRange   %d\n", fmt0->searchRange);
		printf("\tentrySelector %d\n", fmt0->entrySelector);
		printf("\trangeShift    %d\n", fmt0->rangeShift);

		for (j = 0; j < fmt0->nPairs; j++)
			printf("\t[%3d] [%3d] %d\n", fmt0->pair[j].left, fmt0->pair[j].right, fmt0->pair[j].value);
	}
}
void kern_free(struct kern *kern)
{
	free(kern);
}
void kern_dump(struct kern *kern)
{
	int i;

	for (i = 0; i < kern->nTables; i++) {
		struct kern_table *table = &kern->table[i];
		struct kern_format_0 *fmt0 = table->fmt0;
		int j;

		for (j = 0; j < fmt0->nPairs; j++)
			printf("\t[%3d] [%3d] %d\n", fmt0->pair[j].left, fmt0->pair[j].right, fmt0->pair[j].value);
	}
}
