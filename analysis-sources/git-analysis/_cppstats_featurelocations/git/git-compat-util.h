#if !defined(GIT_COMPAT_UTIL_H)
#define GIT_COMPAT_UTIL_H

#if defined(USE_MSVC_CRTDBG)




#include <stdlib.h>
#include <crtdbg.h>
#endif

#define _FILE_OFFSET_BITS 64










#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define GIT_GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))

#else
#define GIT_GNUC_PREREQ(maj, min) 0
#endif


#if !defined(FLEX_ARRAY)



#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && (!defined(__SUNPRO_C) || (__SUNPRO_C > 0x580))
#define FLEX_ARRAY 
#elif defined(__GNUC__)
#if (__GNUC__ >= 3)
#define FLEX_ARRAY 
#else
#define FLEX_ARRAY 0 
#endif
#endif




#if !defined(FLEX_ARRAY)
#define FLEX_ARRAY 1
#endif
#endif














#define BUILD_ASSERT_OR_ZERO(cond) (sizeof(char [1 - 2*!(cond)]) - 1)


#if GIT_GNUC_PREREQ(3, 1)

#define BARF_UNLESS_AN_ARRAY(arr) BUILD_ASSERT_OR_ZERO(!__builtin_types_compatible_p(__typeof__(arr), __typeof__(&(arr)[0])))


#else
#define BARF_UNLESS_AN_ARRAY(arr) 0
#endif








#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]) + BARF_UNLESS_AN_ARRAY(x))

#define bitsizeof(x) (CHAR_BIT * sizeof(x))

#define maximum_signed_value_of_type(a) (INTMAX_MAX >> (bitsizeof(intmax_t) - bitsizeof(a)))


#define maximum_unsigned_value_of_type(a) (UINTMAX_MAX >> (bitsizeof(uintmax_t) - bitsizeof(a)))








#define signed_add_overflows(a, b) ((b) > maximum_signed_value_of_type(a) - (a))


#define unsigned_add_overflows(a, b) ((b) > maximum_unsigned_value_of_type(a) - (a))







#define unsigned_mult_overflows(a, b) ((a) && (b) > maximum_unsigned_value_of_type(a) / (a))


#if defined(__GNUC__)
#define TYPEOF(x) (__typeof__(x))
#else
#define TYPEOF(x)
#endif

#define MSB(x, bits) ((x) & TYPEOF(x)(~0ULL << (bitsizeof(x) - (bits))))
#define HAS_MULTI_BITS(i) ((i) & ((i) - 1)) 

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))


#define decimal_length(x) ((int)(sizeof(x) * 2.56 + 0.5) + 1)

#if defined(__sun__)







#if __STDC_VERSION__ - 0 >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif
#elif !defined(__APPLE__) && !defined(__FreeBSD__) && !defined(__USLC__) && !defined(_M_UNIX) && !defined(__sgi) && !defined(__DragonFly__) && !defined(__TANDEM) && !defined(__QNX__) && !defined(__MirBSD__) && !defined(__CYGWIN__)



#define _XOPEN_SOURCE 600 
#define _XOPEN_SOURCE_EXTENDED 1 
#endif
#define _ALL_SOURCE 1
#define _GNU_SOURCE 1
#define _BSD_SOURCE 1
#define _DEFAULT_SOURCE 1
#define _NETBSD_SOURCE 1
#define _SGI_SOURCE 1

#if defined(WIN32) && !defined(__CYGWIN__) 
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN 
#include <winsock2.h>
#include <windows.h>
#define GIT_WINDOWS_NATIVE
#endif

#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#if defined(HAVE_STRINGS_H)
#include <strings.h> 
#endif
#include <errno.h>
#include <limits.h>
#if defined(NEEDS_SYS_PARAM_H)
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <regex.h>
#include <utime.h>
#include <syslog.h>
#if !defined(NO_POLL_H)
#include <poll.h>
#elif !defined(NO_SYS_POLL_H)
#include <sys/poll.h>
#else

#include <poll.h>
#endif
#if defined(HAVE_BSD_SYSCTL)
#include <sys/sysctl.h>
#endif

