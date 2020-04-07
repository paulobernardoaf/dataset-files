#include "get_bits.h"
#include "blockdsp.h"
#include "proresdsp.h"
typedef struct {
const uint8_t *data;
unsigned mb_x;
unsigned mb_y;
unsigned mb_count;
unsigned data_size;
int ret;
} SliceContext;
typedef struct {
BlockDSPContext bdsp;
ProresDSPContext prodsp;
AVFrame *frame;
int frame_type; 
uint8_t qmat_luma[64];
uint8_t qmat_chroma[64];
SliceContext *slices;
int slice_count; 
unsigned mb_width; 
unsigned mb_height; 
uint8_t progressive_scan[64];
uint8_t interlaced_scan[64];
const uint8_t *scan;
int first_field;
int alpha_info;
void (*unpack_alpha)(GetBitContext *gb, uint16_t *dst, int num_coeffs, const int num_bits);
} ProresContext;
