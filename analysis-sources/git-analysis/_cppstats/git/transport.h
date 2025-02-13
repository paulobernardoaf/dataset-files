#include "cache.h"
#include "run-command.h"
#include "remote.h"
#include "list-objects-filter-options.h"
struct string_list;
struct git_transport_options {
unsigned thin : 1;
unsigned keep : 1;
unsigned followtags : 1;
unsigned check_self_contained_and_connected : 1;
unsigned self_contained_and_connected : 1;
unsigned update_shallow : 1;
unsigned deepen_relative : 1;
unsigned from_promisor : 1;
unsigned no_dependents : 1;
unsigned connectivity_checked:1;
int depth;
const char *deepen_since;
const struct string_list *deepen_not;
const char *uploadpack;
const char *receivepack;
struct push_cas_option *cas;
struct list_objects_filter_options filter_options;
struct oid_array *negotiation_tips;
};
enum transport_family {
TRANSPORT_FAMILY_ALL = 0,
TRANSPORT_FAMILY_IPV4,
TRANSPORT_FAMILY_IPV6
};
struct transport {
const struct transport_vtable *vtable;
struct remote *remote;
const char *url;
void *data;
const struct ref *remote_refs;
unsigned got_remote_refs : 1;
unsigned cannot_reuse : 1;
unsigned cloning : 1;
unsigned stateless_rpc : 1;
const struct string_list *push_options;
const struct string_list *server_options;
char *pack_lockfile;
signed verbose : 3;
unsigned progress : 1;
struct git_transport_options *smart_options;
enum transport_family family;
};
#define TRANSPORT_PUSH_ALL (1<<0)
#define TRANSPORT_PUSH_FORCE (1<<1)
#define TRANSPORT_PUSH_DRY_RUN (1<<2)
#define TRANSPORT_PUSH_MIRROR (1<<3)
#define TRANSPORT_PUSH_PORCELAIN (1<<4)
#define TRANSPORT_PUSH_SET_UPSTREAM (1<<5)
#define TRANSPORT_RECURSE_SUBMODULES_CHECK (1<<6)
#define TRANSPORT_PUSH_PRUNE (1<<7)
#define TRANSPORT_RECURSE_SUBMODULES_ON_DEMAND (1<<8)
#define TRANSPORT_PUSH_NO_HOOK (1<<9)
#define TRANSPORT_PUSH_FOLLOW_TAGS (1<<10)
#define TRANSPORT_PUSH_CERT_ALWAYS (1<<11)
#define TRANSPORT_PUSH_CERT_IF_ASKED (1<<12)
#define TRANSPORT_PUSH_ATOMIC (1<<13)
#define TRANSPORT_PUSH_OPTIONS (1<<14)
#define TRANSPORT_RECURSE_SUBMODULES_ONLY (1<<15)
int transport_summary_width(const struct ref *refs);
struct transport *transport_get(struct remote *, const char *);
int is_transport_allowed(const char *type, int from_user);
void transport_check_allowed(const char *type);
#define TRANS_OPT_UPLOADPACK "uploadpack"
#define TRANS_OPT_RECEIVEPACK "receivepack"
#define TRANS_OPT_THIN "thin"
#define TRANS_OPT_CAS "cas"
#define TRANS_OPT_KEEP "keep"
#define TRANS_OPT_DEPTH "depth"
#define TRANS_OPT_DEEPEN_SINCE "deepen-since"
#define TRANS_OPT_DEEPEN_NOT "deepen-not"
#define TRANS_OPT_DEEPEN_RELATIVE "deepen-relative"
#define TRANS_OPT_FOLLOWTAGS "followtags"
#define TRANS_OPT_UPDATE_SHALLOW "updateshallow"
#define TRANS_OPT_PUSH_CERT "pushcert"
#define TRANS_OPT_FROM_PROMISOR "from-promisor"
#define TRANS_OPT_NO_DEPENDENTS "no-dependents"
#define TRANS_OPT_LIST_OBJECTS_FILTER "filter"
#define TRANS_OPT_ATOMIC "atomic"
int transport_set_option(struct transport *transport, const char *name,
const char *value);
void transport_set_verbosity(struct transport *transport, int verbosity,
int force_progress);
#define REJECT_NON_FF_HEAD 0x01
#define REJECT_NON_FF_OTHER 0x02
#define REJECT_ALREADY_EXISTS 0x04
#define REJECT_FETCH_FIRST 0x08
#define REJECT_NEEDS_FORCE 0x10
int transport_push(struct repository *repo,
struct transport *connection,
struct refspec *rs, int flags,
unsigned int * reject_reasons);
const struct ref *transport_get_remote_refs(struct transport *transport,
const struct argv_array *ref_prefixes);
int transport_fetch_refs(struct transport *transport, struct ref *refs);
void transport_unlock_pack(struct transport *transport);
int transport_disconnect(struct transport *transport);
char *transport_anonymize_url(const char *url);
void transport_take_over(struct transport *transport,
struct child_process *child);
int transport_connect(struct transport *transport, const char *name,
const char *exec, int fd[2]);
int transport_helper_init(struct transport *transport, const char *name);
int bidirectional_transfer_loop(int input, int output);
void transport_update_tracking_ref(struct remote *remote, struct ref *ref, int verbose);
int transport_refs_pushed(struct ref *ref);
void transport_print_push_status(const char *dest, struct ref *refs,
int verbose, int porcelain, unsigned int *reject_reasons);
