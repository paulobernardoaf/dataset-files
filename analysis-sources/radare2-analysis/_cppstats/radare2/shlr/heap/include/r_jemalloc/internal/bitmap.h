#if defined(JEMALLOC_H_TYPES)
#define LG_BITMAP_MAXBITS LG_RUN_MAXREGS
#define BITMAP_MAXBITS (ZU(1) << LG_BITMAP_MAXBITS)
typedef struct bitmap_level_s bitmap_level_t;
typedef struct bitmap_info_s bitmap_info_t;
typedef unsigned long bitmap_t;
#define LG_SIZEOF_BITMAP LG_SIZEOF_LONG
#define LG_BITMAP_GROUP_NBITS (LG_SIZEOF_BITMAP + 3)
#define BITMAP_GROUP_NBITS (ZU(1) << LG_BITMAP_GROUP_NBITS)
#define BITMAP_GROUP_NBITS_MASK (BITMAP_GROUP_NBITS-1)
#if LG_BITMAP_MAXBITS - LG_BITMAP_GROUP_NBITS > 3
#define USE_TREE
#endif
#define BITMAP_BITS2GROUPS(nbits) ((nbits + BITMAP_GROUP_NBITS_MASK) >> LG_BITMAP_GROUP_NBITS)
#define BITMAP_GROUPS_L0(nbits) BITMAP_BITS2GROUPS(nbits)
#define BITMAP_GROUPS_L1(nbits) BITMAP_BITS2GROUPS(BITMAP_BITS2GROUPS(nbits))
#define BITMAP_GROUPS_L2(nbits) BITMAP_BITS2GROUPS(BITMAP_BITS2GROUPS(BITMAP_BITS2GROUPS((nbits))))
#define BITMAP_GROUPS_L3(nbits) BITMAP_BITS2GROUPS(BITMAP_BITS2GROUPS(BITMAP_BITS2GROUPS( BITMAP_BITS2GROUPS((nbits)))))
#define BITMAP_GROUPS_1_LEVEL(nbits) BITMAP_GROUPS_L0(nbits)
#define BITMAP_GROUPS_2_LEVEL(nbits) (BITMAP_GROUPS_1_LEVEL(nbits) + BITMAP_GROUPS_L1(nbits))
#define BITMAP_GROUPS_3_LEVEL(nbits) (BITMAP_GROUPS_2_LEVEL(nbits) + BITMAP_GROUPS_L2(nbits))
#define BITMAP_GROUPS_4_LEVEL(nbits) (BITMAP_GROUPS_3_LEVEL(nbits) + BITMAP_GROUPS_L3(nbits))
#if defined(USE_TREE)
#if LG_BITMAP_MAXBITS <= LG_BITMAP_GROUP_NBITS
#define BITMAP_GROUPS_MAX BITMAP_GROUPS_1_LEVEL(BITMAP_MAXBITS)
#elif LG_BITMAP_MAXBITS <= LG_BITMAP_GROUP_NBITS * 2
#define BITMAP_GROUPS_MAX BITMAP_GROUPS_2_LEVEL(BITMAP_MAXBITS)
#elif LG_BITMAP_MAXBITS <= LG_BITMAP_GROUP_NBITS * 3
#define BITMAP_GROUPS_MAX BITMAP_GROUPS_3_LEVEL(BITMAP_MAXBITS)
#elif LG_BITMAP_MAXBITS <= LG_BITMAP_GROUP_NBITS * 4
#define BITMAP_GROUPS_MAX BITMAP_GROUPS_4_LEVEL(BITMAP_MAXBITS)
#else
#error "Unsupported bitmap size"
#endif
#define BITMAP_MAX_LEVELS (LG_BITMAP_MAXBITS / LG_SIZEOF_BITMAP) + !!(LG_BITMAP_MAXBITS % LG_SIZEOF_BITMAP)
#else 
#define BITMAP_GROUPS_MAX BITMAP_BITS2GROUPS(BITMAP_MAXBITS)
#endif 
#endif 
#if defined(JEMALLOC_H_STRUCTS)
struct bitmap_level_s {
size_t group_offset;
};
struct bitmap_info_s {
size_t nbits;
#if defined(USE_TREE)
unsigned nlevels;
bitmap_level_t levels[BITMAP_MAX_LEVELS+1];
#else 
size_t ngroups;
#endif 
};
#endif 
#if defined(JEMALLOC_H_EXTERNS)
void bitmap_info_init(bitmap_info_t *binfo, size_t nbits);
void bitmap_init(bitmap_t *bitmap, const bitmap_info_t *binfo);
size_t bitmap_size(const bitmap_info_t *binfo);
#endif 
#if defined(JEMALLOC_H_INLINES)
#if !defined(JEMALLOC_ENABLE_INLINE)
bool bitmap_full(bitmap_t *bitmap, const bitmap_info_t *binfo);
bool bitmap_get(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit);
void bitmap_set(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit);
size_t bitmap_sfu(bitmap_t *bitmap, const bitmap_info_t *binfo);
void bitmap_unset(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit);
#endif
#if (defined(JEMALLOC_ENABLE_INLINE) || defined(JEMALLOC_BITMAP_C_))
JEMALLOC_INLINE bool
bitmap_full(bitmap_t *bitmap, const bitmap_info_t *binfo)
{
#if defined(USE_TREE)
size_t rgoff = binfo->levels[binfo->nlevels].group_offset - 1;
bitmap_t rg = bitmap[rgoff];
return (rg == 0);
#else
size_t i;
for (i = 0; i < binfo->ngroups; i++) {
if (bitmap[i] != 0)
return (false);
}
return (true);
#endif
}
JEMALLOC_INLINE bool
bitmap_get(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit)
{
size_t goff;
bitmap_t g;
if (unlikely(bit > binfo->nbits))
return (false);
goff = bit >> LG_BITMAP_GROUP_NBITS;
g = bitmap[goff];
return (!(g & (ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK))));
}
JEMALLOC_INLINE void
bitmap_set(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit)
{
size_t goff;
bitmap_t *gp;
bitmap_t g;
assert(bit < binfo->nbits);
assert(!bitmap_get(bitmap, binfo, bit));
goff = bit >> LG_BITMAP_GROUP_NBITS;
gp = &bitmap[goff];
g = *gp;
assert(g & (ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK)));
g ^= ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK);
*gp = g;
assert(bitmap_get(bitmap, binfo, bit));
#if defined(USE_TREE)
if (g == 0) {
unsigned i;
for (i = 1; i < binfo->nlevels; i++) {
bit = goff;
goff = bit >> LG_BITMAP_GROUP_NBITS;
gp = &bitmap[binfo->levels[i].group_offset + goff];
g = *gp;
assert(g & (ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK)));
g ^= ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK);
*gp = g;
if (g != 0)
break;
}
}
#endif
}
JEMALLOC_INLINE size_t
bitmap_sfu(bitmap_t *bitmap, const bitmap_info_t *binfo)
{
size_t bit;
bitmap_t g;
unsigned i;
assert(!bitmap_full(bitmap, binfo));
#if defined(USE_TREE)
i = binfo->nlevels - 1;
g = bitmap[binfo->levels[i].group_offset];
bit = ffs_lu(g) - 1;
while (i > 0) {
i--;
g = bitmap[binfo->levels[i].group_offset + bit];
bit = (bit << LG_BITMAP_GROUP_NBITS) + (ffs_lu(g) - 1);
}
#else
i = 0;
g = bitmap[0];
while ((bit = ffs_lu(g)) == 0) {
i++;
g = bitmap[i];
}
bit = (i << LG_BITMAP_GROUP_NBITS) + (bit - 1);
#endif
bitmap_set(bitmap, binfo, bit);
return (bit);
}
JEMALLOC_INLINE void
bitmap_unset(bitmap_t *bitmap, const bitmap_info_t *binfo, size_t bit)
{
size_t goff;
bitmap_t *gp;
bitmap_t g;
UNUSED bool propagate;
assert(bit < binfo->nbits);
assert(bitmap_get(bitmap, binfo, bit));
goff = bit >> LG_BITMAP_GROUP_NBITS;
gp = &bitmap[goff];
g = *gp;
propagate = (g == 0);
assert((g & (ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK))) == 0);
g ^= ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK);
*gp = g;
assert(!bitmap_get(bitmap, binfo, bit));
#if defined(USE_TREE)
if (propagate) {
unsigned i;
for (i = 1; i < binfo->nlevels; i++) {
bit = goff;
goff = bit >> LG_BITMAP_GROUP_NBITS;
gp = &bitmap[binfo->levels[i].group_offset + goff];
g = *gp;
propagate = (g == 0);
assert((g & (ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK)))
== 0);
g ^= ZU(1) << (bit & BITMAP_GROUP_NBITS_MASK);
*gp = g;
if (!propagate)
break;
}
}
#endif 
}
#endif
#endif 
