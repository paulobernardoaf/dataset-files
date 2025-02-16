#if defined(JEMALLOC_H_TYPES)
#endif 
#if defined(JEMALLOC_H_STRUCTS)
#endif 
#if defined(JEMALLOC_H_EXTERNS)
#define atomic_read_uint64(p) atomic_add_uint64(p, 0)
#define atomic_read_uint32(p) atomic_add_uint32(p, 0)
#define atomic_read_p(p) atomic_add_p(p, NULL)
#define atomic_read_z(p) atomic_add_z(p, 0)
#define atomic_read_u(p) atomic_add_u(p, 0)
#endif 
#if defined(JEMALLOC_H_INLINES)
#if 0
uint64_t atomic_add_uint64(uint64_t *p, uint64_t x);
uint64_t atomic_sub_uint64(uint64_t *p, uint64_t x);
bool atomic_cas_uint64(uint64_t *p, uint64_t c, uint64_t s);
void atomic_write_uint64(uint64_t *p, uint64_t x);
uint32_t atomic_add_uint32(uint32_t *p, uint32_t x);
uint32_t atomic_sub_uint32(uint32_t *p, uint32_t x);
bool atomic_cas_uint32(uint32_t *p, uint32_t c, uint32_t s);
void atomic_write_uint32(uint32_t *p, uint32_t x);
void *atomic_add_p(void **p, void *x);
void *atomic_sub_p(void **p, void *x);
bool atomic_cas_p(void **p, void *c, void *s);
void atomic_write_p(void **p, const void *x);
size_t atomic_add_z(size_t *p, size_t x);
size_t atomic_sub_z(size_t *p, size_t x);
bool atomic_cas_z(size_t *p, size_t c, size_t s);
void atomic_write_z(size_t *p, size_t x);
unsigned atomic_add_u(unsigned *p, unsigned x);
unsigned atomic_sub_u(unsigned *p, unsigned x);
bool atomic_cas_u(unsigned *p, unsigned c, unsigned s);
void atomic_write_u(unsigned *p, unsigned x);
#endif
#if 1
JEMALLOC_INLINE uint64_t
atomic_add_uint64(uint64_t *p, uint64_t x)
{
return *p += x;
}
JEMALLOC_INLINE uint64_t
atomic_sub_uint64(uint64_t *p, uint64_t x)
{
return *p -= x;
}
JEMALLOC_INLINE bool
atomic_cas_uint64(uint64_t *p, uint64_t c, uint64_t s)
{
if (*p == c) {
*p = s;
return true;
}
return false;
}
JEMALLOC_INLINE void
atomic_write_uint64(uint64_t *p, uint64_t x)
{
uint64_t o;
do {
o = atomic_read_uint64(p);
} while (atomic_cas_uint64(p, o, x));
}
#endif
#if 1
JEMALLOC_INLINE uint32_t
atomic_add_uint32(uint32_t *p, uint32_t x)
{
return *p += x;
}
JEMALLOC_INLINE uint32_t
atomic_sub_uint32(uint32_t *p, uint32_t x)
{
return *p -= x;
}
JEMALLOC_INLINE bool
atomic_cas_uint32(uint32_t *p, uint32_t c, uint32_t s)
{
if (*p == c) {
*p = s;
return true;
}
return false;
}
JEMALLOC_INLINE void
atomic_write_uint32(uint32_t *p, uint32_t x)
{
*p = x;
}
#endif
JEMALLOC_INLINE void *
atomic_add_p(void **p, void *x)
{
if (sizeof (*p) == 8) {
return ((void *)(size_t)atomic_add_uint64((uint64_t *)p, (uint64_t)(size_t)x));
}
return ((void *)(size_t)atomic_add_uint32((uint32_t *)(size_t)p, (uint32_t)(size_t)x));
}
#if 0
JEMALLOC_INLINE void *
atomic_sub_p(void **p, void *x)
{
#if (LG_SIZEOF_PTR == 3)
return ((void *)atomic_add_uint64((uint64_t *)p,
(uint64_t)-((int64_t)x)));
#elif (LG_SIZEOF_PTR == 2)
return ((void *)atomic_add_uint32((uint32_t *)p,
(uint32_t)-((int32_t)x)));
#endif
}
JEMALLOC_INLINE bool
atomic_cas_p(void **p, void *c, void *s)
{
#if (LG_SIZEOF_PTR == 3)
return (atomic_cas_uint64((uint64_t *)p, (uint64_t)c, (uint64_t)s));
#elif (LG_SIZEOF_PTR == 2)
return (atomic_cas_uint32((uint32_t *)p, (uint32_t)c, (uint32_t)s));
#endif
}
#endif
JEMALLOC_INLINE void
atomic_write_p(void **p, const void *x)
{
#if (LG_SIZEOF_PTR == 3)
atomic_write_uint64((uint64_t *)p, (uint64_t)(size_t)x);
#elif (LG_SIZEOF_PTR == 2)
atomic_write_uint32((uint32_t *)p, (uint32_t)(size_t)x);
#endif
}
JEMALLOC_INLINE size_t
atomic_add_z(size_t *p, size_t x)
{
if (sizeof (*p) == 8) {
return ((size_t)atomic_add_uint64((uint64_t *)p, (uint64_t)x));
}
return ((size_t)atomic_add_uint32((uint32_t *)p, (uint32_t)x));
}
JEMALLOC_INLINE size_t
atomic_sub_z(size_t *p, size_t x)
{
if (sizeof (*p) == 8) {
return ((size_t)atomic_add_uint64((uint64_t *)p,
(uint64_t)-((int64_t)x)));
}
return ((size_t)atomic_add_uint32((uint32_t *)p,
(uint32_t)-((int32_t)x)));
}
JEMALLOC_INLINE bool
atomic_cas_z(size_t *p, size_t c, size_t s)
{
if (sizeof (*p) == 8) {
return (atomic_cas_uint64((uint64_t *)p, (uint64_t)c, (uint64_t)s));
}
return (atomic_cas_uint32((uint32_t *)p, (uint32_t)c, (uint32_t)s));
}
#if 0
JEMALLOC_INLINE void
atomic_write_z(size_t *p, size_t x)
{
#if (LG_SIZEOF_PTR == 3)
atomic_write_uint64((uint64_t *)p, (uint64_t)x);
#elif (LG_SIZEOF_PTR == 2)
atomic_write_uint32((uint32_t *)p, (uint32_t)x);
#endif
}
JEMALLOC_INLINE unsigned
atomic_add_u(unsigned *p, unsigned x)
{
#if (LG_SIZEOF_INT == 3)
return ((unsigned)atomic_add_uint64((uint64_t *)p, (uint64_t)x));
#elif (LG_SIZEOF_INT == 2)
return ((unsigned)atomic_add_uint32((uint32_t *)p, (uint32_t)x));
#endif
}
JEMALLOC_INLINE unsigned
atomic_sub_u(unsigned *p, unsigned x)
{
#if (LG_SIZEOF_INT == 3)
return ((unsigned)atomic_add_uint64((uint64_t *)p,
(uint64_t)-((int64_t)x)));
#elif (LG_SIZEOF_INT == 2)
return ((unsigned)atomic_add_uint32((uint32_t *)p,
(uint32_t)-((int32_t)x)));
#endif
}
JEMALLOC_INLINE bool
atomic_cas_u(unsigned *p, unsigned c, unsigned s)
{
if (sizeof(*p) == 8) {
return (atomic_cas_uint64((uint64_t *)p, (uint64_t)c, (uint64_t)s));
}
return (atomic_cas_uint32((uint32_t *)p, (uint32_t)c, (uint32_t)s));
}
JEMALLOC_INLINE void
atomic_write_u(unsigned *p, unsigned x)
{
if (sizeof(*p) == 8) {
atomic_write_uint64((uint64_t *)p, (uint64_t)x);
} else {
atomic_write_uint32((uint32_t *)p, (uint32_t)x);
}
}
#endif
#endif 
