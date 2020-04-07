






















#if !defined(AVCODEC_DNXHDENC_H)
#define AVCODEC_DNXHDENC_H

#include <stdint.h>

#include "config.h"

#include "mpegvideo.h"
#include "dnxhddata.h"

typedef struct RCCMPEntry {
uint16_t mb;
int value;
} RCCMPEntry;

typedef struct RCEntry {
int ssd;
int bits;
} RCEntry;

typedef struct DNXHDEncContext {
AVClass *class;
BlockDSPContext bdsp;
MpegEncContext m; 

int cid;
int profile;
int bit_depth;
int is_444;
const CIDEntry *cid_table;
uint8_t *msip; 
uint32_t *slice_size;
uint32_t *slice_offs;

struct DNXHDEncContext *thread[MAX_THREADS];



unsigned dct_y_offset;
unsigned dct_uv_offset;
unsigned block_width_l2;

int frame_size;
int coding_unit_size;
int data_offset;

int interlaced;
int cur_field;

int nitris_compat;
unsigned min_padding;
int intra_quant_bias;

DECLARE_ALIGNED(32, int16_t, blocks)[12][64];
DECLARE_ALIGNED(16, uint8_t, edge_buf_y)[512]; 
DECLARE_ALIGNED(16, uint8_t, edge_buf_uv)[2][512]; 

int (*qmatrix_c) [64];
int (*qmatrix_l) [64];
uint16_t (*qmatrix_l16)[2][64];
uint16_t (*qmatrix_c16)[2][64];

unsigned frame_bits;
uint8_t *src[3];

uint32_t *orig_vlc_codes;
uint8_t *orig_vlc_bits;
uint32_t *vlc_codes;
uint8_t *vlc_bits;
uint16_t *run_codes;
uint8_t *run_bits;


unsigned slice_bits;
unsigned qscale;
unsigned lambda;

uint16_t *mb_bits;
uint8_t *mb_qscale;

RCCMPEntry *mb_cmp;
RCCMPEntry *mb_cmp_tmp;
RCEntry *mb_rc;

void (*get_pixels_8x4_sym)(int16_t *av_restrict block,
const uint8_t *pixels, ptrdiff_t line_size);
} DNXHDEncContext;

void ff_dnxhdenc_init_x86(DNXHDEncContext *ctx);

#endif 
