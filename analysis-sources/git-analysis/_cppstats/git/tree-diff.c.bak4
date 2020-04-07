


#include "cache.h"
#include "diff.h"
#include "diffcore.h"
#include "tree.h"







#define S_IFXMIN_NEQ S_DIFFTREE_IFXMIN_NEQ

#define FAST_ARRAY_ALLOC(x, nr) do { if ((nr) <= 2) (x) = xalloca((nr) * sizeof(*(x))); else ALLOC_ARRAY((x), nr); } while(0)





#define FAST_ARRAY_FREE(x, nr) do { if ((nr) > 2) free((x)); } while(0)




static struct combine_diff_path *ll_diff_tree_paths(
struct combine_diff_path *p, const struct object_id *oid,
const struct object_id **parents_oid, int nparent,
struct strbuf *base, struct diff_options *opt);
static int ll_diff_tree_oid(const struct object_id *old_oid,
const struct object_id *new_oid,
struct strbuf *base, struct diff_options *opt);














static int tree_entry_pathcmp(struct tree_desc *t1, struct tree_desc *t2)
{
struct name_entry *e1, *e2;
int cmp;


if (!t1->size)
return t2->size ? 1 : 0;
else if (!t2->size)
return -1;

e1 = &t1->entry;
e2 = &t2->entry;
cmp = base_name_compare(e1->path, tree_entry_len(e1), e1->mode,
e2->path, tree_entry_len(e2), e2->mode);
return cmp;
}








static int emit_diff_first_parent_only(struct diff_options *opt, struct combine_diff_path *p)
{
struct combine_diff_parent *p0 = &p->parent[0];
if (p->mode && p0->mode) {
opt->change(opt, p0->mode, p->mode, &p0->oid, &p->oid,
1, 1, p->path, 0, 0);
}
else {
const struct object_id *oid;
unsigned int mode;
int addremove;

if (p->mode) {
addremove = '+';
oid = &p->oid;
mode = p->mode;
} else {
addremove = '-';
oid = &p0->oid;
mode = p0->mode;
}

opt->add_remove(opt, addremove, mode, oid, 1, p->path, 0);
}

return 0; 
}
































static struct combine_diff_path *path_appendnew(struct combine_diff_path *last,
int nparent, const struct strbuf *base, const char *path, int pathlen,
unsigned mode, const struct object_id *oid)
{
struct combine_diff_path *p;
size_t len = st_add(base->len, pathlen);
size_t alloclen = combine_diff_path_size(nparent, len);


p = last->next;
if (p && (alloclen > (intptr_t)p->next)) {
FREE_AND_NULL(p);
}

if (!p) {
p = xmalloc(alloclen);





p->next = (struct combine_diff_path *)(intptr_t)alloclen;
}

last->next = p;

p->path = (char *)&(p->parent[nparent]);
memcpy(p->path, base->buf, base->len);
memcpy(p->path + base->len, path, pathlen);
p->path[len] = 0;
p->mode = mode;
oidcpy(&p->oid, oid ? oid : &null_oid);

return p;
}











static struct combine_diff_path *emit_path(struct combine_diff_path *p,
struct strbuf *base, struct diff_options *opt, int nparent,
struct tree_desc *t, struct tree_desc *tp,
int imin)
{
unsigned short mode;
const char *path;
const struct object_id *oid;
int pathlen;
int old_baselen = base->len;
int i, isdir, recurse = 0, emitthis = 1;


assert(t || tp);

if (t) {

oid = tree_entry_extract(t, &path, &mode);
pathlen = tree_entry_len(&t->entry);
isdir = S_ISDIR(mode);
} else {







tree_entry_extract(&tp[imin], &path, &mode);
pathlen = tree_entry_len(&tp[imin].entry);

isdir = S_ISDIR(mode);
oid = NULL;
mode = 0;
}

if (opt->flags.recursive && isdir) {
recurse = 1;
emitthis = opt->flags.tree_in_recursive;
}

if (emitthis) {
int keep;
struct combine_diff_path *pprev = p;
p = path_appendnew(p, nparent, base, path, pathlen, mode, oid);

for (i = 0; i < nparent; ++i) {




int tpi_valid = tp && !(tp[i].entry.mode & S_IFXMIN_NEQ);

const struct object_id *oid_i;
unsigned mode_i;

p->parent[i].status =
!t ? DIFF_STATUS_DELETED :
tpi_valid ?
DIFF_STATUS_MODIFIED :
DIFF_STATUS_ADDED;

if (tpi_valid) {
oid_i = &tp[i].entry.oid;
mode_i = tp[i].entry.mode;
}
else {
oid_i = &null_oid;
mode_i = 0;
}

p->parent[i].mode = mode_i;
oidcpy(&p->parent[i].oid, oid_i);
}

keep = 1;
if (opt->pathchange)
keep = opt->pathchange(opt, p);












if (!keep)
p = pprev;
else
p->next = NULL;
}

if (recurse) {
const struct object_id **parents_oid;

FAST_ARRAY_ALLOC(parents_oid, nparent);
for (i = 0; i < nparent; ++i) {

int tpi_valid = tp && !(tp[i].entry.mode & S_IFXMIN_NEQ);

parents_oid[i] = tpi_valid ? &tp[i].entry.oid : NULL;
}

strbuf_add(base, path, pathlen);
strbuf_addch(base, '/');
p = ll_diff_tree_paths(p, oid, parents_oid, nparent, base, opt);
FAST_ARRAY_FREE(parents_oid, nparent);
}

strbuf_setlen(base, old_baselen);
return p;
}

