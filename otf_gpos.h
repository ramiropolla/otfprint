#ifndef OTF_GPOS_H
#define OTF_GPOS_H

#include <inttypes.h>

struct langSysTable {
	uint16_t LookupOrder;
	uint16_t ReqFeatureIndex;
	uint16_t FeatureCount;
	uint16_t *FeatureIndex;
};
#if 0
struct langSysRecord {
	uint32_t LangSysTag;
	uint16_t LangSys;
};
#endif
struct scriptTable {
	uint16_t DefaultLangSys;
	uint16_t LangSysCount;

	struct langSysTable defaultLangSys;
//	struct langSysRecord *langSysRecord;
};
struct scriptRecord {
	uint32_t ScriptTag;
	uint16_t Script;

	struct scriptTable scriptTable;
};
struct scriptList {
	uint16_t scriptCount;
	struct scriptRecord *scriptRecord;
};

struct featureTable {
	uint16_t FeatureParams;
	uint16_t LookupCount;
	uint16_t *LookupListIndex;
};
struct featureRecord {
	uint32_t FeatureTag;
	uint16_t Feature;

	struct featureTable featureTable;
};
struct featureList {
	uint16_t FeatureCount;
	struct featureRecord *FeatureRecord;
};

struct RangeRecord {
	uint16_t Start;
	uint16_t End;
	uint16_t StartCoverageIndex;
};
struct Coverage {
	uint16_t CoverageFormat;
	uint16_t RangeCount;
	struct RangeRecord *RangeRecord;
};
struct PairValueRecord {
	uint16_t SecondGlyph;
//	uint16_t XAdvance;
	int16_t ValueRecord1;
	uint16_t ValueRecord2;
};
struct PairSetTable {
	uint16_t PairValueCount;
	struct PairValueRecord *PairValueRecord;
};
struct subTable {
	uint16_t PosFormat;
	uint16_t Coverage;
	uint16_t ValueFormat1;
	uint16_t ValueFormat2;
	uint16_t PairSetCount;
	uint16_t *PairSetOffset;

	struct PairSetTable *pairSetTable;
	struct Coverage coverage;
};
struct lookupTable {
	uint16_t LookupType;
	uint16_t LookupFlag;
	uint16_t SubTableCount;
	uint16_t *SubTable;
	uint16_t MarkFilteringSet;

	struct subTable *subTable;
};
struct lookupList {
	uint16_t LookupCount;
	uint16_t *Lookup;
	struct lookupTable *lookupTable;
};

struct gpos {
	uint32_t version;
	uint16_t ScriptList;
	uint16_t FeatureList;
	uint16_t LookupList;

	struct scriptList *scriptList;
	struct featureList *featureList;
	struct lookupList *lookupList;
};
struct gpos *gpos_parse(uint8_t *p);
void         gpos_debug(struct gpos *gpos);
void         gpos_free(struct gpos *gpos);
int          gpos_kern_get(struct gpos *gpos, uint16_t l, uint16_t r);
void         gpos_kern_dump(struct gpos *gpos);

#endif /* OTF_GPOS_H */
