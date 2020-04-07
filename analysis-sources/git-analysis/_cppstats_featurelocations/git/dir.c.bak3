






#include "cache.h"
#include "config.h"
#include "dir.h"
#include "object-store.h"
#include "attr.h"
#include "refs.h"
#include "wildmatch.h"
#include "pathspec.h"
#include "utf8.h"
#include "varint.h"
#include "ewah/ewok.h"
#include "fsmonitor.h"
#include "submodule-config.h"







enum path_treatment {
path_none = 0,
path_recurse,
path_excluded,
path_untracked
};




struct cached_dir {
DIR *fdir;
struct untracked_cache_dir *untracked;
int nr_files;
int nr_dirs;

const char *d_name;
int d_type;
const char *file;
struct untracked_cache_dir *ucd;
};

static enum path_treatment read_directory_recursive(struct dir_struct *dir,
struct index_state *istate, const char *path, int len,
struct untracked_cache_dir *untracked,
int check_only, int stop_at_first_file, const struct pathspec *pathspec);
static int resolve_dtype(int dtype, struct index_state *istate,
const char *path, int len);

int count_slashes(const char *s)
{
int cnt = 0;
while (*s)
if (*s++ == '/')
cnt++;
return cnt;
}

int fspathcmp(const char *a, const char *b)
{
return ignore_case ? strcasecmp(a, b) : strcmp(a, b);
}

int fspathncmp(const char *a, const char *b, size_t count)
{
return ignore_case ? strncasecmp(a, b, count) : strncmp(a, b, count);
}

int git_fnmatch(const struct pathspec_item *item,
const char *pattern, const char *string,
int prefix)
{
if (prefix > 0) {
if (ps_strncmp(item, pattern, string, prefix))
return WM_NOMATCH;
pattern += prefix;
string += prefix;
}
if (item->flags & PATHSPEC_ONESTAR) {
int pattern_len = strlen(++pattern);
int string_len = strlen(string);
return string_len < pattern_len ||
ps_strcmp(item, pattern,
string + string_len - pattern_len);
}
if (item->magic & PATHSPEC_GLOB)
return wildmatch(pattern, string,
WM_PATHNAME |
(item->magic & PATHSPEC_ICASE ? WM_CASEFOLD : 0));
else

return wildmatch(pattern, string,
item->magic & PATHSPEC_ICASE ? WM_CASEFOLD : 0);
}

static int fnmatch_icase_mem(const char *pattern, int patternlen,
const char *string, int stringlen,
int flags)
{
int match_status;
struct strbuf pat_buf = STRBUF_INIT;
struct strbuf str_buf = STRBUF_INIT;
const char *use_pat = pattern;
const char *use_str = string;

if (pattern[patternlen]) {
strbuf_add(&pat_buf, pattern, patternlen);
use_pat = pat_buf.buf;
}
if (string[stringlen]) {
strbuf_add(&str_buf, string, stringlen);
use_str = str_buf.buf;
}

if (ignore_case)
flags |= WM_CASEFOLD;
match_status = wildmatch(use_pat, use_str, flags);

strbuf_release(&pat_buf);
strbuf_release(&str_buf);

return match_status;
}

static size_t common_prefix_len(const struct pathspec *pathspec)
{
int n;
size_t max = 0;








GUARD_PATHSPEC(pathspec,
PATHSPEC_FROMTOP |
PATHSPEC_MAXDEPTH |
PATHSPEC_LITERAL |
PATHSPEC_GLOB |
PATHSPEC_ICASE |
PATHSPEC_EXCLUDE |
PATHSPEC_ATTR);

for (n = 0; n < pathspec->nr; n++) {
size_t i = 0, len = 0, item_len;
if (pathspec->items[n].magic & PATHSPEC_EXCLUDE)
continue;
if (pathspec->items[n].magic & PATHSPEC_ICASE)
item_len = pathspec->items[n].prefix;
else
item_len = pathspec->items[n].nowildcard_len;
while (i < item_len && (n == 0 || i < max)) {
char c = pathspec->items[n].match[i];
if (c != pathspec->items[0].match[i])
break;
if (c == '/')
len = i + 1;
i++;
}
if (n == 0 || len < max) {
max = len;
if (!max)
break;
}
}
return max;
}





char *common_prefix(const struct pathspec *pathspec)
{
unsigned long len = common_prefix_len(pathspec);

return len ? xmemdupz(pathspec->items[0].match, len) : NULL;
}

int fill_directory(struct dir_struct *dir,
struct index_state *istate,
const struct pathspec *pathspec)
{
const char *prefix;
size_t prefix_len;





prefix_len = common_prefix_len(pathspec);
prefix = prefix_len ? pathspec->items[0].match : "";


read_directory(dir, istate, prefix, prefix_len, pathspec);

return prefix_len;
}

int within_depth(const char *name, int namelen,
int depth, int max_depth)
{
const char *cp = name, *cpe = name + namelen;

while (cp < cpe) {
if (*cp++ != '/')
continue;
depth++;
if (depth > max_depth)
return 0;
}
return 1;
}













static int do_read_blob(const struct object_id *oid, struct oid_stat *oid_stat,
size_t *size_out, char **data_out)
{
enum object_type type;
unsigned long sz;
char *data;

*size_out = 0;
*data_out = NULL;

data = read_object_file(oid, &type, &sz);
if (!data || type != OBJ_BLOB) {
free(data);
return -1;
}

if (oid_stat) {
memset(&oid_stat->stat, 0, sizeof(oid_stat->stat));
oidcpy(&oid_stat->oid, oid);
}

if (sz == 0) {
free(data);
return 0;
}

if (data[sz - 1] != '\n') {
data = xrealloc(data, st_add(sz, 1));
data[sz++] = '\n';
}

*size_out = xsize_t(sz);
*data_out = data;

return 1;
}

#define DO_MATCH_EXCLUDE (1<<0)
#define DO_MATCH_DIRECTORY (1<<1)
#define DO_MATCH_LEADING_PATHSPEC (1<<2)
























static int match_pathspec_item(const struct index_state *istate,
const struct pathspec_item *item, int prefix,
const char *name, int namelen, unsigned flags)
{

const char *match = item->match + prefix;
int matchlen = item->len - prefix;






























if (item->prefix && (item->magic & PATHSPEC_ICASE) &&
strncmp(item->match, name - prefix, item->prefix))
return 0;

if (item->attr_match_nr &&
!match_pathspec_attrs(istate, name, namelen, item))
return 0;


if (!*match)
return MATCHED_RECURSIVELY;

if (matchlen <= namelen && !ps_strncmp(item, match, name, matchlen)) {
if (matchlen == namelen)
return MATCHED_EXACTLY;

if (match[matchlen-1] == '/' || name[matchlen] == '/')
return MATCHED_RECURSIVELY;
} else if ((flags & DO_MATCH_DIRECTORY) &&
match[matchlen - 1] == '/' &&
namelen == matchlen - 1 &&
!ps_strncmp(item, match, name, namelen))
return MATCHED_EXACTLY;

if (item->nowildcard_len < item->len &&
!git_fnmatch(item, match, name,
item->nowildcard_len - prefix))
return MATCHED_FNMATCH;


if (flags & DO_MATCH_LEADING_PATHSPEC) {

int offset = name[namelen-1] == '/' ? 1 : 0;
if ((namelen < matchlen) &&
(match[namelen-offset] == '/') &&
!ps_strncmp(item, match, name, namelen))
return MATCHED_RECURSIVELY_LEADING_PATHSPEC;


if (item->nowildcard_len < item->len &&
ps_strncmp(item, match, name,
item->nowildcard_len - prefix))
return 0;





if (item->nowildcard_len == item->len)
return 0;










return MATCHED_RECURSIVELY_LEADING_PATHSPEC;
}

return 0;
}
















static int do_match_pathspec(const struct index_state *istate,
const struct pathspec *ps,
const char *name, int namelen,
int prefix, char *seen,
unsigned flags)
{
int i, retval = 0, exclude = flags & DO_MATCH_EXCLUDE;

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
return MATCHED_RECURSIVELY;

if (within_depth(name, namelen, 0, ps->max_depth))
return MATCHED_EXACTLY;
else
return 0;
}

name += prefix;
namelen -= prefix;

for (i = ps->nr - 1; i >= 0; i--) {
int how;

if ((!exclude && ps->items[i].magic & PATHSPEC_EXCLUDE) ||
( exclude && !(ps->items[i].magic & PATHSPEC_EXCLUDE)))
continue;

if (seen && seen[i] == MATCHED_EXACTLY)
continue;




if (seen && ps->items[i].magic & PATHSPEC_EXCLUDE)
seen[i] = MATCHED_FNMATCH;
how = match_pathspec_item(istate, ps->items+i, prefix, name,
namelen, flags);
if (ps->recursive &&
(ps->magic & PATHSPEC_MAXDEPTH) &&
ps->max_depth != -1 &&
how && how != MATCHED_FNMATCH) {
int len = ps->items[i].len;
if (name[len] == '/')
len++;
if (within_depth(name+len, namelen-len, 0, ps->max_depth))
how = MATCHED_EXACTLY;
else
how = 0;
}
if (how) {
if (retval < how)
retval = how;
if (seen && seen[i] < how)
seen[i] = how;
}
}
return retval;
}

int match_pathspec(const struct index_state *istate,
const struct pathspec *ps,
const char *name, int namelen,
int prefix, char *seen, int is_dir)
{
int positive, negative;
unsigned flags = is_dir ? DO_MATCH_DIRECTORY : 0;
positive = do_match_pathspec(istate, ps, name, namelen,
prefix, seen, flags);
if (!(ps->magic & PATHSPEC_EXCLUDE) || !positive)
return positive;
negative = do_match_pathspec(istate, ps, name, namelen,
prefix, seen,
flags | DO_MATCH_EXCLUDE);
return negative ? 0 : positive;
}




int submodule_path_match(const struct index_state *istate,
const struct pathspec *ps,
const char *submodule_name,
char *seen)
{
int matched = do_match_pathspec(istate, ps, submodule_name,
strlen(submodule_name),
0, seen,
DO_MATCH_DIRECTORY |
DO_MATCH_LEADING_PATHSPEC);
return matched;
}

