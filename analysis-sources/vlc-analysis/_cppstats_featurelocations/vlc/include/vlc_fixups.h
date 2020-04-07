
























#if !defined(LIBVLC_FIXUPS_H)
#define LIBVLC_FIXUPS_H 1


#if defined(HAVE_FEATURES_H)
#include <features.h>
#endif



#if defined (__cplusplus) && (defined(__MINGW32__) || defined(__UCLIBC__) || defined(__native_client__))
#if !defined(__STDC_FORMAT_MACROS)
#define __STDC_FORMAT_MACROS 1
#endif
#if !defined(__STDC_CONSTANT_MACROS)
#define __STDC_CONSTANT_MACROS 1
#endif
#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS 1
#endif
#endif

#if !defined(__cplusplus)
#if defined(HAVE_THREADS_H)
#include <threads.h>
#elif !defined(thread_local)
#if defined(HAVE_THREAD_LOCAL)
#define thread_local _Thread_local
#elif defined(_MSC_VER)
#define thread_local __declspec(thread)
#endif
#endif
#endif

#if !defined (HAVE_GMTIME_R) || !defined (HAVE_LOCALTIME_R) || !defined (HAVE_TIMEGM)

#include <time.h> 
#endif

#if !defined(HAVE_GETTIMEOFDAY)
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <sys/time.h>
#endif
#endif

#if !defined(HAVE_LLDIV)
typedef struct
{
long long quot; 
long long rem; 
} lldiv_t;
#endif

#if !defined(HAVE_GETENV) || !defined(HAVE_USELOCALE)

#include <stddef.h> 
#endif

#if !defined (HAVE_REWIND) || !defined (HAVE_GETDELIM)

#include <stdio.h> 
#endif

#if !defined (HAVE_ALIGNED_ALLOC) || !defined (HAVE_MEMRCHR) || !defined (HAVE_POSIX_MEMALIGN) || !defined (HAVE_QSORT_R) || !defined (HAVE_STRLCPY) || !defined (HAVE_STRNDUP) || !defined (HAVE_STRNLEN) || !defined (HAVE_STRNSTR)







#include <stddef.h> 
#endif

#if !defined(HAVE_VASPRINTF)
#include <stdarg.h> 
#endif

#if !defined (HAVE_GETDELIM) || !defined (HAVE_GETPID) || !defined (HAVE_SWAB)


#include <sys/types.h> 
#endif

#if !defined (HAVE_DIRFD) || !defined (HAVE_FDOPENDIR)

#include <dirent.h>
#endif

