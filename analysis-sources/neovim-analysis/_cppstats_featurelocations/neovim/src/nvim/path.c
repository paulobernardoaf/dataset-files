


#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/path.h"
#include "nvim/charset.h"
#include "nvim/eval.h"
#include "nvim/ex_docmd.h"
#include "nvim/ex_getln.h"
#include "nvim/fileio.h"
#include "nvim/file_search.h"
#include "nvim/garray.h"
#include "nvim/memfile.h"
#include "nvim/memline.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/misc1.h"
#include "nvim/option.h"
#include "nvim/os/os.h"
#include "nvim/os/shell.h"
#include "nvim/os_unix.h"
#include "nvim/quickfix.h"
#include "nvim/regexp.h"
#include "nvim/screen.h"
#include "nvim/strings.h"
#include "nvim/tag.h"
#include "nvim/types.h"
#include "nvim/os/input.h"
#include "nvim/window.h"

#define URL_SLASH 1 
#define URL_BACKSLASH 2 

#if defined(gen_expand_wildcards)
#undef gen_expand_wildcards
#endif

#if defined(INCLUDE_GENERATED_DECLARATIONS)
#include "path.c.generated.h"
#endif









FileComparison path_full_compare(char_u *const s1, char_u *const s2,
const bool checkname, const bool expandenv)
{
assert(s1 && s2);
char_u exp1[MAXPATHL];
char_u full1[MAXPATHL];
char_u full2[MAXPATHL];
FileID file_id_1, file_id_2;

if (expandenv) {
expand_env(s1, exp1, MAXPATHL);
} else {
xstrlcpy((char *)exp1, (const char *)s1, MAXPATHL - 1);
}
bool id_ok_1 = os_fileid((char *)exp1, &file_id_1);
bool id_ok_2 = os_fileid((char *)s2, &file_id_2);
if (!id_ok_1 && !id_ok_2) {

if (checkname) {
vim_FullName((char *)exp1, (char *)full1, MAXPATHL, FALSE);
vim_FullName((char *)s2, (char *)full2, MAXPATHL, FALSE);
if (fnamecmp(full1, full2) == 0) {
return kEqualFileNames;
}
}
return kBothFilesMissing;
}
if (!id_ok_1 || !id_ok_2) {
return kOneFileMissing;
}
if (os_fileid_equal(&file_id_1, &file_id_2)) {
return kEqualFiles;
}
return kDifferentFiles;
}





char_u *path_tail(const char_u *fname)
FUNC_ATTR_NONNULL_RET
{
if (fname == NULL) {
return (char_u *)"";
}

const char_u *tail = get_past_head(fname);
const char_u *p = tail;

while (*p != NUL) {
if (vim_ispathsep_nocolon(*p)) {
tail = p + 1;
}
MB_PTR_ADV(p);
}
return (char_u *)tail;
}










char_u *path_tail_with_sep(char_u *fname)
{
assert(fname != NULL);


char_u *past_head = get_past_head(fname);
char_u *tail = path_tail(fname);
while (tail > past_head && after_pathsep((char *)fname, (char *)tail)) {
tail--;
}
return tail;
}










const char_u *invocation_path_tail(const char_u *invocation, size_t *len)
FUNC_ATTR_NONNULL_RET FUNC_ATTR_NONNULL_ARG(1)
{
const char_u *tail = get_past_head((char_u *) invocation);
const char_u *p = tail;
while (*p != NUL && *p != ' ') {
bool was_sep = vim_ispathsep_nocolon(*p);
MB_PTR_ADV(p);
if (was_sep) {
tail = p; 
}
}

if (len != NULL) {
*len = (size_t)(p - tail);
}

return tail;
}






const char *path_next_component(const char *fname)
{
assert(fname != NULL);
while (*fname != NUL && !vim_ispathsep(*fname)) {
MB_PTR_ADV(fname);
}
if (*fname != NUL) {
fname++;
}
return fname;
}




char_u *get_past_head(const char_u *path)
{
const char_u *retval = path;

#if defined(WIN32)

if (isalpha(path[0]) && path[1] == ':') {
retval = path + 2;
}
#endif

while (vim_ispathsep(*retval)) {
++retval;
}

return (char_u *)retval;
}





int vim_ispathsep(int c)
{
#if defined(UNIX)
return c == '/'; 
#else
#if defined(BACKSLASH_IN_FILENAME)
return c == ':' || c == '/' || c == '\\';
#else
return c == ':' || c == '/';
#endif
#endif
}




int vim_ispathsep_nocolon(int c)
{
return vim_ispathsep(c)
#if defined(BACKSLASH_IN_FILENAME)
&& c != ':'
#endif
;
}




int vim_ispathlistsep(int c)
{
#if defined(UNIX)
return c == ':';
#else
return c == ';'; 
#endif
}





char_u *shorten_dir(char_u *str)
{
char_u *tail = path_tail(str);
char_u *d = str;
bool skip = false;
for (char_u *s = str;; ++s) {
if (s >= tail) { 
*d++ = *s;
if (*s == NUL)
break;
} else if (vim_ispathsep(*s)) { 
*d++ = *s;
skip = false;
} else if (!skip) {
*d++ = *s; 
if (*s != '~' && *s != '.') 
skip = true;
if (has_mbyte) {
int l = mb_ptr2len(s);
while (--l > 0)
*d++ = *++s;
}
}
}
return str;
}






bool dir_of_file_exists(char_u *fname)
{
char_u *p = path_tail_with_sep(fname);
if (p == fname) {
return true;
}
char_u c = *p;
*p = NUL;
bool retval = os_isdir(fname);
*p = c;
return retval;
}









int path_fnamecmp(const char *fname1, const char *fname2)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
#if defined(BACKSLASH_IN_FILENAME)
const size_t len1 = strlen(fname1);
const size_t len2 = strlen(fname2);
return path_fnamencmp(fname1, fname2, MAX(len1, len2));
#else
return mb_strcmp_ic((bool)p_fic, fname1, fname2);
#endif
}










