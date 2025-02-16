












#include "ruby/config.h"

#if defined(_WIN32)
#include "missing/file.h"
#include "ruby.h"
#endif

#include <ctype.h>
#include <time.h>

#if defined(__CYGWIN__)
#include <windows.h>
#include <sys/cygwin.h>
#include <wchar.h>
#endif

#if defined(__APPLE__)
#if !(defined(__has_feature) && defined(__has_attribute))




#define API_AVAILABLE(...)
#define API_DEPRECATED(...)
#endif
#include <CoreFoundation/CFString.h>
#endif

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#if defined(HAVE_SYS_FILE_H)
#include <sys/file.h>
#else
int flock(int, int);
#endif

#if defined(HAVE_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if !defined(MAXPATHLEN)
#define MAXPATHLEN 1024
#endif

#if defined(HAVE_UTIME_H)
#include <utime.h>
#elif defined HAVE_SYS_UTIME_H
#include <sys/utime.h>
#endif

#if defined(HAVE_PWD_H)
#include <pwd.h>
#endif

#if defined(HAVE_SYS_SYSMACROS_H)
#include <sys/sysmacros.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#if defined(HAVE_SYS_MKDEV_H)
#include <sys/mkdev.h>
#endif

#if defined(HAVE_FCNTL_H)
#include <fcntl.h>
#endif

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif

#if !defined HAVE_LSTAT && !defined lstat
#define lstat stat
#endif


#if defined(_WIN32)
#include "win32/file.h"
#define STAT(p, s) rb_w32_ustati128((p), (s))
#undef lstat
#define lstat(p, s) rb_w32_ulstati128((p), (s))
#undef access
#define access(p, m) rb_w32_uaccess((p), (m))
#undef truncate
#define truncate(p, n) rb_w32_utruncate((p), (n))
#undef chmod
#define chmod(p, m) rb_w32_uchmod((p), (m))
#undef chown
#define chown(p, o, g) rb_w32_uchown((p), (o), (g))
#undef lchown
#define lchown(p, o, g) rb_w32_ulchown((p), (o), (g))
#undef utimensat
#define utimensat(s, p, t, f) rb_w32_uutimensat((s), (p), (t), (f))
#undef link
#define link(f, t) rb_w32_ulink((f), (t))
#undef unlink
#define unlink(p) rb_w32_uunlink(p)
#undef rename
#define rename(f, t) rb_w32_urename((f), (t))
#undef symlink
#define symlink(s, l) rb_w32_usymlink((s), (l))

#if defined(HAVE_REALPATH)


#undef HAVE_REALPATH
#endif
#else
#define STAT(p, s) stat((p), (s))
#endif

#if defined _WIN32 || defined __APPLE__
#define USE_OSPATH 1
#define TO_OSPATH(str) rb_str_encode_ospath(str)
#else
#define USE_OSPATH 0
#define TO_OSPATH(str) (str)
#endif


#if defined DOSISH || defined __CYGWIN__
#define UTIME_EINVAL
#endif


#if defined HAVE_REALPATH && defined __sun && defined __SVR4
#undef HAVE_REALPATH
#endif

#if defined(HAVE_REALPATH)
#include <limits.h>
#include <stdlib.h>
#endif

#include "dln.h"
#include "encindex.h"
#include "id.h"
#include "internal.h"
#include "internal/compilers.h"
#include "internal/dir.h"
#include "internal/error.h"
#include "internal/file.h"
#include "internal/io.h"
#include "internal/load.h"
#include "internal/object.h"
#include "internal/process.h"
#include "internal/thread.h"
#include "internal/vm.h"
#include "ruby/encoding.h"
#include "ruby/io.h"
#include "ruby/thread.h"
#include "ruby/util.h"

VALUE rb_cFile;
VALUE rb_mFileTest;
VALUE rb_cStat;

static VALUE
file_path_convert(VALUE name)
{
#if !defined(_WIN32)
int fname_encidx = ENCODING_GET(name);
int fs_encidx;
if (ENCINDEX_US_ASCII != fname_encidx &&
ENCINDEX_ASCII != fname_encidx &&
(fs_encidx = rb_filesystem_encindex()) != fname_encidx &&
rb_default_internal_encoding() &&
!rb_enc_str_asciionly_p(name)) {


rb_encoding *fname_encoding = rb_enc_from_index(fname_encidx);
rb_encoding *fs_encoding = rb_enc_from_index(fs_encidx);
name = rb_str_conv_enc(name, fname_encoding, fs_encoding);
}
#endif
return name;
}

static rb_encoding *
check_path_encoding(VALUE str)
{
rb_encoding *enc = rb_enc_get(str);
if (!rb_enc_asciicompat(enc)) {
rb_raise(rb_eEncCompatError, "path name must be ASCII-compatible (%s): %"PRIsVALUE,
rb_enc_name(enc), rb_str_inspect(str));
}
return enc;
}

VALUE
rb_get_path_check_to_string(VALUE obj)
{
VALUE tmp;
ID to_path;

if (RB_TYPE_P(obj, T_STRING)) {
return obj;
}
CONST_ID(to_path, "to_path");
tmp = rb_check_funcall_default(obj, to_path, 0, 0, obj);
StringValue(tmp);
return tmp;
}

VALUE
rb_get_path_check_convert(VALUE obj)
{
obj = file_path_convert(obj);

check_path_encoding(obj);
if (!rb_str_to_cstr(obj)) {
rb_raise(rb_eArgError, "path name contains null byte");
}

return rb_str_new4(obj);
}

VALUE
rb_get_path_no_checksafe(VALUE obj)
{
return rb_get_path(obj);
}

VALUE
rb_get_path(VALUE obj)
{
return rb_get_path_check_convert(rb_get_path_check_to_string(obj));
}

VALUE
rb_str_encode_ospath(VALUE path)
{
#if USE_OSPATH
int encidx = ENCODING_GET(path);
#if 0 && defined _WIN32
if (encidx == ENCINDEX_ASCII) {
encidx = rb_filesystem_encindex();
}
#endif
if (encidx != ENCINDEX_ASCII && encidx != ENCINDEX_UTF_8) {
rb_encoding *enc = rb_enc_from_index(encidx);
rb_encoding *utf8 = rb_utf8_encoding();
path = rb_str_conv_enc(path, enc, utf8);
}
#endif
return path;
}

#if defined(__APPLE__)
#define NORMALIZE_UTF8PATH 1
static VALUE
rb_str_append_normalized_ospath(VALUE str, const char *ptr, long len)
{
CFIndex buflen = 0;
CFRange all;
CFStringRef s = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
(const UInt8 *)ptr, len,
kCFStringEncodingUTF8, FALSE,
kCFAllocatorNull);
CFMutableStringRef m = CFStringCreateMutableCopy(kCFAllocatorDefault, len, s);
long oldlen = RSTRING_LEN(str);

CFStringNormalize(m, kCFStringNormalizationFormC);
all = CFRangeMake(0, CFStringGetLength(m));
CFStringGetBytes(m, all, kCFStringEncodingUTF8, '?', FALSE, NULL, 0, &buflen);
rb_str_modify_expand(str, buflen);
CFStringGetBytes(m, all, kCFStringEncodingUTF8, '?', FALSE,
(UInt8 *)(RSTRING_PTR(str) + oldlen), buflen, &buflen);
rb_str_set_len(str, oldlen + buflen);
CFRelease(m);
CFRelease(s);
return str;
}

VALUE
rb_str_normalize_ospath(const char *ptr, long len)
{
const char *p = ptr;
const char *e = ptr + len;
const char *p1 = p;
VALUE str = rb_str_buf_new(len);
rb_encoding *enc = rb_utf8_encoding();
rb_enc_associate(str, enc);

while (p < e) {
int l, c;
int r = rb_enc_precise_mbclen(p, e, enc);
if (!MBCLEN_CHARFOUND_P(r)) {

static const char invalid[3] = "\xEF\xBF\xBD";
rb_str_append_normalized_ospath(str, p1, p-p1);
rb_str_cat(str, invalid, sizeof(invalid));
p += 1;
p1 = p;
continue;
}
l = MBCLEN_CHARFOUND_LEN(r);
c = rb_enc_mbc_to_codepoint(p, e, enc);
if ((0x2000 <= c && c <= 0x2FFF) || (0xF900 <= c && c <= 0xFAFF) ||
(0x2F800 <= c && c <= 0x2FAFF)) {
if (p - p1 > 0) {
rb_str_append_normalized_ospath(str, p1, p-p1);
}
rb_str_cat(str, p, l);
p += l;
p1 = p;
}
else {
p += l;
}
}
if (p - p1 > 0) {
rb_str_append_normalized_ospath(str, p1, p-p1);
}

return str;
}

static int
ignored_char_p(const char *p, const char *e, rb_encoding *enc)
{
unsigned char c;
if (p+3 > e) return 0;
switch ((unsigned char)*p) {
case 0xe2:
switch ((unsigned char)p[1]) {
case 0x80:
c = (unsigned char)p[2];

if (c >= 0x8c && c <= 0x8f) return 3;

if (c >= 0xaa && c <= 0xae) return 3;
return 0;
case 0x81:
c = (unsigned char)p[2];

if (c >= 0xaa && c <= 0xaf) return 3;
return 0;
}
break;
case 0xef:

if ((unsigned char)p[1] == 0xbb &&
(unsigned char)p[2] == 0xbf)
return 3;
break;
}
return 0;
}
#else
#define NORMALIZE_UTF8PATH 0
#endif

#define apply2args(n) (rb_check_arity(argc, n, UNLIMITED_ARGUMENTS), argc-=n)

struct apply_filename {
const char *ptr;
VALUE path;
};

struct apply_arg {
int i;
int argc;
int errnum;
int (*func)(const char *, void *);
void *arg;
struct apply_filename fn[FLEX_ARY_LEN];
};

static void *
no_gvl_apply2files(void *ptr)
{
struct apply_arg *aa = ptr;

for (aa->i = 0; aa->i < aa->argc; aa->i++) {
if (aa->func(aa->fn[aa->i].ptr, aa->arg) < 0) {
aa->errnum = errno;
break;
}
}
return 0;
}

#if defined(UTIME_EINVAL)
NORETURN(static void utime_failed(struct apply_arg *));
static int utime_internal(const char *, void *);
#endif

static VALUE
apply2files(int (*func)(const char *, void *), int argc, VALUE *argv, void *arg)
{
VALUE v;
const size_t size = sizeof(struct apply_filename);
const long len = (long)(offsetof(struct apply_arg, fn) + (size * argc));
struct apply_arg *aa = ALLOCV(v, len);

aa->errnum = 0;
aa->argc = argc;
aa->arg = arg;
aa->func = func;

for (aa->i = 0; aa->i < argc; aa->i++) {
VALUE path = rb_get_path(argv[aa->i]);

path = rb_str_encode_ospath(path);
aa->fn[aa->i].ptr = RSTRING_PTR(path);
aa->fn[aa->i].path = path;
}

rb_thread_call_without_gvl(no_gvl_apply2files, aa, RUBY_UBF_IO, 0);
if (aa->errnum) {
#if defined(UTIME_EINVAL)
if (func == utime_internal) {
utime_failed(aa);
}
#endif
rb_syserr_fail_path(aa->errnum, aa->fn[aa->i].path);
}
if (v) {
ALLOCV_END(v);
}
return LONG2FIX(argc);
}





















static VALUE
rb_file_path(VALUE obj)
{
rb_io_t *fptr;

fptr = RFILE(rb_io_taint_check(obj))->fptr;
rb_io_check_initialized(fptr);

if (NIL_P(fptr->pathv)) {
rb_raise(rb_eIOError, "File is unnamed (TMPFILE?)");
}

return rb_str_dup(fptr->pathv);
}

static size_t
stat_memsize(const void *p)
{
return sizeof(struct stat);
}

static const rb_data_type_t stat_data_type = {
"stat",
{NULL, RUBY_TYPED_DEFAULT_FREE, stat_memsize,},
0, 0, RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE
stat_new_0(VALUE klass, const struct stat *st)
{
struct stat *nst = 0;
VALUE obj = TypedData_Wrap_Struct(klass, &stat_data_type, 0);

if (st) {
nst = ALLOC(struct stat);
*nst = *st;
RTYPEDDATA_DATA(obj) = nst;
}
return obj;
}

VALUE
rb_stat_new(const struct stat *st)
{
return stat_new_0(rb_cStat, st);
}

static struct stat*
get_stat(VALUE self)
{
struct stat* st;
TypedData_Get_Struct(self, struct stat, &stat_data_type, st);
if (!st) rb_raise(rb_eTypeError, "uninitialized File::Stat");
return st;
}

static struct timespec stat_mtimespec(const struct stat *st);
















static VALUE
rb_stat_cmp(VALUE self, VALUE other)
{
if (rb_obj_is_kind_of(other, rb_obj_class(self))) {
struct timespec ts1 = stat_mtimespec(get_stat(self));
struct timespec ts2 = stat_mtimespec(get_stat(other));
if (ts1.tv_sec == ts2.tv_sec) {
if (ts1.tv_nsec == ts2.tv_nsec) return INT2FIX(0);
if (ts1.tv_nsec < ts2.tv_nsec) return INT2FIX(-1);
return INT2FIX(1);
}
if (ts1.tv_sec < ts2.tv_sec) return INT2FIX(-1);
return INT2FIX(1);
}
return Qnil;
}

#define ST2UINT(val) ((val) & ~(~1UL << (sizeof(val) * CHAR_BIT - 1)))

#if !defined(NUM2DEVT)
#define NUM2DEVT(v) NUM2UINT(v)
#endif
#if !defined(DEVT2NUM)
#define DEVT2NUM(v) UINT2NUM(v)
#endif
#if !defined(PRI_DEVT_PREFIX)
#define PRI_DEVT_PREFIX ""
#endif











static VALUE
rb_stat_dev(VALUE self)
{
return DEVT2NUM(get_stat(self)->st_dev);
}












static VALUE
rb_stat_dev_major(VALUE self)
{
#if defined(major)
return UINT2NUM(major(get_stat(self)->st_dev));
#else
return Qnil;
#endif
}












static VALUE
rb_stat_dev_minor(VALUE self)
{
#if defined(minor)
return UINT2NUM(minor(get_stat(self)->st_dev));
#else
return Qnil;
#endif
}











static VALUE
rb_stat_ino(VALUE self)
{
#if defined(HAVE_STRUCT_STAT_ST_INOHIGH)

return rb_integer_unpack(&get_stat(self)->st_ino, 2,
SIZEOF_STRUCT_STAT_ST_INO, 0,
INTEGER_PACK_LSWORD_FIRST|INTEGER_PACK_NATIVE_BYTE_ORDER|
INTEGER_PACK_2COMP);
#elif SIZEOF_STRUCT_STAT_ST_INO > SIZEOF_LONG
return ULL2NUM(get_stat(self)->st_ino);
#else
return ULONG2NUM(get_stat(self)->st_ino);
#endif
}














static VALUE
rb_stat_mode(VALUE self)
{
return UINT2NUM(ST2UINT(get_stat(self)->st_mode));
}













static VALUE
rb_stat_nlink(VALUE self)
{
return UINT2NUM(get_stat(self)->st_nlink);
}











static VALUE
rb_stat_uid(VALUE self)
{
return UIDT2NUM(get_stat(self)->st_uid);
}











static VALUE
rb_stat_gid(VALUE self)
{
return GIDT2NUM(get_stat(self)->st_gid);
}













static VALUE
rb_stat_rdev(VALUE self)
{
#if defined(HAVE_STRUCT_STAT_ST_RDEV)
return DEVT2NUM(get_stat(self)->st_rdev);
#else
return Qnil;
#endif
}












static VALUE
rb_stat_rdev_major(VALUE self)
{
#if defined(HAVE_STRUCT_STAT_ST_RDEV) && defined(major)
return UINT2NUM(major(get_stat(self)->st_rdev));
#else
return Qnil;
#endif
}












static VALUE
rb_stat_rdev_minor(VALUE self)
{
#if defined(HAVE_STRUCT_STAT_ST_RDEV) && defined(minor)
return UINT2NUM(minor(get_stat(self)->st_rdev));
#else
return Qnil;
#endif
}










static VALUE
rb_stat_size(VALUE self)
{
return OFFT2NUM(get_stat(self)->st_size);
}












static VALUE
rb_stat_blksize(VALUE self)
{
#if defined(HAVE_STRUCT_STAT_ST_BLKSIZE)
return ULONG2NUM(get_stat(self)->st_blksize);
#else
return Qnil;
#endif
}












static VALUE
rb_stat_blocks(VALUE self)
{
#if defined(HAVE_STRUCT_STAT_ST_BLOCKS)
#if SIZEOF_STRUCT_STAT_ST_BLOCKS > SIZEOF_LONG
return ULL2NUM(get_stat(self)->st_blocks);
#else
return ULONG2NUM(get_stat(self)->st_blocks);
#endif
#else
return Qnil;
#endif
}

static struct timespec
stat_atimespec(const struct stat *st)
{
struct timespec ts;
ts.tv_sec = st->st_atime;
#if defined(HAVE_STRUCT_STAT_ST_ATIM)
ts.tv_nsec = st->st_atim.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_ATIMESPEC)
ts.tv_nsec = st->st_atimespec.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_ATIMENSEC)
ts.tv_nsec = (long)st->st_atimensec;
#else
ts.tv_nsec = 0;
#endif
return ts;
}

static VALUE
stat_atime(const struct stat *st)
{
struct timespec ts = stat_atimespec(st);
return rb_time_nano_new(ts.tv_sec, ts.tv_nsec);
}

