#include "cache.h"
#include "refs.h"
#include "object-store.h"
#include "cache-tree.h"
#include "mergesort.h"
#include "diff.h"
#include "diffcore.h"
#include "tag.h"
#include "blame.h"
#include "alloc.h"
#include "commit-slab.h"

define_commit_slab(blame_suspects, struct blame_origin *);
static struct blame_suspects blame_suspects;

struct blame_origin *get_blame_suspects(struct commit *commit)
{
struct blame_origin **result;

result = blame_suspects_peek(&blame_suspects, commit);

return result ? *result : NULL;
}

static void set_blame_suspects(struct commit *commit, struct blame_origin *origin)
{
*blame_suspects_at(&blame_suspects, commit) = origin;
}

void blame_origin_decref(struct blame_origin *o)
{
if (o && --o->refcnt <= 0) {
struct blame_origin *p, *l = NULL;
if (o->previous)
blame_origin_decref(o->previous);
free(o->file.ptr);

for (p = get_blame_suspects(o->commit); p; l = p, p = p->next) {
if (p == o) {
if (l)
l->next = p->next;
else
set_blame_suspects(o->commit, p->next);
free(o);
return;
}
}
die("internal error in blame_origin_decref");
}
}







static struct blame_origin *make_origin(struct commit *commit, const char *path)
{
struct blame_origin *o;
FLEX_ALLOC_STR(o, path, path);
o->commit = commit;
o->refcnt = 1;
o->next = get_blame_suspects(commit);
set_blame_suspects(commit, o);
return o;
}





static struct blame_origin *get_origin(struct commit *commit, const char *path)
{
struct blame_origin *o, *l;

for (o = get_blame_suspects(commit), l = NULL; o; l = o, o = o->next) {
if (!strcmp(o->path, path)) {

if (l) {
l->next = o->next;
o->next = get_blame_suspects(commit);
set_blame_suspects(commit, o);
}
return blame_origin_incref(o);
}
}
return make_origin(commit, path);
}



static void verify_working_tree_path(struct repository *r,
struct commit *work_tree, const char *path)
{
struct commit_list *parents;
int pos;

for (parents = work_tree->parents; parents; parents = parents->next) {
const struct object_id *commit_oid = &parents->item->object.oid;
struct object_id blob_oid;
unsigned short mode;

if (!get_tree_entry(r, commit_oid, path, &blob_oid, &mode) &&
oid_object_info(r, &blob_oid, NULL) == OBJ_BLOB)
return;
}

pos = index_name_pos(r->index, path, strlen(path));
if (pos >= 0)
; 
else if (-1 - pos < r->index->cache_nr &&
!strcmp(r->index->cache[-1 - pos]->name, path))
; 
else
die("no such path '%s' in HEAD", path);
}

static struct commit_list **append_parent(struct repository *r,
struct commit_list **tail,
const struct object_id *oid)
{
struct commit *parent;

parent = lookup_commit_reference(r, oid);
if (!parent)
die("no such commit %s", oid_to_hex(oid));
return &commit_list_insert(parent, tail)->next;
}

static void append_merge_parents(struct repository *r,
struct commit_list **tail)
{
int merge_head;
struct strbuf line = STRBUF_INIT;

merge_head = open(git_path_merge_head(r), O_RDONLY);
if (merge_head < 0) {
if (errno == ENOENT)
return;
die("cannot open '%s' for reading",
git_path_merge_head(r));
}

while (!strbuf_getwholeline_fd(&line, merge_head, '\n')) {
struct object_id oid;
if (get_oid_hex(line.buf, &oid))
die("unknown line in '%s': %s",
git_path_merge_head(r), line.buf);
tail = append_parent(r, tail, &oid);
}
close(merge_head);
strbuf_release(&line);
}






static void set_commit_buffer_from_strbuf(struct repository *r,
struct commit *c,
struct strbuf *sb)
{
size_t len;
void *buf = strbuf_detach(sb, &len);
set_commit_buffer(r, c, buf, len);
}





static struct commit *fake_working_tree_commit(struct repository *r,
struct diff_options *opt,
const char *path,
const char *contents_from)
{
struct commit *commit;
struct blame_origin *origin;
struct commit_list **parent_tail, *parent;
struct object_id head_oid;
struct strbuf buf = STRBUF_INIT;
const char *ident;
time_t now;
int len;
struct cache_entry *ce;
unsigned mode;
struct strbuf msg = STRBUF_INIT;

repo_read_index(r);
time(&now);
commit = alloc_commit_node(r);
commit->object.parsed = 1;
commit->date = now;
parent_tail = &commit->parents;

if (!resolve_ref_unsafe("HEAD", RESOLVE_REF_READING, &head_oid, NULL))
die("no such ref: HEAD");

parent_tail = append_parent(r, parent_tail, &head_oid);
append_merge_parents(r, parent_tail);
verify_working_tree_path(r, commit, path);

origin = make_origin(commit, path);

ident = fmt_ident("Not Committed Yet", "not.committed.yet",
WANT_BLANK_IDENT, NULL, 0);
strbuf_addstr(&msg, "tree 0000000000000000000000000000000000000000\n");
for (parent = commit->parents; parent; parent = parent->next)
strbuf_addf(&msg, "parent %s\n",
oid_to_hex(&parent->item->object.oid));
strbuf_addf(&msg,
"author %s\n"
"committer %s\n\n"
"Version of %s from %s\n",
ident, ident, path,
(!contents_from ? path :
(!strcmp(contents_from, "-") ? "standard input" : contents_from)));
set_commit_buffer_from_strbuf(r, commit, &msg);

if (!contents_from || strcmp("-", contents_from)) {
struct stat st;
const char *read_from;
char *buf_ptr;
unsigned long buf_len;

if (contents_from) {
if (stat(contents_from, &st) < 0)
die_errno("Cannot stat '%s'", contents_from);
read_from = contents_from;
}
else {
if (lstat(path, &st) < 0)
die_errno("Cannot lstat '%s'", path);
read_from = path;
}
mode = canon_mode(st.st_mode);

switch (st.st_mode & S_IFMT) {
case S_IFREG:
if (opt->flags.allow_textconv &&
textconv_object(r, read_from, mode, &null_oid, 0, &buf_ptr, &buf_len))
strbuf_attach(&buf, buf_ptr, buf_len, buf_len + 1);
else if (strbuf_read_file(&buf, read_from, st.st_size) != st.st_size)
die_errno("cannot open or read '%s'", read_from);
break;
case S_IFLNK:
if (strbuf_readlink(&buf, read_from, st.st_size) < 0)
die_errno("cannot readlink '%s'", read_from);
break;
default:
die("unsupported file type %s", read_from);
}
}
else {

mode = 0;
if (strbuf_read(&buf, 0, 0) < 0)
die_errno("failed to read from stdin");
}
convert_to_git(r->index, path, buf.buf, buf.len, &buf, 0);
origin->file.ptr = buf.buf;
origin->file.size = buf.len;
pretend_object_file(buf.buf, buf.len, OBJ_BLOB, &origin->blob_oid);







discard_index(r->index);
repo_read_index(r);

len = strlen(path);
if (!mode) {
int pos = index_name_pos(r->index, path, len);
if (0 <= pos)
mode = r->index->cache[pos]->ce_mode;
else

mode = S_IFREG | 0644;
}
ce = make_empty_cache_entry(r->index, len);
oidcpy(&ce->oid, &origin->blob_oid);
memcpy(ce->name, path, len);
ce->ce_flags = create_ce_flags(0);
ce->ce_namelen = len;
ce->ce_mode = create_ce_mode(mode);
add_index_entry(r->index, ce,
ADD_CACHE_OK_TO_ADD | ADD_CACHE_OK_TO_REPLACE);

cache_tree_invalidate_path(r->index, path);

return commit;
}



