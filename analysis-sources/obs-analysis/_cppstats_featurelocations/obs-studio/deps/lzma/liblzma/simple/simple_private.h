











#if !defined(LZMA_SIMPLE_PRIVATE_H)
#define LZMA_SIMPLE_PRIVATE_H

#include "simple_coder.h"


typedef struct lzma_simple_s lzma_simple;

struct lzma_coder_s {

lzma_next_coder next;


bool end_was_reached;





bool is_encoder;



size_t (*filter)(lzma_simple *simple, uint32_t now_pos,
bool is_encoder, uint8_t *buffer, size_t size);



lzma_simple *simple;




uint32_t now_pos;


size_t allocated;



size_t pos;



size_t filtered;



size_t size;


uint8_t buffer[];
};


extern lzma_ret lzma_simple_coder_init(lzma_next_coder *next,
lzma_allocator *allocator, const lzma_filter_info *filters,
size_t (*filter)(lzma_simple *simple, uint32_t now_pos,
bool is_encoder, uint8_t *buffer, size_t size),
size_t simple_size, size_t unfiltered_max,
uint32_t alignment, bool is_encoder);

#endif