static struct timespec
stat_mtimespec(const struct stat *st)
{
struct timespec ts;
ts.tv_sec = st->st_mtime;
#if defined(HAVE_STRUCT_STAT_ST_MTIM)
ts.tv_nsec = st->st_mtim.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_MTIMESPEC)
ts.tv_nsec = st->st_mtimespec.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_MTIMENSEC)
ts.tv_nsec = (long)st->st_mtimensec;
#else
ts.tv_nsec = 0;
#endif
return ts;
}

static VALUE
stat_mtime(const struct stat *st)
{
struct timespec ts = stat_mtimespec(st);
return rb_time_nano_new(ts.tv_sec, ts.tv_nsec);
}

static struct timespec
stat_ctimespec(const struct stat *st)
{
struct timespec ts;
ts.tv_sec = st->st_ctime;
#if defined(HAVE_STRUCT_STAT_ST_CTIM)
ts.tv_nsec = st->st_ctim.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_CTIMESPEC)
ts.tv_nsec = st->st_ctimespec.tv_nsec;
#elif defined(HAVE_STRUCT_STAT_ST_CTIMENSEC)
ts.tv_nsec = (long)st->st_ctimensec;
#else
ts.tv_nsec = 0;
#endif
return ts;
}

static VALUE
stat_ctime(const struct stat *st)
{
struct timespec ts = stat_ctimespec(st);
return rb_time_nano_new(ts.tv_sec, ts.tv_nsec);
}

#define HAVE_STAT_BIRTHTIME
#if defined(HAVE_STRUCT_STAT_ST_BIRTHTIMESPEC)
typedef struct stat statx_data;
static VALUE
stat_birthtime(const struct stat *st)
{
const struct timespec *ts = &st->st_birthtimespec;
return rb_time_nano_new(ts->tv_sec, ts->tv_nsec);
}
#elif defined(_WIN32)
typedef struct stat statx_data;
#define stat_birthtime stat_ctime
#else
#undef HAVE_STAT_BIRTHTIME
#endif












static VALUE
rb_stat_atime(VALUE self)
{
return stat_atime(get_stat(self));
}











static VALUE
rb_stat_mtime(VALUE self)
{
return stat_mtime(get_stat(self));
}















static VALUE
rb_stat_ctime(VALUE self)
{
return stat_ctime(get_stat(self));
}

#if defined(HAVE_STAT_BIRTHTIME)






















static VALUE
rb_stat_birthtime(VALUE self)
{
return stat_birthtime(get_stat(self));
}
#else
#define rb_stat_birthtime rb_f_notimplement
#endif
















static VALUE
rb_stat_inspect(VALUE self)
{
VALUE str;
size_t i;
static const struct {
const char *name;
VALUE (*func)(VALUE);
} member[] = {
{"dev", rb_stat_dev},
{"ino", rb_stat_ino},
{"mode", rb_stat_mode},
{"nlink", rb_stat_nlink},
{"uid", rb_stat_uid},
{"gid", rb_stat_gid},
{"rdev", rb_stat_rdev},
{"size", rb_stat_size},
{"blksize", rb_stat_blksize},
{"blocks", rb_stat_blocks},
{"atime", rb_stat_atime},
{"mtime", rb_stat_mtime},
{"ctime", rb_stat_ctime},
#if defined(HAVE_STRUCT_STAT_ST_BIRTHTIMESPEC)
{"birthtime", rb_stat_birthtime},
#endif
};

struct stat* st;
TypedData_Get_Struct(self, struct stat, &stat_data_type, st);
if (!st) {
return rb_sprintf("#<%s: uninitialized>", rb_obj_classname(self));
}

str = rb_str_buf_new2("#<");
rb_str_buf_cat2(str, rb_obj_classname(self));
rb_str_buf_cat2(str, " ");

for (i = 0; i < sizeof(member)/sizeof(member[0]); i++) {
VALUE v;

if (i > 0) {
rb_str_buf_cat2(str, ", ");
}
rb_str_buf_cat2(str, member[i].name);
rb_str_buf_cat2(str, "=");
v = (*member[i].func)(self);
if (i == 2) { 
rb_str_catf(str, "0%lo", (unsigned long)NUM2ULONG(v));
}
else if (i == 0 || i == 6) { 
rb_str_catf(str, "0x%"PRI_DEVT_PREFIX"x", NUM2DEVT(v));
}
else {
rb_str_append(str, rb_inspect(v));
}
}
rb_str_buf_cat2(str, ">");

return str;
}

typedef struct no_gvl_stat_data {
struct stat *st;
union {
const char *path;
int fd;
} file;
} no_gvl_stat_data;

static VALUE
no_gvl_fstat(void *data)
{
no_gvl_stat_data *arg = data;
return (VALUE)fstat(arg->file.fd, arg->st);
}

static int
fstat_without_gvl(int fd, struct stat *st)
{
no_gvl_stat_data data;

data.file.fd = fd;
data.st = st;

return (int)(VALUE)rb_thread_io_blocking_region(no_gvl_fstat, &data, fd);
}

static void *
no_gvl_stat(void * data)
{
no_gvl_stat_data *arg = data;
return (void *)(VALUE)STAT(arg->file.path, arg->st);
}

static int
stat_without_gvl(const char *path, struct stat *st)
{
no_gvl_stat_data data;

data.file.path = path;
data.st = st;

return (int)(VALUE)rb_thread_call_without_gvl(no_gvl_stat, &data,
RUBY_UBF_IO, NULL);
}

#if !defined(HAVE_STRUCT_STAT_ST_BIRTHTIMESPEC) && defined(HAVE_STRUCT_STATX_STX_BTIME)


#if !defined(HAVE_STATX)
#if defined(HAVE_SYSCALL_H)
#include <syscall.h>
#elif defined HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif
#if defined __linux__
#include <linux/stat.h>
static inline int
statx(int dirfd, const char *pathname, int flags,
unsigned int mask, struct statx *statxbuf)
{
return (int)syscall(__NR_statx, dirfd, pathname, flags, mask, statxbuf);
}
#endif
#endif

typedef struct no_gvl_statx_data {
struct statx *stx;
int fd;
const char *path;
int flags;
unsigned int mask;
} no_gvl_statx_data;

static VALUE
io_blocking_statx(void *data)
{
no_gvl_statx_data *arg = data;
return (VALUE)statx(arg->fd, arg->path, arg->flags, arg->mask, arg->stx);
}

static void *
no_gvl_statx(void *data)
{
return (void *)io_blocking_statx(data);
}

static int
statx_without_gvl(const char *path, struct statx *stx, unsigned int mask)
{
no_gvl_statx_data data = {stx, AT_FDCWD, path, 0, mask};


return (int)(VALUE)rb_thread_call_without_gvl(no_gvl_statx, &data,
RUBY_UBF_IO, NULL);
}

static int
fstatx_without_gvl(int fd, struct statx *stx, unsigned int mask)
{
no_gvl_statx_data data = {stx, fd, "", AT_EMPTY_PATH, mask};


return (int)rb_thread_io_blocking_region(io_blocking_statx, &data, fd);
}

static int
rb_statx(VALUE file, struct statx *stx, unsigned int mask)
{
VALUE tmp;
int result;

tmp = rb_check_convert_type_with_id(file, T_FILE, "IO", idTo_io);
if (!NIL_P(tmp)) {
rb_io_t *fptr;
GetOpenFile(tmp, fptr);
result = fstatx_without_gvl(fptr->fd, stx, mask);
file = tmp;
}
else {
FilePathValue(file);
file = rb_str_encode_ospath(file);
result = statx_without_gvl(RSTRING_PTR(file), stx, mask);
}
RB_GC_GUARD(file);
return result;
}

#define statx_has_birthtime(st) ((st)->stx_mask & STATX_BTIME)

NORETURN(static void statx_notimplement(const char *field_name));



static void
statx_notimplement(const char *field_name)
{
rb_raise(rb_eNotImpError,
"%s is unimplemented on this filesystem",
field_name);
}

static VALUE
statx_birthtime(const struct statx *stx, VALUE fname)
{
if (!statx_has_birthtime(stx)) {

statx_notimplement("birthtime");
}
return rb_time_nano_new(stx->stx_btime.tv_sec, stx->stx_btime.tv_nsec);
}

typedef struct statx statx_data;
#define HAVE_STAT_BIRTHTIME

#elif defined(HAVE_STAT_BIRTHTIME)
#define statx_without_gvl(path, st, mask) stat_without_gvl(path, st)
#define fstatx_without_gvl(fd, st, mask) fstat_without_gvl(fd, st)
#define statx_birthtime(st, fname) stat_birthtime(st)
#define statx_has_birthtime(st) 1
#define rb_statx(file, st, mask) rb_stat(file, st)
#else
#define statx_has_birthtime(st) 0
#endif

static int
rb_stat(VALUE file, struct stat *st)
{
VALUE tmp;
int result;

tmp = rb_check_convert_type_with_id(file, T_FILE, "IO", idTo_io);
if (!NIL_P(tmp)) {
rb_io_t *fptr;

GetOpenFile(tmp, fptr);
result = fstat_without_gvl(fptr->fd, st);
file = tmp;
}
else {
FilePathValue(file);
file = rb_str_encode_ospath(file);
result = stat_without_gvl(RSTRING_PTR(file), st);
}
RB_GC_GUARD(file);
return result;
}











static VALUE
rb_file_s_stat(VALUE klass, VALUE fname)
{
struct stat st;

FilePathValue(fname);
fname = rb_str_encode_ospath(fname);
if (stat_without_gvl(RSTRING_PTR(fname), &st) < 0) {
rb_sys_fail_path(fname);
}
return rb_stat_new(&st);
}
















static VALUE
rb_io_stat(VALUE obj)
{
rb_io_t *fptr;
struct stat st;

GetOpenFile(obj, fptr);
if (fstat(fptr->fd, &st) == -1) {
rb_sys_fail_path(fptr->pathv);
}
return rb_stat_new(&st);
}

#if defined(HAVE_LSTAT)
static void *
no_gvl_lstat(void *ptr)
{
no_gvl_stat_data *arg = ptr;
return (void *)(VALUE)lstat(arg->file.path, arg->st);
}

static int
lstat_without_gvl(const char *path, struct stat *st)
{
no_gvl_stat_data data;

data.file.path = path;
data.st = st;

return (int)(VALUE)rb_thread_call_without_gvl(no_gvl_lstat, &data,
RUBY_UBF_IO, NULL);
}
#endif 















static VALUE
rb_file_s_lstat(VALUE klass, VALUE fname)
{
#if defined(HAVE_LSTAT)
struct stat st;

FilePathValue(fname);
fname = rb_str_encode_ospath(fname);
if (lstat_without_gvl(StringValueCStr(fname), &st) == -1) {
rb_sys_fail_path(fname);
}
return rb_stat_new(&st);
#else
return rb_file_s_stat(klass, fname);
#endif
}















static VALUE
rb_file_lstat(VALUE obj)
{
#if defined(HAVE_LSTAT)
rb_io_t *fptr;
struct stat st;
VALUE path;

GetOpenFile(obj, fptr);
if (NIL_P(fptr->pathv)) return Qnil;
path = rb_str_encode_ospath(fptr->pathv);
if (lstat_without_gvl(RSTRING_PTR(path), &st) == -1) {
rb_sys_fail_path(fptr->pathv);
}
return rb_stat_new(&st);
#else
return rb_io_stat(obj);
#endif
}

static int
rb_group_member(GETGROUPS_T gid)
{
#if defined(_WIN32) || !defined(HAVE_GETGROUPS)
return FALSE;
#else
int rv = FALSE;
int groups = 16;
VALUE v = 0;
GETGROUPS_T *gary;
int anum = -1;

if (getgid() == gid || getegid() == gid)
return TRUE;






while (groups <= RB_MAX_GROUPS) {
gary = ALLOCV_N(GETGROUPS_T, v, groups);
anum = getgroups(groups, gary);
if (anum != -1 && anum != groups)
break;
groups *= 2;
if (v) {
ALLOCV_END(v);
v = 0;
}
}
if (anum == -1)
return FALSE;

while (--anum >= 0) {
if (gary[anum] == gid) {
rv = TRUE;
break;
}
}
if (v)
ALLOCV_END(v);

return rv;
#endif
}

#if !defined(S_IXUGO)
#define S_IXUGO (S_IXUSR | S_IXGRP | S_IXOTH)
#endif

#if defined(S_IXGRP) && !defined(_WIN32) && !defined(__CYGWIN__)
#define USE_GETEUID 1
#endif

#if !defined(HAVE_EACCESS)
int
eaccess(const char *path, int mode)
{
#if defined(USE_GETEUID)
struct stat st;
rb_uid_t euid;

euid = geteuid();


if (getuid() == euid && getgid() == getegid())
return access(path, mode);

if (STAT(path, &st) < 0)
return -1;

if (euid == 0) {

if (!(mode & X_OK))
return 0;



if (st.st_mode & S_IXUGO)
return 0;

return -1;
}

if (st.st_uid == euid) 
mode <<= 6;
else if (rb_group_member(st.st_gid))
mode <<= 3;

if ((int)(st.st_mode & mode) == mode) return 0;

return -1;
#else
return access(path, mode);
#endif
}
#endif

struct access_arg {
const char *path;
int mode;
};

static void *
nogvl_eaccess(void *ptr)
{
struct access_arg *aa = ptr;

return (void *)(VALUE)eaccess(aa->path, aa->mode);
}

static int
rb_eaccess(VALUE fname, int mode)
{
struct access_arg aa;

FilePathValue(fname);
fname = rb_str_encode_ospath(fname);
aa.path = StringValueCStr(fname);
aa.mode = mode;

return (int)(VALUE)rb_thread_call_without_gvl(nogvl_eaccess, &aa,
RUBY_UBF_IO, 0);
}

static void *
nogvl_access(void *ptr)
{
struct access_arg *aa = ptr;

return (void *)(VALUE)access(aa->path, aa->mode);
}

static int
rb_access(VALUE fname, int mode)
{
struct access_arg aa;

FilePathValue(fname);
fname = rb_str_encode_ospath(fname);
aa.path = StringValueCStr(fname);
aa.mode = mode;

return (int)(VALUE)rb_thread_call_without_gvl(nogvl_access, &aa,
RUBY_UBF_IO, 0);
}


























VALUE
rb_file_directory_p(VALUE obj, VALUE fname)
{
#if !defined(S_ISDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (S_ISDIR(st.st_mode)) return Qtrue;
return Qfalse;
}










static VALUE
rb_file_pipe_p(VALUE obj, VALUE fname)
{
#if defined(S_IFIFO)
#if !defined(S_ISFIFO)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif

struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (S_ISFIFO(st.st_mode)) return Qtrue;

#endif
return Qfalse;
}








static VALUE
rb_file_symlink_p(VALUE obj, VALUE fname)
{
#if !defined(S_ISLNK)
#if defined(_S_ISLNK)
#define S_ISLNK(m) _S_ISLNK(m)
#else
#if defined(_S_IFLNK)
#define S_ISLNK(m) (((m) & S_IFMT) == _S_IFLNK)
#else
#if defined(S_IFLNK)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#endif
#endif
#endif

#if defined(S_ISLNK)
struct stat st;

FilePathValue(fname);
fname = rb_str_encode_ospath(fname);
if (lstat_without_gvl(StringValueCStr(fname), &st) < 0) return Qfalse;
if (S_ISLNK(st.st_mode)) return Qtrue;
#endif

return Qfalse;
}










static VALUE
rb_file_socket_p(VALUE obj, VALUE fname)
{
#if !defined(S_ISSOCK)
#if defined(_S_ISSOCK)
#define S_ISSOCK(m) _S_ISSOCK(m)
#else
#if defined(_S_IFSOCK)
#define S_ISSOCK(m) (((m) & S_IFMT) == _S_IFSOCK)
#else
#if defined(S_IFSOCK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif
#endif
#endif
#endif

#if defined(S_ISSOCK)
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (S_ISSOCK(st.st_mode)) return Qtrue;

#endif
return Qfalse;
}










static VALUE
rb_file_blockdev_p(VALUE obj, VALUE fname)
{
#if !defined(S_ISBLK)
#if defined(S_IFBLK)
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#else
#define S_ISBLK(m) (0) 
#endif
#endif

#if defined(S_ISBLK)
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (S_ISBLK(st.st_mode)) return Qtrue;

#endif
return Qfalse;
}









static VALUE
rb_file_chardev_p(VALUE obj, VALUE fname)
{
#if !defined(S_ISCHR)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif

struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (S_ISCHR(st.st_mode)) return Qtrue;

return Qfalse;
}












static VALUE
rb_file_exist_p(VALUE obj, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
return Qtrue;
}







static VALUE
rb_file_exists_p(VALUE obj, VALUE fname)
{
const char *s = "FileTest#";
if (obj == rb_mFileTest) {
s = "FileTest.";
}
else if (obj == rb_cFile ||
(RB_TYPE_P(obj, T_CLASS) &&
RTEST(rb_class_inherited_p(obj, rb_cFile)))) {
s = "File.";
}
rb_warning("%sexists? is a deprecated name, use %sexist? instead", s, s);
return rb_file_exist_p(obj, fname);
}












static VALUE
rb_file_readable_p(VALUE obj, VALUE fname)
{
if (rb_eaccess(fname, R_OK) < 0) return Qfalse;
return Qtrue;
}












static VALUE
rb_file_readable_real_p(VALUE obj, VALUE fname)
{
if (rb_access(fname, R_OK) < 0) return Qfalse;
return Qtrue;
}

#if !defined(S_IRUGO)
#define S_IRUGO (S_IRUSR | S_IRGRP | S_IROTH)
#endif

#if !defined(S_IWUGO)
#define S_IWUGO (S_IWUSR | S_IWGRP | S_IWOTH)
#endif

















