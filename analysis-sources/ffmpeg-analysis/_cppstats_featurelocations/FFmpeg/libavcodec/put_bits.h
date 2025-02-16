
























#if !defined(AVCODEC_PUT_BITS_H)
#define AVCODEC_PUT_BITS_H

#include <stdint.h>
#include <stddef.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/avassert.h"

typedef struct PutBitContext {
uint32_t bit_buf;
int bit_left;
uint8_t *buf, *buf_ptr, *buf_end;
int size_in_bits;
} PutBitContext;







static inline void init_put_bits(PutBitContext *s, uint8_t *buffer,
int buffer_size)
{
if (buffer_size < 0) {
buffer_size = 0;
buffer = NULL;
}

s->size_in_bits = 8 * buffer_size;
s->buf = buffer;
s->buf_end = s->buf + buffer_size;
s->buf_ptr = s->buf;
s->bit_left = 32;
s->bit_buf = 0;
}




static inline int put_bits_count(PutBitContext *s)
{
return (s->buf_ptr - s->buf) * 8 + 32 - s->bit_left;
}








static inline void rebase_put_bits(PutBitContext *s, uint8_t *buffer,
int buffer_size)
{
av_assert0(8*buffer_size >= put_bits_count(s));

s->buf_end = buffer + buffer_size;
s->buf_ptr = buffer + (s->buf_ptr - s->buf);
s->buf = buffer;
s->size_in_bits = 8 * buffer_size;
}




static inline int put_bits_left(PutBitContext* s)
{
return (s->buf_end - s->buf_ptr) * 8 - 32 + s->bit_left;
}




static inline void flush_put_bits(PutBitContext *s)
{
#if !defined(BITSTREAM_WRITER_LE)
if (s->bit_left < 32)
s->bit_buf <<= s->bit_left;
#endif
while (s->bit_left < 32) {
av_assert0(s->buf_ptr < s->buf_end);
#if defined(BITSTREAM_WRITER_LE)
*s->buf_ptr++ = s->bit_buf;
s->bit_buf >>= 8;
#else
*s->buf_ptr++ = s->bit_buf >> 24;
s->bit_buf <<= 8;
#endif
s->bit_left += 8;
}
s->bit_left = 32;
s->bit_buf = 0;
}

static inline void flush_put_bits_le(PutBitContext *s)
{
while (s->bit_left < 32) {
av_assert0(s->buf_ptr < s->buf_end);
*s->buf_ptr++ = s->bit_buf;
s->bit_buf >>= 8;
s->bit_left += 8;
}
s->bit_left = 32;
s->bit_buf = 0;
}

#if defined(BITSTREAM_WRITER_LE)
#define avpriv_align_put_bits align_put_bits_unsupported_here
#define avpriv_put_string ff_put_string_unsupported_here
#define avpriv_copy_bits avpriv_copy_bits_unsupported_here
#else



void avpriv_align_put_bits(PutBitContext *s);






void avpriv_put_string(PutBitContext *pb, const char *string,
int terminate_string);






void avpriv_copy_bits(PutBitContext *pb, const uint8_t *src, int length);
#endif





static inline void put_bits(PutBitContext *s, int n, unsigned int value)
{
unsigned int bit_buf;
int bit_left;

av_assert2(n <= 31 && value < (1U << n));

bit_buf = s->bit_buf;
bit_left = s->bit_left;


#if defined(BITSTREAM_WRITER_LE)
bit_buf |= value << (32 - bit_left);
if (n >= bit_left) {
if (3 < s->buf_end - s->buf_ptr) {
AV_WL32(s->buf_ptr, bit_buf);
s->buf_ptr += 4;
} else {
av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
av_assert2(0);
}
bit_buf = value >> bit_left;
bit_left += 32;
}
bit_left -= n;
#else
if (n < bit_left) {
bit_buf = (bit_buf << n) | value;
bit_left -= n;
} else {
bit_buf <<= bit_left;
bit_buf |= value >> (n - bit_left);
if (3 < s->buf_end - s->buf_ptr) {
AV_WB32(s->buf_ptr, bit_buf);
s->buf_ptr += 4;
} else {
av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
av_assert2(0);
}
bit_left += 32 - n;
bit_buf = value;
}
#endif

s->bit_buf = bit_buf;
s->bit_left = bit_left;
}

