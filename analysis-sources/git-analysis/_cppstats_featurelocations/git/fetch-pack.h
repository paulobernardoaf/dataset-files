#if !defined(FETCH_PACK_H)
#define FETCH_PACK_H

#include "string-list.h"
#include "run-command.h"
#include "protocol.h"
#include "list-objects-filter-options.h"

struct oid_array;

struct fetch_pack_args {
const char *uploadpack;
int unpacklimit;
int depth;
const char *deepen_since;
const struct string_list *deepen_not;
struct list_objects_filter_options filter_options;
const struct string_list *server_options;





const struct oid_array *negotiation_tips;

unsigned deepen_relative:1;
unsigned quiet:1;
unsigned keep_pack:1;
unsigned lock_pack:1;
unsigned use_thin_pack:1;
unsigned fetch_all:1;
unsigned stdin_refs:1;
unsigned diag_url:1;
unsigned verbose:1;
unsigned no_progress:1;
unsigned include_tag:1;
unsigned stateless_rpc:1;
unsigned check_self_contained_and_connected:1;
unsigned self_contained_and_connected:1;
unsigned cloning:1;
unsigned update_shallow:1;
unsigned deepen:1;
unsigned from_promisor:1;













unsigned no_dependents:1;














unsigned connectivity_checked:1;
};






struct ref *fetch_pack(struct fetch_pack_args *args,
int fd[],
const struct ref *ref,
struct ref **sought,
int nr_sought,
struct oid_array *shallow,
char **pack_lockfile,
enum protocol_version version);





int report_unmatched_refs(struct ref **sought, int nr_sought);

#endif
