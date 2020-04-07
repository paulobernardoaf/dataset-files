















#include "check.h"
#include "crc_macros.h"


#if defined(WORDS_BIGENDIAN)
#define A1(x) ((x) >> 56)
#else
#define A1 A
#endif



extern LZMA_API(uint64_t)
lzma_crc64(const uint8_t *buf, size_t size, uint64_t crc)
{
crc = ~crc;

#if defined(WORDS_BIGENDIAN)
crc = bswap64(crc);
#endif

if (size > 4) {
while ((uintptr_t)(buf) & 3) {
crc = lzma_crc64_table[0][*buf++ ^ A1(crc)] ^ S8(crc);
--size;
}

const uint8_t *const limit = buf + (size & ~(size_t)(3));
size &= (size_t)(3);

while (buf < limit) {
#if defined(WORDS_BIGENDIAN)
const uint32_t tmp = (crc >> 32)
^ *(const uint32_t *)(buf);
#else
const uint32_t tmp = crc ^ *(const uint32_t *)(buf);
#endif
buf += 4;

crc = lzma_crc64_table[3][A(tmp)]
^ lzma_crc64_table[2][B(tmp)]
^ S32(crc)
^ lzma_crc64_table[1][C(tmp)]
^ lzma_crc64_table[0][D(tmp)];
}
}

while (size-- != 0)
crc = lzma_crc64_table[0][*buf++ ^ A1(crc)] ^ S8(crc);

#if defined(WORDS_BIGENDIAN)
crc = bswap64(crc);
#endif

return ~crc;
}