static inline void put_bits_le(PutBitContext *s, int n, unsigned int value)
{
unsigned int bit_buf;
int bit_left;

av_assert2(n <= 31 && value < (1U << n));

bit_buf = s->bit_buf;
bit_left = s->bit_left;

bit_buf |= value << (32 - bit_left);
if (n >= bit_left) {
if (3 < s->buf_end - s->buf_ptr) {
AV_WL32(s->buf_ptr, bit_buf);
s->buf_ptr += 4;
} else {
av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
av_assert2(0);
}
bit_buf = value >> bit_left;
bit_left += 32;
}
bit_left -= n;

s->bit_buf = bit_buf;
s->bit_left = bit_left;
}

static inline void put_sbits(PutBitContext *pb, int n, int32_t value)
{
av_assert2(n >= 0 && n <= 31);

put_bits(pb, n, av_mod_uintp2(value, n));
}




static void av_unused put_bits32(PutBitContext *s, uint32_t value)
{
unsigned int bit_buf;
int bit_left;

bit_buf = s->bit_buf;
bit_left = s->bit_left;

#if defined(BITSTREAM_WRITER_LE)
bit_buf |= value << (32 - bit_left);
if (3 < s->buf_end - s->buf_ptr) {
AV_WL32(s->buf_ptr, bit_buf);
s->buf_ptr += 4;
} else {
av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
av_assert2(0);
}
bit_buf = (uint64_t)value >> bit_left;
#else
bit_buf = (uint64_t)bit_buf << bit_left;
bit_buf |= value >> (32 - bit_left);
if (3 < s->buf_end - s->buf_ptr) {
AV_WB32(s->buf_ptr, bit_buf);
s->buf_ptr += 4;
} else {
av_log(NULL, AV_LOG_ERROR, "Internal error, put_bits buffer too small\n");
av_assert2(0);
}
bit_buf = value;
#endif

s->bit_buf = bit_buf;
s->bit_left = bit_left;
}




static inline void put_bits64(PutBitContext *s, int n, uint64_t value)
{
av_assert2((n == 64) || (n < 64 && value < (UINT64_C(1) << n)));

if (n < 32)
put_bits(s, n, value);
else if (n == 32)
put_bits32(s, value);
else if (n < 64) {
uint32_t lo = value & 0xffffffff;
uint32_t hi = value >> 32;
#if defined(BITSTREAM_WRITER_LE)
put_bits32(s, lo);
put_bits(s, n - 32, hi);
#else
put_bits(s, n - 32, hi);
put_bits32(s, lo);
#endif
} else {
uint32_t lo = value & 0xffffffff;
uint32_t hi = value >> 32;
#if defined(BITSTREAM_WRITER_LE)
put_bits32(s, lo);
put_bits32(s, hi);
#else
put_bits32(s, hi);
put_bits32(s, lo);
#endif

}
}





static inline uint8_t *put_bits_ptr(PutBitContext *s)
{
return s->buf_ptr;
}





static inline void skip_put_bytes(PutBitContext *s, int n)
{
av_assert2((put_bits_count(s) & 7) == 0);
av_assert2(s->bit_left == 32);
av_assert0(n <= s->buf_end - s->buf_ptr);
s->buf_ptr += n;
}






static inline void skip_put_bits(PutBitContext *s, int n)
{
s->bit_left -= n;
s->buf_ptr -= 4 * (s->bit_left >> 5);
s->bit_left &= 31;
}






static inline void set_put_bits_buffer_size(PutBitContext *s, int size)
{
av_assert0(size <= INT_MAX/8 - 32);
s->buf_end = s->buf + size;
s->size_in_bits = 8*size;
}

#endif 
