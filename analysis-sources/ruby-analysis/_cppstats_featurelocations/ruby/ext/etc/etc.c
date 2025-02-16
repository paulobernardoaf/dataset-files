








#include "ruby.h"
#include "ruby/encoding.h"
#include "ruby/io.h"

#include <sys/types.h>
#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(HAVE_GETPWENT)
#include <pwd.h>
#endif

#if defined(HAVE_GETGRENT)
#include <grp.h>
#endif

#include <errno.h>

#if defined(HAVE_SYS_UTSNAME_H)
#include <sys/utsname.h>
#endif

#if defined(HAVE_SCHED_GETAFFINITY)
#include <sched.h>
#endif

static VALUE sPasswd;
#if defined(HAVE_GETGRENT)
static VALUE sGroup;
#endif

#if defined(_WIN32)
#include <shlobj.h>
#if !defined(CSIDL_COMMON_APPDATA)
#define CSIDL_COMMON_APPDATA 35
#endif
#define HAVE_UNAME 1
#endif

#if !defined(_WIN32)
char *getenv();
#endif
char *getlogin();

#define RUBY_ETC_VERSION "1.1.0"

#if defined(HAVE_RB_DEPRECATE_CONSTANT)
void rb_deprecate_constant(VALUE mod, const char *name);
#else
#define rb_deprecate_constant(mod,name) ((void)(mod),(void)(name))
#endif

#include "constdefs.h"
















static VALUE
etc_getlogin(VALUE obj)
{
char *login;

#if defined(HAVE_GETLOGIN)
login = getlogin();
if (!login) login = getenv("USER");
#else
login = getenv("USER");
#endif

if (login) {
#if defined(_WIN32)
rb_encoding *extenc = rb_utf8_encoding();
#else
rb_encoding *extenc = rb_locale_encoding();
#endif
return rb_external_str_new_with_enc(login, strlen(login), extenc);
}

return Qnil;
}

#if defined(HAVE_GETPWENT) || defined(HAVE_GETGRENT)
static VALUE
safe_setup_str(const char *str)
{
if (str == 0) str = "";
return rb_str_new2(str);
}

static VALUE
safe_setup_locale_str(const char *str)
{
if (str == 0) str = "";
return rb_locale_str_new_cstr(str);
}

static VALUE
safe_setup_filesystem_str(const char *str)
{
if (str == 0) str = "";
return rb_filesystem_str_new_cstr(str);
}
#endif

#if defined(HAVE_GETPWENT)
static VALUE
setup_passwd(struct passwd *pwd)
{
if (pwd == 0) rb_sys_fail("/etc/passwd");
return rb_struct_new(sPasswd,
safe_setup_locale_str(pwd->pw_name),
#if defined(HAVE_STRUCT_PASSWD_PW_PASSWD)
safe_setup_str(pwd->pw_passwd),
#endif
UIDT2NUM(pwd->pw_uid),
GIDT2NUM(pwd->pw_gid),
#if defined(HAVE_STRUCT_PASSWD_PW_GECOS)
safe_setup_locale_str(pwd->pw_gecos),
#endif
safe_setup_filesystem_str(pwd->pw_dir),
safe_setup_filesystem_str(pwd->pw_shell),
#if defined(HAVE_STRUCT_PASSWD_PW_CHANGE)
INT2NUM(pwd->pw_change),
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_QUOTA)
INT2NUM(pwd->pw_quota),
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_AGE)
PW_AGE2VAL(pwd->pw_age),
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_CLASS)
safe_setup_locale_str(pwd->pw_class),
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_COMMENT)
safe_setup_locale_str(pwd->pw_comment),
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_EXPIRE)
INT2NUM(pwd->pw_expire),
#endif
0 
);
}
#endif


















static VALUE
etc_getpwuid(int argc, VALUE *argv, VALUE obj)
{
#if defined(HAVE_GETPWENT)
VALUE id;
rb_uid_t uid;
struct passwd *pwd;

if (rb_scan_args(argc, argv, "01", &id) == 1) {
uid = NUM2UIDT(id);
}
else {
uid = getuid();
}
pwd = getpwuid(uid);
if (pwd == 0) rb_raise(rb_eArgError, "can't find user for %d", (int)uid);
return setup_passwd(pwd);
#else
return Qnil;
#endif
}
















static VALUE
etc_getpwnam(VALUE obj, VALUE nam)
{
#if defined(HAVE_GETPWENT)
struct passwd *pwd;
const char *p = StringValueCStr(nam);

pwd = getpwnam(p);
if (pwd == 0) rb_raise(rb_eArgError, "can't find user for %"PRIsVALUE, nam);
return setup_passwd(pwd);
#else
return Qnil;
#endif
}