static int diff_hunks(mmfile_t *file_a, mmfile_t *file_b,
xdl_emit_hunk_consume_func_t hunk_func, void *cb_data, int xdl_opts)
{
xpparam_t xpp = {0};
xdemitconf_t xecfg = {0};
xdemitcb_t ecb = {NULL};

xpp.flags = xdl_opts;
xecfg.hunk_func = hunk_func;
ecb.priv = cb_data;
return xdi_diff(file_a, file_b, &xpp, &xecfg, &ecb);
}

static const char *get_next_line(const char *start, const char *end)
{
const char *nl = memchr(start, '\n', end - start);

return nl ? nl + 1 : end;
}

static int find_line_starts(int **line_starts, const char *buf,
unsigned long len)
{
const char *end = buf + len;
const char *p;
int *lineno;
int num = 0;

for (p = buf; p < end; p = get_next_line(p, end))
num++;

ALLOC_ARRAY(*line_starts, num + 1);
lineno = *line_starts;

for (p = buf; p < end; p = get_next_line(p, end))
*lineno++ = p - buf;

*lineno = len;

return num;
}

struct fingerprint_entry;




















struct fingerprint {
struct hashmap map;




struct fingerprint_entry *entries;
};




struct fingerprint_entry {



struct hashmap_entry entry;



int count;
};







static void get_fingerprint(struct fingerprint *result,
const char *line_begin,
const char *line_end)
{
unsigned int hash, c0 = 0, c1;
const char *p;
int max_map_entry_count = 1 + line_end - line_begin;
struct fingerprint_entry *entry = xcalloc(max_map_entry_count,
sizeof(struct fingerprint_entry));
struct fingerprint_entry *found_entry;

hashmap_init(&result->map, NULL, NULL, max_map_entry_count);
result->entries = entry;
for (p = line_begin; p <= line_end; ++p, c0 = c1) {





if ((p == line_end) || isspace(*p))
c1 = 0;
else
c1 = tolower(*p);
hash = c0 | (c1 << 8);

if (hash == 0)
continue;
hashmap_entry_init(&entry->entry, hash);

found_entry = hashmap_get_entry(&result->map, entry,
entry, NULL);
if (found_entry) {
found_entry->count += 1;
} else {
entry->count = 1;
hashmap_add(&result->map, &entry->entry);
++entry;
}
}
}

static void free_fingerprint(struct fingerprint *f)
{
hashmap_free(&f->map);
free(f->entries);
}








static int fingerprint_similarity(struct fingerprint *a, struct fingerprint *b)
{
int intersection = 0;
struct hashmap_iter iter;
const struct fingerprint_entry *entry_a, *entry_b;

hashmap_for_each_entry(&b->map, &iter, entry_b,
entry ) {
entry_a = hashmap_get_entry(&a->map, entry_b, entry, NULL);
if (entry_a) {
intersection += entry_a->count < entry_b->count ?
entry_a->count : entry_b->count;
}
}
return intersection;
}



static void fingerprint_subtract(struct fingerprint *a, struct fingerprint *b)
{
struct hashmap_iter iter;
struct fingerprint_entry *entry_a;
const struct fingerprint_entry *entry_b;

hashmap_iter_init(&b->map, &iter);

hashmap_for_each_entry(&b->map, &iter, entry_b,
entry ) {
entry_a = hashmap_get_entry(&a->map, entry_b, entry, NULL);
if (entry_a) {
if (entry_a->count <= entry_b->count)
hashmap_remove(&a->map, &entry_b->entry, NULL);
else
entry_a->count -= entry_b->count;
}
}
}





static void get_line_fingerprints(struct fingerprint *fingerprints,
const char *content, const int *line_starts,
long first_line, long line_count)
{
int i;
const char *linestart, *lineend;

line_starts += first_line;
for (i = 0; i < line_count; ++i) {
linestart = content + line_starts[i];
lineend = content + line_starts[i + 1];
get_fingerprint(fingerprints + i, linestart, lineend);
}
}

static void free_line_fingerprints(struct fingerprint *fingerprints,
int nr_fingerprints)
{
int i;

for (i = 0; i < nr_fingerprints; i++)
free_fingerprint(&fingerprints[i]);
}





struct line_number_mapping {
int destination_start, destination_length,
source_start, source_length;
};














static int map_line_number(int line_number,
const struct line_number_mapping *mapping)
{
return ((line_number - mapping->source_start) * 2 + 1) *
mapping->destination_length /
(mapping->source_length * 2) +
mapping->destination_start;
}






































static int *get_similarity(int *similarities,
int line_a, int local_line_b,
int closest_line_a, int max_search_distance_a)
{
assert(abs(line_a - closest_line_a) <=
max_search_distance_a);
return similarities + line_a - closest_line_a +
max_search_distance_a +
local_line_b * (max_search_distance_a * 2 + 1);
}

#define CERTAIN_NOTHING_MATCHES -2
#define CERTAINTY_NOT_CALCULATED -1

























static void find_best_line_matches(
int start_a,
int length_a,
int start_b,
int local_line_b,
struct fingerprint *fingerprints_a,
struct fingerprint *fingerprints_b,
int *similarities,
int *certainties,
int *second_best_result,
int *result,
const int max_search_distance_a,
const struct line_number_mapping *map_line_number_in_b_to_a)
{

int i, search_start, search_end, closest_local_line_a, *similarity,
best_similarity = 0, second_best_similarity = 0,
best_similarity_index = 0, second_best_similarity_index = 0;


if (certainties[local_line_b] != CERTAINTY_NOT_CALCULATED)
return;

closest_local_line_a = map_line_number(
local_line_b + start_b, map_line_number_in_b_to_a) - start_a;

search_start = closest_local_line_a - max_search_distance_a;
if (search_start < 0)
search_start = 0;

search_end = closest_local_line_a + max_search_distance_a + 1;
if (search_end > length_a)
search_end = length_a;

for (i = search_start; i < search_end; ++i) {
similarity = get_similarity(similarities,
i, local_line_b,
closest_local_line_a,
max_search_distance_a);
if (*similarity == -1) {



assert(abs(i - closest_local_line_a) < 1000);




*similarity = fingerprint_similarity(
fingerprints_b + local_line_b,
fingerprints_a + i) *
(1000 - abs(i - closest_local_line_a));
}
if (*similarity > best_similarity) {
second_best_similarity = best_similarity;
second_best_similarity_index = best_similarity_index;
best_similarity = *similarity;
best_similarity_index = i;
} else if (*similarity > second_best_similarity) {
second_best_similarity = *similarity;
second_best_similarity_index = i;
}
}

if (best_similarity == 0) {




certainties[local_line_b] = CERTAIN_NOTHING_MATCHES;
result[local_line_b] = -1;
} else {












certainties[local_line_b] = best_similarity * 2 -
second_best_similarity;





result[local_line_b] = start_a + best_similarity_index;
second_best_result[local_line_b] =
start_a + second_best_similarity_index;
}
}

































