#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_sid.h"
#include "otf_mem.h"
#include "otf_cff.h"
#include "otf_read.h"

/**
 ** operax
 **/
struct cff_operax *cff_parse_operax(uint8_t **pp)
{
	uint8_t *p = *pp;
	uint8_t b0 = get_u8(&p);
	struct cff_operax *r = x_calloc(1, sizeof(struct cff_operax));

	if        (b0 == 12) {
		r->v = get_u8(&p);
		r->type = OPERAX_OPERATOR;
		r->flags |= ESCAPED;
		r->bytes = 2;
	} else if (b0 <= 21) {
		r->v = b0;
		r->type = OPERAX_OPERATOR;
		r->bytes = 1;
	} else if (b0 >=  32 && b0 <= 246) {
		r->v = b0 - 139;
		r->type = OPERAX_OPERAND;
		r->bytes = 1;
	} else if (b0 >= 247 && b0 <= 250) {
		r->v =  (b0-247)*256 + get_u8(&p) + 108;
		r->type = OPERAX_OPERAND;
		r->bytes = 2;
	} else if (b0 >= 251 && b0 <= 254) {
		r->v = -(b0-251)*256 + get_u8(&p) - 108;
		r->type = OPERAX_OPERAND;
		r->bytes = 2;
	} else if (b0 == 28) {
		r->v  = get_u8(&p) << 8;
		r->v |= get_u8(&p);
		r->type = OPERAX_OPERAND;
		r->bytes = 3;
	} else if (b0 == 29) {
		r->v  = get_u8(&p) << 24;
		r->v |= get_u8(&p) << 16;
		r->v |= get_u8(&p) <<  8;
		r->v |= get_u8(&p) <<  0;
		r->type = OPERAX_OPERAND;
		r->bytes = 5;
	} else if (b0 == 30) {
		r->v = 0; /* GCC shut up */
		fprintf(stderr, "ERROR: %s (implement real value reader)\n", __func__);
		exit(1);
	} else {
		r->v = 0; /* GCC shut up */
		fprintf(stderr, "ERROR: %s\n", __func__);
		exit(1);
	}

	*pp = p;
	return r;
}

const char *cff_operator_name(struct cff_operax *op)
{
	if (op->flags & ESCAPED) {
		switch (op->v) {
		case  0: return "Copyright";
		case  3: return "UnderlinePosition";
		case  9: return "BlueScale";
		case 10: return "BlueShift";
		case 11: return "BlueFuzz";
		case 12: return "StemSnapH";
		case 13: return "StemSnapV";
		case 14: return "ForceBold";
		case 17: return "LanguageGroup";
		case 18: return "ExpansionFactor";
		case 19: return "initialRandomSeed";
		}
	} else {
		switch (op->v) {
		case  0: return "version";
		case  1: return "Notice";
		case  2: return "FullName";
		case  3: return "FamilyName";
		case  4: return "Weight";
		case  5: return "FontBBox";
		case  6: return "BlueValues";
		case  7: return "OtherBlues";
		case  8: return "FamilyBlues";
		case  9: return "FamilyOtherBlues";
		case 10: return "StdHW";
		case 11: return "StdVW";
		case 15: return "charset";
		case 16: return "Encoding";
		case 17: return "CharStrings";
		case 18: return "Private";
		case 19: return "Subrs";
		case 20: return "defaultWidthX";
		case 21: return "nominalWidthX";
		}
	}
	return "WTF";
}

/**
 ** type 2 operax
 **/
struct cff_operax *cff_type2_parse_operax(uint8_t **pp)
{
	uint8_t *p = *pp;
	uint8_t b0 = get_u8(&p);
	struct cff_operax *r = x_calloc(1, sizeof(struct cff_operax));

