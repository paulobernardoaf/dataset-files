#if !defined(CHECKOUT_H)
#define CHECKOUT_H

#include "cache.h"






const char *unique_tracking_name(const char *name,
struct object_id *oid,
int *dwim_remotes_matched);

#endif 