static void fuzzy_find_matching_lines_recurse(
int start_a, int start_b,
int length_a, int length_b,
struct fingerprint *fingerprints_a,
struct fingerprint *fingerprints_b,
int *similarities,
int *certainties,
int *second_best_result,
int *result,
int max_search_distance_a,
int max_search_distance_b,
const struct line_number_mapping *map_line_number_in_b_to_a)
{
int i, invalidate_min, invalidate_max, offset_b,
second_half_start_a, second_half_start_b,
second_half_length_a, second_half_length_b,
most_certain_line_a, most_certain_local_line_b = -1,
most_certain_line_certainty = -1,
closest_local_line_a;

for (i = 0; i < length_b; ++i) {
find_best_line_matches(start_a,
length_a,
start_b,
i,
fingerprints_a,
fingerprints_b,
similarities,
certainties,
second_best_result,
result,
max_search_distance_a,
map_line_number_in_b_to_a);

if (certainties[i] > most_certain_line_certainty) {
most_certain_line_certainty = certainties[i];
most_certain_local_line_b = i;
}
}


if (most_certain_local_line_b == -1)
return;

most_certain_line_a = result[most_certain_local_line_b];






fingerprint_subtract(fingerprints_a + most_certain_line_a - start_a,
fingerprints_b + most_certain_local_line_b);




invalidate_min = most_certain_local_line_b - max_search_distance_b;
invalidate_max = most_certain_local_line_b + max_search_distance_b + 1;
if (invalidate_min < 0)
invalidate_min = 0;
if (invalidate_max > length_b)
invalidate_max = length_b;




for (i = invalidate_min; i < invalidate_max; ++i) {
closest_local_line_a = map_line_number(
i + start_b, map_line_number_in_b_to_a) - start_a;




if (abs(most_certain_line_a - start_a - closest_local_line_a) >
max_search_distance_a) {
continue;
}

*get_similarity(similarities, most_certain_line_a - start_a,
i, closest_local_line_a,
max_search_distance_a) = -1;
}







for (i = most_certain_local_line_b - 1; i >= invalidate_min; --i) {




if (certainties[i] >= 0 &&
(result[i] >= most_certain_line_a ||
second_best_result[i] >= most_certain_line_a)) {
certainties[i] = CERTAINTY_NOT_CALCULATED;
}
}
for (i = most_certain_local_line_b + 1; i < invalidate_max; ++i) {




if (certainties[i] >= 0 &&
(result[i] <= most_certain_line_a ||
second_best_result[i] <= most_certain_line_a)) {
certainties[i] = CERTAINTY_NOT_CALCULATED;
}
}



if (most_certain_local_line_b > 0) {
fuzzy_find_matching_lines_recurse(
start_a, start_b,
most_certain_line_a + 1 - start_a,
most_certain_local_line_b,
fingerprints_a, fingerprints_b, similarities,
certainties, second_best_result, result,
max_search_distance_a,
max_search_distance_b,
map_line_number_in_b_to_a);
}


if (most_certain_local_line_b + 1 < length_b) {
second_half_start_a = most_certain_line_a;
offset_b = most_certain_local_line_b + 1;
second_half_start_b = start_b + offset_b;
second_half_length_a =
length_a + start_a - second_half_start_a;
second_half_length_b =
length_b + start_b - second_half_start_b;
fuzzy_find_matching_lines_recurse(
second_half_start_a, second_half_start_b,
second_half_length_a, second_half_length_b,
fingerprints_a + second_half_start_a - start_a,
fingerprints_b + offset_b,
similarities +
offset_b * (max_search_distance_a * 2 + 1),
certainties + offset_b,
second_best_result + offset_b, result + offset_b,
max_search_distance_a,
max_search_distance_b,
map_line_number_in_b_to_a);
}
}










static int *fuzzy_find_matching_lines(struct blame_origin *parent,
struct blame_origin *target,
int tlno, int parent_slno, int same,
int parent_len)
{


int start_a = parent_slno;
int length_a = parent_len;
int start_b = tlno;
int length_b = same - tlno;

struct line_number_mapping map_line_number_in_b_to_a = {
start_a, length_a, start_b, length_b
};

struct fingerprint *fingerprints_a = parent->fingerprints;
struct fingerprint *fingerprints_b = target->fingerprints;

int i, *result, *second_best_result,
*certainties, *similarities, similarity_count;











int max_search_distance_a = 10, max_search_distance_b;

if (length_a <= 0)
return NULL;

if (max_search_distance_a >= length_a)
max_search_distance_a = length_a ? length_a - 1 : 0;

max_search_distance_b = ((2 * max_search_distance_a + 1) * length_b
- 1) / length_a;

result = xcalloc(sizeof(int), length_b);
second_best_result = xcalloc(sizeof(int), length_b);
certainties = xcalloc(sizeof(int), length_b);


similarity_count = length_b * (max_search_distance_a * 2 + 1);
similarities = xcalloc(sizeof(int), similarity_count);

for (i = 0; i < length_b; ++i) {
result[i] = -1;
second_best_result[i] = -1;
certainties[i] = CERTAINTY_NOT_CALCULATED;
}

for (i = 0; i < similarity_count; ++i)
similarities[i] = -1;

fuzzy_find_matching_lines_recurse(start_a, start_b,
length_a, length_b,
fingerprints_a + start_a,
fingerprints_b + start_b,
similarities,
certainties,
second_best_result,
result,
max_search_distance_a,
max_search_distance_b,
&map_line_number_in_b_to_a);

free(similarities);
free(certainties);
free(second_best_result);

return result;
}

static void fill_origin_fingerprints(struct blame_origin *o)
{
int *line_starts;

if (o->fingerprints)
return;
o->num_lines = find_line_starts(&line_starts, o->file.ptr,
o->file.size);
o->fingerprints = xcalloc(sizeof(struct fingerprint), o->num_lines);
get_line_fingerprints(o->fingerprints, o->file.ptr, line_starts,
0, o->num_lines);
free(line_starts);
}

static void drop_origin_fingerprints(struct blame_origin *o)
{
if (o->fingerprints) {
free_line_fingerprints(o->fingerprints, o->num_lines);
o->num_lines = 0;
FREE_AND_NULL(o->fingerprints);
}
}





static void fill_origin_blob(struct diff_options *opt,
struct blame_origin *o, mmfile_t *file,
int *num_read_blob, int fill_fingerprints)
{
if (!o->file.ptr) {
enum object_type type;
unsigned long file_size;

(*num_read_blob)++;
if (opt->flags.allow_textconv &&
textconv_object(opt->repo, o->path, o->mode,
&o->blob_oid, 1, &file->ptr, &file_size))
;
else
file->ptr = read_object_file(&o->blob_oid, &type,
&file_size);
file->size = file_size;

if (!file->ptr)
die("Cannot read blob %s for path %s",
oid_to_hex(&o->blob_oid),
o->path);
o->file = *file;
}
else
*file = o->file;
if (fill_fingerprints)
fill_origin_fingerprints(o);
}

static void drop_origin_blob(struct blame_origin *o)
{
FREE_AND_NULL(o->file.ptr);
drop_origin_fingerprints(o);
}









static struct blame_entry *blame_merge(struct blame_entry *list1,
struct blame_entry *list2)
{
struct blame_entry *p1 = list1, *p2 = list2,
**tail = &list1;

if (!p1)
return p2;
if (!p2)
return p1;

if (p1->s_lno <= p2->s_lno) {
do {
tail = &p1->next;
if ((p1 = *tail) == NULL) {
*tail = p2;
return list1;
}
} while (p1->s_lno <= p2->s_lno);
}
for (;;) {
*tail = p2;
do {
tail = &p2->next;
if ((p2 = *tail) == NULL) {
*tail = p1;
return list1;
}
} while (p1->s_lno > p2->s_lno);
*tail = p1;
do {
tail = &p1->next;
if ((p1 = *tail) == NULL) {
*tail = p2;
return list1;
}
} while (p1->s_lno <= p2->s_lno);
}
}

static void *get_next_blame(const void *p)
{
return ((struct blame_entry *)p)->next;
}

static void set_next_blame(void *p1, void *p2)
{
((struct blame_entry *)p1)->next = p2;
}






static int compare_blame_final(const void *p1, const void *p2)
{
return ((struct blame_entry *)p1)->lno > ((struct blame_entry *)p2)->lno
? 1 : -1;
}

static int compare_blame_suspect(const void *p1, const void *p2)
{
const struct blame_entry *s1 = p1, *s2 = p2;







if (s1->suspect != s2->suspect)
return (intptr_t)s1->suspect > (intptr_t)s2->suspect ? 1 : -1;
if (s1->s_lno == s2->s_lno)
return 0;
return s1->s_lno > s2->s_lno ? 1 : -1;
}

