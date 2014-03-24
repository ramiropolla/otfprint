#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "otf_gpos.h"
#include "otf_read.h"
#include "otf_mem.h"

static void langSysTable_parse(struct langSysTable *r, uint8_t **pp)
{
	uint8_t *p = *pp;
	int i;

	r->LookupOrder     = get_u16(&p);
	r->ReqFeatureIndex = get_u16(&p);
	r->FeatureCount    = get_u16(&p);

	r->FeatureIndex = x_calloc(r->FeatureCount, sizeof(uint16_t));
	for (i = 0; i < r->FeatureCount; i++)
		r->FeatureIndex[i] = get_u16(&p);

	*pp = p;
}

static void scriptTable_parse(struct scriptTable *r, uint8_t **pp)
{
	uint8_t *p = *pp;

	r->DefaultLangSys = get_u16(&p);
	r->LangSysCount   = get_u16(&p);

	langSysTable_parse(&r->defaultLangSys, &p);

	if (r->LangSysCount) {
		fprintf(stderr, "%s langsyscount not implemented\n", __func__);
		exit(-1);
	}

	*pp = p;
}

static struct scriptList *scriptList_parse(uint8_t **pp)
{
	struct scriptList *r = x_malloc(sizeof(struct scriptList));
	uint8_t *p = *pp;
	uint8_t *p0 = p;
	int i;

	r->scriptCount = get_u16(&p);

	r->scriptRecord = x_calloc(r->scriptCount, sizeof(struct scriptRecord));

	for (i = 0; i < r->scriptCount; i++) {
		uint8_t *dp;
		r->scriptRecord[i].ScriptTag = get_u32(&p);
		r->scriptRecord[i].Script    = get_u16(&p);
		dp = p0 + r->scriptRecord[i].Script;
		scriptTable_parse(&r->scriptRecord[i].scriptTable, &dp);
	}

	*pp = p;
	return r;
}

static void featureTable_parse(struct featureTable *r, uint8_t **pp)
{
	uint8_t *p = *pp;

	r->FeatureParams = get_u16(&p);
	r->LookupCount   = get_u16(&p);

	if (r->LookupCount) {
		int i;
		r->LookupListIndex = x_calloc(r->LookupCount, sizeof(uint16_t));
		for (i = 0; i < r->LookupCount; i++)
			r->LookupListIndex[i] = get_u16(&p);
	}

	*pp = p;
}
static struct featureList *featureList_parse(uint8_t **pp)
{
	struct featureList *r = x_malloc(sizeof(struct featureList));
	uint8_t *p = *pp;
	uint8_t *p0 = p;
	int i;

	r->FeatureCount = get_u16(&p);

	r->FeatureRecord = x_calloc(r->FeatureCount, sizeof(struct featureRecord));

	for (i = 0; i < r->FeatureCount; i++) {
		uint8_t *dp;
		r->FeatureRecord[i].FeatureTag = get_u32(&p);
		r->FeatureRecord[i].Feature    = get_u16(&p);
		dp = p0 + r->FeatureRecord[i].Feature;
		featureTable_parse(&r->FeatureRecord[i].featureTable, &dp);
	}

	*pp = p;
	return r;
}

static void pairSetTable_parse(struct PairSetTable *r, uint8_t **pp)
{
	uint8_t *p = *pp;
	int i;

	r->PairValueCount = get_u16(&p);

	if (r->PairValueCount) {
		r->PairValueRecord = x_calloc(r->PairValueCount, sizeof(struct PairValueRecord));

		for (i = 0; i < r->PairValueCount; i++) {
			struct PairValueRecord *pv = &r->PairValueRecord[i];
			pv->SecondGlyph  = get_u16(&p);
			pv->ValueRecord1 = get_s16(&p);
		}
	}

	*pp = p;
}
static void subTable_parse(struct subTable *r, uint8_t **pp)
{
	uint8_t *p = *pp;
	uint8_t *p0 = p;
	int i;

	r->PosFormat    = get_u16(&p);
	r->Coverage     = get_u16(&p); /* TODO */
	r->ValueFormat1 = get_u16(&p);
	r->ValueFormat2 = get_u16(&p);
	r->PairSetCount = get_u16(&p);

	if (r->PairSetCount) {
		r->PairSetOffset = x_calloc(r->PairSetCount, sizeof(uint16_t));
		r->pairSetTable  = x_calloc(r->PairSetCount, sizeof(struct PairSetTable));

		for (i = 0; i < r->PairSetCount; i++) {
			uint8_t *dp;
			r->PairSetOffset[i] = get_u16(&p);
			dp = p0 + r->PairSetOffset[i];
			pairSetTable_parse(&r->pairSetTable[i], &dp);
		}
	}

	if (r->Coverage) {
		uint8_t *dp = p0 + r->Coverage;
		r->coverage.CoverageFormat = get_u16(&dp);
		if (r->coverage.CoverageFormat != 2) {
			fprintf(stderr, "%s: only CoverageFormat 2 supported\n", __func__);
			exit(-1);
		}
		r->coverage.RangeCount  = get_u16(&dp);
		r->coverage.RangeRecord = x_calloc(r->coverage.RangeCount, sizeof(struct RangeRecord));
		for (i = 0; i < r->coverage.RangeCount; i++) {
			r->coverage.RangeRecord[i].Start              = get_u16(&dp);
			r->coverage.RangeRecord[i].End                = get_u16(&dp);
			r->coverage.RangeRecord[i].StartCoverageIndex = get_u16(&dp);
		}
	}

	*pp = p;
}
static void lookupTable_parse(struct lookupTable *r, uint8_t **pp)
{
	uint8_t *p = *pp;
	uint8_t *p0 = p;
	int i;

	r->LookupType    = get_u16(&p);
	r->LookupFlag    = get_u16(&p);
	r->SubTableCount = get_u16(&p);

	if (r->SubTableCount) {
		r->SubTable = x_calloc(r->SubTableCount, sizeof(uint16_t));
		r->subTable = x_calloc(r->SubTableCount, sizeof(struct subTable));

		for (i = 0; i < r->SubTableCount; i++) {
			uint8_t *dp;
			r->SubTable[i] = get_u16(&p);
			dp = p0 + r->SubTable[i];
			subTable_parse(&r->subTable[i], &dp);
		}
	}
	if (r->LookupFlag & 0x0010)
		r->MarkFilteringSet = get_u16(&p);

	*pp = p;
}
static struct lookupList *lookupList_parse(uint8_t **pp)
{
	struct lookupList *r = x_malloc(sizeof(struct lookupList));
	uint8_t *p = *pp;
	uint8_t *p0 = p;
	int i;

