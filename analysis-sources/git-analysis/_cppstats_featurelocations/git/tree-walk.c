#include "cache.h"
#include "tree-walk.h"
#include "dir.h"
#include "object-store.h"
#include "tree.h"
#include "pathspec.h"

static const char *get_mode(const char *str, unsigned int *modep)
{
unsigned char c;
unsigned int mode = 0;

if (*str == ' ')
return NULL;

while ((c = *str++) != ' ') {
if (c < '0' || c > '7')
return NULL;
mode = (mode << 3) + (c - '0');
}
*modep = mode;
return str;
}

static int decode_tree_entry(struct tree_desc *desc, const char *buf, unsigned long size, struct strbuf *err)
{
const char *path;
unsigned int mode, len;
const unsigned hashsz = the_hash_algo->rawsz;

if (size < hashsz + 3 || buf[size - (hashsz + 1)]) {
strbuf_addstr(err, _("too-short tree object"));
return -1;
}

path = get_mode(buf, &mode);
if (!path) {
strbuf_addstr(err, _("malformed mode in tree entry"));
return -1;
}
if (!*path) {
strbuf_addstr(err, _("empty filename in tree entry"));
return -1;
}
len = strlen(path) + 1;


desc->entry.path = path;
desc->entry.mode = canon_mode(mode);
desc->entry.pathlen = len - 1;
hashcpy(desc->entry.oid.hash, (const unsigned char *)path + len);

return 0;
}

static int init_tree_desc_internal(struct tree_desc *desc, const void *buffer, unsigned long size, struct strbuf *err)
{
desc->buffer = buffer;
desc->size = size;
if (size)
return decode_tree_entry(desc, buffer, size, err);
return 0;
}

void init_tree_desc(struct tree_desc *desc, const void *buffer, unsigned long size)
{
struct strbuf err = STRBUF_INIT;
if (init_tree_desc_internal(desc, buffer, size, &err))
die("%s", err.buf);
strbuf_release(&err);
}

int init_tree_desc_gently(struct tree_desc *desc, const void *buffer, unsigned long size)
{
struct strbuf err = STRBUF_INIT;
int result = init_tree_desc_internal(desc, buffer, size, &err);
if (result)
error("%s", err.buf);
strbuf_release(&err);
return result;
}

void *fill_tree_descriptor(struct repository *r,
struct tree_desc *desc,
const struct object_id *oid)
{
unsigned long size = 0;
void *buf = NULL;

if (oid) {
buf = read_object_with_reference(r, oid, tree_type, &size, NULL);
if (!buf)
die("unable to read tree %s", oid_to_hex(oid));
}
init_tree_desc(desc, buf, size);
return buf;
}

static void entry_clear(struct name_entry *a)
{
memset(a, 0, sizeof(*a));
}

static void entry_extract(struct tree_desc *t, struct name_entry *a)
{
*a = t->entry;
}

static int update_tree_entry_internal(struct tree_desc *desc, struct strbuf *err)
{
const void *buf = desc->buffer;
const unsigned char *end = (const unsigned char *)desc->entry.path + desc->entry.pathlen + 1 + the_hash_algo->rawsz;
unsigned long size = desc->size;
unsigned long len = end - (const unsigned char *)buf;

if (size < len)
die(_("too-short tree file"));
buf = end;
size -= len;
desc->buffer = buf;
desc->size = size;
if (size)
return decode_tree_entry(desc, buf, size, err);
return 0;
}

void update_tree_entry(struct tree_desc *desc)
{
struct strbuf err = STRBUF_INIT;
if (update_tree_entry_internal(desc, &err))
die("%s", err.buf);
strbuf_release(&err);
}

int update_tree_entry_gently(struct tree_desc *desc)
{
struct strbuf err = STRBUF_INIT;
if (update_tree_entry_internal(desc, &err)) {
error("%s", err.buf);
strbuf_release(&err);

desc->size = 0;
return -1;
}
strbuf_release(&err);
return 0;
}

