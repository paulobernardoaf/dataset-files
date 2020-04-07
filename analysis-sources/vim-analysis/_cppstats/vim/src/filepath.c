#include "vim.h"
#if defined(MSWIN)
static int
get_short_pathname(char_u **fnamep, char_u **bufp, int *fnamelen)
{
int l, len;
WCHAR *newbuf;
WCHAR *wfname;
len = MAXPATHL;
newbuf = malloc(len * sizeof(*newbuf));
if (newbuf == NULL)
return FAIL;
wfname = enc_to_utf16(*fnamep, NULL);
if (wfname == NULL)
{
vim_free(newbuf);
return FAIL;
}
l = GetShortPathNameW(wfname, newbuf, len);
if (l > len - 1)
{
WCHAR *newbuf_t = newbuf;
newbuf = vim_realloc(newbuf, (l + 1) * sizeof(*newbuf));
if (newbuf == NULL)
{
vim_free(wfname);
vim_free(newbuf_t);
return FAIL;
}
l = GetShortPathNameW(wfname, newbuf, l+1);
}
if (l != 0)
{
char_u *p = utf16_to_enc(newbuf, NULL);
if (p != NULL)
{
vim_free(*bufp);
*fnamep = *bufp = p;
}
else
{
vim_free(wfname);
vim_free(newbuf);
return FAIL;
}
}
vim_free(wfname);
vim_free(newbuf);
*fnamelen = l == 0 ? l : (int)STRLEN(*bufp);
return OK;
}
static int
shortpath_for_invalid_fname(
char_u **fname,
char_u **bufp,
int *fnamelen)
{
char_u *short_fname, *save_fname, *pbuf_unused;
char_u *endp, *save_endp;
char_u ch;
int old_len, len;
int new_len, sfx_len;
int retval = OK;
old_len = *fnamelen;
save_fname = vim_strnsave(*fname, old_len);
pbuf_unused = NULL;
short_fname = NULL;
endp = save_fname + old_len - 1; 
save_endp = endp;
len = 0;
for (;;)
{
while (endp > save_fname && !after_pathsep(save_fname, endp + 1))
--endp;
if (endp <= save_fname)
break; 
ch = *endp;
*endp = 0;
short_fname = save_fname;
len = (int)STRLEN(short_fname) + 1;
if (get_short_pathname(&short_fname, &pbuf_unused, &len) == FAIL)
{
retval = FAIL;
goto theend;
}
*endp = ch; 
if (len > 0)
break; 
--endp;
}
if (len > 0)
{
sfx_len = (int)(save_endp - endp) + 1;
new_len = len + sfx_len;
*fnamelen = new_len;
vim_free(*bufp);
if (new_len > old_len)
{
*fname = *bufp = vim_strnsave(short_fname, new_len);
if (*fname == NULL)
{
retval = FAIL;
goto theend;
}
}
else
{
*fname = *bufp = save_fname;
if (short_fname != save_fname)
vim_strncpy(save_fname, short_fname, len);
save_fname = NULL;
}
vim_strncpy(*fname + len, endp, sfx_len);
(*fname)[new_len] = NUL;
}
theend:
vim_free(pbuf_unused);
vim_free(save_fname);
return retval;
}
static int
shortpath_for_partial(
char_u **fnamep,
char_u **bufp,
int *fnamelen)
{
int sepcount, len, tflen;
char_u *p;
char_u *pbuf, *tfname;
int hasTilde;
sepcount = 0;
for (p = *fnamep; p < *fnamep + *fnamelen; MB_PTR_ADV(p))
if (vim_ispathsep(*p))
++sepcount;
hasTilde = (**fnamep == '~');
if (hasTilde)
pbuf = tfname = expand_env_save(*fnamep);
else
pbuf = tfname = FullName_save(*fnamep, FALSE);
len = tflen = (int)STRLEN(tfname);
if (get_short_pathname(&tfname, &pbuf, &len) == FAIL)
return FAIL;
if (len == 0)
{
len = tflen;
if (shortpath_for_invalid_fname(&tfname, &pbuf, &len) == FAIL)
return FAIL;
}
for (p = tfname + len - 1; p >= tfname; --p)
{
if (has_mbyte)
p -= mb_head_off(tfname, p);
if (vim_ispathsep(*p))
{
if (sepcount == 0 || (hasTilde && sepcount == 1))
break;
else
sepcount --;
}
}
if (hasTilde)
{
--p;
if (p >= tfname)
*p = '~';
else
return FAIL;
}
else
++p;
vim_free(*bufp);
*fnamelen = (int)STRLEN(p);
*bufp = pbuf;
*fnamep = p;
return OK;
}
#endif 
int
modify_fname(
char_u *src, 
int tilde_file, 
int *usedlen, 
char_u **fnamep, 
char_u **bufp, 
int *fnamelen) 
{
int valid = 0;
char_u *tail;
char_u *s, *p, *pbuf;
char_u dirname[MAXPATHL];
int c;
int has_fullname = 0;
int has_homerelative = 0;
#if defined(MSWIN)
char_u *fname_start = *fnamep;
int has_shortname = 0;
#endif
repeat:
if (src[*usedlen] == ':' && src[*usedlen + 1] == 'p')
{
has_fullname = 1;
valid |= VALID_PATH;
*usedlen += 2;
if ((*fnamep)[0] == '~'
#if !defined(UNIX) && !(defined(VMS) && defined(USER_HOME))
&& ((*fnamep)[1] == '/'
#if defined(BACKSLASH_IN_FILENAME)
|| (*fnamep)[1] == '\\'
#endif
|| (*fnamep)[1] == NUL)
#endif
&& !(tilde_file && (*fnamep)[1] == NUL)
)
{
*fnamep = expand_env_save(*fnamep);
vim_free(*bufp); 
*bufp = *fnamep;
if (*fnamep == NULL)
return -1;
}
for (p = *fnamep; *p != NUL; MB_PTR_ADV(p))
{
if (vim_ispathsep(*p)
&& p[1] == '.'
&& (p[2] == NUL
|| vim_ispathsep(p[2])
|| (p[2] == '.'
&& (p[3] == NUL || vim_ispathsep(p[3])))))
break;
}
if (*p != NUL || !vim_isAbsName(*fnamep))
{
*fnamep = FullName_save(*fnamep, *p != NUL);
vim_free(*bufp); 
*bufp = *fnamep;
if (*fnamep == NULL)
return -1;
}
#if defined(MSWIN)
#if _WIN32_WINNT >= 0x0500
if (vim_strchr(*fnamep, '~') != NULL)
{
WCHAR *wfname = enc_to_utf16(*fnamep, NULL);
WCHAR buf[_MAX_PATH];
if (wfname != NULL)
{
if (GetLongPathNameW(wfname, buf, _MAX_PATH))
{
char_u *p = utf16_to_enc(buf, NULL);
if (p != NULL)
{
vim_free(*bufp); 
*bufp = *fnamep = p;
}
}
vim_free(wfname);
}
}
#endif
#endif
if (mch_isdir(*fnamep))
{
*fnamep = vim_strnsave(*fnamep, (int)STRLEN(*fnamep) + 2);
vim_free(*bufp); 
*bufp = *fnamep;
if (*fnamep == NULL)
return -1;
add_pathsep(*fnamep);
}
}
while (src[*usedlen] == ':'
&& ((c = src[*usedlen + 1]) == '.' || c == '~' || c == '8'))
{
*usedlen += 2;
if (c == '8')
{
#if defined(MSWIN)
has_shortname = 1; 
#endif
continue;
}
pbuf = NULL;
if (!has_fullname && !has_homerelative)
{
if (c == '.' && **fnamep == '~')
p = pbuf = expand_env_save(*fnamep);
else
p = pbuf = FullName_save(*fnamep, FALSE);
}
else
p = *fnamep;
has_fullname = 0;
if (p != NULL)
{
if (c == '.')
{
size_t namelen;
mch_dirname(dirname, MAXPATHL);
if (has_homerelative)
{
s = vim_strsave(dirname);
if (s != NULL)
{
home_replace(NULL, s, dirname, MAXPATHL, TRUE);
vim_free(s);
}
}
namelen = STRLEN(dirname);
if (fnamencmp(p, dirname, namelen) == 0)
{
p += namelen;
if (vim_ispathsep(*p))
{
while (*p && vim_ispathsep(*p))
++p;
*fnamep = p;
if (pbuf != NULL)
{
vim_free(*bufp);
*bufp = pbuf;
pbuf = NULL;
}
}
}
}
else
{
home_replace(NULL, p, dirname, MAXPATHL, TRUE);
if (*dirname == '~')
{
s = vim_strsave(dirname);
if (s != NULL)
{
*fnamep = s;
vim_free(*bufp);
*bufp = s;
has_homerelative = TRUE;
}
}
}
vim_free(pbuf);
}
}
tail = gettail(*fnamep);
*fnamelen = (int)STRLEN(*fnamep);
while (src[*usedlen] == ':' && src[*usedlen + 1] == 'h')
{
valid |= VALID_HEAD;
*usedlen += 2;
s = get_past_head(*fnamep);
while (tail > s && after_pathsep(s, tail))
MB_PTR_BACK(*fnamep, tail);
*fnamelen = (int)(tail - *fnamep);
#if defined(VMS)
if (*fnamelen > 0)
*fnamelen += 1; 
#endif
if (*fnamelen == 0)
{
p = vim_strsave((char_u *)".");
if (p == NULL)
return -1;
vim_free(*bufp);
*bufp = *fnamep = tail = p;
*fnamelen = 1;
}
else
{
while (tail > s && !after_pathsep(s, tail))
MB_PTR_BACK(*fnamep, tail);
}
}
if (src[*usedlen] == ':' && src[*usedlen + 1] == '8')
{
*usedlen += 2;
#if defined(MSWIN)
has_shortname = 1;
#endif
}
#if defined(MSWIN)
if (has_shortname)
{
if (*fnamelen < (int)STRLEN(*fnamep) || *fnamep == fname_start)
{
p = vim_strnsave(*fnamep, *fnamelen);
if (p == NULL)
return -1;
vim_free(*bufp);
*bufp = *fnamep = p;
}
if (!has_fullname && !vim_isAbsName(*fnamep))
{
if (shortpath_for_partial(fnamep, bufp, fnamelen) == FAIL)
return -1;
}
else
{
int l = *fnamelen;
if (get_short_pathname(fnamep, bufp, &l) == FAIL)
return -1;
if (l == 0)
{
l = *fnamelen;
if (shortpath_for_invalid_fname(fnamep, bufp, &l) == FAIL)
return -1;
}
*fnamelen = l;
}
}
#endif 
if (src[*usedlen] == ':' && src[*usedlen + 1] == 't')
{
*usedlen += 2;
*fnamelen -= (int)(tail - *fnamep);
*fnamep = tail;
}
while (src[*usedlen] == ':'
&& (src[*usedlen + 1] == 'e' || src[*usedlen + 1] == 'r'))
{
if (src[*usedlen + 1] == 'e' && *fnamep > tail)
s = *fnamep - 2;
else
s = *fnamep + *fnamelen - 1;
for ( ; s > tail; --s)
if (s[0] == '.')
break;
if (src[*usedlen + 1] == 'e') 
{
if (s > tail)
{
*fnamelen += (int)(*fnamep - (s + 1));
*fnamep = s + 1;
#if defined(VMS)
s = *fnamep + *fnamelen - 1;
for ( ; s > *fnamep; --s)
if (s[0] == ';')
break;
if (s > *fnamep)
*fnamelen = s - *fnamep;
#endif
}
else if (*fnamep <= tail)
*fnamelen = 0;
}
else 
{
char_u *limit = *fnamep;
if (limit < tail)
limit = tail;
if (s > limit) 
*fnamelen = (int)(s - *fnamep);
}
*usedlen += 2;
}
if (src[*usedlen] == ':'
&& (src[*usedlen + 1] == 's'
|| (src[*usedlen + 1] == 'g' && src[*usedlen + 2] == 's')))
{
char_u *str;
char_u *pat;
char_u *sub;
int sep;
char_u *flags;
int didit = FALSE;
flags = (char_u *)"";
s = src + *usedlen + 2;
if (src[*usedlen + 1] == 'g')
{
flags = (char_u *)"g";
++s;
}
sep = *s++;
if (sep)
{
p = vim_strchr(s, sep);
if (p != NULL)
{
pat = vim_strnsave(s, (int)(p - s));
if (pat != NULL)
{
s = p + 1;
p = vim_strchr(s, sep);
if (p != NULL)
{
sub = vim_strnsave(s, (int)(p - s));
str = vim_strnsave(*fnamep, *fnamelen);
if (sub != NULL && str != NULL)
{
*usedlen = (int)(p + 1 - src);
s = do_string_sub(str, pat, sub, NULL, flags);
if (s != NULL)
{
*fnamep = s;
*fnamelen = (int)STRLEN(s);
vim_free(*bufp);
*bufp = s;
didit = TRUE;
}
}
vim_free(sub);
vim_free(str);
}
vim_free(pat);
}
}
if (didit)
goto repeat;
}
}
if (src[*usedlen] == ':' && src[*usedlen + 1] == 'S')
{
c = (*fnamep)[*fnamelen];
if (c != NUL)
(*fnamep)[*fnamelen] = NUL;
p = vim_strsave_shellescape(*fnamep, FALSE, FALSE);
if (c != NUL)
(*fnamep)[*fnamelen] = c;
if (p == NULL)
return -1;
vim_free(*bufp);
*bufp = *fnamep = p;
*fnamelen = (int)STRLEN(p);
*usedlen += 2;
}
return valid;
}
#if defined(FEAT_EVAL) || defined(PROTO)
void
f_chdir(typval_T *argvars, typval_T *rettv)
{
char_u *cwd;
cdscope_T scope = CDSCOPE_GLOBAL;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (argvars[0].v_type != VAR_STRING)
return;
cwd = alloc(MAXPATHL);
if (cwd != NULL)
{
if (mch_dirname(cwd, MAXPATHL) != FAIL)
{
#if defined(BACKSLASH_IN_FILENAME)
slash_adjust(cwd);
#endif
rettv->vval.v_string = vim_strsave(cwd);
}
vim_free(cwd);
}
if (curwin->w_localdir != NULL)
scope = CDSCOPE_WINDOW;
else if (curtab->tp_localdir != NULL)
scope = CDSCOPE_TABPAGE;
if (!changedir_func(argvars[0].vval.v_string, TRUE, scope))
VIM_CLEAR(rettv->vval.v_string);
}
void
f_delete(typval_T *argvars, typval_T *rettv)
{
char_u nbuf[NUMBUFLEN];
char_u *name;
char_u *flags;
rettv->vval.v_number = -1;
if (check_restricted() || check_secure())
return;
name = tv_get_string(&argvars[0]);
if (name == NULL || *name == NUL)
{
emsg(_(e_invarg));
return;
}
if (argvars[1].v_type != VAR_UNKNOWN)
flags = tv_get_string_buf(&argvars[1], nbuf);
else
flags = (char_u *)"";
if (*flags == NUL)
rettv->vval.v_number = mch_remove(name) == 0 ? 0 : -1;
else if (STRCMP(flags, "d") == 0)
rettv->vval.v_number = mch_rmdir(name) == 0 ? 0 : -1;
else if (STRCMP(flags, "rf") == 0)
rettv->vval.v_number = delete_recursive(name);
else
semsg(_(e_invexpr2), flags);
}
void
f_executable(typval_T *argvars, typval_T *rettv)
{
char_u *name = tv_get_string(&argvars[0]);
rettv->vval.v_number = mch_can_exe(name, NULL, TRUE);
}
void
f_exepath(typval_T *argvars, typval_T *rettv)
{
char_u *p = NULL;
(void)mch_can_exe(tv_get_string(&argvars[0]), &p, TRUE);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = p;
}
void
f_filereadable(typval_T *argvars, typval_T *rettv)
{
int fd;
char_u *p;
int n;
#if !defined(O_NONBLOCK)
#define O_NONBLOCK 0
#endif
p = tv_get_string(&argvars[0]);
if (*p && !mch_isdir(p) && (fd = mch_open((char *)p,
O_RDONLY | O_NONBLOCK, 0)) >= 0)
{
n = TRUE;
close(fd);
}
else
n = FALSE;
rettv->vval.v_number = n;
}
void
f_filewritable(typval_T *argvars, typval_T *rettv)
{
rettv->vval.v_number = filewritable(tv_get_string(&argvars[0]));
}
static void
findfilendir(
typval_T *argvars UNUSED,
typval_T *rettv,
int find_what UNUSED)
{
#if defined(FEAT_SEARCHPATH)
char_u *fname;
char_u *fresult = NULL;
char_u *path = *curbuf->b_p_path == NUL ? p_path : curbuf->b_p_path;
char_u *p;
char_u pathbuf[NUMBUFLEN];
int count = 1;
int first = TRUE;
int error = FALSE;
#endif
rettv->vval.v_string = NULL;
rettv->v_type = VAR_STRING;
#if defined(FEAT_SEARCHPATH)
fname = tv_get_string(&argvars[0]);
if (argvars[1].v_type != VAR_UNKNOWN)
{
p = tv_get_string_buf_chk(&argvars[1], pathbuf);
if (p == NULL)
error = TRUE;
else
{
if (*p != NUL)
path = p;
if (argvars[2].v_type != VAR_UNKNOWN)
count = (int)tv_get_number_chk(&argvars[2], &error);
}
}
if (count < 0 && rettv_list_alloc(rettv) == FAIL)
error = TRUE;
if (*fname != NUL && !error)
{
do
{
if (rettv->v_type == VAR_STRING || rettv->v_type == VAR_LIST)
vim_free(fresult);
fresult = find_file_in_path_option(first ? fname : NULL,
first ? (int)STRLEN(fname) : 0,
0, first, path,
find_what,
curbuf->b_ffname,
find_what == FINDFILE_DIR
? (char_u *)"" : curbuf->b_p_sua);
first = FALSE;
if (fresult != NULL && rettv->v_type == VAR_LIST)
list_append_string(rettv->vval.v_list, fresult, -1);
} while ((rettv->v_type == VAR_LIST || --count > 0) && fresult != NULL);
}
if (rettv->v_type == VAR_STRING)
rettv->vval.v_string = fresult;
#endif
}
void
f_finddir(typval_T *argvars, typval_T *rettv)
{
findfilendir(argvars, rettv, FINDFILE_DIR);
}
void
f_findfile(typval_T *argvars, typval_T *rettv)
{
findfilendir(argvars, rettv, FINDFILE_FILE);
}
void
f_fnamemodify(typval_T *argvars, typval_T *rettv)
{
char_u *fname;
char_u *mods;
int usedlen = 0;
int len;
char_u *fbuf = NULL;
char_u buf[NUMBUFLEN];
fname = tv_get_string_chk(&argvars[0]);
mods = tv_get_string_buf_chk(&argvars[1], buf);
if (fname == NULL || mods == NULL)
fname = NULL;
else
{
len = (int)STRLEN(fname);
(void)modify_fname(mods, FALSE, &usedlen, &fname, &fbuf, &len);
}
rettv->v_type = VAR_STRING;
if (fname == NULL)
rettv->vval.v_string = NULL;
else
rettv->vval.v_string = vim_strnsave(fname, len);
vim_free(fbuf);
}
void
f_getcwd(typval_T *argvars, typval_T *rettv)
{
win_T *wp = NULL;
tabpage_T *tp = NULL;
char_u *cwd;
int global = FALSE;
rettv->v_type = VAR_STRING;
rettv->vval.v_string = NULL;
if (argvars[0].v_type == VAR_NUMBER
&& argvars[0].vval.v_number == -1
&& argvars[1].v_type == VAR_UNKNOWN)
global = TRUE;
else
wp = find_tabwin(&argvars[0], &argvars[1], &tp);
if (wp != NULL && wp->w_localdir != NULL)
rettv->vval.v_string = vim_strsave(wp->w_localdir);
else if (tp != NULL && tp->tp_localdir != NULL)
rettv->vval.v_string = vim_strsave(tp->tp_localdir);
else if (wp != NULL || tp != NULL || global)
{
if (globaldir != NULL)
rettv->vval.v_string = vim_strsave(globaldir);
else
{
cwd = alloc(MAXPATHL);
if (cwd != NULL)
{
if (mch_dirname(cwd, MAXPATHL) != FAIL)
rettv->vval.v_string = vim_strsave(cwd);
vim_free(cwd);
}
}
}
#if defined(BACKSLASH_IN_FILENAME)
if (rettv->vval.v_string != NULL)
slash_adjust(rettv->vval.v_string);
#endif
}
void
f_getfperm(typval_T *argvars, typval_T *rettv)
{
char_u *fname;
stat_T st;
char_u *perm = NULL;
char_u flags[] = "rwx";
int i;
fname = tv_get_string(&argvars[0]);
rettv->v_type = VAR_STRING;
if (mch_stat((char *)fname, &st) >= 0)
{
perm = vim_strsave((char_u *)"---------");
if (perm != NULL)
{
for (i = 0; i < 9; i++)
{
if (st.st_mode & (1 << (8 - i)))
perm[i] = flags[i % 3];
}
}
}
rettv->vval.v_string = perm;
}
void
f_getfsize(typval_T *argvars, typval_T *rettv)
{
char_u *fname;
stat_T st;
fname = tv_get_string(&argvars[0]);
rettv->v_type = VAR_NUMBER;
if (mch_stat((char *)fname, &st) >= 0)
{
if (mch_isdir(fname))
rettv->vval.v_number = 0;
else
{
rettv->vval.v_number = (varnumber_T)st.st_size;
if ((off_T)rettv->vval.v_number != (off_T)st.st_size)
rettv->vval.v_number = -2;
}
}
else
rettv->vval.v_number = -1;
}
void
f_getftime(typval_T *argvars, typval_T *rettv)
{
char_u *fname;
stat_T st;
fname = tv_get_string(&argvars[0]);
if (mch_stat((char *)fname, &st) >= 0)
rettv->vval.v_number = (varnumber_T)st.st_mtime;
else
rettv->vval.v_number = -1;
}
void
f_getftype(typval_T *argvars, typval_T *rettv)
{
char_u *fname;
stat_T st;
char_u *type = NULL;
char *t;
fname = tv_get_string(&argvars[0]);
rettv->v_type = VAR_STRING;
if (mch_lstat((char *)fname, &st) >= 0)
{
if (S_ISREG(st.st_mode))
t = "file";
else if (S_ISDIR(st.st_mode))
t = "dir";
else if (S_ISLNK(st.st_mode))
t = "link";
else if (S_ISBLK(st.st_mode))
t = "bdev";
else if (S_ISCHR(st.st_mode))
t = "cdev";
else if (S_ISFIFO(st.st_mode))
t = "fifo";
else if (S_ISSOCK(st.st_mode))
t = "socket";
else
t = "other";
type = vim_strsave((char_u *)t);
}
rettv->vval.v_string = type;
}
void
f_glob(typval_T *argvars, typval_T *rettv)
{
int options = WILD_SILENT|WILD_USE_NL;
expand_T xpc;
int error = FALSE;
rettv->v_type = VAR_STRING;
if (argvars[1].v_type != VAR_UNKNOWN)
{
if (tv_get_number_chk(&argvars[1], &error))
options |= WILD_KEEP_ALL;
if (argvars[2].v_type != VAR_UNKNOWN)
{
if (tv_get_number_chk(&argvars[2], &error))
rettv_list_set(rettv, NULL);
if (argvars[3].v_type != VAR_UNKNOWN
&& tv_get_number_chk(&argvars[3], &error))
options |= WILD_ALLLINKS;
}
}
if (!error)
{
ExpandInit(&xpc);
xpc.xp_context = EXPAND_FILES;
if (p_wic)
options += WILD_ICASE;
if (rettv->v_type == VAR_STRING)
rettv->vval.v_string = ExpandOne(&xpc, tv_get_string(&argvars[0]),
NULL, options, WILD_ALL);
else if (rettv_list_alloc(rettv) != FAIL)
{
int i;
ExpandOne(&xpc, tv_get_string(&argvars[0]),
NULL, options, WILD_ALL_KEEP);
for (i = 0; i < xpc.xp_numfiles; i++)
list_append_string(rettv->vval.v_list, xpc.xp_files[i], -1);
ExpandCleanup(&xpc);
}
}
else
rettv->vval.v_string = NULL;
}
void
f_glob2regpat(typval_T *argvars, typval_T *rettv)
{
char_u *pat = tv_get_string_chk(&argvars[0]);
rettv->v_type = VAR_STRING;
rettv->vval.v_string = (pat == NULL)
? NULL : file_pat_to_reg_pat(pat, NULL, NULL, FALSE);
}
void
f_globpath(typval_T *argvars, typval_T *rettv)
{
int flags = WILD_IGNORE_COMPLETESLASH;
char_u buf1[NUMBUFLEN];
char_u *file = tv_get_string_buf_chk(&argvars[1], buf1);
int error = FALSE;
garray_T ga;
int i;
rettv->v_type = VAR_STRING;
if (argvars[2].v_type != VAR_UNKNOWN)
{
if (tv_get_number_chk(&argvars[2], &error))
flags |= WILD_KEEP_ALL;
if (argvars[3].v_type != VAR_UNKNOWN)
{
if (tv_get_number_chk(&argvars[3], &error))
rettv_list_set(rettv, NULL);
if (argvars[4].v_type != VAR_UNKNOWN
&& tv_get_number_chk(&argvars[4], &error))
flags |= WILD_ALLLINKS;
}
}
if (file != NULL && !error)
{
ga_init2(&ga, (int)sizeof(char_u *), 10);
globpath(tv_get_string(&argvars[0]), file, &ga, flags);
if (rettv->v_type == VAR_STRING)
rettv->vval.v_string = ga_concat_strings(&ga, "\n");
else if (rettv_list_alloc(rettv) != FAIL)
for (i = 0; i < ga.ga_len; ++i)
list_append_string(rettv->vval.v_list,
((char_u **)(ga.ga_data))[i], -1);
ga_clear_strings(&ga);
}
else
rettv->vval.v_string = NULL;
}
void
f_isdirectory(typval_T *argvars, typval_T *rettv)
{
rettv->vval.v_number = mch_isdir(tv_get_string(&argvars[0]));
}
static int
mkdir_recurse(char_u *dir, int prot)
{
char_u *p;
char_u *updir;
int r = FAIL;
p = gettail_sep(dir);
if (p <= get_past_head(dir))
return OK;
updir = vim_strnsave(dir, (int)(p - dir));
if (updir == NULL)
return FAIL;
if (mch_isdir(updir))
r = OK;
else if (mkdir_recurse(updir, prot) == OK)
r = vim_mkdir_emsg(updir, prot);
vim_free(updir);
return r;
}
void
f_mkdir(typval_T *argvars, typval_T *rettv)
{
char_u *dir;
char_u buf[NUMBUFLEN];
int prot = 0755;
rettv->vval.v_number = FAIL;
if (check_restricted() || check_secure())
return;
dir = tv_get_string_buf(&argvars[0], buf);
if (*dir == NUL)
return;
if (*gettail(dir) == NUL)
*gettail_sep(dir) = NUL;
if (argvars[1].v_type != VAR_UNKNOWN)
{
if (argvars[2].v_type != VAR_UNKNOWN)
{
prot = (int)tv_get_number_chk(&argvars[2], NULL);
if (prot == -1)
return;
}
if (STRCMP(tv_get_string(&argvars[1]), "p") == 0)
{
if (mch_isdir(dir))
{
rettv->vval.v_number = OK;
return;
}
mkdir_recurse(dir, prot);
}
}
rettv->vval.v_number = vim_mkdir_emsg(dir, prot);
}
void
f_pathshorten(typval_T *argvars, typval_T *rettv)
{
char_u *p;
rettv->v_type = VAR_STRING;
p = tv_get_string_chk(&argvars[0]);
if (p == NULL)
rettv->vval.v_string = NULL;
else
{
p = vim_strsave(p);
rettv->vval.v_string = p;
if (p != NULL)
shorten_dir(p);
}
}
static int
readdir_checkitem(void *context, char_u *name)
{
typval_T *expr = (typval_T *)context;
typval_T save_val;
typval_T rettv;
typval_T argv[2];
int retval = 0;
int error = FALSE;
if (expr->v_type == VAR_UNKNOWN)
return 1;
prepare_vimvar(VV_VAL, &save_val);
set_vim_var_string(VV_VAL, name, -1);
argv[0].v_type = VAR_STRING;
argv[0].vval.v_string = name;
if (eval_expr_typval(expr, argv, 1, &rettv) == FAIL)
goto theend;
retval = tv_get_number_chk(&rettv, &error);
if (error)
retval = -1;
clear_tv(&rettv);
theend:
set_vim_var_string(VV_VAL, NULL, 0);
restore_vimvar(VV_VAL, &save_val);
return retval;
}
void
f_readdir(typval_T *argvars, typval_T *rettv)
{
typval_T *expr;
int ret;
char_u *path;
char_u *p;
garray_T ga;
int i;
if (rettv_list_alloc(rettv) == FAIL)
return;
path = tv_get_string(&argvars[0]);
expr = &argvars[1];
ret = readdir_core(&ga, path, (void *)expr, readdir_checkitem);
if (ret == OK && rettv->vval.v_list != NULL && ga.ga_len > 0)
{
for (i = 0; i < ga.ga_len; i++)
{
p = ((char_u **)ga.ga_data)[i];
list_append_string(rettv->vval.v_list, p, -1);
}
}
ga_clear_strings(&ga);
}
void
f_readfile(typval_T *argvars, typval_T *rettv)
{
int binary = FALSE;
int blob = FALSE;
int failed = FALSE;
char_u *fname;
FILE *fd;
char_u buf[(IOSIZE/256)*256]; 
int io_size = sizeof(buf);
int readlen; 
char_u *prev = NULL; 
long prevlen = 0; 
long prevsize = 0; 
long maxline = MAXLNUM;
long cnt = 0;
char_u *p; 
char_u *start; 
if (argvars[1].v_type != VAR_UNKNOWN)
{
if (STRCMP(tv_get_string(&argvars[1]), "b") == 0)
binary = TRUE;
if (STRCMP(tv_get_string(&argvars[1]), "B") == 0)
blob = TRUE;
if (argvars[2].v_type != VAR_UNKNOWN)
maxline = (long)tv_get_number(&argvars[2]);
}
if (blob)
{
if (rettv_blob_alloc(rettv) == FAIL)
return;
}
else
{
if (rettv_list_alloc(rettv) == FAIL)
return;
}
fname = tv_get_string(&argvars[0]);
if (*fname == NUL || (fd = mch_fopen((char *)fname, READBIN)) == NULL)
{
semsg(_(e_notopen), *fname == NUL ? (char_u *)_("<empty>") : fname);
return;
}
if (blob)
{
if (read_blob(fd, rettv->vval.v_blob) == FAIL)
{
emsg("cannot read file");
blob_free(rettv->vval.v_blob);
}
fclose(fd);
return;
}
while (cnt < maxline || maxline < 0)
{
readlen = (int)fread(buf, 1, io_size, fd);
for (p = buf, start = buf;
p < buf + readlen || (readlen <= 0 && (prevlen > 0 || binary));
++p)
{
if (*p == '\n' || readlen <= 0)
{
listitem_T *li;
char_u *s = NULL;
long_u len = p - start;
if (readlen > 0 && !binary)
{
while (len > 0 && start[len - 1] == '\r')
--len;
if (len == 0)
while (prevlen > 0 && prev[prevlen - 1] == '\r')
--prevlen;
}
if (prevlen == 0)
s = vim_strnsave(start, (int)len);
else
{
if ((s = vim_realloc(prev, prevlen + len + 1)) != NULL)
{
mch_memmove(s + prevlen, start, len);
s[prevlen + len] = NUL;
prev = NULL; 
prevlen = prevsize = 0;
}
}
if (s == NULL)
{
do_outofmem_msg((long_u) prevlen + len + 1);
failed = TRUE;
break;
}
if ((li = listitem_alloc()) == NULL)
{
vim_free(s);
failed = TRUE;
break;
}
li->li_tv.v_type = VAR_STRING;
li->li_tv.v_lock = 0;
li->li_tv.vval.v_string = s;
list_append(rettv->vval.v_list, li);
start = p + 1; 
if ((++cnt >= maxline && maxline >= 0) || readlen <= 0)
break;
}
else if (*p == NUL)
*p = '\n';
else if (*p == 0xbf && enc_utf8 && !binary)
{
char_u back1 = p >= buf + 1 ? p[-1]
: prevlen >= 1 ? prev[prevlen - 1] : NUL;
char_u back2 = p >= buf + 2 ? p[-2]
: p == buf + 1 && prevlen >= 1 ? prev[prevlen - 1]
: prevlen >= 2 ? prev[prevlen - 2] : NUL;
if (back2 == 0xef && back1 == 0xbb)
{
char_u *dest = p - 2;
if (start == dest)
start = p + 1;
else
{
int adjust_prevlen = 0;
if (dest < buf)
{
adjust_prevlen = (int)(buf - dest); 
dest = buf;
}
if (readlen > p - buf + 1)
mch_memmove(dest, p + 1, readlen - (p - buf) - 1);
readlen -= 3 - adjust_prevlen;
prevlen -= adjust_prevlen;
p = dest - 1;
}
}
}
} 
if (failed || (cnt >= maxline && maxline >= 0) || readlen <= 0)
break;
if (start < p)
{
if (p - start + prevlen >= prevsize)
{
char_u *newprev;
if (prevsize == 0)
prevsize = (long)(p - start);
else
{
long grow50pc = (prevsize * 3) / 2;
long growmin = (long)((p - start) * 2 + prevlen);
prevsize = grow50pc > growmin ? grow50pc : growmin;
}
newprev = vim_realloc(prev, prevsize);
if (newprev == NULL)
{
do_outofmem_msg((long_u)prevsize);
failed = TRUE;
break;
}
prev = newprev;
}
mch_memmove(prev + prevlen, start, p - start);
prevlen += (long)(p - start);
}
} 
if (!failed && maxline < 0)
while (cnt > -maxline)
{
listitem_remove(rettv->vval.v_list, rettv->vval.v_list->lv_first);
--cnt;
}
if (failed)
{
list_free(rettv->vval.v_list);
rettv_list_alloc(rettv);
}
vim_free(prev);
fclose(fd);
}
void
f_resolve(typval_T *argvars, typval_T *rettv)
{
char_u *p;
#if defined(HAVE_READLINK)
char_u *buf = NULL;
#endif
p = tv_get_string(&argvars[0]);
#if defined(FEAT_SHORTCUT)
{
char_u *v = NULL;
v = mch_resolve_path(p, TRUE);
if (v != NULL)
rettv->vval.v_string = v;
else
rettv->vval.v_string = vim_strsave(p);
}
#else
#if defined(HAVE_READLINK)
{
char_u *cpy;
int len;
char_u *remain = NULL;
char_u *q;
int is_relative_to_current = FALSE;
int has_trailing_pathsep = FALSE;
int limit = 100;
p = vim_strsave(p);
if (p == NULL)
goto fail;
if (p[0] == '.' && (vim_ispathsep(p[1])
|| (p[1] == '.' && (vim_ispathsep(p[2])))))
is_relative_to_current = TRUE;
len = STRLEN(p);
if (len > 0 && after_pathsep(p, p + len))
{
has_trailing_pathsep = TRUE;
p[len - 1] = NUL; 
}
q = getnextcomp(p);
if (*q != NUL)
{
remain = vim_strsave(q - 1);
q[-1] = NUL;
}
buf = alloc(MAXPATHL + 1);
if (buf == NULL)
{
vim_free(p);
goto fail;
}
for (;;)
{
for (;;)
{
len = readlink((char *)p, (char *)buf, MAXPATHL);
if (len <= 0)
break;
buf[len] = NUL;
if (limit-- == 0)
{
vim_free(p);
vim_free(remain);
emsg(_("E655: Too many symbolic links (cycle?)"));
rettv->vval.v_string = NULL;
goto fail;
}
if (remain == NULL && has_trailing_pathsep)
add_pathsep(buf);
q = getnextcomp(vim_ispathsep(*buf) ? buf + 1 : buf);
if (*q != NUL)
{
if (remain == NULL)
remain = vim_strsave(q - 1);
else
{
cpy = concat_str(q - 1, remain);
if (cpy != NULL)
{
vim_free(remain);
remain = cpy;
}
}
q[-1] = NUL;
}
q = gettail(p);
if (q > p && *q == NUL)
{
q[-1] = NUL;
q = gettail(p);
}
if (q > p && !mch_isFullName(buf))
{
cpy = alloc(STRLEN(p) + STRLEN(buf) + 1);
if (cpy != NULL)
{
STRCPY(cpy, p);
STRCPY(gettail(cpy), buf);
vim_free(p);
p = cpy;
}
}
else
{
vim_free(p);
p = vim_strsave(buf);
}
}
if (remain == NULL)
break;
q = getnextcomp(remain + 1);
len = q - remain - (*q != NUL);
cpy = vim_strnsave(p, STRLEN(p) + len);
if (cpy != NULL)
{
STRNCAT(cpy, remain, len);
vim_free(p);
p = cpy;
}
if (*q != NUL)
STRMOVE(remain, q - 1);
else
VIM_CLEAR(remain);
}
if (!vim_ispathsep(*p))
{
if (is_relative_to_current
&& *p != NUL
&& !(p[0] == '.'
&& (p[1] == NUL
|| vim_ispathsep(p[1])
|| (p[1] == '.'
&& (p[2] == NUL
|| vim_ispathsep(p[2]))))))
{
cpy = concat_str((char_u *)"./", p);
if (cpy != NULL)
{
vim_free(p);
p = cpy;
}
}
else if (!is_relative_to_current)
{
q = p;
while (q[0] == '.' && vim_ispathsep(q[1]))
q += 2;
if (q > p)
STRMOVE(p, p + 2);
}
}
if (!has_trailing_pathsep)
{
q = p + STRLEN(p);
if (after_pathsep(p, q))
*gettail_sep(p) = NUL;
}
rettv->vval.v_string = p;
}
#else
rettv->vval.v_string = vim_strsave(p);
#endif
#endif
simplify_filename(rettv->vval.v_string);
#if defined(HAVE_READLINK)
fail:
vim_free(buf);
#endif
rettv->v_type = VAR_STRING;
}
void
f_tempname(typval_T *argvars UNUSED, typval_T *rettv)
{
static int x = 'A';
rettv->v_type = VAR_STRING;
rettv->vval.v_string = vim_tempname(x, FALSE);
do
{
if (x == 'Z')
x = '0';
else if (x == '9')
x = 'A';
else
{
#if defined(EBCDIC)
if (x == 'I')
x = 'J';
else if (x == 'R')
x = 'S';
else
#endif
++x;
}
} while (x == 'I' || x == 'O');
}
void
f_writefile(typval_T *argvars, typval_T *rettv)
{
int binary = FALSE;
int append = FALSE;
#if defined(HAVE_FSYNC)
int do_fsync = p_fs;
#endif
char_u *fname;
FILE *fd;
int ret = 0;
listitem_T *li;
list_T *list = NULL;
blob_T *blob = NULL;
rettv->vval.v_number = -1;
if (check_secure())
return;
if (argvars[0].v_type == VAR_LIST)
{
list = argvars[0].vval.v_list;
if (list == NULL)
return;
range_list_materialize(list);
FOR_ALL_LIST_ITEMS(list, li)
if (tv_get_string_chk(&li->li_tv) == NULL)
return;
}
else if (argvars[0].v_type == VAR_BLOB)
{
blob = argvars[0].vval.v_blob;
if (blob == NULL)
return;
}
else
{
semsg(_(e_invarg2),
_("writefile() first argument must be a List or a Blob"));
return;
}
if (argvars[2].v_type != VAR_UNKNOWN)
{
char_u *arg2 = tv_get_string_chk(&argvars[2]);
if (arg2 == NULL)
return;
if (vim_strchr(arg2, 'b') != NULL)
binary = TRUE;
if (vim_strchr(arg2, 'a') != NULL)
append = TRUE;
#if defined(HAVE_FSYNC)
if (vim_strchr(arg2, 's') != NULL)
do_fsync = TRUE;
else if (vim_strchr(arg2, 'S') != NULL)
do_fsync = FALSE;
#endif
}
fname = tv_get_string_chk(&argvars[1]);
if (fname == NULL)
return;
if (*fname == NUL || (fd = mch_fopen((char *)fname,
append ? APPENDBIN : WRITEBIN)) == NULL)
{
semsg(_(e_notcreate), *fname == NUL ? (char_u *)_("<empty>") : fname);
ret = -1;
}
else if (blob)
{
if (write_blob(fd, blob) == FAIL)
ret = -1;
#if defined(HAVE_FSYNC)
else if (do_fsync)
vim_ignored = vim_fsync(fileno(fd));
#endif
fclose(fd);
}
else
{
if (write_list(fd, list, binary) == FAIL)
ret = -1;
#if defined(HAVE_FSYNC)
else if (do_fsync)
vim_ignored = vim_fsync(fileno(fd));
#endif
fclose(fd);
}
rettv->vval.v_number = ret;
}
#endif 
#if defined(FEAT_BROWSE) || defined(PROTO)
char_u *
do_browse(
int flags, 
char_u *title, 
char_u *dflt, 
char_u *ext, 
char_u *initdir, 
char_u *filter, 
buf_T *buf) 
{
char_u *fname;
static char_u *last_dir = NULL; 
char_u *tofree = NULL;
int save_browse = cmdmod.browse;
cmdmod.browse = FALSE;
if (title == NULL || *title == NUL)
{
if (flags & BROWSE_DIR)
title = (char_u *)_("Select Directory dialog");
else if (flags & BROWSE_SAVE)
title = (char_u *)_("Save File dialog");
else
title = (char_u *)_("Open File dialog");
}
if ((initdir == NULL || *initdir == NUL) && dflt != NULL && *dflt != NUL)
{
if (mch_isdir(dflt)) 
{
initdir = dflt;
dflt = NULL;
}
else if (gettail(dflt) != dflt) 
{
tofree = vim_strsave(dflt);
if (tofree != NULL)
{
initdir = tofree;
*gettail(initdir) = NUL;
dflt = gettail(dflt);
}
}
}
if (initdir == NULL || *initdir == NUL)
{
if (STRCMP(p_bsdir, "last") != 0
&& STRCMP(p_bsdir, "buffer") != 0
&& STRCMP(p_bsdir, "current") != 0
&& mch_isdir(p_bsdir))
initdir = p_bsdir;
else if (((flags & BROWSE_SAVE) || *p_bsdir == 'b')
&& buf != NULL && buf->b_ffname != NULL)
{
if (dflt == NULL || *dflt == NUL)
dflt = gettail(curbuf->b_ffname);
tofree = vim_strsave(curbuf->b_ffname);
if (tofree != NULL)
{
initdir = tofree;
*gettail(initdir) = NUL;
}
}
else if (*p_bsdir == 'l')
initdir = last_dir;
}
#if defined(FEAT_GUI)
if (gui.in_use) 
{
if (filter == NULL
#if defined(FEAT_EVAL)
&& (filter = get_var_value((char_u *)"b:browsefilter")) == NULL
&& (filter = get_var_value((char_u *)"g:browsefilter")) == NULL
#endif
)
filter = BROWSE_FILTER_DEFAULT;
if (flags & BROWSE_DIR)
{
#if defined(FEAT_GUI_GTK) || defined(MSWIN)
fname = gui_mch_browsedir(title, initdir);
#else
fname = gui_mch_browse(0, title, dflt, ext, initdir, (char_u *)"");
#endif
#if !defined(FEAT_GUI_GTK)
if (fname != NULL && *fname != NUL && !mch_isdir(fname))
{
char_u *tail = gettail_sep(fname);
if (tail == fname)
*tail++ = '.'; 
*tail = NUL;
}
#endif
}
else
fname = gui_mch_browse(flags & BROWSE_SAVE,
title, dflt, ext, initdir, (char_u *)_(filter));
need_check_timestamps = TRUE;
did_check_timestamps = FALSE;
}
else
#endif
{
emsg(_("E338: Sorry, no file browser in console mode"));
fname = NULL;
}
if (fname != NULL)
{
vim_free(last_dir);
last_dir = vim_strsave(fname);
if (last_dir != NULL && !(flags & BROWSE_DIR))
{
*gettail(last_dir) = NUL;
if (*last_dir == NUL)
{
vim_free(last_dir);
last_dir = alloc(MAXPATHL);
if (last_dir != NULL)
mch_dirname(last_dir, MAXPATHL);
}
}
}
vim_free(tofree);
cmdmod.browse = save_browse;
return fname;
}
#endif
#if defined(FEAT_EVAL) || defined(PROTO)
void
f_browse(typval_T *argvars UNUSED, typval_T *rettv)
{
#if defined(FEAT_BROWSE)
int save;
char_u *title;
char_u *initdir;
char_u *defname;
char_u buf[NUMBUFLEN];
char_u buf2[NUMBUFLEN];
int error = FALSE;
save = (int)tv_get_number_chk(&argvars[0], &error);
title = tv_get_string_chk(&argvars[1]);
initdir = tv_get_string_buf_chk(&argvars[2], buf);
defname = tv_get_string_buf_chk(&argvars[3], buf2);
if (error || title == NULL || initdir == NULL || defname == NULL)
rettv->vval.v_string = NULL;
else
rettv->vval.v_string =
do_browse(save ? BROWSE_SAVE : 0,
title, defname, NULL, initdir, NULL, curbuf);
#else
rettv->vval.v_string = NULL;
#endif
rettv->v_type = VAR_STRING;
}
void
f_browsedir(typval_T *argvars UNUSED, typval_T *rettv)
{
#if defined(FEAT_BROWSE)
char_u *title;
char_u *initdir;
char_u buf[NUMBUFLEN];
title = tv_get_string_chk(&argvars[0]);
initdir = tv_get_string_buf_chk(&argvars[1], buf);
if (title == NULL || initdir == NULL)
rettv->vval.v_string = NULL;
else
rettv->vval.v_string = do_browse(BROWSE_DIR,
title, NULL, NULL, initdir, NULL, curbuf);
#else
rettv->vval.v_string = NULL;
#endif
rettv->v_type = VAR_STRING;
}
#endif 
void
home_replace(
buf_T *buf, 
char_u *src, 
char_u *dst, 
int dstlen, 
int one) 
{
size_t dirlen = 0, envlen = 0;
size_t len;
char_u *homedir_env, *homedir_env_orig;
char_u *p;
if (src == NULL)
{
*dst = NUL;
return;
}
if (buf != NULL && buf->b_help)
{
vim_snprintf((char *)dst, dstlen, "%s", gettail(src));
return;
}
if (homedir != NULL)
dirlen = STRLEN(homedir);
#if defined(VMS)
homedir_env_orig = homedir_env = mch_getenv((char_u *)"SYS$LOGIN");
#else
homedir_env_orig = homedir_env = mch_getenv((char_u *)"HOME");
#endif
#if defined(MSWIN)
if (homedir_env == NULL)
homedir_env_orig = homedir_env = mch_getenv((char_u *)"USERPROFILE");
#endif
if (homedir_env != NULL && *homedir_env == NUL)
homedir_env = NULL;
if (homedir_env != NULL && *homedir_env == '~')
{
int usedlen = 0;
int flen;
char_u *fbuf = NULL;
flen = (int)STRLEN(homedir_env);
(void)modify_fname((char_u *)":p", FALSE, &usedlen,
&homedir_env, &fbuf, &flen);
flen = (int)STRLEN(homedir_env);
if (flen > 0 && vim_ispathsep(homedir_env[flen - 1]))
homedir_env[flen - 1] = NUL;
}
if (homedir_env != NULL)
envlen = STRLEN(homedir_env);
if (!one)
src = skipwhite(src);
while (*src && dstlen > 0)
{
p = homedir;
len = dirlen;
for (;;)
{
if ( len
&& fnamencmp(src, p, len) == 0
&& (vim_ispathsep(src[len])
|| (!one && (src[len] == ',' || src[len] == ' '))
|| src[len] == NUL))
{
src += len;
if (--dstlen > 0)
*dst++ = '~';
if (!vim_ispathsep(src[0]) && --dstlen > 0)
*dst++ = '/';
break;
}
if (p == homedir_env)
break;
p = homedir_env;
len = envlen;
}
while (*src && (one || (*src != ',' && *src != ' ')) && --dstlen > 0)
*dst++ = *src++;
while ((*src == ' ' || *src == ',') && --dstlen > 0)
*dst++ = *src++;
}
*dst = NUL;
if (homedir_env != homedir_env_orig)
vim_free(homedir_env);
}
char_u *
home_replace_save(
buf_T *buf, 
char_u *src) 
{
char_u *dst;
unsigned len;
len = 3; 
if (src != NULL) 
len += (unsigned)STRLEN(src);
dst = alloc(len);
if (dst != NULL)
home_replace(buf, src, dst, len, TRUE);
return dst;
}
int
fullpathcmp(
char_u *s1,
char_u *s2,
int checkname, 
int expandenv)
{
#if defined(UNIX)
char_u exp1[MAXPATHL];
char_u full1[MAXPATHL];
char_u full2[MAXPATHL];
stat_T st1, st2;
int r1, r2;
if (expandenv)
expand_env(s1, exp1, MAXPATHL);
else
vim_strncpy(exp1, s1, MAXPATHL - 1);
r1 = mch_stat((char *)exp1, &st1);
r2 = mch_stat((char *)s2, &st2);
if (r1 != 0 && r2 != 0)
{
if (checkname)
{
if (fnamecmp(exp1, s2) == 0)
return FPC_SAMEX;
r1 = vim_FullName(exp1, full1, MAXPATHL, FALSE);
r2 = vim_FullName(s2, full2, MAXPATHL, FALSE);
if (r1 == OK && r2 == OK && fnamecmp(full1, full2) == 0)
return FPC_SAMEX;
}
return FPC_NOTX;
}
if (r1 != 0 || r2 != 0)
return FPC_DIFFX;
if (st1.st_dev == st2.st_dev && st1.st_ino == st2.st_ino)
return FPC_SAME;
return FPC_DIFF;
#else
char_u *exp1; 
char_u *full1; 
char_u *full2; 
int retval = FPC_DIFF;
int r1, r2;
if ((exp1 = alloc(MAXPATHL * 3)) != NULL)
{
full1 = exp1 + MAXPATHL;
full2 = full1 + MAXPATHL;
if (expandenv)
expand_env(s1, exp1, MAXPATHL);
else
vim_strncpy(exp1, s1, MAXPATHL - 1);
r1 = vim_FullName(exp1, full1, MAXPATHL, FALSE);
r2 = vim_FullName(s2, full2, MAXPATHL, FALSE);
if (r1 != OK && r2 != OK)
{
if (checkname && fnamecmp(exp1, s2) == 0)
retval = FPC_SAMEX;
else
retval = FPC_NOTX;
}
else if (r1 != OK || r2 != OK)
retval = FPC_DIFFX;
else if (fnamecmp(full1, full2))
retval = FPC_DIFF;
else
retval = FPC_SAME;
vim_free(exp1);
}
return retval;
#endif
}
char_u *
gettail(char_u *fname)
{
char_u *p1, *p2;
if (fname == NULL)
return (char_u *)"";
for (p1 = p2 = get_past_head(fname); *p2; ) 
{
if (vim_ispathsep_nocolon(*p2))
p1 = p2 + 1;
MB_PTR_ADV(p2);
}
return p1;
}
char_u *
gettail_sep(char_u *fname)
{
char_u *p;
char_u *t;
p = get_past_head(fname); 
t = gettail(fname);
while (t > p && after_pathsep(fname, t))
--t;
#if defined(VMS)
++t;
#endif
return t;
}
char_u *
getnextcomp(char_u *fname)
{
while (*fname && !vim_ispathsep(*fname))
MB_PTR_ADV(fname);
if (*fname)
++fname;
return fname;
}
char_u *
get_past_head(char_u *path)
{
char_u *retval;
#if defined(MSWIN)
if (isalpha(path[0]) && path[1] == ':')
retval = path + 2;
else
retval = path;
#else
#if defined(AMIGA)
retval = vim_strchr(path, ':');
if (retval == NULL)
retval = path;
#else 
retval = path;
#endif
#endif
while (vim_ispathsep(*retval))
++retval;
return retval;
}
int
vim_ispathsep(int c)
{
#if defined(UNIX)
return (c == '/'); 
#else
#if defined(BACKSLASH_IN_FILENAME)
return (c == ':' || c == '/' || c == '\\');
#else
#if defined(VMS)
return (c == ':' || c == '[' || c == ']' || c == '/'
|| c == '<' || c == '>' || c == '"' );
#else
return (c == ':' || c == '/');
#endif 
#endif
#endif
}
int
vim_ispathsep_nocolon(int c)
{
return vim_ispathsep(c)
#if defined(BACKSLASH_IN_FILENAME)
&& c != ':'
#endif
;
}
void
shorten_dir(char_u *str)
{
char_u *tail, *s, *d;
int skip = FALSE;
tail = gettail(str);
d = str;
for (s = str; ; ++s)
{
if (s >= tail) 
{
*d++ = *s;
if (*s == NUL)
break;
}
else if (vim_ispathsep(*s)) 
{
*d++ = *s;
skip = FALSE;
}
else if (!skip)
{
*d++ = *s; 
if (*s != '~' && *s != '.') 
skip = TRUE;
if (has_mbyte)
{
int l = mb_ptr2len(s);
while (--l > 0)
*d++ = *++s;
}
}
}
}
int
dir_of_file_exists(char_u *fname)
{
char_u *p;
int c;
int retval;
p = gettail_sep(fname);
if (p == fname)
return TRUE;
c = *p;
*p = NUL;
retval = mch_isdir(fname);
*p = c;
return retval;
}
int
vim_fnamecmp(char_u *x, char_u *y)
{
#if defined(BACKSLASH_IN_FILENAME)
return vim_fnamencmp(x, y, MAXPATHL);
#else
if (p_fic)
return MB_STRICMP(x, y);
return STRCMP(x, y);
#endif
}
int
vim_fnamencmp(char_u *x, char_u *y, size_t len)
{
#if defined(BACKSLASH_IN_FILENAME)
char_u *px = x;
char_u *py = y;
int cx = NUL;
int cy = NUL;
while (len > 0)
{
cx = PTR2CHAR(px);
cy = PTR2CHAR(py);
if (cx == NUL || cy == NUL
|| ((p_fic ? MB_TOLOWER(cx) != MB_TOLOWER(cy) : cx != cy)
&& !(cx == '/' && cy == '\\')
&& !(cx == '\\' && cy == '/')))
break;
len -= mb_ptr2len(px);
px += mb_ptr2len(px);
py += mb_ptr2len(py);
}
if (len == 0)
return 0;
return (cx - cy);
#else
if (p_fic)
return MB_STRNICMP(x, y, len);
return STRNCMP(x, y, len);
#endif
}
char_u *
concat_fnames(char_u *fname1, char_u *fname2, int sep)
{
char_u *dest;
dest = alloc(STRLEN(fname1) + STRLEN(fname2) + 3);
if (dest != NULL)
{
STRCPY(dest, fname1);
if (sep)
add_pathsep(dest);
STRCAT(dest, fname2);
}
return dest;
}
void
add_pathsep(char_u *p)
{
if (*p != NUL && !after_pathsep(p, p + STRLEN(p)))
STRCAT(p, PATHSEPSTR);
}
char_u *
FullName_save(
char_u *fname,
int force) 
{
char_u *buf;
char_u *new_fname = NULL;
if (fname == NULL)
return NULL;
buf = alloc(MAXPATHL);
if (buf != NULL)
{
if (vim_FullName(fname, buf, MAXPATHL, force) != FAIL)
new_fname = vim_strsave(buf);
else
new_fname = vim_strsave(fname);
vim_free(buf);
}
return new_fname;
}
int
vim_fexists(char_u *fname)
{
stat_T st;
if (mch_stat((char *)fname, &st))
return FALSE;
return TRUE;
}
int
expand_wildcards_eval(
char_u **pat, 
int *num_file, 
char_u ***file, 
int flags) 
{
int ret = FAIL;
char_u *eval_pat = NULL;
char_u *exp_pat = *pat;
char *ignored_msg;
int usedlen;
if (*exp_pat == '%' || *exp_pat == '#' || *exp_pat == '<')
{
++emsg_off;
eval_pat = eval_vars(exp_pat, exp_pat, &usedlen,
NULL, &ignored_msg, NULL);
--emsg_off;
if (eval_pat != NULL)
exp_pat = concat_str(eval_pat, exp_pat + usedlen);
}
if (exp_pat != NULL)
ret = expand_wildcards(1, &exp_pat, num_file, file, flags);
if (eval_pat != NULL)
{
vim_free(exp_pat);
vim_free(eval_pat);
}
return ret;
}
int
expand_wildcards(
int num_pat, 
char_u **pat, 
int *num_files, 
char_u ***files, 
int flags) 
{
int retval;
int i, j;
char_u *p;
int non_suf_match; 
retval = gen_expand_wildcards(num_pat, pat, num_files, files, flags);
if ((flags & EW_KEEPALL) || retval == FAIL)
return retval;
#if defined(FEAT_WILDIGN)
if (*p_wig)
{
char_u *ffname;
for (i = 0; i < *num_files; ++i)
{
ffname = FullName_save((*files)[i], FALSE);
if (ffname == NULL) 
break;
#if defined(VMS)
vms_remove_version(ffname);
#endif
if (match_file_list(p_wig, (*files)[i], ffname))
{
vim_free((*files)[i]);
for (j = i; j + 1 < *num_files; ++j)
(*files)[j] = (*files)[j + 1];
--*num_files;
--i;
}
vim_free(ffname);
}
if (*num_files == 0)
{
VIM_CLEAR(*files);
return FAIL;
}
}
#endif
if (*num_files > 1)
{
non_suf_match = 0;
for (i = 0; i < *num_files; ++i)
{
if (!match_suffix((*files)[i]))
{
p = (*files)[i];
for (j = i; j > non_suf_match; --j)
(*files)[j] = (*files)[j - 1];
(*files)[non_suf_match++] = p;
}
}
}
return retval;
}
int
match_suffix(char_u *fname)
{
int fnamelen, setsuflen;
char_u *setsuf;
#define MAXSUFLEN 30 
char_u suf_buf[MAXSUFLEN];
fnamelen = (int)STRLEN(fname);
setsuflen = 0;
for (setsuf = p_su; *setsuf; )
{
setsuflen = copy_option_part(&setsuf, suf_buf, MAXSUFLEN, ".,");
if (setsuflen == 0)
{
char_u *tail = gettail(fname);
if (vim_strchr(tail, '.') == NULL)
{
setsuflen = 1;
break;
}
}
else
{
if (fnamelen >= setsuflen
&& fnamencmp(suf_buf, fname + fnamelen - setsuflen,
(size_t)setsuflen) == 0)
break;
setsuflen = 0;
}
}
return (setsuflen != 0);
}
#if defined(VIM_BACKTICK)
static int
vim_backtick(char_u *p)
{
return (*p == '`' && *(p + 1) != NUL && *(p + STRLEN(p) - 1) == '`');
}
static int
expand_backtick(
garray_T *gap,
char_u *pat,
int flags) 
{
char_u *p;
char_u *cmd;
char_u *buffer;
int cnt = 0;
int i;
cmd = vim_strnsave(pat + 1, (int)STRLEN(pat) - 2);
if (cmd == NULL)
return -1;
#if defined(FEAT_EVAL)
if (*cmd == '=') 
buffer = eval_to_string(cmd + 1, &p, TRUE);
else
#endif
buffer = get_cmd_output(cmd, NULL,
(flags & EW_SILENT) ? SHELL_SILENT : 0, NULL);
vim_free(cmd);
if (buffer == NULL)
return -1;
cmd = buffer;
while (*cmd != NUL)
{
cmd = skipwhite(cmd); 
p = cmd;
while (*p != NUL && *p != '\r' && *p != '\n') 
++p;
if (p > cmd)
{
i = *p;
*p = NUL;
addfile(gap, cmd, flags);
*p = i;
++cnt;
}
cmd = p;
while (*cmd != NUL && (*cmd == '\r' || *cmd == '\n'))
++cmd;
}
vim_free(buffer);
return cnt;
}
#endif 
#if defined(MSWIN)
static int
pstrcmp(const void *a, const void *b)
{
return (pathcmp(*(char **)a, *(char **)b, -1));
}
static int
dos_expandpath(
garray_T *gap,
char_u *path,
int wildoff,
int flags, 
int didstar) 
{
char_u *buf;
char_u *path_end;
char_u *p, *s, *e;
int start_len = gap->ga_len;
char_u *pat;
regmatch_T regmatch;
int starts_with_dot;
int matches;
int len;
int starstar = FALSE;
static int stardepth = 0; 
HANDLE hFind = INVALID_HANDLE_VALUE;
WIN32_FIND_DATAW wfb;
WCHAR *wn = NULL; 
char_u *matchname;
int ok;
if (stardepth > 0)
{
ui_breakcheck();
if (got_int)
return 0;
}
buf = alloc(MAXPATHL);
if (buf == NULL)
return 0;
p = buf;
s = buf;
e = NULL;
path_end = path;
while (*path_end != NUL)
{
if (path_end >= path + wildoff && rem_backslash(path_end))
*p++ = *path_end++;
else if (*path_end == '\\' || *path_end == ':' || *path_end == '/')
{
if (e != NULL)
break;
s = p + 1;
}
else if (path_end >= path + wildoff
&& vim_strchr((char_u *)"*?[~", *path_end) != NULL)
e = p;
if (has_mbyte)
{
len = (*mb_ptr2len)(path_end);
STRNCPY(p, path_end, len);
p += len;
path_end += len;
}
else
*p++ = *path_end++;
}
e = p;
*e = NUL;
for (p = buf + wildoff; p < s; ++p)
if (rem_backslash(p))
{
STRMOVE(p, p + 1);
--e;
--s;
}
for (p = s; p < e; ++p)
if (p[0] == '*' && p[1] == '*')
starstar = TRUE;
starts_with_dot = *s == '.';
pat = file_pat_to_reg_pat(s, e, NULL, FALSE);
if (pat == NULL)
{
vim_free(buf);
return 0;
}
if (flags & (EW_NOERROR | EW_NOTWILD))
++emsg_silent;
regmatch.rm_ic = TRUE; 
regmatch.regprog = vim_regcomp(pat, RE_MAGIC);
if (flags & (EW_NOERROR | EW_NOTWILD))
--emsg_silent;
vim_free(pat);
if (regmatch.regprog == NULL && (flags & EW_NOTWILD) == 0)
{
vim_free(buf);
return 0;
}
matchname = vim_strsave(s);
if (!didstar && stardepth < 100 && starstar && e - s == 2
&& *path_end == '/')
{
STRCPY(s, path_end + 1);
++stardepth;
(void)dos_expandpath(gap, buf, (int)(s - buf), flags, TRUE);
--stardepth;
}
STRCPY(s, "*.*");
wn = enc_to_utf16(buf, NULL);
if (wn != NULL)
hFind = FindFirstFileW(wn, &wfb);
ok = (hFind != INVALID_HANDLE_VALUE);
while (ok)
{
p = utf16_to_enc(wfb.cFileName, NULL); 
if (p == NULL)
break; 
if ((p[0] != '.' || starts_with_dot
|| ((flags & EW_DODOT)
&& p[1] != NUL && (p[1] != '.' || p[2] != NUL)))
&& (matchname == NULL
|| (regmatch.regprog != NULL
&& vim_regexec(&regmatch, p, (colnr_T)0))
|| ((flags & EW_NOTWILD)
&& fnamencmp(path + (s - buf), p, e - s) == 0)))
{
STRCPY(s, p);
len = (int)STRLEN(buf);
if (starstar && stardepth < 100)
{
STRCPY(buf + len, "/**");
STRCPY(buf + len + 3, path_end);
++stardepth;
(void)dos_expandpath(gap, buf, len + 1, flags, TRUE);
--stardepth;
}
STRCPY(buf + len, path_end);
if (mch_has_exp_wildcard(path_end))
{
(void)dos_expandpath(gap, buf, len + 1, flags, FALSE);
}
else
{
if (*path_end != 0)
backslash_halve(buf + len + 1);
if (mch_getperm(buf) >= 0) 
addfile(gap, buf, flags);
}
}
vim_free(p);
ok = FindNextFileW(hFind, &wfb);
if (!ok && matchname != NULL && gap->ga_len == start_len)
{
STRCPY(s, matchname);
FindClose(hFind);
vim_free(wn);
wn = enc_to_utf16(buf, NULL);
if (wn != NULL)
hFind = FindFirstFileW(wn, &wfb);
else
hFind = INVALID_HANDLE_VALUE;
ok = (hFind != INVALID_HANDLE_VALUE);
VIM_CLEAR(matchname);
}
}
FindClose(hFind);
vim_free(wn);
vim_free(buf);
vim_regfree(regmatch.regprog);
vim_free(matchname);
matches = gap->ga_len - start_len;
if (matches > 0)
qsort(((char_u **)gap->ga_data) + start_len, (size_t)matches,
sizeof(char_u *), pstrcmp);
return matches;
}
int
mch_expandpath(
garray_T *gap,
char_u *path,
int flags) 
{
return dos_expandpath(gap, path, 0, flags, FALSE);
}
#endif 
#if (defined(UNIX) && !defined(VMS)) || defined(USE_UNIXFILENAME) || defined(PROTO)
static int
pstrcmp(const void *a, const void *b)
{
return (pathcmp(*(char **)a, *(char **)b, -1));
}
int
unix_expandpath(
garray_T *gap,
char_u *path,
int wildoff,
int flags, 
int didstar) 
{
char_u *buf;
char_u *path_end;
char_u *p, *s, *e;
int start_len = gap->ga_len;
char_u *pat;
regmatch_T regmatch;
int starts_with_dot;
int matches;
int len;
int starstar = FALSE;
static int stardepth = 0; 
DIR *dirp;
struct dirent *dp;
if (stardepth > 0)
{
ui_breakcheck();
if (got_int)
return 0;
}
buf = alloc(STRLEN(path) + BASENAMELEN + 5);
if (buf == NULL)
return 0;
p = buf;
s = buf;
e = NULL;
path_end = path;
while (*path_end != NUL)
{
if (path_end >= path + wildoff && rem_backslash(path_end))
*p++ = *path_end++;
else if (*path_end == '/')
{
if (e != NULL)
break;
s = p + 1;
}
else if (path_end >= path + wildoff
&& (vim_strchr((char_u *)"*?[{~$", *path_end) != NULL
|| (!p_fic && (flags & EW_ICASE)
&& isalpha(PTR2CHAR(path_end)))))
e = p;
if (has_mbyte)
{
len = (*mb_ptr2len)(path_end);
STRNCPY(p, path_end, len);
p += len;
path_end += len;
}
else
*p++ = *path_end++;
}
e = p;
*e = NUL;
for (p = buf + wildoff; p < s; ++p)
if (rem_backslash(p))
{
STRMOVE(p, p + 1);
--e;
--s;
}
for (p = s; p < e; ++p)
if (p[0] == '*' && p[1] == '*')
starstar = TRUE;
starts_with_dot = *s == '.';
pat = file_pat_to_reg_pat(s, e, NULL, FALSE);
if (pat == NULL)
{
vim_free(buf);
return 0;
}
if (flags & EW_ICASE)
regmatch.rm_ic = TRUE; 
else
regmatch.rm_ic = p_fic; 
if (flags & (EW_NOERROR | EW_NOTWILD))
++emsg_silent;
regmatch.regprog = vim_regcomp(pat, RE_MAGIC);
if (flags & (EW_NOERROR | EW_NOTWILD))
--emsg_silent;
vim_free(pat);
if (regmatch.regprog == NULL && (flags & EW_NOTWILD) == 0)
{
vim_free(buf);
return 0;
}
if (!didstar && stardepth < 100 && starstar && e - s == 2
&& *path_end == '/')
{
STRCPY(s, path_end + 1);
++stardepth;
(void)unix_expandpath(gap, buf, (int)(s - buf), flags, TRUE);
--stardepth;
}
*s = NUL;
dirp = opendir(*buf == NUL ? "." : (char *)buf);
if (dirp != NULL)
{
for (;;)
{
dp = readdir(dirp);
if (dp == NULL)
break;
if ((dp->d_name[0] != '.' || starts_with_dot
|| ((flags & EW_DODOT)
&& dp->d_name[1] != NUL
&& (dp->d_name[1] != '.' || dp->d_name[2] != NUL)))
&& ((regmatch.regprog != NULL && vim_regexec(&regmatch,
(char_u *)dp->d_name, (colnr_T)0))
|| ((flags & EW_NOTWILD)
&& fnamencmp(path + (s - buf), dp->d_name, e - s) == 0)))
{
STRCPY(s, dp->d_name);
len = STRLEN(buf);
if (starstar && stardepth < 100)
{
STRCPY(buf + len, "/**");
STRCPY(buf + len + 3, path_end);
++stardepth;
(void)unix_expandpath(gap, buf, len + 1, flags, TRUE);
--stardepth;
}
STRCPY(buf + len, path_end);
if (mch_has_exp_wildcard(path_end)) 
{
(void)unix_expandpath(gap, buf, len + 1, flags, FALSE);
}
else
{
stat_T sb;
if (*path_end != NUL)
backslash_halve(buf + len + 1);
if ((flags & EW_ALLLINKS) ? mch_lstat((char *)buf, &sb) >= 0
: mch_getperm(buf) >= 0)
{
#if defined(MACOS_CONVERT)
size_t precomp_len = STRLEN(buf)+1;
char_u *precomp_buf =
mac_precompose_path(buf, precomp_len, &precomp_len);
if (precomp_buf)
{
mch_memmove(buf, precomp_buf, precomp_len);
vim_free(precomp_buf);
}
#endif
addfile(gap, buf, flags);
}
}
}
}
closedir(dirp);
}
vim_free(buf);
vim_regfree(regmatch.regprog);
matches = gap->ga_len - start_len;
if (matches > 0)
qsort(((char_u **)gap->ga_data) + start_len, matches,
sizeof(char_u *), pstrcmp);
return matches;
}
#endif
static int
has_env_var(char_u *p)
{
for ( ; *p; MB_PTR_ADV(p))
{
if (*p == '\\' && p[1] != NUL)
++p;
else if (vim_strchr((char_u *)
#if defined(MSWIN)
"$%"
#else
"$"
#endif
, *p) != NULL)
return TRUE;
}
return FALSE;
}
#if defined(SPECIAL_WILDCHAR)
static int
has_special_wildchar(char_u *p)
{
for ( ; *p; MB_PTR_ADV(p))
{
if (*p == '\r' || *p == '\n')
break;
if (*p == '\\' && p[1] != NUL && p[1] != '\r' && p[1] != '\n')
++p;
else if (vim_strchr((char_u *)SPECIAL_WILDCHAR, *p) != NULL)
{
if (*p == '{' && vim_strchr(p, '}') == NULL)
continue;
if ((*p == '`' || *p == '\'') && vim_strchr(p, *p) == NULL)
continue;
return TRUE;
}
}
return FALSE;
}
#endif
int
gen_expand_wildcards(
int num_pat, 
char_u **pat, 
int *num_file, 
char_u ***file, 
int flags) 
{
int i;
garray_T ga;
char_u *p;
static int recursive = FALSE;
int add_pat;
int retval = OK;
#if defined(FEAT_SEARCHPATH)
int did_expand_in_path = FALSE;
#endif
if (recursive)
#if defined(SPECIAL_WILDCHAR)
return mch_expand_wildcards(num_pat, pat, num_file, file, flags);
#else
return FAIL;
#endif
#if defined(SPECIAL_WILDCHAR)
for (i = 0; i < num_pat; i++)
{
if (has_special_wildchar(pat[i])
#if defined(VIM_BACKTICK)
&& !(vim_backtick(pat[i]) && pat[i][1] == '=')
#endif
)
return mch_expand_wildcards(num_pat, pat, num_file, file, flags);
}
#endif
recursive = TRUE;
ga_init2(&ga, (int)sizeof(char_u *), 30);
for (i = 0; i < num_pat; ++i)
{
add_pat = -1;
p = pat[i];
#if defined(VIM_BACKTICK)
if (vim_backtick(p))
{
add_pat = expand_backtick(&ga, p, flags);
if (add_pat == -1)
retval = FAIL;
}
else
#endif
{
if ((has_env_var(p) && !(flags & EW_NOTENV)) || *p == '~')
{
p = expand_env_save_opt(p, TRUE);
if (p == NULL)
p = pat[i];
#if defined(UNIX)
else if (has_env_var(p) || *p == '~')
{
vim_free(p);
ga_clear_strings(&ga);
i = mch_expand_wildcards(num_pat, pat, num_file, file,
flags|EW_KEEPDOLLAR);
recursive = FALSE;
return i;
}
#endif
}
if (mch_has_exp_wildcard(p))
{
#if defined(FEAT_SEARCHPATH)
if ((flags & EW_PATH)
&& !mch_isFullName(p)
&& !(p[0] == '.'
&& (vim_ispathsep(p[1])
|| (p[1] == '.' && vim_ispathsep(p[2]))))
)
{
recursive = FALSE;
add_pat = expand_in_path(&ga, p, flags);
recursive = TRUE;
did_expand_in_path = TRUE;
}
else
#endif
add_pat = mch_expandpath(&ga, p, flags);
}
}
if (add_pat == -1 || (add_pat == 0 && (flags & EW_NOTFOUND)))
{
char_u *t = backslash_halve_save(p);
if (flags & EW_NOTFOUND)
addfile(&ga, t, flags | EW_DIR | EW_FILE);
else
addfile(&ga, t, flags);
if (t != p)
vim_free(t);
}
#if defined(FEAT_SEARCHPATH)
if (did_expand_in_path && ga.ga_len > 0 && (flags & EW_PATH))
uniquefy_paths(&ga, p);
#endif
if (p != pat[i])
vim_free(p);
}
*num_file = ga.ga_len;
*file = (ga.ga_data != NULL) ? (char_u **)ga.ga_data : (char_u **)"";
recursive = FALSE;
return ((flags & EW_EMPTYOK) || ga.ga_data != NULL) ? retval : FAIL;
}
void
addfile(
garray_T *gap,
char_u *f, 
int flags)
{
char_u *p;
int isdir;
stat_T sb;
if (!(flags & EW_NOTFOUND) && ((flags & EW_ALLLINKS)
? mch_lstat((char *)f, &sb) < 0 : mch_getperm(f) < 0))
return;
#if defined(FNAME_ILLEGAL)
if (vim_strpbrk(f, (char_u *)FNAME_ILLEGAL) != NULL)
return;
#endif
isdir = mch_isdir(f);
if ((isdir && !(flags & EW_DIR)) || (!isdir && !(flags & EW_FILE)))
return;
if (!isdir && (flags & EW_EXEC)
&& !mch_can_exe(f, NULL, !(flags & EW_SHELLCMD)))
return;
if (ga_grow(gap, 1) == FAIL)
return;
p = alloc(STRLEN(f) + 1 + isdir);
if (p == NULL)
return;
STRCPY(p, f);
#if defined(BACKSLASH_IN_FILENAME)
slash_adjust(p);
#endif
#if !defined(DONT_ADD_PATHSEP_TO_DIR)
if (isdir && (flags & EW_ADDSLASH))
add_pathsep(p);
#endif
((char_u **)gap->ga_data)[gap->ga_len++] = p;
}
void
FreeWild(int count, char_u **files)
{
if (count <= 0 || files == NULL)
return;
while (count--)
vim_free(files[count]);
vim_free(files);
}
int
pathcmp(const char *p, const char *q, int maxlen)
{
int i, j;
int c1, c2;
const char *s = NULL;
for (i = 0, j = 0; maxlen < 0 || (i < maxlen && j < maxlen);)
{
c1 = PTR2CHAR((char_u *)p + i);
c2 = PTR2CHAR((char_u *)q + j);
if (c1 == NUL)
{
if (c2 == NUL) 
return 0;
s = q;
i = j;
break;
}
if (c2 == NUL)
{
s = p;
break;
}
if ((p_fic ? MB_TOUPPER(c1) != MB_TOUPPER(c2) : c1 != c2)
#if defined(BACKSLASH_IN_FILENAME)
&& !((c1 == '/' && c2 == '\\')
|| (c1 == '\\' && c2 == '/'))
#endif
)
{
if (vim_ispathsep(c1))
return -1;
if (vim_ispathsep(c2))
return 1;
return p_fic ? MB_TOUPPER(c1) - MB_TOUPPER(c2)
: c1 - c2; 
}
i += mb_ptr2len((char_u *)p + i);
j += mb_ptr2len((char_u *)q + j);
}
if (s == NULL) 
return 0;
c1 = PTR2CHAR((char_u *)s + i);
c2 = PTR2CHAR((char_u *)s + i + mb_ptr2len((char_u *)s + i));
if (c2 == NUL
&& i > 0
&& !after_pathsep((char_u *)s, (char_u *)s + i)
#if defined(BACKSLASH_IN_FILENAME)
&& (c1 == '/' || c1 == '\\')
#else
&& c1 == '/'
#endif
)
return 0; 
if (s == q)
return -1; 
return 1;
}
int
vim_isAbsName(char_u *name)
{
return (path_with_url(name) != 0 || mch_isFullName(name));
}
int
vim_FullName(
char_u *fname,
char_u *buf,
int len,
int force) 
{
int retval = OK;
int url;
*buf = NUL;
if (fname == NULL)
return FAIL;
url = path_with_url(fname);
if (!url)
retval = mch_FullName(fname, buf, len, force);
if (url || retval == FAIL)
{
vim_strncpy(buf, fname, len - 1);
}
#if defined(MSWIN)
slash_adjust(buf);
#endif
return retval;
}
