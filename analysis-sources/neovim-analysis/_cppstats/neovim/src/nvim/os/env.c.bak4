




#include <assert.h>
#include <uv.h>

#include "nvim/vim.h"
#include "nvim/ascii.h"
#include "nvim/charset.h"
#include "nvim/fileio.h"
#include "nvim/os/os.h"
#include "nvim/memory.h"
#include "nvim/message.h"
#include "nvim/path.h"
#include "nvim/macros.h"
#include "nvim/strings.h"
#include "nvim/eval.h"
#include "nvim/ex_getln.h"
#include "nvim/version.h"
#include "nvim/map.h"

#if defined(WIN32)
#include "nvim/mbyte.h" 
#endif

#if defined(HAVE__NSGETENVIRON)
#include <crt_externs.h>
#endif

#if defined(HAVE_SYS_UTSNAME_H)
#include <sys/utsname.h>
#endif



static PMap(cstr_t) *envmap;
static uv_mutex_t mutex;

void env_init(void)
{
envmap = pmap_new(cstr_t)();
uv_mutex_init(&mutex);
}

void os_env_var_lock(void)
{
uv_mutex_lock(&mutex);
}

void os_env_var_unlock(void)
{
uv_mutex_unlock(&mutex);
}



const char *os_getenv(const char *name)
FUNC_ATTR_NONNULL_ALL
{
char *e;
size_t size = 64;
if (name[0] == '\0') {
return NULL;
}
uv_mutex_lock(&mutex);
int r = 0;
if (pmap_has(cstr_t)(envmap, name)
&& !!(e = (char *)pmap_get(cstr_t)(envmap, name))) {
if (e[0] != '\0') {




goto end;
}
pmap_del2(envmap, name);
}
e = xmalloc(size);
r = uv_os_getenv(name, e, &size);
if (r == UV_ENOBUFS) {
e = xrealloc(e, size);
r = uv_os_getenv(name, e, &size);
}
if (r != 0 || size == 0 || e[0] == '\0') {
xfree(e);
e = NULL;
goto end;
}
pmap_put(cstr_t)(envmap, xstrdup(name), e);
end:

uv_mutex_unlock(&mutex);
if (r != 0 && r != UV_ENOENT && r != UV_UNKNOWN) {
ELOG("uv_os_getenv(%s) failed: %d %s", name, r, uv_err_name(r));
}
return (e == NULL || size == 0 || e[0] == '\0') ? NULL : e;
}



bool os_env_exists(const char *name)
FUNC_ATTR_NONNULL_ALL
{
if (name[0] == '\0') {
return false;
}


char buf[1];
size_t size = sizeof(buf);
int r = uv_os_getenv(name, buf, &size);
assert(r != UV_EINVAL);
if (r != 0 && r != UV_ENOENT && r != UV_ENOBUFS) {
ELOG("uv_os_getenv(%s) failed: %d %s", name, r, uv_err_name(r));
}
return (r == 0 || r == UV_ENOBUFS);
}







int os_setenv(const char *name, const char *value, int overwrite)
FUNC_ATTR_NONNULL_ALL
{
if (name[0] == '\0') {
return -1;
}
#if defined(WIN32)
if (!overwrite && os_getenv(name) != NULL) {
return 0;
}
if (value[0] == '\0') {

return os_unsetenv(name);
}
#else
if (!overwrite && os_env_exists(name)) {
return 0;
}
#endif
uv_mutex_lock(&mutex);
int r;
#if defined(WIN32)

if (striequal(name, "LC_ALL") || striequal(name, "LANGUAGE")
|| striequal(name, "LANG") || striequal(name, "LC_MESSAGES")) {
r = _putenv_s(name, value); 
assert(r == 0);
}
#endif
r = uv_os_setenv(name, value);
assert(r != UV_EINVAL);


pmap_del2(envmap, name);

uv_mutex_unlock(&mutex);
if (r != 0) {
ELOG("uv_os_setenv(%s) failed: %d %s", name, r, uv_err_name(r));
}
return r == 0 ? 0 : -1;
}


int os_unsetenv(const char *name)
FUNC_ATTR_NONNULL_ALL
{
if (name[0] == '\0') {
return -1;
}
uv_mutex_lock(&mutex);
pmap_del2(envmap, name);
int r = uv_os_unsetenv(name);

uv_mutex_unlock(&mutex);
if (r != 0) {
ELOG("uv_os_unsetenv(%s) failed: %d %s", name, r, uv_err_name(r));
}
return r == 0 ? 0 : -1;
}