#if defined(__CYGWIN__)
#include "compat/win32/path-utils.h"
#endif
#if defined(__MINGW32__)

#include "compat/win32/path-utils.h"
#include "compat/mingw.h"
#elif defined(_MSC_VER)
#include "compat/win32/path-utils.h"
#include "compat/msvc.h"
#else
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <termios.h>
#if !defined(NO_SYS_SELECT_H)
#include <sys/select.h>
#endif
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/un.h>
#if !defined(NO_INTTYPES_H)
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#if defined(NO_INTPTR_T)







typedef long intptr_t;
typedef unsigned long uintptr_t;
#endif
#undef _ALL_SOURCE 
#include <grp.h>
#define _ALL_SOURCE 1
#endif


#if defined(PRECOMPOSE_UNICODE)
#include "compat/precompose_utf8.h"
#else
#define precompose_str(in,i_nfd2nfc)
#define precompose_argv(c,v)
#define probe_utf8_pathname_composition()
#endif

#if defined(MKDIR_WO_TRAILING_SLASH)
#define mkdir(a,b) compat_mkdir_wo_trailing_slash((a),(b))
int compat_mkdir_wo_trailing_slash(const char*, mode_t);
#endif

#if defined(NO_STRUCT_ITIMERVAL)
struct itimerval {
struct timeval it_interval;
struct timeval it_value;
};
#endif

#if defined(NO_SETITIMER)
#define setitimer(which,value,ovalue)
#endif

#if !defined(NO_LIBGEN_H)
#include <libgen.h>
#else
#define basename gitbasename
char *gitbasename(char *);
#define dirname gitdirname
char *gitdirname(char *);
#endif

#if !defined(NO_ICONV)
#include <iconv.h>
#endif

#if !defined(NO_OPENSSL)
#if defined(__APPLE__)
#define __AVAILABILITY_MACROS_USES_AVAILABILITY 0
#include <AvailabilityMacros.h>
#undef DEPRECATED_ATTRIBUTE
#define DEPRECATED_ATTRIBUTE
#undef __AVAILABILITY_MACROS_USES_AVAILABILITY
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#if defined(HAVE_SYSINFO)
#include <sys/sysinfo.h>
#endif




#if !defined(NI_MAXHOST)
#define NI_MAXHOST 1025
#endif

#if !defined(NI_MAXSERV)
#define NI_MAXSERV 32
#endif




#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

typedef uintmax_t timestamp_t;
#define PRItime PRIuMAX
#define parse_timestamp strtoumax
#define TIME_MAX UINTMAX_MAX
#define TIME_MIN 0

#if !defined(PATH_SEP)
#define PATH_SEP ':'
#endif

#if defined(HAVE_PATHS_H)
#include <paths.h>
#endif
#if !defined(_PATH_DEFPATH)
#define _PATH_DEFPATH "/usr/local/bin:/usr/bin:/bin"
#endif

#if !defined(platform_core_config)
static inline int noop_core_config(const char *var, const char *value, void *cb)
{
return 0;
}
#define platform_core_config noop_core_config
#endif

#if !defined(has_dos_drive_prefix)
static inline int git_has_dos_drive_prefix(const char *path)
{
return 0;
}
#define has_dos_drive_prefix git_has_dos_drive_prefix
#endif

#if !defined(skip_dos_drive_prefix)
static inline int git_skip_dos_drive_prefix(char **path)
{
return 0;
}
#define skip_dos_drive_prefix git_skip_dos_drive_prefix
#endif

#if !defined(is_dir_sep)
static inline int git_is_dir_sep(int c)
{
return c == '/';
}
#define is_dir_sep git_is_dir_sep
#endif

#if !defined(offset_1st_component)
static inline int git_offset_1st_component(const char *path)
{
return is_dir_sep(path[0]);
}
#define offset_1st_component git_offset_1st_component
#endif

#if !defined(is_valid_path)
#define is_valid_path(path) 1
#endif

#if !defined(find_last_dir_sep)
static inline char *git_find_last_dir_sep(const char *path)
{
return strrchr(path, '/');
}
#define find_last_dir_sep git_find_last_dir_sep
#endif

#if !defined(query_user_email)
#define query_user_email() NULL
#endif