static VALUE
rb_file_world_readable_p(VALUE obj, VALUE fname)
{
#if defined(S_IROTH)
struct stat st;

if (rb_stat(fname, &st) < 0) return Qnil;
if ((st.st_mode & (S_IROTH)) == S_IROTH) {
return UINT2NUM(st.st_mode & (S_IRUGO|S_IWUGO|S_IXUGO));
}
#endif
return Qnil;
}












static VALUE
rb_file_writable_p(VALUE obj, VALUE fname)
{
if (rb_eaccess(fname, W_OK) < 0) return Qfalse;
return Qtrue;
}












static VALUE
rb_file_writable_real_p(VALUE obj, VALUE fname)
{
if (rb_access(fname, W_OK) < 0) return Qfalse;
return Qtrue;
}

















static VALUE
rb_file_world_writable_p(VALUE obj, VALUE fname)
{
#if defined(S_IWOTH)
struct stat st;

if (rb_stat(fname, &st) < 0) return Qnil;
if ((st.st_mode & (S_IWOTH)) == S_IWOTH) {
return UINT2NUM(st.st_mode & (S_IRUGO|S_IWUGO|S_IXUGO));
}
#endif
return Qnil;
}
















static VALUE
rb_file_executable_p(VALUE obj, VALUE fname)
{
if (rb_eaccess(fname, X_OK) < 0) return Qfalse;
return Qtrue;
}
















static VALUE
rb_file_executable_real_p(VALUE obj, VALUE fname)
{
if (rb_access(fname, X_OK) < 0) return Qfalse;
return Qtrue;
}

#if !defined(S_ISREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif













static VALUE
rb_file_file_p(VALUE obj, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (S_ISREG(st.st_mode)) return Qtrue;
return Qfalse;
}











static VALUE
rb_file_zero_p(VALUE obj, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (st.st_size == 0) return Qtrue;
return Qfalse;
}











static VALUE
rb_file_size_p(VALUE obj, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) return Qnil;
if (st.st_size == 0) return Qnil;
return OFFT2NUM(st.st_size);
}












static VALUE
rb_file_owned_p(VALUE obj, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (st.st_uid == geteuid()) return Qtrue;
return Qfalse;
}

static VALUE
rb_file_rowned_p(VALUE obj, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (st.st_uid == getuid()) return Qtrue;
return Qfalse;
}












static VALUE
rb_file_grpowned_p(VALUE obj, VALUE fname)
{
#if !defined(_WIN32)
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (rb_group_member(st.st_gid)) return Qtrue;
#endif
return Qfalse;
}

#if defined(S_ISUID) || defined(S_ISGID) || defined(S_ISVTX)
static VALUE
check3rdbyte(VALUE fname, int mode)
{
struct stat st;

if (rb_stat(fname, &st) < 0) return Qfalse;
if (st.st_mode & mode) return Qtrue;
return Qfalse;
}
#endif










static VALUE
rb_file_suid_p(VALUE obj, VALUE fname)
{
#if defined(S_ISUID)
return check3rdbyte(fname, S_ISUID);
#else
return Qfalse;
#endif
}










static VALUE
rb_file_sgid_p(VALUE obj, VALUE fname)
{
#if defined(S_ISGID)
return check3rdbyte(fname, S_ISGID);
#else
return Qfalse;
#endif
}










static VALUE
rb_file_sticky_p(VALUE obj, VALUE fname)
{
#if defined(S_ISVTX)
return check3rdbyte(fname, S_ISVTX);
#else
return Qnil;
#endif
}




















static VALUE
rb_file_identical_p(VALUE obj, VALUE fname1, VALUE fname2)
{
#if !defined(_WIN32)
struct stat st1, st2;

if (rb_stat(fname1, &st1) < 0) return Qfalse;
if (rb_stat(fname2, &st2) < 0) return Qfalse;
if (st1.st_dev != st2.st_dev) return Qfalse;
if (st1.st_ino != st2.st_ino) return Qfalse;
return Qtrue;
#else
extern VALUE rb_w32_file_identical_p(VALUE, VALUE);
return rb_w32_file_identical_p(fname1, fname2);
#endif
}










static VALUE
rb_file_s_size(VALUE klass, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) {
int e = errno;
FilePathValue(fname);
rb_syserr_fail_path(e, fname);
}
return OFFT2NUM(st.st_size);
}

static VALUE
rb_file_ftype(const struct stat *st)
{
const char *t;

if (S_ISREG(st->st_mode)) {
t = "file";
}
else if (S_ISDIR(st->st_mode)) {
t = "directory";
}
else if (S_ISCHR(st->st_mode)) {
t = "characterSpecial";
}
#if defined(S_ISBLK)
else if (S_ISBLK(st->st_mode)) {
t = "blockSpecial";
}
#endif
#if defined(S_ISFIFO)
else if (S_ISFIFO(st->st_mode)) {
t = "fifo";
}
#endif
#if defined(S_ISLNK)
else if (S_ISLNK(st->st_mode)) {
t = "link";
}
#endif
#if defined(S_ISSOCK)
else if (S_ISSOCK(st->st_mode)) {
t = "socket";
}
#endif
else {
t = "unknown";
}

return rb_usascii_str_new2(t);
}
















static VALUE
rb_file_s_ftype(VALUE klass, VALUE fname)
{
struct stat st;

FilePathValue(fname);
fname = rb_str_encode_ospath(fname);
if (lstat_without_gvl(StringValueCStr(fname), &st) == -1) {
rb_sys_fail_path(fname);
}

return rb_file_ftype(&st);
}













static VALUE
rb_file_s_atime(VALUE klass, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) {
int e = errno;
FilePathValue(fname);
rb_syserr_fail_path(e, fname);
}
return stat_atime(&st);
}












static VALUE
rb_file_atime(VALUE obj)
{
rb_io_t *fptr;
struct stat st;

GetOpenFile(obj, fptr);
if (fstat(fptr->fd, &st) == -1) {
rb_sys_fail_path(fptr->pathv);
}
return stat_atime(&st);
}













static VALUE
rb_file_s_mtime(VALUE klass, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) {
int e = errno;
FilePathValue(fname);
rb_syserr_fail_path(e, fname);
}
return stat_mtime(&st);
}











static VALUE
rb_file_mtime(VALUE obj)
{
rb_io_t *fptr;
struct stat st;

GetOpenFile(obj, fptr);
if (fstat(fptr->fd, &st) == -1) {
rb_sys_fail_path(fptr->pathv);
}
return stat_mtime(&st);
}

















static VALUE
rb_file_s_ctime(VALUE klass, VALUE fname)
{
struct stat st;

if (rb_stat(fname, &st) < 0) {
int e = errno;
FilePathValue(fname);
rb_syserr_fail_path(e, fname);
}
return stat_ctime(&st);
}














static VALUE
rb_file_ctime(VALUE obj)
{
rb_io_t *fptr;
struct stat st;

GetOpenFile(obj, fptr);
if (fstat(fptr->fd, &st) == -1) {
rb_sys_fail_path(fptr->pathv);
}
return stat_ctime(&st);
}















#if defined(HAVE_STAT_BIRTHTIME)
RUBY_FUNC_EXPORTED VALUE
rb_file_s_birthtime(VALUE klass, VALUE fname)
{
statx_data st;

if (rb_statx(fname, &st, STATX_BTIME) < 0) {
int e = errno;
FilePathValue(fname);
rb_syserr_fail_path(e, fname);
}
return statx_birthtime(&st, fname);
}
#else
#define rb_file_s_birthtime rb_f_notimplement
#endif

#if defined(HAVE_STAT_BIRTHTIME)












static VALUE
rb_file_birthtime(VALUE obj)
{
rb_io_t *fptr;
statx_data st;

GetOpenFile(obj, fptr);
if (fstatx_without_gvl(fptr->fd, &st, STATX_BTIME) == -1) {
rb_sys_fail_path(fptr->pathv);
}
return statx_birthtime(&st, fptr->pathv);
}
#else
#define rb_file_birthtime rb_f_notimplement
#endif











static VALUE
rb_file_size(VALUE obj)
{
rb_io_t *fptr;
struct stat st;

GetOpenFile(obj, fptr);
if (fptr->mode & FMODE_WRITABLE) {
rb_io_flush_raw(obj, 0);
}
if (fstat(fptr->fd, &st) == -1) {
rb_sys_fail_path(fptr->pathv);
}
return OFFT2NUM(st.st_size);
}

static int
chmod_internal(const char *path, void *mode)
{
return chmod(path, *(mode_t *)mode);
}














static VALUE
rb_file_s_chmod(int argc, VALUE *argv, VALUE _)
{
mode_t mode;

apply2args(1);
mode = NUM2MODET(*argv++);

return apply2files(chmod_internal, argc, argv, &mode);
}














static VALUE
rb_file_chmod(VALUE obj, VALUE vmode)
{
rb_io_t *fptr;
mode_t mode;
#if !defined HAVE_FCHMOD || !HAVE_FCHMOD
VALUE path;
#endif

mode = NUM2MODET(vmode);

GetOpenFile(obj, fptr);
#if defined(HAVE_FCHMOD)
if (fchmod(fptr->fd, mode) == -1) {
if (HAVE_FCHMOD || errno != ENOSYS)
rb_sys_fail_path(fptr->pathv);
}
else {
if (!HAVE_FCHMOD) return INT2FIX(0);
}
#endif
#if !defined HAVE_FCHMOD || !HAVE_FCHMOD
if (NIL_P(fptr->pathv)) return Qnil;
path = rb_str_encode_ospath(fptr->pathv);
if (chmod(RSTRING_PTR(path), mode) == -1)
rb_sys_fail_path(fptr->pathv);
#endif

return INT2FIX(0);
}

#if defined(HAVE_LCHMOD)
static int
lchmod_internal(const char *path, void *mode)
{
return lchmod(path, *(mode_t *)mode);
}











static VALUE
rb_file_s_lchmod(int argc, VALUE *argv, VALUE _)
{
mode_t mode;

apply2args(1);
mode = NUM2MODET(*argv++);

return apply2files(lchmod_internal, argc, argv, &mode);
}
#else
#define rb_file_s_lchmod rb_f_notimplement
#endif

static inline rb_uid_t
to_uid(VALUE u)
{
if (NIL_P(u)) {
return (rb_uid_t)-1;
}
return NUM2UIDT(u);
}

static inline rb_gid_t
to_gid(VALUE g)
{
if (NIL_P(g)) {
return (rb_gid_t)-1;
}
return NUM2GIDT(g);
}

struct chown_args {
rb_uid_t owner;
rb_gid_t group;
};

static int
chown_internal(const char *path, void *arg)
{
struct chown_args *args = arg;
return chown(path, args->owner, args->group);
}
















static VALUE
rb_file_s_chown(int argc, VALUE *argv, VALUE _)
{
struct chown_args arg;

apply2args(2);
arg.owner = to_uid(*argv++);
arg.group = to_gid(*argv++);

return apply2files(chown_internal, argc, argv, &arg);
}
















static VALUE
rb_file_chown(VALUE obj, VALUE owner, VALUE group)
{
rb_io_t *fptr;
rb_uid_t o;
rb_gid_t g;
#if !defined(HAVE_FCHOWN)
VALUE path;
#endif

o = to_uid(owner);
g = to_gid(group);
GetOpenFile(obj, fptr);
#if !defined(HAVE_FCHOWN)
if (NIL_P(fptr->pathv)) return Qnil;
path = rb_str_encode_ospath(fptr->pathv);
if (chown(RSTRING_PTR(path), o, g) == -1)
rb_sys_fail_path(fptr->pathv);
#else
if (fchown(fptr->fd, o, g) == -1)
rb_sys_fail_path(fptr->pathv);
#endif

return INT2FIX(0);
}

#if defined(HAVE_LCHOWN)
static int
lchown_internal(const char *path, void *arg)
{
struct chown_args *args = arg;
return lchown(path, args->owner, args->group);
}












static VALUE
rb_file_s_lchown(int argc, VALUE *argv, VALUE _)
{
struct chown_args arg;

apply2args(2);
arg.owner = to_uid(*argv++);
arg.group = to_gid(*argv++);

return apply2files(lchown_internal, argc, argv, &arg);
}
#else
#define rb_file_s_lchown rb_f_notimplement
#endif

struct utime_args {
const struct timespec* tsp;
VALUE atime, mtime;
int follow; 
};

#if defined(UTIME_EINVAL)
NORETURN(static void utime_failed(struct apply_arg *));

static void
utime_failed(struct apply_arg *aa)
{
int e = aa->errnum;
VALUE path = aa->fn[aa->i].path;
struct utime_args *ua = aa->arg;

if (ua->tsp && e == EINVAL) {
VALUE e[2], a = Qnil, m = Qnil;
int d = 0;
VALUE atime = ua->atime;
VALUE mtime = ua->mtime;

if (!NIL_P(atime)) {
a = rb_inspect(atime);
}
if (!NIL_P(mtime) && mtime != atime && !rb_equal(atime, mtime)) {
m = rb_inspect(mtime);
}
if (NIL_P(a)) e[0] = m;
else if (NIL_P(m) || rb_str_cmp(a, m) == 0) e[0] = a;
else {
e[0] = rb_str_plus(a, rb_str_new_cstr(" or "));
rb_str_append(e[0], m);
d = 1;
}
if (!NIL_P(e[0])) {
if (path) {
if (!d) e[0] = rb_str_dup(e[0]);
rb_str_append(rb_str_cat2(e[0], " for "), path);
}
e[1] = INT2FIX(EINVAL);
rb_exc_raise(rb_class_new_instance(2, e, rb_eSystemCallError));
}
}
rb_syserr_fail_path(e, path);
}
#endif

#if defined(HAVE_UTIMES)

static int
utime_internal(const char *path, void *arg)
{
struct utime_args *v = arg;
const struct timespec *tsp = v->tsp;
struct timeval tvbuf[2], *tvp = NULL;

#if defined(HAVE_UTIMENSAT)
static int try_utimensat = 1;
#if defined(AT_SYMLINK_NOFOLLOW)
static int try_utimensat_follow = 1;
#else
const int try_utimensat_follow = 0;
#endif
int flags = 0;

if (v->follow ? try_utimensat_follow : try_utimensat) {
#if defined(AT_SYMLINK_NOFOLLOW)
if (v->follow) {
flags = AT_SYMLINK_NOFOLLOW;
}
#endif

if (utimensat(AT_FDCWD, path, tsp, flags) < 0) {
if (errno == ENOSYS) {
#if defined(AT_SYMLINK_NOFOLLOW)
try_utimensat_follow = 0;
#endif
if (!v->follow)
try_utimensat = 0;
goto no_utimensat;
}
return -1; 
}
return 0;
}
no_utimensat:
#endif

if (tsp) {
tvbuf[0].tv_sec = tsp[0].tv_sec;
tvbuf[0].tv_usec = (int)(tsp[0].tv_nsec / 1000);
tvbuf[1].tv_sec = tsp[1].tv_sec;
tvbuf[1].tv_usec = (int)(tsp[1].tv_nsec / 1000);
tvp = tvbuf;
}
#if defined(HAVE_LUTIMES)
if (v->follow) return lutimes(path, tvp);
#endif
return utimes(path, tvp);
}

#else

#if !defined HAVE_UTIME_H && !defined HAVE_SYS_UTIME_H
struct utimbuf {
long actime;
long modtime;
};
#endif

static int
utime_internal(const char *path, void *arg)
{
struct utime_args *v = arg;
const struct timespec *tsp = v->tsp;
struct utimbuf utbuf, *utp = NULL;
if (tsp) {
utbuf.actime = tsp[0].tv_sec;
utbuf.modtime = tsp[1].tv_sec;
utp = &utbuf;
}
return utime(path, utp);
}

#endif

static VALUE
utime_internal_i(int argc, VALUE *argv, int follow)
{
struct utime_args args;
struct timespec tss[2], *tsp = NULL;

apply2args(2);
args.atime = *argv++;
args.mtime = *argv++;

args.follow = follow;

if (!NIL_P(args.atime) || !NIL_P(args.mtime)) {
tsp = tss;
tsp[0] = rb_time_timespec(args.atime);
if (args.atime == args.mtime)
tsp[1] = tsp[0];
else
tsp[1] = rb_time_timespec(args.mtime);
}
args.tsp = tsp;

return apply2files(utime_internal, argc, argv, &args);
}












static VALUE
rb_file_s_utime(int argc, VALUE *argv, VALUE _)
{
return utime_internal_i(argc, argv, FALSE);
}

#if defined(HAVE_UTIMES) && (defined(HAVE_LUTIMES) || (defined(HAVE_UTIMENSAT) && defined(AT_SYMLINK_NOFOLLOW)))












static VALUE
rb_file_s_lutime(int argc, VALUE *argv, VALUE _)
{
return utime_internal_i(argc, argv, TRUE);
}
#else
#define rb_file_s_lutime rb_f_notimplement
#endif

#if defined(RUBY_FUNCTION_NAME_STRING)
#define syserr_fail2(e, s1, s2) syserr_fail2_in(RUBY_FUNCTION_NAME_STRING, e, s1, s2)
#else
#define syserr_fail2_in(func, e, s1, s2) syserr_fail2(e, s1, s2)
#endif
#define sys_fail2(s1, s2) syserr_fail2(errno, s1, s2)
NORETURN(static void syserr_fail2_in(const char *,int,VALUE,VALUE));
static void
syserr_fail2_in(const char *func, int e, VALUE s1, VALUE s2)
{
VALUE str;
#if defined(MAX_PATH)
const int max_pathlen = MAX_PATH;
#else
const int max_pathlen = MAXPATHLEN;
#endif

if (e == EEXIST) {
rb_syserr_fail_path(e, rb_str_ellipsize(s2, max_pathlen));
}
str = rb_str_new_cstr("(");
rb_str_append(str, rb_str_ellipsize(s1, max_pathlen));
rb_str_cat2(str, ", ");
rb_str_append(str, rb_str_ellipsize(s2, max_pathlen));
rb_str_cat2(str, ")");
#if defined(RUBY_FUNCTION_NAME_STRING)
rb_syserr_fail_path_in(func, e, str);
#else
rb_syserr_fail_path(e, str);
#endif
}

