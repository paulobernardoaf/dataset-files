#include "common.h"
typedef struct {
lzma_filter filters[LZMA_FILTERS_MAX + 1];
lzma_options_lzma opt_lzma;
} lzma_options_easy;
extern bool lzma_easy_preset(lzma_options_easy *easy, uint32_t preset);
