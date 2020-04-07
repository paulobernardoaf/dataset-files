#include "cache.h"
#include "split-index.h"
#include "ewah/ewok.h"
struct split_index *init_split_index(struct index_state *istate)
{
if (!istate->split_index) {
istate->split_index = xcalloc(1, sizeof(*istate->split_index));
istate->split_index->refcount = 1;
}
return istate->split_index;
}
int read_link_extension(struct index_state *istate,
const void *data_, unsigned long sz)
{
const unsigned char *data = data_;
struct split_index *si;
int ret;
if (sz < the_hash_algo->rawsz)
return error("corrupt link extension (too short)");
si = init_split_index(istate);
hashcpy(si->base_oid.hash, data);
data += the_hash_algo->rawsz;
sz -= the_hash_algo->rawsz;
if (!sz)
return 0;
si->delete_bitmap = ewah_new();
ret = ewah_read_mmap(si->delete_bitmap, data, sz);
if (ret < 0)
return error("corrupt delete bitmap in link extension");
data += ret;
sz -= ret;
si->replace_bitmap = ewah_new();
ret = ewah_read_mmap(si->replace_bitmap, data, sz);
if (ret < 0)
return error("corrupt replace bitmap in link extension");
if (ret != sz)
return error("garbage at the end of link extension");
return 0;
}
int write_link_extension(struct strbuf *sb,
struct index_state *istate)
{
struct split_index *si = istate->split_index;
strbuf_add(sb, si->base_oid.hash, the_hash_algo->rawsz);
if (!si->delete_bitmap && !si->replace_bitmap)
return 0;
ewah_serialize_strbuf(si->delete_bitmap, sb);
ewah_serialize_strbuf(si->replace_bitmap, sb);
return 0;
}
static void mark_base_index_entries(struct index_state *base)
{
int i;
for (i = 0; i < base->cache_nr; i++)
base->cache[i]->index = i + 1;
}
void move_cache_to_base_index(struct index_state *istate)
{
struct split_index *si = istate->split_index;
int i;
if (si->base &&
si->base->ce_mem_pool) {
if (!istate->ce_mem_pool)
mem_pool_init(&istate->ce_mem_pool, 0);
mem_pool_combine(istate->ce_mem_pool, istate->split_index->base->ce_mem_pool);
}
si->base = xcalloc(1, sizeof(*si->base));
si->base->version = istate->version;
si->base->timestamp = istate->timestamp;
ALLOC_GROW(si->base->cache, istate->cache_nr, si->base->cache_alloc);
si->base->cache_nr = istate->cache_nr;
si->base->ce_mem_pool = istate->ce_mem_pool;
istate->ce_mem_pool = NULL;
COPY_ARRAY(si->base->cache, istate->cache, istate->cache_nr);
mark_base_index_entries(si->base);
for (i = 0; i < si->base->cache_nr; i++)
si->base->cache[i]->ce_flags &= ~CE_UPDATE_IN_BASE;
}
static void mark_entry_for_delete(size_t pos, void *data)
{
struct index_state *istate = data;
if (pos >= istate->cache_nr)
die("position for delete %d exceeds base index size %d",
(int)pos, istate->cache_nr);
istate->cache[pos]->ce_flags |= CE_REMOVE;
istate->split_index->nr_deletions++;
}
static void replace_entry(size_t pos, void *data)
{
struct index_state *istate = data;
struct split_index *si = istate->split_index;
struct cache_entry *dst, *src;
if (pos >= istate->cache_nr)
die("position for replacement %d exceeds base index size %d",
(int)pos, istate->cache_nr);
if (si->nr_replacements >= si->saved_cache_nr)
die("too many replacements (%d vs %d)",
si->nr_replacements, si->saved_cache_nr);
dst = istate->cache[pos];
if (dst->ce_flags & CE_REMOVE)
die("entry %d is marked as both replaced and deleted",
(int)pos);
src = si->saved_cache[si->nr_replacements];
if (ce_namelen(src))
die("corrupt link extension, entry %d should have "
"zero length name", (int)pos);
src->index = pos + 1;
src->ce_flags |= CE_UPDATE_IN_BASE;
src->ce_namelen = dst->ce_namelen;
copy_cache_entry(dst, src);
discard_cache_entry(src);
si->nr_replacements++;
}
void merge_base_index(struct index_state *istate)
{
struct split_index *si = istate->split_index;
unsigned int i;
mark_base_index_entries(si->base);
si->saved_cache = istate->cache;
si->saved_cache_nr = istate->cache_nr;
istate->cache_nr = si->base->cache_nr;
istate->cache = NULL;
istate->cache_alloc = 0;
ALLOC_GROW(istate->cache, istate->cache_nr, istate->cache_alloc);
COPY_ARRAY(istate->cache, si->base->cache, istate->cache_nr);
si->nr_deletions = 0;
si->nr_replacements = 0;
ewah_each_bit(si->replace_bitmap, replace_entry, istate);
ewah_each_bit(si->delete_bitmap, mark_entry_for_delete, istate);
if (si->nr_deletions)
remove_marked_cache_entries(istate, 0);
for (i = si->nr_replacements; i < si->saved_cache_nr; i++) {
if (!ce_namelen(si->saved_cache[i]))
die("corrupt link extension, entry %d should "
"have non-zero length name", i);
add_index_entry(istate, si->saved_cache[i],
ADD_CACHE_OK_TO_ADD |
ADD_CACHE_KEEP_CACHE_TREE |
ADD_CACHE_SKIP_DFCHECK);
si->saved_cache[i] = NULL;
}
ewah_free(si->delete_bitmap);
ewah_free(si->replace_bitmap);
FREE_AND_NULL(si->saved_cache);
si->delete_bitmap = NULL;
si->replace_bitmap = NULL;
si->saved_cache_nr = 0;
}
static int compare_ce_content(struct cache_entry *a, struct cache_entry *b)
{
const unsigned int ondisk_flags = CE_STAGEMASK | CE_VALID |
CE_EXTENDED_FLAGS;
unsigned int ce_flags = a->ce_flags;
unsigned int base_flags = b->ce_flags;
int ret;
a->ce_flags &= ondisk_flags;
b->ce_flags &= ondisk_flags;
ret = memcmp(&a->ce_stat_data, &b->ce_stat_data,
offsetof(struct cache_entry, name) -
offsetof(struct cache_entry, ce_stat_data));
a->ce_flags = ce_flags;
b->ce_flags = base_flags;
return ret;
}
void prepare_to_write_split_index(struct index_state *istate)
{
struct split_index *si = init_split_index(istate);
struct cache_entry **entries = NULL, *ce;
int i, nr_entries = 0, nr_alloc = 0;
si->delete_bitmap = ewah_new();
si->replace_bitmap = ewah_new();
if (si->base) {
for (i = 0; i < istate->cache_nr; i++) {
struct cache_entry *base;
ce = istate->cache[i];
if (!ce->index) {
continue;
}
if (ce->index > si->base->cache_nr) {
BUG("ce refers to a shared ce at %d, which is beyond the shared index size %d",
ce->index, si->base->cache_nr);
}
ce->ce_flags |= CE_MATCHED; 
base = si->base->cache[ce->index - 1];
if (ce == base) {
if (ce->ce_flags & CE_UPDATE_IN_BASE) {
} else if (!ce_uptodate(ce) &&
is_racy_timestamp(istate, ce)) {
ce->ce_flags |= CE_UPDATE_IN_BASE;
} else {
}
continue;
}
if (ce->ce_namelen != base->ce_namelen ||
strcmp(ce->name, base->name)) {
ce->index = 0;
continue;
}
if (ce->ce_flags & CE_UPDATE_IN_BASE) {
} else if (!ce_uptodate(ce) &&
is_racy_timestamp(istate, ce)) {
ce->ce_flags |= CE_UPDATE_IN_BASE;
} else {
if (compare_ce_content(ce, base))
ce->ce_flags |= CE_UPDATE_IN_BASE;
}
discard_cache_entry(base);
si->base->cache[ce->index - 1] = ce;
}
for (i = 0; i < si->base->cache_nr; i++) {
ce = si->base->cache[i];
if ((ce->ce_flags & CE_REMOVE) ||
!(ce->ce_flags & CE_MATCHED))
ewah_set(si->delete_bitmap, i);
else if (ce->ce_flags & CE_UPDATE_IN_BASE) {
ewah_set(si->replace_bitmap, i);
ce->ce_flags |= CE_STRIP_NAME;
ALLOC_GROW(entries, nr_entries+1, nr_alloc);
entries[nr_entries++] = ce;
}
if (is_null_oid(&ce->oid))
istate->drop_cache_tree = 1;
}
}
for (i = 0; i < istate->cache_nr; i++) {
ce = istate->cache[i];
if ((!si->base || !ce->index) && !(ce->ce_flags & CE_REMOVE)) {
assert(!(ce->ce_flags & CE_STRIP_NAME));
ALLOC_GROW(entries, nr_entries+1, nr_alloc);
entries[nr_entries++] = ce;
}
ce->ce_flags &= ~CE_MATCHED;
}
si->saved_cache = istate->cache;
si->saved_cache_nr = istate->cache_nr;
istate->cache = entries;
istate->cache_nr = nr_entries;
}
void finish_writing_split_index(struct index_state *istate)
{
struct split_index *si = init_split_index(istate);
ewah_free(si->delete_bitmap);
ewah_free(si->replace_bitmap);
si->delete_bitmap = NULL;
si->replace_bitmap = NULL;
free(istate->cache);
istate->cache = si->saved_cache;
istate->cache_nr = si->saved_cache_nr;
}
void discard_split_index(struct index_state *istate)
{
struct split_index *si = istate->split_index;
if (!si)
return;
istate->split_index = NULL;
si->refcount--;
if (si->refcount)
return;
if (si->base) {
discard_index(si->base);
free(si->base);
}
free(si);
}
void save_or_free_index_entry(struct index_state *istate, struct cache_entry *ce)
{
if (ce->index &&
istate->split_index &&
istate->split_index->base &&
ce->index <= istate->split_index->base->cache_nr &&
ce == istate->split_index->base->cache[ce->index - 1])
ce->ce_flags |= CE_REMOVE;
else
discard_cache_entry(ce);
}
void replace_index_entry_in_base(struct index_state *istate,
struct cache_entry *old_entry,
struct cache_entry *new_entry)
{
if (old_entry->index &&
istate->split_index &&
istate->split_index->base &&
old_entry->index <= istate->split_index->base->cache_nr) {
new_entry->index = old_entry->index;
if (old_entry != istate->split_index->base->cache[new_entry->index - 1])
discard_cache_entry(istate->split_index->base->cache[new_entry->index - 1]);
istate->split_index->base->cache[new_entry->index - 1] = new_entry;
}
}
void add_split_index(struct index_state *istate)
{
if (!istate->split_index) {
init_split_index(istate);
istate->cache_changed |= SPLIT_INDEX_ORDERED;
}
}
void remove_split_index(struct index_state *istate)
{
if (istate->split_index) {
if (istate->split_index->base) {
mem_pool_combine(istate->ce_mem_pool,
istate->split_index->base->ce_mem_pool);
istate->split_index->base->cache_nr = 0;
}
discard_split_index(istate);
istate->cache_changed |= SOMETHING_CHANGED;
}
}