#if defined(HAVE_LINK)












static VALUE
rb_file_s_link(VALUE klass, VALUE from, VALUE to)
{
FilePathValue(from);
FilePathValue(to);
from = rb_str_encode_ospath(from);
to = rb_str_encode_ospath(to);

if (link(StringValueCStr(from), StringValueCStr(to)) < 0) {
sys_fail2(from, to);
}
return INT2FIX(0);
}
#else
#define rb_file_s_link rb_f_notimplement
#endif

#if defined(HAVE_SYMLINK)












static VALUE
rb_file_s_symlink(VALUE klass, VALUE from, VALUE to)
{
FilePathValue(from);
FilePathValue(to);
from = rb_str_encode_ospath(from);
to = rb_str_encode_ospath(to);

if (symlink(StringValueCStr(from), StringValueCStr(to)) < 0) {
sys_fail2(from, to);
}
return INT2FIX(0);
}
#else
#define rb_file_s_symlink rb_f_notimplement
#endif

#if defined(HAVE_READLINK)











static VALUE
rb_file_s_readlink(VALUE klass, VALUE path)
{
return rb_readlink(path, rb_filesystem_encoding());
}

#if !defined(_WIN32)
struct readlink_arg {
const char *path;
char *buf;
size_t size;
};

static void *
nogvl_readlink(void *ptr)
{
struct readlink_arg *ra = ptr;

return (void *)(VALUE)readlink(ra->path, ra->buf, ra->size);
}

static ssize_t
readlink_without_gvl(VALUE path, VALUE buf, size_t size)
{
struct readlink_arg ra;

ra.path = RSTRING_PTR(path);
ra.buf = RSTRING_PTR(buf);
ra.size = size;

return (ssize_t)rb_thread_call_without_gvl(nogvl_readlink, &ra,
RUBY_UBF_IO, 0);
}

VALUE
rb_readlink(VALUE path, rb_encoding *enc)
{
int size = 100;
ssize_t rv;
VALUE v;

FilePathValue(path);
path = rb_str_encode_ospath(path);
v = rb_enc_str_new(0, size, enc);
while ((rv = readlink_without_gvl(path, v, size)) == size
#if defined(_AIX)
|| (rv < 0 && errno == ERANGE) 
#endif
) {
rb_str_modify_expand(v, size);
size *= 2;
rb_str_set_len(v, size);
}
if (rv < 0) {
int e = errno;
rb_str_resize(v, 0);
rb_syserr_fail_path(e, path);
}
rb_str_resize(v, rv);

return v;
}
#endif
#else
#define rb_file_s_readlink rb_f_notimplement
#endif

static int
unlink_internal(const char *path, void *arg)
{
return unlink(path);
}

















static VALUE
rb_file_s_unlink(int argc, VALUE *argv, VALUE klass)
{
return apply2files(unlink_internal, argc, argv, 0);
}

struct rename_args {
const char *src;
const char *dst;
};

static void *
no_gvl_rename(void *ptr)
{
struct rename_args *ra = ptr;

return (void *)(VALUE)rename(ra->src, ra->dst);
}











static VALUE
rb_file_s_rename(VALUE klass, VALUE from, VALUE to)
{
struct rename_args ra;
VALUE f, t;

FilePathValue(from);
FilePathValue(to);
f = rb_str_encode_ospath(from);
t = rb_str_encode_ospath(to);
ra.src = StringValueCStr(f);
ra.dst = StringValueCStr(t);
#if defined __CYGWIN__
errno = 0;
#endif
if ((int)(VALUE)rb_thread_call_without_gvl(no_gvl_rename, &ra,
RUBY_UBF_IO, 0) < 0) {
int e = errno;
#if defined DOSISH
switch (e) {
case EEXIST:
if (chmod(ra.dst, 0666) == 0 &&
unlink(ra.dst) == 0 &&
rename(ra.src, ra.dst) == 0)
return INT2FIX(0);
}
#endif
syserr_fail2(e, from, to);
}

return INT2FIX(0);
}
















static VALUE
rb_file_s_umask(int argc, VALUE *argv, VALUE _)
{
mode_t omask = 0;

switch (argc) {
case 0:
omask = umask(0);
umask(omask);
break;
case 1:
omask = umask(NUM2MODET(argv[0]));
break;
default:
rb_error_arity(argc, 0, 1);
}
return MODET2NUM(omask);
}

#if defined(__CYGWIN__)
#undef DOSISH
#endif
#if defined __CYGWIN__ || defined DOSISH
#define DOSISH_UNC
#define DOSISH_DRIVE_LETTER
#define FILE_ALT_SEPARATOR '\\'
#endif
#if defined(FILE_ALT_SEPARATOR)
#define isdirsep(x) ((x) == '/' || (x) == FILE_ALT_SEPARATOR)
#if defined(DOSISH)
static const char file_alt_separator[] = {FILE_ALT_SEPARATOR, '\0'};
#endif
#else
#define isdirsep(x) ((x) == '/')
#endif

#if !defined(USE_NTFS)
#if defined _WIN32
#define USE_NTFS 1
#else
#define USE_NTFS 0
#endif
#endif
#if !defined(USE_NTFS_ADS)
#if USE_NTFS
#define USE_NTFS_ADS 1
#else
#define USE_NTFS_ADS 0
#endif
#endif

#if USE_NTFS
#define istrailinggarbage(x) ((x) == '.' || (x) == ' ')
#else
#define istrailinggarbage(x) 0
#endif
#if USE_NTFS_ADS
#define isADS(x) ((x) == ':')
#else
#define isADS(x) 0
#endif

#define Next(p, e, enc) ((p) + rb_enc_mbclen((p), (e), (enc)))
#define Inc(p, e, enc) ((p) = Next((p), (e), (enc)))

#if defined(DOSISH_UNC)
#define has_unc(buf) (isdirsep((buf)[0]) && isdirsep((buf)[1]))
#else
#define has_unc(buf) 0
#endif

#if defined(DOSISH_DRIVE_LETTER)
static inline int
has_drive_letter(const char *buf)
{
if (ISALPHA(buf[0]) && buf[1] == ':') {
return 1;
}
else {
return 0;
}
}

#if !defined(_WIN32)
static char*
getcwdofdrv(int drv)
{
char drive[4];
char *drvcwd, *oldcwd;

drive[0] = drv;
drive[1] = ':';
drive[2] = '\0';





oldcwd = ruby_getcwd();
if (chdir(drive) == 0) {
drvcwd = ruby_getcwd();
chdir(oldcwd);
xfree(oldcwd);
}
else {

drvcwd = strdup(drive);
}
return drvcwd;
}
#endif

static inline int
not_same_drive(VALUE path, int drive)
{
const char *p = RSTRING_PTR(path);
if (RSTRING_LEN(path) < 2) return 0;
if (has_drive_letter(p)) {
return TOLOWER(p[0]) != TOLOWER(drive);
}
else {
return has_unc(p);
}
}
#endif

static inline char *
skiproot(const char *path, const char *end, rb_encoding *enc)
{
#if defined(DOSISH_DRIVE_LETTER)
if (path + 2 <= end && has_drive_letter(path)) path += 2;
#endif
while (path < end && isdirsep(*path)) path++;
return (char *)path;
}

#define nextdirsep rb_enc_path_next
char *
rb_enc_path_next(const char *s, const char *e, rb_encoding *enc)
{
while (s < e && !isdirsep(*s)) {
Inc(s, e, enc);
}
return (char *)s;
}

#if defined(DOSISH_UNC) || defined(DOSISH_DRIVE_LETTER)
#define skipprefix rb_enc_path_skip_prefix
#else
#define skipprefix(path, end, enc) (path)
#endif
char *
rb_enc_path_skip_prefix(const char *path, const char *end, rb_encoding *enc)
{
#if defined(DOSISH_UNC) || defined(DOSISH_DRIVE_LETTER)
#if defined(DOSISH_UNC)
if (path + 2 <= end && isdirsep(path[0]) && isdirsep(path[1])) {
path += 2;
while (path < end && isdirsep(*path)) path++;
if ((path = rb_enc_path_next(path, end, enc)) < end && path[0] && path[1] && !isdirsep(path[1]))
path = rb_enc_path_next(path + 1, end, enc);
return (char *)path;
}
#endif
#if defined(DOSISH_DRIVE_LETTER)
if (has_drive_letter(path))
return (char *)(path + 2);
#endif
#endif
return (char *)path;
}

static inline char *
skipprefixroot(const char *path, const char *end, rb_encoding *enc)
{
#if defined(DOSISH_UNC) || defined(DOSISH_DRIVE_LETTER)
char *p = skipprefix(path, end, enc);
while (isdirsep(*p)) p++;
return p;
#else
return skiproot(path, end, enc);
#endif
}

#define strrdirsep rb_enc_path_last_separator
char *
rb_enc_path_last_separator(const char *path, const char *end, rb_encoding *enc)
{
char *last = NULL;
while (path < end) {
if (isdirsep(*path)) {
const char *tmp = path++;
while (path < end && isdirsep(*path)) path++;
if (path >= end) break;
last = (char *)tmp;
}
else {
Inc(path, end, enc);
}
}
return last;
}

static char *
chompdirsep(const char *path, const char *end, rb_encoding *enc)
{
while (path < end) {
if (isdirsep(*path)) {
const char *last = path++;
while (path < end && isdirsep(*path)) path++;
if (path >= end) return (char *)last;
}
else {
Inc(path, end, enc);
}
}
return (char *)path;
}

char *
rb_enc_path_end(const char *path, const char *end, rb_encoding *enc)
{
if (path < end && isdirsep(*path)) path++;
return chompdirsep(path, end, enc);
}

#if USE_NTFS
static char *
ntfs_tail(const char *path, const char *end, rb_encoding *enc)
{
while (path < end && *path == '.') path++;
while (path < end && !isADS(*path)) {
if (istrailinggarbage(*path)) {
const char *last = path++;
while (path < end && istrailinggarbage(*path)) path++;
if (path >= end || isADS(*path)) return (char *)last;
}
else if (isdirsep(*path)) {
const char *last = path++;
while (path < end && isdirsep(*path)) path++;
if (path >= end) return (char *)last;
if (isADS(*path)) path++;
}
else {
Inc(path, end, enc);
}
}
return (char *)path;
}
#endif

#define BUFCHECK(cond) do {bdiff = p - buf;if (cond) {do {buflen *= 2;} while (cond);rb_str_resize(result, buflen);buf = RSTRING_PTR(result);p = buf + bdiff;pend = buf + buflen;}} while (0)










#define BUFINIT() (p = buf = RSTRING_PTR(result),buflen = RSTRING_LEN(result),pend = p + buflen)




#if defined(__APPLE__)
#define SKIPPATHSEP(p) ((*(p)) ? 1 : 0)
#else
#define SKIPPATHSEP(p) 1
#endif

#define BUFCOPY(srcptr, srclen) do { const int skip = SKIPPATHSEP(p); rb_str_set_len(result, p-buf+skip); BUFCHECK(bdiff + ((srclen)+skip) >= buflen); p += skip; memcpy(p, (srcptr), (srclen)); p += (srclen); } while (0)








#define WITH_ROOTDIFF(stmt) do { long rootdiff = root - buf; stmt; root = buf + rootdiff; } while (0)





static VALUE
copy_home_path(VALUE result, const char *dir)
{
char *buf;
#if defined DOSISH || defined __CYGWIN__
char *p, *bend;
rb_encoding *enc;
#endif
long dirlen;
int encidx;

dirlen = strlen(dir);
rb_str_resize(result, dirlen);
memcpy(buf = RSTRING_PTR(result), dir, dirlen);
encidx = rb_filesystem_encindex();
rb_enc_associate_index(result, encidx);
#if defined DOSISH || defined __CYGWIN__
enc = rb_enc_from_index(encidx);
for (bend = (p = buf) + dirlen; p < bend; Inc(p, bend, enc)) {
if (*p == '\\') {
*p = '/';
}
}
#endif
return result;
}

VALUE
rb_home_dir_of(VALUE user, VALUE result)
{
#if defined(HAVE_PWD_H)
struct passwd *pwPtr;
#else
extern char *getlogin(void);
const char *pwPtr = 0;
#define endpwent() ((void)0)
#endif
const char *dir, *username = RSTRING_PTR(user);
rb_encoding *enc = rb_enc_get(user);
#if defined _WIN32
rb_encoding *fsenc = rb_utf8_encoding();
#else
rb_encoding *fsenc = rb_filesystem_encoding();
#endif
if (enc != fsenc) {
dir = username = RSTRING_PTR(rb_str_conv_enc(user, enc, fsenc));
}

#if defined(HAVE_PWD_H)
pwPtr = getpwnam(username);
#else
if (strcasecmp(username, getlogin()) == 0)
dir = pwPtr = getenv("HOME");
#endif
if (!pwPtr) {
endpwent();
rb_raise(rb_eArgError, "user %"PRIsVALUE" doesn't exist", user);
}
#if defined(HAVE_PWD_H)
dir = pwPtr->pw_dir;
#endif
copy_home_path(result, dir);
endpwent();
return result;
}

#if !defined(_WIN32)
VALUE
rb_default_home_dir(VALUE result)
{
const char *dir = getenv("HOME");

#if defined HAVE_PWD_H
if (!dir) {
const char *login = getlogin();
if (login) {
struct passwd *pw = getpwnam(login);
if (pw) {
copy_home_path(result, pw->pw_dir);
endpwent();
return result;
}
endpwent();
rb_raise(rb_eArgError, "couldn't find HOME for login `%s' -- expanding `~'",
login);
}
else {
rb_raise(rb_eArgError, "couldn't find login name -- expanding `~'");
}
}
#endif
if (!dir) {
rb_raise(rb_eArgError, "couldn't find HOME environment -- expanding `~'");
}
return copy_home_path(result, dir);
}

static VALUE
ospath_new(const char *ptr, long len, rb_encoding *fsenc)
{
#if NORMALIZE_UTF8PATH
VALUE path = rb_str_normalize_ospath(ptr, len);
rb_enc_associate(path, fsenc);
return path;
#else
return rb_enc_str_new(ptr, len, fsenc);
#endif
}

static char *
append_fspath(VALUE result, VALUE fname, char *dir, rb_encoding **enc, rb_encoding *fsenc)
{
char *buf, *cwdp = dir;
VALUE dirname = Qnil;
size_t dirlen = strlen(dir), buflen = rb_str_capacity(result);

if (NORMALIZE_UTF8PATH || *enc != fsenc) {
rb_encoding *direnc = rb_enc_check(fname, dirname = ospath_new(dir, dirlen, fsenc));
if (direnc != fsenc) {
dirname = rb_str_conv_enc(dirname, fsenc, direnc);
RSTRING_GETMEM(dirname, cwdp, dirlen);
}
else if (NORMALIZE_UTF8PATH) {
RSTRING_GETMEM(dirname, cwdp, dirlen);
}
*enc = direnc;
}
do {buflen *= 2;} while (dirlen > buflen);
rb_str_resize(result, buflen);
buf = RSTRING_PTR(result);
memcpy(buf, cwdp, dirlen);
xfree(dir);
if (!NIL_P(dirname)) rb_str_resize(dirname, 0);
rb_enc_associate(result, *enc);
return buf + dirlen;
}

