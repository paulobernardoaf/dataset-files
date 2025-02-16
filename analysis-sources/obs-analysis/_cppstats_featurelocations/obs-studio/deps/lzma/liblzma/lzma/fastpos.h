












#if !defined(LZMA_FASTPOS_H)
#define LZMA_FASTPOS_H






























































#if defined(HAVE_SMALL)
#define get_pos_slot(pos) ((pos) <= 4 ? (pos) : get_pos_slot_2(pos))

static inline uint32_t
get_pos_slot_2(uint32_t pos)
{
const uint32_t i = bsr32(pos);
return (i + i) + ((pos >> (i - 1)) & 1);
}


#else

#define FASTPOS_BITS 13

extern const uint8_t lzma_fastpos[1 << FASTPOS_BITS];


#define fastpos_shift(extra, n) ((extra) + (n) * (FASTPOS_BITS - 1))


#define fastpos_limit(extra, n) (UINT32_C(1) << (FASTPOS_BITS + fastpos_shift(extra, n)))


#define fastpos_result(pos, extra, n) lzma_fastpos[(pos) >> fastpos_shift(extra, n)] + 2 * fastpos_shift(extra, n)




static inline uint32_t
get_pos_slot(uint32_t pos)
{


if (pos < fastpos_limit(0, 0))
return lzma_fastpos[pos];

if (pos < fastpos_limit(0, 1))
return fastpos_result(pos, 0, 1);

return fastpos_result(pos, 0, 2);
}


#if defined(FULL_DISTANCES_BITS)
static inline uint32_t
get_pos_slot_2(uint32_t pos)
{
assert(pos >= FULL_DISTANCES);

if (pos < fastpos_limit(FULL_DISTANCES_BITS - 1, 0))
return fastpos_result(pos, FULL_DISTANCES_BITS - 1, 0);

if (pos < fastpos_limit(FULL_DISTANCES_BITS - 1, 1))
return fastpos_result(pos, FULL_DISTANCES_BITS - 1, 1);

return fastpos_result(pos, FULL_DISTANCES_BITS - 1, 2);
}
#endif

#endif

#endif