#if defined(__TANDEM)
#include <floss.h(floss_execl,floss_execlp,floss_execv,floss_execvp)>
#include <floss.h(floss_getpwuid)>
#if !defined(NSIG)





#define NSIG 100
#endif
#endif

#if defined(__HP_cc) && (__HP_cc >= 61000)
#define NORETURN __attribute__((noreturn))
#define NORETURN_PTR
#elif defined(__GNUC__) && !defined(NO_NORETURN)
#define NORETURN __attribute__((__noreturn__))
#define NORETURN_PTR __attribute__((__noreturn__))
#elif defined(_MSC_VER)
#define NORETURN __declspec(noreturn)
#define NORETURN_PTR
#else
#define NORETURN
#define NORETURN_PTR
#if !defined(__GNUC__)
#if !defined(__attribute__)
#define __attribute__(x)
#endif
#endif
#endif


#if defined(__GNUC__) && (__GNUC__ >= 4)
#define LAST_ARG_MUST_BE_NULL __attribute__((sentinel))
#else
#define LAST_ARG_MUST_BE_NULL
#endif

#define MAYBE_UNUSED __attribute__((__unused__))

#include "compat/bswap.h"

#include "wildmatch.h"

struct strbuf;


void vreportf(const char *prefix, const char *err, va_list params);
NORETURN void usage(const char *err);
NORETURN void usagef(const char *err, ...) __attribute__((format (printf, 1, 2)));
NORETURN void die(const char *err, ...) __attribute__((format (printf, 1, 2)));
NORETURN void die_errno(const char *err, ...) __attribute__((format (printf, 1, 2)));
int error(const char *err, ...) __attribute__((format (printf, 1, 2)));
int error_errno(const char *err, ...) __attribute__((format (printf, 1, 2)));
void warning(const char *err, ...) __attribute__((format (printf, 1, 2)));
void warning_errno(const char *err, ...) __attribute__((format (printf, 1, 2)));

#if !defined(NO_OPENSSL)
#if defined(APPLE_COMMON_CRYPTO)
#include "compat/apple-common-crypto.h"
#else
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif 
#include <openssl/x509v3.h>
#endif 








#if defined(__GNUC__)
static inline int const_error(void)
{
return -1;
}
#define error(...) (error(__VA_ARGS__), const_error())
#define error_errno(...) (error_errno(__VA_ARGS__), const_error())
#endif

void set_die_routine(NORETURN_PTR void (*routine)(const char *err, va_list params));
void set_error_routine(void (*routine)(const char *err, va_list params));
extern void (*get_error_routine(void))(const char *err, va_list params);
void set_warn_routine(void (*routine)(const char *warn, va_list params));
extern void (*get_warn_routine(void))(const char *warn, va_list params);
void set_die_is_recursing_routine(int (*routine)(void));

int starts_with(const char *str, const char *prefix);
int istarts_with(const char *str, const char *prefix);

















static inline int skip_prefix(const char *str, const char *prefix,
const char **out)
{
do {
if (!*prefix) {
*out = str;
return 1;
}
} while (*str++ == *prefix++);
return 0;
}















int skip_to_optional_arg_default(const char *str, const char *prefix,
const char **arg, const char *def);

static inline int skip_to_optional_arg(const char *str, const char *prefix,
const char **arg)
{
return skip_to_optional_arg_default(str, prefix, arg, "");
}





static inline int skip_prefix_mem(const char *buf, size_t len,
const char *prefix,
const char **out, size_t *outlen)
{
size_t prefix_len = strlen(prefix);
if (prefix_len <= len && !memcmp(buf, prefix, prefix_len)) {
*out = buf + prefix_len;
*outlen = len - prefix_len;
return 1;
}
return 0;
}





static inline int strip_suffix_mem(const char *buf, size_t *len,
const char *suffix)
{
size_t suflen = strlen(suffix);
if (*len < suflen || memcmp(buf + (*len - suflen), suffix, suflen))
return 0;
*len -= suflen;
return 1;
}








static inline int strip_suffix(const char *str, const char *suffix, size_t *len)
{
*len = strlen(str);
return strip_suffix_mem(str, len, suffix);
}