int path_fnamencmp(const char *const fname1, const char *const fname2,
size_t len)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT
{
#if defined(BACKSLASH_IN_FILENAME)
int c1 = NUL;
int c2 = NUL;

const char *p1 = fname1;
const char *p2 = fname2;
while (len > 0) {
c1 = PTR2CHAR((const char_u *)p1);
c2 = PTR2CHAR((const char_u *)p2);
if ((c1 == NUL || c2 == NUL
|| (!((c1 == '/' || c1 == '\\') && (c2 == '\\' || c2 == '/'))))
&& (p_fic ? (c1 != c2 && CH_FOLD(c1) != CH_FOLD(c2)) : c1 != c2)) {
break;
}
len -= (size_t)utfc_ptr2len((const char_u *)p1);
p1 += utfc_ptr2len((const char_u *)p1);
p2 += utfc_ptr2len((const char_u *)p2);
}
return c1 - c2;
#else
if (p_fic) {
return mb_strnicmp((const char_u *)fname1, (const char_u *)fname2, len);
}
return strncmp(fname1, fname2, len);
#endif
}











static inline char *do_concat_fnames(char *fname1, const size_t len1,
const char *fname2, const size_t len2,
const bool sep)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
if (sep && *fname1 && !after_pathsep(fname1, fname1 + len1)) {
fname1[len1] = PATHSEP;
memmove(fname1 + len1 + 1, fname2, len2 + 1);
} else {
memmove(fname1 + len1, fname2, len2 + 1);
}

return fname1;
}










char *concat_fnames(const char *fname1, const char *fname2, bool sep)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
const size_t len1 = strlen(fname1);
const size_t len2 = strlen(fname2);
char *dest = xmalloc(len1 + len2 + 3);
memmove(dest, fname1, len1 + 1);
return do_concat_fnames(dest, len1, fname2, len2, sep);
}












char *concat_fnames_realloc(char *fname1, const char *fname2, bool sep)
FUNC_ATTR_NONNULL_ALL FUNC_ATTR_NONNULL_RET
{
const size_t len1 = strlen(fname1);
const size_t len2 = strlen(fname2);
return do_concat_fnames(xrealloc(fname1, len1 + len2 + 3), len1,
fname2, len2, sep);
}





bool add_pathsep(char *p)
FUNC_ATTR_NONNULL_ALL
{
const size_t len = strlen(p);
if (*p != NUL && !after_pathsep(p, p + len)) {
const size_t pathsep_len = sizeof(PATHSEPSTR);
if (len > MAXPATHL - pathsep_len) {
return false;
}
memcpy(p + len, PATHSEPSTR, pathsep_len);
}
return true;
}








char *FullName_save(const char *fname, bool force)
FUNC_ATTR_MALLOC
{
if (fname == NULL) {
return NULL;
}

char *buf = xmalloc(MAXPATHL);
if (vim_FullName(fname, buf, MAXPATHL, force) == FAIL) {
xfree(buf);
return xstrdup(fname);
}
return buf;
}




char *save_abs_path(const char *name)
FUNC_ATTR_MALLOC FUNC_ATTR_NONNULL_ALL
{
if (!path_is_absolute((char_u *)name)) {
return FullName_save(name, true);
}
return (char *)vim_strsave((char_u *)name);
}





bool path_has_wildcard(const char_u *p)
FUNC_ATTR_NONNULL_ALL
{
for (; *p; MB_PTR_ADV(p)) {
#if defined(UNIX)
if (p[0] == '\\' && p[1] != NUL) {
p++;
continue;
}

const char *wildcards = "*?[{`'$";
#else

const char *wildcards = "?*$[`";
#endif
if (vim_strchr((char_u *)wildcards, *p) != NULL
|| (p[0] == '~' && p[1] != NUL)) {
return true;
}
}
return false;
}




static int pstrcmp(const void *a, const void *b)
{
return pathcmp(*(char **)a, *(char **)b, -1);
}





bool path_has_exp_wildcard(const char_u *p)
FUNC_ATTR_NONNULL_ALL
{
for (; *p != NUL; MB_PTR_ADV(p)) {
#if defined(UNIX)
if (p[0] == '\\' && p[1] != NUL) {
p++;
continue;
}

const char *wildcards = "*?[{";
#else
const char *wildcards = "*?["; 
#endif
if (vim_strchr((char_u *) wildcards, *p) != NULL) {
return true;
}
}
return false;
}












static size_t path_expand(garray_T *gap, const char_u *path, int flags)
FUNC_ATTR_NONNULL_ALL
{
return do_path_expand(gap, path, 0, flags, false);
}

static const char *scandir_next_with_dots(Directory *dir)
{
static int count = 0;
if (dir == NULL) { 
count = 0;
return NULL;
}

count += 1;
if (count == 1 || count == 2) {
return (count == 1) ? "." : "..";
}
return os_scandir_next(dir);
}