VALUE
rb_file_expand_path_internal(VALUE fname, VALUE dname, int abs_mode, int long_name, VALUE result)
{
const char *s, *b, *fend;
char *buf, *p, *pend, *root;
size_t buflen, bdiff;
rb_encoding *enc, *fsenc = rb_filesystem_encoding();

s = StringValuePtr(fname);
fend = s + RSTRING_LEN(fname);
enc = rb_enc_get(fname);
BUFINIT();

if (s[0] == '~' && abs_mode == 0) { 
long userlen = 0;
if (isdirsep(s[1]) || s[1] == '\0') {
buf = 0;
b = 0;
rb_str_set_len(result, 0);
if (*++s) ++s;
rb_default_home_dir(result);
}
else {
s = nextdirsep(b = s, fend, enc);
b++; 
userlen = s - b;
BUFCHECK(bdiff + userlen >= buflen);
memcpy(p, b, userlen);
ENC_CODERANGE_CLEAR(result);
rb_str_set_len(result, userlen);
rb_enc_associate(result, enc);
rb_home_dir_of(result, result);
buf = p + 1;
p += userlen;
}
if (!rb_is_absolute_path(RSTRING_PTR(result))) {
if (userlen) {
rb_enc_raise(enc, rb_eArgError, "non-absolute home of %.*s%.0"PRIsVALUE,
(int)userlen, b, fname);
}
else {
rb_raise(rb_eArgError, "non-absolute home");
}
}
BUFINIT();
p = pend;
}
#if defined(DOSISH_DRIVE_LETTER)

else if (has_drive_letter(s)) {
if (isdirsep(s[2])) {


BUFCHECK(bdiff + 2 >= buflen);
memcpy(p, s, 2);
p += 2;
s += 2;
rb_enc_copy(result, fname);
}
else {

int same = 0;
if (!NIL_P(dname) && !not_same_drive(dname, s[0])) {
rb_file_expand_path_internal(dname, Qnil, abs_mode, long_name, result);
BUFINIT();
if (has_drive_letter(p) && TOLOWER(p[0]) == TOLOWER(s[0])) {

same = 1;
}
}
if (!same) {
char *e = append_fspath(result, fname, getcwdofdrv(*s), &enc, fsenc);
BUFINIT();
p = e;
}
else {
rb_enc_associate(result, enc = rb_enc_check(result, fname));
p = pend;
}
p = chompdirsep(skiproot(buf, p, enc), p, enc);
s += 2;
}
}
#endif
else if (!rb_is_absolute_path(s)) {
if (!NIL_P(dname)) {
rb_file_expand_path_internal(dname, Qnil, abs_mode, long_name, result);
rb_enc_associate(result, rb_enc_check(result, fname));
BUFINIT();
p = pend;
}
else {
char *e = append_fspath(result, fname, ruby_getcwd(), &enc, fsenc);
BUFINIT();
p = e;
}
#if defined DOSISH || defined __CYGWIN__
if (isdirsep(*s)) {


p = skipprefix(buf, p, enc);
}
else
#endif
p = chompdirsep(skiproot(buf, p, enc), p, enc);
}
else {
size_t len;
b = s;
do s++; while (isdirsep(*s));
len = s - b;
p = buf + len;
BUFCHECK(bdiff >= buflen);
memset(buf, '/', len);
rb_str_set_len(result, len);
rb_enc_associate(result, rb_enc_check(result, fname));
}
if (p > buf && p[-1] == '/')
--p;
else {
rb_str_set_len(result, p-buf);
BUFCHECK(bdiff + 1 >= buflen);
*p = '/';
}

rb_str_set_len(result, p-buf+1);
BUFCHECK(bdiff + 1 >= buflen);
p[1] = 0;
root = skipprefix(buf, p+1, enc);

b = s;
while (*s) {
switch (*s) {
case '.':
if (b == s++) { 
switch (*s) {
case '\0':
b = s;
break;
case '.':
if (*(s+1) == '\0' || isdirsep(*(s+1))) {

char *n;
*p = '\0';
if (!(n = strrdirsep(root, p, enc))) {
*p = '/';
}
else {
p = n;
}
b = ++s;
}
#if USE_NTFS
else {
do ++s; while (istrailinggarbage(*s));
}
#endif
break;
case '/':
#if defined DOSISH || defined __CYGWIN__
case '\\':
#endif
b = ++s;
break;
default:

break;
}
}
#if USE_NTFS
else {
--s;
case ' ': {
const char *e = s;
while (s < fend && istrailinggarbage(*s)) s++;
if (s >= fend) {
s = e;
goto endpath;
}
}
}
#endif
break;
case '/':
#if defined DOSISH || defined __CYGWIN__
case '\\':
#endif
if (s > b) {
WITH_ROOTDIFF(BUFCOPY(b, s-b));
*p = '/';
}
b = ++s;
break;
default:
#if defined(__APPLE__)
{
int n = ignored_char_p(s, fend, enc);
if (n) {
if (s > b) {
WITH_ROOTDIFF(BUFCOPY(b, s-b));
*p = '\0';
}
b = s += n;
break;
}
}
#endif
Inc(s, fend, enc);
break;
}
}

if (s > b) {
#if USE_NTFS
#if USE_NTFS_ADS
static const char prime[] = ":$DATA";
enum {prime_len = sizeof(prime) -1};
#endif
endpath:
#if USE_NTFS_ADS
if (s > b + prime_len && strncasecmp(s - prime_len, prime, prime_len) == 0) {


if (isADS(*(s - (prime_len+1)))) {
s -= prime_len + 1; 
}
else if (memchr(b, ':', s - prime_len - b)) {
s -= prime_len; 
}
}
#endif
#endif
BUFCOPY(b, s-b);
rb_str_set_len(result, p-buf);
}
if (p == skiproot(buf, p + !!*p, enc) - 1) p++;

#if USE_NTFS
*p = '\0';
if ((s = strrdirsep(b = buf, p, enc)) != 0 && !strpbrk(s, "*?")) {
VALUE tmp, v;
size_t len;
int encidx;
WCHAR *wstr;
WIN32_FIND_DATAW wfd;
HANDLE h;
#if defined(__CYGWIN__)
#if defined(HAVE_CYGWIN_CONV_PATH)
char *w32buf = NULL;
const int flags = CCP_POSIX_TO_WIN_A | CCP_RELATIVE;
#else
char w32buf[MAXPATHLEN];
#endif
const char *path;
ssize_t bufsize;
int lnk_added = 0, is_symlink = 0;
struct stat st;
p = (char *)s;
len = strlen(p);
if (lstat_without_gvl(buf, &st) == 0 && S_ISLNK(st.st_mode)) {
is_symlink = 1;
if (len > 4 && STRCASECMP(p + len - 4, ".lnk") != 0) {
lnk_added = 1;
}
}
path = *buf ? buf : "/";
#if defined(HAVE_CYGWIN_CONV_PATH)
bufsize = cygwin_conv_path(flags, path, NULL, 0);
if (bufsize > 0) {
bufsize += len;
if (lnk_added) bufsize += 4;
w32buf = ALLOCA_N(char, bufsize);
if (cygwin_conv_path(flags, path, w32buf, bufsize) == 0) {
b = w32buf;
}
}
#else
bufsize = MAXPATHLEN;
if (cygwin_conv_to_win32_path(path, w32buf) == 0) {
b = w32buf;
}
#endif
if (is_symlink && b == w32buf) {
*p = '\\';
strlcat(w32buf, p, bufsize);
if (lnk_added) {
strlcat(w32buf, ".lnk", bufsize);
}
}
else {
lnk_added = 0;
}
*p = '/';
#endif
rb_str_set_len(result, p - buf + strlen(p));
encidx = ENCODING_GET(result);
tmp = result;
if (encidx != ENCINDEX_UTF_8 && rb_enc_str_coderange(result) != ENC_CODERANGE_7BIT) {
tmp = rb_str_encode_ospath(result);
}
len = MultiByteToWideChar(CP_UTF8, 0, RSTRING_PTR(tmp), -1, NULL, 0);
wstr = ALLOCV_N(WCHAR, v, len);
MultiByteToWideChar(CP_UTF8, 0, RSTRING_PTR(tmp), -1, wstr, len);
if (tmp != result) rb_str_set_len(tmp, 0);
h = FindFirstFileW(wstr, &wfd);
ALLOCV_END(v);
if (h != INVALID_HANDLE_VALUE) {
size_t wlen;
FindClose(h);
len = lstrlenW(wfd.cFileName);
#if defined(__CYGWIN__)
if (lnk_added && len > 4 &&
wcscasecmp(wfd.cFileName + len - 4, L".lnk") == 0) {
wfd.cFileName[len -= 4] = L'\0';
}
#else
p = (char *)s;
#endif
++p;
wlen = (int)len;
len = WideCharToMultiByte(CP_UTF8, 0, wfd.cFileName, wlen, NULL, 0, NULL, NULL);
if (tmp == result) {
BUFCHECK(bdiff + len >= buflen);
WideCharToMultiByte(CP_UTF8, 0, wfd.cFileName, wlen, p, len + 1, NULL, NULL);
}
else {
rb_str_modify_expand(tmp, len);
WideCharToMultiByte(CP_UTF8, 0, wfd.cFileName, wlen, RSTRING_PTR(tmp), len + 1, NULL, NULL);
rb_str_cat_conv_enc_opts(result, bdiff, RSTRING_PTR(tmp), len,
rb_utf8_encoding(), 0, Qnil);
BUFINIT();
rb_str_resize(tmp, 0);
}
p += len;
}
#if defined(__CYGWIN__)
else {
p += strlen(p);
}
#endif
}
#endif

rb_str_set_len(result, p - buf);
rb_enc_check(fname, result);
ENC_CODERANGE_CLEAR(result);
return result;
}
#endif 

#define EXPAND_PATH_BUFFER() rb_usascii_str_new(0, MAXPATHLEN + 2)

static VALUE
str_shrink(VALUE str)
{
rb_str_resize(str, RSTRING_LEN(str));
return str;
}

#define expand_path(fname, dname, abs_mode, long_name, result) str_shrink(rb_file_expand_path_internal(fname, dname, abs_mode, long_name, result))


#define check_expand_path_args(fname, dname) (((fname) = rb_get_path(fname)), (void)(NIL_P(dname) ? (dname) : ((dname) = rb_get_path(dname))))



static VALUE
file_expand_path_1(VALUE fname)
{
return rb_file_expand_path_internal(fname, Qnil, 0, 0, EXPAND_PATH_BUFFER());
}

VALUE
rb_file_expand_path(VALUE fname, VALUE dname)
{
check_expand_path_args(fname, dname);
return expand_path(fname, dname, 0, 1, EXPAND_PATH_BUFFER());
}

VALUE
rb_file_expand_path_fast(VALUE fname, VALUE dname)
{
return expand_path(fname, dname, 0, 0, EXPAND_PATH_BUFFER());
}

VALUE
rb_file_s_expand_path(int argc, const VALUE *argv)
{
rb_check_arity(argc, 1, 2);
return rb_file_expand_path(argv[0], argc > 1 ? argv[1] : Qnil);
}





























static VALUE
s_expand_path(int c, const VALUE * v, VALUE _)
{
return rb_file_s_expand_path(c, v);
}

VALUE
rb_file_absolute_path(VALUE fname, VALUE dname)
{
check_expand_path_args(fname, dname);
return expand_path(fname, dname, 1, 1, EXPAND_PATH_BUFFER());
}

VALUE
rb_file_s_absolute_path(int argc, const VALUE *argv)
{
rb_check_arity(argc, 1, 2);
return rb_file_absolute_path(argv[0], argc > 1 ? argv[1] : Qnil);
}














static VALUE
s_absolute_path(int c, const VALUE * v, VALUE _)
{
return rb_file_s_absolute_path(c, v);
}











static VALUE
s_absolute_path_p(VALUE klass, VALUE fname)
{
VALUE path = rb_get_path(fname);

if (!rb_is_absolute_path(RSTRING_PTR(path))) return Qfalse;
return Qtrue;
}

enum rb_realpath_mode {
RB_REALPATH_CHECK,
RB_REALPATH_DIR,
RB_REALPATH_STRICT,
RB_REALPATH_MODE_MAX
};

static int
realpath_rec(long *prefixlenp, VALUE *resolvedp, const char *unresolved, VALUE fallback,
VALUE loopcheck, enum rb_realpath_mode mode, int last)
{
const char *pend = unresolved + strlen(unresolved);
rb_encoding *enc = rb_enc_get(*resolvedp);
ID resolving;
CONST_ID(resolving, "resolving");
while (unresolved < pend) {
const char *testname = unresolved;
const char *unresolved_firstsep = rb_enc_path_next(unresolved, pend, enc);
long testnamelen = unresolved_firstsep - unresolved;
const char *unresolved_nextname = unresolved_firstsep;
while (unresolved_nextname < pend && isdirsep(*unresolved_nextname))
unresolved_nextname++;
unresolved = unresolved_nextname;
if (testnamelen == 1 && testname[0] == '.') {
}
else if (testnamelen == 2 && testname[0] == '.' && testname[1] == '.') {
if (*prefixlenp < RSTRING_LEN(*resolvedp)) {
const char *resolved_str = RSTRING_PTR(*resolvedp);
const char *resolved_names = resolved_str + *prefixlenp;
const char *lastsep = strrdirsep(resolved_names, resolved_str + RSTRING_LEN(*resolvedp), enc);
long len = lastsep ? lastsep - resolved_names : 0;
rb_str_resize(*resolvedp, *prefixlenp + len);
}
}
else {
VALUE checkval;
VALUE testpath = rb_str_dup(*resolvedp);
if (*prefixlenp < RSTRING_LEN(testpath))
rb_str_cat2(testpath, "/");
#if defined(DOSISH_UNC) || defined(DOSISH_DRIVE_LETTER)
if (*prefixlenp > 1 && *prefixlenp == RSTRING_LEN(testpath)) {
const char *prefix = RSTRING_PTR(testpath);
const char *last = rb_enc_left_char_head(prefix, prefix + *prefixlenp - 1, prefix + *prefixlenp, enc);
if (!isdirsep(*last)) rb_str_cat2(testpath, "/");
}
#endif
rb_str_cat(testpath, testname, testnamelen);
checkval = rb_hash_aref(loopcheck, testpath);
if (!NIL_P(checkval)) {
if (checkval == ID2SYM(resolving)) {
if (mode == RB_REALPATH_CHECK) {
errno = ELOOP;
return -1;
}
rb_syserr_fail_path(ELOOP, testpath);
}
else {
*resolvedp = rb_str_dup(checkval);
}
}
else {
struct stat sbuf;
int ret;
ret = lstat_without_gvl(RSTRING_PTR(testpath), &sbuf);
if (ret == -1) {
int e = errno;
if (e == ENOENT && !NIL_P(fallback)) {
if (stat_without_gvl(RSTRING_PTR(fallback), &sbuf) == 0) {
rb_str_replace(*resolvedp, fallback);
return 0;
}
}
if (mode == RB_REALPATH_CHECK) return -1;
if (e == ENOENT) {
if (mode == RB_REALPATH_STRICT || !last || *unresolved_firstsep)
rb_syserr_fail_path(e, testpath);
*resolvedp = testpath;
break;
}
else {
rb_syserr_fail_path(e, testpath);
}
}
#if defined(HAVE_READLINK)
if (S_ISLNK(sbuf.st_mode)) {
VALUE link;
VALUE link_orig = Qnil;
const char *link_prefix, *link_names;
long link_prefixlen;
rb_hash_aset(loopcheck, testpath, ID2SYM(resolving));
link = rb_readlink(testpath, enc);
link_prefix = RSTRING_PTR(link);
link_names = skipprefixroot(link_prefix, link_prefix + RSTRING_LEN(link), rb_enc_get(link));
link_prefixlen = link_names - link_prefix;
if (link_prefixlen > 0) {
rb_encoding *tmpenc, *linkenc = rb_enc_get(link);
link_orig = link;
link = rb_str_subseq(link, 0, link_prefixlen);
tmpenc = rb_enc_check(*resolvedp, link);
if (tmpenc != linkenc) link = rb_str_conv_enc(link, linkenc, tmpenc);
*resolvedp = link;
*prefixlenp = link_prefixlen;
}
if (realpath_rec(prefixlenp, resolvedp, link_names, testpath,
loopcheck, mode, !*unresolved_firstsep))
return -1;
RB_GC_GUARD(link_orig);
rb_hash_aset(loopcheck, testpath, rb_str_dup_frozen(*resolvedp));
}
else
#endif
{
VALUE s = rb_str_dup_frozen(testpath);
rb_hash_aset(loopcheck, s, s);
*resolvedp = testpath;
}
}
}
}
return 0;
}

static VALUE
rb_check_realpath_emulate(VALUE basedir, VALUE path, rb_encoding *origenc, enum rb_realpath_mode mode)
{
long prefixlen;
VALUE resolved;
VALUE unresolved_path;
VALUE loopcheck;
VALUE curdir = Qnil;

rb_encoding *enc;
char *path_names = NULL, *basedir_names = NULL, *curdir_names = NULL;
char *ptr, *prefixptr = NULL, *pend;
long len;

unresolved_path = rb_str_dup_frozen(path);

if (!NIL_P(basedir)) {
FilePathValue(basedir);
basedir = TO_OSPATH(rb_str_dup_frozen(basedir));
}

enc = rb_enc_get(unresolved_path);
unresolved_path = TO_OSPATH(unresolved_path);
RSTRING_GETMEM(unresolved_path, ptr, len);
path_names = skipprefixroot(ptr, ptr + len, rb_enc_get(unresolved_path));
if (ptr != path_names) {
resolved = rb_str_subseq(unresolved_path, 0, path_names - ptr);
goto root_found;
}

if (!NIL_P(basedir)) {
RSTRING_GETMEM(basedir, ptr, len);
basedir_names = skipprefixroot(ptr, ptr + len, rb_enc_get(basedir));
if (ptr != basedir_names) {
resolved = rb_str_subseq(basedir, 0, basedir_names - ptr);
goto root_found;
}
}

curdir = rb_dir_getwd_ospath();
RSTRING_GETMEM(curdir, ptr, len);
curdir_names = skipprefixroot(ptr, ptr + len, rb_enc_get(curdir));
resolved = rb_str_subseq(curdir, 0, curdir_names - ptr);

root_found:
RSTRING_GETMEM(resolved, prefixptr, prefixlen);
pend = prefixptr + prefixlen;
ptr = chompdirsep(prefixptr, pend, enc);
if (ptr < pend) {
prefixlen = ++ptr - prefixptr;
rb_str_set_len(resolved, prefixlen);
}
#if defined(FILE_ALT_SEPARATOR)
while (prefixptr < ptr) {
if (*prefixptr == FILE_ALT_SEPARATOR) {
*prefixptr = '/';
}
Inc(prefixptr, pend, enc);
}
#endif

switch (rb_enc_to_index(enc)) {
case ENCINDEX_ASCII:
case ENCINDEX_US_ASCII:
rb_enc_associate_index(resolved, rb_filesystem_encindex());
}

loopcheck = rb_hash_new();
if (curdir_names) {
if (realpath_rec(&prefixlen, &resolved, curdir_names, Qnil, loopcheck, mode, 0))
return Qnil;
}
if (basedir_names) {
if (realpath_rec(&prefixlen, &resolved, basedir_names, Qnil, loopcheck, mode, 0))
return Qnil;
}
if (realpath_rec(&prefixlen, &resolved, path_names, Qnil, loopcheck, mode, 1))
return Qnil;

if (origenc && origenc != rb_enc_get(resolved)) {
if (rb_enc_str_asciionly_p(resolved)) {
rb_enc_associate(resolved, origenc);
}
else {
resolved = rb_str_conv_enc(resolved, NULL, origenc);
}
}

RB_GC_GUARD(unresolved_path);
RB_GC_GUARD(curdir);
return resolved;
}