size_t os_get_fullenv_size(void)
{
size_t len = 0;
#if defined(_WIN32)
wchar_t *envstrings = GetEnvironmentStringsW();
wchar_t *p = envstrings;
size_t l;
if (!envstrings) {
return len;
}


while ((l = wcslen(p)) != 0) {
p += l + 1;
len++;
}

FreeEnvironmentStringsW(envstrings);
#else
#if defined(HAVE__NSGETENVIRON)
char **environ = *_NSGetEnviron();
#else
extern char **environ;
#endif

while (environ[len] != NULL) {
len++;
}

#endif
return len;
}

void os_free_fullenv(char **env)
{
if (!env) { return; }
for (char **it = env; *it; it++) {
XFREE_CLEAR(*it);
}
xfree(env);
}







void os_copy_fullenv(char **env, size_t env_size)
{
#if defined(_WIN32)
wchar_t *envstrings = GetEnvironmentStringsW();
if (!envstrings) {
return;
}
wchar_t *p = envstrings;
size_t i = 0;
size_t l;


while ((l = wcslen(p)) != 0 && i < env_size) {
char *utf8_str;
int conversion_result = utf16_to_utf8(p, -1, &utf8_str);
if (conversion_result != 0) {
EMSG2("utf16_to_utf8 failed: %d", conversion_result);
break;
}
p += l + 1;

env[i] = utf8_str;
i++;
}

FreeEnvironmentStringsW(envstrings);
#else
#if defined(HAVE__NSGETENVIRON)
char **environ = *_NSGetEnviron();
#else
extern char **environ;
#endif

for (size_t i = 0; i < env_size && environ[i] != NULL; i++) {
env[i] = xstrdup(environ[i]);
}
#endif
}







char *os_getenvname_at_index(size_t index)
{
#if defined(_WIN32)
wchar_t *envstrings = GetEnvironmentStringsW();
if (!envstrings) {
return NULL;
}
wchar_t *p = envstrings;
char *name = NULL;
size_t i = 0;
size_t l;


while ((l = wcslen(p)) != 0 && i <= index) {
if (i == index) {
char *utf8_str;
int conversion_result = utf16_to_utf8(p, -1, &utf8_str);
if (conversion_result != 0) {
EMSG2("utf16_to_utf8 failed: %d", conversion_result);
break;
}



const char * const end = strchr(utf8_str + (utf8_str[0] == '=' ? 1 : 0),
'=');
assert(end != NULL);
ptrdiff_t len = end - utf8_str;
assert(len > 0);
name = xstrndup(utf8_str, (size_t)len);
xfree(utf8_str);
break;
}


p += l + 1;
i++;
}

FreeEnvironmentStringsW(envstrings);
return name;
#else
#if defined(HAVE__NSGETENVIRON)
char **environ = *_NSGetEnviron();
#else
extern char **environ;
#endif


for (size_t i = 0; i <= index; i++) {
if (environ[i] == NULL) {
return NULL;
}
}
char *str = environ[index];
assert(str != NULL);
const char * const end = strchr(str, '=');
assert(end != NULL);
ptrdiff_t len = end - str;
assert(len > 0);
return xstrndup(str, (size_t)len);
#endif
}




int64_t os_get_pid(void)
{
#if defined(_WIN32)
return (int64_t)GetCurrentProcessId();
#else
return (int64_t)getpid();
#endif
}





void os_get_hostname(char *hostname, size_t size)
{
#if defined(HAVE_SYS_UTSNAME_H)
struct utsname vutsname;

if (uname(&vutsname) < 0) {
*hostname = '\0';
} else {
xstrlcpy(hostname, vutsname.nodename, size);
}
#elif defined(WIN32)
wchar_t host_utf16[MAX_COMPUTERNAME_LENGTH + 1];
DWORD host_wsize = sizeof(host_utf16) / sizeof(host_utf16[0]);
if (GetComputerNameW(host_utf16, &host_wsize) == 0) {
*hostname = '\0';
DWORD err = GetLastError();
EMSG2("GetComputerNameW failed: %d", err);
return;
}
host_utf16[host_wsize] = '\0';

char *host_utf8;
int conversion_result = utf16_to_utf8(host_utf16, -1, &host_utf8);
if (conversion_result != 0) {
EMSG2("utf16_to_utf8 failed: %d", conversion_result);
return;
}
xstrlcpy(hostname, host_utf8, size);
xfree(host_utf8);
#else
EMSG("os_get_hostname failed: missing uname()");
*hostname = '\0';
#endif
}








