#include "check.h"
#include "crc_macros.h"
extern LZMA_API(uint32_t)
lzma_crc32(const uint8_t *buf, size_t size, uint32_t crc)
{
crc = ~crc;
#if defined(WORDS_BIGENDIAN)
crc = bswap32(crc);
#endif
if (size > 8) {
while ((uintptr_t)(buf) & 7) {
crc = lzma_crc32_table[0][*buf++ ^ A(crc)] ^ S8(crc);
--size;
}
const uint8_t *const limit = buf + (size & ~(size_t)(7));
size &= (size_t)(7);
while (buf < limit) {
crc ^= *(const uint32_t *)(buf);
buf += 4;
crc = lzma_crc32_table[7][A(crc)]
^ lzma_crc32_table[6][B(crc)]
^ lzma_crc32_table[5][C(crc)]
^ lzma_crc32_table[4][D(crc)];
const uint32_t tmp = *(const uint32_t *)(buf);
buf += 4;
crc = lzma_crc32_table[3][A(tmp)]
^ lzma_crc32_table[2][B(tmp)]
^ crc
^ lzma_crc32_table[1][C(tmp)]
^ lzma_crc32_table[0][D(tmp)];
}
}
while (size-- != 0)
crc = lzma_crc32_table[0][*buf++ ^ A(crc)] ^ S8(crc);
#if defined(WORDS_BIGENDIAN)
crc = bswap32(crc);
#endif
return ~crc;
}