	if        (b0 == 12) {
		r->v = get_u8(&p);
		r->type = OPERAX_OPERATOR;
		r->flags |= ESCAPED;
		r->bytes = 2;
	} else if (b0 == 28) {
		r->v  = get_u8(&p) << 8;
		r->v |= get_u8(&p);
		r->type = OPERAX_OPERAND;
		r->bytes = 3;
	} else if (b0 <= 31) {
		r->v = b0;
		r->type = OPERAX_OPERATOR;
		r->bytes = 1;
	} else if (b0 >=  32 && b0 <= 246) {
		r->v = b0 - 139;
		r->type = OPERAX_OPERAND;
		r->bytes = 1;
	} else if (b0 >= 247 && b0 <= 250) {
		r->v =  (b0-247)*256 + get_u8(&p) + 108;
		r->type = OPERAX_OPERAND;
		r->bytes = 2;
	} else if (b0 >= 251 && b0 <= 254) {
		r->v = -(b0-251)*256 + get_u8(&p) - 108;
		r->type = OPERAX_OPERAND;
		r->bytes = 2;
	} else if (b0 == 255) {
		r->v  = get_u8(&p) << 24;
		r->v |= get_u8(&p) << 16;
		r->v |= get_u8(&p) <<  8;
		r->v |= get_u8(&p) <<  0;
		r->type = OPERAX_OPERAND;
		r->bytes = 5;
	} else {
		r->v = 0; /* GCC shut up */
		fprintf(stderr, "ERROR: %s\n", __func__);
		exit(1);
	}

	*pp = p;
	return r;
}
const char *cff_type2_operator_name(struct cff_operax *op)
{
	if (op->flags & ESCAPED) {
		switch (op->v) {
		case 0: return "Copyright";
		case 3: return "UnderlinePosition";
		}
	} else {
		switch (op->v) {
		case  0: return "RESERVED_0";
		case  1: return "hstem";
		case  2: return "RESERVED_2";
		case  3: return "vstem";
		case  4: return "vmoveto";
		case  5: return "rlineto";
		case  6: return "hlineto";
		case  7: return "vlineto";
		case  8: return "rrcurveto";
		case  9: return "RESERVED_9";
		case 10: return "callsubr";
		case 11: return "return";
		case 13: return "RESERVED_13";
		case 14: return "endchar";
		case 15: return "RESERVED_15";
		case 16: return "RESERVED_16";
		case 17: return "RESERVED_17";
		case 18: return "hstemhm";
		case 19: return "hintmask";
		case 20: return "cntrmask";
		case 21: return "rmoveto";
		case 22: return "hmoveto";
		case 23: return "vstemhm";
		case 24: return "rcurveline";
		case 25: return "rlinecurve";
		case 26: return "vvcurveto";
		case 27: return "hhcurveto";
		case 29: return "callgsubr";
		case 30: return "vhcurveto";
		case 31: return "hmcurveto";
		}
	}
	return "WTF";
}

/**
 ** index data
 **/
struct cff_index_data *cff_parse_index_data(uint8_t **pp)
{
	struct cff_index_data *r;
	uint16_t last_offset;
	uint8_t *p = *pp;
	int i;

	r = x_malloc(sizeof(struct cff_index_data));

	r->count   = get_u16(&p);
	r->offSize = get_u8(&p);

	r->offset = x_malloc((r->count+1) * sizeof(uint32_t));

	for (i = 0; i <= r->count; i++) {
		r->offset[i] = get_n(&p, r->offSize);
		last_offset = r->offset[i];
	}

	if (last_offset != 1) {
		r->data = x_malloc(last_offset);
		memcpy(r->data, p, last_offset);
		p += last_offset-1;
	}

	*pp = p;

	return r;
}
void cff_index_data_free(struct cff_index_data *idat)
{
	if (idat->data)
		free(idat->data);
	free(idat->offset);
	free(idat);
}

/**
 ** charset
 **/
struct charset *cff_parse_charset(uint8_t **pp, int nGlyphs)
{
	struct charset *r;
	uint8_t *p = *pp;
	int i;

	r = x_malloc(sizeof(struct charset));

	r->format = get_u8(&p);

	r->glyph = x_malloc((nGlyphs+1) * sizeof(uint32_t));