static char *homedir = NULL;

void init_homedir(void)
{

xfree(homedir);
homedir = NULL;

const char *var = os_getenv("HOME");

#if defined(WIN32)




if (var == NULL) {
const char *homedrive = os_getenv("HOMEDRIVE");
const char *homepath = os_getenv("HOMEPATH");
if (homepath == NULL) {
homepath = "\\";
}
if (homedrive != NULL
&& strlen(homedrive) + strlen(homepath) < MAXPATHL) {
snprintf(os_buf, MAXPATHL, "%s%s", homedrive, homepath);
if (os_buf[0] != NUL) {
var = os_buf;
}
}
}
if (var == NULL) {
var = os_getenv("USERPROFILE");
}




if (var != NULL && *var == '%') {
const char *p = strchr(var + 1, '%');
if (p != NULL) {
vim_snprintf(os_buf, (size_t)(p - var), "%s", var + 1);
const char *exp = os_getenv(os_buf);
if (exp != NULL && *exp != NUL
&& STRLEN(exp) + STRLEN(p) < MAXPATHL) {
vim_snprintf(os_buf, MAXPATHL, "%s%s", exp, p + 1);
var = os_buf;
}
}
}



if (var == NULL

|| *var == NUL) {
var = "C:/";
}
#endif

if (var != NULL) {
#if defined(UNIX)


if (os_dirname((char_u *)os_buf, MAXPATHL) == OK && os_chdir(os_buf) == 0) {
if (!os_chdir(var) && os_dirname(IObuff, IOSIZE) == OK) {
var = (char *)IObuff;
}
if (os_chdir(os_buf) != 0) {
EMSG(_(e_prev_dir));
}
}
#endif
homedir = xstrdup(var);
}
}

#if defined(EXITFREE)

void free_homedir(void)
{
xfree(homedir);
}

#endif






char_u *expand_env_save(char_u *src)
{
return expand_env_save_opt(src, false);
}






char_u *expand_env_save_opt(char_u *src, bool one)
{
char_u *p = xmalloc(MAXPATHL);
expand_env_esc(src, p, MAXPATHL, false, one, NULL);
return p;
}









void expand_env(char_u *src, char_u *dst, int dstlen)
{
expand_env_esc(src, dst, dstlen, false, false, NULL);
}