int report_path_error(const char *ps_matched,
const struct pathspec *pathspec)
{



int num, errors = 0;
for (num = 0; num < pathspec->nr; num++) {
int other, found_dup;

if (ps_matched[num])
continue;






for (found_dup = other = 0;
!found_dup && other < pathspec->nr;
other++) {
if (other == num || !ps_matched[other])
continue;
if (!strcmp(pathspec->items[other].original,
pathspec->items[num].original))



found_dup = 1;
}
if (found_dup)
continue;

error(_("pathspec '%s' did not match any file(s) known to git"),
pathspec->items[num].original);
errors++;
}
return errors;
}




int simple_length(const char *match)
{
int len = -1;

for (;;) {
unsigned char c = *match++;
len++;
if (c == '\0' || is_glob_special(c))
return len;
}
}

int no_wildcard(const char *string)
{
return string[simple_length(string)] == '\0';
}

void parse_path_pattern(const char **pattern,
int *patternlen,
unsigned *flags,
int *nowildcardlen)
{
const char *p = *pattern;
size_t i, len;

*flags = 0;
if (*p == '!') {
*flags |= PATTERN_FLAG_NEGATIVE;
p++;
}
len = strlen(p);
if (len && p[len - 1] == '/') {
len--;
*flags |= PATTERN_FLAG_MUSTBEDIR;
}
for (i = 0; i < len; i++) {
if (p[i] == '/')
break;
}
if (i == len)
*flags |= PATTERN_FLAG_NODIR;
*nowildcardlen = simple_length(p);





if (*nowildcardlen > len)
*nowildcardlen = len;
if (*p == '*' && no_wildcard(p + 1))
*flags |= PATTERN_FLAG_ENDSWITH;
*pattern = p;
*patternlen = len;
}

int pl_hashmap_cmp(const void *unused_cmp_data,
const struct hashmap_entry *a,
const struct hashmap_entry *b,
const void *key)
{
const struct pattern_entry *ee1 =
container_of(a, struct pattern_entry, ent);
const struct pattern_entry *ee2 =
container_of(b, struct pattern_entry, ent);

size_t min_len = ee1->patternlen <= ee2->patternlen
? ee1->patternlen
: ee2->patternlen;

if (ignore_case)
return strncasecmp(ee1->pattern, ee2->pattern, min_len);
return strncmp(ee1->pattern, ee2->pattern, min_len);
}

static char *dup_and_filter_pattern(const char *pattern)
{
char *set, *read;
size_t count = 0;
char *result = xstrdup(pattern);

set = result;
read = result;

while (*read) {

if (*read == '\\')
read++;

*set = *read;

set++;
read++;
count++;
}
*set = 0;

if (count > 2 &&
*(set - 1) == '*' &&
*(set - 2) == '/')
*(set - 2) = 0;

return result;
}

static void add_pattern_to_hashsets(struct pattern_list *pl, struct path_pattern *given)
{
struct pattern_entry *translated;
char *truncated;
char *data = NULL;
const char *prev, *cur, *next;

if (!pl->use_cone_patterns)
return;

if (given->flags & PATTERN_FLAG_NEGATIVE &&
given->flags & PATTERN_FLAG_MUSTBEDIR &&
!strcmp(given->pattern, "/*")) {
pl->full_cone = 0;
return;
}

if (!given->flags && !strcmp(given->pattern, "/*")) {
pl->full_cone = 1;
return;
}

if (given->patternlen < 2 ||
*given->pattern == '*' ||
strstr(given->pattern, "**")) {

warning(_("unrecognized pattern: '%s'"), given->pattern);
goto clear_hashmaps;
}

prev = given->pattern;
cur = given->pattern + 1;
next = given->pattern + 2;

while (*cur) {

if (!is_glob_special(*cur))
goto increment;


if (*prev == '\\')
goto increment;


if (*cur == '\\' &&
is_glob_special(*next))
goto increment;


if (*prev == '/' &&
*cur == '*' &&
*next == 0)
goto increment;


warning(_("unrecognized pattern: '%s'"), given->pattern);
goto clear_hashmaps;

increment:
prev++;
cur++;
next++;
}

if (given->patternlen > 2 &&
!strcmp(given->pattern + given->patternlen - 2, "/*")) {
if (!(given->flags & PATTERN_FLAG_NEGATIVE)) {

warning(_("unrecognized pattern: '%s'"), given->pattern);
goto clear_hashmaps;
}

truncated = dup_and_filter_pattern(given->pattern);

translated = xmalloc(sizeof(struct pattern_entry));
translated->pattern = truncated;
translated->patternlen = given->patternlen - 2;
hashmap_entry_init(&translated->ent,
ignore_case ?
strihash(translated->pattern) :
strhash(translated->pattern));

if (!hashmap_get_entry(&pl->recursive_hashmap,
translated, ent, NULL)) {

warning(_("unrecognized negative pattern: '%s'"),
given->pattern);
free(truncated);
free(translated);
goto clear_hashmaps;
}

hashmap_add(&pl->parent_hashmap, &translated->ent);
hashmap_remove(&pl->recursive_hashmap, &translated->ent, &data);
free(data);
return;
}

if (given->flags & PATTERN_FLAG_NEGATIVE) {
warning(_("unrecognized negative pattern: '%s'"),
given->pattern);
goto clear_hashmaps;
}

translated = xmalloc(sizeof(struct pattern_entry));

translated->pattern = dup_and_filter_pattern(given->pattern);
translated->patternlen = given->patternlen;
hashmap_entry_init(&translated->ent,
ignore_case ?
strihash(translated->pattern) :
strhash(translated->pattern));

hashmap_add(&pl->recursive_hashmap, &translated->ent);

if (hashmap_get_entry(&pl->parent_hashmap, translated, ent, NULL)) {

warning(_("your sparse-checkout file may have issues: pattern '%s' is repeated"),
given->pattern);
hashmap_remove(&pl->parent_hashmap, &translated->ent, &data);
free(data);
free(translated);
}

return;

clear_hashmaps:
warning(_("disabling cone pattern matching"));
hashmap_free_entries(&pl->parent_hashmap, struct pattern_entry, ent);
hashmap_free_entries(&pl->recursive_hashmap, struct pattern_entry, ent);
pl->use_cone_patterns = 0;
}

static int hashmap_contains_path(struct hashmap *map,
struct strbuf *pattern)
{
struct pattern_entry p;


p.pattern = pattern->buf;
p.patternlen = pattern->len;
hashmap_entry_init(&p.ent,
ignore_case ?
strihash(p.pattern) :
strhash(p.pattern));
return !!hashmap_get_entry(map, &p, ent, NULL);
}

int hashmap_contains_parent(struct hashmap *map,
const char *path,
struct strbuf *buffer)
{
char *slash_pos;

strbuf_setlen(buffer, 0);

if (path[0] != '/')
strbuf_addch(buffer, '/');

strbuf_addstr(buffer, path);

slash_pos = strrchr(buffer->buf, '/');

while (slash_pos > buffer->buf) {
strbuf_setlen(buffer, slash_pos - buffer->buf);

if (hashmap_contains_path(map, buffer))
return 1;

slash_pos = strrchr(buffer->buf, '/');
}

return 0;
}

void add_pattern(const char *string, const char *base,
int baselen, struct pattern_list *pl, int srcpos)
{
struct path_pattern *pattern;
int patternlen;
unsigned flags;
int nowildcardlen;

parse_path_pattern(&string, &patternlen, &flags, &nowildcardlen);
if (flags & PATTERN_FLAG_MUSTBEDIR) {
FLEXPTR_ALLOC_MEM(pattern, pattern, string, patternlen);
} else {
pattern = xmalloc(sizeof(*pattern));
pattern->pattern = string;
}
pattern->patternlen = patternlen;
pattern->nowildcardlen = nowildcardlen;
pattern->base = base;
pattern->baselen = baselen;
pattern->flags = flags;
pattern->srcpos = srcpos;
ALLOC_GROW(pl->patterns, pl->nr + 1, pl->alloc);
pl->patterns[pl->nr++] = pattern;
pattern->pl = pl;

add_pattern_to_hashsets(pl, pattern);
}

static int read_skip_worktree_file_from_index(const struct index_state *istate,
const char *path,
size_t *size_out, char **data_out,
struct oid_stat *oid_stat)
{
int pos, len;

len = strlen(path);
pos = index_name_pos(istate, path, len);
if (pos < 0)
return -1;
if (!ce_skip_worktree(istate->cache[pos]))
return -1;

return do_read_blob(&istate->cache[pos]->oid, oid_stat, size_out, data_out);
}





void clear_pattern_list(struct pattern_list *pl)
{
int i;

for (i = 0; i < pl->nr; i++)
free(pl->patterns[i]);
free(pl->patterns);
free(pl->filebuf);

memset(pl, 0, sizeof(*pl));
}

static void trim_trailing_spaces(char *buf)
{
char *p, *last_space = NULL;

for (p = buf; *p; p++)
switch (*p) {
case ' ':
if (!last_space)
last_space = p;
break;
case '\\':
p++;
if (!*p)
return;

default:
last_space = NULL;
}

if (last_space)
*last_space = '\0';
}








static struct untracked_cache_dir *lookup_untracked(struct untracked_cache *uc,
struct untracked_cache_dir *dir,
const char *name, int len)
{
int first, last;
struct untracked_cache_dir *d;
if (!dir)
return NULL;
if (len && name[len - 1] == '/')
len--;
first = 0;
last = dir->dirs_nr;
while (last > first) {
int cmp, next = first + ((last - first) >> 1);
d = dir->dirs[next];
cmp = strncmp(name, d->name, len);
if (!cmp && strlen(d->name) > len)
cmp = -1;
if (!cmp)
return d;
if (cmp < 0) {
last = next;
continue;
}
first = next+1;
}

uc->dir_created++;
FLEX_ALLOC_MEM(d, name, name, len);

ALLOC_GROW(dir->dirs, dir->dirs_nr + 1, dir->dirs_alloc);
MOVE_ARRAY(dir->dirs + first + 1, dir->dirs + first,
dir->dirs_nr - first);
dir->dirs_nr++;
dir->dirs[first] = d;
return d;
}