#if defined(__cplusplus)
#define VLC_NOTHROW throw ()
extern "C" {
#else
#define VLC_NOTHROW
#endif


#if !defined(HAVE_SIGWAIT) && defined(__native_client__)



#include <signal.h>
int sigwait(const sigset_t *set, int *sig);
#endif


#if !defined (__cplusplus) && !defined (HAVE_MAX_ALIGN_T)
typedef struct {
long long ll;
long double ld;
} max_align_t;
#endif


#if !defined(HAVE_ASPRINTF)
int asprintf (char **, const char *, ...);
#endif

#if !defined(HAVE_FLOCKFILE)
void flockfile (FILE *);
void funlockfile (FILE *);
int getc_unlocked (FILE *);
int getchar_unlocked (void);
int putc_unlocked (int, FILE *);
int putchar_unlocked (int);
#endif

#if !defined(HAVE_GETDELIM)
ssize_t getdelim (char **, size_t *, int, FILE *);
ssize_t getline (char **, size_t *, FILE *);
#endif

#if !defined(HAVE_REWIND)
void rewind (FILE *);
#endif

#if !defined(HAVE_VASPRINTF)
int vasprintf (char **, const char *, va_list);
#endif


#if !defined(HAVE_MEMRCHR)
void *memrchr(const void *, int, size_t);
#endif

#if !defined(HAVE_STRCASECMP)
int strcasecmp (const char *, const char *);
#endif

#if !defined(HAVE_STRCASESTR)
char *strcasestr (const char *, const char *);
#endif

#if !defined(HAVE_STRDUP)
char *strdup (const char *);
#endif

#if !defined(HAVE_STRVERSCMP)
int strverscmp (const char *, const char *);
#endif

#if !defined(HAVE_STRNLEN)
size_t strnlen (const char *, size_t);
#endif

#if !defined(HAVE_STRNSTR)
char * strnstr (const char *, const char *, size_t);
#endif

#if !defined(HAVE_STRNDUP)
char *strndup (const char *, size_t);
#endif

#if !defined(HAVE_STRLCPY)
size_t strlcpy (char *, const char *, size_t);
#endif

#if !defined(HAVE_STRSEP)
char *strsep (char **, const char *);
#endif

#if !defined(HAVE_STRTOK_R)
char *strtok_r(char *, const char *, char **);
#endif


#if !defined(HAVE_ATOF)
#if !defined(__ANDROID__)
double atof (const char *);
#endif
#endif

#if !defined(HAVE_ATOLL)
long long atoll (const char *);
#endif

#if !defined(HAVE_LLDIV)
lldiv_t lldiv (long long, long long);
#endif

#if !defined(HAVE_STRTOF)
#if !defined(__ANDROID__)
float strtof (const char *, char **);
#endif
#endif

#if !defined(HAVE_STRTOLL)
long long int strtoll (const char *, char **, int);
#endif


#if !defined(HAVE_GMTIME_R)
struct tm *gmtime_r (const time_t *, struct tm *);
#endif

#if !defined(HAVE_LOCALTIME_R)
struct tm *localtime_r (const time_t *, struct tm *);
#endif

#if !defined(HAVE_TIMEGM)
time_t timegm(struct tm *);
#endif

#if !defined(HAVE_TIMESPEC_GET)
#define TIME_UTC 1
struct timespec;
int timespec_get(struct timespec *, int);
#endif


#if !defined(HAVE_GETTIMEOFDAY)
struct timezone;
int gettimeofday(struct timeval *, struct timezone *);
#endif


#if !defined(HAVE_GETPID)
pid_t getpid (void) VLC_NOTHROW;
#endif

#if !defined(HAVE_FSYNC)
int fsync (int fd);
#endif

#if !defined(HAVE_PATHCONF)
long pathconf (const char *path, int name);
#endif


#if !defined(HAVE_DIRFD)
int (dirfd) (DIR *);
#endif

#if !defined(HAVE_FDOPENDIR)
DIR *fdopendir (int);
#endif

#if defined(__cplusplus)
} 
#endif


#if !defined(HAVE_GETENV)
static inline char *getenv (const char *name)
{
(void)name;
return NULL;
}
#endif

#if !defined(HAVE_SETENV)
int setenv (const char *, const char *, int);
int unsetenv (const char *);
#endif

#if !defined(HAVE_POSIX_MEMALIGN)
int posix_memalign(void **, size_t, size_t);
#endif

#if !defined(HAVE_ALIGNED_ALLOC)
void *aligned_alloc(size_t, size_t);
#endif

#if defined (_WIN32) && defined(__MINGW32__)
#define aligned_free(ptr) __mingw_aligned_free(ptr)
#elif defined (_WIN32) && defined(_MSC_VER)
#define aligned_free(ptr) _aligned_free(ptr)
#else
#define aligned_free(ptr) free(ptr)
#endif

#if defined(__native_client__) && defined(__cplusplus)
#define HAVE_USELOCALE
#endif

#if !defined(HAVE_NEWLOCALE) && defined(HAVE_CXX_LOCALE_T) && defined(__cplusplus)
#include <locale>
#define HAVE_NEWLOCALE
#endif


#if !defined(HAVE_USELOCALE)
#if !defined(HAVE_NEWLOCALE)
#define LC_ALL_MASK 0
#define LC_NUMERIC_MASK 0
#define LC_MESSAGES_MASK 0
#define LC_GLOBAL_LOCALE ((locale_t)(uintptr_t)1)
typedef void *locale_t;

static inline void freelocale(locale_t loc)
{
(void)loc;
}
static inline locale_t newlocale(int mask, const char * locale, locale_t base)
{
(void)mask; (void)locale; (void)base;
return NULL;
}
#else
#include <locale.h>
#endif

static inline locale_t uselocale(locale_t loc)
{
(void)loc;
return NULL;
}
#endif