	if        (r->format == 0) {
		for (i = 0; i <= nGlyphs; i++)
			r->glyph[i] = get_u16(&p);
	} else if (r->format == 1) {
		uint16_t first;
		uint8_t nLeft;
		int count = 0;
		int idx = 0;

		while (count < nGlyphs-1) {
			int j;

			first = get_u16(&p);
			nLeft = get_u8(&p);

			for (j = 0; j <= nLeft; j++) {
				r->glyph[idx++] = first + j;
			}

			count += nLeft + 1;
		}
		r->glyph[idx] = 0;
	} else if (r->format == 2) {
		fprintf(stderr, "%s: charset format2 not implemented\n", __func__);
		exit(-1);
	} else {
		fprintf(stderr, "%s: bad charset format\n", __func__);
		exit(-1);
	}

	*pp = p;

	return r;
}
int cff_charset_get_sid(struct charset *r, int index)
{
	return r->glyph[index];
}

/**
 ** Private data
 **/
struct s_private *cff_parse_private_data(uint8_t **pp, int size)
{
	struct cff_operax *op;
	struct s_private *r;
	uint8_t *p = *pp;
	uint8_t *pend = p + size;

	r = x_malloc(sizeof(struct s_private));

	while (p < pend) {
		struct cff_operax *operands[48];
		int op_count = 0;
		int j;

		while (1) {
			op = cff_parse_operax(&p);
			if (op->type != OPERAX_OPERAND)
				break;
			operands[op_count++] = op;
		}

		if (op->flags & ESCAPED) {
			switch (op->v) {
			case  9: r->BlueScale         = operands[0]->v; break;
			case 10: r->BlueShift         = operands[0]->v; break;
			case 11: r->BlueFuzz          = operands[0]->v; break;
			case 12: r->StemSnapH         = operands[0]->v; break;
			case 13: r->StemSnapV         = operands[0]->v; break;
			case 14: r->ForceBold         = operands[0]->v; break;
			case 17: r->LanguageGroup     = operands[0]->v; break;
			case 18: r->ExpansionFactor   = operands[0]->v; break;
			case 19: r->initialRandomSeed = operands[0]->v; break;
			default: goto not_impl;
			}
		} else {
			switch (op->v) {
			case  6: r->BlueValues       = operands[0]->v; break;
			case  7: r->OtherBlues       = operands[0]->v; break;
			case  8: r->FamilyBlues      = operands[0]->v; break;
			case  9: r->FamilyOtherBlues = operands[0]->v; break;
			case 10: r->StdHW            = operands[0]->v; break;
			case 11: r->StdVW            = operands[0]->v; break;
			case 19: r->Subrs            = operands[0]->v; break;
			case 20: r->defaultWidthX    = operands[0]->v; break;
			case 21: r->nominalWidthX    = operands[0]->v; break;
			default: goto not_impl;
			}
		}

		for (j = 0; j < op_count; j++)
			free(operands[j]);
		free(op);
	}

	*pp = p;

	return r;

not_impl:
	fprintf(stderr, "%s: %s not implemented\n", __func__, cff_operator_name(op));
	exit(-1);
}

void cff_private_data_free(struct s_private *p)
{
	free(p);
}

/**
 ** main cff
 **/
struct cff *cff_parse(uint8_t *p)
{
	struct cff *cff = x_calloc(1, sizeof(struct cff));
	struct cff_operax *op;
	uint8_t *p0 = p;
	int i;

	/* header */
	cff->major   = get_u8(&p);
	cff->minor   = get_u8(&p);
	cff->hdrSize = get_u8(&p);
	cff->offSize = get_u8(&p);

	p = p0 + cff->hdrSize;

	cff->name_idx = cff_parse_index_data(&p);
	if (!cff->name_idx) {
		fprintf(stderr, "could not parse NAME index\n");
		goto error;
	}

	cff->font = x_calloc(cff->name_idx->count, sizeof(struct font));

