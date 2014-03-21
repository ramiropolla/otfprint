#ifndef OTF_SID_H
#define OTF_SID_H

#include "otf_cff.h"

struct sid {
	int count;
	char **strings;
};

struct sid *sid_new(struct cff_index_data *string_idx);
char *      sid_get(struct sid *sid, int idx);
void        sid_free(struct sid *sid);

#endif /* OTF_SID_H */
