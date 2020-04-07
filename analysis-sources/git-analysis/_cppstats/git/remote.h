#include "cache.h"
#include "parse-options.h"
#include "hashmap.h"
#include "refspec.h"
enum {
REMOTE_UNCONFIGURED = 0,
REMOTE_CONFIG,
REMOTE_REMOTES,
REMOTE_BRANCHES
};
struct remote {
struct hashmap_entry ent;
const char *name;
int origin, configured_in_repo;
const char *foreign_vcs;
const char **url;
int url_nr;
int url_alloc;
const char **pushurl;
int pushurl_nr;
int pushurl_alloc;
struct refspec push;
struct refspec fetch;
int fetch_tags;
int skip_default_update;
int mirror;
int prune;
int prune_tags;
const char *receivepack;
const char *uploadpack;
char *http_proxy;
char *http_proxy_authmethod;
};
struct remote *remote_get(const char *name);
struct remote *pushremote_get(const char *name);
int remote_is_configured(struct remote *remote, int in_repo);
typedef int each_remote_fn(struct remote *remote, void *priv);
int for_each_remote(each_remote_fn fn, void *priv);
int remote_has_url(struct remote *remote, const char *url);
struct ref {
struct ref *next;
struct object_id old_oid;
struct object_id new_oid;
struct object_id old_oid_expect; 
char *symref;
unsigned int
force:1,
forced_update:1,
expect_old_sha1:1,
exact_oid:1,
deletion:1;
enum {
REF_NOT_MATCHED = 0, 
REF_MATCHED,
REF_UNADVERTISED_NOT_ALLOWED
} match_status;
enum {
FETCH_HEAD_MERGE = -1,
FETCH_HEAD_NOT_FOR_MERGE = 0,
FETCH_HEAD_IGNORE = 1
} fetch_head_status;
enum {
REF_STATUS_NONE = 0,
REF_STATUS_OK,
REF_STATUS_REJECT_NONFASTFORWARD,
REF_STATUS_REJECT_ALREADY_EXISTS,
REF_STATUS_REJECT_NODELETE,
REF_STATUS_REJECT_FETCH_FIRST,
REF_STATUS_REJECT_NEEDS_FORCE,
REF_STATUS_REJECT_STALE,
REF_STATUS_REJECT_SHALLOW,
REF_STATUS_UPTODATE,
REF_STATUS_REMOTE_REJECT,
REF_STATUS_EXPECTING_REPORT,
REF_STATUS_ATOMIC_PUSH_FAILED
} status;
char *remote_status;
struct ref *peer_ref; 
char name[FLEX_ARRAY]; 
};
#define REF_NORMAL (1u << 0)
#define REF_HEADS (1u << 1)
#define REF_TAGS (1u << 2)
struct ref *find_ref_by_name(const struct ref *list, const char *name);
struct ref *alloc_ref(const char *name);
struct ref *copy_ref(const struct ref *ref);
struct ref *copy_ref_list(const struct ref *ref);
void sort_ref_list(struct ref **, int (*cmp)(const void *, const void *));
int count_refspec_match(const char *, struct ref *refs, struct ref **matched_ref);
int ref_compare_name(const void *, const void *);
int check_ref_type(const struct ref *ref, int flags);
void free_one_ref(struct ref *ref);
void free_refs(struct ref *ref);
struct oid_array;
struct packet_reader;
struct argv_array;
struct string_list;
struct ref **get_remote_heads(struct packet_reader *reader,
struct ref **list, unsigned int flags,
struct oid_array *extra_have,
struct oid_array *shallow_points);
struct ref **get_remote_refs(int fd_out, struct packet_reader *reader,
struct ref **list, int for_push,
const struct argv_array *ref_prefixes,
const struct string_list *server_options);
int resolve_remote_symref(struct ref *ref, struct ref *list);
struct ref *ref_remove_duplicates(struct ref *ref_map);
int query_refspecs(struct refspec *rs, struct refspec_item *query);
char *apply_refspecs(struct refspec *rs, const char *name);
int check_push_refs(struct ref *src, struct refspec *rs);
int match_push_refs(struct ref *src, struct ref **dst,
struct refspec *rs, int flags);
void set_ref_status_for_push(struct ref *remote_refs, int send_mirror,
int force_update);
int get_fetch_map(const struct ref *remote_refs, const struct refspec_item *refspec,
struct ref ***tail, int missing_ok);
struct ref *get_remote_ref(const struct ref *remote_refs, const char *name);
int remote_find_tracking(struct remote *remote, struct refspec_item *refspec);
struct branch {
const char *name;
const char *refname;
const char *remote_name;
const char *pushremote_name;
const char **merge_name;
struct refspec_item **merge;
int merge_nr;
int merge_alloc;
const char *push_tracking_ref;
};
struct branch *branch_get(const char *name);
const char *remote_for_branch(struct branch *branch, int *explicit);
const char *pushremote_for_branch(struct branch *branch, int *explicit);
const char *remote_ref_for_branch(struct branch *branch, int for_push);
int branch_has_merge_config(struct branch *branch);
int branch_merge_matches(struct branch *, int n, const char *);
const char *branch_get_upstream(struct branch *branch, struct strbuf *err);
const char *branch_get_push(struct branch *branch, struct strbuf *err);
enum match_refs_flags {
MATCH_REFS_NONE = 0,
MATCH_REFS_ALL = (1 << 0),
MATCH_REFS_MIRROR = (1 << 1),
MATCH_REFS_PRUNE = (1 << 2),
MATCH_REFS_FOLLOW_TAGS = (1 << 3)
};
enum ahead_behind_flags {
AHEAD_BEHIND_UNSPECIFIED = -1,
AHEAD_BEHIND_QUICK = 0, 
AHEAD_BEHIND_FULL = 1, 
};
int stat_tracking_info(struct branch *branch, int *num_ours, int *num_theirs,
const char **upstream_name, int for_push,
enum ahead_behind_flags abf);
int format_tracking_info(struct branch *branch, struct strbuf *sb,
enum ahead_behind_flags abf);
struct ref *get_local_heads(void);
struct ref *guess_remote_head(const struct ref *head,
const struct ref *refs,
int all);
struct ref *get_stale_heads(struct refspec *rs, struct ref *fetch_map);
#define CAS_OPT_NAME "force-with-lease"
struct push_cas_option {
unsigned use_tracking_for_rest:1;
struct push_cas {
struct object_id expect;
unsigned use_tracking:1;
char *refname;
} *entry;
int nr;
int alloc;
};
int parseopt_push_cas_option(const struct option *, const char *arg, int unset);
int is_empty_cas(const struct push_cas_option *);
void apply_push_cas(struct push_cas_option *, struct remote *, struct ref *);