void blame_sort_final(struct blame_scoreboard *sb)
{
sb->ent = llist_mergesort(sb->ent, get_next_blame, set_next_blame,
compare_blame_final);
}

static int compare_commits_by_reverse_commit_date(const void *a,
const void *b,
void *c)
{
return -compare_commits_by_commit_date(a, b, c);
}





static void sanity_check_refcnt(struct blame_scoreboard *sb)
{
int baa = 0;
struct blame_entry *ent;

for (ent = sb->ent; ent; ent = ent->next) {

if (ent->suspect->refcnt <= 0) {
fprintf(stderr, "%s in %s has negative refcnt %d\n",
ent->suspect->path,
oid_to_hex(&ent->suspect->commit->object.oid),
ent->suspect->refcnt);
baa = 1;
}
}
if (baa)
sb->on_sanity_fail(sb, baa);
}






void blame_coalesce(struct blame_scoreboard *sb)
{
struct blame_entry *ent, *next;

for (ent = sb->ent; ent && (next = ent->next); ent = next) {
if (ent->suspect == next->suspect &&
ent->s_lno + ent->num_lines == next->s_lno &&
ent->ignored == next->ignored &&
ent->unblamable == next->unblamable) {
ent->num_lines += next->num_lines;
ent->next = next->next;
blame_origin_decref(next->suspect);
free(next);
ent->score = 0;
next = ent; 
}
}

if (sb->debug) 
sanity_check_refcnt(sb);
}







static void queue_blames(struct blame_scoreboard *sb, struct blame_origin *porigin,
struct blame_entry *sorted)
{
if (porigin->suspects)
porigin->suspects = blame_merge(porigin->suspects, sorted);
else {
struct blame_origin *o;
for (o = get_blame_suspects(porigin->commit); o; o = o->next) {
if (o->suspects) {
porigin->suspects = sorted;
return;
}
}
porigin->suspects = sorted;
prio_queue_put(&sb->commits, porigin->commit);
}
}










static int fill_blob_sha1_and_mode(struct repository *r,
struct blame_origin *origin)
{
if (!is_null_oid(&origin->blob_oid))
return 0;
if (get_tree_entry(r, &origin->commit->object.oid, origin->path, &origin->blob_oid, &origin->mode))
goto error_out;
if (oid_object_info(r, &origin->blob_oid, NULL) != OBJ_BLOB)
goto error_out;
return 0;
error_out:
oidclr(&origin->blob_oid);
origin->mode = S_IFINVALID;
return -1;
}





static struct blame_origin *find_origin(struct repository *r,
struct commit *parent,
struct blame_origin *origin)
{
struct blame_origin *porigin;
struct diff_options diff_opts;
const char *paths[2];


for (porigin = get_blame_suspects(parent); porigin; porigin = porigin->next)
if (!strcmp(porigin->path, origin->path)) {




return blame_origin_incref (porigin);
}





repo_diff_setup(r, &diff_opts);
diff_opts.flags.recursive = 1;
diff_opts.detect_rename = 0;
diff_opts.output_format = DIFF_FORMAT_NO_OUTPUT;
paths[0] = origin->path;
paths[1] = NULL;

parse_pathspec(&diff_opts.pathspec,
PATHSPEC_ALL_MAGIC & ~PATHSPEC_LITERAL,
PATHSPEC_LITERAL_PATH, "", paths);
diff_setup_done(&diff_opts);

if (is_null_oid(&origin->commit->object.oid))
do_diff_cache(get_commit_tree_oid(parent), &diff_opts);
else
diff_tree_oid(get_commit_tree_oid(parent),
get_commit_tree_oid(origin->commit),
"", &diff_opts);
diffcore_std(&diff_opts);

if (!diff_queued_diff.nr) {

porigin = get_origin(parent, origin->path);
oidcpy(&porigin->blob_oid, &origin->blob_oid);
porigin->mode = origin->mode;
} else {






int i;
struct diff_filepair *p = NULL;
for (i = 0; i < diff_queued_diff.nr; i++) {
const char *name;
p = diff_queued_diff.queue[i];
name = p->one->path ? p->one->path : p->two->path;
if (!strcmp(name, origin->path))
break;
}
if (!p)
die("internal error in blame::find_origin");
switch (p->status) {
default:
die("internal error in blame::find_origin (%c)",
p->status);
case 'M':
porigin = get_origin(parent, origin->path);
oidcpy(&porigin->blob_oid, &p->one->oid);
porigin->mode = p->one->mode;
break;
case 'A':
case 'T':

break;
}
}
diff_flush(&diff_opts);
clear_pathspec(&diff_opts.pathspec);
return porigin;
}





static struct blame_origin *find_rename(struct repository *r,
struct commit *parent,
struct blame_origin *origin)
{
struct blame_origin *porigin = NULL;
struct diff_options diff_opts;
int i;

repo_diff_setup(r, &diff_opts);
diff_opts.flags.recursive = 1;
diff_opts.detect_rename = DIFF_DETECT_RENAME;
diff_opts.output_format = DIFF_FORMAT_NO_OUTPUT;
diff_opts.single_follow = origin->path;
diff_setup_done(&diff_opts);

if (is_null_oid(&origin->commit->object.oid))
do_diff_cache(get_commit_tree_oid(parent), &diff_opts);
else
diff_tree_oid(get_commit_tree_oid(parent),
get_commit_tree_oid(origin->commit),
"", &diff_opts);
diffcore_std(&diff_opts);

for (i = 0; i < diff_queued_diff.nr; i++) {
struct diff_filepair *p = diff_queued_diff.queue[i];
if ((p->status == 'R' || p->status == 'C') &&
!strcmp(p->two->path, origin->path)) {
porigin = get_origin(parent, p->one->path);
oidcpy(&porigin->blob_oid, &p->one->oid);
porigin->mode = p->one->mode;
break;
}
}
diff_flush(&diff_opts);
clear_pathspec(&diff_opts.pathspec);
return porigin;
}




static void add_blame_entry(struct blame_entry ***queue,
const struct blame_entry *src)
{
struct blame_entry *e = xmalloc(sizeof(*e));
memcpy(e, src, sizeof(*e));
blame_origin_incref(e->suspect);

e->next = **queue;
**queue = e;
*queue = &e->next;
}






static void dup_entry(struct blame_entry ***queue,
struct blame_entry *dst, struct blame_entry *src)
{
blame_origin_incref(src->suspect);
blame_origin_decref(dst->suspect);
memcpy(dst, src, sizeof(*src));
dst->next = **queue;
**queue = dst;
*queue = &dst->next;
}

const char *blame_nth_line(struct blame_scoreboard *sb, long lno)
{
return sb->final_buf + sb->lineno[lno];
}















static void split_overlap(struct blame_entry *split,
struct blame_entry *e,
int tlno, int plno, int same,
struct blame_origin *parent)
{
int chunk_end_lno;
int i;
memset(split, 0, sizeof(struct blame_entry [3]));

for (i = 0; i < 3; i++) {
split[i].ignored = e->ignored;
split[i].unblamable = e->unblamable;
}

if (e->s_lno < tlno) {

split[0].suspect = blame_origin_incref(e->suspect);
split[0].lno = e->lno;
split[0].s_lno = e->s_lno;
split[0].num_lines = tlno - e->s_lno;
split[1].lno = e->lno + tlno - e->s_lno;
split[1].s_lno = plno;
}
else {
split[1].lno = e->lno;
split[1].s_lno = plno + (e->s_lno - tlno);
}

if (same < e->s_lno + e->num_lines) {

split[2].suspect = blame_origin_incref(e->suspect);
split[2].lno = e->lno + (same - e->s_lno);
split[2].s_lno = e->s_lno + (same - e->s_lno);
split[2].num_lines = e->s_lno + e->num_lines - same;
chunk_end_lno = split[2].lno;
}
else
chunk_end_lno = e->lno + e->num_lines;
split[1].num_lines = chunk_end_lno - split[1].lno;





if (split[1].num_lines < 1)
return;
split[1].suspect = blame_origin_incref(parent);
}






