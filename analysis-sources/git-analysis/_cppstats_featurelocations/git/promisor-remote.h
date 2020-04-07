#if !defined(PROMISOR_REMOTE_H)
#define PROMISOR_REMOTE_H

#include "repository.h"

struct object_id;







struct promisor_remote {
struct promisor_remote *next;
const char *partial_clone_filter;
const char name[FLEX_ARRAY];
};

void promisor_remote_reinit(void);
struct promisor_remote *promisor_remote_find(const char *remote_name);
int has_promisor_remote(void);
int promisor_remote_get_direct(struct repository *repo,
const struct object_id *oids,
int oid_nr);





void set_repository_format_partial_clone(char *partial_clone);

#endif 