#if !defined (HAVE_STATIC_ASSERT) && !defined(__cpp_static_assert)
#define STATIC_ASSERT_CONCAT_(a, b) a##b
#define STATIC_ASSERT_CONCAT(a, b) STATIC_ASSERT_CONCAT_(a, b)
#define _Static_assert(x, s) extern char STATIC_ASSERT_CONCAT(static_assert_, __LINE__)[sizeof(struct { unsigned:-!(x); })]
#define static_assert _Static_assert
#endif


#define _(str) vlc_gettext (str)
#define N_(str) gettext_noop (str)
#define gettext_noop(str) (str)

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(HAVE_SWAB)
void swab (const void *, void *, ssize_t);
#endif


#if !defined(HAVE_INET_PTON)
#if !defined(_WIN32)
#include <sys/socket.h>
#else
typedef int socklen_t;
#endif
int inet_pton(int, const char *, void *);
const char *inet_ntop(int, const void *, char *, socklen_t);
#endif


#if defined(__native_client__) && !defined( HAVE_NETINET_TCP_H )
#define TCP_NODELAY 1
#endif

#if !defined(HAVE_STRUCT_POLLFD)
enum
{
POLLERR=0x1,
POLLHUP=0x2,
POLLNVAL=0x4,
POLLWRNORM=0x10,
POLLWRBAND=0x20,
POLLRDNORM=0x100,
POLLRDBAND=0x200,
POLLPRI=0x400,
};
#define POLLIN (POLLRDNORM|POLLRDBAND)
#define POLLOUT (POLLWRNORM|POLLWRBAND)

struct pollfd
{
int fd;
short events;
short revents;
};
#endif
#if !defined(HAVE_POLL)
struct pollfd;
int poll (struct pollfd *, unsigned, int);
#endif

#if !defined(HAVE_IF_NAMEINDEX)
#include <errno.h>
#if !defined(HAVE_STRUCT_IF_NAMEINDEX)
struct if_nameindex
{
unsigned if_index;
char *if_name;
};
#endif
#define if_nameindex() (errno = ENOBUFS, NULL)
#define if_freenameindex(list) (void)0
#endif

#if !defined(HAVE_STRUCT_TIMESPEC)
struct timespec {
time_t tv_sec; 
long tv_nsec; 
};
#endif

#if defined(_WIN32)
struct iovec
{
void *iov_base;
size_t iov_len;
};
#define IOV_MAX 255
struct msghdr
{
void *msg_name;
size_t msg_namelen;
struct iovec *msg_iov;
size_t msg_iovlen;
void *msg_control;
size_t msg_controllen;
int msg_flags;
};
#endif

#if defined(_NEWLIB_VERSION)
#define IOV_MAX 255
#endif

#if !defined(HAVE_RECVMSG)
struct msghdr;
ssize_t recvmsg(int, struct msghdr *, int);
#endif

#if !defined(HAVE_SENDMSG)
struct msghdr;
ssize_t sendmsg(int, const struct msghdr *, int);
#endif


#if !defined(HAVE_SEARCH_H)
typedef struct entry {
char *key;
void *data;
} ENTRY;

typedef enum {
FIND, ENTER
} ACTION;

typedef enum {
preorder,
postorder,
endorder,
leaf
} VISIT;

void *tsearch( const void *key, void **rootp, int(*cmp)(const void *, const void *) );
void *tfind( const void *key, const void **rootp, int(*cmp)(const void *, const void *) );
void *tdelete( const void *key, void **rootp, int(*cmp)(const void *, const void *) );
void twalk( const void *root, void(*action)(const void *nodep, VISIT which, int depth) );
void *lfind( const void *key, const void *base, size_t *nmemb,
size_t size, int(*cmp)(const void *, const void *) );
#endif 

#if defined(_WIN64)
#if defined(HAVE_SEARCH_H)
#include <search.h>
#endif

#define lfind(a,b,c,d,e) lfind((a),(b), &(unsigned){ (*(c) > UINT_MAX) ? UINT_MAX : *(c) }, (d),(e))

#endif 

#if !defined(HAVE_TDESTROY)
void tdestroy( void *root, void (*free_node)(void *nodep) );
#endif


#if !defined(HAVE_NRAND48)
double erand48 (unsigned short subi[3]);
long jrand48 (unsigned short subi[3]);
long nrand48 (unsigned short subi[3]);
#endif

#if defined(__OS2__)
#undef HAVE_FORK 

#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2