void expand_env_esc(char_u *restrict srcp,
char_u *restrict dst,
int dstlen,
bool esc,
bool one,
char_u *prefix)
FUNC_ATTR_NONNULL_ARG(1, 2)
{
char_u *tail;
char_u *var;
bool copy_char;
bool mustfree; 
bool at_start = true; 

int prefix_len = (prefix == NULL) ? 0 : (int)STRLEN(prefix);

char_u *src = skipwhite(srcp);
dstlen--; 
while (*src && dstlen > 0) {

if (src[0] == '`' && src[1] == '=') {
var = src;
src += 2;
(void)skip_expr(&src);
if (*src == '`') {
src++;
}
size_t len = (size_t)(src - var);
if (len > (size_t)dstlen) {
len = (size_t)dstlen;
}
memcpy((char *)dst, (char *)var, len);
dst += len;
dstlen -= (int)len;
continue;
}

copy_char = true;
if ((*src == '$') || (*src == '~' && at_start)) {
mustfree = false;



if (*src != '~') { 
tail = src + 1;
var = dst;
int c = dstlen - 1;

#if defined(UNIX)

if (*tail == '{' && !vim_isIDc('{')) {
tail++; 
while (c-- > 0 && *tail != NUL && *tail != '}') {
*var++ = *tail++;
}
} else 
#endif
{
while (c-- > 0 && *tail != NUL && vim_isIDc(*tail)) {
*var++ = *tail++;
}
}

#if defined(UNIX)

if (src[1] == '{' && *tail != '}') {
var = NULL;
} else {
if (src[1] == '{') {
tail++;
}
#endif
*var = NUL;
var = (char_u *)vim_getenv((char *)dst);
mustfree = true;
#if defined(UNIX)
}
#endif
} else if (src[1] == NUL 
|| vim_ispathsep(src[1])
|| vim_strchr((char_u *)" ,\t\n", src[1]) != NULL) {
var = (char_u *)homedir;
tail = src + 1;
} else { 
#if defined(UNIX)

tail = src;
var = dst;
int c = dstlen - 1;
while (c-- > 0
&& *tail
&& vim_isfilec(*tail)
&& !vim_ispathsep(*tail)) {
*var++ = *tail++;
}
*var = NUL;


var = (*dst == NUL) ? NULL
: (char_u *)os_get_user_directory((char *)dst + 1);
mustfree = true;
if (var == NULL) {
expand_T xpc;

ExpandInit(&xpc);
xpc.xp_context = EXPAND_FILES;
var = ExpandOne(&xpc, dst, NULL,
WILD_ADD_SLASH|WILD_SILENT, WILD_EXPAND_FREE);
mustfree = true;
}
#else

var = NULL;
tail = (char_u *)""; 
#endif 
}

#if defined(BACKSLASH_IN_FILENAME)


if (p_ssl && var != NULL && vim_strchr(var, '\\') != NULL) {
char_u *p = vim_strsave(var);

if (mustfree) {
xfree(var);
}
var = p;
mustfree = true;
forward_slash(var);
}
#endif



if (esc && var != NULL && vim_strpbrk(var, (char_u *)" \t") != NULL) {
char_u *p = vim_strsave_escaped(var, (char_u *)" \t");

if (mustfree) {
xfree(var);
}
var = p;
mustfree = true;
}

if (var != NULL && *var != NUL
&& (STRLEN(var) + STRLEN(tail) + 1 < (unsigned)dstlen)) {
STRCPY(dst, var);
dstlen -= (int)STRLEN(var);
int c = (int)STRLEN(var);


if (*var != NUL && after_pathsep((char *)dst, (char *)dst + c)
#if defined(BACKSLASH_IN_FILENAME)
&& dst[-1] != ':'
#endif
&& vim_ispathsep(*tail))
++tail;
dst += c;
src = tail;
copy_char = false;
}
if (mustfree) {
xfree(var);
}
}

if (copy_char) { 



at_start = false;
if (src[0] == '\\' && src[1] != NUL) {
*dst++ = *src++;
--dstlen;
} else if ((src[0] == ' ' || src[0] == ',') && !one) {
at_start = true;
}
if (dstlen > 0) {
*dst++ = *src++;
dstlen--;

if (prefix != NULL
&& src - prefix_len >= srcp
&& STRNCMP(src - prefix_len, prefix, prefix_len) == 0) {
at_start = true;
}
}
}
}
*dst = NUL;
}




static char *vim_version_dir(const char *vimdir)
{
if (vimdir == NULL || *vimdir == NUL) {
return NULL;
}
char *p = concat_fnames(vimdir, VIM_VERSION_NODOT, true);
if (os_isdir((char_u *)p)) {
return p;
}
xfree(p);
p = concat_fnames(vimdir, RUNTIME_DIRNAME, true);
if (os_isdir((char_u *)p)) {
return p;
}
xfree(p);
return NULL;
}






















static char *remove_tail(char *path, char *pend, char *dirname)
{
size_t len = STRLEN(dirname);
char *new_tail = pend - len - 1;

if (new_tail >= path
&& fnamencmp((char_u *)new_tail, (char_u *)dirname, len) == 0
&& (new_tail == path || after_pathsep(path, new_tail))) {
return new_tail;
}
return pend;
}














const void *vim_env_iter(const char delim,
const char *const val,
const void *const iter,
const char **const dir,
size_t *const len)
FUNC_ATTR_NONNULL_ARG(2, 4, 5) FUNC_ATTR_WARN_UNUSED_RESULT
{
const char *varval = (const char *) iter;
if (varval == NULL) {
varval = val;
}
*dir = varval;
const char *const dirend = strchr(varval, delim);
if (dirend == NULL) {
*len = strlen(varval);
return NULL;
} else {
*len = (size_t) (dirend - varval);
return dirend + 1;
}
}














const void *vim_env_iter_rev(const char delim,
const char *const val,
const void *const iter,
const char **const dir,
size_t *const len)
FUNC_ATTR_NONNULL_ARG(2, 4, 5) FUNC_ATTR_WARN_UNUSED_RESULT
{
const char *varend = (const char *) iter;
if (varend == NULL) {
varend = val + strlen(val) - 1;
}
const size_t varlen = (size_t)(varend - val) + 1;
const char *const colon = xmemrchr(val, (uint8_t)delim, varlen);
if (colon == NULL) {
*len = varlen;
*dir = val;
return NULL;
} else {
*dir = colon + 1;
*len = (size_t) (varend - colon);
return colon - 1;
}
}