static size_t do_path_expand(garray_T *gap, const char_u *path,
size_t wildoff, int flags, bool didstar)
FUNC_ATTR_NONNULL_ALL
{
int start_len = gap->ga_len;
size_t len;
bool starstar = false;
static int stardepth = 0; 


if (stardepth > 0) {
os_breakcheck();
if (got_int)
return 0;
}



char_u *buf = xmalloc(MAXPATHL);




char_u *p = buf;
char_u *s = buf;
char_u *e = NULL;
const char_u *path_end = path;
while (*path_end != NUL) {


if (path_end >= path + wildoff && rem_backslash(path_end)) {
*p++ = *path_end++;
} else if (vim_ispathsep_nocolon(*path_end)) {
if (e != NULL) {
break;
}
s = p + 1;
} else if (path_end >= path + wildoff
&& (vim_strchr((char_u *)"*?[{~$", *path_end) != NULL
#if !defined(WIN32)
|| (!p_fic && (flags & EW_ICASE)
&& isalpha(PTR2CHAR(path_end)))
#endif
)) {
e = p;
}
if (has_mbyte) {
len = (size_t)(*mb_ptr2len)(path_end);
memcpy(p, path_end, len);
p += len;
path_end += len;
} else
*p++ = *path_end++;
}
e = p;
*e = NUL;




for (p = buf + wildoff; p < s; ++p)
if (rem_backslash(p)) {
STRMOVE(p, p + 1);
--e;
--s;
}


for (p = s; p < e; ++p)
if (p[0] == '*' && p[1] == '*')
starstar = true;


int starts_with_dot = *s == '.';
char_u *pat = file_pat_to_reg_pat(s, e, NULL, false);
if (pat == NULL) {
xfree(buf);
return 0;
}


regmatch_T regmatch;
#if defined(UNIX)

regmatch.rm_ic = (flags & EW_ICASE) || p_fic;
#else
regmatch.rm_ic = true; 
#endif
if (flags & (EW_NOERROR | EW_NOTWILD))
++emsg_silent;
regmatch.regprog = vim_regcomp(pat, RE_MAGIC);
if (flags & (EW_NOERROR | EW_NOTWILD))
--emsg_silent;
xfree(pat);

if (regmatch.regprog == NULL && (flags & EW_NOTWILD) == 0) {
xfree(buf);
return 0;
}



if (!didstar && stardepth < 100 && starstar && e - s == 2
&& *path_end == '/') {
STRCPY(s, path_end + 1);
stardepth++;
(void)do_path_expand(gap, buf, (size_t)(s - buf), flags, true);
stardepth--;
}
*s = NUL;

Directory dir;
char *dirpath = (*buf == NUL ? "." : (char *)buf);
if (os_file_is_readable(dirpath) && os_scandir(&dir, dirpath)) {

char_u *name;
scandir_next_with_dots(NULL); 
while ((name = (char_u *)scandir_next_with_dots(&dir)) != NULL) {
if ((name[0] != '.'
|| starts_with_dot
|| ((flags & EW_DODOT)
&& name[1] != NUL
&& (name[1] != '.' || name[2] != NUL))) 
&& ((regmatch.regprog != NULL && vim_regexec(&regmatch, name, 0))
|| ((flags & EW_NOTWILD)
&& fnamencmp(path + (s - buf), name, e - s) == 0))) {
STRCPY(s, name);
len = STRLEN(buf);

if (starstar && stardepth < 100) {


STRCPY(buf + len, "/**");
STRCPY(buf + len + 3, path_end);
++stardepth;
(void)do_path_expand(gap, buf, len + 1, flags, true);
--stardepth;
}

STRCPY(buf + len, path_end);
if (path_has_exp_wildcard(path_end)) { 


(void)do_path_expand(gap, buf, len + 1, flags, false);
} else {
FileInfo file_info;



if (*path_end != NUL) {
backslash_halve(buf + len + 1);
}

if ((flags & EW_ALLLINKS) ? os_fileinfo_link((char *)buf, &file_info)
: os_path_exists(buf)) {
addfile(gap, buf, flags);
}
}
}
}
os_closedir(&dir);
}

xfree(buf);
vim_regfree(regmatch.regprog);

size_t matches = (size_t)(gap->ga_len - start_len);
if (matches > 0) {
qsort(((char_u **)gap->ga_data) + start_len, matches,
sizeof(char_u *), pstrcmp);
}
return matches;
}





static int find_previous_pathsep(char_u *path, char_u **psep)
{

if (*psep > path && vim_ispathsep(**psep))
--*psep;


while (*psep > path) {
if (vim_ispathsep(**psep))
return OK;
MB_PTR_BACK(path, *psep);
}

return FAIL;
}





static bool is_unique(char_u *maybe_unique, garray_T *gap, int i)
{
char_u **other_paths = (char_u **)gap->ga_data;

for (int j = 0; j < gap->ga_len; j++) {
if (j == i) {
continue; 
}
size_t candidate_len = STRLEN(maybe_unique);
size_t other_path_len = STRLEN(other_paths[j]);
if (other_path_len < candidate_len) {
continue; 
}
char_u *rival = other_paths[j] + other_path_len - candidate_len;
if (fnamecmp(maybe_unique, rival) == 0
&& (rival == other_paths[j] || vim_ispathsep(*(rival - 1)))) {
return false; 
}
}
return true; 
}










static void expand_path_option(char_u *curdir, garray_T *gap)
{
char_u *path_option = *curbuf->b_p_path == NUL ? p_path : curbuf->b_p_path;
char_u *buf = xmalloc(MAXPATHL);

while (*path_option != NUL) {
copy_option_part(&path_option, buf, MAXPATHL, " ,");

if (buf[0] == '.' && (buf[1] == NUL || vim_ispathsep(buf[1]))) {



if (curbuf->b_ffname == NULL)
continue;
char_u *p = path_tail(curbuf->b_ffname);
size_t len = (size_t)(p - curbuf->b_ffname);
if (len + STRLEN(buf) >= MAXPATHL) {
continue;
}
if (buf[1] == NUL) {
buf[len] = NUL;
} else {
STRMOVE(buf + len, buf + 2);
}
memmove(buf, curbuf->b_ffname, len);
simplify_filename(buf);
} else if (buf[0] == NUL) {
STRCPY(buf, curdir); 
} else if (path_with_url((char *)buf)) {
continue; 
} else if (!path_is_absolute(buf)) {

size_t len = STRLEN(curdir);
if (len + STRLEN(buf) + 3 > MAXPATHL) {
continue;
}
STRMOVE(buf + len + 1, buf);
STRCPY(buf, curdir);
buf[len] = (char_u)PATHSEP;
simplify_filename(buf);
}

GA_APPEND(char_u *, gap, vim_strsave(buf));
}

xfree(buf);
}









static char_u *get_path_cutoff(char_u *fname, garray_T *gap)
{
int maxlen = 0;
char_u **path_part = (char_u **)gap->ga_data;
char_u *cutoff = NULL;

for (int i = 0; i < gap->ga_len; i++) {
int j = 0;

while ((fname[j] == path_part[i][j]
#if defined(WIN32)
|| (vim_ispathsep(fname[j]) && vim_ispathsep(path_part[i][j]))
#endif
) 
&& fname[j] != NUL && path_part[i][j] != NUL) {
j++;
}
if (j > maxlen) {
maxlen = j;
cutoff = &fname[j];
}
}


if (cutoff != NULL) {
while (vim_ispathsep(*cutoff)) {
MB_PTR_ADV(cutoff);
}
}

return cutoff;
}