static void skip_uninteresting(struct tree_desc *t, struct strbuf *base,
struct diff_options *opt)
{
enum interesting match;

while (t->size) {
match = tree_entry_interesting(opt->repo->index, &t->entry,
base, 0, &opt->pathspec);
if (match) {
if (match == all_entries_not_interesting)
t->size = 0;
break;
}
update_tree_entry(t);
}
}






















































































static inline void update_tp_entries(struct tree_desc *tp, int nparent)
{
int i;
for (i = 0; i < nparent; ++i)
if (!(tp[i].entry.mode & S_IFXMIN_NEQ))
update_tree_entry(&tp[i]);
}

static struct combine_diff_path *ll_diff_tree_paths(
struct combine_diff_path *p, const struct object_id *oid,
const struct object_id **parents_oid, int nparent,
struct strbuf *base, struct diff_options *opt)
{
struct tree_desc t, *tp;
void *ttree, **tptree;
int i;

FAST_ARRAY_ALLOC(tp, nparent);
FAST_ARRAY_ALLOC(tptree, nparent);







for (i = 0; i < nparent; ++i)
tptree[i] = fill_tree_descriptor(opt->repo, &tp[i], parents_oid[i]);
ttree = fill_tree_descriptor(opt->repo, &t, oid);


opt->pathspec.recursive = opt->flags.recursive;

for (;;) {
int imin, cmp;

if (diff_can_quit_early(opt))
break;

if (opt->pathspec.nr) {
skip_uninteresting(&t, base, opt);
for (i = 0; i < nparent; i++)
skip_uninteresting(&tp[i], base, opt);
}


if (!t.size) {
int done = 1;
for (i = 0; i < nparent; ++i)
if (tp[i].size) {
done = 0;
break;
}
if (done)
break;
}





imin = 0;
tp[0].entry.mode &= ~S_IFXMIN_NEQ;

for (i = 1; i < nparent; ++i) {
cmp = tree_entry_pathcmp(&tp[i], &tp[imin]);
if (cmp < 0) {
imin = i;
tp[i].entry.mode &= ~S_IFXMIN_NEQ;
}
else if (cmp == 0) {
tp[i].entry.mode &= ~S_IFXMIN_NEQ;
}
else {
tp[i].entry.mode |= S_IFXMIN_NEQ;
}
}


for (i = 0; i < imin; ++i)
tp[i].entry.mode |= S_IFXMIN_NEQ; 




cmp = tree_entry_pathcmp(&t, &tp[imin]);


if (cmp == 0) {

if (!opt->flags.find_copies_harder) {
for (i = 0; i < nparent; ++i) {

if (tp[i].entry.mode & S_IFXMIN_NEQ)
continue;


if (!oideq(&t.entry.oid, &tp[i].entry.oid) ||
(t.entry.mode != tp[i].entry.mode))
continue;

goto skip_emit_t_tp;
}
}


p = emit_path(p, base, opt, nparent,
&t, tp, imin);

skip_emit_t_tp:

update_tree_entry(&t);
update_tp_entries(tp, nparent);
}


else if (cmp < 0) {

p = emit_path(p, base, opt, nparent,
&t, NULL, -1);


update_tree_entry(&t);
}


else {

if (!opt->flags.find_copies_harder) {
for (i = 0; i < nparent; ++i)
if (tp[i].entry.mode & S_IFXMIN_NEQ)
goto skip_emit_tp;
}

p = emit_path(p, base, opt, nparent,
NULL, tp, imin);

skip_emit_tp:

update_tp_entries(tp, nparent);
}
}

free(ttree);
for (i = nparent-1; i >= 0; i--)
free(tptree[i]);
FAST_ARRAY_FREE(tptree, nparent);
FAST_ARRAY_FREE(tp, nparent);

return p;
}