static void do_invalidate_gitignore(struct untracked_cache_dir *dir)
{
int i;
dir->valid = 0;
dir->untracked_nr = 0;
for (i = 0; i < dir->dirs_nr; i++)
do_invalidate_gitignore(dir->dirs[i]);
}

static void invalidate_gitignore(struct untracked_cache *uc,
struct untracked_cache_dir *dir)
{
uc->gitignore_invalidated++;
do_invalidate_gitignore(dir);
}

static void invalidate_directory(struct untracked_cache *uc,
struct untracked_cache_dir *dir)
{
int i;







if (dir->valid)
uc->dir_invalidated++;

dir->valid = 0;
dir->untracked_nr = 0;
for (i = 0; i < dir->dirs_nr; i++)
dir->dirs[i]->recurse = 0;
}

static int add_patterns_from_buffer(char *buf, size_t size,
const char *base, int baselen,
struct pattern_list *pl);










static int add_patterns(const char *fname, const char *base, int baselen,
struct pattern_list *pl, struct index_state *istate,
struct oid_stat *oid_stat)
{
struct stat st;
int r;
int fd;
size_t size = 0;
char *buf;

fd = open(fname, O_RDONLY);
if (fd < 0 || fstat(fd, &st) < 0) {
if (fd < 0)
warn_on_fopen_errors(fname);
else
close(fd);
if (!istate)
return -1;
r = read_skip_worktree_file_from_index(istate, fname,
&size, &buf,
oid_stat);
if (r != 1)
return r;
} else {
size = xsize_t(st.st_size);
if (size == 0) {
if (oid_stat) {
fill_stat_data(&oid_stat->stat, &st);
oidcpy(&oid_stat->oid, the_hash_algo->empty_blob);
oid_stat->valid = 1;
}
close(fd);
return 0;
}
buf = xmallocz(size);
if (read_in_full(fd, buf, size) != size) {
free(buf);
close(fd);
return -1;
}
buf[size++] = '\n';
close(fd);
if (oid_stat) {
int pos;
if (oid_stat->valid &&
!match_stat_data_racy(istate, &oid_stat->stat, &st))
; 
else if (istate &&
(pos = index_name_pos(istate, fname, strlen(fname))) >= 0 &&
!ce_stage(istate->cache[pos]) &&
ce_uptodate(istate->cache[pos]) &&
!would_convert_to_git(istate, fname))
oidcpy(&oid_stat->oid,
&istate->cache[pos]->oid);
else
hash_object_file(the_hash_algo, buf, size,
"blob", &oid_stat->oid);
fill_stat_data(&oid_stat->stat, &st);
oid_stat->valid = 1;
}
}

add_patterns_from_buffer(buf, size, base, baselen, pl);
return 0;
}

static int add_patterns_from_buffer(char *buf, size_t size,
const char *base, int baselen,
struct pattern_list *pl)
{
int i, lineno = 1;
char *entry;

hashmap_init(&pl->recursive_hashmap, pl_hashmap_cmp, NULL, 0);
hashmap_init(&pl->parent_hashmap, pl_hashmap_cmp, NULL, 0);

pl->filebuf = buf;

if (skip_utf8_bom(&buf, size))
size -= buf - pl->filebuf;

entry = buf;

for (i = 0; i < size; i++) {
if (buf[i] == '\n') {
if (entry != buf + i && entry[0] != '#') {
buf[i - (i && buf[i-1] == '\r')] = 0;
trim_trailing_spaces(entry);
add_pattern(entry, base, baselen, pl, lineno);
}
lineno++;
entry = buf + i + 1;
}
}
return 0;
}

int add_patterns_from_file_to_list(const char *fname, const char *base,
int baselen, struct pattern_list *pl,
struct index_state *istate)
{
return add_patterns(fname, base, baselen, pl, istate, NULL);
}

int add_patterns_from_blob_to_list(
struct object_id *oid,
const char *base, int baselen,
struct pattern_list *pl)
{
char *buf;
size_t size;
int r;

r = do_read_blob(oid, NULL, &size, &buf);
if (r != 1)
return r;

add_patterns_from_buffer(buf, size, base, baselen, pl);
return 0;
}

struct pattern_list *add_pattern_list(struct dir_struct *dir,
int group_type, const char *src)
{
struct pattern_list *pl;
struct exclude_list_group *group;

group = &dir->exclude_list_group[group_type];
ALLOC_GROW(group->pl, group->nr + 1, group->alloc);
pl = &group->pl[group->nr++];
memset(pl, 0, sizeof(*pl));
pl->src = src;
return pl;
}




static void add_patterns_from_file_1(struct dir_struct *dir, const char *fname,
struct oid_stat *oid_stat)
{
struct pattern_list *pl;





if (!dir->untracked)
dir->unmanaged_exclude_files++;
pl = add_pattern_list(dir, EXC_FILE, fname);
if (add_patterns(fname, "", 0, pl, NULL, oid_stat) < 0)
die(_("cannot use %s as an exclude file"), fname);
}

void add_patterns_from_file(struct dir_struct *dir, const char *fname)
{
dir->unmanaged_exclude_files++; 
add_patterns_from_file_1(dir, fname, NULL);
}

int match_basename(const char *basename, int basenamelen,
const char *pattern, int prefix, int patternlen,
unsigned flags)
{
if (prefix == patternlen) {
if (patternlen == basenamelen &&
!fspathncmp(pattern, basename, basenamelen))
return 1;
} else if (flags & PATTERN_FLAG_ENDSWITH) {

if (patternlen - 1 <= basenamelen &&
!fspathncmp(pattern + 1,
basename + basenamelen - (patternlen - 1),
patternlen - 1))
return 1;
} else {
if (fnmatch_icase_mem(pattern, patternlen,
basename, basenamelen,
0) == 0)
return 1;
}
return 0;
}

int match_pathname(const char *pathname, int pathlen,
const char *base, int baselen,
const char *pattern, int prefix, int patternlen,
unsigned flags)
{
const char *name;
int namelen;





if (*pattern == '/') {
pattern++;
patternlen--;
prefix--;
}





if (pathlen < baselen + 1 ||
(baselen && pathname[baselen] != '/') ||
fspathncmp(pathname, base, baselen))
return 0;

namelen = baselen ? pathlen - baselen - 1 : pathlen;
name = pathname + pathlen - namelen;

if (prefix) {




if (prefix > namelen)
return 0;

if (fspathncmp(pattern, name, prefix))
return 0;
pattern += prefix;
patternlen -= prefix;
name += prefix;
namelen -= prefix;






if (!patternlen && !namelen)
return 1;
}

return fnmatch_icase_mem(pattern, patternlen,
name, namelen,
WM_PATHNAME) == 0;
}







static struct path_pattern *last_matching_pattern_from_list(const char *pathname,
int pathlen,
const char *basename,
int *dtype,
struct pattern_list *pl,
struct index_state *istate)
{
struct path_pattern *res = NULL; 
int i;

if (!pl->nr)
return NULL; 

for (i = pl->nr - 1; 0 <= i; i--) {
struct path_pattern *pattern = pl->patterns[i];
const char *exclude = pattern->pattern;
int prefix = pattern->nowildcardlen;

if (pattern->flags & PATTERN_FLAG_MUSTBEDIR) {
*dtype = resolve_dtype(*dtype, istate, pathname, pathlen);
if (*dtype != DT_DIR)
continue;
}

if (pattern->flags & PATTERN_FLAG_NODIR) {
if (match_basename(basename,
pathlen - (basename - pathname),
exclude, prefix, pattern->patternlen,
pattern->flags)) {
res = pattern;
break;
}
continue;
}

assert(pattern->baselen == 0 ||
pattern->base[pattern->baselen - 1] == '/');
if (match_pathname(pathname, pathlen,
pattern->base,
pattern->baselen ? pattern->baselen - 1 : 0,
exclude, prefix, pattern->patternlen,
pattern->flags)) {
res = pattern;
break;
}
}
return res;
}







enum pattern_match_result path_matches_pattern_list(
const char *pathname, int pathlen,
const char *basename, int *dtype,
struct pattern_list *pl,
struct index_state *istate)
{
struct path_pattern *pattern;
struct strbuf parent_pathname = STRBUF_INIT;
int result = NOT_MATCHED;
const char *slash_pos;

if (!pl->use_cone_patterns) {
pattern = last_matching_pattern_from_list(pathname, pathlen, basename,
dtype, pl, istate);
if (pattern) {
if (pattern->flags & PATTERN_FLAG_NEGATIVE)
return NOT_MATCHED;
else
return MATCHED;
}

return UNDECIDED;
}

if (pl->full_cone)
return MATCHED;

strbuf_addch(&parent_pathname, '/');
strbuf_add(&parent_pathname, pathname, pathlen);

if (hashmap_contains_path(&pl->recursive_hashmap,
&parent_pathname)) {
result = MATCHED_RECURSIVE;
goto done;
}

slash_pos = strrchr(parent_pathname.buf, '/');

if (slash_pos == parent_pathname.buf) {

result = MATCHED;
goto done;
}

strbuf_setlen(&parent_pathname, slash_pos - parent_pathname.buf);

if (hashmap_contains_path(&pl->parent_hashmap, &parent_pathname)) {
result = MATCHED;
goto done;
}

if (hashmap_contains_parent(&pl->recursive_hashmap,
pathname,
&parent_pathname))
result = MATCHED_RECURSIVE;

done:
strbuf_release(&parent_pathname);
return result;
}

static struct path_pattern *last_matching_pattern_from_lists(
struct dir_struct *dir, struct index_state *istate,
const char *pathname, int pathlen,
const char *basename, int *dtype_p)
{
int i, j;
struct exclude_list_group *group;
struct path_pattern *pattern;
for (i = EXC_CMDL; i <= EXC_FILE; i++) {
group = &dir->exclude_list_group[i];
for (j = group->nr - 1; j >= 0; j--) {
pattern = last_matching_pattern_from_list(
pathname, pathlen, basename, dtype_p,
&group->pl[j], istate);
if (pattern)
return pattern;
}
}
return NULL;
}