static void uniquefy_paths(garray_T *gap, char_u *pattern)
{
char_u **fnames = (char_u **)gap->ga_data;
bool sort_again = false;
regmatch_T regmatch;
garray_T path_ga;
char_u **in_curdir = NULL;
char_u *short_name;

ga_remove_duplicate_strings(gap);
ga_init(&path_ga, (int)sizeof(char_u *), 1);




size_t len = STRLEN(pattern);
char_u *file_pattern = xmalloc(len + 2);
file_pattern[0] = '*';
file_pattern[1] = NUL;
STRCAT(file_pattern, pattern);
char_u *pat = file_pat_to_reg_pat(file_pattern, NULL, NULL, true);
xfree(file_pattern);
if (pat == NULL)
return;

regmatch.rm_ic = TRUE; 
regmatch.regprog = vim_regcomp(pat, RE_MAGIC + RE_STRING);
xfree(pat);
if (regmatch.regprog == NULL)
return;

char_u *curdir = xmalloc(MAXPATHL);
os_dirname(curdir, MAXPATHL);
expand_path_option(curdir, &path_ga);

in_curdir = xcalloc((size_t)gap->ga_len, sizeof(char_u *));

for (int i = 0; i < gap->ga_len && !got_int; i++) {
char_u *path = fnames[i];
int is_in_curdir;
char_u *dir_end = (char_u *)gettail_dir((const char *)path);
char_u *pathsep_p;
char_u *path_cutoff;

len = STRLEN(path);
is_in_curdir = fnamencmp(curdir, path, dir_end - path) == 0
&& curdir[dir_end - path] == NUL;
if (is_in_curdir)
in_curdir[i] = vim_strsave(path);


path_cutoff = get_path_cutoff(path, &path_ga);




if (pattern[0] == '*' && pattern[1] == '*'
&& vim_ispathsep_nocolon(pattern[2])
&& path_cutoff != NULL
&& vim_regexec(&regmatch, path_cutoff, (colnr_T)0)
&& is_unique(path_cutoff, gap, i)) {
sort_again = true;
memmove(path, path_cutoff, STRLEN(path_cutoff) + 1);
} else {


pathsep_p = path + len - 1;
while (find_previous_pathsep(path, &pathsep_p)) {
if (vim_regexec(&regmatch, pathsep_p + 1, (colnr_T)0)
&& is_unique(pathsep_p + 1, gap, i)
&& path_cutoff != NULL && pathsep_p + 1 >= path_cutoff) {
sort_again = true;
memmove(path, pathsep_p + 1, STRLEN(pathsep_p));
break;
}
}
}

if (path_is_absolute(path)) {










short_name = path_shorten_fname(path, curdir);
if (short_name != NULL && short_name > path + 1
) {
STRCPY(path, ".");
add_pathsep((char *)path);
STRMOVE(path + STRLEN(path), short_name);
}
}
os_breakcheck();
}


for (int i = 0; i < gap->ga_len && !got_int; i++) {
char_u *rel_path;
char_u *path = in_curdir[i];

if (path == NULL)
continue;



short_name = path_shorten_fname(path, curdir);
if (short_name == NULL)
short_name = path;
if (is_unique(short_name, gap, i)) {
STRCPY(fnames[i], short_name);
continue;
}

rel_path = xmalloc(STRLEN(short_name) + STRLEN(PATHSEPSTR) + 2);
STRCPY(rel_path, ".");
add_pathsep((char *)rel_path);
STRCAT(rel_path, short_name);

xfree(fnames[i]);
fnames[i] = rel_path;
sort_again = true;
os_breakcheck();
}

xfree(curdir);
for (int i = 0; i < gap->ga_len; i++)
xfree(in_curdir[i]);
xfree(in_curdir);
ga_clear_strings(&path_ga);
vim_regfree(regmatch.regprog);

if (sort_again)
ga_remove_duplicate_strings(gap);
}









const char *gettail_dir(const char *const fname)
FUNC_ATTR_PURE FUNC_ATTR_WARN_UNUSED_RESULT FUNC_ATTR_NONNULL_ALL
{
const char *dir_end = fname;
const char *next_dir_end = fname;
bool look_for_sep = true;

for (const char *p = fname; *p != NUL; ) {
if (vim_ispathsep(*p)) {
if (look_for_sep) {
next_dir_end = p;
look_for_sep = false;
}
} else {
if (!look_for_sep)
dir_end = next_dir_end;
look_for_sep = true;
}
MB_PTR_ADV(p);
}
return dir_end;
}







static int expand_in_path(
garray_T *const gap,
char_u *const pattern,
const int flags 
)
{
garray_T path_ga;

char_u *const curdir = xmalloc(MAXPATHL);
os_dirname(curdir, MAXPATHL);

ga_init(&path_ga, (int)sizeof(char_u *), 1);
expand_path_option(curdir, &path_ga);
xfree(curdir);
if (GA_EMPTY(&path_ga)) {
return 0;
}

char_u *const paths = ga_concat_strings(&path_ga);
ga_clear_strings(&path_ga);

int glob_flags = 0;
if (flags & EW_ICASE) {
glob_flags |= WILD_ICASE;
}
if (flags & EW_ADDSLASH) {
glob_flags |= WILD_ADD_SLASH;
}
globpath(paths, pattern, gap, glob_flags);
xfree(paths);

return gap->ga_len;
}






static bool has_env_var(char_u *p)
{
for (; *p; MB_PTR_ADV(p)) {
if (*p == '\\' && p[1] != NUL) {
p++;
} else if (vim_strchr((char_u *) "$" , *p) != NULL) {
return true;
}
}
return false;
}

#if defined(SPECIAL_WILDCHAR)



