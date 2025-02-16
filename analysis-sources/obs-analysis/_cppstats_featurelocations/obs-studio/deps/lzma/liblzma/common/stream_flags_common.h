











#if !defined(LZMA_STREAM_FLAGS_COMMON_H)
#define LZMA_STREAM_FLAGS_COMMON_H

#include "common.h"


#define LZMA_STREAM_FLAGS_SIZE 2

extern const uint8_t lzma_header_magic[6];
extern const uint8_t lzma_footer_magic[2];


static inline bool
is_backward_size_valid(const lzma_stream_flags *options)
{
return options->backward_size >= LZMA_BACKWARD_SIZE_MIN
&& options->backward_size <= LZMA_BACKWARD_SIZE_MAX
&& (options->backward_size & 3) == 0;
}

#endif