static inline int ends_with(const char *str, const char *suffix)
{
size_t len;
return strip_suffix(str, suffix, &len);
}

#define SWAP(a, b) do { void *_swap_a_ptr = &(a); void *_swap_b_ptr = &(b); unsigned char _swap_buffer[sizeof(a)]; memcpy(_swap_buffer, _swap_a_ptr, sizeof(a)); memcpy(_swap_a_ptr, _swap_b_ptr, sizeof(a) + BUILD_ASSERT_OR_ZERO(sizeof(a) == sizeof(b))); memcpy(_swap_b_ptr, _swap_buffer, sizeof(a)); } while (0)









#if defined(NO_MMAP) || defined(USE_WIN32_MMAP)

#if !defined(PROT_READ)
#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_PRIVATE 1
#endif

#define mmap git_mmap
#define munmap git_munmap
void *git_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
int git_munmap(void *start, size_t length);

#else 

#include <sys/mman.h>

#endif 

#if defined(NO_MMAP)


#define DEFAULT_PACKED_GIT_WINDOW_SIZE (1 * 1024 * 1024)

#else 


#define DEFAULT_PACKED_GIT_WINDOW_SIZE (sizeof(void*) >= 8 ? 1 * 1024 * 1024 * 1024 : 32 * 1024 * 1024)




#endif 

#if !defined(MAP_FAILED)
#define MAP_FAILED ((void *)-1)
#endif

#if defined(NO_ST_BLOCKS_IN_STRUCT_STAT)
#define on_disk_bytes(st) ((st).st_size)
#else
#define on_disk_bytes(st) ((st).st_blocks * 512)
#endif

#if defined(NEEDS_MODE_TRANSLATION)
#undef S_IFMT
#undef S_IFREG
#undef S_IFDIR
#undef S_IFLNK
#undef S_IFBLK
#undef S_IFCHR
#undef S_IFIFO
#undef S_IFSOCK
#define S_IFMT 0170000
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFLNK 0120000
#define S_IFBLK 0060000
#define S_IFCHR 0020000
#define S_IFIFO 0010000
#define S_IFSOCK 0140000
#if defined(stat)
#undef stat
#endif
#define stat(path, buf) git_stat(path, buf)
int git_stat(const char *, struct stat *);
#if defined(fstat)
#undef fstat
#endif
#define fstat(fd, buf) git_fstat(fd, buf)
int git_fstat(int, struct stat *);
#if defined(lstat)
#undef lstat
#endif
#define lstat(path, buf) git_lstat(path, buf)
int git_lstat(const char *, struct stat *);
#endif

#define DEFAULT_PACKED_GIT_LIMIT ((1024L * 1024L) * (size_t)(sizeof(void*) >= 8 ? (32 * 1024L * 1024L) : 256))


#if defined(NO_PREAD)
#define pread git_pread
ssize_t git_pread(int fd, void *buf, size_t count, off_t offset);
#endif





ssize_t read_in_full(int fd, void *buf, size_t count);

#if defined(NO_SETENV)
#define setenv gitsetenv
int gitsetenv(const char *, const char *, int);
#endif

#if defined(NO_MKDTEMP)
#define mkdtemp gitmkdtemp
char *gitmkdtemp(char *);
#endif

#if defined(NO_UNSETENV)
#define unsetenv gitunsetenv
void gitunsetenv(const char *);
#endif

#if defined(NO_STRCASESTR)
#define strcasestr gitstrcasestr
char *gitstrcasestr(const char *haystack, const char *needle);
#endif

#if defined(NO_STRLCPY)
#define strlcpy gitstrlcpy
size_t gitstrlcpy(char *, const char *, size_t);
#endif

#if defined(NO_STRTOUMAX)
#define strtoumax gitstrtoumax
uintmax_t gitstrtoumax(const char *, char **, int);
#define strtoimax gitstrtoimax
intmax_t gitstrtoimax(const char *, char **, int);
#endif

#if defined(NO_HSTRERROR)
#define hstrerror githstrerror
const char *githstrerror(int herror);
#endif

