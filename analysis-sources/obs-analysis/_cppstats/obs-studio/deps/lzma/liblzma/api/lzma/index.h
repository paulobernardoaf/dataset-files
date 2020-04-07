#if !defined(LZMA_H_INTERNAL)
#error Never include this file directly. Use <lzma.h> instead.
#endif
typedef struct lzma_index_s lzma_index;
typedef struct {
struct {
const lzma_stream_flags *flags;
const void *reserved_ptr1;
const void *reserved_ptr2;
const void *reserved_ptr3;
lzma_vli number;
lzma_vli block_count;
lzma_vli compressed_offset;
lzma_vli uncompressed_offset;
lzma_vli compressed_size;
lzma_vli uncompressed_size;
lzma_vli padding;
lzma_vli reserved_vli1;
lzma_vli reserved_vli2;
lzma_vli reserved_vli3;
lzma_vli reserved_vli4;
} stream;
struct {
lzma_vli number_in_file;
lzma_vli compressed_file_offset;
lzma_vli uncompressed_file_offset;
lzma_vli number_in_stream;
lzma_vli compressed_stream_offset;
lzma_vli uncompressed_stream_offset;
lzma_vli uncompressed_size;
lzma_vli unpadded_size;
lzma_vli total_size;
lzma_vli reserved_vli1;
lzma_vli reserved_vli2;
lzma_vli reserved_vli3;
lzma_vli reserved_vli4;
const void *reserved_ptr1;
const void *reserved_ptr2;
const void *reserved_ptr3;
const void *reserved_ptr4;
} block;
union {
const void *p;
size_t s;
lzma_vli v;
} internal[6];
} lzma_index_iter;
typedef enum {
LZMA_INDEX_ITER_ANY = 0,
LZMA_INDEX_ITER_STREAM = 1,
LZMA_INDEX_ITER_BLOCK = 2,
LZMA_INDEX_ITER_NONEMPTY_BLOCK = 3
} lzma_index_iter_mode;
extern LZMA_API(uint64_t) lzma_index_memusage(
lzma_vli streams, lzma_vli blocks) lzma_nothrow;
extern LZMA_API(uint64_t) lzma_index_memused(const lzma_index *i)
lzma_nothrow;
extern LZMA_API(lzma_index *) lzma_index_init(lzma_allocator *allocator)
lzma_nothrow;
extern LZMA_API(void) lzma_index_end(lzma_index *i, lzma_allocator *allocator)
lzma_nothrow;
extern LZMA_API(lzma_ret) lzma_index_append(
lzma_index *i, lzma_allocator *allocator,
lzma_vli unpadded_size, lzma_vli uncompressed_size)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(lzma_ret) lzma_index_stream_flags(
lzma_index *i, const lzma_stream_flags *stream_flags)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(uint32_t) lzma_index_checks(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_ret) lzma_index_stream_padding(
lzma_index *i, lzma_vli stream_padding)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(lzma_vli) lzma_index_stream_count(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_vli) lzma_index_block_count(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_vli) lzma_index_size(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_vli) lzma_index_stream_size(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_vli) lzma_index_total_size(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_vli) lzma_index_file_size(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(lzma_vli) lzma_index_uncompressed_size(const lzma_index *i)
lzma_nothrow lzma_attr_pure;
extern LZMA_API(void) lzma_index_iter_init(
lzma_index_iter *iter, const lzma_index *i) lzma_nothrow;
extern LZMA_API(void) lzma_index_iter_rewind(lzma_index_iter *iter)
lzma_nothrow;
extern LZMA_API(lzma_bool) lzma_index_iter_next(
lzma_index_iter *iter, lzma_index_iter_mode mode)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(lzma_bool) lzma_index_iter_locate(
lzma_index_iter *iter, lzma_vli target) lzma_nothrow;
extern LZMA_API(lzma_ret) lzma_index_cat(
lzma_index *restrict dest, lzma_index *restrict src,
lzma_allocator *allocator)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(lzma_index *) lzma_index_dup(
const lzma_index *i, lzma_allocator *allocator)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(lzma_ret) lzma_index_encoder(
lzma_stream *strm, const lzma_index *i)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(lzma_ret) lzma_index_decoder(
lzma_stream *strm, lzma_index **i, uint64_t memlimit)
lzma_nothrow lzma_attr_warn_unused_result;
extern LZMA_API(lzma_ret) lzma_index_buffer_encode(const lzma_index *i,
uint8_t *out, size_t *out_pos, size_t out_size) lzma_nothrow;
extern LZMA_API(lzma_ret) lzma_index_buffer_decode(lzma_index **i,
uint64_t *memlimit, lzma_allocator *allocator,
const uint8_t *in, size_t *in_pos, size_t in_size)
lzma_nothrow;