static void split_blame(struct blame_entry ***blamed,
struct blame_entry ***unblamed,
struct blame_entry *split,
struct blame_entry *e)
{
if (split[0].suspect && split[2].suspect) {

dup_entry(unblamed, e, &split[0]);


add_blame_entry(unblamed, &split[2]);


add_blame_entry(blamed, &split[1]);
}
else if (!split[0].suspect && !split[2].suspect)




dup_entry(blamed, e, &split[1]);
else if (split[0].suspect) {

dup_entry(unblamed, e, &split[0]);
add_blame_entry(blamed, &split[1]);
}
else {

dup_entry(blamed, e, &split[1]);
add_blame_entry(unblamed, &split[2]);
}
}





static void decref_split(struct blame_entry *split)
{
int i;

for (i = 0; i < 3; i++)
blame_origin_decref(split[i].suspect);
}











static struct blame_entry *reverse_blame(struct blame_entry *head,
struct blame_entry *tail)
{
while (head) {
struct blame_entry *next = head->next;
head->next = tail;
tail = head;
head = next;
}
return tail;
}








static struct blame_entry *split_blame_at(struct blame_entry *e, int len,
struct blame_origin *new_suspect)
{
struct blame_entry *n = xcalloc(1, sizeof(struct blame_entry));

n->suspect = new_suspect;
n->ignored = e->ignored;
n->unblamable = e->unblamable;
n->lno = e->lno + len;
n->s_lno = e->s_lno + len;
n->num_lines = e->num_lines - len;
e->num_lines = len;
e->score = 0;
return n;
}

struct blame_line_tracker {
int is_parent;
int s_lno;
};

static int are_lines_adjacent(struct blame_line_tracker *first,
struct blame_line_tracker *second)
{
return first->is_parent == second->is_parent &&
first->s_lno + 1 == second->s_lno;
}

static int scan_parent_range(struct fingerprint *p_fps,
struct fingerprint *t_fps, int t_idx,
int from, int nr_lines)
{
int sim, p_idx;
#define FINGERPRINT_FILE_THRESHOLD 10
int best_sim_val = FINGERPRINT_FILE_THRESHOLD;
int best_sim_idx = -1;

for (p_idx = from; p_idx < from + nr_lines; p_idx++) {
sim = fingerprint_similarity(&t_fps[t_idx], &p_fps[p_idx]);
if (sim < best_sim_val)
continue;

if (sim == best_sim_val && best_sim_idx != -1 &&
abs(best_sim_idx - t_idx) < abs(p_idx - t_idx))
continue;
best_sim_val = sim;
best_sim_idx = p_idx;
}
return best_sim_idx;
}







static void guess_line_blames(struct blame_origin *parent,
struct blame_origin *target,
int tlno, int offset, int same, int parent_len,
struct blame_line_tracker *line_blames)
{
int i, best_idx, target_idx;
int parent_slno = tlno + offset;
int *fuzzy_matches;

fuzzy_matches = fuzzy_find_matching_lines(parent, target,
tlno, parent_slno, same,
parent_len);
for (i = 0; i < same - tlno; i++) {
target_idx = tlno + i;
if (fuzzy_matches && fuzzy_matches[i] >= 0) {
best_idx = fuzzy_matches[i];
} else {
best_idx = scan_parent_range(parent->fingerprints,
target->fingerprints,
target_idx, 0,
parent->num_lines);
}
if (best_idx >= 0) {
line_blames[i].is_parent = 1;
line_blames[i].s_lno = best_idx;
} else {
line_blames[i].is_parent = 0;
line_blames[i].s_lno = target_idx;
}
}
free(fuzzy_matches);
}











static void ignore_blame_entry(struct blame_entry *e,
struct blame_origin *parent,
struct blame_entry **diffp,
struct blame_entry **ignoredp,
struct blame_line_tracker *line_blames)
{
int entry_len, nr_lines, i;






entry_len = 1;
nr_lines = e->num_lines; 
for (i = 0; i < nr_lines; i++) {
struct blame_entry *next = NULL;





if (i + 1 < nr_lines) {
if (are_lines_adjacent(&line_blames[i],
&line_blames[i + 1])) {
entry_len++;
continue;
}
next = split_blame_at(e, entry_len,
blame_origin_incref(e->suspect));
}
if (line_blames[i].is_parent) {
e->ignored = 1;
blame_origin_decref(e->suspect);
e->suspect = blame_origin_incref(parent);
e->s_lno = line_blames[i - entry_len + 1].s_lno;
e->next = *ignoredp;
*ignoredp = e;
} else {
e->unblamable = 1;

e->next = *diffp;
*diffp = e;
}
assert(e->num_lines == entry_len);
e = next;
entry_len = 1;
}
assert(!e);
}
















static void blame_chunk(struct blame_entry ***dstq, struct blame_entry ***srcq,
int tlno, int offset, int same, int parent_len,
struct blame_origin *parent,
struct blame_origin *target, int ignore_diffs)
{
struct blame_entry *e = **srcq;
struct blame_entry *samep = NULL, *diffp = NULL, *ignoredp = NULL;
struct blame_line_tracker *line_blames = NULL;

while (e && e->s_lno < tlno) {
struct blame_entry *next = e->next;





if (e->s_lno + e->num_lines > tlno) {

struct blame_entry *n;

n = split_blame_at(e, tlno - e->s_lno, e->suspect);

n->next = diffp;
diffp = n;
} else
blame_origin_decref(e->suspect);


e->suspect = blame_origin_incref(parent);
e->s_lno += offset;
e->next = samep;
samep = e;
e = next;
}






if (samep) {
**dstq = reverse_blame(samep, **dstq);
*dstq = &samep->next;
}




e = reverse_blame(diffp, e);





samep = NULL;
diffp = NULL;

if (ignore_diffs && same - tlno > 0) {
line_blames = xcalloc(sizeof(struct blame_line_tracker),
same - tlno);
guess_line_blames(parent, target, tlno, offset, same,
parent_len, line_blames);
}

while (e && e->s_lno < same) {
struct blame_entry *next = e->next;




if (e->s_lno + e->num_lines > same) {




struct blame_entry *n;

n = split_blame_at(e, same - e->s_lno,
blame_origin_incref(e->suspect));

n->next = samep;
samep = n;
}
if (ignore_diffs) {
ignore_blame_entry(e, parent, &diffp, &ignoredp,
line_blames + e->s_lno - tlno);
} else {
e->next = diffp;
diffp = e;
}
e = next;
}
free(line_blames);
if (ignoredp) {









**dstq = reverse_blame(ignoredp, **dstq);
*dstq = &ignoredp->next;
}
**srcq = reverse_blame(diffp, reverse_blame(samep, e));

if (diffp)
*srcq = &diffp->next;
}

struct blame_chunk_cb_data {
struct blame_origin *parent;
struct blame_origin *target;
long offset;
int ignore_diffs;
struct blame_entry **dstq;
struct blame_entry **srcq;
};


static int blame_chunk_cb(long start_a, long count_a,
long start_b, long count_b, void *data)
{
struct blame_chunk_cb_data *d = data;
if (start_a - start_b != d->offset)
die("internal error in blame::blame_chunk_cb");
blame_chunk(&d->dstq, &d->srcq, start_b, start_a - start_b,
start_b + count_b, count_a, d->parent, d->target,
d->ignore_diffs);
d->offset = start_a + count_a - (start_b + count_b);
return 0;
}






