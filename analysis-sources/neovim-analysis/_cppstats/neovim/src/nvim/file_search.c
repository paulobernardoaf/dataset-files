#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <limits.h>
#include "nvim/vim.h"
#include "nvim/eval.h"
#include "nvim/ascii.h"
#include "nvim/file_search.h"
#include "nvim/charset.h"
#include "nvim/fileio.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/option.h"
#include "nvim/os_unix.h"
#include "nvim/path.h"
#include "nvim/strings.h"
#include "nvim/tag.h"
#include "nvim/window.h"
#include "nvim/os/os.h"
#include "nvim/os/input.h"
#include "nvim/os/fs_defs.h"
static char_u *ff_expand_buffer = NULL; 
typedef struct ff_stack {
struct ff_stack *ffs_prev;
char_u *ffs_fix_path;
char_u *ffs_wc_path;
char_u **ffs_filearray;
int ffs_filearray_size;
char_u ffs_filearray_cur; 
int ffs_stage;
int ffs_level;
int ffs_star_star_empty;
} ff_stack_T;
typedef struct ff_visited {
struct ff_visited *ffv_next;
char_u *ffv_wc_path;
bool file_id_valid;
FileID file_id;
char_u ffv_fname[1]; 
} ff_visited_T;
typedef struct ff_visited_list_hdr {
struct ff_visited_list_hdr *ffvl_next;
char_u *ffvl_filename;
ff_visited_T *ffvl_visited_list;
} ff_visited_list_hdr_T;
#define FF_MAX_STAR_STAR_EXPAND ((char_u)30)
typedef struct ff_search_ctx_T {
ff_stack_T *ffsc_stack_ptr;
ff_visited_list_hdr_T *ffsc_visited_list;
ff_visited_list_hdr_T *ffsc_dir_visited_list;
ff_visited_list_hdr_T *ffsc_visited_lists_list;
ff_visited_list_hdr_T *ffsc_dir_visited_lists_list;
char_u *ffsc_file_to_search;
char_u *ffsc_start_dir;
char_u *ffsc_fix_path;
char_u *ffsc_wc_path;
int ffsc_level;
char_u **ffsc_stopdirs_v;
int ffsc_find_what;
int ffsc_tagfile;
} ff_search_ctx_T;
#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "file_search.c.generated.h"
#endif
static char_u e_pathtoolong[] = N_("E854: path too long for completion");
void *
vim_findfile_init (
char_u *path,
char_u *filename,
char_u *stopdirs,
int level,
int free_visited,
int find_what,
void *search_ctx_arg,
int tagfile, 
char_u *rel_fname 
)
{
char_u *wc_part;
ff_stack_T *sptr;
ff_search_ctx_T *search_ctx;
if (search_ctx_arg != NULL)
search_ctx = search_ctx_arg;
else {
search_ctx = xcalloc(1, sizeof(ff_search_ctx_T));
}
search_ctx->ffsc_find_what = find_what;
search_ctx->ffsc_tagfile = tagfile;
ff_clear(search_ctx);
if (free_visited == TRUE)
vim_findfile_free_visited(search_ctx);
else {
search_ctx->ffsc_visited_list = ff_get_visited_list(filename,
&search_ctx->ffsc_visited_lists_list);
if (search_ctx->ffsc_visited_list == NULL)
goto error_return;
search_ctx->ffsc_dir_visited_list = ff_get_visited_list(filename,
&search_ctx->ffsc_dir_visited_lists_list);
if (search_ctx->ffsc_dir_visited_list == NULL)
goto error_return;
}
if (ff_expand_buffer == NULL) {
ff_expand_buffer = xmalloc(MAXPATHL);
}
if (path[0] == '.'
&& (vim_ispathsep(path[1]) || path[1] == NUL)
&& (!tagfile || vim_strchr(p_cpo, CPO_DOTTAG) == NULL)
&& rel_fname != NULL) {
size_t len = (size_t)(path_tail(rel_fname) - rel_fname);
if (!vim_isAbsName(rel_fname) && len + 1 < MAXPATHL) {
STRLCPY(ff_expand_buffer, rel_fname, len + 1);
search_ctx->ffsc_start_dir = (char_u *)FullName_save((char *)ff_expand_buffer, FALSE);
} else
search_ctx->ffsc_start_dir = vim_strnsave(rel_fname, len);
if (*++path != NUL)
++path;
} else if (*path == NUL || !vim_isAbsName(path)) {
#if defined(BACKSLASH_IN_FILENAME)
if (*path != NUL && path[1] == ':') {
char_u drive[3];
drive[0] = path[0];
drive[1] = ':';
drive[2] = NUL;
if (vim_FullName((const char *)drive, (char *)ff_expand_buffer, MAXPATHL,
true)
== FAIL) {
goto error_return;
}
path += 2;
} else
#endif
if (os_dirname(ff_expand_buffer, MAXPATHL) == FAIL)
goto error_return;
search_ctx->ffsc_start_dir = vim_strsave(ff_expand_buffer);
#if defined(BACKSLASH_IN_FILENAME)
if ((*path == '/' || *path == '\\')
&& path[1] != path[0]
&& search_ctx->ffsc_start_dir[1] == ':')
search_ctx->ffsc_start_dir[2] = NUL;
#endif
}
if (stopdirs != NULL) {
char_u *walker = stopdirs;
while (*walker == ';')
walker++;
size_t dircount = 1;
search_ctx->ffsc_stopdirs_v = xmalloc(sizeof(char_u *));
do {
char_u *helper;
void *ptr;
helper = walker;
ptr = xrealloc(search_ctx->ffsc_stopdirs_v,
(dircount + 1) * sizeof(char_u *));
search_ctx->ffsc_stopdirs_v = ptr;
walker = vim_strchr(walker, ';');
if (walker) {
assert(walker - helper >= 0);
search_ctx->ffsc_stopdirs_v[dircount-1] =
vim_strnsave(helper, (size_t)(walker - helper));
walker++;
} else
search_ctx->ffsc_stopdirs_v[dircount-1] =
vim_strsave(helper);
dircount++;
} while (walker != NULL);
search_ctx->ffsc_stopdirs_v[dircount-1] = NULL;
}
search_ctx->ffsc_level = level;
wc_part = vim_strchr(path, '*');
if (wc_part != NULL) {
int64_t llevel;
int len;
char *errpt;
assert(wc_part - path >= 0);
search_ctx->ffsc_fix_path = vim_strnsave(path, (size_t)(wc_part - path));
len = 0;
while (*wc_part != NUL) {
if (len + 5 >= MAXPATHL) {
EMSG(_(e_pathtoolong));
break;
}
if (STRNCMP(wc_part, "**", 2) == 0) {
ff_expand_buffer[len++] = *wc_part++;
ff_expand_buffer[len++] = *wc_part++;
llevel = strtol((char *)wc_part, &errpt, 10);
if ((char_u *)errpt != wc_part && llevel > 0 && llevel < 255)
ff_expand_buffer[len++] = (char_u)llevel;
else if ((char_u *)errpt != wc_part && llevel == 0)
len -= 2;
else
ff_expand_buffer[len++] = FF_MAX_STAR_STAR_EXPAND;
wc_part = (char_u *)errpt;
if (*wc_part != NUL && !vim_ispathsep(*wc_part)) {
EMSG2(_(
"E343: Invalid path: '**[number]' must be at the end of the path or be followed by '%s'."),
PATHSEPSTR);
goto error_return;
}
} else
ff_expand_buffer[len++] = *wc_part++;
}
ff_expand_buffer[len] = NUL;
search_ctx->ffsc_wc_path = vim_strsave(ff_expand_buffer);
} else
search_ctx->ffsc_fix_path = vim_strsave(path);
if (search_ctx->ffsc_start_dir == NULL) {
search_ctx->ffsc_start_dir = vim_strsave(search_ctx->ffsc_fix_path);
search_ctx->ffsc_fix_path[0] = NUL;
}
if (STRLEN(search_ctx->ffsc_start_dir)
+ STRLEN(search_ctx->ffsc_fix_path) + 3 >= MAXPATHL) {
EMSG(_(e_pathtoolong));
goto error_return;
}
STRCPY(ff_expand_buffer, search_ctx->ffsc_start_dir);
add_pathsep((char *)ff_expand_buffer);
{
size_t eb_len = STRLEN(ff_expand_buffer);
char_u *buf = xmalloc(eb_len + STRLEN(search_ctx->ffsc_fix_path) + 1);
STRCPY(buf, ff_expand_buffer);
STRCPY(buf + eb_len, search_ctx->ffsc_fix_path);
if (os_isdir(buf)) {
STRCAT(ff_expand_buffer, search_ctx->ffsc_fix_path);
add_pathsep((char *)ff_expand_buffer);
} else {
char_u *p = path_tail(search_ctx->ffsc_fix_path);
char_u *wc_path = NULL;
char_u *temp = NULL;
int len = 0;
if (p > search_ctx->ffsc_fix_path) {
len = (int)(p - search_ctx->ffsc_fix_path) - 1;
STRNCAT(ff_expand_buffer, search_ctx->ffsc_fix_path, len);
add_pathsep((char *)ff_expand_buffer);
} else {
len = (int)STRLEN(search_ctx->ffsc_fix_path);
}
if (search_ctx->ffsc_wc_path != NULL) {
wc_path = vim_strsave(search_ctx->ffsc_wc_path);
temp = xmalloc(STRLEN(search_ctx->ffsc_wc_path)
+ STRLEN(search_ctx->ffsc_fix_path + len)
+ 1);
STRCPY(temp, search_ctx->ffsc_fix_path + len);
STRCAT(temp, search_ctx->ffsc_wc_path);
xfree(search_ctx->ffsc_wc_path);
xfree(wc_path);
search_ctx->ffsc_wc_path = temp;
}
}
xfree(buf);
}
sptr = ff_create_stack_element(ff_expand_buffer,
search_ctx->ffsc_wc_path,
level, 0);
ff_push(search_ctx, sptr);
search_ctx->ffsc_file_to_search = vim_strsave(filename);
return search_ctx;
error_return:
vim_findfile_cleanup(search_ctx);
return NULL;
}
char_u *vim_findfile_stopdir(char_u *buf)
{
char_u *r_ptr = buf;
while (*r_ptr != NUL && *r_ptr != ';') {
if (r_ptr[0] == '\\' && r_ptr[1] == ';') {
STRMOVE(r_ptr, r_ptr + 1);
r_ptr++;
}
r_ptr++;
}
if (*r_ptr == ';') {
*r_ptr = 0;
r_ptr++;
} else if (*r_ptr == NUL)
r_ptr = NULL;
return r_ptr;
}
void vim_findfile_cleanup(void *ctx)
{
if (ctx == NULL)
return;
vim_findfile_free_visited(ctx);
ff_clear(ctx);
xfree(ctx);
}
char_u *vim_findfile(void *search_ctx_arg)
{
char_u *file_path;
char_u *rest_of_wildcards;
char_u *path_end = NULL;
ff_stack_T *stackp = NULL;
size_t len;
char_u *p;
char_u *suf;
ff_search_ctx_T *search_ctx;
if (search_ctx_arg == NULL)
return NULL;
search_ctx = (ff_search_ctx_T *)search_ctx_arg;
file_path = xmalloc(MAXPATHL);
if (search_ctx->ffsc_start_dir != NULL)
path_end = &search_ctx->ffsc_start_dir[
STRLEN(search_ctx->ffsc_start_dir)];
for (;; ) {
for (;; ) {
os_breakcheck();
if (got_int)
break;
stackp = ff_pop(search_ctx);
if (stackp == NULL)
break;
if (stackp->ffs_filearray == NULL
&& ff_check_visited(&search_ctx->ffsc_dir_visited_list
->ffvl_visited_list,
stackp->ffs_fix_path
, stackp->ffs_wc_path
) == FAIL) {
#if defined(FF_VERBOSE)
if (p_verbose >= 5) {
verbose_enter_scroll();
smsg("Already Searched: %s (%s)",
stackp->ffs_fix_path, stackp->ffs_wc_path);
msg_puts("\n"); 
verbose_leave_scroll();
}
#endif
ff_free_stack_element(stackp);
continue;
}
#if defined(FF_VERBOSE)
else if (p_verbose >= 5) {
verbose_enter_scroll();
smsg("Searching: %s (%s)",
stackp->ffs_fix_path, stackp->ffs_wc_path);
msg_puts("\n"); 
verbose_leave_scroll();
}
#endif
if (stackp->ffs_level <= 0) {
ff_free_stack_element(stackp);
continue;
}
file_path[0] = NUL;
if (stackp->ffs_filearray == NULL) {
char_u *dirptrs[2];
dirptrs[0] = file_path;
dirptrs[1] = NULL;
if (!vim_isAbsName(stackp->ffs_fix_path)
&& search_ctx->ffsc_start_dir) {
if (STRLEN(search_ctx->ffsc_start_dir) + 1 >= MAXPATHL) {
ff_free_stack_element(stackp);
goto fail;
}
STRCPY(file_path, search_ctx->ffsc_start_dir);
if (!add_pathsep((char *)file_path)) {
ff_free_stack_element(stackp);
goto fail;
}
}
if (STRLEN(file_path) + STRLEN(stackp->ffs_fix_path) + 1 >= MAXPATHL) {
ff_free_stack_element(stackp);
goto fail;
}
STRCAT(file_path, stackp->ffs_fix_path);
if (!add_pathsep((char *)file_path)) {
ff_free_stack_element(stackp);
goto fail;
}
rest_of_wildcards = stackp->ffs_wc_path;
if (*rest_of_wildcards != NUL) {
len = STRLEN(file_path);
if (STRNCMP(rest_of_wildcards, "**", 2) == 0) {
p = rest_of_wildcards + 2;
if (*p > 0) {
(*p)--;
if (len + 1 >= MAXPATHL) {
ff_free_stack_element(stackp);
goto fail;
}
file_path[len++] = '*';
}
if (*p == 0) {
STRMOVE(rest_of_wildcards, rest_of_wildcards + 3);
} else
rest_of_wildcards += 3;
if (stackp->ffs_star_star_empty == 0) {
stackp->ffs_star_star_empty = 1;
dirptrs[1] = stackp->ffs_fix_path;
}
}
while (*rest_of_wildcards
&& !vim_ispathsep(*rest_of_wildcards)) {
if (len + 1 >= MAXPATHL) {
ff_free_stack_element(stackp);
goto fail;
}
file_path[len++] = *rest_of_wildcards++;
}
file_path[len] = NUL;
if (vim_ispathsep(*rest_of_wildcards))
rest_of_wildcards++;
}
if (path_with_url((char *)dirptrs[0])) {
stackp->ffs_filearray = xmalloc(sizeof(char *));
stackp->ffs_filearray[0] = vim_strsave(dirptrs[0]);
stackp->ffs_filearray_size = 1;
} else
expand_wildcards((dirptrs[1] == NULL) ? 1 : 2, dirptrs,
&stackp->ffs_filearray_size,
&stackp->ffs_filearray,
EW_DIR|EW_ADDSLASH|EW_SILENT|EW_NOTWILD);
stackp->ffs_filearray_cur = 0;
stackp->ffs_stage = 0;
} else
rest_of_wildcards = &stackp->ffs_wc_path[
STRLEN(stackp->ffs_wc_path)];
if (stackp->ffs_stage == 0) {
if (*rest_of_wildcards == NUL) {
for (int i = stackp->ffs_filearray_cur;
i < stackp->ffs_filearray_size; ++i) {
if (!path_with_url((char *)stackp->ffs_filearray[i])
&& !os_isdir(stackp->ffs_filearray[i]))
continue; 
if (STRLEN(stackp->ffs_filearray[i]) + 1
+ STRLEN(search_ctx->ffsc_file_to_search) >= MAXPATHL) {
ff_free_stack_element(stackp);
goto fail;
}
STRCPY(file_path, stackp->ffs_filearray[i]);
if (!add_pathsep((char *)file_path)) {
ff_free_stack_element(stackp);
goto fail;
}
STRCAT(file_path, search_ctx->ffsc_file_to_search);
len = STRLEN(file_path);
if (search_ctx->ffsc_tagfile)
suf = (char_u *)"";
else
suf = curbuf->b_p_sua;
for (;; ) {
if ((path_with_url((char *)file_path)
|| (os_path_exists(file_path)
&& (search_ctx->ffsc_find_what
== FINDFILE_BOTH
|| ((search_ctx->ffsc_find_what
== FINDFILE_DIR)
== os_isdir(file_path)))))
#if !defined(FF_VERBOSE)
&& (ff_check_visited(
&search_ctx->ffsc_visited_list->ffvl_visited_list,
file_path
, (char_u *)""
) == OK)
#endif
) {
#if defined(FF_VERBOSE)
if (ff_check_visited(
&search_ctx->ffsc_visited_list->ffvl_visited_list,
file_path
, (char_u *)""
) == FAIL) {
if (p_verbose >= 5) {
verbose_enter_scroll();
smsg("Already: %s", file_path);
msg_puts("\n"); 
verbose_leave_scroll();
}
continue;
}
#endif
assert(i < UCHAR_MAX - 1);
stackp->ffs_filearray_cur = (char_u)(i + 1);
ff_push(search_ctx, stackp);
if (!path_with_url((char *)file_path))
simplify_filename(file_path);
if (os_dirname(ff_expand_buffer, MAXPATHL)
== OK) {
p = path_shorten_fname(file_path,
ff_expand_buffer);
if (p != NULL)
STRMOVE(file_path, p);
}
#if defined(FF_VERBOSE)
if (p_verbose >= 5) {
verbose_enter_scroll();
smsg("HIT: %s", file_path);
msg_puts("\n"); 
verbose_leave_scroll();
}
#endif
return file_path;
}
if (*suf == NUL)
break;
assert(MAXPATHL >= len);
copy_option_part(&suf, file_path + len,
MAXPATHL - len, ",");
}
}
} else {
for (int i = stackp->ffs_filearray_cur;
i < stackp->ffs_filearray_size; ++i) {
if (!os_isdir(stackp->ffs_filearray[i]))
continue; 
ff_push(search_ctx,
ff_create_stack_element(
stackp->ffs_filearray[i],
rest_of_wildcards,
stackp->ffs_level - 1, 0));
}
}
stackp->ffs_filearray_cur = 0;
stackp->ffs_stage = 1;
}
if (STRNCMP(stackp->ffs_wc_path, "**", 2) == 0) {
for (int i = stackp->ffs_filearray_cur;
i < stackp->ffs_filearray_size; ++i) {
if (fnamecmp(stackp->ffs_filearray[i],
stackp->ffs_fix_path) == 0)
continue; 
if (!os_isdir(stackp->ffs_filearray[i]))
continue; 
ff_push(search_ctx,
ff_create_stack_element(stackp->ffs_filearray[i],
stackp->ffs_wc_path, stackp->ffs_level - 1, 1));
}
}
ff_free_stack_element(stackp);
}
if (search_ctx->ffsc_start_dir
&& search_ctx->ffsc_stopdirs_v != NULL && !got_int) {
ff_stack_T *sptr;
if (ff_path_in_stoplist(search_ctx->ffsc_start_dir,
(int)(path_end - search_ctx->ffsc_start_dir),
search_ctx->ffsc_stopdirs_v) == TRUE)
break;
while (path_end > search_ctx->ffsc_start_dir
&& vim_ispathsep(*path_end))
path_end--;
while (path_end > search_ctx->ffsc_start_dir
&& !vim_ispathsep(path_end[-1]))
path_end--;
*path_end = 0;
path_end--;
if (*search_ctx->ffsc_start_dir == 0)
break;
if (STRLEN(search_ctx->ffsc_start_dir) + 1
+ STRLEN(search_ctx->ffsc_fix_path) >= MAXPATHL) {
goto fail;
}
STRCPY(file_path, search_ctx->ffsc_start_dir);
if (!add_pathsep((char *)file_path)) {
goto fail;
}
STRCAT(file_path, search_ctx->ffsc_fix_path);
sptr = ff_create_stack_element(file_path,
search_ctx->ffsc_wc_path, search_ctx->ffsc_level, 0);
ff_push(search_ctx, sptr);
} else
break;
}
fail:
xfree(file_path);
return NULL;
}
void vim_findfile_free_visited(void *search_ctx_arg)
{
ff_search_ctx_T *search_ctx;
if (search_ctx_arg == NULL)
return;
search_ctx = (ff_search_ctx_T *)search_ctx_arg;
vim_findfile_free_visited_list(&search_ctx->ffsc_visited_lists_list);
vim_findfile_free_visited_list(&search_ctx->ffsc_dir_visited_lists_list);
}
static void vim_findfile_free_visited_list(ff_visited_list_hdr_T **list_headp)
{
ff_visited_list_hdr_T *vp;
while (*list_headp != NULL) {
vp = (*list_headp)->ffvl_next;
ff_free_visited_list((*list_headp)->ffvl_visited_list);
xfree((*list_headp)->ffvl_filename);
xfree(*list_headp);
*list_headp = vp;
}
*list_headp = NULL;
}
static void ff_free_visited_list(ff_visited_T *vl)
{
ff_visited_T *vp;
while (vl != NULL) {
vp = vl->ffv_next;
xfree(vl->ffv_wc_path);
xfree(vl);
vl = vp;
}
vl = NULL;
}
static ff_visited_list_hdr_T *ff_get_visited_list(char_u *filename, ff_visited_list_hdr_T **list_headp)
{
ff_visited_list_hdr_T *retptr = NULL;
if (*list_headp != NULL) {
retptr = *list_headp;
while (retptr != NULL) {
if (fnamecmp(filename, retptr->ffvl_filename) == 0) {
#if defined(FF_VERBOSE)
if (p_verbose >= 5) {
verbose_enter_scroll();
smsg("ff_get_visited_list: FOUND list for %s", filename);
msg_puts("\n"); 
verbose_leave_scroll();
}
#endif
return retptr;
}
retptr = retptr->ffvl_next;
}
}
#if defined(FF_VERBOSE)
if (p_verbose >= 5) {
verbose_enter_scroll();
smsg("ff_get_visited_list: new list for %s", filename);
msg_puts("\n"); 
verbose_leave_scroll();
}
#endif
retptr = xmalloc(sizeof(*retptr));
retptr->ffvl_visited_list = NULL;
retptr->ffvl_filename = vim_strsave(filename);
retptr->ffvl_next = *list_headp;
*list_headp = retptr;
return retptr;
}
static bool ff_wc_equal(char_u *s1, char_u *s2)
{
int i, j;
int c1 = NUL;
int c2 = NUL;
int prev1 = NUL;
int prev2 = NUL;
if (s1 == s2) {
return true;
}
if (s1 == NULL || s2 == NULL) {
return false;
}
for (i = 0, j = 0; s1[i] != NUL && s2[j] != NUL;) {
c1 = PTR2CHAR(s1 + i);
c2 = PTR2CHAR(s2 + j);
if ((p_fic ? mb_tolower(c1) != mb_tolower(c2) : c1 != c2)
&& (prev1 != '*' || prev2 != '*')) {
return false;
}
prev2 = prev1;
prev1 = c1;
i += utfc_ptr2len(s1 + i);
j += utfc_ptr2len(s2 + j);
}
return s1[i] == s2[j];
}
static int ff_check_visited(ff_visited_T **visited_list, char_u *fname, char_u *wc_path)
{
ff_visited_T *vp;
bool url = false;
FileID file_id;
if (path_with_url((char *)fname)) {
STRLCPY(ff_expand_buffer, fname, MAXPATHL);
url = true;
} else {
ff_expand_buffer[0] = NUL;
if (!os_fileid((char *)fname, &file_id)) {
return FAIL;
}
}
for (vp = *visited_list; vp != NULL; vp = vp->ffv_next) {
if ((url && fnamecmp(vp->ffv_fname, ff_expand_buffer) == 0)
|| (!url && vp->file_id_valid
&& os_fileid_equal(&(vp->file_id), &file_id))) {
if (ff_wc_equal(vp->ffv_wc_path, wc_path)) {
return FAIL;
}
}
}
vp = xmalloc(sizeof(ff_visited_T) + STRLEN(ff_expand_buffer));
if (!url) {
vp->file_id_valid = true;
vp->file_id = file_id;
vp->ffv_fname[0] = NUL;
} else {
vp->file_id_valid = false;
STRCPY(vp->ffv_fname, ff_expand_buffer);
}
if (wc_path != NULL)
vp->ffv_wc_path = vim_strsave(wc_path);
else
vp->ffv_wc_path = NULL;
vp->ffv_next = *visited_list;
*visited_list = vp;
return OK;
}
static ff_stack_T *ff_create_stack_element(char_u *fix_part, char_u *wc_part, int level, int star_star_empty)
{
ff_stack_T *new = xmalloc(sizeof(ff_stack_T));
new->ffs_prev = NULL;
new->ffs_filearray = NULL;
new->ffs_filearray_size = 0;
new->ffs_filearray_cur = 0;
new->ffs_stage = 0;
new->ffs_level = level;
new->ffs_star_star_empty = star_star_empty;
if (fix_part == NULL)
fix_part = (char_u *)"";
new->ffs_fix_path = vim_strsave(fix_part);
if (wc_part == NULL)
wc_part = (char_u *)"";
new->ffs_wc_path = vim_strsave(wc_part);
return new;
}
static void ff_push(ff_search_ctx_T *search_ctx, ff_stack_T *stack_ptr)
{
if (stack_ptr != NULL) {
stack_ptr->ffs_prev = search_ctx->ffsc_stack_ptr;
search_ctx->ffsc_stack_ptr = stack_ptr;
}
}
static ff_stack_T *ff_pop(ff_search_ctx_T *search_ctx)
{
ff_stack_T *sptr;
sptr = search_ctx->ffsc_stack_ptr;
if (search_ctx->ffsc_stack_ptr != NULL)
search_ctx->ffsc_stack_ptr = search_ctx->ffsc_stack_ptr->ffs_prev;
return sptr;
}
static void ff_free_stack_element(ff_stack_T *const stack_ptr)
{
if (stack_ptr == NULL) {
return;
}
xfree(stack_ptr->ffs_fix_path);
xfree(stack_ptr->ffs_wc_path);
if (stack_ptr->ffs_filearray != NULL) {
FreeWild(stack_ptr->ffs_filearray_size, stack_ptr->ffs_filearray);
}
xfree(stack_ptr);
}
static void ff_clear(ff_search_ctx_T *search_ctx)
{
ff_stack_T *sptr;
while ((sptr = ff_pop(search_ctx)) != NULL)
ff_free_stack_element(sptr);
xfree(search_ctx->ffsc_file_to_search);
xfree(search_ctx->ffsc_start_dir);
xfree(search_ctx->ffsc_fix_path);
xfree(search_ctx->ffsc_wc_path);
if (search_ctx->ffsc_stopdirs_v != NULL) {
int i = 0;
while (search_ctx->ffsc_stopdirs_v[i] != NULL) {
xfree(search_ctx->ffsc_stopdirs_v[i]);
i++;
}
xfree(search_ctx->ffsc_stopdirs_v);
}
search_ctx->ffsc_stopdirs_v = NULL;
search_ctx->ffsc_file_to_search = NULL;
search_ctx->ffsc_start_dir = NULL;
search_ctx->ffsc_fix_path = NULL;
search_ctx->ffsc_wc_path = NULL;
search_ctx->ffsc_level = 0;
}
static int ff_path_in_stoplist(char_u *path, int path_len, char_u **stopdirs_v)
{
int i = 0;
while (path_len > 1 && vim_ispathsep(path[path_len - 1]))
path_len--;
if (path_len == 0)
return TRUE;
for (i = 0; stopdirs_v[i] != NULL; i++) {
if ((int)STRLEN(stopdirs_v[i]) > path_len) {
if (fnamencmp(stopdirs_v[i], path, path_len) == 0
&& vim_ispathsep(stopdirs_v[i][path_len]))
return TRUE;
} else {
if (fnamecmp(stopdirs_v[i], path) == 0)
return TRUE;
}
}
return FALSE;
}
char_u *
find_file_in_path (
char_u *ptr, 
size_t len, 
int options,
int first, 
char_u *rel_fname 
)
{
return find_file_in_path_option(ptr, len, options, first,
(*curbuf->b_p_path == NUL
? p_path
: curbuf->b_p_path),
FINDFILE_BOTH, rel_fname, curbuf->b_p_sua);
}
static char_u *ff_file_to_find = NULL;
static void *fdip_search_ctx = NULL;
#if defined(EXITFREE)
void free_findfile(void)
{
xfree(ff_file_to_find);
vim_findfile_cleanup(fdip_search_ctx);
xfree(ff_expand_buffer);
}
#endif
char_u *
find_directory_in_path (
char_u *ptr, 
size_t len, 
int options,
char_u *rel_fname 
)
{
return find_file_in_path_option(ptr, len, options, TRUE, p_cdpath,
FINDFILE_DIR, rel_fname, (char_u *)"");
}
char_u *
find_file_in_path_option (
char_u *ptr, 
size_t len, 
int options,
int first, 
char_u *path_option, 
int find_what, 
char_u *rel_fname, 
char_u *suffixes 
)
{
static char_u *dir;
static int did_findfile_init = FALSE;
char_u save_char;
char_u *file_name = NULL;
char_u *buf = NULL;
int rel_to_curdir;
if (rel_fname != NULL && path_with_url((const char *)rel_fname)) {
rel_fname = NULL;
}
if (first == TRUE) {
save_char = ptr[len];
ptr[len] = NUL;
expand_env_esc(ptr, NameBuff, MAXPATHL, false, true, NULL);
ptr[len] = save_char;
xfree(ff_file_to_find);
ff_file_to_find = vim_strsave(NameBuff);
if (options & FNAME_UNESC) {
for (ptr = ff_file_to_find; *ptr != NUL; ++ptr) {
if (ptr[0] == '\\' && ptr[1] == ' ') {
memmove(ptr, ptr + 1, STRLEN(ptr));
}
}
}
}
rel_to_curdir = (ff_file_to_find[0] == '.'
&& (ff_file_to_find[1] == NUL
|| vim_ispathsep(ff_file_to_find[1])
|| (ff_file_to_find[1] == '.'
&& (ff_file_to_find[2] == NUL
|| vim_ispathsep(ff_file_to_find[2])))));
if (vim_isAbsName(ff_file_to_find)
|| rel_to_curdir
#if defined(WIN32)
|| vim_ispathsep(ff_file_to_find[0])
|| (ff_file_to_find[0] != NUL && ff_file_to_find[1] == ':')
#endif
) {
if (first == TRUE) {
if (path_with_url((char *)ff_file_to_find)) {
file_name = vim_strsave(ff_file_to_find);
goto theend;
}
for (int run = 1; run <= 2; ++run) {
size_t l = STRLEN(ff_file_to_find);
if (run == 1
&& rel_to_curdir
&& (options & FNAME_REL)
&& rel_fname != NULL
&& STRLEN(rel_fname) + l < MAXPATHL) {
STRCPY(NameBuff, rel_fname);
STRCPY(path_tail(NameBuff), ff_file_to_find);
l = STRLEN(NameBuff);
} else {
STRCPY(NameBuff, ff_file_to_find);
run = 2;
}
buf = suffixes;
for (;; ) {
if (
(os_path_exists(NameBuff)
&& (find_what == FINDFILE_BOTH
|| ((find_what == FINDFILE_DIR)
== os_isdir(NameBuff))))) {
file_name = vim_strsave(NameBuff);
goto theend;
}
if (*buf == NUL)
break;
assert(MAXPATHL >= l);
copy_option_part(&buf, NameBuff + l, MAXPATHL - l, ",");
}
}
}
} else {
if (first == TRUE) {
vim_findfile_free_visited(fdip_search_ctx);
dir = path_option;
did_findfile_init = FALSE;
}
for (;; ) {
if (did_findfile_init) {
file_name = vim_findfile(fdip_search_ctx);
if (file_name != NULL)
break;
did_findfile_init = FALSE;
} else {
char_u *r_ptr;
if (dir == NULL || *dir == NUL) {
vim_findfile_cleanup(fdip_search_ctx);
fdip_search_ctx = NULL;
break;
}
buf = xmalloc(MAXPATHL);
buf[0] = 0;
copy_option_part(&dir, buf, MAXPATHL, " ,");
r_ptr = vim_findfile_stopdir(buf);
fdip_search_ctx = vim_findfile_init(buf, ff_file_to_find,
r_ptr, 100, FALSE, find_what,
fdip_search_ctx, FALSE, rel_fname);
if (fdip_search_ctx != NULL)
did_findfile_init = TRUE;
xfree(buf);
}
}
}
if (file_name == NULL && (options & FNAME_MESS)) {
if (first == TRUE) {
if (find_what == FINDFILE_DIR)
EMSG2(_("E344: Can't find directory \"%s\" in cdpath"),
ff_file_to_find);
else
EMSG2(_("E345: Can't find file \"%s\" in path"),
ff_file_to_find);
} else {
if (find_what == FINDFILE_DIR)
EMSG2(_("E346: No more directory \"%s\" found in cdpath"),
ff_file_to_find);
else
EMSG2(_("E347: No more file \"%s\" found in path"),
ff_file_to_find);
}
}
theend:
return file_name;
}
void do_autocmd_dirchanged(char *new_dir, CdScope scope)
{
static bool recursive = false;
if (recursive || !has_event(EVENT_DIRCHANGED)) {
return;
}
recursive = true;
dict_T *dict = get_vim_var_dict(VV_EVENT);
char buf[8];
switch (scope) {
case kCdScopeGlobal: {
snprintf(buf, sizeof(buf), "global");
break;
}
case kCdScopeTab: {
snprintf(buf, sizeof(buf), "tab");
break;
}
case kCdScopeWindow: {
snprintf(buf, sizeof(buf), "window");
break;
}
case kCdScopeInvalid: {
assert(false);
}
}
tv_dict_add_str(dict, S_LEN("scope"), buf); 
tv_dict_add_str(dict, S_LEN("cwd"), new_dir);
tv_dict_set_keys_readonly(dict);
apply_autocmds(EVENT_DIRCHANGED, (char_u *)buf, (char_u *)new_dir, false,
curbuf);
tv_dict_clear(dict);
recursive = false;
}
int vim_chdirfile(char_u *fname)
{
char dir[MAXPATHL];
STRLCPY(dir, fname, MAXPATHL);
*path_tail_with_sep((char_u *)dir) = NUL;
if (os_dirname(NameBuff, sizeof(NameBuff)) != OK) {
NameBuff[0] = NUL;
}
if (os_chdir(dir) != 0) {
return FAIL;
}
#if defined(BACKSLASH_IN_FILENAME)
slash_adjust((char_u *)dir);
#endif
if (!strequal(dir, (char *)NameBuff)) {
do_autocmd_dirchanged(dir, kCdScopeWindow);
}
return OK;
}
int vim_chdir(char_u *new_dir)
{
char_u *dir_name = find_directory_in_path(new_dir, STRLEN(new_dir),
FNAME_MESS, curbuf->b_ffname);
if (dir_name == NULL) {
return -1;
}
int r = os_chdir((char *)dir_name);
xfree(dir_name);
return r;
}