static void prep_exclude(struct dir_struct *dir,
struct index_state *istate,
const char *base, int baselen)
{
struct exclude_list_group *group;
struct pattern_list *pl;
struct exclude_stack *stk = NULL;
struct untracked_cache_dir *untracked;
int current;

group = &dir->exclude_list_group[EXC_DIRS];






while ((stk = dir->exclude_stack) != NULL) {
if (stk->baselen <= baselen &&
!strncmp(dir->basebuf.buf, base, stk->baselen))
break;
pl = &group->pl[dir->exclude_stack->exclude_ix];
dir->exclude_stack = stk->prev;
dir->pattern = NULL;
free((char *)pl->src); 
clear_pattern_list(pl);
free(stk);
group->nr--;
}


if (dir->pattern)
return;






if (!dir->basebuf.buf)
strbuf_init(&dir->basebuf, PATH_MAX);


current = stk ? stk->baselen : -1;
strbuf_setlen(&dir->basebuf, current < 0 ? 0 : current);
if (dir->untracked)
untracked = stk ? stk->ucd : dir->untracked->root;
else
untracked = NULL;

while (current < baselen) {
const char *cp;
struct oid_stat oid_stat;

stk = xcalloc(1, sizeof(*stk));
if (current < 0) {
cp = base;
current = 0;
} else {
cp = strchr(base + current + 1, '/');
if (!cp)
die("oops in prep_exclude");
cp++;
untracked =
lookup_untracked(dir->untracked, untracked,
base + current,
cp - base - current);
}
stk->prev = dir->exclude_stack;
stk->baselen = cp - base;
stk->exclude_ix = group->nr;
stk->ucd = untracked;
pl = add_pattern_list(dir, EXC_DIRS, NULL);
strbuf_add(&dir->basebuf, base + current, stk->baselen - current);
assert(stk->baselen == dir->basebuf.len);


if (stk->baselen) {
int dt = DT_DIR;
dir->basebuf.buf[stk->baselen - 1] = 0;
dir->pattern = last_matching_pattern_from_lists(dir,
istate,
dir->basebuf.buf, stk->baselen - 1,
dir->basebuf.buf + current, &dt);
dir->basebuf.buf[stk->baselen - 1] = '/';
if (dir->pattern &&
dir->pattern->flags & PATTERN_FLAG_NEGATIVE)
dir->pattern = NULL;
if (dir->pattern) {
dir->exclude_stack = stk;
return;
}
}


oidclr(&oid_stat.oid);
oid_stat.valid = 0;
if (dir->exclude_per_dir &&





(!untracked || !untracked->valid ||






!is_null_oid(&untracked->exclude_oid))) {








struct strbuf sb = STRBUF_INIT;
strbuf_addbuf(&sb, &dir->basebuf);
strbuf_addstr(&sb, dir->exclude_per_dir);
pl->src = strbuf_detach(&sb, NULL);
add_patterns(pl->src, pl->src, stk->baselen, pl, istate,
untracked ? &oid_stat : NULL);
}














if (untracked &&
!oideq(&oid_stat.oid, &untracked->exclude_oid)) {
invalidate_gitignore(dir->untracked, untracked);
oidcpy(&untracked->exclude_oid, &oid_stat.oid);
}
dir->exclude_stack = stk;
current = stk->baselen;
}
strbuf_setlen(&dir->basebuf, baselen);
}







struct path_pattern *last_matching_pattern(struct dir_struct *dir,
struct index_state *istate,
const char *pathname,
int *dtype_p)
{
int pathlen = strlen(pathname);
const char *basename = strrchr(pathname, '/');
basename = (basename) ? basename+1 : pathname;

prep_exclude(dir, istate, pathname, basename-pathname);

if (dir->pattern)
return dir->pattern;

return last_matching_pattern_from_lists(dir, istate, pathname, pathlen,
basename, dtype_p);
}






int is_excluded(struct dir_struct *dir, struct index_state *istate,
const char *pathname, int *dtype_p)
{
struct path_pattern *pattern =
last_matching_pattern(dir, istate, pathname, dtype_p);
if (pattern)
return pattern->flags & PATTERN_FLAG_NEGATIVE ? 0 : 1;
return 0;
}

static struct dir_entry *dir_entry_new(const char *pathname, int len)
{
struct dir_entry *ent;

FLEX_ALLOC_MEM(ent, name, pathname, len);
ent->len = len;
return ent;
}

static struct dir_entry *dir_add_name(struct dir_struct *dir,
struct index_state *istate,
const char *pathname, int len)
{
if (index_file_exists(istate, pathname, len, ignore_case))
return NULL;

ALLOC_GROW(dir->entries, dir->nr+1, dir->alloc);
return dir->entries[dir->nr++] = dir_entry_new(pathname, len);
}

struct dir_entry *dir_add_ignored(struct dir_struct *dir,
struct index_state *istate,
const char *pathname, int len)
{
if (!index_name_is_other(istate, pathname, len))
return NULL;

ALLOC_GROW(dir->ignored, dir->ignored_nr+1, dir->ignored_alloc);
return dir->ignored[dir->ignored_nr++] = dir_entry_new(pathname, len);
}

enum exist_status {
index_nonexistent = 0,
index_directory,
index_gitdir
};






static enum exist_status directory_exists_in_index_icase(struct index_state *istate,
const char *dirname, int len)
{
struct cache_entry *ce;

if (index_dir_exists(istate, dirname, len))
return index_directory;

ce = index_file_exists(istate, dirname, len, ignore_case);
if (ce && S_ISGITLINK(ce->ce_mode))
return index_gitdir;

return index_nonexistent;
}








static enum exist_status directory_exists_in_index(struct index_state *istate,
const char *dirname, int len)
{
int pos;

if (ignore_case)
return directory_exists_in_index_icase(istate, dirname, len);

pos = index_name_pos(istate, dirname, len);
if (pos < 0)
pos = -pos-1;
while (pos < istate->cache_nr) {
const struct cache_entry *ce = istate->cache[pos++];
unsigned char endchar;

if (strncmp(ce->name, dirname, len))
break;
endchar = ce->name[len];
if (endchar > '/')
break;
if (endchar == '/')
return index_directory;
if (!endchar && S_ISGITLINK(ce->ce_mode))
return index_gitdir;
}
return index_nonexistent;
}


































static enum path_treatment treat_directory(struct dir_struct *dir,
struct index_state *istate,
struct untracked_cache_dir *untracked,
const char *dirname, int len, int baselen, int exclude,
const struct pathspec *pathspec)
{
int nested_repo = 0;


switch (directory_exists_in_index(istate, dirname, len-1)) {
case index_directory:
return path_recurse;

case index_gitdir:
return path_none;

case index_nonexistent:
if ((dir->flags & DIR_SKIP_NESTED_GIT) ||
!(dir->flags & DIR_NO_GITLINKS)) {
struct strbuf sb = STRBUF_INIT;
strbuf_addstr(&sb, dirname);
nested_repo = is_nonbare_repository_dir(&sb);
strbuf_release(&sb);
}
if (nested_repo)
return ((dir->flags & DIR_SKIP_NESTED_GIT) ? path_none :
(exclude ? path_excluded : path_untracked));

if (dir->flags & DIR_SHOW_OTHER_DIRECTORIES)
break;
if (exclude &&
(dir->flags & DIR_SHOW_IGNORED_TOO) &&
(dir->flags & DIR_SHOW_IGNORED_TOO_MODE_MATCHING)) {












if (!(dir->flags & DIR_HIDE_EMPTY_DIRECTORIES))
return path_excluded;

if (read_directory_recursive(dir, istate, dirname, len,
untracked, 1, 1, pathspec) == path_excluded)
return path_excluded;

return path_none;
}
return path_recurse;
}



if (!(dir->flags & DIR_HIDE_EMPTY_DIRECTORIES))
return exclude ? path_excluded : path_untracked;

untracked = lookup_untracked(dir->untracked, untracked,
dirname + baselen, len - baselen);





return read_directory_recursive(dir, istate, dirname, len,
untracked, 1, exclude, pathspec);
}






static int simplify_away(const char *path, int pathlen,
const struct pathspec *pathspec)
{
int i;

if (!pathspec || !pathspec->nr)
return 0;

GUARD_PATHSPEC(pathspec,
PATHSPEC_FROMTOP |
PATHSPEC_MAXDEPTH |
PATHSPEC_LITERAL |
PATHSPEC_GLOB |
PATHSPEC_ICASE |
PATHSPEC_EXCLUDE |
PATHSPEC_ATTR);

for (i = 0; i < pathspec->nr; i++) {
const struct pathspec_item *item = &pathspec->items[i];
int len = item->nowildcard_len;

if (len > pathlen)
len = pathlen;
if (!ps_strncmp(item, item->match, path, len))
return 0;
}

return 1;
}












static int exclude_matches_pathspec(const char *path, int pathlen,
const struct pathspec *pathspec)
{
int i;

if (!pathspec || !pathspec->nr)
return 0;

GUARD_PATHSPEC(pathspec,
PATHSPEC_FROMTOP |
PATHSPEC_MAXDEPTH |
PATHSPEC_LITERAL |
PATHSPEC_GLOB |
PATHSPEC_ICASE |
PATHSPEC_EXCLUDE);

for (i = 0; i < pathspec->nr; i++) {
const struct pathspec_item *item = &pathspec->items[i];
int len = item->nowildcard_len;

if (len == pathlen &&
!ps_strncmp(item, item->match, path, pathlen))
return 1;
if (len > pathlen &&
item->match[pathlen] == '/' &&
!ps_strncmp(item, item->match, path, pathlen))
return 1;
}
return 0;
}

static int get_index_dtype(struct index_state *istate,
const char *path, int len)
{
int pos;
const struct cache_entry *ce;

ce = index_file_exists(istate, path, len, 0);
if (ce) {
if (!ce_uptodate(ce))
return DT_UNKNOWN;
if (S_ISGITLINK(ce->ce_mode))
return DT_DIR;




return DT_REG;
}


pos = index_name_pos(istate, path, len);
if (pos >= 0)
return DT_UNKNOWN;
pos = -pos-1;
while (pos < istate->cache_nr) {
ce = istate->cache[pos++];
if (strncmp(ce->name, path, len))
break;
if (ce->name[len] > '/')
break;
if (ce->name[len] < '/')
continue;
if (!ce_uptodate(ce))
break; 
return DT_DIR;
}
return DT_UNKNOWN;
}