#if defined(NO_MEMMEM)
#define memmem gitmemmem
void *gitmemmem(const void *haystack, size_t haystacklen,
const void *needle, size_t needlelen);
#endif

#if defined(OVERRIDE_STRDUP)
#if defined(strdup)
#undef strdup
#endif
#define strdup gitstrdup
char *gitstrdup(const char *s);
#endif

#if defined(NO_GETPAGESIZE)
#define getpagesize() sysconf(_SC_PAGESIZE)
#endif

#if !defined(O_CLOEXEC)
#define O_CLOEXEC 0
#endif

#if defined(FREAD_READS_DIRECTORIES)
#if !defined(SUPPRESS_FOPEN_REDEFINITION)
#if defined(fopen)
#undef fopen
#endif
#define fopen(a,b) git_fopen(a,b)
#endif
FILE *git_fopen(const char*, const char*);
#endif

#if defined(SNPRINTF_RETURNS_BOGUS)
#if defined(snprintf)
#undef snprintf
#endif
#define snprintf git_snprintf
int git_snprintf(char *str, size_t maxsize,
const char *format, ...);
#if defined(vsnprintf)
#undef vsnprintf
#endif
#define vsnprintf git_vsnprintf
int git_vsnprintf(char *str, size_t maxsize,
const char *format, va_list ap);
#endif

#if defined(__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2, 1)
#define HAVE_STRCHRNUL
#endif
#endif

#if !defined(HAVE_STRCHRNUL)
#define strchrnul gitstrchrnul
static inline char *gitstrchrnul(const char *s, int c)
{
while (*s && *s != c)
s++;
return (char *)s;
}
#endif

#if defined(NO_INET_PTON)
int inet_pton(int af, const char *src, void *dst);
#endif

#if defined(NO_INET_NTOP)
const char *inet_ntop(int af, const void *src, char *dst, size_t size);
#endif

#if defined(NO_PTHREADS)
#define atexit git_atexit
int git_atexit(void (*handler)(void));
#endif

static inline size_t st_add(size_t a, size_t b)
{
if (unsigned_add_overflows(a, b))
die("size_t overflow: %"PRIuMAX" + %"PRIuMAX,
(uintmax_t)a, (uintmax_t)b);
return a + b;
}
#define st_add3(a,b,c) st_add(st_add((a),(b)),(c))
#define st_add4(a,b,c,d) st_add(st_add3((a),(b),(c)),(d))

static inline size_t st_mult(size_t a, size_t b)
{
if (unsigned_mult_overflows(a, b))
die("size_t overflow: %"PRIuMAX" * %"PRIuMAX,
(uintmax_t)a, (uintmax_t)b);
return a * b;
}

static inline size_t st_sub(size_t a, size_t b)
{
if (a < b)
die("size_t underflow: %"PRIuMAX" - %"PRIuMAX,
(uintmax_t)a, (uintmax_t)b);
return a - b;
}

#if defined(HAVE_ALLOCA_H)
#include <alloca.h>
#define xalloca(size) (alloca(size))
#define xalloca_free(p) do {} while (0)
#else
#define xalloca(size) (xmalloc(size))
#define xalloca_free(p) (free(p))
#endif
char *xstrdup(const char *str);
void *xmalloc(size_t size);
void *xmallocz(size_t size);
void *xmallocz_gently(size_t size);
void *xmemdupz(const void *data, size_t len);
char *xstrndup(const char *str, size_t len);
void *xrealloc(void *ptr, size_t size);
void *xcalloc(size_t nmemb, size_t size);
void *xmmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
void *xmmap_gently(void *start, size_t length, int prot, int flags, int fd, off_t offset);
int xopen(const char *path, int flags, ...);
ssize_t xread(int fd, void *buf, size_t len);
ssize_t xwrite(int fd, const void *buf, size_t len);
ssize_t xpread(int fd, void *buf, size_t len, off_t offset);
int xdup(int fd);
FILE *xfopen(const char *path, const char *mode);
FILE *xfdopen(int fd, const char *mode);
int xmkstemp(char *temp_filename);
int xmkstemp_mode(char *temp_filename, int mode);
char *xgetcwd(void);
FILE *fopen_for_writing(const char *path);
FILE *fopen_or_warn(const char *path, const char *mode);