	cff->top_dict_idx = cff_parse_index_data(&p);
	if (!cff->top_dict_idx) {
		fprintf(stderr, "could not parse Top DICT index\n");
		goto error;
	}
	for (i = 0; i < cff->top_dict_idx->count; i++) {
		uint8_t *dp = cff->top_dict_idx->data + cff->top_dict_idx->offset[i] - 1;

		while (dp < (cff->top_dict_idx->data + cff->top_dict_idx->offset[i+1] - 1)) {
			struct cff_operax *operands[48];
			int op_count = 0;
			int j;

			while (1) {
				op = cff_parse_operax(&dp);
				if (op->type != OPERAX_OPERAND)
					break;
				operands[op_count++] = op;
			}

			if (op->flags & ESCAPED) {
				switch (op->v) {
				case  0: cff->font[i].Copyright         = operands[0]->v; break;
				case  3: cff->font[i].UnderlinePosition = operands[0]->v; break;;
				default: goto not_impl;
				}
			} else {
				switch (op->v) {
				case  0: cff->font[i].version          = operands[0]->v; break;
				case  1: cff->font[i].Notice           = operands[0]->v; break;
				case  2: cff->font[i].FullName         = operands[0]->v; break;
				case  3: cff->font[i].FamilyName       = operands[0]->v; break;
				case  4: cff->font[i].Weight           = operands[0]->v; break;
				case  5: cff->font[i].FontBBox[0]      = operands[0]->v;
				         cff->font[i].FontBBox[1]      = operands[1]->v;
				         cff->font[i].FontBBox[2]      = operands[2]->v;
				         cff->font[i].FontBBox[3]      = operands[3]->v; break;
				case 15: cff->font[i].charset          = operands[0]->v; break;
				case 16: cff->font[i].Encoding         = operands[0]->v; break;
				case 17: cff->font[i].CharStrings      = operands[0]->v; break;
				case 18: cff->font[i].Private[0]       = operands[0]->v;
				         cff->font[i].Private[1]       = operands[1]->v; break;
				default: goto not_impl;
				}
			}

			for (j = 0; j < op_count; j++)
				free(operands[j]);
			free(op);
		}
	}

	cff->string_idx = cff_parse_index_data(&p);
	if (!cff->string_idx) {
		fprintf(stderr, "could not parse STRING index\n");
		goto error;
	}

	cff->global_subr_idx = cff_parse_index_data(&p);
	if (!cff->global_subr_idx) {
		fprintf(stderr, "could not parse Global SUBR index\n");
		goto error;
	}

	for (i = 0; i < cff->name_idx->count; i++) {
		struct font *font = &cff->font[i];
		uint8_t *p2;
		if (font->CharStrings) {
			p2 = p0 + font->CharStrings;
			font->CharStrings_idx = cff_parse_index_data(&p2);
		}
		if (font->Private[1]) {
			int offset = font->Private[1];
			int size = font->Private[0];

			p2 = p0 + offset;
			font->private_mem = x_malloc(size);
			memcpy(font->private_mem, p2, size);
			font->private_data = cff_parse_private_data(&p2, size);
			if (font->private_data->Subrs) {
				p2 = p0 + offset + font->private_data->Subrs;
				font->local_subr_idx = cff_parse_index_data(&p2);
				if (!font->local_subr_idx) {
					fprintf(stderr, "could not parse Local SUBR index\n");
					goto error;
				}
				font->subr_count = font->local_subr_idx->count;
			}
		}
		if (font->charset) {
			p2 = p0 + font->charset;
			font->charset_data = cff_parse_charset(&p2, font->CharStrings_idx->count);
		}
	}