int tree_entry(struct tree_desc *desc, struct name_entry *entry)
{
if (!desc->size)
return 0;

*entry = desc->entry;
update_tree_entry(desc);
return 1;
}

int tree_entry_gently(struct tree_desc *desc, struct name_entry *entry)
{
if (!desc->size)
return 0;

*entry = desc->entry;
if (update_tree_entry_gently(desc))
return 0;
return 1;
}

void setup_traverse_info(struct traverse_info *info, const char *base)
{
size_t pathlen = strlen(base);
static struct traverse_info dummy;

memset(info, 0, sizeof(*info));
if (pathlen && base[pathlen-1] == '/')
pathlen--;
info->pathlen = pathlen ? pathlen + 1 : 0;
info->name = base;
info->namelen = pathlen;
if (pathlen)
info->prev = &dummy;
}

char *make_traverse_path(char *path, size_t pathlen,
const struct traverse_info *info,
const char *name, size_t namelen)
{

size_t pos = st_add(info->pathlen, namelen);

if (pos >= pathlen)
BUG("too small buffer passed to make_traverse_path");

path[pos] = 0;
for (;;) {
if (pos < namelen)
BUG("traverse_info pathlen does not match strings");
pos -= namelen;
memcpy(path + pos, name, namelen);

if (!pos)
break;
path[--pos] = '/';

if (!info)
BUG("traverse_info ran out of list items");
name = info->name;
namelen = info->namelen;
info = info->prev;
}
return path;
}

void strbuf_make_traverse_path(struct strbuf *out,
const struct traverse_info *info,
const char *name, size_t namelen)
{
size_t len = traverse_path_len(info, namelen);

strbuf_grow(out, len);
make_traverse_path(out->buf + out->len, out->alloc - out->len,
info, name, namelen);
strbuf_setlen(out, out->len + len);
}

struct tree_desc_skip {
struct tree_desc_skip *prev;
const void *ptr;
};

struct tree_desc_x {
struct tree_desc d;
struct tree_desc_skip *skip;
};

static int check_entry_match(const char *a, int a_len, const char *b, int b_len)
{


















int cmp = name_compare(a, a_len, b, b_len);


if (!cmp)
return cmp;

if (0 < cmp) {




return 1; 
}


if (a_len < b_len && !memcmp(a, b, a_len) && b[a_len] < '/')
return 1; 

return -1; 
}
























static void extended_entry_extract(struct tree_desc_x *t,
struct name_entry *a,
const char *first,
int first_len)
{
const char *path;
int len;
struct tree_desc probe;
struct tree_desc_skip *skip;





while (1) {
if (!t->d.size) {
entry_clear(a);
break; 
}
entry_extract(&t->d, a);
for (skip = t->skip; skip; skip = skip->prev)
if (a->path == skip->ptr)
break; 
if (!skip)
break;

update_tree_entry(&t->d);
}

if (!first || !a->path)
return;




path = a->path;
len = tree_entry_len(a);
switch (check_entry_match(first, first_len, path, len)) {
case -1:
entry_clear(a);
case 0:
return;
default:
break;
}





probe = t->d;
while (probe.size) {
entry_extract(&probe, a);
path = a->path;
len = tree_entry_len(a);
switch (check_entry_match(first, first_len, path, len)) {
case -1:
entry_clear(a);
case 0:
return;
default:
update_tree_entry(&probe);
break;
}

}
entry_clear(a);
}

static void update_extended_entry(struct tree_desc_x *t, struct name_entry *a)
{
if (t->d.entry.path == a->path) {
update_tree_entry(&t->d);
} else {

struct tree_desc_skip *skip = xmalloc(sizeof(*skip));
skip->ptr = a->path;
skip->prev = t->skip;
t->skip = skip;
}
}

static void free_extended_entry(struct tree_desc_x *t)
{
struct tree_desc_skip *p, *s;

for (s = t->skip; s; s = p) {
p = s->prev;
free(s);
}
}