#define FREE_AND_NULL(p) do { free(p); (p) = NULL; } while (0)

#define ALLOC_ARRAY(x, alloc) (x) = xmalloc(st_mult(sizeof(*(x)), (alloc)))
#define CALLOC_ARRAY(x, alloc) (x) = xcalloc((alloc), sizeof(*(x)));
#define REALLOC_ARRAY(x, alloc) (x) = xrealloc((x), st_mult(sizeof(*(x)), (alloc)))

#define COPY_ARRAY(dst, src, n) copy_array((dst), (src), (n), sizeof(*(dst)) + BUILD_ASSERT_OR_ZERO(sizeof(*(dst)) == sizeof(*(src))))

static inline void copy_array(void *dst, const void *src, size_t n, size_t size)
{
if (n)
memcpy(dst, src, st_mult(size, n));
}

#define MOVE_ARRAY(dst, src, n) move_array((dst), (src), (n), sizeof(*(dst)) + BUILD_ASSERT_OR_ZERO(sizeof(*(dst)) == sizeof(*(src))))

static inline void move_array(void *dst, const void *src, size_t n, size_t size)
{
if (n)
memmove(dst, src, st_mult(size, n));
}










































#define FLEX_ALLOC_MEM(x, flexname, buf, len) do { size_t flex_array_len_ = (len); (x) = xcalloc(1, st_add3(sizeof(*(x)), flex_array_len_, 1)); memcpy((void *)(x)->flexname, (buf), flex_array_len_); } while (0)




#define FLEXPTR_ALLOC_MEM(x, ptrname, buf, len) do { size_t flex_array_len_ = (len); (x) = xcalloc(1, st_add3(sizeof(*(x)), flex_array_len_, 1)); memcpy((x) + 1, (buf), flex_array_len_); (x)->ptrname = (void *)((x)+1); } while(0)





#define FLEX_ALLOC_STR(x, flexname, str) FLEX_ALLOC_MEM((x), flexname, (str), strlen(str))

#define FLEXPTR_ALLOC_STR(x, ptrname, str) FLEXPTR_ALLOC_MEM((x), ptrname, (str), strlen(str))


static inline char *xstrdup_or_null(const char *str)
{
return str ? xstrdup(str) : NULL;
}

static inline size_t xsize_t(off_t len)
{
size_t size = (size_t) len;

if (len != (off_t) size)
die("Cannot handle files this big");
return size;
}

__attribute__((format (printf, 3, 4)))
int xsnprintf(char *dst, size_t max, const char *fmt, ...);

#if !defined(HOST_NAME_MAX)
#define HOST_NAME_MAX 256
#endif

int xgethostname(char *buf, size_t len);


extern const unsigned char tolower_trans_tbl[256];


#undef isascii
#undef isspace
#undef isdigit
#undef isalpha
#undef isalnum
#undef isprint
#undef islower
#undef isupper
#undef tolower
#undef toupper
#undef iscntrl
#undef ispunct
#undef isxdigit

extern const unsigned char sane_ctype[256];
#define GIT_SPACE 0x01
#define GIT_DIGIT 0x02
#define GIT_ALPHA 0x04
#define GIT_GLOB_SPECIAL 0x08
#define GIT_REGEX_SPECIAL 0x10
#define GIT_PATHSPEC_MAGIC 0x20
#define GIT_CNTRL 0x40
#define GIT_PUNCT 0x80
#define sane_istest(x,mask) ((sane_ctype[(unsigned char)(x)] & (mask)) != 0)
#define isascii(x) (((x) & ~0x7f) == 0)
#define isspace(x) sane_istest(x,GIT_SPACE)
#define isdigit(x) sane_istest(x,GIT_DIGIT)
#define isalpha(x) sane_istest(x,GIT_ALPHA)
#define isalnum(x) sane_istest(x,GIT_ALPHA | GIT_DIGIT)
#define isprint(x) ((x) >= 0x20 && (x) <= 0x7e)
#define islower(x) sane_iscase(x, 1)
#define isupper(x) sane_iscase(x, 0)
#define is_glob_special(x) sane_istest(x,GIT_GLOB_SPECIAL)
#define is_regex_special(x) sane_istest(x,GIT_GLOB_SPECIAL | GIT_REGEX_SPECIAL)
#define iscntrl(x) (sane_istest(x,GIT_CNTRL))
#define ispunct(x) sane_istest(x, GIT_PUNCT | GIT_REGEX_SPECIAL | GIT_GLOB_SPECIAL | GIT_PATHSPEC_MAGIC)