#if defined(HAVE_GETPWENT)
static int passwd_blocking = 0;
static VALUE
passwd_ensure(VALUE _)
{
endpwent();
passwd_blocking = (int)Qfalse;
return Qnil;
}

static VALUE
passwd_iterate(VALUE _)
{
struct passwd *pw;

setpwent();
while ((pw = getpwent()) != 0) {
rb_yield(setup_passwd(pw));
}
return Qnil;
}

static void
each_passwd(void)
{
if (passwd_blocking) {
rb_raise(rb_eRuntimeError, "parallel passwd iteration");
}
passwd_blocking = (int)Qtrue;
rb_ensure(passwd_iterate, 0, passwd_ensure, 0);
}
#endif





















static VALUE
etc_passwd(VALUE obj)
{
#if defined(HAVE_GETPWENT)
struct passwd *pw;

if (rb_block_given_p()) {
each_passwd();
}
else if ((pw = getpwent()) != 0) {
return setup_passwd(pw);
}
#endif
return Qnil;
}

























static VALUE
etc_each_passwd(VALUE obj)
{
#if defined(HAVE_GETPWENT)
RETURN_ENUMERATOR(obj, 0, 0);
each_passwd();
#endif
return obj;
}




static VALUE
etc_setpwent(VALUE obj)
{
#if defined(HAVE_GETPWENT)
setpwent();
#endif
return Qnil;
}




static VALUE
etc_endpwent(VALUE obj)
{
#if defined(HAVE_GETPWENT)
endpwent();
#endif
return Qnil;
}












static VALUE
etc_getpwent(VALUE obj)
{
#if defined(HAVE_GETPWENT)
struct passwd *pw;

if ((pw = getpwent()) != 0) {
return setup_passwd(pw);
}
#endif
return Qnil;
}

#if defined(HAVE_GETGRENT)
static VALUE
setup_group(struct group *grp)
{
VALUE mem;
char **tbl;

mem = rb_ary_new();
tbl = grp->gr_mem;
while (*tbl) {
rb_ary_push(mem, safe_setup_locale_str(*tbl));
tbl++;
}
return rb_struct_new(sGroup,
safe_setup_locale_str(grp->gr_name),
#if defined(HAVE_STRUCT_GROUP_GR_PASSWD)
safe_setup_str(grp->gr_passwd),
#endif
GIDT2NUM(grp->gr_gid),
mem);
}
#endif

















static VALUE
etc_getgrgid(int argc, VALUE *argv, VALUE obj)
{
#if defined(HAVE_GETGRENT)
VALUE id;
gid_t gid;
struct group *grp;

if (rb_scan_args(argc, argv, "01", &id) == 1) {
gid = NUM2GIDT(id);
}
else {
gid = getgid();
}
grp = getgrgid(gid);
if (grp == 0) rb_raise(rb_eArgError, "can't find group for %d", (int)gid);
return setup_group(grp);
#else
return Qnil;
#endif
}

















static VALUE
etc_getgrnam(VALUE obj, VALUE nam)
{
#if defined(HAVE_GETGRENT)
struct group *grp;
const char *p = StringValueCStr(nam);

grp = getgrnam(p);
if (grp == 0) rb_raise(rb_eArgError, "can't find group for %"PRIsVALUE, nam);
return setup_group(grp);
#else
return Qnil;
#endif
}

#if defined(HAVE_GETGRENT)
static int group_blocking = 0;
static VALUE
group_ensure(VALUE _)
{
endgrent();
group_blocking = (int)Qfalse;
return Qnil;
}


static VALUE
group_iterate(VALUE _)
{
struct group *pw;

setgrent();
while ((pw = getgrent()) != 0) {
rb_yield(setup_group(pw));
}
return Qnil;
}

static void
each_group(void)
{
if (group_blocking) {
rb_raise(rb_eRuntimeError, "parallel group iteration");
}
group_blocking = (int)Qtrue;
rb_ensure(group_iterate, 0, group_ensure, 0);
}
#endif

















static VALUE
etc_group(VALUE obj)
{
#if defined(HAVE_GETGRENT)
struct group *grp;

if (rb_block_given_p()) {
each_group();
}
else if ((grp = getgrent()) != 0) {
return setup_group(grp);
}
#endif
return Qnil;
}

#if defined(HAVE_GETGRENT)






