static void pass_blame_to_parent(struct blame_scoreboard *sb,
struct blame_origin *target,
struct blame_origin *parent, int ignore_diffs)
{
mmfile_t file_p, file_o;
struct blame_chunk_cb_data d;
struct blame_entry *newdest = NULL;

if (!target->suspects)
return; 

d.parent = parent;
d.target = target;
d.offset = 0;
d.ignore_diffs = ignore_diffs;
d.dstq = &newdest; d.srcq = &target->suspects;

fill_origin_blob(&sb->revs->diffopt, parent, &file_p,
&sb->num_read_blob, ignore_diffs);
fill_origin_blob(&sb->revs->diffopt, target, &file_o,
&sb->num_read_blob, ignore_diffs);
sb->num_get_patch++;

if (diff_hunks(&file_p, &file_o, blame_chunk_cb, &d, sb->xdl_opts))
die("unable to generate diff (%s -> %s)",
oid_to_hex(&parent->commit->object.oid),
oid_to_hex(&target->commit->object.oid));

blame_chunk(&d.dstq, &d.srcq, INT_MAX, d.offset, INT_MAX, 0,
parent, target, 0);
*d.dstq = NULL;
if (ignore_diffs)
newdest = llist_mergesort(newdest, get_next_blame,
set_next_blame,
compare_blame_suspect);
queue_blames(sb, parent, newdest);

return;
}










unsigned blame_entry_score(struct blame_scoreboard *sb, struct blame_entry *e)
{
unsigned score;
const char *cp, *ep;

if (e->score)
return e->score;

score = 1;
cp = blame_nth_line(sb, e->lno);
ep = blame_nth_line(sb, e->lno + e->num_lines);
while (cp < ep) {
unsigned ch = *((unsigned char *)cp);
if (isalnum(ch))
score++;
cp++;
}
e->score = score;
return score;
}







static void copy_split_if_better(struct blame_scoreboard *sb,
struct blame_entry *best_so_far,
struct blame_entry *potential)
{
int i;

if (!potential[1].suspect)
return;
if (best_so_far[1].suspect) {
if (blame_entry_score(sb, &potential[1]) <
blame_entry_score(sb, &best_so_far[1]))
return;
}

for (i = 0; i < 3; i++)
blame_origin_incref(potential[i].suspect);
decref_split(best_so_far);
memcpy(best_so_far, potential, sizeof(struct blame_entry[3]));
}
















static void handle_split(struct blame_scoreboard *sb,
struct blame_entry *ent,
int tlno, int plno, int same,
struct blame_origin *parent,
struct blame_entry *split)
{
if (ent->num_lines <= tlno)
return;
if (tlno < same) {
struct blame_entry potential[3];
tlno += ent->s_lno;
same += ent->s_lno;
split_overlap(potential, ent, tlno, plno, same, parent);
copy_split_if_better(sb, split, potential);
decref_split(potential);
}
}

struct handle_split_cb_data {
struct blame_scoreboard *sb;
struct blame_entry *ent;
struct blame_origin *parent;
struct blame_entry *split;
long plno;
long tlno;
};

static int handle_split_cb(long start_a, long count_a,
long start_b, long count_b, void *data)
{
struct handle_split_cb_data *d = data;
handle_split(d->sb, d->ent, d->tlno, d->plno, start_b, d->parent,
d->split);
d->plno = start_a + count_a;
d->tlno = start_b + count_b;
return 0;
}






static void find_copy_in_blob(struct blame_scoreboard *sb,
struct blame_entry *ent,
struct blame_origin *parent,
struct blame_entry *split,
mmfile_t *file_p)
{
const char *cp;
mmfile_t file_o;
struct handle_split_cb_data d;

memset(&d, 0, sizeof(d));
d.sb = sb; d.ent = ent; d.parent = parent; d.split = split;



cp = blame_nth_line(sb, ent->lno);
file_o.ptr = (char *) cp;
file_o.size = blame_nth_line(sb, ent->lno + ent->num_lines) - cp;





memset(split, 0, sizeof(struct blame_entry [3]));
if (diff_hunks(file_p, &file_o, handle_split_cb, &d, sb->xdl_opts))
die("unable to generate diff (%s)",
oid_to_hex(&parent->commit->object.oid));

handle_split(sb, ent, d.tlno, d.plno, ent->num_lines, parent, split);
}






static struct blame_entry **filter_small(struct blame_scoreboard *sb,
struct blame_entry **small,
struct blame_entry **source,
unsigned score_min)
{
struct blame_entry *p = *source;
struct blame_entry *oldsmall = *small;
while (p) {
if (blame_entry_score(sb, p) <= score_min) {
*small = p;
small = &p->next;
p = *small;
} else {
*source = p;
source = &p->next;
p = *source;
}
}
*small = oldsmall;
*source = NULL;
return small;
}





static void find_move_in_parent(struct blame_scoreboard *sb,
struct blame_entry ***blamed,
struct blame_entry **toosmall,
struct blame_origin *target,
struct blame_origin *parent)
{
struct blame_entry *e, split[3];
struct blame_entry *unblamed = target->suspects;
struct blame_entry *leftover = NULL;
mmfile_t file_p;

if (!unblamed)
return; 

fill_origin_blob(&sb->revs->diffopt, parent, &file_p,
&sb->num_read_blob, 0);
if (!file_p.ptr)
return;






do {
struct blame_entry **unblamedtail = &unblamed;
struct blame_entry *next;
for (e = unblamed; e; e = next) {
next = e->next;
find_copy_in_blob(sb, e, parent, split, &file_p);
if (split[1].suspect &&
sb->move_score < blame_entry_score(sb, &split[1])) {
split_blame(blamed, &unblamedtail, split, e);
} else {
e->next = leftover;
leftover = e;
}
decref_split(split);
}
*unblamedtail = NULL;
toosmall = filter_small(sb, toosmall, &unblamed, sb->move_score);
} while (unblamed);
target->suspects = reverse_blame(leftover, NULL);
}

struct blame_list {
struct blame_entry *ent;
struct blame_entry split[3];
};





static struct blame_list *setup_blame_list(struct blame_entry *unblamed,
int *num_ents_p)
{
struct blame_entry *e;
int num_ents, i;
struct blame_list *blame_list = NULL;

for (e = unblamed, num_ents = 0; e; e = e->next)
num_ents++;
if (num_ents) {
blame_list = xcalloc(num_ents, sizeof(struct blame_list));
for (e = unblamed, i = 0; e; e = e->next)
blame_list[i++].ent = e;
}
*num_ents_p = num_ents;
return blame_list;
}






