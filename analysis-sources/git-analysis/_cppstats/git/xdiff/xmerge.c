#include "xinclude.h"
typedef struct s_xdmerge {
struct s_xdmerge *next;
int mode;
long i1, i2;
long chg1, chg2;
long i0;
long chg0;
} xdmerge_t;
static int xdl_append_merge(xdmerge_t **merge, int mode,
long i0, long chg0,
long i1, long chg1,
long i2, long chg2)
{
xdmerge_t *m = *merge;
if (m && (i1 <= m->i1 + m->chg1 || i2 <= m->i2 + m->chg2)) {
if (mode != m->mode)
m->mode = 0;
m->chg0 = i0 + chg0 - m->i0;
m->chg1 = i1 + chg1 - m->i1;
m->chg2 = i2 + chg2 - m->i2;
} else {
m = xdl_malloc(sizeof(xdmerge_t));
if (!m)
return -1;
m->next = NULL;
m->mode = mode;
m->i0 = i0;
m->chg0 = chg0;
m->i1 = i1;
m->chg1 = chg1;
m->i2 = i2;
m->chg2 = chg2;
if (*merge)
(*merge)->next = m;
*merge = m;
}
return 0;
}
static int xdl_cleanup_merge(xdmerge_t *c)
{
int count = 0;
xdmerge_t *next_c;
for (; c; c = next_c) {
if (c->mode == 0)
count++;
next_c = c->next;
free(c);
}
return count;
}
static int xdl_merge_cmp_lines(xdfenv_t *xe1, int i1, xdfenv_t *xe2, int i2,
int line_count, long flags)
{
int i;
xrecord_t **rec1 = xe1->xdf2.recs + i1;
xrecord_t **rec2 = xe2->xdf2.recs + i2;
for (i = 0; i < line_count; i++) {
int result = xdl_recmatch(rec1[i]->ptr, rec1[i]->size,
rec2[i]->ptr, rec2[i]->size, flags);
if (!result)
return -1;
}
return 0;
}
static int xdl_recs_copy_0(int use_orig, xdfenv_t *xe, int i, int count, int needs_cr, int add_nl, char *dest)
{
xrecord_t **recs;
int size = 0;
recs = (use_orig ? xe->xdf1.recs : xe->xdf2.recs) + i;
if (count < 1)
return 0;
for (i = 0; i < count; size += recs[i++]->size)
if (dest)
memcpy(dest + size, recs[i]->ptr, recs[i]->size);
if (add_nl) {
i = recs[count - 1]->size;
if (i == 0 || recs[count - 1]->ptr[i - 1] != '\n') {
if (needs_cr) {
if (dest)
dest[size] = '\r';
size++;
}
if (dest)
dest[size] = '\n';
size++;
}
}
return size;
}
static int xdl_recs_copy(xdfenv_t *xe, int i, int count, int needs_cr, int add_nl, char *dest)
{
return xdl_recs_copy_0(0, xe, i, count, needs_cr, add_nl, dest);
}
static int xdl_orig_copy(xdfenv_t *xe, int i, int count, int needs_cr, int add_nl, char *dest)
{
return xdl_recs_copy_0(1, xe, i, count, needs_cr, add_nl, dest);
}
static int is_eol_crlf(xdfile_t *file, int i)
{
long size;
if (i < file->nrec - 1)
return (size = file->recs[i]->size) > 1 &&
file->recs[i]->ptr[size - 2] == '\r';
if (!file->nrec)
return -1;
if ((size = file->recs[i]->size) &&
file->recs[i]->ptr[size - 1] == '\n')
return size > 1 &&
file->recs[i]->ptr[size - 2] == '\r';
if (!i)
return -1;
return (size = file->recs[i - 1]->size) > 1 &&
file->recs[i - 1]->ptr[size - 2] == '\r';
}
static int is_cr_needed(xdfenv_t *xe1, xdfenv_t *xe2, xdmerge_t *m)
{
int needs_cr;
needs_cr = is_eol_crlf(&xe1->xdf2, m->i1 ? m->i1 - 1 : 0);
if (needs_cr)
needs_cr = is_eol_crlf(&xe2->xdf2, m->i2 ? m->i2 - 1 : 0);
if (needs_cr)
needs_cr = is_eol_crlf(&xe1->xdf1, 0);
return needs_cr < 0 ? 0 : needs_cr;
}
static int fill_conflict_hunk(xdfenv_t *xe1, const char *name1,
xdfenv_t *xe2, const char *name2,
const char *name3,
int size, int i, int style,
xdmerge_t *m, char *dest, int marker_size)
{
int marker1_size = (name1 ? strlen(name1) + 1 : 0);
int marker2_size = (name2 ? strlen(name2) + 1 : 0);
int marker3_size = (name3 ? strlen(name3) + 1 : 0);
int needs_cr = is_cr_needed(xe1, xe2, m);
if (marker_size <= 0)
marker_size = DEFAULT_CONFLICT_MARKER_SIZE;
size += xdl_recs_copy(xe1, i, m->i1 - i, 0, 0,
dest ? dest + size : NULL);
if (!dest) {
size += marker_size + 1 + needs_cr + marker1_size;
} else {
memset(dest + size, '<', marker_size);
size += marker_size;
if (marker1_size) {
dest[size] = ' ';
memcpy(dest + size + 1, name1, marker1_size - 1);
size += marker1_size;
}
if (needs_cr)
dest[size++] = '\r';
dest[size++] = '\n';
}
size += xdl_recs_copy(xe1, m->i1, m->chg1, needs_cr, 1,
dest ? dest + size : NULL);
if (style == XDL_MERGE_DIFF3) {
if (!dest) {
size += marker_size + 1 + needs_cr + marker3_size;
} else {
memset(dest + size, '|', marker_size);
size += marker_size;
if (marker3_size) {
dest[size] = ' ';
memcpy(dest + size + 1, name3, marker3_size - 1);
size += marker3_size;
}
if (needs_cr)
dest[size++] = '\r';
dest[size++] = '\n';
}
size += xdl_orig_copy(xe1, m->i0, m->chg0, needs_cr, 1,
dest ? dest + size : NULL);
}
if (!dest) {
size += marker_size + 1 + needs_cr;
} else {
memset(dest + size, '=', marker_size);
size += marker_size;
if (needs_cr)
dest[size++] = '\r';
dest[size++] = '\n';
}
size += xdl_recs_copy(xe2, m->i2, m->chg2, needs_cr, 1,
dest ? dest + size : NULL);
if (!dest) {
size += marker_size + 1 + needs_cr + marker2_size;
} else {
memset(dest + size, '>', marker_size);
size += marker_size;
if (marker2_size) {
dest[size] = ' ';
memcpy(dest + size + 1, name2, marker2_size - 1);
size += marker2_size;
}
if (needs_cr)
dest[size++] = '\r';
dest[size++] = '\n';
}
return size;
}
static int xdl_fill_merge_buffer(xdfenv_t *xe1, const char *name1,
xdfenv_t *xe2, const char *name2,
const char *ancestor_name,
int favor,
xdmerge_t *m, char *dest, int style,
int marker_size)
{
int size, i;
for (size = i = 0; m; m = m->next) {
if (favor && !m->mode)
m->mode = favor;
if (m->mode == 0)
size = fill_conflict_hunk(xe1, name1, xe2, name2,
ancestor_name,
size, i, style, m, dest,
marker_size);
else if (m->mode & 3) {
size += xdl_recs_copy(xe1, i, m->i1 - i, 0, 0,
dest ? dest + size : NULL);
if (m->mode & 1) {
int needs_cr = is_cr_needed(xe1, xe2, m);
size += xdl_recs_copy(xe1, m->i1, m->chg1, needs_cr, (m->mode & 2),
dest ? dest + size : NULL);
}
if (m->mode & 2)
size += xdl_recs_copy(xe2, m->i2, m->chg2, 0, 0,
dest ? dest + size : NULL);
} else
continue;
i = m->i1 + m->chg1;
}
size += xdl_recs_copy(xe1, i, xe1->xdf2.nrec - i, 0, 0,
dest ? dest + size : NULL);
return size;
}
static int xdl_refine_conflicts(xdfenv_t *xe1, xdfenv_t *xe2, xdmerge_t *m,
xpparam_t const *xpp)
{
for (; m; m = m->next) {
mmfile_t t1, t2;
xdfenv_t xe;
xdchange_t *xscr, *x;
int i1 = m->i1, i2 = m->i2;
if (m->mode)
continue;
if (m->chg1 == 0 || m->chg2 == 0)
continue;
t1.ptr = (char *)xe1->xdf2.recs[m->i1]->ptr;
t1.size = xe1->xdf2.recs[m->i1 + m->chg1 - 1]->ptr
+ xe1->xdf2.recs[m->i1 + m->chg1 - 1]->size - t1.ptr;
t2.ptr = (char *)xe2->xdf2.recs[m->i2]->ptr;
t2.size = xe2->xdf2.recs[m->i2 + m->chg2 - 1]->ptr
+ xe2->xdf2.recs[m->i2 + m->chg2 - 1]->size - t2.ptr;
if (xdl_do_diff(&t1, &t2, xpp, &xe) < 0)
return -1;
if (xdl_change_compact(&xe.xdf1, &xe.xdf2, xpp->flags) < 0 ||
xdl_change_compact(&xe.xdf2, &xe.xdf1, xpp->flags) < 0 ||
xdl_build_script(&xe, &xscr) < 0) {
xdl_free_env(&xe);
return -1;
}
if (!xscr) {
xdl_free_env(&xe);
m->mode = 4;
continue;
}
x = xscr;
m->i1 = xscr->i1 + i1;
m->chg1 = xscr->chg1;
m->i2 = xscr->i2 + i2;
m->chg2 = xscr->chg2;
while (xscr->next) {
xdmerge_t *m2 = xdl_malloc(sizeof(xdmerge_t));
if (!m2) {
xdl_free_env(&xe);
xdl_free_script(x);
return -1;
}
xscr = xscr->next;
m2->next = m->next;
m->next = m2;
m = m2;
m->mode = 0;
m->i1 = xscr->i1 + i1;
m->chg1 = xscr->chg1;
m->i2 = xscr->i2 + i2;
m->chg2 = xscr->chg2;
}
xdl_free_env(&xe);
xdl_free_script(x);
}
return 0;
}
static int line_contains_alnum(const char *ptr, long size)
{
while (size--)
if (isalnum((unsigned char)*(ptr++)))
return 1;
return 0;
}
static int lines_contain_alnum(xdfenv_t *xe, int i, int chg)
{
for (; chg; chg--, i++)
if (line_contains_alnum(xe->xdf2.recs[i]->ptr,
xe->xdf2.recs[i]->size))
return 1;
return 0;
}
static void xdl_merge_two_conflicts(xdmerge_t *m)
{
xdmerge_t *next_m = m->next;
m->chg1 = next_m->i1 + next_m->chg1 - m->i1;
m->chg2 = next_m->i2 + next_m->chg2 - m->i2;
m->next = next_m->next;
free(next_m);
}
static int xdl_simplify_non_conflicts(xdfenv_t *xe1, xdmerge_t *m,
int simplify_if_no_alnum)
{
int result = 0;
if (!m)
return result;
for (;;) {
xdmerge_t *next_m = m->next;
int begin, end;
if (!next_m)
return result;
begin = m->i1 + m->chg1;
end = next_m->i1;
if (m->mode != 0 || next_m->mode != 0 ||
(end - begin > 3 &&
(!simplify_if_no_alnum ||
lines_contain_alnum(xe1, begin, end - begin)))) {
m = next_m;
} else {
result++;
xdl_merge_two_conflicts(m);
}
}
}
static int xdl_do_merge(xdfenv_t *xe1, xdchange_t *xscr1,
xdfenv_t *xe2, xdchange_t *xscr2,
xmparam_t const *xmp, mmbuffer_t *result)
{
xdmerge_t *changes, *c;
xpparam_t const *xpp = &xmp->xpp;
const char *const ancestor_name = xmp->ancestor;
const char *const name1 = xmp->file1;
const char *const name2 = xmp->file2;
int i0, i1, i2, chg0, chg1, chg2;
int level = xmp->level;
int style = xmp->style;
int favor = xmp->favor;
if (style == XDL_MERGE_DIFF3) {
if (XDL_MERGE_EAGER < level)
level = XDL_MERGE_EAGER;
}
c = changes = NULL;
while (xscr1 && xscr2) {
if (!changes)
changes = c;
if (xscr1->i1 + xscr1->chg1 < xscr2->i1) {
i0 = xscr1->i1;
i1 = xscr1->i2;
i2 = xscr2->i2 - xscr2->i1 + xscr1->i1;
chg0 = xscr1->chg1;
chg1 = xscr1->chg2;
chg2 = xscr1->chg1;
if (xdl_append_merge(&c, 1,
i0, chg0, i1, chg1, i2, chg2)) {
xdl_cleanup_merge(changes);
return -1;
}
xscr1 = xscr1->next;
continue;
}
if (xscr2->i1 + xscr2->chg1 < xscr1->i1) {
i0 = xscr2->i1;
i1 = xscr1->i2 - xscr1->i1 + xscr2->i1;
i2 = xscr2->i2;
chg0 = xscr2->chg1;
chg1 = xscr2->chg1;
chg2 = xscr2->chg2;
if (xdl_append_merge(&c, 2,
i0, chg0, i1, chg1, i2, chg2)) {
xdl_cleanup_merge(changes);
return -1;
}
xscr2 = xscr2->next;
continue;
}
if (level == XDL_MERGE_MINIMAL || xscr1->i1 != xscr2->i1 ||
xscr1->chg1 != xscr2->chg1 ||
xscr1->chg2 != xscr2->chg2 ||
xdl_merge_cmp_lines(xe1, xscr1->i2,
xe2, xscr2->i2,
xscr1->chg2, xpp->flags)) {
int off = xscr1->i1 - xscr2->i1;
int ffo = off + xscr1->chg1 - xscr2->chg1;
i0 = xscr1->i1;
i1 = xscr1->i2;
i2 = xscr2->i2;
if (off > 0) {
i0 -= off;
i1 -= off;
}
else
i2 += off;
chg0 = xscr1->i1 + xscr1->chg1 - i0;
chg1 = xscr1->i2 + xscr1->chg2 - i1;
chg2 = xscr2->i2 + xscr2->chg2 - i2;
if (ffo < 0) {
chg0 -= ffo;
chg1 -= ffo;
} else
chg2 += ffo;
if (xdl_append_merge(&c, 0,
i0, chg0, i1, chg1, i2, chg2)) {
xdl_cleanup_merge(changes);
return -1;
}
}
i1 = xscr1->i1 + xscr1->chg1;
i2 = xscr2->i1 + xscr2->chg1;
if (i1 >= i2)
xscr2 = xscr2->next;
if (i2 >= i1)
xscr1 = xscr1->next;
}
while (xscr1) {
if (!changes)
changes = c;
i0 = xscr1->i1;
i1 = xscr1->i2;
i2 = xscr1->i1 + xe2->xdf2.nrec - xe2->xdf1.nrec;
chg0 = xscr1->chg1;
chg1 = xscr1->chg2;
chg2 = xscr1->chg1;
if (xdl_append_merge(&c, 1,
i0, chg0, i1, chg1, i2, chg2)) {
xdl_cleanup_merge(changes);
return -1;
}
xscr1 = xscr1->next;
}
while (xscr2) {
if (!changes)
changes = c;
i0 = xscr2->i1;
i1 = xscr2->i1 + xe1->xdf2.nrec - xe1->xdf1.nrec;
i2 = xscr2->i2;
chg0 = xscr2->chg1;
chg1 = xscr2->chg1;
chg2 = xscr2->chg2;
if (xdl_append_merge(&c, 2,
i0, chg0, i1, chg1, i2, chg2)) {
xdl_cleanup_merge(changes);
return -1;
}
xscr2 = xscr2->next;
}
if (!changes)
changes = c;
if (XDL_MERGE_ZEALOUS <= level &&
(xdl_refine_conflicts(xe1, xe2, changes, xpp) < 0 ||
xdl_simplify_non_conflicts(xe1, changes,
XDL_MERGE_ZEALOUS < level) < 0)) {
xdl_cleanup_merge(changes);
return -1;
}
if (result) {
int marker_size = xmp->marker_size;
int size = xdl_fill_merge_buffer(xe1, name1, xe2, name2,
ancestor_name,
favor, changes, NULL, style,
marker_size);
result->ptr = xdl_malloc(size);
if (!result->ptr) {
xdl_cleanup_merge(changes);
return -1;
}
result->size = size;
xdl_fill_merge_buffer(xe1, name1, xe2, name2,
ancestor_name, favor, changes,
result->ptr, style, marker_size);
}
return xdl_cleanup_merge(changes);
}
int xdl_merge(mmfile_t *orig, mmfile_t *mf1, mmfile_t *mf2,
xmparam_t const *xmp, mmbuffer_t *result)
{
xdchange_t *xscr1, *xscr2;
xdfenv_t xe1, xe2;
int status;
xpparam_t const *xpp = &xmp->xpp;
result->ptr = NULL;
result->size = 0;
if (xdl_do_diff(orig, mf1, xpp, &xe1) < 0) {
return -1;
}
if (xdl_do_diff(orig, mf2, xpp, &xe2) < 0) {
xdl_free_env(&xe1);
return -1;
}
if (xdl_change_compact(&xe1.xdf1, &xe1.xdf2, xpp->flags) < 0 ||
xdl_change_compact(&xe1.xdf2, &xe1.xdf1, xpp->flags) < 0 ||
xdl_build_script(&xe1, &xscr1) < 0) {
xdl_free_env(&xe1);
return -1;
}
if (xdl_change_compact(&xe2.xdf1, &xe2.xdf2, xpp->flags) < 0 ||
xdl_change_compact(&xe2.xdf2, &xe2.xdf1, xpp->flags) < 0 ||
xdl_build_script(&xe2, &xscr2) < 0) {
xdl_free_script(xscr1);
xdl_free_env(&xe1);
xdl_free_env(&xe2);
return -1;
}
status = 0;
if (!xscr1) {
result->ptr = xdl_malloc(mf2->size);
memcpy(result->ptr, mf2->ptr, mf2->size);
result->size = mf2->size;
} else if (!xscr2) {
result->ptr = xdl_malloc(mf1->size);
memcpy(result->ptr, mf1->ptr, mf1->size);
result->size = mf1->size;
} else {
status = xdl_do_merge(&xe1, xscr1,
&xe2, xscr2,
xmp, result);
}
xdl_free_script(xscr1);
xdl_free_script(xscr2);
xdl_free_env(&xe1);
xdl_free_env(&xe2);
return status;
}
