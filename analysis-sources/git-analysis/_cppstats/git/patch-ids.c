#include "cache.h"
#include "diff.h"
#include "commit.h"
#include "sha1-lookup.h"
#include "patch-ids.h"
static int patch_id_defined(struct commit *commit)
{
return !commit->parents || !commit->parents->next;
}
int commit_patch_id(struct commit *commit, struct diff_options *options,
struct object_id *oid, int diff_header_only, int stable)
{
if (!patch_id_defined(commit))
return -1;
if (commit->parents)
diff_tree_oid(&commit->parents->item->object.oid,
&commit->object.oid, "", options);
else
diff_root_tree_oid(&commit->object.oid, "", options);
diffcore_std(options);
return diff_flush_patch_id(options, oid, diff_header_only, stable);
}
static int patch_id_neq(const void *cmpfn_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *unused_keydata)
{
struct diff_options *opt = (void *)cmpfn_data;
struct patch_id *a, *b;
a = container_of(eptr, struct patch_id, ent);
b = container_of(entry_or_key, struct patch_id, ent);
if (is_null_oid(&a->patch_id) &&
commit_patch_id(a->commit, opt, &a->patch_id, 0, 0))
return error("Could not get patch ID for %s",
oid_to_hex(&a->commit->object.oid));
if (is_null_oid(&b->patch_id) &&
commit_patch_id(b->commit, opt, &b->patch_id, 0, 0))
return error("Could not get patch ID for %s",
oid_to_hex(&b->commit->object.oid));
return !oideq(&a->patch_id, &b->patch_id);
}
int init_patch_ids(struct repository *r, struct patch_ids *ids)
{
memset(ids, 0, sizeof(*ids));
repo_diff_setup(r, &ids->diffopts);
ids->diffopts.detect_rename = 0;
ids->diffopts.flags.recursive = 1;
diff_setup_done(&ids->diffopts);
hashmap_init(&ids->patches, patch_id_neq, &ids->diffopts, 256);
return 0;
}
int free_patch_ids(struct patch_ids *ids)
{
hashmap_free_entries(&ids->patches, struct patch_id, ent);
return 0;
}
static int init_patch_id_entry(struct patch_id *patch,
struct commit *commit,
struct patch_ids *ids)
{
struct object_id header_only_patch_id;
patch->commit = commit;
if (commit_patch_id(commit, &ids->diffopts, &header_only_patch_id, 1, 0))
return -1;
hashmap_entry_init(&patch->ent, oidhash(&header_only_patch_id));
return 0;
}
struct patch_id *has_commit_patch_id(struct commit *commit,
struct patch_ids *ids)
{
struct patch_id patch;
if (!patch_id_defined(commit))
return NULL;
memset(&patch, 0, sizeof(patch));
if (init_patch_id_entry(&patch, commit, ids))
return NULL;
return hashmap_get_entry(&ids->patches, &patch, ent, NULL);
}
struct patch_id *add_commit_patch_id(struct commit *commit,
struct patch_ids *ids)
{
struct patch_id *key;
if (!patch_id_defined(commit))
return NULL;
key = xcalloc(1, sizeof(*key));
if (init_patch_id_entry(key, commit, ids)) {
free(key);
return NULL;
}
hashmap_add(&ids->patches, &key->ent);
return key;
}
