#include "cache.h"
int index_bulk_checkin(struct object_id *oid,
int fd, size_t size, enum object_type type,
const char *path, unsigned flags);
void plug_bulk_checkin(void);
void unplug_bulk_checkin(void);
