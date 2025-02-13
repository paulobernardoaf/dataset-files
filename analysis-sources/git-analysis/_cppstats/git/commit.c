#include "cache.h"
#include "tag.h"
#include "commit.h"
#include "commit-graph.h"
#include "repository.h"
#include "object-store.h"
#include "pkt-line.h"
#include "utf8.h"
#include "diff.h"
#include "revision.h"
#include "notes.h"
#include "alloc.h"
#include "gpg-interface.h"
#include "mergesort.h"
#include "commit-slab.h"
#include "prio-queue.h"
#include "sha1-lookup.h"
#include "wt-status.h"
#include "advice.h"
#include "refs.h"
#include "commit-reach.h"
#include "run-command.h"
static struct commit_extra_header *read_commit_extra_header_lines(const char *buf, size_t len, const char **);
int save_commit_buffer = 1;
const char *commit_type = "commit";
struct commit *lookup_commit_reference_gently(struct repository *r,
const struct object_id *oid, int quiet)
{
struct object *obj = deref_tag(r,
parse_object(r, oid),
NULL, 0);
if (!obj)
return NULL;
return object_as_type(r, obj, OBJ_COMMIT, quiet);
}
struct commit *lookup_commit_reference(struct repository *r, const struct object_id *oid)
{
return lookup_commit_reference_gently(r, oid, 0);
}
struct commit *lookup_commit_or_die(const struct object_id *oid, const char *ref_name)
{
struct commit *c = lookup_commit_reference(the_repository, oid);
if (!c)
die(_("could not parse %s"), ref_name);
if (!oideq(oid, &c->object.oid)) {
warning(_("%s %s is not a commit!"),
ref_name, oid_to_hex(oid));
}
return c;
}
struct commit *lookup_commit(struct repository *r, const struct object_id *oid)
{
struct object *obj = lookup_object(r, oid);
if (!obj)
return create_object(r, oid, alloc_commit_node(r));
return object_as_type(r, obj, OBJ_COMMIT, 0);
}
struct commit *lookup_commit_reference_by_name(const char *name)
{
struct object_id oid;
struct commit *commit;
if (get_oid_committish(name, &oid))
return NULL;
commit = lookup_commit_reference(the_repository, &oid);
if (parse_commit(commit))
return NULL;
return commit;
}
static timestamp_t parse_commit_date(const char *buf, const char *tail)
{
const char *dateptr;
if (buf + 6 >= tail)
return 0;
if (memcmp(buf, "author", 6))
return 0;
while (buf < tail && *buf++ != '\n')
;
if (buf + 9 >= tail)
return 0;
if (memcmp(buf, "committer", 9))
return 0;
while (buf < tail && *buf++ != '>')
;
if (buf >= tail)
return 0;
dateptr = buf;
while (buf < tail && *buf++ != '\n')
;
if (buf >= tail)
return 0;
return parse_timestamp(dateptr, NULL, 10);
}
static const unsigned char *commit_graft_sha1_access(size_t index, void *table)
{
struct commit_graft **commit_graft_table = table;
return commit_graft_table[index]->oid.hash;
}
static int commit_graft_pos(struct repository *r, const unsigned char *sha1)
{
return sha1_pos(sha1, r->parsed_objects->grafts,
r->parsed_objects->grafts_nr,
commit_graft_sha1_access);
}
int register_commit_graft(struct repository *r, struct commit_graft *graft,
int ignore_dups)
{
int pos = commit_graft_pos(r, graft->oid.hash);
if (0 <= pos) {
if (ignore_dups)
free(graft);
else {
free(r->parsed_objects->grafts[pos]);
r->parsed_objects->grafts[pos] = graft;
}
return 1;
}
pos = -pos - 1;
ALLOC_GROW(r->parsed_objects->grafts,
r->parsed_objects->grafts_nr + 1,
r->parsed_objects->grafts_alloc);
r->parsed_objects->grafts_nr++;
if (pos < r->parsed_objects->grafts_nr)
memmove(r->parsed_objects->grafts + pos + 1,
r->parsed_objects->grafts + pos,
(r->parsed_objects->grafts_nr - pos - 1) *
sizeof(*r->parsed_objects->grafts));
r->parsed_objects->grafts[pos] = graft;
return 0;
}
struct commit_graft *read_graft_line(struct strbuf *line)
{
int i, phase;
const char *tail = NULL;
struct commit_graft *graft = NULL;
struct object_id dummy_oid, *oid;
strbuf_rtrim(line);
if (!line->len || line->buf[0] == '#')
return NULL;
for (phase = 0; phase < 2; phase++) {
oid = graft ? &graft->oid : &dummy_oid;
if (parse_oid_hex(line->buf, oid, &tail))
goto bad_graft_data;
for (i = 0; *tail != '\0'; i++) {
oid = graft ? &graft->parent[i] : &dummy_oid;
if (!isspace(*tail++) || parse_oid_hex(tail, oid, &tail))
goto bad_graft_data;
}
if (!graft) {
graft = xmalloc(st_add(sizeof(*graft),
st_mult(sizeof(struct object_id), i)));
graft->nr_parent = i;
}
}
return graft;
bad_graft_data:
error("bad graft data: %s", line->buf);
assert(!graft);
return NULL;
}
static int read_graft_file(struct repository *r, const char *graft_file)
{
FILE *fp = fopen_or_warn(graft_file, "r");
struct strbuf buf = STRBUF_INIT;
if (!fp)
return -1;
if (advice_graft_file_deprecated)
advise(_("Support for <GIT_DIR>/info/grafts is deprecated\n"
"and will be removed in a future Git version.\n"
"\n"
"Please use \"git replace --convert-graft-file\"\n"
"to convert the grafts into replace refs.\n"
"\n"
"Turn this message off by running\n"
"\"git config advice.graftFileDeprecated false\""));
while (!strbuf_getwholeline(&buf, fp, '\n')) {
struct commit_graft *graft = read_graft_line(&buf);
if (!graft)
continue;
if (register_commit_graft(r, graft, 1))
error("duplicate graft data: %s", buf.buf);
}
fclose(fp);
strbuf_release(&buf);
return 0;
}
void prepare_commit_graft(struct repository *r)
{
char *graft_file;
if (r->parsed_objects->commit_graft_prepared)
return;
if (!startup_info->have_repository)
return;
graft_file = get_graft_file(r);
read_graft_file(r, graft_file);
is_repository_shallow(r);
r->parsed_objects->commit_graft_prepared = 1;
}
struct commit_graft *lookup_commit_graft(struct repository *r, const struct object_id *oid)
{
int pos;
prepare_commit_graft(r);
pos = commit_graft_pos(r, oid->hash);
if (pos < 0)
return NULL;
return r->parsed_objects->grafts[pos];
}
int for_each_commit_graft(each_commit_graft_fn fn, void *cb_data)
{
int i, ret;
for (i = ret = 0; i < the_repository->parsed_objects->grafts_nr && !ret; i++)
ret = fn(the_repository->parsed_objects->grafts[i], cb_data);
return ret;
}
int unregister_shallow(const struct object_id *oid)
{
int pos = commit_graft_pos(the_repository, oid->hash);
if (pos < 0)
return -1;
if (pos + 1 < the_repository->parsed_objects->grafts_nr)
MOVE_ARRAY(the_repository->parsed_objects->grafts + pos,
the_repository->parsed_objects->grafts + pos + 1,
the_repository->parsed_objects->grafts_nr - pos - 1);
the_repository->parsed_objects->grafts_nr--;
return 0;
}
struct commit_buffer {
void *buffer;
unsigned long size;
};
define_commit_slab(buffer_slab, struct commit_buffer);
struct buffer_slab *allocate_commit_buffer_slab(void)
{
struct buffer_slab *bs = xmalloc(sizeof(*bs));
init_buffer_slab(bs);
return bs;
}
void free_commit_buffer_slab(struct buffer_slab *bs)
{
clear_buffer_slab(bs);
free(bs);
}
void set_commit_buffer(struct repository *r, struct commit *commit, void *buffer, unsigned long size)
{
struct commit_buffer *v = buffer_slab_at(
r->parsed_objects->buffer_slab, commit);
v->buffer = buffer;
v->size = size;
}
const void *get_cached_commit_buffer(struct repository *r, const struct commit *commit, unsigned long *sizep)
{
struct commit_buffer *v = buffer_slab_peek(
r->parsed_objects->buffer_slab, commit);
if (!v) {
if (sizep)
*sizep = 0;
return NULL;
}
if (sizep)
*sizep = v->size;
return v->buffer;
}
const void *repo_get_commit_buffer(struct repository *r,
const struct commit *commit,
unsigned long *sizep)
{
const void *ret = get_cached_commit_buffer(r, commit, sizep);
if (!ret) {
enum object_type type;
unsigned long size;
ret = repo_read_object_file(r, &commit->object.oid, &type, &size);
if (!ret)
die("cannot read commit object %s",
oid_to_hex(&commit->object.oid));
if (type != OBJ_COMMIT)
die("expected commit for %s, got %s",
oid_to_hex(&commit->object.oid), type_name(type));
if (sizep)
*sizep = size;
}
return ret;
}
void repo_unuse_commit_buffer(struct repository *r,
const struct commit *commit,
const void *buffer)
{
struct commit_buffer *v = buffer_slab_peek(
r->parsed_objects->buffer_slab, commit);
if (!(v && v->buffer == buffer))
free((void *)buffer);
}
void free_commit_buffer(struct parsed_object_pool *pool, struct commit *commit)
{
struct commit_buffer *v = buffer_slab_peek(
pool->buffer_slab, commit);
if (v) {
FREE_AND_NULL(v->buffer);
v->size = 0;
}
}
static inline void set_commit_tree(struct commit *c, struct tree *t)
{
c->maybe_tree = t;
}
struct tree *repo_get_commit_tree(struct repository *r,
const struct commit *commit)
{
if (commit->maybe_tree || !commit->object.parsed)
return commit->maybe_tree;
if (commit->graph_pos != COMMIT_NOT_FROM_GRAPH)
return get_commit_tree_in_graph(r, commit);
return NULL;
}
struct object_id *get_commit_tree_oid(const struct commit *commit)
{
struct tree *tree = get_commit_tree(commit);
return tree ? &tree->object.oid : NULL;
}
void release_commit_memory(struct parsed_object_pool *pool, struct commit *c)
{
set_commit_tree(c, NULL);
free_commit_buffer(pool, c);
c->index = 0;
free_commit_list(c->parents);
c->object.parsed = 0;
}
const void *detach_commit_buffer(struct commit *commit, unsigned long *sizep)
{
struct commit_buffer *v = buffer_slab_peek(
the_repository->parsed_objects->buffer_slab, commit);
void *ret;
if (!v) {
if (sizep)
*sizep = 0;
return NULL;
}
ret = v->buffer;
if (sizep)
*sizep = v->size;
v->buffer = NULL;
v->size = 0;
return ret;
}
int parse_commit_buffer(struct repository *r, struct commit *item, const void *buffer, unsigned long size, int check_graph)
{
const char *tail = buffer;
const char *bufptr = buffer;
struct object_id parent;
struct commit_list **pptr;
struct commit_graft *graft;
const int tree_entry_len = the_hash_algo->hexsz + 5;
const int parent_entry_len = the_hash_algo->hexsz + 7;
struct tree *tree;
if (item->object.parsed)
return 0;
if (item->parents) {
free_commit_list(item->parents);
item->parents = NULL;
}
tail += size;
if (tail <= bufptr + tree_entry_len + 1 || memcmp(bufptr, "tree ", 5) ||
bufptr[tree_entry_len] != '\n')
return error("bogus commit object %s", oid_to_hex(&item->object.oid));
if (get_oid_hex(bufptr + 5, &parent) < 0)
return error("bad tree pointer in commit %s",
oid_to_hex(&item->object.oid));
tree = lookup_tree(r, &parent);
if (!tree)
return error("bad tree pointer %s in commit %s",
oid_to_hex(&parent),
oid_to_hex(&item->object.oid));
set_commit_tree(item, tree);
bufptr += tree_entry_len + 1; 
pptr = &item->parents;
graft = lookup_commit_graft(r, &item->object.oid);
while (bufptr + parent_entry_len < tail && !memcmp(bufptr, "parent ", 7)) {
struct commit *new_parent;
if (tail <= bufptr + parent_entry_len + 1 ||
get_oid_hex(bufptr + 7, &parent) ||
bufptr[parent_entry_len] != '\n')
return error("bad parents in commit %s", oid_to_hex(&item->object.oid));
bufptr += parent_entry_len + 1;
if (graft && (graft->nr_parent < 0 || grafts_replace_parents))
continue;
new_parent = lookup_commit(r, &parent);
if (!new_parent)
return error("bad parent %s in commit %s",
oid_to_hex(&parent),
oid_to_hex(&item->object.oid));
pptr = &commit_list_insert(new_parent, pptr)->next;
}
if (graft) {
int i;
struct commit *new_parent;
for (i = 0; i < graft->nr_parent; i++) {
new_parent = lookup_commit(r,
&graft->parent[i]);
if (!new_parent)
return error("bad graft parent %s in commit %s",
oid_to_hex(&graft->parent[i]),
oid_to_hex(&item->object.oid));
pptr = &commit_list_insert(new_parent, pptr)->next;
}
}
item->date = parse_commit_date(bufptr, tail);
if (check_graph)
load_commit_graph_info(r, item);
item->object.parsed = 1;
return 0;
}
int repo_parse_commit_internal(struct repository *r,
struct commit *item,
int quiet_on_missing,
int use_commit_graph)
{
enum object_type type;
void *buffer;
unsigned long size;
int ret;
if (!item)
return -1;
if (item->object.parsed)
return 0;
if (use_commit_graph && parse_commit_in_graph(r, item))
return 0;
buffer = repo_read_object_file(r, &item->object.oid, &type, &size);
if (!buffer)
return quiet_on_missing ? -1 :
error("Could not read %s",
oid_to_hex(&item->object.oid));
if (type != OBJ_COMMIT) {
free(buffer);
return error("Object %s not a commit",
oid_to_hex(&item->object.oid));
}
ret = parse_commit_buffer(r, item, buffer, size, 0);
if (save_commit_buffer && !ret) {
set_commit_buffer(r, item, buffer, size);
return 0;
}
free(buffer);
return ret;
}
int repo_parse_commit_gently(struct repository *r,
struct commit *item, int quiet_on_missing)
{
return repo_parse_commit_internal(r, item, quiet_on_missing, 1);
}
void parse_commit_or_die(struct commit *item)
{
if (parse_commit(item))
die("unable to parse commit %s",
item ? oid_to_hex(&item->object.oid) : "(null)");
}
int find_commit_subject(const char *commit_buffer, const char **subject)
{
const char *eol;
const char *p = commit_buffer;
while (*p && (*p != '\n' || p[1] != '\n'))
p++;
if (*p) {
p = skip_blank_lines(p + 2);
eol = strchrnul(p, '\n');
} else
eol = p;
*subject = p;
return eol - p;
}
struct commit_list *commit_list_insert(struct commit *item, struct commit_list **list_p)
{
struct commit_list *new_list = xmalloc(sizeof(struct commit_list));
new_list->item = item;
new_list->next = *list_p;
*list_p = new_list;
return new_list;
}
unsigned commit_list_count(const struct commit_list *l)
{
unsigned c = 0;
for (; l; l = l->next )
c++;
return c;
}
struct commit_list *copy_commit_list(struct commit_list *list)
{
struct commit_list *head = NULL;
struct commit_list **pp = &head;
while (list) {
pp = commit_list_append(list->item, pp);
list = list->next;
}
return head;
}
void free_commit_list(struct commit_list *list)
{
while (list)
pop_commit(&list);
}
struct commit_list * commit_list_insert_by_date(struct commit *item, struct commit_list **list)
{
struct commit_list **pp = list;
struct commit_list *p;
while ((p = *pp) != NULL) {
if (p->item->date < item->date) {
break;
}
pp = &p->next;
}
return commit_list_insert(item, pp);
}
static int commit_list_compare_by_date(const void *a, const void *b)
{
timestamp_t a_date = ((const struct commit_list *)a)->item->date;
timestamp_t b_date = ((const struct commit_list *)b)->item->date;
if (a_date < b_date)
return 1;
if (a_date > b_date)
return -1;
return 0;
}
static void *commit_list_get_next(const void *a)
{
return ((const struct commit_list *)a)->next;
}
static void commit_list_set_next(void *a, void *next)
{
((struct commit_list *)a)->next = next;
}
void commit_list_sort_by_date(struct commit_list **list)
{
*list = llist_mergesort(*list, commit_list_get_next, commit_list_set_next,
commit_list_compare_by_date);
}
struct commit *pop_most_recent_commit(struct commit_list **list,
unsigned int mark)
{
struct commit *ret = pop_commit(list);
struct commit_list *parents = ret->parents;
while (parents) {
struct commit *commit = parents->item;
if (!parse_commit(commit) && !(commit->object.flags & mark)) {
commit->object.flags |= mark;
commit_list_insert_by_date(commit, list);
}
parents = parents->next;
}
return ret;
}
static void clear_commit_marks_1(struct commit_list **plist,
struct commit *commit, unsigned int mark)
{
while (commit) {
struct commit_list *parents;
if (!(mark & commit->object.flags))
return;
commit->object.flags &= ~mark;
parents = commit->parents;
if (!parents)
return;
while ((parents = parents->next))
commit_list_insert(parents->item, plist);
commit = commit->parents->item;
}
}
void clear_commit_marks_many(int nr, struct commit **commit, unsigned int mark)
{
struct commit_list *list = NULL;
while (nr--) {
clear_commit_marks_1(&list, *commit, mark);
commit++;
}
while (list)
clear_commit_marks_1(&list, pop_commit(&list), mark);
}
void clear_commit_marks(struct commit *commit, unsigned int mark)
{
clear_commit_marks_many(1, &commit, mark);
}
struct commit *pop_commit(struct commit_list **stack)
{
struct commit_list *top = *stack;
struct commit *item = top ? top->item : NULL;
if (top) {
*stack = top->next;
free(top);
}
return item;
}
define_commit_slab(indegree_slab, int);
define_commit_slab(author_date_slab, timestamp_t);
void record_author_date(struct author_date_slab *author_date,
struct commit *commit)
{
const char *buffer = get_commit_buffer(commit, NULL);
struct ident_split ident;
const char *ident_line;
size_t ident_len;
char *date_end;
timestamp_t date;
ident_line = find_commit_header(buffer, "author", &ident_len);
if (!ident_line)
goto fail_exit; 
if (split_ident_line(&ident, ident_line, ident_len) ||
!ident.date_begin || !ident.date_end)
goto fail_exit; 
date = parse_timestamp(ident.date_begin, &date_end, 10);
if (date_end != ident.date_end)
goto fail_exit; 
*(author_date_slab_at(author_date, commit)) = date;
fail_exit:
unuse_commit_buffer(commit, buffer);
}
int compare_commits_by_author_date(const void *a_, const void *b_,
void *cb_data)
{
const struct commit *a = a_, *b = b_;
struct author_date_slab *author_date = cb_data;
timestamp_t a_date = *(author_date_slab_at(author_date, a));
timestamp_t b_date = *(author_date_slab_at(author_date, b));
if (a_date < b_date)
return 1;
else if (a_date > b_date)
return -1;
return 0;
}
int compare_commits_by_gen_then_commit_date(const void *a_, const void *b_, void *unused)
{
const struct commit *a = a_, *b = b_;
if (a->generation < b->generation)
return 1;
else if (a->generation > b->generation)
return -1;
if (a->date < b->date)
return 1;
else if (a->date > b->date)
return -1;
return 0;
}
int compare_commits_by_commit_date(const void *a_, const void *b_, void *unused)
{
const struct commit *a = a_, *b = b_;
if (a->date < b->date)
return 1;
else if (a->date > b->date)
return -1;
return 0;
}
void sort_in_topological_order(struct commit_list **list, enum rev_sort_order sort_order)
{
struct commit_list *next, *orig = *list;
struct commit_list **pptr;
struct indegree_slab indegree;
struct prio_queue queue;
struct commit *commit;
struct author_date_slab author_date;
if (!orig)
return;
*list = NULL;
init_indegree_slab(&indegree);
memset(&queue, '\0', sizeof(queue));
switch (sort_order) {
default: 
queue.compare = NULL;
break;
case REV_SORT_BY_COMMIT_DATE:
queue.compare = compare_commits_by_commit_date;
break;
case REV_SORT_BY_AUTHOR_DATE:
init_author_date_slab(&author_date);
queue.compare = compare_commits_by_author_date;
queue.cb_data = &author_date;
break;
}
for (next = orig; next; next = next->next) {
struct commit *commit = next->item;
*(indegree_slab_at(&indegree, commit)) = 1;
if (sort_order == REV_SORT_BY_AUTHOR_DATE)
record_author_date(&author_date, commit);
}
for (next = orig; next; next = next->next) {
struct commit_list *parents = next->item->parents;
while (parents) {
struct commit *parent = parents->item;
int *pi = indegree_slab_at(&indegree, parent);
if (*pi)
(*pi)++;
parents = parents->next;
}
}
for (next = orig; next; next = next->next) {
struct commit *commit = next->item;
if (*(indegree_slab_at(&indegree, commit)) == 1)
prio_queue_put(&queue, commit);
}
if (sort_order == REV_SORT_IN_GRAPH_ORDER)
prio_queue_reverse(&queue);
free_commit_list(orig);
pptr = list;
*list = NULL;
while ((commit = prio_queue_get(&queue)) != NULL) {
struct commit_list *parents;
for (parents = commit->parents; parents ; parents = parents->next) {
struct commit *parent = parents->item;
int *pi = indegree_slab_at(&indegree, parent);
if (!*pi)
continue;
if (--(*pi) == 1)
prio_queue_put(&queue, parent);
}
*(indegree_slab_at(&indegree, commit)) = 0;
pptr = &commit_list_insert(commit, pptr)->next;
}
clear_indegree_slab(&indegree);
clear_prio_queue(&queue);
if (sort_order == REV_SORT_BY_AUTHOR_DATE)
clear_author_date_slab(&author_date);
}
struct rev_collect {
struct commit **commit;
int nr;
int alloc;
unsigned int initial : 1;
};
static void add_one_commit(struct object_id *oid, struct rev_collect *revs)
{
struct commit *commit;
if (is_null_oid(oid))
return;
commit = lookup_commit(the_repository, oid);
if (!commit ||
(commit->object.flags & TMP_MARK) ||
parse_commit(commit))
return;
ALLOC_GROW(revs->commit, revs->nr + 1, revs->alloc);
revs->commit[revs->nr++] = commit;
commit->object.flags |= TMP_MARK;
}
static int collect_one_reflog_ent(struct object_id *ooid, struct object_id *noid,
const char *ident, timestamp_t timestamp,
int tz, const char *message, void *cbdata)
{
struct rev_collect *revs = cbdata;
if (revs->initial) {
revs->initial = 0;
add_one_commit(ooid, revs);
}
add_one_commit(noid, revs);
return 0;
}
struct commit *get_fork_point(const char *refname, struct commit *commit)
{
struct object_id oid;
struct rev_collect revs;
struct commit_list *bases;
int i;
struct commit *ret = NULL;
char *full_refname;
switch (dwim_ref(refname, strlen(refname), &oid, &full_refname)) {
case 0:
die("No such ref: '%s'", refname);
case 1:
break; 
default:
die("Ambiguous refname: '%s'", refname);
}
memset(&revs, 0, sizeof(revs));
revs.initial = 1;
for_each_reflog_ent(full_refname, collect_one_reflog_ent, &revs);
if (!revs.nr)
add_one_commit(&oid, &revs);
for (i = 0; i < revs.nr; i++)
revs.commit[i]->object.flags &= ~TMP_MARK;
bases = get_merge_bases_many(commit, revs.nr, revs.commit);
if (!bases || bases->next)
goto cleanup_return;
for (i = 0; i < revs.nr; i++)
if (&bases->item->object == &revs.commit[i]->object)
break; 
if (revs.nr <= i)
goto cleanup_return;
ret = bases->item;
cleanup_return:
free_commit_list(bases);
free(full_refname);
return ret;
}
static const char *gpg_sig_headers[] = {
NULL,
"gpgsig",
"gpgsig-sha256",
};
static int do_sign_commit(struct strbuf *buf, const char *keyid)
{
struct strbuf sig = STRBUF_INIT;
int inspos, copypos;
const char *eoh;
const char *gpg_sig_header = gpg_sig_headers[hash_algo_by_ptr(the_hash_algo)];
int gpg_sig_header_len = strlen(gpg_sig_header);
eoh = strstr(buf->buf, "\n\n");
if (!eoh)
inspos = buf->len;
else
inspos = eoh - buf->buf + 1;
if (!keyid || !*keyid)
keyid = get_signing_key();
if (sign_buffer(buf, &sig, keyid)) {
strbuf_release(&sig);
return -1;
}
for (copypos = 0; sig.buf[copypos]; ) {
const char *bol = sig.buf + copypos;
const char *eol = strchrnul(bol, '\n');
int len = (eol - bol) + !!*eol;
if (!copypos) {
strbuf_insert(buf, inspos, gpg_sig_header, gpg_sig_header_len);
inspos += gpg_sig_header_len;
}
strbuf_insertstr(buf, inspos++, " ");
strbuf_insert(buf, inspos, bol, len);
inspos += len;
copypos += len;
}
strbuf_release(&sig);
return 0;
}
int parse_signed_commit(const struct commit *commit,
struct strbuf *payload, struct strbuf *signature)
{
unsigned long size;
const char *buffer = get_commit_buffer(commit, &size);
int in_signature, saw_signature = -1;
const char *line, *tail;
const char *gpg_sig_header = gpg_sig_headers[hash_algo_by_ptr(the_hash_algo)];
int gpg_sig_header_len = strlen(gpg_sig_header);
line = buffer;
tail = buffer + size;
in_signature = 0;
saw_signature = 0;
while (line < tail) {
const char *sig = NULL;
const char *next = memchr(line, '\n', tail - line);
next = next ? next + 1 : tail;
if (in_signature && line[0] == ' ')
sig = line + 1;
else if (starts_with(line, gpg_sig_header) &&
line[gpg_sig_header_len] == ' ')
sig = line + gpg_sig_header_len + 1;
if (sig) {
strbuf_add(signature, sig, next - sig);
saw_signature = 1;
in_signature = 1;
} else {
if (*line == '\n')
next = tail;
strbuf_add(payload, line, next - line);
in_signature = 0;
}
line = next;
}
unuse_commit_buffer(commit, buffer);
return saw_signature;
}
int remove_signature(struct strbuf *buf)
{
const char *line = buf->buf;
const char *tail = buf->buf + buf->len;
int in_signature = 0;
const char *sig_start = NULL;
const char *sig_end = NULL;
while (line < tail) {
const char *next = memchr(line, '\n', tail - line);
next = next ? next + 1 : tail;
if (in_signature && line[0] == ' ')
sig_end = next;
else if (starts_with(line, "gpgsig")) {
int i;
for (i = 1; i < GIT_HASH_NALGOS; i++) {
const char *p;
if (skip_prefix(line, gpg_sig_headers[i], &p) &&
*p == ' ') {
sig_start = line;
sig_end = next;
in_signature = 1;
}
}
} else {
if (*line == '\n')
next = tail;
in_signature = 0;
}
line = next;
}
if (sig_start)
strbuf_remove(buf, sig_start - buf->buf, sig_end - sig_start);
return sig_start != NULL;
}
static void handle_signed_tag(struct commit *parent, struct commit_extra_header ***tail)
{
struct merge_remote_desc *desc;
struct commit_extra_header *mergetag;
char *buf;
unsigned long size, len;
enum object_type type;
desc = merge_remote_util(parent);
if (!desc || !desc->obj)
return;
buf = read_object_file(&desc->obj->oid, &type, &size);
if (!buf || type != OBJ_TAG)
goto free_return;
len = parse_signature(buf, size);
if (size == len)
goto free_return;
mergetag = xcalloc(1, sizeof(*mergetag));
mergetag->key = xstrdup("mergetag");
mergetag->value = buf;
mergetag->len = size;
**tail = mergetag;
*tail = &mergetag->next;
return;
free_return:
free(buf);
}
int check_commit_signature(const struct commit *commit, struct signature_check *sigc)
{
struct strbuf payload = STRBUF_INIT;
struct strbuf signature = STRBUF_INIT;
int ret = 1;
sigc->result = 'N';
if (parse_signed_commit(commit, &payload, &signature) <= 0)
goto out;
ret = check_signature(payload.buf, payload.len, signature.buf,
signature.len, sigc);
out:
strbuf_release(&payload);
strbuf_release(&signature);
return ret;
}
void verify_merge_signature(struct commit *commit, int verbosity,
int check_trust)
{
char hex[GIT_MAX_HEXSZ + 1];
struct signature_check signature_check;
int ret;
memset(&signature_check, 0, sizeof(signature_check));
ret = check_commit_signature(commit, &signature_check);
find_unique_abbrev_r(hex, &commit->object.oid, DEFAULT_ABBREV);
switch (signature_check.result) {
case 'G':
if (ret || (check_trust && signature_check.trust_level < TRUST_MARGINAL))
die(_("Commit %s has an untrusted GPG signature, "
"allegedly by %s."), hex, signature_check.signer);
break;
case 'B':
die(_("Commit %s has a bad GPG signature "
"allegedly by %s."), hex, signature_check.signer);
default: 
die(_("Commit %s does not have a GPG signature."), hex);
}
if (verbosity >= 0 && signature_check.result == 'G')
printf(_("Commit %s has a good GPG signature by %s\n"),
hex, signature_check.signer);
signature_check_clear(&signature_check);
}
void append_merge_tag_headers(struct commit_list *parents,
struct commit_extra_header ***tail)
{
while (parents) {
struct commit *parent = parents->item;
handle_signed_tag(parent, tail);
parents = parents->next;
}
}
static void add_extra_header(struct strbuf *buffer,
struct commit_extra_header *extra)
{
strbuf_addstr(buffer, extra->key);
if (extra->len)
strbuf_add_lines(buffer, " ", extra->value, extra->len);
else
strbuf_addch(buffer, '\n');
}
struct commit_extra_header *read_commit_extra_headers(struct commit *commit,
const char **exclude)
{
struct commit_extra_header *extra = NULL;
unsigned long size;
const char *buffer = get_commit_buffer(commit, &size);
extra = read_commit_extra_header_lines(buffer, size, exclude);
unuse_commit_buffer(commit, buffer);
return extra;
}
int for_each_mergetag(each_mergetag_fn fn, struct commit *commit, void *data)
{
struct commit_extra_header *extra, *to_free;
int res = 0;
to_free = read_commit_extra_headers(commit, NULL);
for (extra = to_free; !res && extra; extra = extra->next) {
if (strcmp(extra->key, "mergetag"))
continue; 
res = fn(commit, extra, data);
}
free_commit_extra_headers(to_free);
return res;
}
static inline int standard_header_field(const char *field, size_t len)
{
return ((len == 4 && !memcmp(field, "tree", 4)) ||
(len == 6 && !memcmp(field, "parent", 6)) ||
(len == 6 && !memcmp(field, "author", 6)) ||
(len == 9 && !memcmp(field, "committer", 9)) ||
(len == 8 && !memcmp(field, "encoding", 8)));
}
static int excluded_header_field(const char *field, size_t len, const char **exclude)
{
if (!exclude)
return 0;
while (*exclude) {
size_t xlen = strlen(*exclude);
if (len == xlen && !memcmp(field, *exclude, xlen))
return 1;
exclude++;
}
return 0;
}
static struct commit_extra_header *read_commit_extra_header_lines(
const char *buffer, size_t size,
const char **exclude)
{
struct commit_extra_header *extra = NULL, **tail = &extra, *it = NULL;
const char *line, *next, *eof, *eob;
struct strbuf buf = STRBUF_INIT;
for (line = buffer, eob = line + size;
line < eob && *line != '\n';
line = next) {
next = memchr(line, '\n', eob - line);
next = next ? next + 1 : eob;
if (*line == ' ') {
if (it)
strbuf_add(&buf, line + 1, next - (line + 1));
continue;
}
if (it)
it->value = strbuf_detach(&buf, &it->len);
strbuf_reset(&buf);
it = NULL;
eof = memchr(line, ' ', next - line);
if (!eof)
eof = next;
else if (standard_header_field(line, eof - line) ||
excluded_header_field(line, eof - line, exclude))
continue;
it = xcalloc(1, sizeof(*it));
it->key = xmemdupz(line, eof-line);
*tail = it;
tail = &it->next;
if (eof + 1 < next)
strbuf_add(&buf, eof + 1, next - (eof + 1));
}
if (it)
it->value = strbuf_detach(&buf, &it->len);
return extra;
}
void free_commit_extra_headers(struct commit_extra_header *extra)
{
while (extra) {
struct commit_extra_header *next = extra->next;
free(extra->key);
free(extra->value);
free(extra);
extra = next;
}
}
int commit_tree(const char *msg, size_t msg_len, const struct object_id *tree,
struct commit_list *parents, struct object_id *ret,
const char *author, const char *sign_commit)
{
struct commit_extra_header *extra = NULL, **tail = &extra;
int result;
append_merge_tag_headers(parents, &tail);
result = commit_tree_extended(msg, msg_len, tree, parents, ret,
author, sign_commit, extra);
free_commit_extra_headers(extra);
return result;
}
static int find_invalid_utf8(const char *buf, int len)
{
int offset = 0;
static const unsigned int max_codepoint[] = {
0x7f, 0x7ff, 0xffff, 0x10ffff
};
while (len) {
unsigned char c = *buf++;
int bytes, bad_offset;
unsigned int codepoint;
unsigned int min_val, max_val;
len--;
offset++;
if (c < 0x80)
continue;
bad_offset = offset-1;
bytes = 0;
while (c & 0x40) {
c <<= 1;
bytes++;
}
if (bytes < 1 || 3 < bytes)
return bad_offset;
if (len < bytes)
return bad_offset;
codepoint = (c & 0x7f) >> bytes;
min_val = max_codepoint[bytes-1] + 1;
max_val = max_codepoint[bytes];
offset += bytes;
len -= bytes;
do {
codepoint <<= 6;
codepoint |= *buf & 0x3f;
if ((*buf++ & 0xc0) != 0x80)
return bad_offset;
} while (--bytes);
if (codepoint < min_val || codepoint > max_val)
return bad_offset;
if ((codepoint & 0x1ff800) == 0xd800)
return bad_offset;
if ((codepoint & 0xfffe) == 0xfffe)
return bad_offset;
if (codepoint >= 0xfdd0 && codepoint <= 0xfdef)
return bad_offset;
}
return -1;
}
static int verify_utf8(struct strbuf *buf)
{
int ok = 1;
long pos = 0;
for (;;) {
int bad;
unsigned char c;
unsigned char replace[2];
bad = find_invalid_utf8(buf->buf + pos, buf->len - pos);
if (bad < 0)
return ok;
pos += bad;
ok = 0;
c = buf->buf[pos];
strbuf_remove(buf, pos, 1);
replace[0] = 0xc0 + (c >> 6);
replace[1] = 0x80 + (c & 0x3f);
strbuf_insert(buf, pos, replace, 2);
pos += 2;
}
}
static const char commit_utf8_warn[] =
N_("Warning: commit message did not conform to UTF-8.\n"
"You may want to amend it after fixing the message, or set the config\n"
"variable i18n.commitencoding to the encoding your project uses.\n");
int commit_tree_extended(const char *msg, size_t msg_len,
const struct object_id *tree,
struct commit_list *parents, struct object_id *ret,
const char *author, const char *sign_commit,
struct commit_extra_header *extra)
{
int result;
int encoding_is_utf8;
struct strbuf buffer;
assert_oid_type(tree, OBJ_TREE);
if (memchr(msg, '\0', msg_len))
return error("a NUL byte in commit log message not allowed.");
encoding_is_utf8 = is_encoding_utf8(git_commit_encoding);
strbuf_init(&buffer, 8192); 
strbuf_addf(&buffer, "tree %s\n", oid_to_hex(tree));
while (parents) {
struct commit *parent = pop_commit(&parents);
strbuf_addf(&buffer, "parent %s\n",
oid_to_hex(&parent->object.oid));
}
if (!author)
author = git_author_info(IDENT_STRICT);
strbuf_addf(&buffer, "author %s\n", author);
strbuf_addf(&buffer, "committer %s\n", git_committer_info(IDENT_STRICT));
if (!encoding_is_utf8)
strbuf_addf(&buffer, "encoding %s\n", git_commit_encoding);
while (extra) {
add_extra_header(&buffer, extra);
extra = extra->next;
}
strbuf_addch(&buffer, '\n');
strbuf_add(&buffer, msg, msg_len);
if (encoding_is_utf8 && !verify_utf8(&buffer))
fprintf(stderr, _(commit_utf8_warn));
if (sign_commit && do_sign_commit(&buffer, sign_commit)) {
result = -1;
goto out;
}
result = write_object_file(buffer.buf, buffer.len, commit_type, ret);
out:
strbuf_release(&buffer);
return result;
}
define_commit_slab(merge_desc_slab, struct merge_remote_desc *);
static struct merge_desc_slab merge_desc_slab = COMMIT_SLAB_INIT(1, merge_desc_slab);
struct merge_remote_desc *merge_remote_util(struct commit *commit)
{
return *merge_desc_slab_at(&merge_desc_slab, commit);
}
void set_merge_remote_desc(struct commit *commit,
const char *name, struct object *obj)
{
struct merge_remote_desc *desc;
FLEX_ALLOC_STR(desc, name, name);
desc->obj = obj;
*merge_desc_slab_at(&merge_desc_slab, commit) = desc;
}
struct commit *get_merge_parent(const char *name)
{
struct object *obj;
struct commit *commit;
struct object_id oid;
if (get_oid(name, &oid))
return NULL;
obj = parse_object(the_repository, &oid);
commit = (struct commit *)peel_to_type(name, 0, obj, OBJ_COMMIT);
if (commit && !merge_remote_util(commit))
set_merge_remote_desc(commit, name, obj);
return commit;
}
struct commit_list **commit_list_append(struct commit *commit,
struct commit_list **next)
{
struct commit_list *new_commit = xmalloc(sizeof(struct commit_list));
new_commit->item = commit;
*next = new_commit;
new_commit->next = NULL;
return &new_commit->next;
}
const char *find_commit_header(const char *msg, const char *key, size_t *out_len)
{
int key_len = strlen(key);
const char *line = msg;
while (line) {
const char *eol = strchrnul(line, '\n');
if (line == eol)
return NULL;
if (eol - line > key_len &&
!strncmp(line, key, key_len) &&
line[key_len] == ' ') {
*out_len = eol - line - key_len - 1;
return line + key_len + 1;
}
line = *eol ? eol + 1 : NULL;
}
return NULL;
}
size_t ignore_non_trailer(const char *buf, size_t len)
{
size_t boc = 0;
size_t bol = 0;
int in_old_conflicts_block = 0;
size_t cutoff = wt_status_locate_end(buf, len);
while (bol < cutoff) {
const char *next_line = memchr(buf + bol, '\n', len - bol);
if (!next_line)
next_line = buf + len;
else
next_line++;
if (buf[bol] == comment_line_char || buf[bol] == '\n') {
if (!boc)
boc = bol;
} else if (starts_with(buf + bol, "Conflicts:\n")) {
in_old_conflicts_block = 1;
if (!boc)
boc = bol;
} else if (in_old_conflicts_block && buf[bol] == '\t') {
; 
} else if (boc) {
boc = 0;
in_old_conflicts_block = 0;
}
bol = next_line - buf;
}
return boc ? len - boc : len - cutoff;
}
int run_commit_hook(int editor_is_used, const char *index_file,
const char *name, ...)
{
struct argv_array hook_env = ARGV_ARRAY_INIT;
va_list args;
int ret;
argv_array_pushf(&hook_env, "GIT_INDEX_FILE=%s", index_file);
if (!editor_is_used)
argv_array_push(&hook_env, "GIT_EDITOR=:");
va_start(args, name);
ret = run_hook_ve(hook_env.argv,name, args);
va_end(args);
argv_array_clear(&hook_env);
return ret;
}