static VALUE
etc_each_group(VALUE obj)
{
RETURN_ENUMERATOR(obj, 0, 0);
each_group();
return obj;
}
#endif




static VALUE
etc_setgrent(VALUE obj)
{
#if defined(HAVE_GETGRENT)
setgrent();
#endif
return Qnil;
}




static VALUE
etc_endgrent(VALUE obj)
{
#if defined(HAVE_GETGRENT)
endgrent();
#endif
return Qnil;
}











static VALUE
etc_getgrent(VALUE obj)
{
#if defined(HAVE_GETGRENT)
struct group *gr;

if ((gr = getgrent()) != 0) {
return setup_group(gr);
}
#endif
return Qnil;
}

#define numberof(array) (sizeof(array) / sizeof(*(array)))

#if defined(_WIN32)
VALUE rb_w32_special_folder(int type);
UINT rb_w32_system_tmpdir(WCHAR *path, UINT len);
VALUE rb_w32_conv_from_wchar(const WCHAR *wstr, rb_encoding *enc);
#endif









static VALUE
etc_sysconfdir(VALUE obj)
{
#if defined(_WIN32)
return rb_w32_special_folder(CSIDL_COMMON_APPDATA);
#else
return rb_filesystem_str_new_cstr(SYSCONFDIR);
#endif
}




static VALUE
etc_systmpdir(VALUE _)
{
VALUE tmpdir;
#if defined(_WIN32)
WCHAR path[_MAX_PATH];
UINT len = rb_w32_system_tmpdir(path, numberof(path));
if (!len) return Qnil;
tmpdir = rb_w32_conv_from_wchar(path, rb_filesystem_encoding());
#else
const char default_tmp[] = "/tmp";
const char *tmpstr = default_tmp;
size_t tmplen = strlen(default_tmp);
#if defined _CS_DARWIN_USER_TEMP_DIR
#if !defined(MAXPATHLEN)
#define MAXPATHLEN 1024
#endif
char path[MAXPATHLEN];
size_t len;
len = confstr(_CS_DARWIN_USER_TEMP_DIR, path, sizeof(path));
if (len > 0) {
tmpstr = path;
tmplen = len - 1;
if (len > sizeof(path)) tmpstr = 0;
}
#endif
tmpdir = rb_filesystem_str_new(tmpstr, tmplen);
#if defined _CS_DARWIN_USER_TEMP_DIR
if (!tmpstr) {
confstr(_CS_DARWIN_USER_TEMP_DIR, RSTRING_PTR(tmpdir), len);
}
#endif
#endif
#if !defined(RB_PASS_KEYWORDS)

FL_UNSET(tmpdir, FL_TAINT);
#endif
return tmpdir;
}

#if defined(HAVE_UNAME)



















