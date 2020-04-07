#if !defined(LZMA_H_INTERNAL)
#error Never include this file directly. Use <lzma.h> instead.
#endif
typedef enum {
LZMA_CHECK_NONE = 0,
LZMA_CHECK_CRC32 = 1,
LZMA_CHECK_CRC64 = 4,
LZMA_CHECK_SHA256 = 10
} lzma_check;
#define LZMA_CHECK_ID_MAX 15
extern LZMA_API(lzma_bool) lzma_check_is_supported(lzma_check check)
lzma_nothrow lzma_attr_const;
extern LZMA_API(uint32_t) lzma_check_size(lzma_check check)
lzma_nothrow lzma_attr_const;
#define LZMA_CHECK_SIZE_MAX 64
extern LZMA_API(uint32_t) lzma_crc32(
const uint8_t *buf, size_t size, uint32_t crc)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(uint64_t) lzma_crc64(
const uint8_t *buf, size_t size, uint64_t crc)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_check) lzma_get_check(const lzma_stream *strm)
lzma_nothrow;
