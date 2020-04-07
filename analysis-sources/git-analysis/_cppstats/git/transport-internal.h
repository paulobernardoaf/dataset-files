struct ref;
struct transport;
struct argv_array;
struct transport_vtable {
int (*set_option)(struct transport *connection, const char *name,
const char *value);
struct ref *(*get_refs_list)(struct transport *transport, int for_push,
const struct argv_array *ref_prefixes);
int (*fetch)(struct transport *transport, int refs_nr, struct ref **refs);
int (*push_refs)(struct transport *transport, struct ref *refs, int flags);
int (*connect)(struct transport *connection, const char *name,
const char *executable, int fd[2]);
int (*disconnect)(struct transport *connection);
};