static int resolve_dtype(int dtype, struct index_state *istate,
const char *path, int len)
{
struct stat st;

if (dtype != DT_UNKNOWN)
return dtype;
dtype = get_index_dtype(istate, path, len);
if (dtype != DT_UNKNOWN)
return dtype;
if (lstat(path, &st))
return dtype;
if (S_ISREG(st.st_mode))
return DT_REG;
if (S_ISDIR(st.st_mode))
return DT_DIR;
if (S_ISLNK(st.st_mode))
return DT_LNK;
return dtype;
}

static enum path_treatment treat_one_path(struct dir_struct *dir,
struct untracked_cache_dir *untracked,
struct index_state *istate,
struct strbuf *path,
int baselen,
const struct pathspec *pathspec,
int dtype)
{
int exclude;
int has_path_in_index = !!index_file_exists(istate, path->buf, path->len, ignore_case);
enum path_treatment path_treatment;

dtype = resolve_dtype(dtype, istate, path->buf, path->len);


if (dtype != DT_DIR && has_path_in_index)
return path_none;



















if ((dir->flags & DIR_COLLECT_KILLED_ONLY) &&
(dtype == DT_DIR) &&
!has_path_in_index &&
(directory_exists_in_index(istate, path->buf, path->len) == index_nonexistent))
return path_none;

exclude = is_excluded(dir, istate, path->buf, &dtype);





if (exclude && !(dir->flags & (DIR_SHOW_IGNORED|DIR_SHOW_IGNORED_TOO)))
return path_excluded;

switch (dtype) {
default:
return path_none;
case DT_DIR:
strbuf_addch(path, '/');
path_treatment = treat_directory(dir, istate, untracked,
path->buf, path->len,
baselen, exclude, pathspec);








if (!exclude &&
path_treatment == path_excluded &&
(dir->flags & DIR_SHOW_IGNORED_TOO) &&
(dir->flags & DIR_SHOW_IGNORED_TOO_MODE_MATCHING))
return path_recurse;
return path_treatment;
case DT_REG:
case DT_LNK:
return exclude ? path_excluded : path_untracked;
}
}

static enum path_treatment treat_path_fast(struct dir_struct *dir,
struct untracked_cache_dir *untracked,
struct cached_dir *cdir,
struct index_state *istate,
struct strbuf *path,
int baselen,
const struct pathspec *pathspec)
{
strbuf_setlen(path, baselen);
if (!cdir->ucd) {
strbuf_addstr(path, cdir->file);
return path_untracked;
}
strbuf_addstr(path, cdir->ucd->name);

strbuf_complete(path, '/');
if (cdir->ucd->check_only)





return read_directory_recursive(dir, istate, path->buf, path->len,
cdir->ucd, 1, 0, pathspec);






return path_recurse;
}

static enum path_treatment treat_path(struct dir_struct *dir,
struct untracked_cache_dir *untracked,
struct cached_dir *cdir,
struct index_state *istate,
struct strbuf *path,
int baselen,
const struct pathspec *pathspec)
{
if (!cdir->d_name)
return treat_path_fast(dir, untracked, cdir, istate, path,
baselen, pathspec);
if (is_dot_or_dotdot(cdir->d_name) || !fspathcmp(cdir->d_name, ".git"))
return path_none;
strbuf_setlen(path, baselen);
strbuf_addstr(path, cdir->d_name);
if (simplify_away(path->buf, path->len, pathspec))
return path_none;

return treat_one_path(dir, untracked, istate, path, baselen, pathspec,
cdir->d_type);
}

static void add_untracked(struct untracked_cache_dir *dir, const char *name)
{
if (!dir)
return;
ALLOC_GROW(dir->untracked, dir->untracked_nr + 1,
dir->untracked_alloc);
dir->untracked[dir->untracked_nr++] = xstrdup(name);
}

static int valid_cached_dir(struct dir_struct *dir,
struct untracked_cache_dir *untracked,
struct index_state *istate,
struct strbuf *path,
int check_only)
{
struct stat st;

if (!untracked)
return 0;




refresh_fsmonitor(istate);
if (!(dir->untracked->use_fsmonitor && untracked->valid)) {
if (lstat(path->len ? path->buf : ".", &st)) {
memset(&untracked->stat_data, 0, sizeof(untracked->stat_data));
return 0;
}
if (!untracked->valid ||
match_stat_data_racy(istate, &untracked->stat_data, &st)) {
fill_stat_data(&untracked->stat_data, &st);
return 0;
}
}

if (untracked->check_only != !!check_only)
return 0;








if (path->len && path->buf[path->len - 1] != '/') {
strbuf_addch(path, '/');
prep_exclude(dir, istate, path->buf, path->len);
strbuf_setlen(path, path->len - 1);
} else
prep_exclude(dir, istate, path->buf, path->len);


return untracked->valid;
}

static int open_cached_dir(struct cached_dir *cdir,
struct dir_struct *dir,
struct untracked_cache_dir *untracked,
struct index_state *istate,
struct strbuf *path,
int check_only)
{
const char *c_path;

memset(cdir, 0, sizeof(*cdir));
cdir->untracked = untracked;
if (valid_cached_dir(dir, untracked, istate, path, check_only))
return 0;
c_path = path->len ? path->buf : ".";
cdir->fdir = opendir(c_path);
if (!cdir->fdir)
warning_errno(_("could not open directory '%s'"), c_path);
if (dir->untracked) {
invalidate_directory(dir->untracked, untracked);
dir->untracked->dir_opened++;
}
if (!cdir->fdir)
return -1;
return 0;
}

static int read_cached_dir(struct cached_dir *cdir)
{
struct dirent *de;

if (cdir->fdir) {
de = readdir(cdir->fdir);
if (!de) {
cdir->d_name = NULL;
cdir->d_type = DT_UNKNOWN;
return -1;
}
cdir->d_name = de->d_name;
cdir->d_type = DTYPE(de);
return 0;
}
while (cdir->nr_dirs < cdir->untracked->dirs_nr) {
struct untracked_cache_dir *d = cdir->untracked->dirs[cdir->nr_dirs];
if (!d->recurse) {
cdir->nr_dirs++;
continue;
}
cdir->ucd = d;
cdir->nr_dirs++;
return 0;
}
cdir->ucd = NULL;
if (cdir->nr_files < cdir->untracked->untracked_nr) {
struct untracked_cache_dir *d = cdir->untracked;
cdir->file = d->untracked[cdir->nr_files++];
return 0;
}
return -1;
}

static void close_cached_dir(struct cached_dir *cdir)
{
if (cdir->fdir)
closedir(cdir->fdir);




if (cdir->untracked) {
cdir->untracked->valid = 1;
cdir->untracked->recurse = 1;
}
}

static void add_path_to_appropriate_result_list(struct dir_struct *dir,
struct untracked_cache_dir *untracked,
struct cached_dir *cdir,
struct index_state *istate,
struct strbuf *path,
int baselen,
const struct pathspec *pathspec,
enum path_treatment state)
{

switch (state) {
case path_excluded:
if (dir->flags & DIR_SHOW_IGNORED)
dir_add_name(dir, istate, path->buf, path->len);
else if ((dir->flags & DIR_SHOW_IGNORED_TOO) ||
((dir->flags & DIR_COLLECT_IGNORED) &&
exclude_matches_pathspec(path->buf, path->len,
pathspec)))
dir_add_ignored(dir, istate, path->buf, path->len);
break;

case path_untracked:
if (dir->flags & DIR_SHOW_IGNORED)
break;
dir_add_name(dir, istate, path->buf, path->len);
if (cdir->fdir)
add_untracked(untracked, path->buf + baselen);
break;

default:
break;
}
}




















static enum path_treatment read_directory_recursive(struct dir_struct *dir,
struct index_state *istate, const char *base, int baselen,
struct untracked_cache_dir *untracked, int check_only,
int stop_at_first_file, const struct pathspec *pathspec)
{









struct cached_dir cdir;
enum path_treatment state, subdir_state, dir_state = path_none;
struct strbuf path = STRBUF_INIT;

strbuf_add(&path, base, baselen);

if (open_cached_dir(&cdir, dir, untracked, istate, &path, check_only))
goto out;

if (untracked)
untracked->check_only = !!check_only;

while (!read_cached_dir(&cdir)) {

state = treat_path(dir, untracked, &cdir, istate, &path,
baselen, pathspec);

if (state > dir_state)
dir_state = state;


if ((state == path_recurse) ||
((state == path_untracked) &&
(resolve_dtype(cdir.d_type, istate, path.buf, path.len) == DT_DIR) &&
((dir->flags & DIR_SHOW_IGNORED_TOO) ||
(pathspec &&
do_match_pathspec(istate, pathspec, path.buf, path.len,
baselen, NULL, DO_MATCH_LEADING_PATHSPEC) == MATCHED_RECURSIVELY_LEADING_PATHSPEC)))) {
struct untracked_cache_dir *ud;
ud = lookup_untracked(dir->untracked, untracked,
path.buf + baselen,
path.len - baselen);
subdir_state =
read_directory_recursive(dir, istate, path.buf,
path.len, ud,
check_only, stop_at_first_file, pathspec);
if (subdir_state > dir_state)
dir_state = subdir_state;

if (pathspec &&
!match_pathspec(istate, pathspec, path.buf, path.len,
0 , NULL,
0 ))
state = path_none;
}

if (check_only) {
if (stop_at_first_file) {















if (dir_state >= path_excluded) {
dir_state = path_excluded;
break;
}
}


if (dir_state == path_untracked) {
if (cdir.fdir)
add_untracked(untracked, path.buf + baselen);
break;
}

continue;
}

add_path_to_appropriate_result_list(dir, untracked, &cdir,
istate, &path, baselen,
pathspec, state);
}
close_cached_dir(&cdir);
out:
strbuf_release(&path);

return dir_state;
}