static bool has_special_wildchar(char_u *p)
{
for (; *p; MB_PTR_ADV(p)) {

if (*p == '\r' || *p == '\n') {
break;
}

if (*p == '\\' && p[1] != NUL && p[1] != '\r' && p[1] != '\n') {
p++;
} else if (vim_strchr((char_u *)SPECIAL_WILDCHAR, *p) != NULL) {

if (*p == '{' && vim_strchr(p, '}') == NULL) {
continue;
}

if ((*p == '`' || *p == '\'') && vim_strchr(p, *p) == NULL) {
continue;
}
return true;
}
}
return false;
}
#endif



















int gen_expand_wildcards(int num_pat, char_u **pat, int *num_file,
char_u ***file, int flags)
{
garray_T ga;
char_u *p;
static bool recursive = false;
int add_pat;
bool did_expand_in_path = false;







if (recursive)
#if defined(SPECIAL_WILDCHAR)
return os_expand_wildcards(num_pat, pat, num_file, file, flags);
#else
return FAIL;
#endif

#if defined(SPECIAL_WILDCHAR)






for (int i = 0; i < num_pat; i++) {
if (has_special_wildchar(pat[i])
&& !(vim_backtick(pat[i]) && pat[i][1] == '=')) {
return os_expand_wildcards(num_pat, pat, num_file, file, flags);
}
}
#endif

recursive = true;




ga_init(&ga, (int)sizeof(char_u *), 30);

for (int i = 0; i < num_pat; ++i) {
add_pat = -1;
p = pat[i];

if (vim_backtick(p)) {
add_pat = expand_backtick(&ga, p, flags);
if (add_pat == -1) {
recursive = false;
FreeWild(ga.ga_len, (char_u **)ga.ga_data);
*num_file = 0;
*file = NULL;
return FAIL;
}
} else {

if ((has_env_var(p) && !(flags & EW_NOTENV)) || *p == '~') {
p = expand_env_save_opt(p, true);
if (p == NULL)
p = pat[i];
#if defined(UNIX)





else if (has_env_var(p) || *p == '~') {
xfree(p);
ga_clear_strings(&ga);
i = os_expand_wildcards(num_pat, pat, num_file, file,
flags | EW_KEEPDOLLAR);
recursive = false;
return i;
}
#endif
}








if (path_has_exp_wildcard(p)) {
if ((flags & EW_PATH)
&& !path_is_absolute(p)
&& !(p[0] == '.'
&& (vim_ispathsep(p[1])
|| (p[1] == '.' && vim_ispathsep(p[2]))))
) {


recursive = false;
add_pat = expand_in_path(&ga, p, flags);
recursive = true;
did_expand_in_path = true;
} else {
size_t tmp_add_pat = path_expand(&ga, p, flags);
assert(tmp_add_pat <= INT_MAX);
add_pat = (int)tmp_add_pat;
}
}
}

if (add_pat == -1 || (add_pat == 0 && (flags & EW_NOTFOUND))) {
char_u *t = backslash_halve_save(p);



if (flags & EW_NOTFOUND) {
addfile(&ga, t, flags | EW_DIR | EW_FILE);
} else {
addfile(&ga, t, flags);
}

if (t != p) {
xfree(t);
}
}

if (did_expand_in_path && !GA_EMPTY(&ga) && (flags & EW_PATH))
uniquefy_paths(&ga, p);
if (p != pat[i])
xfree(p);
}

*num_file = ga.ga_len;
*file = (ga.ga_data != NULL) ? (char_u **)ga.ga_data : NULL;

recursive = false;

return ((flags & EW_EMPTYOK) || ga.ga_data != NULL) ? OK : FAIL;
}





static int vim_backtick(char_u *p)
{
return *p == '`' && *(p + 1) != NUL && *(p + STRLEN(p) - 1) == '`';
}




static int expand_backtick(
garray_T *gap,
char_u *pat,
int flags 
)
{
char_u *p;
char_u *buffer;
int cnt = 0;


char_u *cmd = vim_strnsave(pat + 1, STRLEN(pat) - 2);

if (*cmd == '=') 
buffer = eval_to_string(cmd + 1, &p, TRUE);
else
buffer = get_cmd_output(cmd, NULL,
(flags & EW_SILENT) ? kShellOptSilent : 0, NULL);
xfree(cmd);
if (buffer == NULL) {
return -1;
}

cmd = buffer;
while (*cmd != NUL) {
cmd = skipwhite(cmd); 
p = cmd;
while (*p != NUL && *p != '\r' && *p != '\n') 
++p;

if (p > cmd) {
char_u i = *p;
*p = NUL;
addfile(gap, cmd, flags);
*p = i;
++cnt;
}
cmd = p;
while (*cmd != NUL && (*cmd == '\r' || *cmd == '\n'))
++cmd;
}

xfree(buffer);
return cnt;
}

#if defined(BACKSLASH_IN_FILENAME)








void slash_adjust(char_u *p)
{
if (path_with_url((const char *)p)) {
return;
}

if (*p == '`') {

const size_t len = STRLEN(p);
if (len > 2 && *(p + len - 1) == '`') {
return;
}
}

while (*p) {
if (*p == (char_u)psepcN) {
*p = (char_u)psepc;
}
MB_PTR_ADV(p);
}
}
#endif








void addfile(
garray_T *gap,
char_u *f, 
int flags
)
{
bool isdir;
FileInfo file_info;


if (!(flags & EW_NOTFOUND)
&& ((flags & EW_ALLLINKS)
? !os_fileinfo_link((char *)f, &file_info)
: !os_path_exists(f))) {
return;
}

#if defined(FNAME_ILLEGAL)

if (vim_strpbrk(f, (char_u *)FNAME_ILLEGAL) != NULL)
return;
#endif

isdir = os_isdir(f);
if ((isdir && !(flags & EW_DIR)) || (!isdir && !(flags & EW_FILE)))
return;



if (!isdir && (flags & EW_EXEC)
&& !os_can_exe((char *)f, NULL, !(flags & EW_SHELLCMD))) {
return;
}

char_u *p = xmalloc(STRLEN(f) + 1 + isdir);

STRCPY(p, f);
#if defined(BACKSLASH_IN_FILENAME)
slash_adjust(p);
#endif



if (isdir && (flags & EW_ADDSLASH))
add_pathsep((char *)p);
GA_APPEND(char_u *, gap, p);
}







