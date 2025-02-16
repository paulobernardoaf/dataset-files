#include <sys/types.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#undef eprintf
#define eprintf(...) fprintf(stderr,__VA_ARGS__)
#if !defined(SDB_API)
#if defined(__GNUC__) && __GNUC__ >= 4
#define SDB_API __attribute__((visibility("default")))
#else
#define SDB_API
#endif
#endif
#if !defined(SDB_IPI)
#if defined(__GNUC__) && __GNUC__ >= 4
#endif
#define SDB_IPI static
#endif
#if MINGW || __MINGW32__ || __MINGW64__
#define __MINGW__ 1
#endif
#if __WIN32__ || __MINGW__ || __WINDOWS__ || _MSC_VER
#define __SDB_WINDOWS__ 1
#define DIRSEP '\\'
#include <windows.h>
#include <io.h>
#else
#define __SDB_WINDOWS__ 0
#define DIRSEP '/'
#include <unistd.h>
#endif
#include <inttypes.h>
#if __SDB_WINDOWS__ && !__CYGWIN__
#define HAVE_MMAN 0
#define ULLFMT "I64"
#else
#define HAVE_MMAN 1
#define ULLFMT "ll"
#endif
#if !defined(USE_MMAN)
#define USE_MMAN HAVE_MMAN
#endif
#if !defined(UNUSED)
#define UNUSED
#if defined(__GNUC__)
#if __GNUC__ >= 4
#undef UNUSED
#define UNUSED __attribute__((__unused__))
#endif
#endif
#endif
#if !defined(ut8)
#define ut8 unsigned char
#define ut32 unsigned int
#define ut64 unsigned long long
#define st64 long long
#define boolt int
#if !defined(R_NEW)
#define R_NEW(x) (x*)malloc(sizeof(x))
#endif
#if !defined(R_NEW0)
#define R_NEW0(x) (x*)calloc(1, sizeof(x))
#endif
#if !defined(R_FREE)
#define R_FREE(x) { free (x); x = NULL; }
#endif
#define UT32_MAX ((ut32)0xffffffff)
#define UT64_MAX ((ut64)(0xffffffffffffffffLL))
#endif
#if !defined(R_MAX_DEFINED)
#define R_MAX(x,y) (((x)>(y))?(x):(y))
#define R_MAX_DEFINED 1
#endif
#if !defined(R_MIN_DEFINED)
#define R_MIN(x,y) (((x)>(y))?(y):(x))
#define R_MIN_DEFINED 1
#endif
#include "config.h"
static inline int seek_set(int fd, off_t pos) {
return ((fd == -1) || (lseek (fd, (off_t) pos, SEEK_SET) == -1))? 0:1;
}
static inline void ut32_pack(char s[4], ut32 u) {
s[0] = u & 255;
u >>= 8;
s[1] = u & 255;
u >>= 8;
s[2] = u & 255;
s[3] = u >> 8;
}
static inline void ut32_pack_big(char s[4], ut32 u) {
s[3] = u & 255;
u >>= 8;
s[2] = u & 255;
u >>= 8;
s[1] = u & 255;
s[0] = u >> 8;
}
static inline void ut32_unpack(char s[4], ut32 *u) {
ut32 result = 0;
result = (ut8) s[3];
result <<= 8;
result += (ut8) s[2];
result <<= 8;
result += (ut8) s[1];
result <<= 8;
result += (ut8) s[0];
*u = result;
}