static VALUE rb_file_join(VALUE ary);

static VALUE
rb_check_realpath_internal(VALUE basedir, VALUE path, rb_encoding *origenc, enum rb_realpath_mode mode)
{
#if defined(HAVE_REALPATH)
VALUE unresolved_path;
char *resolved_ptr = NULL;
VALUE resolved;
struct stat st;

if (mode == RB_REALPATH_DIR) {
return rb_check_realpath_emulate(basedir, path, origenc, mode);
}

unresolved_path = rb_str_dup_frozen(path);
if (*RSTRING_PTR(unresolved_path) != '/' && !NIL_P(basedir)) {
unresolved_path = rb_file_join(rb_assoc_new(basedir, unresolved_path));
}
if (origenc) unresolved_path = TO_OSPATH(unresolved_path);

if ((resolved_ptr = realpath(RSTRING_PTR(unresolved_path), NULL)) == NULL) {





if (errno == ENOTDIR ||
(errno == ENOENT && rb_file_exist_p(0, unresolved_path))) {
return rb_check_realpath_emulate(basedir, path, origenc, mode);

}
if (mode == RB_REALPATH_CHECK) {
return Qnil;
}
rb_sys_fail_path(unresolved_path);
}
resolved = ospath_new(resolved_ptr, strlen(resolved_ptr), rb_filesystem_encoding());
free(resolved_ptr);



if (stat_without_gvl(RSTRING_PTR(resolved), &st) < 0) {
if (mode == RB_REALPATH_CHECK) {
return Qnil;
}
rb_sys_fail_path(unresolved_path);
}

if (origenc && origenc != rb_enc_get(resolved)) {
if (!rb_enc_str_asciionly_p(resolved)) {
resolved = rb_str_conv_enc(resolved, NULL, origenc);
}
rb_enc_associate(resolved, origenc);
}

if (rb_enc_str_coderange(resolved) == ENC_CODERANGE_BROKEN) {
rb_enc_associate(resolved, rb_filesystem_encoding());
if (rb_enc_str_coderange(resolved) == ENC_CODERANGE_BROKEN) {
rb_enc_associate(resolved, rb_ascii8bit_encoding());
}
}

RB_GC_GUARD(unresolved_path);
return resolved;
#else
return rb_check_realpath_emulate(basedir, path, origenc, mode);
#endif 
}

VALUE
rb_realpath_internal(VALUE basedir, VALUE path, int strict)
{
const enum rb_realpath_mode mode =
strict ? RB_REALPATH_STRICT : RB_REALPATH_DIR;
return rb_check_realpath_internal(basedir, path, rb_enc_get(path), mode);
}

VALUE
rb_check_realpath(VALUE basedir, VALUE path, rb_encoding *enc)
{
return rb_check_realpath_internal(basedir, path, enc, RB_REALPATH_CHECK);
}














static VALUE
rb_file_s_realpath(int argc, VALUE *argv, VALUE klass)
{
VALUE basedir = (rb_check_arity(argc, 1, 2) > 1) ? argv[1] : Qnil;
VALUE path = argv[0];
FilePathValue(path);
return rb_realpath_internal(basedir, path, 1);
}













static VALUE
rb_file_s_realdirpath(int argc, VALUE *argv, VALUE klass)
{
VALUE basedir = (rb_check_arity(argc, 1, 2) > 1) ? argv[1] : Qnil;
VALUE path = argv[0];
FilePathValue(path);
return rb_realpath_internal(basedir, path, 0);
}

static size_t
rmext(const char *p, long l0, long l1, const char *e, long l2, rb_encoding *enc)
{
int len1, len2;
unsigned int c;
const char *s, *last;

if (!e || !l2) return 0;

c = rb_enc_codepoint_len(e, e + l2, &len1, enc);
if (rb_enc_ascget(e + len1, e + l2, &len2, enc) == '*' && len1 + len2 == l2) {
if (c == '.') return l0;
s = p;
e = p + l1;
last = e;
while (s < e) {
if (rb_enc_codepoint_len(s, e, &len1, enc) == c) last = s;
s += len1;
}
return last - p;
}
if (l1 < l2) return l1;

s = p+l1-l2;
if (rb_enc_left_char_head(p, s, p+l1, enc) != s) return 0;
#if CASEFOLD_FILESYSTEM
#define fncomp strncasecmp
#else
#define fncomp strncmp
#endif
if (fncomp(s, e, l2) == 0) {
return l1-l2;
}
return 0;
}

const char *
ruby_enc_find_basename(const char *name, long *baselen, long *alllen, rb_encoding *enc)
{
const char *p, *q, *e, *end;
#if defined DOSISH_DRIVE_LETTER || defined DOSISH_UNC
const char *root;
#endif
long f = 0, n = -1;

end = name + (alllen ? (size_t)*alllen : strlen(name));
name = skipprefix(name, end, enc);
#if defined DOSISH_DRIVE_LETTER || defined DOSISH_UNC
root = name;
#endif
while (isdirsep(*name))
name++;
if (!*name) {
p = name - 1;
f = 1;
#if defined DOSISH_DRIVE_LETTER || defined DOSISH_UNC
if (name != root) {

}
#if defined(DOSISH_DRIVE_LETTER)
else if (*p == ':') {
p++;
f = 0;
}
#endif
#if defined(DOSISH_UNC)
else {
p = "/";
}
#endif
#endif
}
else {
if (!(p = strrdirsep(name, end, enc))) {
p = name;
}
else {
while (isdirsep(*p)) p++; 
}
#if USE_NTFS
n = ntfs_tail(p, end, enc) - p;
#else
n = chompdirsep(p, end, enc) - p;
#endif
for (q = p; q - p < n && *q == '.'; q++);
for (e = 0; q - p < n; Inc(q, end, enc)) {
if (*q == '.') e = q;
}
if (e) f = e - p;
else f = n;
}

if (baselen)
*baselen = f;
if (alllen)
*alllen = n;
return p;
}


















static VALUE
rb_file_s_basename(int argc, VALUE *argv, VALUE _)
{
VALUE fname, fext, basename;
const char *name, *p;
long f, n;
rb_encoding *enc;

fext = Qnil;
if (rb_check_arity(argc, 1, 2) == 2) {
fext = argv[1];
StringValue(fext);
enc = check_path_encoding(fext);
}
fname = argv[0];
FilePathStringValue(fname);
if (NIL_P(fext) || !(enc = rb_enc_compatible(fname, fext))) {
enc = rb_enc_get(fname);
fext = Qnil;
}
if ((n = RSTRING_LEN(fname)) == 0 || !*(name = RSTRING_PTR(fname)))
return rb_str_new_shared(fname);

p = ruby_enc_find_basename(name, &f, &n, enc);
if (n >= 0) {
if (NIL_P(fext)) {
f = n;
}
else {
const char *fp;
fp = StringValueCStr(fext);
if (!(f = rmext(p, f, n, fp, RSTRING_LEN(fext), enc))) {
f = n;
}
RB_GC_GUARD(fext);
}
if (f == RSTRING_LEN(fname)) return rb_str_new_shared(fname);
}

basename = rb_str_new(p, f);
rb_enc_copy(basename, fname);
return basename;
}














static VALUE
rb_file_s_dirname(VALUE klass, VALUE fname)
{
return rb_file_dirname(fname);
}

VALUE
rb_file_dirname(VALUE fname)
{
const char *name, *root, *p, *end;
VALUE dirname;
rb_encoding *enc;

FilePathStringValue(fname);
name = StringValueCStr(fname);
end = name + RSTRING_LEN(fname);
enc = rb_enc_get(fname);
root = skiproot(name, end, enc);
#if defined(DOSISH_UNC)
if (root > name + 1 && isdirsep(*name))
root = skipprefix(name = root - 2, end, enc);
#else
if (root > name + 1)
name = root - 1;
#endif
p = strrdirsep(root, end, enc);
if (!p) {
p = root;
}
if (p == name)
return rb_usascii_str_new2(".");
#if defined(DOSISH_DRIVE_LETTER)
if (has_drive_letter(name) && isdirsep(*(name + 2))) {
const char *top = skiproot(name + 2, end, enc);
dirname = rb_str_new(name, 3);
rb_str_cat(dirname, top, p - top);
}
else
#endif
dirname = rb_str_new(name, p - name);
#if defined(DOSISH_DRIVE_LETTER)
if (has_drive_letter(name) && root == name + 2 && p - name == 2)
rb_str_cat(dirname, ".", 1);
#endif
rb_enc_copy(dirname, fname);
return dirname;
}













const char *
ruby_enc_find_extname(const char *name, long *len, rb_encoding *enc)
{
const char *p, *e, *end = name + (len ? *len : (long)strlen(name));

p = strrdirsep(name, end, enc); 
if (!p)
p = name;
else
do name = ++p; while (isdirsep(*p));

e = 0;
while (*p && *p == '.') p++;
while (*p) {
if (*p == '.' || istrailinggarbage(*p)) {
#if USE_NTFS
const char *last = p++, *dot = last;
while (istrailinggarbage(*p)) {
if (*p == '.') dot = p;
p++;
}
if (!*p || isADS(*p)) {
p = last;
break;
}
if (*last == '.' || dot > last) e = dot;
continue;
#else
e = p; 
#endif
}
#if USE_NTFS
else if (isADS(*p)) {
break;
}
#endif
else if (isdirsep(*p))
break;
Inc(p, end, enc);
}

if (len) {

if (!e || e == name)
*len = 0;
else if (e+1 == p)
*len = 1;
else
*len = p - e;
}
return e;
}



























static VALUE
rb_file_s_extname(VALUE klass, VALUE fname)
{
const char *name, *e;
long len;
VALUE extname;

FilePathStringValue(fname);
name = StringValueCStr(fname);
len = RSTRING_LEN(fname);
e = ruby_enc_find_extname(name, &len, rb_enc_get(fname));
if (len < 1)
return rb_str_new(0, 0);
extname = rb_str_subseq(fname, e - name, len); 
return extname;
}












static VALUE
rb_file_s_path(VALUE klass, VALUE fname)
{
return rb_get_path(fname);
}












static VALUE
rb_file_s_split(VALUE klass, VALUE path)
{
FilePathStringValue(path); 
return rb_assoc_new(rb_file_dirname(path), rb_file_s_basename(1,&path,Qundef));
}

static VALUE
file_inspect_join(VALUE ary, VALUE arg, int recur)
{
if (recur || ary == arg) rb_raise(rb_eArgError, "recursive array");
return rb_file_join(arg);
}

static VALUE
rb_file_join(VALUE ary)
{
long len, i;
VALUE result, tmp;
const char *name, *tail;
int checked = TRUE;
rb_encoding *enc;

if (RARRAY_LEN(ary) == 0) return rb_str_new(0, 0);

len = 1;
for (i=0; i<RARRAY_LEN(ary); i++) {
tmp = RARRAY_AREF(ary, i);
if (RB_TYPE_P(tmp, T_STRING)) {
check_path_encoding(tmp);
len += RSTRING_LEN(tmp);
}
else {
len += 10;
}
}
len += RARRAY_LEN(ary) - 1;
result = rb_str_buf_new(len);
RBASIC_CLEAR_CLASS(result);
for (i=0; i<RARRAY_LEN(ary); i++) {
tmp = RARRAY_AREF(ary, i);
switch (OBJ_BUILTIN_TYPE(tmp)) {
case T_STRING:
if (!checked) check_path_encoding(tmp);
StringValueCStr(tmp);
break;
case T_ARRAY:
if (ary == tmp) {
rb_raise(rb_eArgError, "recursive array");
}
else {
tmp = rb_exec_recursive(file_inspect_join, ary, tmp);
}
break;
default:
FilePathStringValue(tmp);
checked = FALSE;
}
RSTRING_GETMEM(result, name, len);
if (i == 0) {
rb_enc_copy(result, tmp);
}
else {
tail = chompdirsep(name, name + len, rb_enc_get(result));
if (RSTRING_PTR(tmp) && isdirsep(RSTRING_PTR(tmp)[0])) {
rb_str_set_len(result, tail - name);
}
else if (!*tail) {
rb_str_cat(result, "/", 1);
}
}
enc = rb_enc_check(result, tmp);
rb_str_buf_append(result, tmp);
rb_enc_associate(result, enc);
}
RBASIC_SET_CLASS_RAW(result, rb_cString);

return result;
}












static VALUE
rb_file_s_join(VALUE klass, VALUE args)
{
return rb_file_join(args);
}

#if defined(HAVE_TRUNCATE) || defined(HAVE_CHSIZE)
struct truncate_arg {
const char *path;
#if defined(HAVE_TRUNCATE)
#define NUM2POS(n) NUM2OFFT(n)
off_t pos;
#else
#define NUM2POS(n) NUM2LONG(n)
long pos;
#endif
};

static void *
nogvl_truncate(void *ptr)
{
struct truncate_arg *ta = ptr;
#if defined(HAVE_TRUNCATE)
return (void *)(VALUE)truncate(ta->path, ta->pos);
#else 
{
int tmpfd = rb_cloexec_open(ta->path, 0, 0);

if (tmpfd < 0)
return (void *)-1;
rb_update_max_fd(tmpfd);
if (chsize(tmpfd, ta->pos) < 0) {
int e = errno;
close(tmpfd);
errno = e;
return (void *)-1;
}
close(tmpfd);
return 0;
}
#endif
}
















static VALUE
rb_file_s_truncate(VALUE klass, VALUE path, VALUE len)
{
struct truncate_arg ta;
int r;

ta.pos = NUM2POS(len);
FilePathValue(path);
path = rb_str_encode_ospath(path);
ta.path = StringValueCStr(path);

r = (int)(VALUE)rb_thread_call_without_gvl(nogvl_truncate, &ta,
RUBY_UBF_IO, NULL);
if (r < 0)
rb_sys_fail_path(path);
return INT2FIX(0);
#undef NUM2POS
}
#else
#define rb_file_s_truncate rb_f_notimplement
#endif

#if defined(HAVE_FTRUNCATE) || defined(HAVE_CHSIZE)
struct ftruncate_arg {
int fd;
#if defined(HAVE_FTRUNCATE)
#define NUM2POS(n) NUM2OFFT(n)
off_t pos;
#else
#define NUM2POS(n) NUM2LONG(n)
long pos;
#endif
};

static VALUE
nogvl_ftruncate(void *ptr)
{
struct ftruncate_arg *fa = ptr;

#if defined(HAVE_FTRUNCATE)
return (VALUE)ftruncate(fa->fd, fa->pos);
#else 
return (VALUE)chsize(fa->fd, fa->pos);
#endif
}















static VALUE
rb_file_truncate(VALUE obj, VALUE len)
{
rb_io_t *fptr;
struct ftruncate_arg fa;

fa.pos = NUM2POS(len);
GetOpenFile(obj, fptr);
if (!(fptr->mode & FMODE_WRITABLE)) {
rb_raise(rb_eIOError, "not opened for writing");
}
rb_io_flush_raw(obj, 0);
fa.fd = fptr->fd;
if ((int)rb_thread_io_blocking_region(nogvl_ftruncate, &fa, fa.fd) < 0) {
rb_sys_fail_path(fptr->pathv);
}
return INT2FIX(0);
#undef NUM2POS
}
#else
#define rb_file_truncate rb_f_notimplement
#endif

#if !defined(LOCK_SH)
#define LOCK_SH 1
#endif
#if !defined(LOCK_EX)
#define LOCK_EX 2
#endif
#if !defined(LOCK_NB)
#define LOCK_NB 4
#endif
#if !defined(LOCK_UN)
#define LOCK_UN 8
#endif

#if defined(__CYGWIN__)
#include <winerror.h>
#endif

static VALUE
rb_thread_flock(void *data)
{
#if defined(__CYGWIN__)
int old_errno = errno;
#endif
int *op = data, ret = flock(op[0], op[1]);

#if defined(__CYGWIN__)
if (GetLastError() == ERROR_NOT_LOCKED) {
ret = 0;
errno = old_errno;
}
#endif
return (VALUE)ret;
}













































static VALUE
rb_file_flock(VALUE obj, VALUE operation)
{
rb_io_t *fptr;
int op[2], op1;
struct timeval time;

op[1] = op1 = NUM2INT(operation);
GetOpenFile(obj, fptr);
op[0] = fptr->fd;

if (fptr->mode & FMODE_WRITABLE) {
rb_io_flush_raw(obj, 0);
}
while ((int)rb_thread_io_blocking_region(rb_thread_flock, op, fptr->fd) < 0) {
int e = errno;
switch (e) {
case EAGAIN:
case EACCES:
#if defined(EWOULDBLOCK) && EWOULDBLOCK != EAGAIN
case EWOULDBLOCK:
#endif
if (op1 & LOCK_NB) return Qfalse;

time.tv_sec = 0;
time.tv_usec = 100 * 1000; 
rb_thread_wait_for(time);
rb_io_check_closed(fptr);
continue;

case EINTR:
#if defined(ERESTART)
case ERESTART:
#endif
break;

default:
rb_syserr_fail_path(e, fptr->pathv);
}
}
return INT2FIX(0);
}

static void
test_check(int n, int argc, VALUE *argv)
{
int i;

n+=1;
rb_check_arity(argc, n, n);
for (i=1; i<n; i++) {
if (!RB_TYPE_P(argv[i], T_FILE)) {
FilePathValue(argv[i]);
}
}
}

#define CHECK(n) test_check((n), argc, argv)



























