	r->LookupCount = get_u16(&p);

	if (r->LookupCount) {
		r->Lookup      = x_calloc(r->LookupCount, sizeof(uint16_t));
		r->lookupTable = x_calloc(r->LookupCount, sizeof(struct lookupTable));

		for (i = 0; i < r->LookupCount; i++) {
			uint8_t *dp;
			r->Lookup[i] = get_u16(&p);
			dp = p0 + r->Lookup[i];
			lookupTable_parse(&r->lookupTable[i], &dp);
		}
	}

	*pp = p;
	return r;
}

struct gpos *gpos_parse(uint8_t *p)
{
	struct gpos *gpos = x_calloc(1, sizeof(struct gpos));
	uint8_t *p0 = p;

	gpos->version     = get_u32(&p);
	gpos->ScriptList  = get_u16(&p);
	gpos->FeatureList = get_u16(&p);
	gpos->LookupList  = get_u16(&p);

	if (gpos->ScriptList) {
		uint8_t *dp = p0 + gpos->ScriptList;
		gpos->scriptList = scriptList_parse(&dp);
	}
	if (gpos->FeatureList) {
		uint8_t *dp = p0 + gpos->FeatureList;
		gpos->featureList = featureList_parse(&dp);
	}
	if (gpos->LookupList) {
		uint8_t *dp = p0 + gpos->LookupList;
		gpos->lookupList = lookupList_parse(&dp);
	}