struct combine_diff_path *diff_tree_paths(
struct combine_diff_path *p, const struct object_id *oid,
const struct object_id **parents_oid, int nparent,
struct strbuf *base, struct diff_options *opt)
{
p = ll_diff_tree_paths(p, oid, parents_oid, nparent, base, opt);





FREE_AND_NULL(p->next);

return p;
}






static inline int diff_might_be_rename(void)
{
return diff_queued_diff.nr == 1 &&
!DIFF_FILE_VALID(diff_queued_diff.queue[0]->one);
}

static void try_to_follow_renames(const struct object_id *old_oid,
const struct object_id *new_oid,
struct strbuf *base, struct diff_options *opt)
{
struct diff_options diff_opts;
struct diff_queue_struct *q = &diff_queued_diff;
struct diff_filepair *choice;
int i;






GUARD_PATHSPEC(&opt->pathspec, PATHSPEC_FROMTOP | PATHSPEC_LITERAL);
#if 0







if (opt->pathspec.has_wildcard)
die("BUG:%s:%d: wildcards are not supported",
__FILE__, __LINE__);
#endif


choice = q->queue[0];
q->nr = 0;

repo_diff_setup(opt->repo, &diff_opts);
diff_opts.flags.recursive = 1;
diff_opts.flags.find_copies_harder = 1;
diff_opts.output_format = DIFF_FORMAT_NO_OUTPUT;
diff_opts.single_follow = opt->pathspec.items[0].match;
diff_opts.break_opt = opt->break_opt;
diff_opts.rename_score = opt->rename_score;
diff_setup_done(&diff_opts);
ll_diff_tree_oid(old_oid, new_oid, base, &diff_opts);
diffcore_std(&diff_opts);
clear_pathspec(&diff_opts.pathspec);


opt->found_follow = 0;
for (i = 0; i < q->nr; i++) {
struct diff_filepair *p = q->queue[i];






if ((p->status == 'R' || p->status == 'C') &&
!strcmp(p->two->path, opt->pathspec.items[0].match)) {
const char *path[2];


q->queue[i] = choice;
choice = p;


path[0] = p->one->path;
path[1] = NULL;
clear_pathspec(&opt->pathspec);
parse_pathspec(&opt->pathspec,
PATHSPEC_ALL_MAGIC & ~PATHSPEC_LITERAL,
PATHSPEC_LITERAL_PATH, "", path);









opt->found_follow = 1;
break;
}
}




for (i = 0; i < q->nr; i++) {
struct diff_filepair *p = q->queue[i];
diff_free_filepair(p);
}





q->queue[0] = choice;
q->nr = 1;
}

static int ll_diff_tree_oid(const struct object_id *old_oid,
const struct object_id *new_oid,
struct strbuf *base, struct diff_options *opt)
{
struct combine_diff_path phead, *p;
pathchange_fn_t pathchange_old = opt->pathchange;

phead.next = NULL;
opt->pathchange = emit_diff_first_parent_only;
diff_tree_paths(&phead, new_oid, &old_oid, 1, base, opt);

for (p = phead.next; p;) {
struct combine_diff_path *pprev = p;
p = p->next;
free(pprev);
}

opt->pathchange = pathchange_old;
return 0;
}

int diff_tree_oid(const struct object_id *old_oid,
const struct object_id *new_oid,
const char *base_str, struct diff_options *opt)
{
struct strbuf base;
int retval;

strbuf_init(&base, PATH_MAX);
strbuf_addstr(&base, base_str);

retval = ll_diff_tree_oid(old_oid, new_oid, &base, opt);
if (!*base_str && opt->flags.follow_renames && diff_might_be_rename())
try_to_follow_renames(old_oid, new_oid, &base, opt);

strbuf_release(&base);

return retval;
}

int diff_root_tree_oid(const struct object_id *new_oid, const char *base, struct diff_options *opt)
{
return diff_tree_oid(NULL, new_oid, base, opt);
}