void vim_get_prefix_from_exepath(char *exe_name)
{


xstrlcpy(exe_name, (char *)get_vim_var_str(VV_PROGPATH),
MAXPATHL * sizeof(*exe_name));
char *path_end = (char *)path_tail_with_sep((char_u *)exe_name);
*path_end = '\0'; 
path_end = (char *)path_tail((char_u *)exe_name);
*path_end = '\0'; 
}







char *vim_getenv(const char *name)
{

assert(get_vim_var_str(VV_PROGPATH)[0] != NUL);

#if defined(WIN32)
if (strcmp(name, "HOME") == 0) {
return xstrdup(homedir);
}
#endif

const char *kos_env_path = os_getenv(name);
if (kos_env_path != NULL) {
return xstrdup(kos_env_path);
}

bool vimruntime = (strcmp(name, "VIMRUNTIME") == 0);
if (!vimruntime && strcmp(name, "VIM") != 0) {
return NULL;
}



char *vim_path = NULL;
if (vimruntime
#if defined(HAVE_PATHDEF)
&& *default_vimruntime_dir == NUL
#endif
) {
kos_env_path = os_getenv("VIM");
if (kos_env_path != NULL) {
vim_path = vim_version_dir(kos_env_path);
if (vim_path == NULL) {
vim_path = xstrdup(kos_env_path);
}
}
}




if (vim_path == NULL) {
if (p_hf != NULL && vim_strchr(p_hf, '$') == NULL) {
vim_path = (char *)p_hf;
}

char exe_name[MAXPATHL];

if (vim_path == NULL) {
vim_get_prefix_from_exepath(exe_name);
if (append_path(
exe_name,
"share" _PATHSEPSTR "nvim" _PATHSEPSTR "runtime" _PATHSEPSTR,
MAXPATHL) == OK) {
vim_path = exe_name; 
}
}

if (vim_path != NULL) {

char *vim_path_end = (char *)path_tail((char_u *)vim_path);


if (vim_path == (char *)p_hf) {
vim_path_end = remove_tail(vim_path, vim_path_end, "doc");
}


if (!vimruntime) {
vim_path_end = remove_tail(vim_path, vim_path_end, RUNTIME_DIRNAME);
vim_path_end = remove_tail(vim_path, vim_path_end, VIM_VERSION_NODOT);
}


if (vim_path_end > vim_path && after_pathsep(vim_path, vim_path_end)) {
vim_path_end--;
}


assert(vim_path_end >= vim_path);
vim_path = xstrndup(vim_path, (size_t)(vim_path_end - vim_path));

if (!os_isdir((char_u *)vim_path)) {
xfree(vim_path);
vim_path = NULL;
}
}
assert(vim_path != exe_name);
}

#if defined(HAVE_PATHDEF)


if (vim_path == NULL) {

if (vimruntime && *default_vimruntime_dir != NUL) {
vim_path = xstrdup(default_vimruntime_dir);
} else if (*default_vim_dir != NUL) {
if (vimruntime
&& (vim_path = vim_version_dir(default_vim_dir)) == NULL) {
vim_path = xstrdup(default_vim_dir);
}
}
}
#endif



if (vim_path != NULL) {
if (vimruntime) {
os_setenv("VIMRUNTIME", vim_path, 1);
didset_vimruntime = true;
} else {
os_setenv("VIM", vim_path, 1);
didset_vim = true;
}
}
return vim_path;
}



