static VALUE
rb_f_test(int argc, VALUE *argv, VALUE _)
{
int cmd;

if (argc == 0) rb_check_arity(argc, 2, 3);
cmd = NUM2CHR(argv[0]);
if (cmd == 0) {
unknown:

if (ISPRINT(cmd)) {
rb_raise(rb_eArgError, "unknown command '%s%c'", cmd == '\'' || cmd == '\\' ? "\\" : "", cmd);
}
else {
rb_raise(rb_eArgError, "unknown command \"\\x%02X\"", cmd);
}
}
if (strchr("bcdefgGkloOprRsSuwWxXz", cmd)) {
CHECK(1);
switch (cmd) {
case 'b':
return rb_file_blockdev_p(0, argv[1]);

case 'c':
return rb_file_chardev_p(0, argv[1]);

case 'd':
return rb_file_directory_p(0, argv[1]);

case 'e':
return rb_file_exist_p(0, argv[1]);

case 'f':
return rb_file_file_p(0, argv[1]);

case 'g':
return rb_file_sgid_p(0, argv[1]);

case 'G':
return rb_file_grpowned_p(0, argv[1]);

case 'k':
return rb_file_sticky_p(0, argv[1]);

case 'l':
return rb_file_symlink_p(0, argv[1]);

case 'o':
return rb_file_owned_p(0, argv[1]);

case 'O':
return rb_file_rowned_p(0, argv[1]);

case 'p':
return rb_file_pipe_p(0, argv[1]);

case 'r':
return rb_file_readable_p(0, argv[1]);

case 'R':
return rb_file_readable_real_p(0, argv[1]);

case 's':
return rb_file_size_p(0, argv[1]);

case 'S':
return rb_file_socket_p(0, argv[1]);

case 'u':
return rb_file_suid_p(0, argv[1]);

case 'w':
return rb_file_writable_p(0, argv[1]);

case 'W':
return rb_file_writable_real_p(0, argv[1]);

case 'x':
return rb_file_executable_p(0, argv[1]);

case 'X':
return rb_file_executable_real_p(0, argv[1]);

case 'z':
return rb_file_zero_p(0, argv[1]);
}
}

if (strchr("MAC", cmd)) {
struct stat st;
VALUE fname = argv[1];

CHECK(1);
if (rb_stat(fname, &st) == -1) {
int e = errno;
FilePathValue(fname);
rb_syserr_fail_path(e, fname);
}

switch (cmd) {
case 'A':
return stat_atime(&st);
case 'M':
return stat_mtime(&st);
case 'C':
return stat_ctime(&st);
}
}

if (cmd == '-') {
CHECK(2);
return rb_file_identical_p(0, argv[1], argv[2]);
}

if (strchr("=<>", cmd)) {
struct stat st1, st2;
struct timespec t1, t2;

CHECK(2);
if (rb_stat(argv[1], &st1) < 0) return Qfalse;
if (rb_stat(argv[2], &st2) < 0) return Qfalse;

t1 = stat_mtimespec(&st1);
t2 = stat_mtimespec(&st2);

switch (cmd) {
case '=':
if (t1.tv_sec == t2.tv_sec && t1.tv_nsec == t2.tv_nsec) return Qtrue;
return Qfalse;

case '>':
if (t1.tv_sec > t2.tv_sec) return Qtrue;
if (t1.tv_sec == t2.tv_sec && t1.tv_nsec > t2.tv_nsec) return Qtrue;
return Qfalse;

case '<':
if (t1.tv_sec < t2.tv_sec) return Qtrue;
if (t1.tv_sec == t2.tv_sec && t1.tv_nsec < t2.tv_nsec) return Qtrue;
return Qfalse;
}
}
goto unknown;
}














static VALUE
rb_stat_s_alloc(VALUE klass)
{
return stat_new_0(klass, 0);
}










static VALUE
rb_stat_init(VALUE obj, VALUE fname)
{
struct stat st, *nst;

FilePathValue(fname);
fname = rb_str_encode_ospath(fname);
if (STAT(StringValueCStr(fname), &st) == -1) {
rb_sys_fail_path(fname);
}
if (DATA_PTR(obj)) {
xfree(DATA_PTR(obj));
DATA_PTR(obj) = NULL;
}
nst = ALLOC(struct stat);
*nst = st;
DATA_PTR(obj) = nst;

return Qnil;
}


static VALUE
rb_stat_init_copy(VALUE copy, VALUE orig)
{
struct stat *nst;

if (!OBJ_INIT_COPY(copy, orig)) return copy;
if (DATA_PTR(copy)) {
xfree(DATA_PTR(copy));
DATA_PTR(copy) = 0;
}
if (DATA_PTR(orig)) {
nst = ALLOC(struct stat);
*nst = *(struct stat*)DATA_PTR(orig);
DATA_PTR(copy) = nst;
}

return copy;
}















static VALUE
rb_stat_ftype(VALUE obj)
{
return rb_file_ftype(get_stat(obj));
}












static VALUE
rb_stat_d(VALUE obj)
{
if (S_ISDIR(get_stat(obj)->st_mode)) return Qtrue;
return Qfalse;
}









static VALUE
rb_stat_p(VALUE obj)
{
#if defined(S_IFIFO)
if (S_ISFIFO(get_stat(obj)->st_mode)) return Qtrue;

#endif
return Qfalse;
}

















static VALUE
rb_stat_l(VALUE obj)
{
#if defined(S_ISLNK)
if (S_ISLNK(get_stat(obj)->st_mode)) return Qtrue;
#endif
return Qfalse;
}













static VALUE
rb_stat_S(VALUE obj)
{
#if defined(S_ISSOCK)
if (S_ISSOCK(get_stat(obj)->st_mode)) return Qtrue;

#endif
return Qfalse;
}














static VALUE
rb_stat_b(VALUE obj)
{
#if defined(S_ISBLK)
if (S_ISBLK(get_stat(obj)->st_mode)) return Qtrue;

#endif
return Qfalse;
}













static VALUE
rb_stat_c(VALUE obj)
{
if (S_ISCHR(get_stat(obj)->st_mode)) return Qtrue;

return Qfalse;
}













static VALUE
rb_stat_owned(VALUE obj)
{
if (get_stat(obj)->st_uid == geteuid()) return Qtrue;
return Qfalse;
}

static VALUE
rb_stat_rowned(VALUE obj)
{
if (get_stat(obj)->st_uid == getuid()) return Qtrue;
return Qfalse;
}













static VALUE
rb_stat_grpowned(VALUE obj)
{
#if !defined(_WIN32)
if (rb_group_member(get_stat(obj)->st_gid)) return Qtrue;
#endif
return Qfalse;
}












static VALUE
rb_stat_r(VALUE obj)
{
struct stat *st = get_stat(obj);

#if defined(USE_GETEUID)
if (geteuid() == 0) return Qtrue;
#endif
#if defined(S_IRUSR)
if (rb_stat_owned(obj))
return st->st_mode & S_IRUSR ? Qtrue : Qfalse;
#endif
#if defined(S_IRGRP)
if (rb_stat_grpowned(obj))
return st->st_mode & S_IRGRP ? Qtrue : Qfalse;
#endif
#if defined(S_IROTH)
if (!(st->st_mode & S_IROTH)) return Qfalse;
#endif
return Qtrue;
}












static VALUE
rb_stat_R(VALUE obj)
{
struct stat *st = get_stat(obj);

#if defined(USE_GETEUID)
if (getuid() == 0) return Qtrue;
#endif
#if defined(S_IRUSR)
if (rb_stat_rowned(obj))
return st->st_mode & S_IRUSR ? Qtrue : Qfalse;
#endif
#if defined(S_IRGRP)
if (rb_group_member(get_stat(obj)->st_gid))
return st->st_mode & S_IRGRP ? Qtrue : Qfalse;
#endif
#if defined(S_IROTH)
if (!(st->st_mode & S_IROTH)) return Qfalse;
#endif
return Qtrue;
}














static VALUE
rb_stat_wr(VALUE obj)
{
#if defined(S_IROTH)
struct stat *st = get_stat(obj);
if ((st->st_mode & (S_IROTH)) == S_IROTH) {
return UINT2NUM(st->st_mode & (S_IRUGO|S_IWUGO|S_IXUGO));
}
else {
return Qnil;
}
#endif
}












static VALUE
rb_stat_w(VALUE obj)
{
struct stat *st = get_stat(obj);

#if defined(USE_GETEUID)
if (geteuid() == 0) return Qtrue;
#endif
#if defined(S_IWUSR)
if (rb_stat_owned(obj))
return st->st_mode & S_IWUSR ? Qtrue : Qfalse;
#endif
#if defined(S_IWGRP)
if (rb_stat_grpowned(obj))
return st->st_mode & S_IWGRP ? Qtrue : Qfalse;
#endif
#if defined(S_IWOTH)
if (!(st->st_mode & S_IWOTH)) return Qfalse;
#endif
return Qtrue;
}












static VALUE
rb_stat_W(VALUE obj)
{
struct stat *st = get_stat(obj);

#if defined(USE_GETEUID)
if (getuid() == 0) return Qtrue;
#endif
#if defined(S_IWUSR)
if (rb_stat_rowned(obj))
return st->st_mode & S_IWUSR ? Qtrue : Qfalse;
#endif
#if defined(S_IWGRP)
if (rb_group_member(get_stat(obj)->st_gid))
return st->st_mode & S_IWGRP ? Qtrue : Qfalse;
#endif
#if defined(S_IWOTH)
if (!(st->st_mode & S_IWOTH)) return Qfalse;
#endif
return Qtrue;
}














static VALUE
rb_stat_ww(VALUE obj)
{
#if defined(S_IROTH)
struct stat *st = get_stat(obj);
if ((st->st_mode & (S_IWOTH)) == S_IWOTH) {
return UINT2NUM(st->st_mode & (S_IRUGO|S_IWUGO|S_IXUGO));
}
else {
return Qnil;
}
#endif
}














static VALUE
rb_stat_x(VALUE obj)
{
struct stat *st = get_stat(obj);

#if defined(USE_GETEUID)
if (geteuid() == 0) {
return st->st_mode & S_IXUGO ? Qtrue : Qfalse;
}
#endif
#if defined(S_IXUSR)
if (rb_stat_owned(obj))
return st->st_mode & S_IXUSR ? Qtrue : Qfalse;
#endif
#if defined(S_IXGRP)
if (rb_stat_grpowned(obj))
return st->st_mode & S_IXGRP ? Qtrue : Qfalse;
#endif
#if defined(S_IXOTH)
if (!(st->st_mode & S_IXOTH)) return Qfalse;
#endif
return Qtrue;
}









static VALUE
rb_stat_X(VALUE obj)
{
struct stat *st = get_stat(obj);

#if defined(USE_GETEUID)
if (getuid() == 0) {
return st->st_mode & S_IXUGO ? Qtrue : Qfalse;
}
#endif
#if defined(S_IXUSR)
if (rb_stat_rowned(obj))
return st->st_mode & S_IXUSR ? Qtrue : Qfalse;
#endif
#if defined(S_IXGRP)
if (rb_group_member(get_stat(obj)->st_gid))
return st->st_mode & S_IXGRP ? Qtrue : Qfalse;
#endif
#if defined(S_IXOTH)
if (!(st->st_mode & S_IXOTH)) return Qfalse;
#endif
return Qtrue;
}












static VALUE
rb_stat_f(VALUE obj)
{
if (S_ISREG(get_stat(obj)->st_mode)) return Qtrue;
return Qfalse;
}












static VALUE
rb_stat_z(VALUE obj)
{
if (get_stat(obj)->st_size == 0) return Qtrue;
return Qfalse;
}











static VALUE
rb_stat_s(VALUE obj)
{
off_t size = get_stat(obj)->st_size;

if (size == 0) return Qnil;
return OFFT2NUM(size);
}












static VALUE
rb_stat_suid(VALUE obj)
{
#if defined(S_ISUID)
if (get_stat(obj)->st_mode & S_ISUID) return Qtrue;
#endif
return Qfalse;
}













static VALUE
rb_stat_sgid(VALUE obj)
{
#if defined(S_ISGID)
if (get_stat(obj)->st_mode & S_ISGID) return Qtrue;
#endif
return Qfalse;
}













static VALUE
rb_stat_sticky(VALUE obj)
{
#if defined(S_ISVTX)
if (get_stat(obj)->st_mode & S_ISVTX) return Qtrue;
#endif
return Qfalse;
}

#if !defined HAVE_MKFIFO && defined HAVE_MKNOD && defined S_IFIFO
#define mkfifo(path, mode) mknod(path, (mode)&~S_IFMT|S_IFIFO, 0)
#define HAVE_MKFIFO
#endif

#if defined(HAVE_MKFIFO)
struct mkfifo_arg {
const char *path;
mode_t mode;
};

static void *
nogvl_mkfifo(void *ptr)
{
struct mkfifo_arg *ma = ptr;

return (void *)(VALUE)mkfifo(ma->path, ma->mode);
}











static VALUE
rb_file_s_mkfifo(int argc, VALUE *argv, VALUE _)
{
VALUE path;
struct mkfifo_arg ma;

ma.mode = 0666;
rb_check_arity(argc, 1, 2);
if (argc > 1) {
ma.mode = NUM2MODET(argv[1]);
}
path = argv[0];
FilePathValue(path);
path = rb_str_encode_ospath(path);
ma.path = RSTRING_PTR(path);
if (rb_thread_call_without_gvl(nogvl_mkfifo, &ma, RUBY_UBF_IO, 0)) {
rb_sys_fail_path(path);
}
return INT2FIX(0);
}
#else
#define rb_file_s_mkfifo rb_f_notimplement
#endif

static VALUE rb_mFConst;

void
rb_file_const(const char *name, VALUE value)
{
rb_define_const(rb_mFConst, name, value);
}

int
rb_is_absolute_path(const char *path)
{
#if defined(DOSISH_DRIVE_LETTER)
if (has_drive_letter(path) && isdirsep(path[2])) return 1;
#endif
#if defined(DOSISH_UNC)
if (isdirsep(path[0]) && isdirsep(path[1])) return 1;
#endif
#if !defined(DOSISH)
if (path[0] == '/') return 1;
#endif
return 0;
}

#if !defined(ENABLE_PATH_CHECK)
#if defined DOSISH || defined __CYGWIN__
#define ENABLE_PATH_CHECK 0
#else
#define ENABLE_PATH_CHECK 1
#endif
#endif

#if ENABLE_PATH_CHECK
static int
path_check_0(VALUE path, int execpath)
{
struct stat st;
const char *p0 = StringValueCStr(path);
const char *e0;
rb_encoding *enc;
char *p = 0, *s;

if (!rb_is_absolute_path(p0)) {
char *buf = ruby_getcwd();
VALUE newpath;

newpath = rb_str_new2(buf);
xfree(buf);

rb_str_cat2(newpath, "/");
rb_str_cat2(newpath, p0);
path = newpath;
p0 = RSTRING_PTR(path);
}
e0 = p0 + RSTRING_LEN(path);
enc = rb_enc_get(path);
for (;;) {
#if !defined(S_IWOTH)
#define S_IWOTH 002
#endif
if (STAT(p0, &st) == 0 && S_ISDIR(st.st_mode) && (st.st_mode & S_IWOTH)
#if defined(S_ISVTX)
&& !(p && execpath && (st.st_mode & S_ISVTX))
#endif
&& !access(p0, W_OK)) {
rb_enc_warn(enc, "Insecure world writable dir %s in %sPATH, mode 0%"
PRI_MODET_PREFIX"o",
p0, (execpath ? "" : "LOAD_"), st.st_mode);
if (p) *p = '/';
RB_GC_GUARD(path);
return 0;
}
s = strrdirsep(p0, e0, enc);
if (p) *p = '/';
if (!s || s == p0) return 1;
p = s;
e0 = p;
*p = '\0';
}
}
#endif

#if ENABLE_PATH_CHECK
#define fpath_check(path) path_check_0((path), FALSE)
#else
#define fpath_check(path) 1
#endif

int
rb_path_check(const char *path)
{
#if ENABLE_PATH_CHECK
const char *p0, *p, *pend;
const char sep = PATH_SEP_CHAR;

if (!path) return 1;

pend = path + strlen(path);
p0 = path;
p = strchr(path, sep);
if (!p) p = pend;

for (;;) {
if (!path_check_0(rb_str_new(p0, p - p0), TRUE)) {
return 0; 
}
p0 = p + 1;
if (p0 > pend) break;
p = strchr(p0, sep);
if (!p) p = pend;
}
#endif
return 1;
}

int
ruby_is_fd_loadable(int fd)
{
#if defined(_WIN32)
return 1;
#else
struct stat st;

if (fstat(fd, &st) < 0)
return 0;

if (S_ISREG(st.st_mode))
return 1;

if (S_ISFIFO(st.st_mode) || S_ISCHR(st.st_mode))
return -1;

if (S_ISDIR(st.st_mode))
errno = EISDIR;
else
errno = ENXIO;

return 0;
#endif
}

#if !defined(_WIN32)
int
rb_file_load_ok(const char *path)
{
int ret = 1;




int mode = (O_RDONLY |
#if defined O_NONBLOCK
O_NONBLOCK |
#elif defined O_NDELAY
O_NDELAY |
#endif
0);
int fd = rb_cloexec_open(path, mode, 0);
if (fd == -1) return 0;
rb_update_max_fd(fd);
ret = ruby_is_fd_loadable(fd);
(void)close(fd);
return ret;
}
#endif

static int
is_explicit_relative(const char *path)
{
if (*path++ != '.') return 0;
if (*path == '.') path++;
return isdirsep(*path);
}