	return gpos;
}
void gpos_debug(struct gpos *gpos)
{
	int i;

	printf("GPOS\n");
	printf("\tversion     : %08x\n", gpos->version);
	printf("\tScriptList  : %04x\n", gpos->ScriptList);
	printf("\tFeatureList : %04x\n", gpos->FeatureList);
	printf("\tLookupList  : %04x\n", gpos->LookupList);

	if (gpos->ScriptList) {
		struct scriptList *s1 = gpos->scriptList;
		printf("GPOS ScriptList\n");
		printf("scriptCount: %d\n", s1->scriptCount);

		for (i = 0; i < s1->scriptCount; i++) {
			struct scriptRecord *s2 = &s1->scriptRecord[i];
			struct scriptTable *t1 = &s2->scriptTable;
			struct langSysTable *t2 = &t1->defaultLangSys;
			int j;

			printf("\tScriptTag: %08x (%.4s)\n", s2->ScriptTag, (char*) &s2->ScriptTag);
			printf("\tScript   : %d\n", s2->Script);

			printf("scriptTable\n");
			printf("\tDefaultLangSys: %d\n", t1->DefaultLangSys);
			printf("\tLangSysCount  : %d\n", t1->LangSysCount);

			printf("defaultLangSys\n");
			printf("\tReqFeatureIndex: %d\n", t2->ReqFeatureIndex);
			printf("\tFeatureCount   : %d\n", t2->FeatureCount);

			for (j = 0; j < t2->FeatureCount; j++)
				printf("\tFeatureIndex[%d]: %d\n", j, t2->FeatureIndex[j]);
		}
	}
	if (gpos->FeatureList) {
		struct featureList *f1 = gpos->featureList;
		printf("GPOS FeatureList\n");
		printf("FeatureCount: %d\n", f1->FeatureCount);

		for (i = 0; i < f1->FeatureCount; i++) {
			struct featureRecord *f2 = &f1->FeatureRecord[i];
			struct featureTable *t1 = &f2->featureTable;
			int j;

			printf("\tFeatureTag: %08x (%.4s)\n", f2->FeatureTag, (char*) &f2->FeatureTag);
			printf("\tFeature   : %d\n", f2->Feature);

			printf("featureTable\n");
			printf("\tFeatureParams: %d\n", t1->FeatureParams);
			printf("\tLookupCount  : %d\n", t1->LookupCount);

			for (j = 0; j < t1->LookupCount; j++)
				printf("\tLookupListIndex[%d]: %d\n", j, t1->LookupListIndex[j]);
		}
	}
	if (gpos->LookupList) {
		struct lookupList *f1 = gpos->lookupList;
		int j;

		printf("GPOS LookupList\n");
		printf("LookupCount: %d\n", f1->LookupCount);
		for (j = 0; j < f1->LookupCount; j++) {
			struct lookupTable *t1 = &f1->lookupTable[j];
			int k;

			printf("\tLookup[%d]: %d\n", j, f1->Lookup[j]);
			printf("\tLookupType: %d\n", t1->LookupType);
			printf("\tLookupFlag: %d\n", t1->LookupFlag);
			printf("\tSubTableCount: %d\n", t1->SubTableCount);

			for (k = 0; k < t1->SubTableCount; k++) {
				struct subTable *t2 = &t1->subTable[k];
				int m;

				printf("\t\tSubTable[%d]: %d\n", k, t1->SubTable[k]);
				printf("\t\tPosFormat   : %d\n", t2->PosFormat);
				printf("\t\tCoverage    : %d\n", t2->Coverage);
				printf("\t\tValueFormat1: %d\n", t2->ValueFormat1);
				printf("\t\tValueFormat2: %d\n", t2->ValueFormat2);
				printf("\t\tPairSetCount: %d\n", t2->PairSetCount);

				for (m = 0; m < t2->PairSetCount; m++) {
					struct PairSetTable *t3 = &t2->pairSetTable[m];
					int o;

					printf("\t\tPairSetOffset[%d]: %d\n", m, t2->PairSetOffset[m]);
					printf("\t\t\tPairValueCount: %d\n", t3->PairValueCount);

					for (o = 0; o < t3->PairValueCount; o++) {
						struct PairValueRecord *t4 = &t3->PairValueRecord[o];
						printf("\t\t\t[%3d] %d\n", t4->SecondGlyph, t4->ValueRecord1);
					}
				}

				printf("\t\t** Coverage\n");
				printf("\t\t\tCoverageFormat: %d\n", t2->coverage.CoverageFormat);
				printf("\t\t\tRangeCount    : %d\n", t2->coverage.RangeCount);
				for (m = 0; m < t2->coverage.RangeCount; m++)
					printf("\t\t\tRangeRecord[%3d] [Start %3d] [End %3d] %d\n", m,
					       t2->coverage.RangeRecord[m].Start,
					       t2->coverage.RangeRecord[m].End,
					       t2->coverage.RangeRecord[m].StartCoverageIndex);
			}
		}
	}
}
void gpos_free(struct gpos *gpos)
{
}
void gpos_kern_dump(struct gpos *gpos)
{
	/* FIXME: hardcoded to my specific font */
	struct subTable *t2 = &gpos->lookupList->lookupTable[0].subTable[0];
	int i;

	for (i = 0; i < t2->coverage.RangeCount; i++) {
		struct RangeRecord *rr = &t2->coverage.RangeRecord[i];
		uint16_t idx = rr->StartCoverageIndex;
		uint16_t g;
		for (g = rr->Start; g <= rr->End; g++) {
			struct PairSetTable *t3 = &t2->pairSetTable[idx++];
			int j;
			for (j = 0; j < t3->PairValueCount; j++) {
				struct PairValueRecord *t4 = &t3->PairValueRecord[j];
				printf("\t[%3d] [%3d] %d\n", g, t4->SecondGlyph, t4->ValueRecord1);
			}
		}
	}
}
int gpos_kern_get(struct gpos *gpos, uint16_t l, uint16_t r)
{
	/* FIXME: hardcoded to my specific font */
	struct subTable *t2 = &gpos->lookupList->lookupTable[0].subTable[0];
	int idx = -1;
	int i;

	for (i = 0; i < t2->coverage.RangeCount; i++) {
		struct RangeRecord *rr = &t2->coverage.RangeRecord[i];
		if (l >= rr->Start && l <= rr->End) {
			idx = rr->StartCoverageIndex + l - rr->Start;
			break;
		}
	}

	if (idx != -1) {
		struct PairSetTable *t3 = &t2->pairSetTable[idx];
		for (i = 0; i < t3->PairValueCount; i++) {
			struct PairValueRecord *t4 = &t3->PairValueRecord[i];
			if (t4->SecondGlyph == r)
				return t4->ValueRecord1;
		}
	}

	return 0;
}
