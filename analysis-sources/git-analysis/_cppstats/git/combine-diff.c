#include "cache.h"
#include "object-store.h"
#include "commit.h"
#include "blob.h"
#include "diff.h"
#include "diffcore.h"
#include "quote.h"
#include "xdiff-interface.h"
#include "xdiff/xmacros.h"
#include "log-tree.h"
#include "refs.h"
#include "userdiff.h"
#include "sha1-array.h"
#include "revision.h"
static int compare_paths(const struct combine_diff_path *one,
const struct diff_filespec *two)
{
if (!S_ISDIR(one->mode) && !S_ISDIR(two->mode))
return strcmp(one->path, two->path);
return base_name_compare(one->path, strlen(one->path), one->mode,
two->path, strlen(two->path), two->mode);
}
static int filename_changed(char status)
{
return status == 'R' || status == 'C';
}
static struct combine_diff_path *intersect_paths(
struct combine_diff_path *curr,
int n,
int num_parent,
int combined_all_paths)
{
struct diff_queue_struct *q = &diff_queued_diff;
struct combine_diff_path *p, **tail = &curr;
int i, j, cmp;
if (!n) {
for (i = 0; i < q->nr; i++) {
int len;
const char *path;
if (diff_unmodified_pair(q->queue[i]))
continue;
path = q->queue[i]->two->path;
len = strlen(path);
p = xmalloc(combine_diff_path_size(num_parent, len));
p->path = (char *) &(p->parent[num_parent]);
memcpy(p->path, path, len);
p->path[len] = 0;
p->next = NULL;
memset(p->parent, 0,
sizeof(p->parent[0]) * num_parent);
oidcpy(&p->oid, &q->queue[i]->two->oid);
p->mode = q->queue[i]->two->mode;
oidcpy(&p->parent[n].oid, &q->queue[i]->one->oid);
p->parent[n].mode = q->queue[i]->one->mode;
p->parent[n].status = q->queue[i]->status;
if (combined_all_paths &&
filename_changed(p->parent[n].status)) {
strbuf_init(&p->parent[n].path, 0);
strbuf_addstr(&p->parent[n].path,
q->queue[i]->one->path);
}
*tail = p;
tail = &p->next;
}
return curr;
}
i = 0;
while ((p = *tail) != NULL) {
cmp = ((i >= q->nr)
? -1 : compare_paths(p, q->queue[i]->two));
if (cmp < 0) {
*tail = p->next;
for (j = 0; j < num_parent; j++)
if (combined_all_paths &&
filename_changed(p->parent[j].status))
strbuf_release(&p->parent[j].path);
free(p);
continue;
}
if (cmp > 0) {
i++;
continue;
}
oidcpy(&p->parent[n].oid, &q->queue[i]->one->oid);
p->parent[n].mode = q->queue[i]->one->mode;
p->parent[n].status = q->queue[i]->status;
if (combined_all_paths &&
filename_changed(p->parent[n].status))
strbuf_addstr(&p->parent[n].path,
q->queue[i]->one->path);
tail = &p->next;
i++;
}
return curr;
}
struct lline {
struct lline *next, *prev;
int len;
unsigned long parent_map;
char line[FLEX_ARRAY];
};
struct plost {
struct lline *lost_head, *lost_tail;
int len;
};
struct sline {
struct lline *lost;
int lenlost;
struct plost plost;
char *bol;
int len;
unsigned long flag;
unsigned long *p_lno;
};
static int match_string_spaces(const char *line1, int len1,
const char *line2, int len2,
long flags)
{
if (flags & XDF_WHITESPACE_FLAGS) {
for (; len1 > 0 && XDL_ISSPACE(line1[len1 - 1]); len1--);
for (; len2 > 0 && XDL_ISSPACE(line2[len2 - 1]); len2--);
}
if (!(flags & (XDF_IGNORE_WHITESPACE | XDF_IGNORE_WHITESPACE_CHANGE)))
return (len1 == len2 && !memcmp(line1, line2, len1));
while (len1 > 0 && len2 > 0) {
len1--;
len2--;
if (XDL_ISSPACE(line1[len1]) || XDL_ISSPACE(line2[len2])) {
if ((flags & XDF_IGNORE_WHITESPACE_CHANGE) &&
(!XDL_ISSPACE(line1[len1]) || !XDL_ISSPACE(line2[len2])))
return 0;
for (; len1 > 0 && XDL_ISSPACE(line1[len1]); len1--);
for (; len2 > 0 && XDL_ISSPACE(line2[len2]); len2--);
}
if (line1[len1] != line2[len2])
return 0;
}
if (flags & XDF_IGNORE_WHITESPACE) {
for (; len1 > 0 && XDL_ISSPACE(line1[len1 - 1]); len1--);
for (; len2 > 0 && XDL_ISSPACE(line2[len2 - 1]); len2--);
}
if (!len1 && !len2)
return 1;
return 0;
}
enum coalesce_direction { MATCH, BASE, NEW };
static struct lline *coalesce_lines(struct lline *base, int *lenbase,
struct lline *newline, int lennew,
unsigned long parent, long flags)
{
int **lcs;
enum coalesce_direction **directions;
struct lline *baseend, *newend = NULL;
int i, j, origbaselen = *lenbase;
if (newline == NULL)
return base;
if (base == NULL) {
*lenbase = lennew;
return newline;
}
lcs = xcalloc(st_add(origbaselen, 1), sizeof(int*));
directions = xcalloc(st_add(origbaselen, 1), sizeof(enum coalesce_direction*));
for (i = 0; i < origbaselen + 1; i++) {
lcs[i] = xcalloc(st_add(lennew, 1), sizeof(int));
directions[i] = xcalloc(st_add(lennew, 1), sizeof(enum coalesce_direction));
directions[i][0] = BASE;
}
for (j = 1; j < lennew + 1; j++)
directions[0][j] = NEW;
for (i = 1, baseend = base; i < origbaselen + 1; i++) {
for (j = 1, newend = newline; j < lennew + 1; j++) {
if (match_string_spaces(baseend->line, baseend->len,
newend->line, newend->len, flags)) {
lcs[i][j] = lcs[i - 1][j - 1] + 1;
directions[i][j] = MATCH;
} else if (lcs[i][j - 1] >= lcs[i - 1][j]) {
lcs[i][j] = lcs[i][j - 1];
directions[i][j] = NEW;
} else {
lcs[i][j] = lcs[i - 1][j];
directions[i][j] = BASE;
}
if (newend->next)
newend = newend->next;
}
if (baseend->next)
baseend = baseend->next;
}
for (i = 0; i < origbaselen + 1; i++)
free(lcs[i]);
free(lcs);
i--;
j--;
while (i != 0 || j != 0) {
if (directions[i][j] == MATCH) {
baseend->parent_map |= 1<<parent;
baseend = baseend->prev;
newend = newend->prev;
i--;
j--;
} else if (directions[i][j] == NEW) {
struct lline *lline;
lline = newend;
if (lline->prev)
lline->prev->next = lline->next;
else
newline = lline->next;
if (lline->next)
lline->next->prev = lline->prev;
newend = lline->prev;
j--;
if (baseend) {
lline->next = baseend->next;
lline->prev = baseend;
if (lline->prev)
lline->prev->next = lline;
}
else {
lline->next = base;
base = lline;
}
(*lenbase)++;
if (lline->next)
lline->next->prev = lline;
} else {
baseend = baseend->prev;
i--;
}
}
newend = newline;
while (newend) {
struct lline *lline = newend;
newend = newend->next;
free(lline);
}
for (i = 0; i < origbaselen + 1; i++)
free(directions[i]);
free(directions);
return base;
}
static char *grab_blob(struct repository *r,
const struct object_id *oid, unsigned int mode,
unsigned long *size, struct userdiff_driver *textconv,
const char *path)
{
char *blob;
enum object_type type;
if (S_ISGITLINK(mode)) {
struct strbuf buf = STRBUF_INIT;
strbuf_addf(&buf, "Subproject commit %s\n", oid_to_hex(oid));
*size = buf.len;
blob = strbuf_detach(&buf, NULL);
} else if (is_null_oid(oid)) {
*size = 0;
return xcalloc(1, 1);
} else if (textconv) {
struct diff_filespec *df = alloc_filespec(path);
fill_filespec(df, oid, 1, mode);
*size = fill_textconv(r, textconv, df, &blob);
free_filespec(df);
} else {
blob = read_object_file(oid, &type, size);
if (type != OBJ_BLOB)
die("object '%s' is not a blob!", oid_to_hex(oid));
}
return blob;
}
static void append_lost(struct sline *sline, int n, const char *line, int len)
{
struct lline *lline;
unsigned long this_mask = (1UL<<n);
if (line[len-1] == '\n')
len--;
FLEX_ALLOC_MEM(lline, line, line, len);
lline->len = len;
lline->next = NULL;
lline->prev = sline->plost.lost_tail;
if (lline->prev)
lline->prev->next = lline;
else
sline->plost.lost_head = lline;
sline->plost.lost_tail = lline;
sline->plost.len++;
lline->parent_map = this_mask;
}
struct combine_diff_state {
unsigned int lno;
int ob, on, nb, nn;
unsigned long nmask;
int num_parent;
int n;
struct sline *sline;
struct sline *lost_bucket;
};
static void consume_hunk(void *state_,
long ob, long on,
long nb, long nn,
const char *funcline, long funclen)
{
struct combine_diff_state *state = state_;
state->ob = ob;
state->on = on;
state->nb = nb;
state->nn = nn;
state->lno = state->nb;
if (state->nn == 0) {
state->lost_bucket = &state->sline[state->nb];
if (!state->nb)
state->nb = 1;
} else {
state->lost_bucket = &state->sline[state->nb-1];
}
if (!state->sline[state->nb-1].p_lno)
state->sline[state->nb-1].p_lno =
xcalloc(state->num_parent, sizeof(unsigned long));
state->sline[state->nb-1].p_lno[state->n] = state->ob;
}
static void consume_line(void *state_, char *line, unsigned long len)
{
struct combine_diff_state *state = state_;
if (!state->lost_bucket)
return; 
switch (line[0]) {
case '-':
append_lost(state->lost_bucket, state->n, line+1, len-1);
break;
case '+':
state->sline[state->lno-1].flag |= state->nmask;
state->lno++;
break;
}
}
static void combine_diff(struct repository *r,
const struct object_id *parent, unsigned int mode,
mmfile_t *result_file,
struct sline *sline, unsigned int cnt, int n,
int num_parent, int result_deleted,
struct userdiff_driver *textconv,
const char *path, long flags)
{
unsigned int p_lno, lno;
unsigned long nmask = (1UL << n);
xpparam_t xpp;
xdemitconf_t xecfg;
mmfile_t parent_file;
struct combine_diff_state state;
unsigned long sz;
if (result_deleted)
return; 
parent_file.ptr = grab_blob(r, parent, mode, &sz, textconv, path);
parent_file.size = sz;
memset(&xpp, 0, sizeof(xpp));
xpp.flags = flags;
memset(&xecfg, 0, sizeof(xecfg));
memset(&state, 0, sizeof(state));
state.nmask = nmask;
state.sline = sline;
state.lno = 1;
state.num_parent = num_parent;
state.n = n;
if (xdi_diff_outf(&parent_file, result_file, consume_hunk,
consume_line, &state, &xpp, &xecfg))
die("unable to generate combined diff for %s",
oid_to_hex(parent));
free(parent_file.ptr);
for (lno = 0, p_lno = 1; lno <= cnt; lno++) {
struct lline *ll;
sline[lno].p_lno[n] = p_lno;
if (sline[lno].plost.lost_head) {
struct sline *sl = &sline[lno];
sl->lost = coalesce_lines(sl->lost, &sl->lenlost,
sl->plost.lost_head,
sl->plost.len, n, flags);
sl->plost.lost_head = sl->plost.lost_tail = NULL;
sl->plost.len = 0;
}
ll = sline[lno].lost;
while (ll) {
if (ll->parent_map & nmask)
p_lno++; 
ll = ll->next;
}
if (lno < cnt && !(sline[lno].flag & nmask))
p_lno++; 
}
sline[lno].p_lno[n] = p_lno; 
}
static unsigned long context = 3;
static char combine_marker = '@';
static int interesting(struct sline *sline, unsigned long all_mask)
{
return ((sline->flag & all_mask) || sline->lost);
}
static unsigned long adjust_hunk_tail(struct sline *sline,
unsigned long all_mask,
unsigned long hunk_begin,
unsigned long i)
{
if ((hunk_begin + 1 <= i) && !(sline[i-1].flag & all_mask))
i--;
return i;
}
static unsigned long find_next(struct sline *sline,
unsigned long mark,
unsigned long i,
unsigned long cnt,
int look_for_uninteresting)
{
while (i <= cnt)
if (look_for_uninteresting
? !(sline[i].flag & mark)
: (sline[i].flag & mark))
return i;
else
i++;
return i;
}
static int give_context(struct sline *sline, unsigned long cnt, int num_parent)
{
unsigned long all_mask = (1UL<<num_parent) - 1;
unsigned long mark = (1UL<<num_parent);
unsigned long no_pre_delete = (2UL<<num_parent);
unsigned long i;
i = find_next(sline, mark, 0, cnt, 0);
if (cnt < i)
return 0;
while (i <= cnt) {
unsigned long j = (context < i) ? (i - context) : 0;
unsigned long k;
while (j < i) {
if (!(sline[j].flag & mark))
sline[j].flag |= no_pre_delete;
sline[j++].flag |= mark;
}
again:
j = find_next(sline, mark, i, cnt, 1);
if (cnt < j)
break; 
k = find_next(sline, mark, j, cnt, 0);
j = adjust_hunk_tail(sline, all_mask, i, j);
if (k < j + context) {
while (j < k)
sline[j++].flag |= mark;
i = k;
goto again;
}
i = k;
k = (j + context < cnt+1) ? j + context : cnt+1;
while (j < k)
sline[j++].flag |= mark;
}
return 1;
}
static int make_hunks(struct sline *sline, unsigned long cnt,
int num_parent, int dense)
{
unsigned long all_mask = (1UL<<num_parent) - 1;
unsigned long mark = (1UL<<num_parent);
unsigned long i;
int has_interesting = 0;
for (i = 0; i <= cnt; i++) {
if (interesting(&sline[i], all_mask))
sline[i].flag |= mark;
else
sline[i].flag &= ~mark;
}
if (!dense)
return give_context(sline, cnt, num_parent);
i = 0;
while (i <= cnt) {
unsigned long j, hunk_begin, hunk_end;
unsigned long same_diff;
while (i <= cnt && !(sline[i].flag & mark))
i++;
if (cnt < i)
break; 
hunk_begin = i;
for (j = i + 1; j <= cnt; j++) {
if (!(sline[j].flag & mark)) {
unsigned long la; 
int contin = 0;
la = adjust_hunk_tail(sline, all_mask,
hunk_begin, j);
la = (la + context < cnt + 1) ?
(la + context) : cnt + 1;
while (la && j <= --la) {
if (sline[la].flag & mark) {
contin = 1;
break;
}
}
if (!contin)
break;
j = la;
}
}
hunk_end = j;
same_diff = 0;
has_interesting = 0;
for (j = i; j < hunk_end && !has_interesting; j++) {
unsigned long this_diff = sline[j].flag & all_mask;
struct lline *ll = sline[j].lost;
if (this_diff) {
if (!same_diff)
same_diff = this_diff;
else if (same_diff != this_diff) {
has_interesting = 1;
break;
}
}
while (ll && !has_interesting) {
this_diff = ll->parent_map;
if (!same_diff)
same_diff = this_diff;
else if (same_diff != this_diff) {
has_interesting = 1;
}
ll = ll->next;
}
}
if (!has_interesting && same_diff != all_mask) {
for (j = hunk_begin; j < hunk_end; j++)
sline[j].flag &= ~mark;
}
i = hunk_end;
}
has_interesting = give_context(sline, cnt, num_parent);
return has_interesting;
}
static void show_parent_lno(struct sline *sline, unsigned long l0, unsigned long l1, int n, unsigned long null_context)
{
l0 = sline[l0].p_lno[n];
l1 = sline[l1].p_lno[n];
printf(" -%lu,%lu", l0, l1-l0-null_context);
}
static int hunk_comment_line(const char *bol)
{
int ch;
if (!bol)
return 0;
ch = *bol & 0xff;
return (isalpha(ch) || ch == '_' || ch == '$');
}
static void show_line_to_eol(const char *line, int len, const char *reset)
{
int saw_cr_at_eol = 0;
if (len < 0)
len = strlen(line);
saw_cr_at_eol = (len && line[len-1] == '\r');
printf("%.*s%s%s\n", len - saw_cr_at_eol, line,
reset,
saw_cr_at_eol ? "\r" : "");
}
static void dump_sline(struct sline *sline, const char *line_prefix,
unsigned long cnt, int num_parent,
int use_color, int result_deleted)
{
unsigned long mark = (1UL<<num_parent);
unsigned long no_pre_delete = (2UL<<num_parent);
int i;
unsigned long lno = 0;
const char *c_frag = diff_get_color(use_color, DIFF_FRAGINFO);
const char *c_func = diff_get_color(use_color, DIFF_FUNCINFO);
const char *c_new = diff_get_color(use_color, DIFF_FILE_NEW);
const char *c_old = diff_get_color(use_color, DIFF_FILE_OLD);
const char *c_context = diff_get_color(use_color, DIFF_CONTEXT);
const char *c_reset = diff_get_color(use_color, DIFF_RESET);
if (result_deleted)
return; 
while (1) {
unsigned long hunk_end;
unsigned long rlines;
const char *hunk_comment = NULL;
unsigned long null_context = 0;
while (lno <= cnt && !(sline[lno].flag & mark)) {
if (hunk_comment_line(sline[lno].bol))
hunk_comment = sline[lno].bol;
lno++;
}
if (cnt < lno)
break;
else {
for (hunk_end = lno + 1; hunk_end <= cnt; hunk_end++)
if (!(sline[hunk_end].flag & mark))
break;
}
rlines = hunk_end - lno;
if (cnt < hunk_end)
rlines--; 
if (!context) {
unsigned long j;
for (j = lno; j < hunk_end; j++)
if (!(sline[j].flag & (mark-1)))
null_context++;
rlines -= null_context;
}
printf("%s%s", line_prefix, c_frag);
for (i = 0; i <= num_parent; i++) putchar(combine_marker);
for (i = 0; i < num_parent; i++)
show_parent_lno(sline, lno, hunk_end, i, null_context);
printf(" +%lu,%lu ", lno+1, rlines);
for (i = 0; i <= num_parent; i++) putchar(combine_marker);
if (hunk_comment) {
int comment_end = 0;
for (i = 0; i < 40; i++) {
int ch = hunk_comment[i] & 0xff;
if (!ch || ch == '\n')
break;
if (!isspace(ch))
comment_end = i;
}
if (comment_end)
printf("%s%s %s%s", c_reset,
c_context, c_reset,
c_func);
for (i = 0; i < comment_end; i++)
putchar(hunk_comment[i]);
}
printf("%s\n", c_reset);
while (lno < hunk_end) {
struct lline *ll;
int j;
unsigned long p_mask;
struct sline *sl = &sline[lno++];
ll = (sl->flag & no_pre_delete) ? NULL : sl->lost;
while (ll) {
printf("%s%s", line_prefix, c_old);
for (j = 0; j < num_parent; j++) {
if (ll->parent_map & (1UL<<j))
putchar('-');
else
putchar(' ');
}
show_line_to_eol(ll->line, -1, c_reset);
ll = ll->next;
}
if (cnt < lno)
break;
p_mask = 1;
fputs(line_prefix, stdout);
if (!(sl->flag & (mark-1))) {
if (!context)
continue;
fputs(c_context, stdout);
}
else
fputs(c_new, stdout);
for (j = 0; j < num_parent; j++) {
if (p_mask & sl->flag)
putchar('+');
else
putchar(' ');
p_mask <<= 1;
}
show_line_to_eol(sl->bol, sl->len, c_reset);
}
}
}
static void reuse_combine_diff(struct sline *sline, unsigned long cnt,
int i, int j)
{
unsigned long lno, imask, jmask;
imask = (1UL<<i);
jmask = (1UL<<j);
for (lno = 0; lno <= cnt; lno++) {
struct lline *ll = sline->lost;
sline->p_lno[i] = sline->p_lno[j];
while (ll) {
if (ll->parent_map & jmask)
ll->parent_map |= imask;
ll = ll->next;
}
if (sline->flag & jmask)
sline->flag |= imask;
sline++;
}
sline->p_lno[i] = sline->p_lno[j];
}
static void dump_quoted_path(const char *head,
const char *prefix,
const char *path,
const char *line_prefix,
const char *c_meta, const char *c_reset)
{
static struct strbuf buf = STRBUF_INIT;
strbuf_reset(&buf);
strbuf_addstr(&buf, line_prefix);
strbuf_addstr(&buf, c_meta);
strbuf_addstr(&buf, head);
quote_two_c_style(&buf, prefix, path, 0);
strbuf_addstr(&buf, c_reset);
puts(buf.buf);
}
static void show_combined_header(struct combine_diff_path *elem,
int num_parent,
int dense,
struct rev_info *rev,
const char *line_prefix,
int mode_differs,
int show_file_header)
{
struct diff_options *opt = &rev->diffopt;
int abbrev = opt->flags.full_index ? the_hash_algo->hexsz : DEFAULT_ABBREV;
const char *a_prefix = opt->a_prefix ? opt->a_prefix : "a/";
const char *b_prefix = opt->b_prefix ? opt->b_prefix : "b/";
const char *c_meta = diff_get_color_opt(opt, DIFF_METAINFO);
const char *c_reset = diff_get_color_opt(opt, DIFF_RESET);
const char *abb;
int added = 0;
int deleted = 0;
int i;
if (rev->loginfo && !rev->no_commit_id)
show_log(rev);
dump_quoted_path(dense ? "diff --cc " : "diff --combined ",
"", elem->path, line_prefix, c_meta, c_reset);
printf("%s%sindex ", line_prefix, c_meta);
for (i = 0; i < num_parent; i++) {
abb = find_unique_abbrev(&elem->parent[i].oid,
abbrev);
printf("%s%s", i ? "," : "", abb);
}
abb = find_unique_abbrev(&elem->oid, abbrev);
printf("..%s%s\n", abb, c_reset);
if (mode_differs) {
deleted = !elem->mode;
added = !deleted;
for (i = 0; added && i < num_parent; i++)
if (elem->parent[i].status !=
DIFF_STATUS_ADDED)
added = 0;
if (added)
printf("%s%snew file mode %06o",
line_prefix, c_meta, elem->mode);
else {
if (deleted)
printf("%s%sdeleted file ",
line_prefix, c_meta);
printf("mode ");
for (i = 0; i < num_parent; i++) {
printf("%s%06o", i ? "," : "",
elem->parent[i].mode);
}
if (elem->mode)
printf("..%06o", elem->mode);
}
printf("%s\n", c_reset);
}
if (!show_file_header)
return;
if (rev->combined_all_paths) {
for (i = 0; i < num_parent; i++) {
char *path = filename_changed(elem->parent[i].status)
? elem->parent[i].path.buf : elem->path;
if (elem->parent[i].status == DIFF_STATUS_ADDED)
dump_quoted_path("--- ", "", "/dev/null",
line_prefix, c_meta, c_reset);
else
dump_quoted_path("--- ", a_prefix, path,
line_prefix, c_meta, c_reset);
}
} else {
if (added)
dump_quoted_path("--- ", "", "/dev/null",
line_prefix, c_meta, c_reset);
else
dump_quoted_path("--- ", a_prefix, elem->path,
line_prefix, c_meta, c_reset);
}
if (deleted)
dump_quoted_path("+++ ", "", "/dev/null",
line_prefix, c_meta, c_reset);
else
dump_quoted_path("+++ ", b_prefix, elem->path,
line_prefix, c_meta, c_reset);
}
static void show_patch_diff(struct combine_diff_path *elem, int num_parent,
int dense, int working_tree_file,
struct rev_info *rev)
{
struct diff_options *opt = &rev->diffopt;
unsigned long result_size, cnt, lno;
int result_deleted = 0;
char *result, *cp;
struct sline *sline; 
int mode_differs = 0;
int i, show_hunks;
mmfile_t result_file;
struct userdiff_driver *userdiff;
struct userdiff_driver *textconv = NULL;
int is_binary;
const char *line_prefix = diff_line_prefix(opt);
context = opt->context;
userdiff = userdiff_find_by_path(opt->repo->index, elem->path);
if (!userdiff)
userdiff = userdiff_find_by_name("default");
if (opt->flags.allow_textconv)
textconv = userdiff_get_textconv(opt->repo, userdiff);
if (!working_tree_file)
result = grab_blob(opt->repo, &elem->oid, elem->mode, &result_size,
textconv, elem->path);
else {
struct stat st;
int fd = -1;
if (lstat(elem->path, &st) < 0)
goto deleted_file;
if (S_ISLNK(st.st_mode)) {
struct strbuf buf = STRBUF_INIT;
if (strbuf_readlink(&buf, elem->path, st.st_size) < 0) {
error_errno("readlink(%s)", elem->path);
return;
}
result_size = buf.len;
result = strbuf_detach(&buf, NULL);
elem->mode = canon_mode(st.st_mode);
} else if (S_ISDIR(st.st_mode)) {
struct object_id oid;
if (resolve_gitlink_ref(elem->path, "HEAD", &oid) < 0)
result = grab_blob(opt->repo, &elem->oid,
elem->mode, &result_size,
NULL, NULL);
else
result = grab_blob(opt->repo, &oid, elem->mode,
&result_size, NULL, NULL);
} else if (textconv) {
struct diff_filespec *df = alloc_filespec(elem->path);
fill_filespec(df, &null_oid, 0, st.st_mode);
result_size = fill_textconv(opt->repo, textconv, df, &result);
free_filespec(df);
} else if (0 <= (fd = open(elem->path, O_RDONLY))) {
size_t len = xsize_t(st.st_size);
ssize_t done;
int is_file, i;
elem->mode = canon_mode(st.st_mode);
is_file = has_symlinks;
for (i = 0; !is_file && i < num_parent; i++)
is_file = !S_ISLNK(elem->parent[i].mode);
if (!is_file)
elem->mode = canon_mode(S_IFLNK);
result_size = len;
result = xmallocz(len);
done = read_in_full(fd, result, len);
if (done < 0)
die_errno("read error '%s'", elem->path);
else if (done < len)
die("early EOF '%s'", elem->path);
if (is_file) {
struct strbuf buf = STRBUF_INIT;
if (convert_to_git(rev->diffopt.repo->index,
elem->path, result, len, &buf, global_conv_flags_eol)) {
free(result);
result = strbuf_detach(&buf, &len);
result_size = len;
}
}
}
else {
deleted_file:
result_deleted = 1;
result_size = 0;
elem->mode = 0;
result = xcalloc(1, 1);
}
if (0 <= fd)
close(fd);
}
for (i = 0; i < num_parent; i++) {
if (elem->parent[i].mode != elem->mode) {
mode_differs = 1;
break;
}
}
if (textconv)
is_binary = 0;
else if (userdiff->binary != -1)
is_binary = userdiff->binary;
else {
is_binary = buffer_is_binary(result, result_size);
for (i = 0; !is_binary && i < num_parent; i++) {
char *buf;
unsigned long size;
buf = grab_blob(opt->repo,
&elem->parent[i].oid,
elem->parent[i].mode,
&size, NULL, NULL);
if (buffer_is_binary(buf, size))
is_binary = 1;
free(buf);
}
}
if (is_binary) {
show_combined_header(elem, num_parent, dense, rev,
line_prefix, mode_differs, 0);
printf("Binary files differ\n");
free(result);
return;
}
for (cnt = 0, cp = result; cp < result + result_size; cp++) {
if (*cp == '\n')
cnt++;
}
if (result_size && result[result_size-1] != '\n')
cnt++; 
sline = xcalloc(st_add(cnt, 2), sizeof(*sline));
sline[0].bol = result;
for (lno = 0, cp = result; cp < result + result_size; cp++) {
if (*cp == '\n') {
sline[lno].len = cp - sline[lno].bol;
lno++;
if (lno < cnt)
sline[lno].bol = cp + 1;
}
}
if (result_size && result[result_size-1] != '\n')
sline[cnt-1].len = result_size - (sline[cnt-1].bol - result);
result_file.ptr = result;
result_file.size = result_size;
sline[0].p_lno = xcalloc(st_mult(st_add(cnt, 2), num_parent), sizeof(unsigned long));
for (lno = 0; lno <= cnt; lno++)
sline[lno+1].p_lno = sline[lno].p_lno + num_parent;
for (i = 0; i < num_parent; i++) {
int j;
for (j = 0; j < i; j++) {
if (oideq(&elem->parent[i].oid,
&elem->parent[j].oid)) {
reuse_combine_diff(sline, cnt, i, j);
break;
}
}
if (i <= j)
combine_diff(opt->repo,
&elem->parent[i].oid,
elem->parent[i].mode,
&result_file, sline,
cnt, i, num_parent, result_deleted,
textconv, elem->path, opt->xdl_opts);
}
show_hunks = make_hunks(sline, cnt, num_parent, dense);
if (show_hunks || mode_differs || working_tree_file) {
show_combined_header(elem, num_parent, dense, rev,
line_prefix, mode_differs, 1);
dump_sline(sline, line_prefix, cnt, num_parent,
opt->use_color, result_deleted);
}
free(result);
for (lno = 0; lno < cnt; lno++) {
if (sline[lno].lost) {
struct lline *ll = sline[lno].lost;
while (ll) {
struct lline *tmp = ll;
ll = ll->next;
free(tmp);
}
}
}
free(sline[0].p_lno);
free(sline);
}
static void show_raw_diff(struct combine_diff_path *p, int num_parent, struct rev_info *rev)
{
struct diff_options *opt = &rev->diffopt;
int line_termination, inter_name_termination, i;
const char *line_prefix = diff_line_prefix(opt);
line_termination = opt->line_termination;
inter_name_termination = '\t';
if (!line_termination)
inter_name_termination = 0;
if (rev->loginfo && !rev->no_commit_id)
show_log(rev);
if (opt->output_format & DIFF_FORMAT_RAW) {
printf("%s", line_prefix);
for (i = 0; i < num_parent; i++)
putchar(':');
for (i = 0; i < num_parent; i++)
printf("%06o ", p->parent[i].mode);
printf("%06o", p->mode);
for (i = 0; i < num_parent; i++)
printf(" %s", diff_aligned_abbrev(&p->parent[i].oid,
opt->abbrev));
printf(" %s ", diff_aligned_abbrev(&p->oid, opt->abbrev));
}
if (opt->output_format & (DIFF_FORMAT_RAW | DIFF_FORMAT_NAME_STATUS)) {
for (i = 0; i < num_parent; i++)
putchar(p->parent[i].status);
putchar(inter_name_termination);
}
for (i = 0; i < num_parent; i++)
if (rev->combined_all_paths) {
if (filename_changed(p->parent[i].status))
write_name_quoted(p->parent[i].path.buf, stdout,
inter_name_termination);
else
write_name_quoted(p->path, stdout,
inter_name_termination);
}
write_name_quoted(p->path, stdout, line_termination);
}
void show_combined_diff(struct combine_diff_path *p,
int num_parent,
int dense,
struct rev_info *rev)
{
struct diff_options *opt = &rev->diffopt;
if (opt->output_format & (DIFF_FORMAT_RAW |
DIFF_FORMAT_NAME |
DIFF_FORMAT_NAME_STATUS))
show_raw_diff(p, num_parent, rev);
else if (opt->output_format & DIFF_FORMAT_PATCH)
show_patch_diff(p, num_parent, dense, 1, rev);
}
static void free_combined_pair(struct diff_filepair *pair)
{
free(pair->two);
free(pair);
}
static struct diff_filepair *combined_pair(struct combine_diff_path *p,
int num_parent)
{
int i;
struct diff_filepair *pair;
struct diff_filespec *pool;
pair = xmalloc(sizeof(*pair));
pool = xcalloc(st_add(num_parent, 1), sizeof(struct diff_filespec));
pair->one = pool + 1;
pair->two = pool;
for (i = 0; i < num_parent; i++) {
pair->one[i].path = p->path;
pair->one[i].mode = p->parent[i].mode;
oidcpy(&pair->one[i].oid, &p->parent[i].oid);
pair->one[i].oid_valid = !is_null_oid(&p->parent[i].oid);
pair->one[i].has_more_entries = 1;
}
pair->one[num_parent - 1].has_more_entries = 0;
pair->two->path = p->path;
pair->two->mode = p->mode;
oidcpy(&pair->two->oid, &p->oid);
pair->two->oid_valid = !is_null_oid(&p->oid);
return pair;
}
static void handle_combined_callback(struct diff_options *opt,
struct combine_diff_path *paths,
int num_parent,
int num_paths)
{
struct combine_diff_path *p;
struct diff_queue_struct q;
int i;
q.queue = xcalloc(num_paths, sizeof(struct diff_filepair *));
q.alloc = num_paths;
q.nr = num_paths;
for (i = 0, p = paths; p; p = p->next)
q.queue[i++] = combined_pair(p, num_parent);
opt->format_callback(&q, opt, opt->format_callback_data);
for (i = 0; i < num_paths; i++)
free_combined_pair(q.queue[i]);
free(q.queue);
}
static const char *path_path(void *obj)
{
struct combine_diff_path *path = (struct combine_diff_path *)obj;
return path->path;
}
#define STAT_FORMAT_MASK (DIFF_FORMAT_NUMSTAT | DIFF_FORMAT_SHORTSTAT | DIFF_FORMAT_SUMMARY | DIFF_FORMAT_DIRSTAT | DIFF_FORMAT_DIFFSTAT)
static struct combine_diff_path *find_paths_generic(const struct object_id *oid,
const struct oid_array *parents,
struct diff_options *opt,
int combined_all_paths)
{
struct combine_diff_path *paths = NULL;
int i, num_parent = parents->nr;
int output_format = opt->output_format;
const char *orderfile = opt->orderfile;
opt->output_format = DIFF_FORMAT_NO_OUTPUT;
opt->orderfile = NULL;
for (i = 0; i < num_parent; i++) {
int stat_opt = output_format & STAT_FORMAT_MASK;
if (i == 0 && stat_opt)
opt->output_format = stat_opt;
else
opt->output_format = DIFF_FORMAT_NO_OUTPUT;
diff_tree_oid(&parents->oid[i], oid, "", opt);
diffcore_std(opt);
paths = intersect_paths(paths, i, num_parent,
combined_all_paths);
if (opt->output_format != DIFF_FORMAT_NO_OUTPUT &&
orderfile) {
diffcore_order(orderfile);
}
diff_flush(opt);
}
opt->output_format = output_format;
opt->orderfile = orderfile;
return paths;
}
static struct combine_diff_path *find_paths_multitree(
const struct object_id *oid, const struct oid_array *parents,
struct diff_options *opt)
{
int i, nparent = parents->nr;
const struct object_id **parents_oid;
struct combine_diff_path paths_head;
struct strbuf base;
ALLOC_ARRAY(parents_oid, nparent);
for (i = 0; i < nparent; i++)
parents_oid[i] = &parents->oid[i];
paths_head.next = NULL;
strbuf_init(&base, PATH_MAX);
diff_tree_paths(&paths_head, oid, parents_oid, nparent, &base, opt);
strbuf_release(&base);
free(parents_oid);
return paths_head.next;
}
void diff_tree_combined(const struct object_id *oid,
const struct oid_array *parents,
int dense,
struct rev_info *rev)
{
struct diff_options *opt = &rev->diffopt;
struct diff_options diffopts;
struct combine_diff_path *p, *paths;
int i, num_paths, needsep, show_log_first, num_parent = parents->nr;
int need_generic_pathscan;
if (!num_parent)
return;
show_log_first = !!rev->loginfo && !rev->no_commit_id;
needsep = 0;
if (show_log_first) {
show_log(rev);
if (rev->verbose_header && opt->output_format &&
opt->output_format != DIFF_FORMAT_NO_OUTPUT &&
!commit_format_is_empty(rev->commit_format))
printf("%s%c", diff_line_prefix(opt),
opt->line_termination);
}
diffopts = *opt;
copy_pathspec(&diffopts.pathspec, &opt->pathspec);
diffopts.flags.recursive = 1;
diffopts.flags.allow_external = 0;
need_generic_pathscan = opt->skip_stat_unmatch ||
opt->flags.follow_renames ||
opt->break_opt != -1 ||
opt->detect_rename ||
(opt->pickaxe_opts & DIFF_PICKAXE_KINDS_MASK) ||
opt->filter;
if (need_generic_pathscan) {
paths = find_paths_generic(oid, parents, &diffopts,
rev->combined_all_paths);
}
else {
int stat_opt;
paths = find_paths_multitree(oid, parents, &diffopts);
stat_opt = opt->output_format & STAT_FORMAT_MASK;
if (stat_opt) {
diffopts.output_format = stat_opt;
diff_tree_oid(&parents->oid[0], oid, "", &diffopts);
diffcore_std(&diffopts);
if (opt->orderfile)
diffcore_order(opt->orderfile);
diff_flush(&diffopts);
}
}
for (num_paths = 0, p = paths; p; p = p->next)
num_paths++;
if (opt->orderfile && num_paths) {
struct obj_order *o;
ALLOC_ARRAY(o, num_paths);
for (i = 0, p = paths; p; p = p->next, i++)
o[i].obj = p;
order_objects(opt->orderfile, path_path, o, num_paths);
for (i = 0; i < num_paths - 1; i++) {
p = o[i].obj;
p->next = o[i+1].obj;
}
p = o[num_paths-1].obj;
p->next = NULL;
paths = o[0].obj;
free(o);
}
if (num_paths) {
if (opt->output_format & (DIFF_FORMAT_RAW |
DIFF_FORMAT_NAME |
DIFF_FORMAT_NAME_STATUS)) {
for (p = paths; p; p = p->next)
show_raw_diff(p, num_parent, rev);
needsep = 1;
}
else if (opt->output_format & STAT_FORMAT_MASK)
needsep = 1;
else if (opt->output_format & DIFF_FORMAT_CALLBACK)
handle_combined_callback(opt, paths, num_parent, num_paths);
if (opt->output_format & DIFF_FORMAT_PATCH) {
if (needsep)
printf("%s%c", diff_line_prefix(opt),
opt->line_termination);
for (p = paths; p; p = p->next)
show_patch_diff(p, num_parent, dense,
0, rev);
}
}
while (paths) {
struct combine_diff_path *tmp = paths;
paths = paths->next;
for (i = 0; i < num_parent; i++)
if (rev->combined_all_paths &&
filename_changed(tmp->parent[i].status))
strbuf_release(&tmp->parent[i].path);
free(tmp);
}
clear_pathspec(&diffopts.pathspec);
}
void diff_tree_combined_merge(const struct commit *commit, int dense,
struct rev_info *rev)
{
struct commit_list *parent = get_saved_parents(rev, commit);
struct oid_array parents = OID_ARRAY_INIT;
while (parent) {
oid_array_append(&parents, &parent->item->object.oid);
parent = parent->next;
}
diff_tree_combined(&commit->object.oid, &parents, dense, rev);
oid_array_clear(&parents);
}
