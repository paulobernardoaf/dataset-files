
#if defined(JEMALLOC_H_TYPES)



#if defined(_WIN32)
#if defined(_WIN64)
#define FMT64_PREFIX "ll"
#define FMTPTR_PREFIX "ll"
#else
#define FMT64_PREFIX "ll"
#define FMTPTR_PREFIX ""
#endif
#define FMTd32 "d"
#define FMTu32 "u"
#define FMTx32 "x"
#define FMTd64 FMT64_PREFIX "d"
#define FMTu64 FMT64_PREFIX "u"
#define FMTx64 FMT64_PREFIX "x"
#define FMTdPTR FMTPTR_PREFIX "d"
#define FMTuPTR FMTPTR_PREFIX "u"
#define FMTxPTR FMTPTR_PREFIX "x"
#else
#include <inttypes.h>
#define FMTd32 PRId32
#define FMTu32 PRIu32
#define FMTx32 PRIx32
#define FMTd64 PRId64
#define FMTu64 PRIu64
#define FMTx64 PRIx64
#define FMTdPTR PRIdPTR
#define FMTuPTR PRIuPTR
#define FMTxPTR PRIxPTR
#endif







#define BUFERROR_BUF 64





#define MALLOC_PRINTF_BUFSIZE 4096


#if !defined(JEMALLOC_ALLOC_JUNK)
#define JEMALLOC_ALLOC_JUNK ((uint8_t)0xa5)
#endif
#if !defined(JEMALLOC_FREE_JUNK)
#define JEMALLOC_FREE_JUNK ((uint8_t)0x5a)
#endif





#define JEMALLOC_ARG_CONCAT(...) __VA_ARGS__






#if defined(JEMALLOC_CC_SILENCE)
#define JEMALLOC_CC_SILENCE_INIT(v) = v
#else
#define JEMALLOC_CC_SILENCE_INIT(v)
#endif

#if defined(__GNUC__)
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) !!(x)
#define unlikely(x) !!(x)
#endif

#if !defined(JEMALLOC_INTERNAL_UNREACHABLE)
#error JEMALLOC_INTERNAL_UNREACHABLE should have been defined by configure
#endif

#define unreachable() JEMALLOC_INTERNAL_UNREACHABLE()

#include "assert.h"


#define cassert(c) do { if (unlikely(!(c))) not_reached(); } while (0)




#endif 

#if defined(JEMALLOC_H_STRUCTS)

#endif 

#if defined(JEMALLOC_H_EXTERNS)

int buferror(int err, char *buf, size_t buflen);
uintmax_t malloc_strtoumax(const char *nptr,
char **endptr, int base);
void malloc_write(const char *s);





size_t malloc_vsnprintf(char *str, size_t size, const char *format,
va_list ap);










#endif 

#if defined(JEMALLOC_H_INLINES)

#if !defined(JEMALLOC_ENABLE_INLINE)
unsigned ffs_llu(unsigned long long bitmap);
unsigned ffs_lu(unsigned long bitmap);
unsigned ffs_u(unsigned bitmap);
unsigned ffs_zu(size_t bitmap);
unsigned ffs_u64(uint64_t bitmap);
unsigned ffs_u32(uint32_t bitmap);
uint64_t pow2_ceil_u64(uint64_t x);
uint32_t pow2_ceil_u32(uint32_t x);
size_t pow2_ceil_zu(size_t x);
unsigned lg_floor(size_t x);
void set_errno(int errnum);
int get_errno(void);
#endif

#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_UTIL_C_))


#if !defined(JEMALLOC_INTERNAL_FFSLL) || !defined(JEMALLOC_INTERNAL_FFSL) || !defined(JEMALLOC_INTERNAL_FFS)

#error JEMALLOC_INTERNAL_FFS{,L,LL} should have been defined by configure
#endif

JEMALLOC_ALWAYS_INLINE unsigned
ffs_llu(unsigned long long bitmap)
{

return (JEMALLOC_INTERNAL_FFSLL(bitmap));
}

JEMALLOC_ALWAYS_INLINE unsigned
ffs_lu(unsigned long bitmap)
{

return (JEMALLOC_INTERNAL_FFSL(bitmap));
}

JEMALLOC_ALWAYS_INLINE unsigned
ffs_u(unsigned bitmap)
{

return (JEMALLOC_INTERNAL_FFS(bitmap));
}