static VALUE
copy_path_class(VALUE path, VALUE orig)
{
str_shrink(path);
RBASIC_SET_CLASS(path, rb_obj_class(orig));
OBJ_FREEZE(path);
return path;
}

int
rb_find_file_ext_safe(VALUE *filep, const char *const *ext, int _level)
{
rb_warn("rb_find_file_ext_safe will be removed in Ruby 3.0");
return rb_find_file_ext(filep, ext);
}

int
rb_find_file_ext(VALUE *filep, const char *const *ext)
{
const char *f = StringValueCStr(*filep);
VALUE fname = *filep, load_path, tmp;
long i, j, fnlen;
int expanded = 0;

if (!ext[0]) return 0;

if (f[0] == '~') {
fname = file_expand_path_1(fname);
f = RSTRING_PTR(fname);
*filep = fname;
expanded = 1;
}

if (expanded || rb_is_absolute_path(f) || is_explicit_relative(f)) {
if (!expanded) fname = file_expand_path_1(fname);
fnlen = RSTRING_LEN(fname);
for (i=0; ext[i]; i++) {
rb_str_cat2(fname, ext[i]);
if (rb_file_load_ok(RSTRING_PTR(fname))) {
*filep = copy_path_class(fname, *filep);
return (int)(i+1);
}
rb_str_set_len(fname, fnlen);
}
return 0;
}

RB_GC_GUARD(load_path) = rb_get_expanded_load_path();
if (!load_path) return 0;

fname = rb_str_dup(*filep);
RBASIC_CLEAR_CLASS(fname);
fnlen = RSTRING_LEN(fname);
tmp = rb_str_tmp_new(MAXPATHLEN + 2);
rb_enc_associate_index(tmp, rb_usascii_encindex());
for (j=0; ext[j]; j++) {
rb_str_cat2(fname, ext[j]);
for (i = 0; i < RARRAY_LEN(load_path); i++) {
VALUE str = RARRAY_AREF(load_path, i);

RB_GC_GUARD(str) = rb_get_path(str);
if (RSTRING_LEN(str) == 0) continue;
rb_file_expand_path_internal(fname, str, 0, 0, tmp);
if (rb_file_load_ok(RSTRING_PTR(tmp))) {
*filep = copy_path_class(tmp, *filep);
return (int)(j+1);
}
}
rb_str_set_len(fname, fnlen);
}
rb_str_resize(tmp, 0);
RB_GC_GUARD(load_path);
return 0;
}

VALUE
rb_find_file_safe(VALUE path, int _level)
{
rb_warn("rb_find_file_safe will be removed in Ruby 3.0");
return rb_find_file(path);
}

VALUE
rb_find_file(VALUE path)
{
VALUE tmp, load_path;
const char *f = StringValueCStr(path);
int expanded = 0;

if (f[0] == '~') {
tmp = file_expand_path_1(path);
path = copy_path_class(tmp, path);
f = RSTRING_PTR(path);
expanded = 1;
}

if (expanded || rb_is_absolute_path(f) || is_explicit_relative(f)) {
if (!rb_file_load_ok(f)) return 0;
if (!expanded)
path = copy_path_class(file_expand_path_1(path), path);
return path;
}

RB_GC_GUARD(load_path) = rb_get_expanded_load_path();
if (load_path) {
long i;

tmp = rb_str_tmp_new(MAXPATHLEN + 2);
rb_enc_associate_index(tmp, rb_usascii_encindex());
for (i = 0; i < RARRAY_LEN(load_path); i++) {
VALUE str = RARRAY_AREF(load_path, i);
RB_GC_GUARD(str) = rb_get_path(str);
if (RSTRING_LEN(str) > 0) {
rb_file_expand_path_internal(path, str, 0, 0, tmp);
f = RSTRING_PTR(tmp);
if (rb_file_load_ok(f)) goto found;
}
}
rb_str_resize(tmp, 0);
return 0;
}
else {
return 0; 
}

found:
return copy_path_class(tmp, path);
}

static void
define_filetest_function(const char *name, VALUE (*func)(ANYARGS), int argc)
{
rb_define_module_function(rb_mFileTest, name, func, argc);
rb_define_singleton_method(rb_cFile, name, func, argc);
}

const char ruby_null_device[] =
#if defined DOSISH
"NUL"
#elif defined AMIGA || defined __amigaos__
"NIL"
#elif defined __VMS
"NL:"
#else
"/dev/null"
#endif
;


































void
Init_File(void)
{
VALUE separator;

rb_mFileTest = rb_define_module("FileTest");
rb_cFile = rb_define_class("File", rb_cIO);

define_filetest_function("directory?", rb_file_directory_p, 1);
define_filetest_function("exist?", rb_file_exist_p, 1);
define_filetest_function("exists?", rb_file_exists_p, 1);
define_filetest_function("readable?", rb_file_readable_p, 1);
define_filetest_function("readable_real?", rb_file_readable_real_p, 1);
define_filetest_function("world_readable?", rb_file_world_readable_p, 1);
define_filetest_function("writable?", rb_file_writable_p, 1);
define_filetest_function("writable_real?", rb_file_writable_real_p, 1);
define_filetest_function("world_writable?", rb_file_world_writable_p, 1);
define_filetest_function("executable?", rb_file_executable_p, 1);
define_filetest_function("executable_real?", rb_file_executable_real_p, 1);
define_filetest_function("file?", rb_file_file_p, 1);
define_filetest_function("zero?", rb_file_zero_p, 1);
define_filetest_function("empty?", rb_file_zero_p, 1);
define_filetest_function("size?", rb_file_size_p, 1);
define_filetest_function("size", rb_file_s_size, 1);
define_filetest_function("owned?", rb_file_owned_p, 1);
define_filetest_function("grpowned?", rb_file_grpowned_p, 1);

define_filetest_function("pipe?", rb_file_pipe_p, 1);
define_filetest_function("symlink?", rb_file_symlink_p, 1);
define_filetest_function("socket?", rb_file_socket_p, 1);

define_filetest_function("blockdev?", rb_file_blockdev_p, 1);
define_filetest_function("chardev?", rb_file_chardev_p, 1);

define_filetest_function("setuid?", rb_file_suid_p, 1);
define_filetest_function("setgid?", rb_file_sgid_p, 1);
define_filetest_function("sticky?", rb_file_sticky_p, 1);

define_filetest_function("identical?", rb_file_identical_p, 2);

rb_define_singleton_method(rb_cFile, "stat", rb_file_s_stat, 1);
rb_define_singleton_method(rb_cFile, "lstat", rb_file_s_lstat, 1);
rb_define_singleton_method(rb_cFile, "ftype", rb_file_s_ftype, 1);

rb_define_singleton_method(rb_cFile, "atime", rb_file_s_atime, 1);
rb_define_singleton_method(rb_cFile, "mtime", rb_file_s_mtime, 1);
rb_define_singleton_method(rb_cFile, "ctime", rb_file_s_ctime, 1);
rb_define_singleton_method(rb_cFile, "birthtime", rb_file_s_birthtime, 1);

rb_define_singleton_method(rb_cFile, "utime", rb_file_s_utime, -1);
rb_define_singleton_method(rb_cFile, "chmod", rb_file_s_chmod, -1);
rb_define_singleton_method(rb_cFile, "chown", rb_file_s_chown, -1);
rb_define_singleton_method(rb_cFile, "lchmod", rb_file_s_lchmod, -1);
rb_define_singleton_method(rb_cFile, "lchown", rb_file_s_lchown, -1);
rb_define_singleton_method(rb_cFile, "lutime", rb_file_s_lutime, -1);

rb_define_singleton_method(rb_cFile, "link", rb_file_s_link, 2);
rb_define_singleton_method(rb_cFile, "symlink", rb_file_s_symlink, 2);
rb_define_singleton_method(rb_cFile, "readlink", rb_file_s_readlink, 1);

rb_define_singleton_method(rb_cFile, "unlink", rb_file_s_unlink, -1);
rb_define_singleton_method(rb_cFile, "delete", rb_file_s_unlink, -1);
rb_define_singleton_method(rb_cFile, "rename", rb_file_s_rename, 2);
rb_define_singleton_method(rb_cFile, "umask", rb_file_s_umask, -1);
rb_define_singleton_method(rb_cFile, "truncate", rb_file_s_truncate, 2);
rb_define_singleton_method(rb_cFile, "mkfifo", rb_file_s_mkfifo, -1);
rb_define_singleton_method(rb_cFile, "expand_path", s_expand_path, -1);
rb_define_singleton_method(rb_cFile, "absolute_path", s_absolute_path, -1);
rb_define_singleton_method(rb_cFile, "absolute_path?", s_absolute_path_p, 1);
rb_define_singleton_method(rb_cFile, "realpath", rb_file_s_realpath, -1);
rb_define_singleton_method(rb_cFile, "realdirpath", rb_file_s_realdirpath, -1);
rb_define_singleton_method(rb_cFile, "basename", rb_file_s_basename, -1);
rb_define_singleton_method(rb_cFile, "dirname", rb_file_s_dirname, 1);
rb_define_singleton_method(rb_cFile, "extname", rb_file_s_extname, 1);
rb_define_singleton_method(rb_cFile, "path", rb_file_s_path, 1);

separator = rb_fstring_lit("/");

rb_define_const(rb_cFile, "Separator", separator);

rb_define_const(rb_cFile, "SEPARATOR", separator);
rb_define_singleton_method(rb_cFile, "split", rb_file_s_split, 1);
rb_define_singleton_method(rb_cFile, "join", rb_file_s_join, -2);

#if defined(DOSISH)

rb_define_const(rb_cFile, "ALT_SEPARATOR", rb_obj_freeze(rb_usascii_str_new2(file_alt_separator)));
#else
rb_define_const(rb_cFile, "ALT_SEPARATOR", Qnil);
#endif

rb_define_const(rb_cFile, "PATH_SEPARATOR", rb_fstring_cstr(PATH_SEP));

rb_define_method(rb_cIO, "stat", rb_io_stat, 0); 
rb_define_method(rb_cFile, "lstat", rb_file_lstat, 0);

rb_define_method(rb_cFile, "atime", rb_file_atime, 0);
rb_define_method(rb_cFile, "mtime", rb_file_mtime, 0);
rb_define_method(rb_cFile, "ctime", rb_file_ctime, 0);
rb_define_method(rb_cFile, "birthtime", rb_file_birthtime, 0);
rb_define_method(rb_cFile, "size", rb_file_size, 0);

rb_define_method(rb_cFile, "chmod", rb_file_chmod, 1);
rb_define_method(rb_cFile, "chown", rb_file_chown, 2);
rb_define_method(rb_cFile, "truncate", rb_file_truncate, 1);

rb_define_method(rb_cFile, "flock", rb_file_flock, 1);














rb_mFConst = rb_define_module_under(rb_cFile, "Constants");
rb_include_module(rb_cIO, rb_mFConst);


rb_define_const(rb_mFConst, "RDONLY", INT2FIX(O_RDONLY));

rb_define_const(rb_mFConst, "WRONLY", INT2FIX(O_WRONLY));

rb_define_const(rb_mFConst, "RDWR", INT2FIX(O_RDWR));

rb_define_const(rb_mFConst, "APPEND", INT2FIX(O_APPEND));

rb_define_const(rb_mFConst, "CREAT", INT2FIX(O_CREAT));

rb_define_const(rb_mFConst, "EXCL", INT2FIX(O_EXCL));
#if defined(O_NDELAY) || defined(O_NONBLOCK)
#if !defined(O_NONBLOCK)
#define O_NONBLOCK O_NDELAY
#endif

rb_define_const(rb_mFConst, "NONBLOCK", INT2FIX(O_NONBLOCK));
#endif

rb_define_const(rb_mFConst, "TRUNC", INT2FIX(O_TRUNC));
#if defined(O_NOCTTY)

rb_define_const(rb_mFConst, "NOCTTY", INT2FIX(O_NOCTTY));
#endif
#if !defined(O_BINARY)
#define O_BINARY 0
#endif

rb_define_const(rb_mFConst, "BINARY", INT2FIX(O_BINARY));
#if !defined(O_SHARE_DELETE)
#define O_SHARE_DELETE 0
#endif

rb_define_const(rb_mFConst, "SHARE_DELETE", INT2FIX(O_SHARE_DELETE));
#if defined(O_SYNC)

rb_define_const(rb_mFConst, "SYNC", INT2FIX(O_SYNC));
#endif
#if defined(O_DSYNC)

rb_define_const(rb_mFConst, "DSYNC", INT2FIX(O_DSYNC));
#endif
#if defined(O_RSYNC)

rb_define_const(rb_mFConst, "RSYNC", INT2FIX(O_RSYNC));
#endif
#if defined(O_NOFOLLOW)

rb_define_const(rb_mFConst, "NOFOLLOW", INT2FIX(O_NOFOLLOW)); 
#endif
#if defined(O_NOATIME)

rb_define_const(rb_mFConst, "NOATIME", INT2FIX(O_NOATIME)); 
#endif
#if defined(O_DIRECT)

rb_define_const(rb_mFConst, "DIRECT", INT2FIX(O_DIRECT));
#endif
#if defined(O_TMPFILE)

rb_define_const(rb_mFConst, "TMPFILE", INT2FIX(O_TMPFILE));
#endif


rb_define_const(rb_mFConst, "LOCK_SH", INT2FIX(LOCK_SH));

rb_define_const(rb_mFConst, "LOCK_EX", INT2FIX(LOCK_EX));

rb_define_const(rb_mFConst, "LOCK_UN", INT2FIX(LOCK_UN));

rb_define_const(rb_mFConst, "LOCK_NB", INT2FIX(LOCK_NB));


rb_define_const(rb_mFConst, "NULL", rb_fstring_cstr(ruby_null_device));

rb_define_method(rb_cFile, "path", rb_file_path, 0);
rb_define_method(rb_cFile, "to_path", rb_file_path, 0);
rb_define_global_function("test", rb_f_test, -1);

rb_cStat = rb_define_class_under(rb_cFile, "Stat", rb_cObject);
rb_define_alloc_func(rb_cStat, rb_stat_s_alloc);
rb_define_method(rb_cStat, "initialize", rb_stat_init, 1);
rb_define_method(rb_cStat, "initialize_copy", rb_stat_init_copy, 1);

rb_include_module(rb_cStat, rb_mComparable);

rb_define_method(rb_cStat, "<=>", rb_stat_cmp, 1);

rb_define_method(rb_cStat, "dev", rb_stat_dev, 0);
rb_define_method(rb_cStat, "dev_major", rb_stat_dev_major, 0);
rb_define_method(rb_cStat, "dev_minor", rb_stat_dev_minor, 0);
rb_define_method(rb_cStat, "ino", rb_stat_ino, 0);
rb_define_method(rb_cStat, "mode", rb_stat_mode, 0);
rb_define_method(rb_cStat, "nlink", rb_stat_nlink, 0);
rb_define_method(rb_cStat, "uid", rb_stat_uid, 0);
rb_define_method(rb_cStat, "gid", rb_stat_gid, 0);
rb_define_method(rb_cStat, "rdev", rb_stat_rdev, 0);
rb_define_method(rb_cStat, "rdev_major", rb_stat_rdev_major, 0);
rb_define_method(rb_cStat, "rdev_minor", rb_stat_rdev_minor, 0);
rb_define_method(rb_cStat, "size", rb_stat_size, 0);
rb_define_method(rb_cStat, "blksize", rb_stat_blksize, 0);
rb_define_method(rb_cStat, "blocks", rb_stat_blocks, 0);
rb_define_method(rb_cStat, "atime", rb_stat_atime, 0);
rb_define_method(rb_cStat, "mtime", rb_stat_mtime, 0);
rb_define_method(rb_cStat, "ctime", rb_stat_ctime, 0);
rb_define_method(rb_cStat, "birthtime", rb_stat_birthtime, 0);

rb_define_method(rb_cStat, "inspect", rb_stat_inspect, 0);

rb_define_method(rb_cStat, "ftype", rb_stat_ftype, 0);

rb_define_method(rb_cStat, "directory?", rb_stat_d, 0);
rb_define_method(rb_cStat, "readable?", rb_stat_r, 0);
rb_define_method(rb_cStat, "readable_real?", rb_stat_R, 0);
rb_define_method(rb_cStat, "world_readable?", rb_stat_wr, 0);
rb_define_method(rb_cStat, "writable?", rb_stat_w, 0);
rb_define_method(rb_cStat, "writable_real?", rb_stat_W, 0);
rb_define_method(rb_cStat, "world_writable?", rb_stat_ww, 0);
rb_define_method(rb_cStat, "executable?", rb_stat_x, 0);
rb_define_method(rb_cStat, "executable_real?", rb_stat_X, 0);
rb_define_method(rb_cStat, "file?", rb_stat_f, 0);
rb_define_method(rb_cStat, "zero?", rb_stat_z, 0);
rb_define_method(rb_cStat, "size?", rb_stat_s, 0);
rb_define_method(rb_cStat, "owned?", rb_stat_owned, 0);
rb_define_method(rb_cStat, "grpowned?", rb_stat_grpowned, 0);

rb_define_method(rb_cStat, "pipe?", rb_stat_p, 0);
rb_define_method(rb_cStat, "symlink?", rb_stat_l, 0);
rb_define_method(rb_cStat, "socket?", rb_stat_S, 0);

rb_define_method(rb_cStat, "blockdev?", rb_stat_b, 0);
rb_define_method(rb_cStat, "chardev?", rb_stat_c, 0);

rb_define_method(rb_cStat, "setuid?", rb_stat_suid, 0);
rb_define_method(rb_cStat, "setgid?", rb_stat_sgid, 0);
rb_define_method(rb_cStat, "sticky?", rb_stat_sticky, 0);
}