void simplify_filename(char_u *filename)
{
int components = 0;
char_u *p, *tail, *start;
bool stripping_disabled = false;
bool relative = true;

p = filename;
#if defined(BACKSLASH_IN_FILENAME)
if (p[1] == ':') 
p += 2;
#endif

if (vim_ispathsep(*p)) {
relative = false;
do
++p;
while (vim_ispathsep(*p));
}
start = p; 

do {


if (vim_ispathsep(*p))
STRMOVE(p, p + 1); 
else if (p[0] == '.' && (vim_ispathsep(p[1]) || p[1] == NUL)) {
if (p == start && relative)
p += 1 + (p[1] != NUL); 
else {




tail = p + 1;
if (p[1] != NUL) {
while (vim_ispathsep(*tail)) {
MB_PTR_ADV(tail);
}
} else if (p > start) {
p--; 
}
STRMOVE(p, tail);
}
} else if (p[0] == '.' && p[1] == '.'
&& (vim_ispathsep(p[2]) || p[2] == NUL)) {

tail = p + 2;
while (vim_ispathsep(*tail)) {
MB_PTR_ADV(tail);
}

if (components > 0) { 
bool do_strip = false;
char_u saved_char;


if (!stripping_disabled) {



saved_char = p[-1];
p[-1] = NUL;
FileInfo file_info;
if (!os_fileinfo_link((char *)filename, &file_info)) {
do_strip = true;
}
p[-1] = saved_char;

p--;

while (p > start && !after_pathsep((char *)start, (char *)p)) {
MB_PTR_BACK(start, p);
}

if (!do_strip) {










saved_char = *tail;
*tail = NUL;
if (os_fileinfo((char *)filename, &file_info)) {
do_strip = true;
} else {
stripping_disabled = true;
}
*tail = saved_char;
if (do_strip) {







FileInfo new_file_info;
if (p == start && relative) {
os_fileinfo(".", &new_file_info);
} else {
saved_char = *p;
*p = NUL;
os_fileinfo((char *)filename, &new_file_info);
*p = saved_char;
}

if (!os_fileinfo_id_equal(&file_info, &new_file_info)) {
do_strip = false;



}
}
}
}

if (!do_strip) {


p = tail;
components = 0;
} else {






if (p == start && relative && tail[-1] == '.') {
*p++ = '.';
*p = NUL;
} else {
if (p > start && tail[-1] == '.')
--p;
STRMOVE(p, tail); 
}

--components;
}
} else if (p == start && !relative) 
STRMOVE(p, tail); 
else {
if (p == start + 2 && p[-2] == '.') { 
STRMOVE(p - 2, p); 
tail -= 2;
}
p = tail; 
}
} else {
components++; 
p = (char_u *)path_next_component((const char *)p);
}
} while (*p != NUL);
}

static char *eval_includeexpr(const char *const ptr, const size_t len)
{
set_vim_var_string(VV_FNAME, ptr, (ptrdiff_t) len);
char *res = (char *) eval_to_string_safe(
curbuf->b_p_inex, NULL, was_set_insecurely((char_u *)"includeexpr",
OPT_LOCAL));
set_vim_var_string(VV_FNAME, NULL, 0);
return res;
}





char_u *
find_file_name_in_path (
char_u *ptr,
size_t len,
int options,
long count,
char_u *rel_fname 
)
{
char_u *file_name;
char_u *tofree = NULL;

if ((options & FNAME_INCL) && *curbuf->b_p_inex != NUL) {
tofree = (char_u *) eval_includeexpr((char *) ptr, len);
if (tofree != NULL) {
ptr = tofree;
len = STRLEN(ptr);
}
}

if (options & FNAME_EXP) {
file_name = find_file_in_path(ptr, len, options & ~FNAME_MESS, true,
rel_fname);





if (file_name == NULL
&& !(options & FNAME_INCL) && *curbuf->b_p_inex != NUL) {
tofree = (char_u *) eval_includeexpr((char *) ptr, len);
if (tofree != NULL) {
ptr = tofree;
len = STRLEN(ptr);
file_name = find_file_in_path(ptr, len, options & ~FNAME_MESS,
TRUE, rel_fname);
}
}
if (file_name == NULL && (options & FNAME_MESS)) {
char_u c = ptr[len];
ptr[len] = NUL;
EMSG2(_("E447: Can't find file \"%s\" in path"), ptr);
ptr[len] = c;
}



while (file_name != NULL && --count > 0) {
xfree(file_name);
file_name = find_file_in_path(ptr, len, options, FALSE, rel_fname);
}
} else
file_name = vim_strnsave(ptr, len);

xfree(tofree);

return file_name;
}




int path_is_url(const char *p)
{
if (strncmp(p, "://", 3) == 0)
return URL_SLASH;
else if (strncmp(p, ":\\\\", 3) == 0)
return URL_BACKSLASH;
return 0;
}





int path_with_url(const char *fname)
{
const char *p;
for (p = fname; isalpha(*p); p++) {}
return path_is_url(p);
}




bool vim_isAbsName(char_u *name)
{
return path_with_url((char *)name) != 0 || path_is_absolute(name);
}











int vim_FullName(const char *fname, char *buf, size_t len, bool force)
FUNC_ATTR_NONNULL_ARG(2)
{
*buf = NUL;
if (fname == NULL) {
return FAIL;
}

if (strlen(fname) > (len - 1)) {
xstrlcpy(buf, fname, len); 
#if defined(WIN32)
slash_adjust((char_u *)buf);
#endif
return FAIL;
}

if (path_with_url(fname)) {
xstrlcpy(buf, fname, len);
return OK;
}

int rv = path_to_absolute((char_u *)fname, (char_u *)buf, len, force);
if (rv == FAIL) {
xstrlcpy(buf, fname, len); 
}
#if defined(WIN32)
slash_adjust((char_u *)buf);
#endif
return rv;
}