#if !defined(EAI_BADFLAGS)
#define EAI_BADFLAGS -1
#endif
#if !defined(EAI_NONAME)
#define EAI_NONAME -2
#endif
#if !defined(EAI_AGAIN)
#define EAI_AGAIN -3
#endif
#if !defined(EAI_FAIL)
#define EAI_FAIL -4
#endif
#if !defined(EAI_NODATA)
#define EAI_NODATA -5
#endif
#if !defined(EAI_FAMILY)
#define EAI_FAMILY -6
#endif
#if !defined(EAI_SOCKTYPE)
#define EAI_SOCKTYPE -7
#endif
#if !defined(EAI_SERVICE)
#define EAI_SERVICE -8
#endif
#if !defined(EAI_ADDRFAMILY)
#define EAI_ADDRFAMILY -9
#endif
#if !defined(EAI_MEMORY)
#define EAI_MEMORY -10
#endif
#if !defined(EAI_OVERFLOW)
#define EAI_OVERFLOW -11
#endif
#if !defined(EAI_SYSTEM)
#define EAI_SYSTEM -12
#endif

#if !defined(NI_NUMERICHOST)
#define NI_NUMERICHOST 0x01
#define NI_NUMERICSERV 0x02
#define NI_NOFQDN 0x04
#define NI_NAMEREQD 0x08
#define NI_DGRAM 0x10
#endif

#if !defined(NI_MAXHOST)
#define NI_MAXHOST 1025
#define NI_MAXSERV 32
#endif

#define AI_PASSIVE 1
#define AI_CANONNAME 2
#define AI_NUMERICHOST 4

struct addrinfo
{
int ai_flags;
int ai_family;
int ai_socktype;
int ai_protocol;
size_t ai_addrlen;
struct sockaddr *ai_addr;
char *ai_canonname;
struct addrinfo *ai_next;
};

const char *gai_strerror (int);

int getaddrinfo (const char *node, const char *service,
const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo (struct addrinfo *res);
int getnameinfo (const struct sockaddr *sa, socklen_t salen,
char *host, int hostlen, char *serv, int servlen,
int flags);


#include <stdint.h>

struct in6_addr
{
uint8_t s6_addr[16];
};

struct sockaddr_in6
{
uint8_t sin6_len;
uint8_t sin6_family;
uint16_t sin6_port;
uint32_t sin6_flowinfo;
struct in6_addr sin6_addr;
uint32_t sin6_scope_id;
};

#define IN6_IS_ADDR_MULTICAST(a) (((__const uint8_t *) (a))[0] == 0xff)

static const struct in6_addr in6addr_any =
{ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } };

#include <errno.h>
#if !defined(EPROTO)
#define EPROTO (ELAST + 1)
#endif

#if !defined(HAVE_IF_NAMETOINDEX)
#define if_nametoindex(name) atoi(name)
#endif
#endif



#if !defined(HAVE_NANF)
#define nanf(tagp) NAN
#endif

#if !defined(HAVE_SINCOS)
void sincos(double, double *, double *);
void sincosf(float, float *, float *);
#endif

#if !defined(HAVE_REALPATH)
char *realpath(const char * restrict pathname, char * restrict resolved_path);
#endif

#if defined(_WIN32)
FILE *vlc_win32_tmpfile(void);
#endif


#if defined(_WIN32) && defined(__MINGW64_VERSION_MAJOR)
#define IN6_IS_ADDR_MULTICAST IN6_IS_ADDR_MULTICAST
#endif

#if defined(__APPLE__)
#define fdatasync fsync

#include <time.h>
#if !defined(TIMER_ABSTIME)
#define TIMER_ABSTIME 0x01
#endif
#if !defined(CLOCK_REALTIME)
#define CLOCK_REALTIME 0
#endif
#if !defined(CLOCK_MONOTONIC)
#define CLOCK_MONOTONIC 6
#endif
#if !defined(HAVE_CLOCK_GETTIME)
int clock_gettime(clockid_t clock_id, struct timespec *tp);
#endif
#if !defined(HAVE_CLOCK_GETRES)
int clock_getres(clockid_t clock_id, struct timespec *tp);
#endif
#if !defined(HAVE_CLOCK_NANOSLEEP)
int clock_nanosleep(clockid_t clock_id, int flags,
const struct timespec *rqtp, struct timespec *rmtp);
#endif
#endif

#if defined(__cplusplus)
} 
#endif

#endif 
