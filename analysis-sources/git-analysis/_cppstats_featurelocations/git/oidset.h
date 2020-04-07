#if !defined(OIDSET_H)
#define OIDSET_H

#include "khash.h"
















struct oidset {
kh_oid_set_t set;
};

#define OIDSET_INIT { { 0 } }








void oidset_init(struct oidset *set, size_t initial_size);




int oidset_contains(const struct oidset *set, const struct object_id *oid);








int oidset_insert(struct oidset *set, const struct object_id *oid);






int oidset_remove(struct oidset *set, const struct object_id *oid);





void oidset_clear(struct oidset *set);







void oidset_parse_file(struct oidset *set, const char *path);

struct oidset_iter {
kh_oid_set_t *set;
khiter_t iter;
};

static inline void oidset_iter_init(struct oidset *set,
struct oidset_iter *iter)
{
iter->set = &set->set;
iter->iter = kh_begin(iter->set);
}

static inline struct object_id *oidset_iter_next(struct oidset_iter *iter)
{
for (; iter->iter != kh_end(iter->set); iter->iter++) {
if (kh_exist(iter->set, iter->iter))
return &kh_key(iter->set, iter->iter++);
}
return NULL;
}

static inline struct object_id *oidset_iter_first(struct oidset *set,
struct oidset_iter *iter)
{
oidset_iter_init(set, iter);
return oidset_iter_next(iter);
}

#endif 
