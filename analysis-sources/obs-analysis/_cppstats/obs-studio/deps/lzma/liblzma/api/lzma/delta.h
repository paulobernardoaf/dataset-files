#if !defined(LZMA_H_INTERNAL)
#error Never include this file directly. Use <lzma.h> instead.
#endif
#define LZMA_FILTER_DELTA LZMA_VLI_C(0x03)
typedef enum {
LZMA_DELTA_TYPE_BYTE
} lzma_delta_type;
typedef struct {
lzma_delta_type type;
uint32_t dist;
#define LZMA_DELTA_DIST_MIN 1
#define LZMA_DELTA_DIST_MAX 256
uint32_t reserved_int1;
uint32_t reserved_int2;
uint32_t reserved_int3;
uint32_t reserved_int4;
void *reserved_ptr1;
void *reserved_ptr2;
} lzma_options_delta;