static VALUE
etc_uname(VALUE obj)
{
#if defined(_WIN32)
OSVERSIONINFOW v;
SYSTEM_INFO s;
const char *sysname, *mach;
VALUE result, release, version;
VALUE vbuf, nodename = Qnil;
DWORD len = 0;
WCHAR *buf;

v.dwOSVersionInfoSize = sizeof(v);
if (!GetVersionExW(&v))
rb_sys_fail("GetVersionEx");

result = rb_hash_new();
switch (v.dwPlatformId) {
case VER_PLATFORM_WIN32s:
sysname = "Win32s";
break;
case VER_PLATFORM_WIN32_NT:
sysname = "Windows_NT";
break;
case VER_PLATFORM_WIN32_WINDOWS:
default:
sysname = "Windows";
break;
}
rb_hash_aset(result, ID2SYM(rb_intern("sysname")), rb_str_new_cstr(sysname));
release = rb_sprintf("%lu.%lu.%lu", v.dwMajorVersion, v.dwMinorVersion, v.dwBuildNumber);
rb_hash_aset(result, ID2SYM(rb_intern("release")), release);
version = rb_sprintf("%s Version %"PRIsVALUE": %"PRIsVALUE, sysname, release,
rb_w32_conv_from_wchar(v.szCSDVersion, rb_utf8_encoding()));
rb_hash_aset(result, ID2SYM(rb_intern("version")), version);

#if defined _MSC_VER && _MSC_VER < 1300
#define GET_COMPUTER_NAME(ptr, plen) GetComputerNameW(ptr, plen)
#else
#define GET_COMPUTER_NAME(ptr, plen) GetComputerNameExW(ComputerNameDnsFullyQualified, ptr, plen)
#endif
GET_COMPUTER_NAME(NULL, &len);
buf = ALLOCV_N(WCHAR, vbuf, len);
if (GET_COMPUTER_NAME(buf, &len)) {
nodename = rb_w32_conv_from_wchar(buf, rb_utf8_encoding());
}
ALLOCV_END(vbuf);
if (NIL_P(nodename)) nodename = rb_str_new(0, 0);
rb_hash_aset(result, ID2SYM(rb_intern("nodename")), nodename);

#if !defined(PROCESSOR_ARCHITECTURE_AMD64)
#define PROCESSOR_ARCHITECTURE_AMD64 9
#endif
#if !defined(PROCESSOR_ARCHITECTURE_INTEL)
#define PROCESSOR_ARCHITECTURE_INTEL 0
#endif
GetSystemInfo(&s);
switch (s.wProcessorArchitecture) {
case PROCESSOR_ARCHITECTURE_AMD64:
mach = "x64";
break;
case PROCESSOR_ARCHITECTURE_ARM:
mach = "ARM";
break;
case PROCESSOR_ARCHITECTURE_INTEL:
mach = "x86";
break;
default:
mach = "unknown";
break;
}

rb_hash_aset(result, ID2SYM(rb_intern("machine")), rb_str_new_cstr(mach));
#else
struct utsname u;
int ret;
VALUE result;

ret = uname(&u);
if (ret == -1)
rb_sys_fail("uname");

result = rb_hash_new();
rb_hash_aset(result, ID2SYM(rb_intern("sysname")), rb_str_new_cstr(u.sysname));
rb_hash_aset(result, ID2SYM(rb_intern("nodename")), rb_str_new_cstr(u.nodename));
rb_hash_aset(result, ID2SYM(rb_intern("release")), rb_str_new_cstr(u.release));
rb_hash_aset(result, ID2SYM(rb_intern("version")), rb_str_new_cstr(u.version));
rb_hash_aset(result, ID2SYM(rb_intern("machine")), rb_str_new_cstr(u.machine));
#endif

return result;
}
#else
#define etc_uname rb_f_notimplement
#endif

#if defined(HAVE_SYSCONF)












static VALUE
etc_sysconf(VALUE obj, VALUE arg)
{
int name;
long ret;

name = NUM2INT(arg);

errno = 0;
ret = sysconf(name);
if (ret == -1) {
if (errno == 0) 
return Qnil;
rb_sys_fail("sysconf");
}
return LONG2NUM(ret);
}
#else
#define etc_sysconf rb_f_notimplement
#endif

#if defined(HAVE_CONFSTR)















static VALUE
etc_confstr(VALUE obj, VALUE arg)
{
int name;
char localbuf[128], *buf = localbuf;
size_t bufsize = sizeof(localbuf), ret;
VALUE tmp;

name = NUM2INT(arg);

errno = 0;
ret = confstr(name, buf, bufsize);
if (bufsize < ret) {
bufsize = ret;
buf = ALLOCV_N(char, tmp, bufsize);
errno = 0;
ret = confstr(name, buf, bufsize);
}
if (bufsize < ret)
rb_bug("required buffer size for confstr() changed dynamically.");
if (ret == 0) {
if (errno == 0) 
return Qnil;
rb_sys_fail("confstr");
}
return rb_str_new_cstr(buf);
}
#else
#define etc_confstr rb_f_notimplement
#endif

#if defined(HAVE_FPATHCONF)














static VALUE
io_pathconf(VALUE io, VALUE arg)
{
int name;
long ret;
rb_io_t *fptr;

name = NUM2INT(arg);

GetOpenFile(io, fptr);

errno = 0;
ret = fpathconf(fptr->fd, name);
if (ret == -1) {
if (errno == 0) 
return Qnil;
rb_sys_fail("fpathconf");
}
return LONG2NUM(ret);
}
#else
#define io_pathconf rb_f_notimplement
#endif

#if (defined(HAVE_SYSCONF) && defined(_SC_NPROCESSORS_ONLN)) || defined(_WIN32)

#if defined(HAVE_SCHED_GETAFFINITY) && defined(CPU_ALLOC)
static int
etc_nprocessors_affin(void)
{
cpu_set_t *cpuset;
size_t size;
int ret;
int n;

















for (n=64; n <= 16384; n *= 2) {
size = CPU_ALLOC_SIZE(n);
if (size >= 1024) {
cpuset = xcalloc(1, size);
if (!cpuset)
return -1;
} else {
cpuset = alloca(size);
CPU_ZERO_S(size, cpuset);
}

ret = sched_getaffinity(0, size, cpuset);
if (ret == 0) {

ret = CPU_COUNT_S(size, cpuset);
}

if (size >= 1024) {
xfree(cpuset);
}
if (ret > 0) {
return ret;
}
}

return ret;
}
#endif

