static void find_copy_in_parent(struct blame_scoreboard *sb,
struct blame_entry ***blamed,
struct blame_entry **toosmall,
struct blame_origin *target,
struct commit *parent,
struct blame_origin *porigin,
int opt)
{
struct diff_options diff_opts;
int i, j;
struct blame_list *blame_list;
int num_ents;
struct blame_entry *unblamed = target->suspects;
struct blame_entry *leftover = NULL;

if (!unblamed)
return; 

repo_diff_setup(sb->repo, &diff_opts);
diff_opts.flags.recursive = 1;
diff_opts.output_format = DIFF_FORMAT_NO_OUTPUT;

diff_setup_done(&diff_opts);








if ((opt & PICKAXE_BLAME_COPY_HARDEST)
|| ((opt & PICKAXE_BLAME_COPY_HARDER)
&& (!porigin || strcmp(target->path, porigin->path))))
diff_opts.flags.find_copies_harder = 1;

if (is_null_oid(&target->commit->object.oid))
do_diff_cache(get_commit_tree_oid(parent), &diff_opts);
else
diff_tree_oid(get_commit_tree_oid(parent),
get_commit_tree_oid(target->commit),
"", &diff_opts);

if (!diff_opts.flags.find_copies_harder)
diffcore_std(&diff_opts);

do {
struct blame_entry **unblamedtail = &unblamed;
blame_list = setup_blame_list(unblamed, &num_ents);

for (i = 0; i < diff_queued_diff.nr; i++) {
struct diff_filepair *p = diff_queued_diff.queue[i];
struct blame_origin *norigin;
mmfile_t file_p;
struct blame_entry potential[3];

if (!DIFF_FILE_VALID(p->one))
continue; 
if (S_ISGITLINK(p->one->mode))
continue; 
if (porigin && !strcmp(p->one->path, porigin->path))

continue;

norigin = get_origin(parent, p->one->path);
oidcpy(&norigin->blob_oid, &p->one->oid);
norigin->mode = p->one->mode;
fill_origin_blob(&sb->revs->diffopt, norigin, &file_p,
&sb->num_read_blob, 0);
if (!file_p.ptr)
continue;

for (j = 0; j < num_ents; j++) {
find_copy_in_blob(sb, blame_list[j].ent,
norigin, potential, &file_p);
copy_split_if_better(sb, blame_list[j].split,
potential);
decref_split(potential);
}
blame_origin_decref(norigin);
}

for (j = 0; j < num_ents; j++) {
struct blame_entry *split = blame_list[j].split;
if (split[1].suspect &&
sb->copy_score < blame_entry_score(sb, &split[1])) {
split_blame(blamed, &unblamedtail, split,
blame_list[j].ent);
} else {
blame_list[j].ent->next = leftover;
leftover = blame_list[j].ent;
}
decref_split(split);
}
free(blame_list);
*unblamedtail = NULL;
toosmall = filter_small(sb, toosmall, &unblamed, sb->copy_score);
} while (unblamed);
target->suspects = reverse_blame(leftover, NULL);
diff_flush(&diff_opts);
clear_pathspec(&diff_opts.pathspec);
}





static void pass_whole_blame(struct blame_scoreboard *sb,
struct blame_origin *origin, struct blame_origin *porigin)
{
struct blame_entry *e, *suspects;

if (!porigin->file.ptr && origin->file.ptr) {

porigin->file = origin->file;
origin->file.ptr = NULL;
}
suspects = origin->suspects;
origin->suspects = NULL;
for (e = suspects; e; e = e->next) {
blame_origin_incref(porigin);
blame_origin_decref(e->suspect);
e->suspect = porigin;
}
queue_blames(sb, porigin, suspects);
}






static struct commit_list *first_scapegoat(struct rev_info *revs, struct commit *commit,
int reverse)
{
if (!reverse) {
if (revs->first_parent_only &&
commit->parents &&
commit->parents->next) {
free_commit_list(commit->parents->next);
commit->parents->next = NULL;
}
return commit->parents;
}
return lookup_decoration(&revs->children, &commit->object);
}

static int num_scapegoats(struct rev_info *revs, struct commit *commit, int reverse)
{
struct commit_list *l = first_scapegoat(revs, commit, reverse);
return commit_list_count(l);
}




static void distribute_blame(struct blame_scoreboard *sb, struct blame_entry *blamed)
{
blamed = llist_mergesort(blamed, get_next_blame, set_next_blame,
compare_blame_suspect);
while (blamed)
{
struct blame_origin *porigin = blamed->suspect;
struct blame_entry *suspects = NULL;
do {
struct blame_entry *next = blamed->next;
blamed->next = suspects;
suspects = blamed;
blamed = next;
} while (blamed && blamed->suspect == porigin);
suspects = reverse_blame(suspects, NULL);
queue_blames(sb, porigin, suspects);
}
}

#define MAXSG 16

static void pass_blame(struct blame_scoreboard *sb, struct blame_origin *origin, int opt)
{
struct rev_info *revs = sb->revs;
int i, pass, num_sg;
struct commit *commit = origin->commit;
struct commit_list *sg;
struct blame_origin *sg_buf[MAXSG];
struct blame_origin *porigin, **sg_origin = sg_buf;
struct blame_entry *toosmall = NULL;
struct blame_entry *blames, **blametail = &blames;

num_sg = num_scapegoats(revs, commit, sb->reverse);
if (!num_sg)
goto finish;
else if (num_sg < ARRAY_SIZE(sg_buf))
memset(sg_buf, 0, sizeof(sg_buf));
else
sg_origin = xcalloc(num_sg, sizeof(*sg_origin));





for (pass = 0; pass < 2 - sb->no_whole_file_rename; pass++) {
struct blame_origin *(*find)(struct repository *, struct commit *, struct blame_origin *);
find = pass ? find_rename : find_origin;

for (i = 0, sg = first_scapegoat(revs, commit, sb->reverse);
i < num_sg && sg;
sg = sg->next, i++) {
struct commit *p = sg->item;
int j, same;

if (sg_origin[i])
continue;
if (parse_commit(p))
continue;
porigin = find(sb->repo, p, origin);
if (!porigin)
continue;
if (oideq(&porigin->blob_oid, &origin->blob_oid)) {
pass_whole_blame(sb, origin, porigin);
blame_origin_decref(porigin);
goto finish;
}
for (j = same = 0; j < i; j++)
if (sg_origin[j] &&
oideq(&sg_origin[j]->blob_oid, &porigin->blob_oid)) {
same = 1;
break;
}
if (!same)
sg_origin[i] = porigin;
else
blame_origin_decref(porigin);
}
}

sb->num_commits++;
for (i = 0, sg = first_scapegoat(revs, commit, sb->reverse);
i < num_sg && sg;
sg = sg->next, i++) {
struct blame_origin *porigin = sg_origin[i];
if (!porigin)
continue;
if (!origin->previous) {
blame_origin_incref(porigin);
origin->previous = porigin;
}
pass_blame_to_parent(sb, origin, porigin, 0);
if (!origin->suspects)
goto finish;
}




if (oidset_contains(&sb->ignore_list, &commit->object.oid)) {
for (i = 0, sg = first_scapegoat(revs, commit, sb->reverse);
i < num_sg && sg;
sg = sg->next, i++) {
struct blame_origin *porigin = sg_origin[i];

if (!porigin)
continue;
pass_blame_to_parent(sb, origin, porigin, 1);





drop_origin_blob(porigin);
if (!origin->suspects)
goto finish;
}
}




if (opt & PICKAXE_BLAME_MOVE) {
filter_small(sb, &toosmall, &origin->suspects, sb->move_score);
if (origin->suspects) {
for (i = 0, sg = first_scapegoat(revs, commit, sb->reverse);
i < num_sg && sg;
sg = sg->next, i++) {
struct blame_origin *porigin = sg_origin[i];
if (!porigin)
continue;
find_move_in_parent(sb, &blametail, &toosmall, origin, porigin);
if (!origin->suspects)
break;
}
}
}




if (opt & PICKAXE_BLAME_COPY) {
if (sb->copy_score > sb->move_score)
filter_small(sb, &toosmall, &origin->suspects, sb->copy_score);
else if (sb->copy_score < sb->move_score) {
origin->suspects = blame_merge(origin->suspects, toosmall);
toosmall = NULL;
filter_small(sb, &toosmall, &origin->suspects, sb->copy_score);
}
if (!origin->suspects)
goto finish;

for (i = 0, sg = first_scapegoat(revs, commit, sb->reverse);
i < num_sg && sg;
sg = sg->next, i++) {
struct blame_origin *porigin = sg_origin[i];
find_copy_in_parent(sb, &blametail, &toosmall,
origin, sg->item, porigin, opt);
if (!origin->suspects)
goto finish;
}
}

finish:
*blametail = NULL;
distribute_blame(sb, blames);






if (toosmall) {
struct blame_entry **tail = &toosmall;
while (*tail)
tail = &(*tail)->next;
*tail = origin->suspects;
origin->suspects = toosmall;
}
for (i = 0; i < num_sg; i++) {
if (sg_origin[i]) {
if (!sg_origin[i]->suspects)
drop_origin_blob(sg_origin[i]);
blame_origin_decref(sg_origin[i]);
}
}
drop_origin_blob(origin);
if (sg_buf != sg_origin)
free(sg_origin);
}





