#include <inttypes.h>

/** Operators and Operands */
enum e_operax {
	OPERAX_OPERAND,
	OPERAX_OPERATOR,
};
#define ESCAPED (1<<0)
struct cff_operax {
	enum e_operax type;
	int flags;
	int32_t v;
	double d;
	int bytes;
};
struct cff_operax *cff_parse_operax(uint8_t **pp);
const char *       cff_operator_name(struct cff_operax *op);

struct cff_operax *cff_type2_parse_operax(uint8_t **pp);
const char *       cff_type2_operator_name(struct cff_operax *op);

/** Index data */
struct cff_index_data {
	uint16_t count;
	uint32_t offSize;
	uint32_t *offset;
	uint8_t  *data;
};

struct cff_index_data *cff_parse_index_data(uint8_t **pp);
void                   cff_index_data_free(struct cff_index_data *idat);

/** charset */
struct charset {
	uint8_t format;
	uint32_t *glyph;
};
struct charset *cff_parse_charset(uint8_t **pp, int nGlyphs);
int             cff_get_charset(struct charset *r, int index);

/** Private */
struct s_private {
	uint32_t BlueValues;
	uint32_t OtherBlues;
	uint32_t FamilyBlues;
	uint32_t FamilyOtherBlues;
	uint32_t BlueScale;
	uint32_t BlueShift;
	uint32_t BlueFuzz;
	uint32_t StdHW;
	uint32_t StdVW;
	uint32_t StemSnapH;
	uint32_t StemSnapV;
	uint32_t ForceBold;
	uint32_t LanguageGroup;
	uint32_t ExpansionFactor;
	uint32_t initialRandomSeed;
	uint32_t Subrs; /* offset */
	uint32_t defaultWidthX;
	uint32_t nominalWidthX;
};

/** per-font struct */
struct font {
	uint32_t version; /* SID */
	uint32_t Notice; /* SID */
	uint32_t Copyright; /* SID */
	uint32_t FullName; /* SID */
	uint32_t FamilyName; /* SID */
	uint32_t Weight; /* SID */

	uint32_t UnderlinePosition; /* number */

	uint32_t charset; /* offset */
	uint32_t Encoding; /* offset */
	uint32_t CharStrings; /* offset */

	uint32_t FontBBox[4]; /* array */
	uint32_t Private[2]; /* size, offset */

	struct cff_index_data *CharStrings_idx;
	struct cff_index_data *Private_idx;
	struct s_private *private_data;
	uint8_t *private_mem;
	struct charset *charset_data;

	struct cff_index_data *local_subr_idx;

	int subr_count;
};

/** main cff */
struct cff {
	uint8_t major;
	uint8_t minor;
	uint8_t hdrSize;
	uint8_t offSize;
	struct cff_index_data *name_idx;
	struct cff_index_data *top_dict_idx;
	struct cff_index_data *string_idx;
	struct cff_index_data *global_subr_idx;

	struct font *font;
};
struct cff *cff_parse(uint8_t *p);
void        cff_debug(struct cff *cff);
void        cff_free(struct cff *cff);