int cmp_dir_entry(const void *p1, const void *p2)
{
const struct dir_entry *e1 = *(const struct dir_entry **)p1;
const struct dir_entry *e2 = *(const struct dir_entry **)p2;

return name_compare(e1->name, e1->len, e2->name, e2->len);
}


int check_dir_entry_contains(const struct dir_entry *out, const struct dir_entry *in)
{
return (out->len < in->len) &&
(out->name[out->len - 1] == '/') &&
!memcmp(out->name, in->name, out->len);
}

static int treat_leading_path(struct dir_struct *dir,
struct index_state *istate,
const char *path, int len,
const struct pathspec *pathspec)
{









struct strbuf sb = STRBUF_INIT;
struct strbuf subdir = STRBUF_INIT;
int prevlen, baselen;
const char *cp;
struct cached_dir cdir;
enum path_treatment state = path_none;


















while (len && path[len - 1] == '/')
len--;
if (!len)
return 1;

memset(&cdir, 0, sizeof(cdir));
cdir.d_type = DT_DIR;
baselen = 0;
prevlen = 0;
while (1) {
prevlen = baselen + !!baselen;
cp = path + prevlen;
cp = memchr(cp, '/', path + len - cp);
if (!cp)
baselen = len;
else
baselen = cp - path;
strbuf_reset(&sb);
strbuf_add(&sb, path, baselen);
if (!is_directory(sb.buf))
break;
strbuf_reset(&sb);
strbuf_add(&sb, path, prevlen);
strbuf_reset(&subdir);
strbuf_add(&subdir, path+prevlen, baselen-prevlen);
cdir.d_name = subdir.buf;
state = treat_path(dir, NULL, &cdir, istate, &sb, prevlen,
pathspec);
if (state == path_untracked &&
resolve_dtype(cdir.d_type, istate, sb.buf, sb.len) == DT_DIR &&
(dir->flags & DIR_SHOW_IGNORED_TOO ||
do_match_pathspec(istate, pathspec, sb.buf, sb.len,
baselen, NULL, DO_MATCH_LEADING_PATHSPEC) == MATCHED_RECURSIVELY_LEADING_PATHSPEC)) {
if (!match_pathspec(istate, pathspec, sb.buf, sb.len,
0 , NULL,
0 ))
state = path_none;
add_path_to_appropriate_result_list(dir, NULL, &cdir,
istate,
&sb, baselen,
pathspec, state);
state = path_recurse;
}

if (state != path_recurse)
break; 
if (len <= baselen)
break; 
}
add_path_to_appropriate_result_list(dir, NULL, &cdir, istate,
&sb, baselen, pathspec,
state);

strbuf_release(&subdir);
strbuf_release(&sb);
return state == path_recurse;
}

static const char *get_ident_string(void)
{
static struct strbuf sb = STRBUF_INIT;
struct utsname uts;

if (sb.len)
return sb.buf;
if (uname(&uts) < 0)
die_errno(_("failed to get kernel name and information"));
strbuf_addf(&sb, "Location %s, system %s", get_git_work_tree(),
uts.sysname);
return sb.buf;
}

static int ident_in_untracked(const struct untracked_cache *uc)
{






return !strcmp(uc->ident.buf, get_ident_string());
}

static void set_untracked_ident(struct untracked_cache *uc)
{
strbuf_reset(&uc->ident);
strbuf_addstr(&uc->ident, get_ident_string());





strbuf_addch(&uc->ident, 0);
}

static void new_untracked_cache(struct index_state *istate)
{
struct untracked_cache *uc = xcalloc(1, sizeof(*uc));
strbuf_init(&uc->ident, 100);
uc->exclude_per_dir = ".gitignore";

uc->dir_flags = DIR_SHOW_OTHER_DIRECTORIES | DIR_HIDE_EMPTY_DIRECTORIES;
set_untracked_ident(uc);
istate->untracked = uc;
istate->cache_changed |= UNTRACKED_CHANGED;
}

void add_untracked_cache(struct index_state *istate)
{
if (!istate->untracked) {
new_untracked_cache(istate);
} else {
if (!ident_in_untracked(istate->untracked)) {
free_untracked_cache(istate->untracked);
new_untracked_cache(istate);
}
}
}

void remove_untracked_cache(struct index_state *istate)
{
if (istate->untracked) {
free_untracked_cache(istate->untracked);
istate->untracked = NULL;
istate->cache_changed |= UNTRACKED_CHANGED;
}
}

static struct untracked_cache_dir *validate_untracked_cache(struct dir_struct *dir,
int base_len,
const struct pathspec *pathspec)
{
struct untracked_cache_dir *root;
static int untracked_cache_disabled = -1;

if (!dir->untracked)
return NULL;
if (untracked_cache_disabled < 0)
untracked_cache_disabled = git_env_bool("GIT_DISABLE_UNTRACKED_CACHE", 0);
if (untracked_cache_disabled)
return NULL;








if (dir->unmanaged_exclude_files)
return NULL;







if (base_len || (pathspec && pathspec->nr))
return NULL;


if (dir->flags != dir->untracked->dir_flags ||





!(dir->flags & DIR_SHOW_OTHER_DIRECTORIES) ||

(dir->flags & (DIR_SHOW_IGNORED | DIR_SHOW_IGNORED_TOO |
DIR_COLLECT_IGNORED)))
return NULL;





if (dir->exclude_per_dir != dir->untracked->exclude_per_dir &&
strcmp(dir->exclude_per_dir, dir->untracked->exclude_per_dir))
return NULL;





if (dir->exclude_list_group[EXC_CMDL].nr)
return NULL;

if (!ident_in_untracked(dir->untracked)) {
warning(_("untracked cache is disabled on this system or location"));
return NULL;
}

if (!dir->untracked->root) {
const int len = sizeof(*dir->untracked->root);
dir->untracked->root = xmalloc(len);
memset(dir->untracked->root, 0, len);
}


root = dir->untracked->root;
if (!oideq(&dir->ss_info_exclude.oid,
&dir->untracked->ss_info_exclude.oid)) {
invalidate_gitignore(dir->untracked, root);
dir->untracked->ss_info_exclude = dir->ss_info_exclude;
}
if (!oideq(&dir->ss_excludes_file.oid,
&dir->untracked->ss_excludes_file.oid)) {
invalidate_gitignore(dir->untracked, root);
dir->untracked->ss_excludes_file = dir->ss_excludes_file;
}


root->recurse = 1;
return root;
}

int read_directory(struct dir_struct *dir, struct index_state *istate,
const char *path, int len, const struct pathspec *pathspec)
{
struct untracked_cache_dir *untracked;

trace_performance_enter();

if (has_symlink_leading_path(path, len)) {
trace_performance_leave("read directory %.*s", len, path);
return dir->nr;
}

untracked = validate_untracked_cache(dir, len, pathspec);
if (!untracked)




dir->untracked = NULL;
if (!len || treat_leading_path(dir, istate, path, len, pathspec))
read_directory_recursive(dir, istate, path, len, untracked, 0, 0, pathspec);
QSORT(dir->entries, dir->nr, cmp_dir_entry);
QSORT(dir->ignored, dir->ignored_nr, cmp_dir_entry);






if ((dir->flags & DIR_SHOW_IGNORED_TOO) &&
!(dir->flags & DIR_KEEP_UNTRACKED_CONTENTS)) {
int i, j;


for (i = j = 0; j < dir->nr; j++) {
if (i &&
check_dir_entry_contains(dir->entries[i - 1], dir->entries[j])) {
FREE_AND_NULL(dir->entries[j]);
} else {
dir->entries[i++] = dir->entries[j];
}
}

dir->nr = i;
}

trace_performance_leave("read directory %.*s", len, path);
if (dir->untracked) {
static int force_untracked_cache = -1;
static struct trace_key trace_untracked_stats = TRACE_KEY_INIT(UNTRACKED_STATS);

if (force_untracked_cache < 0)
force_untracked_cache =
git_env_bool("GIT_FORCE_UNTRACKED_CACHE", 0);
trace_printf_key(&trace_untracked_stats,
"node creation: %u\n"
"gitignore invalidation: %u\n"
"directory invalidation: %u\n"
"opendir: %u\n",
dir->untracked->dir_created,
dir->untracked->gitignore_invalidated,
dir->untracked->dir_invalidated,
dir->untracked->dir_opened);
if (force_untracked_cache &&
dir->untracked == istate->untracked &&
(dir->untracked->dir_opened ||
dir->untracked->gitignore_invalidated ||
dir->untracked->dir_invalidated))
istate->cache_changed |= UNTRACKED_CHANGED;
if (dir->untracked != istate->untracked) {
FREE_AND_NULL(dir->untracked);
}
}
return dir->nr;
}

int file_exists(const char *f)
{
struct stat sb;
return lstat(f, &sb) == 0;
}

int repo_file_exists(struct repository *repo, const char *path)
{
if (repo != the_repository)
BUG("do not know how to check file existence in arbitrary repo");

return file_exists(path);
}

static int cmp_icase(char a, char b)
{
if (a == b)
return 0;
if (ignore_case)
return toupper(a) - toupper(b);
return a - b;
}






int dir_inside_of(const char *subdir, const char *dir)
{
int offset = 0;

assert(dir && subdir && *dir && *subdir);

while (*dir && *subdir && !cmp_icase(*dir, *subdir)) {
dir++;
subdir++;
offset++;
}


if (*dir && *subdir)
return -1;

if (!*subdir)
return !*dir ? offset : -1; 


if (is_dir_sep(dir[-1]))
return is_dir_sep(subdir[-1]) ? offset : -1;


return is_dir_sep(*subdir) ? offset + 1 : -1;
}

int is_inside_dir(const char *dir)
{
char *cwd;
int rc;

if (!dir)
return 0;

cwd = xgetcwd();
rc = (dir_inside_of(cwd, dir) >= 0);
free(cwd);
return rc;
}

int is_empty_dir(const char *path)
{
DIR *dir = opendir(path);
struct dirent *e;
int ret = 1;

if (!dir)
return 0;

while ((e = readdir(dir)) != NULL)
if (!is_dot_or_dotdot(e->d_name)) {
ret = 0;
break;
}

closedir(dir);
return ret;
}