void assign_blame(struct blame_scoreboard *sb, int opt)
{
struct rev_info *revs = sb->revs;
struct commit *commit = prio_queue_get(&sb->commits);

while (commit) {
struct blame_entry *ent;
struct blame_origin *suspect = get_blame_suspects(commit);


while (suspect && !suspect->suspects)
suspect = suspect->next;

if (!suspect) {
commit = prio_queue_get(&sb->commits);
continue;
}

assert(commit == suspect->commit);





blame_origin_incref(suspect);
parse_commit(commit);
if (sb->reverse ||
(!(commit->object.flags & UNINTERESTING) &&
!(revs->max_age != -1 && commit->date < revs->max_age)))
pass_blame(sb, suspect, opt);
else {
commit->object.flags |= UNINTERESTING;
if (commit->object.parsed)
mark_parents_uninteresting(commit);
}

if (!commit->parents && !sb->show_root)
commit->object.flags |= UNINTERESTING;


ent = suspect->suspects;
if (ent) {
suspect->guilty = 1;
for (;;) {
struct blame_entry *next = ent->next;
if (sb->found_guilty_entry)
sb->found_guilty_entry(ent, sb->found_guilty_entry_data);
if (next) {
ent = next;
continue;
}
ent->next = sb->ent;
sb->ent = suspect->suspects;
suspect->suspects = NULL;
break;
}
}
blame_origin_decref(suspect);

if (sb->debug) 
sanity_check_refcnt(sb);
}
}





static int prepare_lines(struct blame_scoreboard *sb)
{
sb->num_lines = find_line_starts(&sb->lineno, sb->final_buf,
sb->final_buf_size);
return sb->num_lines;
}

static struct commit *find_single_final(struct rev_info *revs,
const char **name_p)
{
int i;
struct commit *found = NULL;
const char *name = NULL;

for (i = 0; i < revs->pending.nr; i++) {
struct object *obj = revs->pending.objects[i].item;
if (obj->flags & UNINTERESTING)
continue;
obj = deref_tag(revs->repo, obj, NULL, 0);
if (obj->type != OBJ_COMMIT)
die("Non commit %s?", revs->pending.objects[i].name);
if (found)
die("More than one commit to dig from %s and %s?",
revs->pending.objects[i].name, name);
found = (struct commit *)obj;
name = revs->pending.objects[i].name;
}
if (name_p)
*name_p = xstrdup_or_null(name);
return found;
}

static struct commit *dwim_reverse_initial(struct rev_info *revs,
const char **name_p)
{





struct object *obj;
struct commit *head_commit;
struct object_id head_oid;

if (revs->pending.nr != 1)
return NULL;


obj = revs->pending.objects[0].item;
obj = deref_tag(revs->repo, obj, NULL, 0);
if (obj->type != OBJ_COMMIT)
return NULL;


if (!resolve_ref_unsafe("HEAD", RESOLVE_REF_READING, &head_oid, NULL))
return NULL;
head_commit = lookup_commit_reference_gently(revs->repo,
&head_oid, 1);
if (!head_commit)
return NULL;


obj->flags |= UNINTERESTING;
add_pending_object(revs, &head_commit->object, "HEAD");

if (name_p)
*name_p = revs->pending.objects[0].name;
return (struct commit *)obj;
}

static struct commit *find_single_initial(struct rev_info *revs,
const char **name_p)
{
int i;
struct commit *found = NULL;
const char *name = NULL;





for (i = 0; i < revs->pending.nr; i++) {
struct object *obj = revs->pending.objects[i].item;
if (!(obj->flags & UNINTERESTING))
continue;
obj = deref_tag(revs->repo, obj, NULL, 0);
if (obj->type != OBJ_COMMIT)
die("Non commit %s?", revs->pending.objects[i].name);
if (found)
die("More than one commit to dig up from, %s and %s?",
revs->pending.objects[i].name, name);
found = (struct commit *) obj;
name = revs->pending.objects[i].name;
}

if (!name)
found = dwim_reverse_initial(revs, &name);
if (!name)
die("No commit to dig up from?");

if (name_p)
*name_p = xstrdup(name);
return found;
}

void init_scoreboard(struct blame_scoreboard *sb)
{
memset(sb, 0, sizeof(struct blame_scoreboard));
sb->move_score = BLAME_DEFAULT_MOVE_SCORE;
sb->copy_score = BLAME_DEFAULT_COPY_SCORE;
}

void setup_scoreboard(struct blame_scoreboard *sb,
const char *path,
struct blame_origin **orig)
{
const char *final_commit_name = NULL;
struct blame_origin *o;
struct commit *final_commit = NULL;
enum object_type type;

init_blame_suspects(&blame_suspects);

if (sb->reverse && sb->contents_from)
die(_("--contents and --reverse do not blend well."));

if (!sb->repo)
BUG("repo is NULL");

if (!sb->reverse) {
sb->final = find_single_final(sb->revs, &final_commit_name);
sb->commits.compare = compare_commits_by_commit_date;
} else {
sb->final = find_single_initial(sb->revs, &final_commit_name);
sb->commits.compare = compare_commits_by_reverse_commit_date;
}

if (sb->final && sb->contents_from)
die(_("cannot use --contents with final commit object name"));

if (sb->reverse && sb->revs->first_parent_only)
sb->revs->children.name = NULL;

if (!sb->final) {





setup_work_tree();
sb->final = fake_working_tree_commit(sb->repo,
&sb->revs->diffopt,
path, sb->contents_from);
add_pending_object(sb->revs, &(sb->final->object), ":");
}

if (sb->reverse && sb->revs->first_parent_only) {
final_commit = find_single_final(sb->revs, NULL);
if (!final_commit)
die(_("--reverse and --first-parent together require specified latest commit"));
}






if (prepare_revision_walk(sb->revs))
die(_("revision walk setup failed"));

if (sb->reverse && sb->revs->first_parent_only) {
struct commit *c = final_commit;

sb->revs->children.name = "children";
while (c->parents &&
!oideq(&c->object.oid, &sb->final->object.oid)) {
struct commit_list *l = xcalloc(1, sizeof(*l));

l->item = c;
if (add_decoration(&sb->revs->children,
&c->parents->item->object, l))
BUG("not unique item in first-parent chain");
c = c->parents->item;
}

if (!oideq(&c->object.oid, &sb->final->object.oid))
die(_("--reverse --first-parent together require range along first-parent chain"));
}

if (is_null_oid(&sb->final->object.oid)) {
o = get_blame_suspects(sb->final);
sb->final_buf = xmemdupz(o->file.ptr, o->file.size);
sb->final_buf_size = o->file.size;
}
else {
o = get_origin(sb->final, path);
if (fill_blob_sha1_and_mode(sb->repo, o))
die(_("no such path %s in %s"), path, final_commit_name);

if (sb->revs->diffopt.flags.allow_textconv &&
textconv_object(sb->repo, path, o->mode, &o->blob_oid, 1, (char **) &sb->final_buf,
&sb->final_buf_size))
;
else
sb->final_buf = read_object_file(&o->blob_oid, &type,
&sb->final_buf_size);

if (!sb->final_buf)
die(_("cannot read blob %s for path %s"),
oid_to_hex(&o->blob_oid),
path);
}
sb->num_read_blob++;
prepare_lines(sb);

if (orig)
*orig = o;

free((char *)final_commit_name);
}



struct blame_entry *blame_entry_prepend(struct blame_entry *head,
long start, long end,
struct blame_origin *o)
{
struct blame_entry *new_head = xcalloc(1, sizeof(struct blame_entry));
new_head->lno = start;
new_head->num_lines = end - start;
new_head->suspect = o;
new_head->s_lno = start;
new_head->next = head;
blame_origin_incref(o);
return new_head;
}