static inline int prune_traversal(struct index_state *istate,
struct name_entry *e,
struct traverse_info *info,
struct strbuf *base,
int still_interesting)
{
if (!info->pathspec || still_interesting == 2)
return 2;
if (still_interesting < 0)
return still_interesting;
return tree_entry_interesting(istate, e, base,
0, info->pathspec);
}

int traverse_trees(struct index_state *istate,
int n, struct tree_desc *t,
struct traverse_info *info)
{
int error = 0;
struct name_entry entry[MAX_TRAVERSE_TREES];
int i;
struct tree_desc_x tx[ARRAY_SIZE(entry)];
struct strbuf base = STRBUF_INIT;
int interesting = 1;
char *traverse_path;

if (n >= ARRAY_SIZE(entry))
BUG("traverse_trees() called with too many trees (%d)", n);

for (i = 0; i < n; i++) {
tx[i].d = t[i];
tx[i].skip = NULL;
}

if (info->prev) {
strbuf_make_traverse_path(&base, info->prev,
info->name, info->namelen);
strbuf_addch(&base, '/');
traverse_path = xstrndup(base.buf, base.len);
} else {
traverse_path = xstrndup(info->name, info->pathlen);
}
info->traverse_path = traverse_path;
for (;;) {
int trees_used;
unsigned long mask, dirmask;
const char *first = NULL;
int first_len = 0;
struct name_entry *e = NULL;
int len;

for (i = 0; i < n; i++) {
e = entry + i;
extended_entry_extract(tx + i, e, NULL, 0);
}







for (i = 0; i < n; i++) {
e = entry + i;
if (!e->path)
continue;
len = tree_entry_len(e);
if (!first) {
first = e->path;
first_len = len;
continue;
}
if (name_compare(e->path, len, first, first_len) < 0) {
first = e->path;
first_len = len;
}
}

if (first) {
for (i = 0; i < n; i++) {
e = entry + i;
extended_entry_extract(tx + i, e, first, first_len);

if (!e->path)
continue;
len = tree_entry_len(e);
if (name_compare(e->path, len, first, first_len))
entry_clear(e);
}
}


mask = 0;
dirmask = 0;
for (i = 0; i < n; i++) {
if (!entry[i].path)
continue;
mask |= 1ul << i;
if (S_ISDIR(entry[i].mode))
dirmask |= 1ul << i;
e = &entry[i];
}
if (!mask)
break;
interesting = prune_traversal(istate, e, info, &base, interesting);
if (interesting < 0)
break;
if (interesting) {
trees_used = info->fn(n, mask, dirmask, entry, info);
if (trees_used < 0) {
error = trees_used;
if (!info->show_all_errors)
break;
}
mask &= trees_used;
}
for (i = 0; i < n; i++)
if (mask & (1ul << i))
update_extended_entry(tx + i, entry + i);
}
for (i = 0; i < n; i++)
free_extended_entry(tx + i);
free(traverse_path);
info->traverse_path = NULL;
strbuf_release(&base);
return error;
}

struct dir_state {
void *tree;
unsigned long size;
struct object_id oid;
};

static int find_tree_entry(struct repository *r, struct tree_desc *t,
const char *name, struct object_id *result,
unsigned short *mode)
{
int namelen = strlen(name);
while (t->size) {
const char *entry;
struct object_id oid;
int entrylen, cmp;

oidcpy(&oid, tree_entry_extract(t, &entry, mode));
entrylen = tree_entry_len(&t->entry);
update_tree_entry(t);
if (entrylen > namelen)
continue;
cmp = memcmp(name, entry, entrylen);
if (cmp > 0)
continue;
if (cmp < 0)
break;
if (entrylen == namelen) {
oidcpy(result, &oid);
return 0;
}
if (name[entrylen] != '/')
continue;
if (!S_ISDIR(*mode))
break;
if (++entrylen == namelen) {
oidcpy(result, &oid);
return 0;
}
return get_tree_entry(r, &oid, name + entrylen, result, mode);
}
return -1;
}