static int remove_dir_recurse(struct strbuf *path, int flag, int *kept_up)
{
DIR *dir;
struct dirent *e;
int ret = 0, original_len = path->len, len, kept_down = 0;
int only_empty = (flag & REMOVE_DIR_EMPTY_ONLY);
int keep_toplevel = (flag & REMOVE_DIR_KEEP_TOPLEVEL);
struct object_id submodule_head;

if ((flag & REMOVE_DIR_KEEP_NESTED_GIT) &&
!resolve_gitlink_ref(path->buf, "HEAD", &submodule_head)) {

if (kept_up)
*kept_up = 1;
return 0;
}

flag &= ~REMOVE_DIR_KEEP_TOPLEVEL;
dir = opendir(path->buf);
if (!dir) {
if (errno == ENOENT)
return keep_toplevel ? -1 : 0;
else if (errno == EACCES && !keep_toplevel)




return rmdir(path->buf);
else
return -1;
}
strbuf_complete(path, '/');

len = path->len;
while ((e = readdir(dir)) != NULL) {
struct stat st;
if (is_dot_or_dotdot(e->d_name))
continue;

strbuf_setlen(path, len);
strbuf_addstr(path, e->d_name);
if (lstat(path->buf, &st)) {
if (errno == ENOENT)




continue;

} else if (S_ISDIR(st.st_mode)) {
if (!remove_dir_recurse(path, flag, &kept_down))
continue; 
} else if (!only_empty &&
(!unlink(path->buf) || errno == ENOENT)) {
continue; 
}


ret = -1;
break;
}
closedir(dir);

strbuf_setlen(path, original_len);
if (!ret && !keep_toplevel && !kept_down)
ret = (!rmdir(path->buf) || errno == ENOENT) ? 0 : -1;
else if (kept_up)




*kept_up = !ret;
return ret;
}

int remove_dir_recursively(struct strbuf *path, int flag)
{
return remove_dir_recurse(path, flag, NULL);
}

static GIT_PATH_FUNC(git_path_info_exclude, "info/exclude")

void setup_standard_excludes(struct dir_struct *dir)
{
dir->exclude_per_dir = ".gitignore";


if (!excludes_file)
excludes_file = xdg_config_home("ignore");
if (excludes_file && !access_or_warn(excludes_file, R_OK, 0))
add_patterns_from_file_1(dir, excludes_file,
dir->untracked ? &dir->ss_excludes_file : NULL);


if (startup_info->have_repository) {
const char *path = git_path_info_exclude();
if (!access_or_warn(path, R_OK, 0))
add_patterns_from_file_1(dir, path,
dir->untracked ? &dir->ss_info_exclude : NULL);
}
}

int remove_path(const char *name)
{
char *slash;

if (unlink(name) && !is_missing_file_error(errno))
return -1;

slash = strrchr(name, '/');
if (slash) {
char *dirs = xstrdup(name);
slash = dirs + (slash - name);
do {
*slash = '\0';
} while (rmdir(dirs) == 0 && (slash = strrchr(dirs, '/')));
free(dirs);
}
return 0;
}





void clear_directory(struct dir_struct *dir)
{
int i, j;
struct exclude_list_group *group;
struct pattern_list *pl;
struct exclude_stack *stk;

for (i = EXC_CMDL; i <= EXC_FILE; i++) {
group = &dir->exclude_list_group[i];
for (j = 0; j < group->nr; j++) {
pl = &group->pl[j];
if (i == EXC_DIRS)
free((char *)pl->src);
clear_pattern_list(pl);
}
free(group->pl);
}

stk = dir->exclude_stack;
while (stk) {
struct exclude_stack *prev = stk->prev;
free(stk);
stk = prev;
}
strbuf_release(&dir->basebuf);
}

struct ondisk_untracked_cache {
struct stat_data info_exclude_stat;
struct stat_data excludes_file_stat;
uint32_t dir_flags;
};

#define ouc_offset(x) offsetof(struct ondisk_untracked_cache, x)

struct write_data {
int index; 
struct ewah_bitmap *check_only; 
struct ewah_bitmap *valid; 
struct ewah_bitmap *sha1_valid; 
struct strbuf out;
struct strbuf sb_stat;
struct strbuf sb_sha1;
};

static void stat_data_to_disk(struct stat_data *to, const struct stat_data *from)
{
to->sd_ctime.sec = htonl(from->sd_ctime.sec);
to->sd_ctime.nsec = htonl(from->sd_ctime.nsec);
to->sd_mtime.sec = htonl(from->sd_mtime.sec);
to->sd_mtime.nsec = htonl(from->sd_mtime.nsec);
to->sd_dev = htonl(from->sd_dev);
to->sd_ino = htonl(from->sd_ino);
to->sd_uid = htonl(from->sd_uid);
to->sd_gid = htonl(from->sd_gid);
to->sd_size = htonl(from->sd_size);
}

static void write_one_dir(struct untracked_cache_dir *untracked,
struct write_data *wd)
{
struct stat_data stat_data;
struct strbuf *out = &wd->out;
unsigned char intbuf[16];
unsigned int intlen, value;
int i = wd->index++;





if (!untracked->valid) {
untracked->untracked_nr = 0;
untracked->check_only = 0;
}

if (untracked->check_only)
ewah_set(wd->check_only, i);
if (untracked->valid) {
ewah_set(wd->valid, i);
stat_data_to_disk(&stat_data, &untracked->stat_data);
strbuf_add(&wd->sb_stat, &stat_data, sizeof(stat_data));
}
if (!is_null_oid(&untracked->exclude_oid)) {
ewah_set(wd->sha1_valid, i);
strbuf_add(&wd->sb_sha1, untracked->exclude_oid.hash,
the_hash_algo->rawsz);
}

intlen = encode_varint(untracked->untracked_nr, intbuf);
strbuf_add(out, intbuf, intlen);


for (i = 0, value = 0; i < untracked->dirs_nr; i++)
if (untracked->dirs[i]->recurse)
value++;
intlen = encode_varint(value, intbuf);
strbuf_add(out, intbuf, intlen);

strbuf_add(out, untracked->name, strlen(untracked->name) + 1);

for (i = 0; i < untracked->untracked_nr; i++)
strbuf_add(out, untracked->untracked[i],
strlen(untracked->untracked[i]) + 1);

for (i = 0; i < untracked->dirs_nr; i++)
if (untracked->dirs[i]->recurse)
write_one_dir(untracked->dirs[i], wd);
}

void write_untracked_extension(struct strbuf *out, struct untracked_cache *untracked)
{
struct ondisk_untracked_cache *ouc;
struct write_data wd;
unsigned char varbuf[16];
int varint_len;
const unsigned hashsz = the_hash_algo->rawsz;

ouc = xcalloc(1, sizeof(*ouc));
stat_data_to_disk(&ouc->info_exclude_stat, &untracked->ss_info_exclude.stat);
stat_data_to_disk(&ouc->excludes_file_stat, &untracked->ss_excludes_file.stat);
ouc->dir_flags = htonl(untracked->dir_flags);

varint_len = encode_varint(untracked->ident.len, varbuf);
strbuf_add(out, varbuf, varint_len);
strbuf_addbuf(out, &untracked->ident);

strbuf_add(out, ouc, sizeof(*ouc));
strbuf_add(out, untracked->ss_info_exclude.oid.hash, hashsz);
strbuf_add(out, untracked->ss_excludes_file.oid.hash, hashsz);
strbuf_add(out, untracked->exclude_per_dir, strlen(untracked->exclude_per_dir) + 1);
FREE_AND_NULL(ouc);

if (!untracked->root) {
varint_len = encode_varint(0, varbuf);
strbuf_add(out, varbuf, varint_len);
return;
}

wd.index = 0;
wd.check_only = ewah_new();
wd.valid = ewah_new();
wd.sha1_valid = ewah_new();
strbuf_init(&wd.out, 1024);
strbuf_init(&wd.sb_stat, 1024);
strbuf_init(&wd.sb_sha1, 1024);
write_one_dir(untracked->root, &wd);

varint_len = encode_varint(wd.index, varbuf);
strbuf_add(out, varbuf, varint_len);
strbuf_addbuf(out, &wd.out);
ewah_serialize_strbuf(wd.valid, out);
ewah_serialize_strbuf(wd.check_only, out);
ewah_serialize_strbuf(wd.sha1_valid, out);
strbuf_addbuf(out, &wd.sb_stat);
strbuf_addbuf(out, &wd.sb_sha1);
strbuf_addch(out, '\0'); 

ewah_free(wd.valid);
ewah_free(wd.check_only);
ewah_free(wd.sha1_valid);
strbuf_release(&wd.out);
strbuf_release(&wd.sb_stat);
strbuf_release(&wd.sb_sha1);
}

static void free_untracked(struct untracked_cache_dir *ucd)
{
int i;
if (!ucd)
return;
for (i = 0; i < ucd->dirs_nr; i++)
free_untracked(ucd->dirs[i]);
for (i = 0; i < ucd->untracked_nr; i++)
free(ucd->untracked[i]);
free(ucd->untracked);
free(ucd->dirs);
free(ucd);
}

void free_untracked_cache(struct untracked_cache *uc)
{
if (uc)
free_untracked(uc->root);
free(uc);
}

struct read_data {
int index;
struct untracked_cache_dir **ucd;
struct ewah_bitmap *check_only;
struct ewah_bitmap *valid;
struct ewah_bitmap *sha1_valid;
const unsigned char *data;
const unsigned char *end;
};

static void stat_data_from_disk(struct stat_data *to, const unsigned char *data)
{
memcpy(to, data, sizeof(*to));
to->sd_ctime.sec = ntohl(to->sd_ctime.sec);
to->sd_ctime.nsec = ntohl(to->sd_ctime.nsec);
to->sd_mtime.sec = ntohl(to->sd_mtime.sec);
to->sd_mtime.nsec = ntohl(to->sd_mtime.nsec);
to->sd_dev = ntohl(to->sd_dev);
to->sd_ino = ntohl(to->sd_ino);
to->sd_uid = ntohl(to->sd_uid);
to->sd_gid = ntohl(to->sd_gid);
to->sd_size = ntohl(to->sd_size);
}