	return cff;

error:
	cff_free(cff);
	return NULL;

not_impl:
	fprintf(stderr, "%s: %s not implemented\n", __func__, cff_operator_name(op));
	exit(-1);
}
void cff_debug(struct cff *cff, struct sid *sid)
{
	int i;

	printf("*** CFF\n");
	printf("\tmajor %d\n", cff->major);
	printf("\tminor %d\n", cff->minor);
	printf("\thdrSize %d\n", cff->hdrSize);
	printf("\toffSize %d\n", cff->offSize);

	printf("*** NAME index\n");
	printf("\tcount %d\n", cff->name_idx->count);
	printf("\toffSize %d\n", cff->name_idx->offSize);
	for (i = 0; i < cff->name_idx->count; i++) {
		int size = cff->name_idx->offset[i+1] - cff->name_idx->offset[i] + 1;
		printf("\t[%d] (%d) %s\n", i, size, cff->name_idx->data + cff->name_idx->offset[i]-1);
	}

	printf("*** Top DICT index\n");
	printf("\tcount %d\n", cff->top_dict_idx->count);
	printf("\toffSize %d\n", cff->top_dict_idx->offSize);
	for (i = 0; i < cff->top_dict_idx->count; i++) {
		int size = cff->top_dict_idx->offset[i+1] - cff->top_dict_idx->offset[i] + 1;
		uint8_t *dp = cff->top_dict_idx->data + cff->top_dict_idx->offset[i] - 1;
		int j;
		printf("\t[%d] (%d) ", i, size);
		for (j = 0; j < size; j++) {
			printf("%02x ", dp[j]);
		}
		printf("\n");
		while (dp < (cff->top_dict_idx->data + cff->top_dict_idx->offset[i+1] - 1)) {
			struct cff_operax *operands[48];
			struct cff_operax *op;
			int op_count = 0;
			int ix;

			while (1) {
				op = cff_parse_operax(&dp);
				if (op->type != OPERAX_OPERAND)
					break;
				operands[op_count++] = op;
			}

			printf("operator: (%c%d) %s | operands:", op->flags & ESCAPED ? 'x' : ' ', op->v, cff_operator_name(op));
			for (ix = 0; ix < op_count; ix++) {
				printf(" %08x", operands[ix]->v);
			}

			if (op->flags & ESCAPED) {
				switch (op->v) {
				case  0: printf(" '%s'", sid_get(sid, operands[0]->v)); break;
				case  3: printf(" (%d)",              operands[0]->v ); break;
				}
			} else {
				switch (op->v) {
				case  0:
				case  1:
				case  2:
				case  3:
				case  4: printf(" '%s'", sid_get(sid, operands[0]->v)); break;
				case 15:
				case 16:
				case 17:
				case 18: printf(" (%d)",              operands[0]->v ); break;
				}
			}

			printf("\n");
		}
		printf("\n");
	}

	printf("*** STRING index\n");
	printf("\tcount %d\n", cff->string_idx->count);
	printf("\toffSize %d\n", cff->string_idx->offSize);
	for (i = 0; i < cff->string_idx->count; i++) {
		int size = cff->string_idx->offset[i+1] - cff->string_idx->offset[i] + 1;
		uint8_t str[size];
		memcpy(str, cff->string_idx->data + cff->string_idx->offset[i]-1, size-1);
		str[size-1] = 0;
		printf("\t[%d] (%d) %s\n", i, size, str);
	}

	printf("*** Global SUBR index\n");
	printf("\tcount %d\n", cff->global_subr_idx->count);
	printf("\toffSize %d\n", cff->global_subr_idx->offSize);
	for (i = 0; i < cff->global_subr_idx->count; i++) {
		int size = cff->global_subr_idx->offset[i+1] - cff->global_subr_idx->offset[i] + 1;
		uint8_t *dp = cff->global_subr_idx->data + cff->global_subr_idx->offset[i] - 1;
		int j;
		printf("\t[%d] (%d) ", i, size);
		for (j = 0; j < size; j++) {
			printf("%02x ", dp[j]);
		}
		printf("\n");
	}

	for (i = 0; i < cff->name_idx->count; i++) {
		struct font *font = &cff->font[i];
		int j;

		printf("*** Font %d\n", i);
		printf("\tcharset     %08x\n", font->charset);
		printf("\tEncoding    %08x\n", font->Encoding);
		printf("\tCharStrings %08x\n", font->CharStrings);
		printf("\tPrivate     %08x\n", font->Private[1]);

#if 1-1
		if (font->CharStrings_idx) {
			printf("*** CharStrings index (offset %08x size %08x)\n", font->CharStrings, font->CharStrings_idx->offset[font->CharStrings_idx->count]);
			printf("\tcount %d\n", font->CharStrings_idx->count);
			printf("\toffSize %d\n", font->CharStrings_idx->offSize);
			for (j = 0; j < font->CharStrings_idx->count; j++) {
				int size = font->CharStrings_idx->offset[j+1] - font->CharStrings_idx->offset[j] + 1;
				uint8_t *dp = font->CharStrings_idx->data + font->CharStrings_idx->offset[j] - 1;
				int k;
				printf("\t[%d] (charset %d) (%d) ", j, font->charset_data->glyph[j], size);
				for (k = 0; k < size-1; k++) {
					printf("%02x ", dp[k]);
				}
				printf("\n");
				while (dp < (font->CharStrings_idx->data + font->CharStrings_idx->offset[j+1]) - 1) {
					struct cff_operax *operands[48];
					struct cff_operax *op;
					int op_count = 0;

					while (1) {
						op = cff_type2_parse_operax(&dp);
						if (op->type != OPERAX_OPERAND)
							break;
						operands[op_count++] = op;
					}

					printf("operator: (%c%d) %s | operands:", op->flags & ESCAPED ? 'x' : ' ', op->v, cff_type2_operator_name(op));
					while (op_count--)
						printf(" [%d] %08x", operands[op_count]->bytes, operands[op_count]->v);
					printf("\n");
				}
				printf("\n");
			}
		}
#endif
		if (font->charset) {
			printf("*** charset (offset %08x)\n", font->charset);
			printf("\tformat: %d\n", font->charset_data->format);
			printf("\tcount: %d\n", font->CharStrings_idx->count);
			for (j = 0; j < font->CharStrings_idx->count; j++) {
				printf("\t[%d] %d\n", j, font->charset_data->glyph[j]);
			}
		}
		if (font->private_data) {
			uint8_t *dp = font->private_mem;
			int size = font->Private[0];
			uint8_t *pend = font->private_mem + size;
			printf("*** Private data (offset %08x size %08x)\n", font->Private[1], size);
			printf("\toffset %d\n", font->Private[1]);
			printf("\tsize %d\n", size);
			printf("\t");
			for (j = 0; j < size; j++) {
				printf("%02x ", dp[j]);
			}
			printf("\n");
			while (dp < pend) {
				struct cff_operax *operands[48];
				struct cff_operax *op;
				int op_count = 0;
				int ix;
				int j;

				while (1) {
					op = cff_parse_operax(&dp);
					if (op->type != OPERAX_OPERAND)
						break;
					operands[op_count++] = op;
				}

				printf("operator: (%c%d) %s | operands:", op->flags & ESCAPED ? 'x' : ' ', op->v, cff_operator_name(op));
				for (ix = 0; ix < op_count; ix++) {
					printf(" %08x", operands[ix]->v);
				}
				printf("\n");

				for (j = 0; j < op_count; j++)
					free(operands[j]);
				free(op);
			}
			printf("\tsubr_count: %d\n", font->subr_count);
		}
	}
}
void cff_free(struct cff *cff)
{
	int i;

	if (cff->top_dict_idx)
		cff_index_data_free(cff->top_dict_idx);
	if (cff->name_idx)
		cff_index_data_free(cff->name_idx);
	if (cff->string_idx)
		cff_index_data_free(cff->string_idx);
	if (cff->global_subr_idx)
		cff_index_data_free(cff->global_subr_idx);

	for (i = 0; i < cff->name_idx->count; i++) {
		struct font *font = &cff->font[i];

		if (font->CharStrings_idx)
			cff_index_data_free(font->CharStrings_idx);
		if (font->private_data)
			cff_private_data_free(font->private_data);
	}
}