#define isxdigit(x) (hexval_table[(unsigned char)(x)] != -1)
#define tolower(x) sane_case((unsigned char)(x), 0x20)
#define toupper(x) sane_case((unsigned char)(x), 0)
#define is_pathspec_magic(x) sane_istest(x,GIT_PATHSPEC_MAGIC)

static inline int sane_case(int x, int high)
{
if (sane_istest(x, GIT_ALPHA))
x = (x & ~0x20) | high;
return x;
}

static inline int sane_iscase(int x, int is_lower)
{
if (!sane_istest(x, GIT_ALPHA))
return 0;

if (is_lower)
return (x & 0x20) != 0;
else
return (x & 0x20) == 0;
}






static inline int skip_iprefix(const char *str, const char *prefix,
const char **out)
{
do {
if (!*prefix) {
*out = str;
return 1;
}
} while (tolower(*str++) == tolower(*prefix++));
return 0;
}

static inline int strtoul_ui(char const *s, int base, unsigned int *result)
{
unsigned long ul;
char *p;

errno = 0;

if (strchr(s, '-'))
return -1;
ul = strtoul(s, &p, base);
if (errno || *p || p == s || (unsigned int) ul != ul)
return -1;
*result = ul;
return 0;
}

static inline int strtol_i(char const *s, int base, int *result)
{
long ul;
char *p;

errno = 0;
ul = strtol(s, &p, base);
if (errno || *p || p == s || (int) ul != ul)
return -1;
*result = ul;
return 0;
}

void git_stable_qsort(void *base, size_t nmemb, size_t size,
int(*compar)(const void *, const void *));
#if defined(INTERNAL_QSORT)
#define qsort git_stable_qsort
#endif

#define QSORT(base, n, compar) sane_qsort((base), (n), sizeof(*(base)), compar)
static inline void sane_qsort(void *base, size_t nmemb, size_t size,
int(*compar)(const void *, const void *))
{
if (nmemb > 1)
qsort(base, nmemb, size, compar);
}

#define STABLE_QSORT(base, n, compar) git_stable_qsort((base), (n), sizeof(*(base)), compar)


#if !defined(HAVE_ISO_QSORT_S)
int git_qsort_s(void *base, size_t nmemb, size_t size,
int (*compar)(const void *, const void *, void *), void *ctx);
#define qsort_s git_qsort_s
#endif

#define QSORT_S(base, n, compar, ctx) do { if (qsort_s((base), (n), sizeof(*(base)), compar, ctx)) BUG("qsort_s() failed"); } while (0)




#if !defined(REG_STARTEND)
#error "Git requires REG_STARTEND support. Compile with NO_REGEX=NeedsStartEnd"
#endif

static inline int regexec_buf(const regex_t *preg, const char *buf, size_t size,
size_t nmatch, regmatch_t pmatch[], int eflags)
{
assert(nmatch > 0 && pmatch);
pmatch[0].rm_so = 0;
pmatch[0].rm_eo = size;
return regexec(preg, buf, nmatch, pmatch, eflags | REG_STARTEND);
}

#if !defined(DIR_HAS_BSD_GROUP_SEMANTICS)
#define FORCE_DIR_SET_GID S_ISGID
#else
#define FORCE_DIR_SET_GID 0
#endif

#if defined(NO_NSEC)
#undef USE_NSEC
#define ST_CTIME_NSEC(st) 0
#define ST_MTIME_NSEC(st) 0
#else
#if defined(USE_ST_TIMESPEC)
#define ST_CTIME_NSEC(st) ((unsigned int)((st).st_ctimespec.tv_nsec))
#define ST_MTIME_NSEC(st) ((unsigned int)((st).st_mtimespec.tv_nsec))
#else
#define ST_CTIME_NSEC(st) ((unsigned int)((st).st_ctim.tv_nsec))
#define ST_MTIME_NSEC(st) ((unsigned int)((st).st_mtim.tv_nsec))
#endif
#endif