int get_tree_entry(struct repository *r,
const struct object_id *tree_oid,
const char *name,
struct object_id *oid,
unsigned short *mode)
{
int retval;
void *tree;
unsigned long size;
struct object_id root;

tree = read_object_with_reference(r, tree_oid, tree_type, &size, &root);
if (!tree)
return -1;

if (name[0] == '\0') {
oidcpy(oid, &root);
free(tree);
return 0;
}

if (!size) {
retval = -1;
} else {
struct tree_desc t;
init_tree_desc(&t, tree, size);
retval = find_tree_entry(r, &t, name, oid, mode);
}
free(tree);
return retval;
}






#define GET_TREE_ENTRY_FOLLOW_SYMLINKS_MAX_LINKS 40
















enum get_oid_result get_tree_entry_follow_symlinks(struct repository *r,
struct object_id *tree_oid, const char *name,
struct object_id *result, struct strbuf *result_path,
unsigned short *mode)
{
int retval = MISSING_OBJECT;
struct dir_state *parents = NULL;
size_t parents_alloc = 0;
size_t i, parents_nr = 0;
struct object_id current_tree_oid;
struct strbuf namebuf = STRBUF_INIT;
struct tree_desc t;
int follows_remaining = GET_TREE_ENTRY_FOLLOW_SYMLINKS_MAX_LINKS;

init_tree_desc(&t, NULL, 0UL);
strbuf_addstr(&namebuf, name);
oidcpy(&current_tree_oid, tree_oid);

while (1) {
int find_result;
char *first_slash;
char *remainder = NULL;

if (!t.buffer) {
void *tree;
struct object_id root;
unsigned long size;
tree = read_object_with_reference(r,
&current_tree_oid,
tree_type, &size,
&root);
if (!tree)
goto done;

ALLOC_GROW(parents, parents_nr + 1, parents_alloc);
parents[parents_nr].tree = tree;
parents[parents_nr].size = size;
oidcpy(&parents[parents_nr].oid, &root);
parents_nr++;

if (namebuf.buf[0] == '\0') {
oidcpy(result, &root);
retval = FOUND;
goto done;
}

if (!size)
goto done;


init_tree_desc(&t, tree, size);
}


while (namebuf.buf[0] == '/') {
strbuf_remove(&namebuf, 0, 1);
}


if ((first_slash = strchr(namebuf.buf, '/'))) {
*first_slash = 0;
remainder = first_slash + 1;
}

if (!strcmp(namebuf.buf, "..")) {
struct dir_state *parent;





if (parents_nr == 1) {
if (remainder)
*first_slash = '/';
strbuf_add(result_path, namebuf.buf,
namebuf.len);
*mode = 0;
retval = FOUND;
goto done;
}
parent = &parents[parents_nr - 1];
free(parent->tree);
parents_nr--;
parent = &parents[parents_nr - 1];
init_tree_desc(&t, parent->tree, parent->size);
strbuf_remove(&namebuf, 0, remainder ? 3 : 2);
continue;
}


if (namebuf.buf[0] == '\0') {
oidcpy(result, &parents[parents_nr - 1].oid);
retval = FOUND;
goto done;
}


find_result = find_tree_entry(r, &t, namebuf.buf,
&current_tree_oid, mode);
if (find_result) {
goto done;
}

if (S_ISDIR(*mode)) {
if (!remainder) {
oidcpy(result, &current_tree_oid);
retval = FOUND;
goto done;
}

t.buffer = NULL;
strbuf_remove(&namebuf, 0,
1 + first_slash - namebuf.buf);
} else if (S_ISREG(*mode)) {
if (!remainder) {
oidcpy(result, &current_tree_oid);
retval = FOUND;
} else {
retval = NOT_DIR;
}
goto done;
} else if (S_ISLNK(*mode)) {

unsigned long link_len;
size_t len;
char *contents, *contents_start;
struct dir_state *parent;
enum object_type type;

if (follows_remaining-- == 0) {

retval = SYMLINK_LOOP;
goto done;
}





retval = DANGLING_SYMLINK;

contents = repo_read_object_file(r,
&current_tree_oid, &type,
&link_len);

if (!contents)
goto done;

if (contents[0] == '/') {
strbuf_addstr(result_path, contents);
free(contents);
*mode = 0;
retval = FOUND;
goto done;
}

if (remainder)
len = first_slash - namebuf.buf;
else
len = namebuf.len;

contents_start = contents;

parent = &parents[parents_nr - 1];
init_tree_desc(&t, parent->tree, parent->size);
strbuf_splice(&namebuf, 0, len,
contents_start, link_len);
if (remainder)
namebuf.buf[link_len] = '/';
free(contents);
}
}
done:
for (i = 0; i < parents_nr; i++)
free(parents[i].tree);
free(parents);

strbuf_release(&namebuf);
return retval;
}