static VALUE
etc_nprocessors(VALUE obj)
{
long ret;

#if !defined(_WIN32)

#if defined(HAVE_SCHED_GETAFFINITY) && defined(CPU_ALLOC)
int ncpus;

ncpus = etc_nprocessors_affin();
if (ncpus != -1) {
return INT2NUM(ncpus);
}

#endif

errno = 0;
ret = sysconf(_SC_NPROCESSORS_ONLN);
if (ret == -1) {
rb_sys_fail("sysconf(_SC_NPROCESSORS_ONLN)");
}
#else
SYSTEM_INFO si;
GetSystemInfo(&si);
ret = (long)si.dwNumberOfProcessors;
#endif
return LONG2NUM(ret);
}
#else
#define etc_nprocessors rb_f_notimplement
#endif



























void
Init_etc(void)
{
VALUE mEtc;

mEtc = rb_define_module("Etc");
rb_define_const(mEtc, "VERSION", rb_str_new_cstr(RUBY_ETC_VERSION));
init_constants(mEtc);

rb_define_module_function(mEtc, "getlogin", etc_getlogin, 0);

rb_define_module_function(mEtc, "getpwuid", etc_getpwuid, -1);
rb_define_module_function(mEtc, "getpwnam", etc_getpwnam, 1);
rb_define_module_function(mEtc, "setpwent", etc_setpwent, 0);
rb_define_module_function(mEtc, "endpwent", etc_endpwent, 0);
rb_define_module_function(mEtc, "getpwent", etc_getpwent, 0);
rb_define_module_function(mEtc, "passwd", etc_passwd, 0);

rb_define_module_function(mEtc, "getgrgid", etc_getgrgid, -1);
rb_define_module_function(mEtc, "getgrnam", etc_getgrnam, 1);
rb_define_module_function(mEtc, "group", etc_group, 0);
rb_define_module_function(mEtc, "setgrent", etc_setgrent, 0);
rb_define_module_function(mEtc, "endgrent", etc_endgrent, 0);
rb_define_module_function(mEtc, "getgrent", etc_getgrent, 0);
rb_define_module_function(mEtc, "sysconfdir", etc_sysconfdir, 0);
rb_define_module_function(mEtc, "systmpdir", etc_systmpdir, 0);
rb_define_module_function(mEtc, "uname", etc_uname, 0);
rb_define_module_function(mEtc, "sysconf", etc_sysconf, 1);
rb_define_module_function(mEtc, "confstr", etc_confstr, 1);
rb_define_method(rb_cIO, "pathconf", io_pathconf, 1);
rb_define_module_function(mEtc, "nprocessors", etc_nprocessors, 0);

sPasswd = rb_struct_define_under(mEtc, "Passwd",
"name",
#if defined(HAVE_STRUCT_PASSWD_PW_PASSWD)
"passwd",
#endif
"uid",
"gid",
#if defined(HAVE_STRUCT_PASSWD_PW_GECOS)
"gecos",
#endif
"dir",
"shell",
#if defined(HAVE_STRUCT_PASSWD_PW_CHANGE)
"change",
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_QUOTA)
"quota",
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_AGE)
"age",
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_CLASS)
"uclass",
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_COMMENT)
"comment",
#endif
#if defined(HAVE_STRUCT_PASSWD_PW_EXPIRE)
"expire",
#endif
NULL);
#if 0







































rb_define_const(mEtc, "Passwd", sPasswd);
#endif
rb_define_const(rb_cStruct, "Passwd", sPasswd); 
rb_deprecate_constant(rb_cStruct, "Passwd");
rb_extend_object(sPasswd, rb_mEnumerable);
rb_define_singleton_method(sPasswd, "each", etc_each_passwd, 0);

#if defined(HAVE_GETGRENT)
sGroup = rb_struct_define_under(mEtc, "Group", "name",
#if defined(HAVE_STRUCT_GROUP_GR_PASSWD)
"passwd",
#endif
"gid", "mem", NULL);

#if 0





















rb_define_const(mEtc, "Group", sGroup);
#endif
rb_define_const(rb_cStruct, "Group", sGroup); 
rb_deprecate_constant(rb_cStruct, "Group");
rb_extend_object(sGroup, rb_mEnumerable);
rb_define_singleton_method(sGroup, "each", etc_each_group, 0);
#endif
}