static int read_one_dir(struct untracked_cache_dir **untracked_,
struct read_data *rd)
{
struct untracked_cache_dir ud, *untracked;
const unsigned char *data = rd->data, *end = rd->end;
const unsigned char *eos;
unsigned int value;
int i;

memset(&ud, 0, sizeof(ud));

value = decode_varint(&data);
if (data > end)
return -1;
ud.recurse = 1;
ud.untracked_alloc = value;
ud.untracked_nr = value;
if (ud.untracked_nr)
ALLOC_ARRAY(ud.untracked, ud.untracked_nr);

ud.dirs_alloc = ud.dirs_nr = decode_varint(&data);
if (data > end)
return -1;
ALLOC_ARRAY(ud.dirs, ud.dirs_nr);

eos = memchr(data, '\0', end - data);
if (!eos || eos == end)
return -1;

*untracked_ = untracked = xmalloc(st_add3(sizeof(*untracked), eos - data, 1));
memcpy(untracked, &ud, sizeof(ud));
memcpy(untracked->name, data, eos - data + 1);
data = eos + 1;

for (i = 0; i < untracked->untracked_nr; i++) {
eos = memchr(data, '\0', end - data);
if (!eos || eos == end)
return -1;
untracked->untracked[i] = xmemdupz(data, eos - data);
data = eos + 1;
}

rd->ucd[rd->index++] = untracked;
rd->data = data;

for (i = 0; i < untracked->dirs_nr; i++) {
if (read_one_dir(untracked->dirs + i, rd) < 0)
return -1;
}
return 0;
}

static void set_check_only(size_t pos, void *cb)
{
struct read_data *rd = cb;
struct untracked_cache_dir *ud = rd->ucd[pos];
ud->check_only = 1;
}

static void read_stat(size_t pos, void *cb)
{
struct read_data *rd = cb;
struct untracked_cache_dir *ud = rd->ucd[pos];
if (rd->data + sizeof(struct stat_data) > rd->end) {
rd->data = rd->end + 1;
return;
}
stat_data_from_disk(&ud->stat_data, rd->data);
rd->data += sizeof(struct stat_data);
ud->valid = 1;
}

static void read_oid(size_t pos, void *cb)
{
struct read_data *rd = cb;
struct untracked_cache_dir *ud = rd->ucd[pos];
if (rd->data + the_hash_algo->rawsz > rd->end) {
rd->data = rd->end + 1;
return;
}
hashcpy(ud->exclude_oid.hash, rd->data);
rd->data += the_hash_algo->rawsz;
}

static void load_oid_stat(struct oid_stat *oid_stat, const unsigned char *data,
const unsigned char *sha1)
{
stat_data_from_disk(&oid_stat->stat, data);
hashcpy(oid_stat->oid.hash, sha1);
oid_stat->valid = 1;
}

struct untracked_cache *read_untracked_extension(const void *data, unsigned long sz)
{
struct untracked_cache *uc;
struct read_data rd;
const unsigned char *next = data, *end = (const unsigned char *)data + sz;
const char *ident;
int ident_len;
ssize_t len;
const char *exclude_per_dir;
const unsigned hashsz = the_hash_algo->rawsz;
const unsigned offset = sizeof(struct ondisk_untracked_cache);
const unsigned exclude_per_dir_offset = offset + 2 * hashsz;

if (sz <= 1 || end[-1] != '\0')
return NULL;
end--;

ident_len = decode_varint(&next);
if (next + ident_len > end)
return NULL;
ident = (const char *)next;
next += ident_len;

if (next + exclude_per_dir_offset + 1 > end)
return NULL;

uc = xcalloc(1, sizeof(*uc));
strbuf_init(&uc->ident, ident_len);
strbuf_add(&uc->ident, ident, ident_len);
load_oid_stat(&uc->ss_info_exclude,
next + ouc_offset(info_exclude_stat),
next + offset);
load_oid_stat(&uc->ss_excludes_file,
next + ouc_offset(excludes_file_stat),
next + offset + hashsz);
uc->dir_flags = get_be32(next + ouc_offset(dir_flags));
exclude_per_dir = (const char *)next + exclude_per_dir_offset;
uc->exclude_per_dir = xstrdup(exclude_per_dir);

next += exclude_per_dir_offset + strlen(exclude_per_dir) + 1;
if (next >= end)
goto done2;

len = decode_varint(&next);
if (next > end || len == 0)
goto done2;

rd.valid = ewah_new();
rd.check_only = ewah_new();
rd.sha1_valid = ewah_new();
rd.data = next;
rd.end = end;
rd.index = 0;
ALLOC_ARRAY(rd.ucd, len);

if (read_one_dir(&uc->root, &rd) || rd.index != len)
goto done;

next = rd.data;
len = ewah_read_mmap(rd.valid, next, end - next);
if (len < 0)
goto done;

next += len;
len = ewah_read_mmap(rd.check_only, next, end - next);
if (len < 0)
goto done;

next += len;
len = ewah_read_mmap(rd.sha1_valid, next, end - next);
if (len < 0)
goto done;

ewah_each_bit(rd.check_only, set_check_only, &rd);
rd.data = next + len;
ewah_each_bit(rd.valid, read_stat, &rd);
ewah_each_bit(rd.sha1_valid, read_oid, &rd);
next = rd.data;

done:
free(rd.ucd);
ewah_free(rd.valid);
ewah_free(rd.check_only);
ewah_free(rd.sha1_valid);
done2:
if (next != end) {
free_untracked_cache(uc);
uc = NULL;
}
return uc;
}

static void invalidate_one_directory(struct untracked_cache *uc,
struct untracked_cache_dir *ucd)
{
uc->dir_invalidated++;
ucd->valid = 0;
ucd->untracked_nr = 0;
}

























static int invalidate_one_component(struct untracked_cache *uc,
struct untracked_cache_dir *dir,
const char *path, int len)
{
const char *rest = strchr(path, '/');

if (rest) {
int component_len = rest - path;
struct untracked_cache_dir *d =
lookup_untracked(uc, dir, path, component_len);
int ret =
invalidate_one_component(uc, d, rest + 1,
len - (component_len + 1));
if (ret)
invalidate_one_directory(uc, dir);
return ret;
}

invalidate_one_directory(uc, dir);
return uc->dir_flags & DIR_SHOW_OTHER_DIRECTORIES;
}

void untracked_cache_invalidate_path(struct index_state *istate,
const char *path, int safe_path)
{
if (!istate->untracked || !istate->untracked->root)
return;
if (!safe_path && !verify_path(path, 0))
return;
invalidate_one_component(istate->untracked, istate->untracked->root,
path, strlen(path));
}

void untracked_cache_remove_from_index(struct index_state *istate,
const char *path)
{
untracked_cache_invalidate_path(istate, path, 1);
}

void untracked_cache_add_to_index(struct index_state *istate,
const char *path)
{
untracked_cache_invalidate_path(istate, path, 1);
}

static void connect_wt_gitdir_in_nested(const char *sub_worktree,
const char *sub_gitdir)
{
int i;
struct repository subrepo;
struct strbuf sub_wt = STRBUF_INIT;
struct strbuf sub_gd = STRBUF_INIT;

const struct submodule *sub;


if (repo_init(&subrepo, sub_gitdir, sub_worktree))
return;

if (repo_read_index(&subrepo) < 0)
die(_("index file corrupt in repo %s"), subrepo.gitdir);

for (i = 0; i < subrepo.index->cache_nr; i++) {
const struct cache_entry *ce = subrepo.index->cache[i];

if (!S_ISGITLINK(ce->ce_mode))
continue;

while (i + 1 < subrepo.index->cache_nr &&
!strcmp(ce->name, subrepo.index->cache[i + 1]->name))




i++;

sub = submodule_from_path(&subrepo, &null_oid, ce->name);
if (!sub || !is_submodule_active(&subrepo, ce->name))

continue;

strbuf_reset(&sub_wt);
strbuf_reset(&sub_gd);
strbuf_addf(&sub_wt, "%s/%s", sub_worktree, sub->path);
strbuf_addf(&sub_gd, "%s/modules/%s", sub_gitdir, sub->name);

connect_work_tree_and_git_dir(sub_wt.buf, sub_gd.buf, 1);
}
strbuf_release(&sub_wt);
strbuf_release(&sub_gd);
repo_clear(&subrepo);
}

void connect_work_tree_and_git_dir(const char *work_tree_,
const char *git_dir_,
int recurse_into_nested)
{
struct strbuf gitfile_sb = STRBUF_INIT;
struct strbuf cfg_sb = STRBUF_INIT;
struct strbuf rel_path = STRBUF_INIT;
char *git_dir, *work_tree;


strbuf_addf(&gitfile_sb, "%s/.git", work_tree_);
if (safe_create_leading_directories_const(gitfile_sb.buf))
die(_("could not create directories for %s"), gitfile_sb.buf);


strbuf_addf(&cfg_sb, "%s/config", git_dir_);
if (safe_create_leading_directories_const(cfg_sb.buf))
die(_("could not create directories for %s"), cfg_sb.buf);

git_dir = real_pathdup(git_dir_, 1);
work_tree = real_pathdup(work_tree_, 1);


write_file(gitfile_sb.buf, "gitdir: %s",
relative_path(git_dir, work_tree, &rel_path));

git_config_set_in_file(cfg_sb.buf, "core.worktree",
relative_path(work_tree, git_dir, &rel_path));

strbuf_release(&gitfile_sb);
strbuf_release(&cfg_sb);
strbuf_release(&rel_path);

if (recurse_into_nested)
connect_wt_gitdir_in_nested(work_tree, git_dir);

free(work_tree);
free(git_dir);
}




void relocate_gitdir(const char *path, const char *old_git_dir, const char *new_git_dir)
{
if (rename(old_git_dir, new_git_dir) < 0)
die_errno(_("could not migrate git directory from '%s' to '%s'"),
old_git_dir, new_git_dir);

connect_work_tree_and_git_dir(path, new_git_dir, 0);
}