static int match_entry(const struct pathspec_item *item,
const struct name_entry *entry, int pathlen,
const char *match, int matchlen,
enum interesting *never_interesting)
{
int m = -1; 

if (item->magic & PATHSPEC_ICASE)









*never_interesting = entry_not_interesting;
else if (*never_interesting != entry_not_interesting) {









m = strncmp(match, entry->path,
(matchlen < pathlen) ? matchlen : pathlen);
if (m < 0)
return 0;














*never_interesting = entry_not_interesting;
}

if (pathlen > matchlen)
return 0;

if (matchlen > pathlen) {
if (match[pathlen] != '/')
return 0;
if (!S_ISDIR(entry->mode) && !S_ISGITLINK(entry->mode))
return 0;
}

if (m == -1)




m = ps_strncmp(item, match, entry->path, pathlen);






if (!m)







return 1;

return 0;
}


static int basecmp(const struct pathspec_item *item,
const char *base, const char *match, int len)
{
if (item->magic & PATHSPEC_ICASE) {
int ret, n = len > item->prefix ? item->prefix : len;
ret = strncmp(base, match, n);
if (ret)
return ret;
base += n;
match += n;
len -= n;
}
return ps_strncmp(item, base, match, len);
}

static int match_dir_prefix(const struct pathspec_item *item,
const char *base,
const char *match, int matchlen)
{
if (basecmp(item, base, match, matchlen))
return 0;





if (!matchlen ||
base[matchlen] == '/' ||
match[matchlen - 1] == '/')
return 1;


return 0;
}






static int match_wildcard_base(const struct pathspec_item *item,
const char *base, int baselen,
int *matched)
{
const char *match = item->match;

int matchlen = item->nowildcard_len;

if (baselen) {
int dirlen;




if (baselen >= matchlen) {
*matched = matchlen;
return !basecmp(item, base, match, matchlen);
}

dirlen = matchlen;
while (dirlen && match[dirlen - 1] != '/')
dirlen--;







if (basecmp(item, base, match, baselen))
return 0;
*matched = baselen;
} else
*matched = 0;






return entry_interesting;
}