#if defined(UNRELIABLE_FSTAT)
#define fstat_is_reliable() 0
#else
#define fstat_is_reliable() 1
#endif

#if !defined(va_copy)





#if defined(__va_copy)
#define va_copy(dst, src) __va_copy(dst, src)
#else
#define va_copy(dst, src) ((dst) = (src))
#endif
#endif

#if defined(__GNUC__) || (_MSC_VER >= 1400) || defined(__C99_MACRO_WITH_VA_ARGS)
#define HAVE_VARIADIC_MACROS 1
#endif


extern int BUG_exit_code;

#if defined(HAVE_VARIADIC_MACROS)
__attribute__((format (printf, 3, 4))) NORETURN
void BUG_fl(const char *file, int line, const char *fmt, ...);
#define BUG(...) BUG_fl(__FILE__, __LINE__, __VA_ARGS__)
#else
__attribute__((format (printf, 1, 2))) NORETURN
void BUG(const char *fmt, ...);
#endif






int unlink_or_warn(const char *path);






int unlink_or_msg(const char *file, struct strbuf *err);





int rmdir_or_warn(const char *path);




int remove_or_warn(unsigned int mode, const char *path);





#define ACCESS_EACCES_OK (1U << 0)
int access_or_warn(const char *path, int mode, unsigned flag);
int access_or_die(const char *path, int mode, unsigned flag);


int warn_on_fopen_errors(const char *path);

#if defined(GMTIME_UNRELIABLE_ERRORS)
struct tm *git_gmtime(const time_t *);
struct tm *git_gmtime_r(const time_t *, struct tm *);
#define gmtime git_gmtime
#define gmtime_r git_gmtime_r
#endif

#if !defined(USE_PARENS_AROUND_GETTEXT_N) && defined(__GNUC__)
#define USE_PARENS_AROUND_GETTEXT_N 1
#endif

#if !defined(SHELL_PATH)
#define SHELL_PATH "/bin/sh"
#endif

#if !defined(_POSIX_THREAD_SAFE_FUNCTIONS)
#define flockfile(fh)
#define funlockfile(fh)
#define getc_unlocked(fh) getc(fh)
#endif

#if defined(FILENO_IS_A_MACRO)
int git_fileno(FILE *stream);
#if !defined(COMPAT_CODE_FILENO)
#undef fileno
#define fileno(p) git_fileno(p)
#endif
#endif

#if defined(NEED_ACCESS_ROOT_HANDLER)
int git_access(const char *path, int mode);
#if !defined(COMPAT_CODE_ACCESS)
#if defined(access)
#undef access
#endif
#define access(path, mode) git_access(path, mode)
#endif
#endif











static inline int is_missing_file_error(int errno_)
{
return (errno_ == ENOENT || errno_ == ENOTDIR);
}

int cmd_main(int, const char **);





int trace2_cmd_exit_fl(const char *file, int line, int code);
#define exit(code) exit(trace2_cmd_exit_fl(__FILE__, __LINE__, (code)))














#if defined(SUPPRESS_ANNOTATED_LEAKS)
void unleak_memory(const void *ptr, size_t len);
#define UNLEAK(var) unleak_memory(&(var), sizeof(var))
#else
#define UNLEAK(var) do {} while (0)
#endif





#include "banned.h"








#define container_of(ptr, type, member) ((type *) ((char *)(ptr) - offsetof(type, member)))






static inline void *container_of_or_null_offset(void *ptr, size_t offset)
{
return ptr ? (char *)ptr - offset : NULL;
}




#define container_of_or_null(ptr, type, member) (type *)container_of_or_null_offset(ptr, offsetof(type, member))








#if defined(__GNUC__) 
#define OFFSETOF_VAR(ptr, member) offsetof(__typeof__(*ptr), member)
#else 
#define OFFSETOF_VAR(ptr, member) ((uintptr_t)&(ptr)->member - (uintptr_t)(ptr))

#endif 

#endif
