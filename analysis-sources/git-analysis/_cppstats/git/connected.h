struct object_id;
struct transport;
typedef int (*oid_iterate_fn)(void *, struct object_id *oid);
struct check_connected_options {
int quiet;
const char *shallow_file;
struct transport *transport;
int err_fd;
int progress;
const char **env;
unsigned is_deepening_fetch : 1;
unsigned check_refs_are_promisor_objects_only : 1;
};
#define CHECK_CONNECTED_INIT { 0 }
int check_connected(oid_iterate_fn fn, void *cb_data,
struct check_connected_options *opt);