char *fix_fname(const char *fname)
{
#if defined(UNIX)
return FullName_save(fname, true);
#else
if (!vim_isAbsName((char_u *)fname)
|| strstr(fname, "..") != NULL
|| strstr(fname, "//") != NULL
#if defined(BACKSLASH_IN_FILENAME)
|| strstr(fname, "\\\\") != NULL
#endif
)
return FullName_save(fname, false);

fname = xstrdup(fname);

#if defined(USE_FNAME_CASE)
path_fix_case((char_u *)fname); 
#endif

return (char *)fname;
#endif
}






void path_fix_case(char_u *name)
FUNC_ATTR_NONNULL_ALL
{
FileInfo file_info;
if (!os_fileinfo_link((char *)name, &file_info)) {
return;
}


char_u *slash = STRRCHR(name, '/');
char_u *tail;
Directory dir;
bool ok;
if (slash == NULL) {
ok = os_scandir(&dir, ".");
tail = name;
} else {
*slash = NUL;
ok = os_scandir(&dir, (char *) name);
*slash = '/';
tail = slash + 1;
}

if (!ok) {
return;
}

char_u *entry;
while ((entry = (char_u *) os_scandir_next(&dir))) {


if (STRICMP(tail, entry) == 0 && STRLEN(tail) == STRLEN(entry)) {
char_u newname[MAXPATHL + 1];


STRLCPY(newname, name, MAXPATHL + 1);
STRLCPY(newname + (tail - name), entry,
MAXPATHL - (tail - name) + 1);
FileInfo file_info_new;
if (os_fileinfo_link((char *)newname, &file_info_new)
&& os_fileinfo_id_equal(&file_info, &file_info_new)) {
STRCPY(tail, entry);
break;
}
}
}

os_closedir(&dir);
}






int after_pathsep(const char *b, const char *p)
{
return p > b && vim_ispathsep(p[-1])
&& utf_head_off((char_u *)b, (char_u *)p - 1) == 0;
}





bool same_directory(char_u *f1, char_u *f2)
{
char_u ffname[MAXPATHL];
char_u *t1;
char_u *t2;


if (f1 == NULL || f2 == NULL)
return false;

(void)vim_FullName((char *)f1, (char *)ffname, MAXPATHL, FALSE);
t1 = path_tail_with_sep(ffname);
t2 = path_tail_with_sep(f2);
return t1 - ffname == t2 - f2
&& pathcmp((char *)ffname, (char *)f2, (int)(t1 - ffname)) == 0;
}






int pathcmp(const char *p, const char *q, int maxlen)
{
int i, j;
int c1, c2;
const char *s = NULL;

for (i = 0, j = 0; maxlen < 0 || (i < maxlen && j < maxlen);) {
c1 = PTR2CHAR((char_u *)p + i);
c2 = PTR2CHAR((char_u *)q + j);


if (c1 == NUL) {
if (c2 == NUL) 
return 0;
s = q;
i = j;
break;
}


if (c2 == NUL) {
s = p;
break;
}

if ((p_fic ? mb_toupper(c1) != mb_toupper(c2) : c1 != c2)
#if defined(BACKSLASH_IN_FILENAME)

&& !((c1 == '/' && c2 == '\\')
|| (c1 == '\\' && c2 == '/'))
#endif
) {
if (vim_ispathsep(c1))
return -1;
if (vim_ispathsep(c2))
return 1;
return p_fic ? mb_toupper(c1) - mb_toupper(c2)
: c1 - c2; 
}

i += utfc_ptr2len((char_u *)p + i);
j += utfc_ptr2len((char_u *)q + j);
}
if (s == NULL) { 
return 0;
}

c1 = PTR2CHAR((char_u *)s + i);
c2 = PTR2CHAR((char_u *)s + i + utfc_ptr2len((char_u *)s + i));

if (c2 == NUL
&& i > 0
&& !after_pathsep((char *)s, (char *)s + i)
#if defined(BACKSLASH_IN_FILENAME)
&& (c1 == '/' || c1 == '\\')
#else
&& c1 == '/'
#endif
) {
return 0; 
}
if (s == q) {
return -1; 
}
return 1;
}









char_u *path_try_shorten_fname(char_u *full_path)
{
char_u *dirname = xmalloc(MAXPATHL);
char_u *p = full_path;

if (os_dirname(dirname, MAXPATHL) == OK) {
p = path_shorten_fname(full_path, dirname);
if (p == NULL || *p == NUL) {
p = full_path;
}
}
xfree(dirname);
return p;
}








char_u *path_shorten_fname(char_u *full_path, char_u *dir_name)
{
if (full_path == NULL) {
return NULL;
}

assert(dir_name != NULL);
size_t len = strlen((char *)dir_name);
char_u *p = full_path + len;

if (fnamencmp(dir_name, full_path, len) != 0
|| !vim_ispathsep(*p)) {
return NULL;
}

return p + 1;
}















int expand_wildcards_eval(char_u **pat, int *num_file, char_u ***file,
int flags)
{
int ret = FAIL;
char_u *eval_pat = NULL;
char_u *exp_pat = *pat;
char_u *ignored_msg;
size_t usedlen;

if (*exp_pat == '%' || *exp_pat == '#' || *exp_pat == '<') {
++emsg_off;
eval_pat = eval_vars(exp_pat, exp_pat, &usedlen,
NULL, &ignored_msg, NULL);
--emsg_off;
if (eval_pat != NULL)
exp_pat = concat_str(eval_pat, exp_pat + usedlen);
}

if (exp_pat != NULL)
ret = expand_wildcards(1, &exp_pat, num_file, file, flags);

if (eval_pat != NULL) {
xfree(exp_pat);
xfree(eval_pat);
}

return ret;
}