static enum interesting do_match(struct index_state *istate,
const struct name_entry *entry,
struct strbuf *base, int base_offset,
const struct pathspec *ps,
int exclude)
{
int i;
int pathlen, baselen = base->len - base_offset;
enum interesting never_interesting = ps->has_wildcard ?
entry_not_interesting : all_entries_not_interesting;

GUARD_PATHSPEC(ps,
PATHSPEC_FROMTOP |
PATHSPEC_MAXDEPTH |
PATHSPEC_LITERAL |
PATHSPEC_GLOB |
PATHSPEC_ICASE |
PATHSPEC_EXCLUDE |
PATHSPEC_ATTR);

if (!ps->nr) {
if (!ps->recursive ||
!(ps->magic & PATHSPEC_MAXDEPTH) ||
ps->max_depth == -1)
return all_entries_interesting;
return within_depth(base->buf + base_offset, baselen,
!!S_ISDIR(entry->mode),
ps->max_depth) ?
entry_interesting : entry_not_interesting;
}

pathlen = tree_entry_len(entry);

for (i = ps->nr - 1; i >= 0; i--) {
const struct pathspec_item *item = ps->items+i;
const char *match = item->match;
const char *base_str = base->buf + base_offset;
int matchlen = item->len, matched = 0;

if ((!exclude && item->magic & PATHSPEC_EXCLUDE) ||
( exclude && !(item->magic & PATHSPEC_EXCLUDE)))
continue;

if (baselen >= matchlen) {

if (!match_dir_prefix(item, base_str, match, matchlen))
goto match_wildcards;

if (!ps->recursive ||
!(ps->magic & PATHSPEC_MAXDEPTH) ||
ps->max_depth == -1) {
if (!item->attr_match_nr)
return all_entries_interesting;
else
goto interesting;
}

if (within_depth(base_str + matchlen + 1,
baselen - matchlen - 1,
!!S_ISDIR(entry->mode),
ps->max_depth))
goto interesting;
else
return entry_not_interesting;
}


if (baselen == 0 || !basecmp(item, base_str, match, baselen)) {
if (match_entry(item, entry, pathlen,
match + baselen, matchlen - baselen,
&never_interesting))
goto interesting;

if (item->nowildcard_len < item->len) {
if (!git_fnmatch(item, match + baselen, entry->path,
item->nowildcard_len - baselen))
goto interesting;





if (ps->recursive && S_ISDIR(entry->mode))
return entry_interesting;








if (ps->recurse_submodules &&
S_ISGITLINK(entry->mode) &&
!ps_strncmp(item, match + baselen,
entry->path,
item->nowildcard_len - baselen))
goto interesting;
}

continue;
}

match_wildcards:
if (item->nowildcard_len == item->len)
continue;

if (item->nowildcard_len &&
!match_wildcard_base(item, base_str, baselen, &matched))
continue;

















strbuf_add(base, entry->path, pathlen);

if (!git_fnmatch(item, match, base->buf + base_offset,
item->nowildcard_len)) {
strbuf_setlen(base, base_offset + baselen);
goto interesting;
}








if (ps->recurse_submodules && S_ISGITLINK(entry->mode) &&
!ps_strncmp(item, match, base->buf + base_offset,
item->nowildcard_len)) {
strbuf_setlen(base, base_offset + baselen);
goto interesting;
}

strbuf_setlen(base, base_offset + baselen);








if (ps->recursive && S_ISDIR(entry->mode))
return entry_interesting;
continue;
interesting:
if (item->attr_match_nr) {
int ret;






never_interesting = entry_not_interesting;










if (S_ISDIR(entry->mode))
return entry_interesting;

strbuf_add(base, entry->path, pathlen);
ret = match_pathspec_attrs(istate, base->buf + base_offset,
base->len - base_offset, item);
strbuf_setlen(base, base_offset + baselen);
if (!ret)
continue;
}
return entry_interesting;
}
return never_interesting; 
}







enum interesting tree_entry_interesting(struct index_state *istate,
const struct name_entry *entry,
struct strbuf *base, int base_offset,
const struct pathspec *ps)
{
enum interesting positive, negative;
positive = do_match(istate, entry, base, base_offset, ps, 0);
































if (!(ps->magic & PATHSPEC_EXCLUDE) ||
positive <= entry_not_interesting) 
return positive;

negative = do_match(istate, entry, base, base_offset, ps, 1);


if (positive == all_entries_interesting &&
negative == entry_not_interesting)
return entry_interesting;


if (negative <= entry_not_interesting)
return positive;


if (S_ISDIR(entry->mode) &&
positive >= entry_interesting &&
negative == entry_interesting)
return entry_interesting;

if ((positive == entry_interesting &&
negative >= entry_interesting) || 
(positive == all_entries_interesting &&
negative == entry_interesting)) 
return entry_not_interesting;

return all_entries_not_interesting; 
}