size_t home_replace(const buf_T *const buf, const char_u *src,
char_u *const dst, size_t dstlen, const bool one)
FUNC_ATTR_NONNULL_ARG(3)
{
size_t dirlen = 0;
size_t envlen = 0;

if (src == NULL) {
*dst = NUL;
return 0;
}

if (buf != NULL && buf->b_help) {
const size_t dlen = xstrlcpy((char *)dst, (char *)path_tail(src), dstlen);
return MIN(dlen, dstlen - 1);
}



if (homedir != NULL) {
dirlen = strlen(homedir);
}

const char *homedir_env = os_getenv("HOME");
#if defined(WIN32)
if (homedir_env == NULL) {
homedir_env = os_getenv("USERPROFILE");
}
#endif
char *homedir_env_mod = (char *)homedir_env;
bool must_free = false;

if (homedir_env_mod != NULL && *homedir_env_mod == '~') {
must_free = true;
size_t usedlen = 0;
size_t flen = strlen(homedir_env_mod);
char_u *fbuf = NULL;
(void)modify_fname((char_u *)":p", false, &usedlen,
(char_u **)&homedir_env_mod, &fbuf, &flen);
flen = strlen(homedir_env_mod);
assert(homedir_env_mod != homedir_env);
if (vim_ispathsep(homedir_env_mod[flen - 1])) {

homedir_env_mod[flen - 1] = NUL;
}
}

if (homedir_env_mod != NULL) {
envlen = strlen(homedir_env_mod);
}

if (!one) {
src = skipwhite(src);
}
char *dst_p = (char *)dst;
while (*src && dstlen > 0) {







char *p = homedir;
size_t len = dirlen;
for (;;) {
if (len
&& fnamencmp(src, (char_u *)p, len) == 0
&& (vim_ispathsep(src[len])
|| (!one && (src[len] == ',' || src[len] == ' '))
|| src[len] == NUL)) {
src += len;
if (--dstlen > 0) {
*dst_p++ = '~';
}


if (!vim_ispathsep(src[0]) && --dstlen > 0) {
*dst_p++ = '/';
}
break;
}
if (p == homedir_env_mod) {
break;
}
p = homedir_env_mod;
len = envlen;
}


while (*src && (one || (*src != ',' && *src != ' ')) && --dstlen > 0) {
*dst_p++ = (char)(*src++);
}

while ((*src == ' ' || *src == ',') && --dstlen > 0) {
*dst_p++ = (char)(*src++);
}
}


*dst_p = NUL;

if (must_free) {
xfree(homedir_env_mod);
}
return (size_t)(dst_p - (char *)dst);
}




char_u * home_replace_save(buf_T *buf, char_u *src) FUNC_ATTR_NONNULL_RET
{
size_t len = 3; 
if (src != NULL) { 
len += STRLEN(src);
}
char_u *dst = xmalloc(len);
home_replace(buf, src, dst, len, true);
return dst;
}



char_u *get_env_name(expand_T *xp, int idx)
{
#define ENVNAMELEN 100

static char_u name[ENVNAMELEN];
assert(idx >= 0);
char *envname = os_getenvname_at_index((size_t)idx);
if (envname) {
STRLCPY(name, envname, ENVNAMELEN);
xfree(envname);
return name;
}
return NULL;
}






bool os_setenv_append_path(const char *fname)
FUNC_ATTR_NONNULL_ALL
{
#if defined(WIN32)

#define MAX_ENVPATHLEN 8192
#else

#define MAX_ENVPATHLEN INT_MAX
#endif
if (!path_is_absolute((char_u *)fname)) {
internal_error("os_setenv_append_path()");
return false;
}
const char *tail = (char *)path_tail_with_sep((char_u *)fname);
size_t dirlen = (size_t)(tail - fname);
assert(tail >= fname && dirlen + 1 < sizeof(os_buf));
xstrlcpy(os_buf, fname, dirlen + 1);
const char *path = os_getenv("PATH");
const size_t pathlen = path ? strlen(path) : 0;
const size_t newlen = pathlen + dirlen + 2;
if (newlen < MAX_ENVPATHLEN) {
char *temp = xmalloc(newlen);
if (pathlen == 0) {
temp[0] = NUL;
} else {
xstrlcpy(temp, path, newlen);
xstrlcat(temp, ENV_SEPSTR, newlen);
}
xstrlcat(temp, os_buf, newlen);
os_setenv("PATH", temp, 1);
xfree(temp);
return true;
}
return false;
}


bool os_shell_is_cmdexe(const char *sh)
FUNC_ATTR_NONNULL_ALL
{
if (*sh == NUL) {
return false;
}
if (striequal(sh, "$COMSPEC")) {
const char *comspec = os_getenv("COMSPEC");
return striequal("cmd.exe", (char *)path_tail((char_u *)comspec));
}
if (striequal(sh, "cmd.exe") || striequal(sh, "cmd")) {
return true;
}
return striequal("cmd.exe", (char *)path_tail((char_u *)sh));
}
