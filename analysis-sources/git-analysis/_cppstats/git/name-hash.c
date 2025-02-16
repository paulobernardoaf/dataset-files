#include "cache.h"
#include "thread-utils.h"
struct dir_entry {
struct hashmap_entry ent;
struct dir_entry *parent;
int nr;
unsigned int namelen;
char name[FLEX_ARRAY];
};
static int dir_entry_cmp(const void *unused_cmp_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *keydata)
{
const struct dir_entry *e1, *e2;
const char *name = keydata;
e1 = container_of(eptr, const struct dir_entry, ent);
e2 = container_of(entry_or_key, const struct dir_entry, ent);
return e1->namelen != e2->namelen || strncasecmp(e1->name,
name ? name : e2->name, e1->namelen);
}
static struct dir_entry *find_dir_entry__hash(struct index_state *istate,
const char *name, unsigned int namelen, unsigned int hash)
{
struct dir_entry key;
hashmap_entry_init(&key.ent, hash);
key.namelen = namelen;
return hashmap_get_entry(&istate->dir_hash, &key, ent, name);
}
static struct dir_entry *find_dir_entry(struct index_state *istate,
const char *name, unsigned int namelen)
{
return find_dir_entry__hash(istate, name, namelen, memihash(name, namelen));
}
static struct dir_entry *hash_dir_entry(struct index_state *istate,
struct cache_entry *ce, int namelen)
{
struct dir_entry *dir;
while (namelen > 0 && !is_dir_sep(ce->name[namelen - 1]))
namelen--;
if (namelen <= 0)
return NULL;
namelen--;
dir = find_dir_entry(istate, ce->name, namelen);
if (!dir) {
FLEX_ALLOC_MEM(dir, name, ce->name, namelen);
hashmap_entry_init(&dir->ent, memihash(ce->name, namelen));
dir->namelen = namelen;
hashmap_add(&istate->dir_hash, &dir->ent);
dir->parent = hash_dir_entry(istate, ce, namelen);
}
return dir;
}
static void add_dir_entry(struct index_state *istate, struct cache_entry *ce)
{
struct dir_entry *dir = hash_dir_entry(istate, ce, ce_namelen(ce));
while (dir && !(dir->nr++))
dir = dir->parent;
}
static void remove_dir_entry(struct index_state *istate, struct cache_entry *ce)
{
struct dir_entry *dir = hash_dir_entry(istate, ce, ce_namelen(ce));
while (dir && !(--dir->nr)) {
struct dir_entry *parent = dir->parent;
hashmap_remove(&istate->dir_hash, &dir->ent, NULL);
free(dir);
dir = parent;
}
}
static void hash_index_entry(struct index_state *istate, struct cache_entry *ce)
{
if (ce->ce_flags & CE_HASHED)
return;
ce->ce_flags |= CE_HASHED;
hashmap_entry_init(&ce->ent, memihash(ce->name, ce_namelen(ce)));
hashmap_add(&istate->name_hash, &ce->ent);
if (ignore_case)
add_dir_entry(istate, ce);
}
static int cache_entry_cmp(const void *unused_cmp_data,
const struct hashmap_entry *eptr,
const struct hashmap_entry *entry_or_key,
const void *remove)
{
const struct cache_entry *ce1, *ce2;
ce1 = container_of(eptr, const struct cache_entry, ent);
ce2 = container_of(entry_or_key, const struct cache_entry, ent);
return remove ? !(ce1 == ce2) : 0;
}
static int lazy_try_threaded = 1;
static int lazy_nr_dir_threads;
#define LAZY_THREAD_COST (2000)
#define LAZY_MAX_MUTEX (32)
static pthread_mutex_t *lazy_dir_mutex_array;
struct lazy_entry {
struct dir_entry *dir;
unsigned int hash_dir;
unsigned int hash_name;
};
static int lookup_lazy_params(struct index_state *istate)
{
int nr_cpus;
lazy_nr_dir_threads = 0;
if (!lazy_try_threaded)
return 0;
if (!ignore_case)
return 0;
nr_cpus = online_cpus();
if (nr_cpus < 2)
return 0;
if (istate->cache_nr < 2 * LAZY_THREAD_COST)
return 0;
if (istate->cache_nr < nr_cpus * LAZY_THREAD_COST)
nr_cpus = istate->cache_nr / LAZY_THREAD_COST;
lazy_nr_dir_threads = nr_cpus;
return lazy_nr_dir_threads;
}
static void init_dir_mutex(void)
{
int j;
lazy_dir_mutex_array = xcalloc(LAZY_MAX_MUTEX, sizeof(pthread_mutex_t));
for (j = 0; j < LAZY_MAX_MUTEX; j++)
init_recursive_mutex(&lazy_dir_mutex_array[j]);
}
static void cleanup_dir_mutex(void)
{
int j;
for (j = 0; j < LAZY_MAX_MUTEX; j++)
pthread_mutex_destroy(&lazy_dir_mutex_array[j]);
free(lazy_dir_mutex_array);
}
static void lock_dir_mutex(int j)
{
pthread_mutex_lock(&lazy_dir_mutex_array[j]);
}
static void unlock_dir_mutex(int j)
{
pthread_mutex_unlock(&lazy_dir_mutex_array[j]);
}
static inline int compute_dir_lock_nr(
const struct hashmap *map,
unsigned int hash)
{
return hashmap_bucket(map, hash) % LAZY_MAX_MUTEX;
}
static struct dir_entry *hash_dir_entry_with_parent_and_prefix(
struct index_state *istate,
struct dir_entry *parent,
struct strbuf *prefix)
{
struct dir_entry *dir;
unsigned int hash;
int lock_nr;
assert((parent != NULL) ^ (strchr(prefix->buf, '/') == NULL));
if (parent)
hash = memihash_cont(parent->ent.hash,
prefix->buf + parent->namelen,
prefix->len - parent->namelen);
else
hash = memihash(prefix->buf, prefix->len);
lock_nr = compute_dir_lock_nr(&istate->dir_hash, hash);
lock_dir_mutex(lock_nr);
dir = find_dir_entry__hash(istate, prefix->buf, prefix->len, hash);
if (!dir) {
FLEX_ALLOC_MEM(dir, name, prefix->buf, prefix->len);
hashmap_entry_init(&dir->ent, hash);
dir->namelen = prefix->len;
dir->parent = parent;
hashmap_add(&istate->dir_hash, &dir->ent);
if (parent) {
unlock_dir_mutex(lock_nr);
lock_nr = compute_dir_lock_nr(&istate->dir_hash, parent->ent.hash);
lock_dir_mutex(lock_nr);
parent->nr++;
}
}
unlock_dir_mutex(lock_nr);
return dir;
}
static int handle_range_1(
struct index_state *istate,
int k_start,
int k_end,
struct dir_entry *parent,
struct strbuf *prefix,
struct lazy_entry *lazy_entries);
static int handle_range_dir(
struct index_state *istate,
int k_start,
int k_end,
struct dir_entry *parent,
struct strbuf *prefix,
struct lazy_entry *lazy_entries,
struct dir_entry **dir_new_out)
{
int rc, k;
int input_prefix_len = prefix->len;
struct dir_entry *dir_new;
dir_new = hash_dir_entry_with_parent_and_prefix(istate, parent, prefix);
strbuf_addch(prefix, '/');
if (k_start + 1 >= k_end)
k = k_end;
else if (strncmp(istate->cache[k_start + 1]->name, prefix->buf, prefix->len) > 0)
k = k_start + 1;
else if (strncmp(istate->cache[k_end - 1]->name, prefix->buf, prefix->len) == 0)
k = k_end;
else {
int begin = k_start;
int end = k_end;
assert(begin >= 0);
while (begin < end) {
int mid = begin + ((end - begin) >> 1);
int cmp = strncmp(istate->cache[mid]->name, prefix->buf, prefix->len);
if (cmp == 0) 
begin = mid + 1;
else if (cmp > 0) 
end = mid;
else
die("cache entry out of order");
}
k = begin;
}
rc = handle_range_1(istate, k_start, k, dir_new, prefix, lazy_entries);
strbuf_setlen(prefix, input_prefix_len);
*dir_new_out = dir_new;
return rc;
}
static int handle_range_1(
struct index_state *istate,
int k_start,
int k_end,
struct dir_entry *parent,
struct strbuf *prefix,
struct lazy_entry *lazy_entries)
{
int input_prefix_len = prefix->len;
int k = k_start;
while (k < k_end) {
struct cache_entry *ce_k = istate->cache[k];
const char *name, *slash;
if (prefix->len && strncmp(ce_k->name, prefix->buf, prefix->len))
break;
name = ce_k->name + prefix->len;
slash = strchr(name, '/');
if (slash) {
int len = slash - name;
int processed;
struct dir_entry *dir_new;
strbuf_add(prefix, name, len);
processed = handle_range_dir(istate, k, k_end, parent, prefix, lazy_entries, &dir_new);
if (processed) {
k += processed;
strbuf_setlen(prefix, input_prefix_len);
continue;
}
strbuf_addch(prefix, '/');
processed = handle_range_1(istate, k, k_end, dir_new, prefix, lazy_entries);
k += processed;
strbuf_setlen(prefix, input_prefix_len);
continue;
}
lazy_entries[k].dir = parent;
if (parent) {
lazy_entries[k].hash_name = memihash_cont(
parent->ent.hash,
ce_k->name + parent->namelen,
ce_namelen(ce_k) - parent->namelen);
lazy_entries[k].hash_dir = parent->ent.hash;
} else {
lazy_entries[k].hash_name = memihash(ce_k->name, ce_namelen(ce_k));
}
k++;
}
return k - k_start;
}
struct lazy_dir_thread_data {
pthread_t pthread;
struct index_state *istate;
struct lazy_entry *lazy_entries;
int k_start;
int k_end;
};
static void *lazy_dir_thread_proc(void *_data)
{
struct lazy_dir_thread_data *d = _data;
struct strbuf prefix = STRBUF_INIT;
handle_range_1(d->istate, d->k_start, d->k_end, NULL, &prefix, d->lazy_entries);
strbuf_release(&prefix);
return NULL;
}
struct lazy_name_thread_data {
pthread_t pthread;
struct index_state *istate;
struct lazy_entry *lazy_entries;
};
static void *lazy_name_thread_proc(void *_data)
{
struct lazy_name_thread_data *d = _data;
int k;
for (k = 0; k < d->istate->cache_nr; k++) {
struct cache_entry *ce_k = d->istate->cache[k];
ce_k->ce_flags |= CE_HASHED;
hashmap_entry_init(&ce_k->ent, d->lazy_entries[k].hash_name);
hashmap_add(&d->istate->name_hash, &ce_k->ent);
}
return NULL;
}
static inline void lazy_update_dir_ref_counts(
struct index_state *istate,
struct lazy_entry *lazy_entries)
{
int k;
for (k = 0; k < istate->cache_nr; k++) {
if (lazy_entries[k].dir)
lazy_entries[k].dir->nr++;
}
}
static void threaded_lazy_init_name_hash(
struct index_state *istate)
{
int err;
int nr_each;
int k_start;
int t;
struct lazy_entry *lazy_entries;
struct lazy_dir_thread_data *td_dir;
struct lazy_name_thread_data *td_name;
if (!HAVE_THREADS)
return;
k_start = 0;
nr_each = DIV_ROUND_UP(istate->cache_nr, lazy_nr_dir_threads);
lazy_entries = xcalloc(istate->cache_nr, sizeof(struct lazy_entry));
td_dir = xcalloc(lazy_nr_dir_threads, sizeof(struct lazy_dir_thread_data));
td_name = xcalloc(1, sizeof(struct lazy_name_thread_data));
init_dir_mutex();
for (t = 0; t < lazy_nr_dir_threads; t++) {
struct lazy_dir_thread_data *td_dir_t = td_dir + t;
td_dir_t->istate = istate;
td_dir_t->lazy_entries = lazy_entries;
td_dir_t->k_start = k_start;
k_start += nr_each;
if (k_start > istate->cache_nr)
k_start = istate->cache_nr;
td_dir_t->k_end = k_start;
err = pthread_create(&td_dir_t->pthread, NULL, lazy_dir_thread_proc, td_dir_t);
if (err)
die(_("unable to create lazy_dir thread: %s"), strerror(err));
}
for (t = 0; t < lazy_nr_dir_threads; t++) {
struct lazy_dir_thread_data *td_dir_t = td_dir + t;
if (pthread_join(td_dir_t->pthread, NULL))
die("unable to join lazy_dir_thread");
}
td_name->istate = istate;
td_name->lazy_entries = lazy_entries;
err = pthread_create(&td_name->pthread, NULL, lazy_name_thread_proc, td_name);
if (err)
die(_("unable to create lazy_name thread: %s"), strerror(err));
lazy_update_dir_ref_counts(istate, lazy_entries);
err = pthread_join(td_name->pthread, NULL);
if (err)
die(_("unable to join lazy_name thread: %s"), strerror(err));
cleanup_dir_mutex();
free(td_name);
free(td_dir);
free(lazy_entries);
}
static void lazy_init_name_hash(struct index_state *istate)
{
if (istate->name_hash_initialized)
return;
trace_performance_enter();
hashmap_init(&istate->name_hash, cache_entry_cmp, NULL, istate->cache_nr);
hashmap_init(&istate->dir_hash, dir_entry_cmp, NULL, istate->cache_nr);
if (lookup_lazy_params(istate)) {
hashmap_disable_item_counting(&istate->dir_hash);
threaded_lazy_init_name_hash(istate);
hashmap_enable_item_counting(&istate->dir_hash);
} else {
int nr;
for (nr = 0; nr < istate->cache_nr; nr++)
hash_index_entry(istate, istate->cache[nr]);
}
istate->name_hash_initialized = 1;
trace_performance_leave("initialize name hash");
}
int test_lazy_init_name_hash(struct index_state *istate, int try_threaded)
{
lazy_nr_dir_threads = 0;
lazy_try_threaded = try_threaded;
lazy_init_name_hash(istate);
return lazy_nr_dir_threads;
}
void add_name_hash(struct index_state *istate, struct cache_entry *ce)
{
if (istate->name_hash_initialized)
hash_index_entry(istate, ce);
}
void remove_name_hash(struct index_state *istate, struct cache_entry *ce)
{
if (!istate->name_hash_initialized || !(ce->ce_flags & CE_HASHED))
return;
ce->ce_flags &= ~CE_HASHED;
hashmap_remove(&istate->name_hash, &ce->ent, ce);
if (ignore_case)
remove_dir_entry(istate, ce);
}
static int slow_same_name(const char *name1, int len1, const char *name2, int len2)
{
if (len1 != len2)
return 0;
while (len1) {
unsigned char c1 = *name1++;
unsigned char c2 = *name2++;
len1--;
if (c1 != c2) {
c1 = toupper(c1);
c2 = toupper(c2);
if (c1 != c2)
return 0;
}
}
return 1;
}
static int same_name(const struct cache_entry *ce, const char *name, int namelen, int icase)
{
int len = ce_namelen(ce);
if (len == namelen && !memcmp(name, ce->name, len))
return 1;
if (!icase)
return 0;
return slow_same_name(name, namelen, ce->name, len);
}
int index_dir_exists(struct index_state *istate, const char *name, int namelen)
{
struct dir_entry *dir;
lazy_init_name_hash(istate);
dir = find_dir_entry(istate, name, namelen);
return dir && dir->nr;
}
void adjust_dirname_case(struct index_state *istate, char *name)
{
const char *startPtr = name;
const char *ptr = startPtr;
lazy_init_name_hash(istate);
while (*ptr) {
while (*ptr && *ptr != '/')
ptr++;
if (*ptr == '/') {
struct dir_entry *dir;
dir = find_dir_entry(istate, name, ptr - name);
if (dir) {
memcpy((void *)startPtr, dir->name + (startPtr - name), ptr - startPtr);
startPtr = ptr + 1;
}
ptr++;
}
}
}
struct cache_entry *index_file_exists(struct index_state *istate, const char *name, int namelen, int icase)
{
struct cache_entry *ce;
unsigned int hash = memihash(name, namelen);
lazy_init_name_hash(istate);
ce = hashmap_get_entry_from_hash(&istate->name_hash, hash, NULL,
struct cache_entry, ent);
hashmap_for_each_entry_from(&istate->name_hash, ce, ent) {
if (same_name(ce, name, namelen, icase))
return ce;
}
return NULL;
}
void free_name_hash(struct index_state *istate)
{
if (!istate->name_hash_initialized)
return;
istate->name_hash_initialized = 0;
hashmap_free(&istate->name_hash);
hashmap_free_entries(&istate->dir_hash, struct dir_entry, ent);
}