JEMALLOC_ALWAYS_INLINE unsigned
ffs_zu(size_t bitmap)
{

#if LG_SIZEOF_PTR == LG_SIZEOF_INT
return (ffs_u(bitmap));
#elif LG_SIZEOF_PTR == LG_SIZEOF_LONG
return (ffs_lu(bitmap));
#elif LG_SIZEOF_PTR == LG_SIZEOF_LONG_LONG
return (ffs_llu(bitmap));
#else
#error No implementation for size_t ffs()
#endif
}

JEMALLOC_ALWAYS_INLINE unsigned
ffs_u64(uint64_t bitmap)
{

#if LG_SIZEOF_LONG == 3
return (ffs_lu(bitmap));
#elif LG_SIZEOF_LONG_LONG == 3
return (ffs_llu(bitmap));
#else
#error No implementation for 64-bit ffs()
#endif
}

JEMALLOC_ALWAYS_INLINE unsigned
ffs_u32(uint32_t bitmap)
{

#if LG_SIZEOF_INT == 2
return (ffs_u(bitmap));
#else
#error No implementation for 32-bit ffs()
#endif
return (ffs_u(bitmap));
}

JEMALLOC_INLINE uint64_t
pow2_ceil_u64(uint64_t x)
{

x--;
x |= x >> 1;
x |= x >> 2;
x |= x >> 4;
x |= x >> 8;
x |= x >> 16;
x |= x >> 32;
x++;
return (x);
}

JEMALLOC_INLINE uint32_t
pow2_ceil_u32(uint32_t x)
{

x--;
x |= x >> 1;
x |= x >> 2;
x |= x >> 4;
x |= x >> 8;
x |= x >> 16;
x++;
return (x);
}


JEMALLOC_INLINE size_t
pow2_ceil_zu(size_t x)
{

#if (LG_SIZEOF_PTR == 3)
return (pow2_ceil_u64(x));
#else
return (pow2_ceil_u32(x));
#endif
}

#if (defined(__i386__) || defined(__amd64__) || defined(__x86_64__))
JEMALLOC_INLINE unsigned
lg_floor(size_t x)
{
size_t ret;

if (x == 0) {
return UINT_MAX;
}

asm ("bsr %1, %0"
: "=r"(ret) 
: "r"(x) 
);

return ((unsigned)ret);
}
#elif (defined(_MSC_VER))
JEMALLOC_INLINE unsigned
lg_floor(size_t x)
{
unsigned long ret;

assert(x != 0);

#if (LG_SIZEOF_PTR == 3)
_BitScanReverse64(&ret, x);
#elif (LG_SIZEOF_PTR == 2)
_BitScanReverse(&ret, x);
#else
#error "Unsupported type size for lg_floor()"
#endif
assert(ret < UINT_MAX);
return ((unsigned)ret);
}
#elif (defined(JEMALLOC_HAVE_BUILTIN_CLZ))
JEMALLOC_INLINE unsigned
lg_floor(size_t x)
{

assert(x != 0);

#if (LG_SIZEOF_PTR == LG_SIZEOF_INT)
return (((8 << LG_SIZEOF_PTR) - 1) - __builtin_clz(x));
#elif (LG_SIZEOF_PTR == LG_SIZEOF_LONG)
return (((8 << LG_SIZEOF_PTR) - 1) - __builtin_clzl(x));
#else
#error "Unsupported type size for lg_floor()"
#endif
}
#else
JEMALLOC_INLINE unsigned
lg_floor(size_t x)
{

assert(x != 0);

x |= (x >> 1);
x |= (x >> 2);
x |= (x >> 4);
x |= (x >> 8);
x |= (x >> 16);
#if (LG_SIZEOF_PTR == 3)
x |= (x >> 32);
#endif
if (x == SIZE_T_MAX)
return ((8 << LG_SIZEOF_PTR) - 1);
x++;
return (ffs_zu(x) - 2);
}
#endif


JEMALLOC_INLINE void
set_errno(int errnum)
{

#if defined(_WIN32)
SetLastError(errnum);
#else
errno = errnum;
#endif
}


JEMALLOC_INLINE int
get_errno(void)
{

#if defined(_WIN32)
return (GetLastError());
#else
return (errno);
#endif
}
#endif

#endif 