int expand_wildcards(int num_pat, char_u **pat, int *num_files, char_u ***files,
int flags)
{
int retval;
int i, j;
char_u *p;
int non_suf_match; 

retval = gen_expand_wildcards(num_pat, pat, num_files, files, flags);


if ((flags & EW_KEEPALL) || retval == FAIL)
return retval;




if (*p_wig) {
char_u *ffname;


assert(*num_files == 0 || *files != NULL);
for (i = 0; i < *num_files; i++) {
ffname = (char_u *)FullName_save((char *)(*files)[i], false);
assert((*files)[i] != NULL);
assert(ffname != NULL);
if (match_file_list(p_wig, (*files)[i], ffname)) {

xfree((*files)[i]);
for (j = i; j + 1 < *num_files; j++) {
(*files)[j] = (*files)[j + 1];
}
(*num_files)--;
i--;
}
xfree(ffname);
}
}




assert(*num_files == 0 || *files != NULL);
if (*num_files > 1) {
non_suf_match = 0;
for (i = 0; i < *num_files; i++) {
if (!match_suffix((*files)[i])) {



p = (*files)[i];
for (j = i; j > non_suf_match; j--) {
(*files)[j] = (*files)[j - 1];
}
(*files)[non_suf_match++] = p;
}
}
}


if (*num_files == 0) {
XFREE_CLEAR(*files);
return FAIL;
}

return retval;
}




int match_suffix(char_u *fname)
{
#define MAXSUFLEN 30 
char_u suf_buf[MAXSUFLEN];

size_t fnamelen = STRLEN(fname);
size_t setsuflen = 0;
for (char_u *setsuf = p_su; *setsuf; ) {
setsuflen = copy_option_part(&setsuf, suf_buf, MAXSUFLEN, ".,");
if (setsuflen == 0) {
char_u *tail = path_tail(fname);


if (vim_strchr(tail, '.') == NULL) {
setsuflen = 1;
break;
}
} else {
if (fnamelen >= setsuflen
&& fnamencmp(suf_buf, fname + fnamelen - setsuflen, setsuflen) == 0) {
break;
}
setsuflen = 0;
}
}
return setsuflen != 0;
}





int path_full_dir_name(char *directory, char *buffer, size_t len)
{
int SUCCESS = 0;
int retval = OK;

if (STRLEN(directory) == 0) {
return os_dirname((char_u *) buffer, len);
}

char old_dir[MAXPATHL];


if (os_dirname((char_u *) old_dir, MAXPATHL) == FAIL) {
return FAIL;
}


if (os_chdir(old_dir) != SUCCESS) {
return FAIL;
}

if (os_chdir(directory) != SUCCESS) {

retval = FAIL;
}

if (retval == FAIL || os_dirname((char_u *) buffer, len) == FAIL) {

retval = FAIL;
}

if (os_chdir(old_dir) != SUCCESS) {

retval = FAIL;
EMSG(_(e_prev_dir));
}

return retval;
}


int append_path(char *path, const char *to_append, size_t max_len)
{
size_t current_length = strlen(path);
size_t to_append_length = strlen(to_append);


if (to_append_length == 0 || strcmp(to_append, ".") == 0) {
return OK;
}


if (current_length > 0 && !vim_ispathsep_nocolon(path[current_length-1])) {
current_length += 1; 


if (current_length + 1 > max_len) {
return FAIL;
}

xstrlcat(path, PATHSEPSTR, max_len);
}


if (current_length + to_append_length + 1 > max_len) {
return FAIL;
}

xstrlcat(path, to_append, max_len);
return OK;
}









static int path_to_absolute(const char_u *fname, char_u *buf, size_t len,
int force)
{
char_u *p;
*buf = NUL;

char *relative_directory = xmalloc(len);
char *end_of_path = (char *) fname;


if (force || !path_is_absolute(fname)) {
p = STRRCHR(fname, '/');
#if defined(WIN32)
if (p == NULL) {
p = STRRCHR(fname, '\\');
}
#endif
if (p != NULL) {

if (p == fname) {

relative_directory[0] = PATHSEP;
relative_directory[1] = NUL;
} else {
assert(p >= fname);
memcpy(relative_directory, fname, (size_t)(p - fname));
relative_directory[p-fname] = NUL;
}
end_of_path = (char *) (p + 1);
} else {
relative_directory[0] = NUL;
end_of_path = (char *) fname;
}

if (FAIL == path_full_dir_name(relative_directory, (char *) buf, len)) {
xfree(relative_directory);
return FAIL;
}
}
xfree(relative_directory);
return append_path((char *)buf, end_of_path, len);
}




int path_is_absolute(const char_u *fname)
{
#if defined(WIN32)

return ((isalpha(fname[0]) && fname[1] == ':'
&& vim_ispathsep_nocolon(fname[2]))
|| (vim_ispathsep_nocolon(fname[0]) && fname[0] == fname[1]));
#else

return *fname == '/' || *fname == '~';
#endif
}








void path_guess_exepath(const char *argv0, char *buf, size_t bufsize)
FUNC_ATTR_NONNULL_ALL
{
const char *path = os_getenv("PATH");

if (path == NULL || path_is_absolute((char_u *)argv0)) {
xstrlcpy(buf, argv0, bufsize);
} else if (argv0[0] == '.' || strchr(argv0, PATHSEP)) {

if (os_dirname((char_u *)buf, MAXPATHL) != OK) {
buf[0] = NUL;
}
xstrlcat(buf, PATHSEPSTR, bufsize);
xstrlcat(buf, argv0, bufsize);
} else {

const void *iter = NULL;
do {
const char *dir;
size_t dir_len;
iter = vim_env_iter(ENV_SEPCHAR, path, iter, &dir, &dir_len);
if (dir == NULL || dir_len == 0) {
break;
}
if (dir_len + 1 > sizeof(NameBuff)) {
continue;
}
xstrlcpy((char *)NameBuff, dir, dir_len + 1);
xstrlcat((char *)NameBuff, PATHSEPSTR, sizeof(NameBuff));
xstrlcat((char *)NameBuff, argv0, sizeof(NameBuff));
if (os_can_exe((char *)NameBuff, NULL, false)) {
xstrlcpy(buf, (char *)NameBuff, bufsize);
return;
}
} while (iter != NULL);

xstrlcpy(buf, argv0, bufsize);
}
}
