#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "get_bits.h"
#include "bytestream.h"
#define MODEL_MIN_SYMS 2
#define MODEL_MAX_SYMS 256
#define THRESH_ADAPTIVE -1
#define THRESH_LOW 15
#define THRESH_HIGH 50
typedef struct Model {
int16_t cum_prob[MODEL_MAX_SYMS + 1];
int16_t weights[MODEL_MAX_SYMS + 1];
uint8_t idx2sym[MODEL_MAX_SYMS + 1];
int num_syms;
int thr_weight, threshold;
} Model;
typedef struct ArithCoder {
int low, high, value;
int overread;
#define MAX_OVERREAD 16
union {
GetBitContext *gb;
GetByteContext *gB;
} gbc;
int (*get_model_sym)(struct ArithCoder *c, Model *m);
int (*get_number) (struct ArithCoder *c, int n);
} ArithCoder;
typedef struct PixContext {
int cache_size, num_syms;
uint8_t cache[12];
Model cache_model, full_model;
Model sec_models[15][4];
int special_initial_cache;
} PixContext;
struct MSS12Context;
typedef struct SliceContext {
struct MSS12Context *c;
Model intra_region, inter_region;
Model pivot, edge_mode, split_mode;
PixContext intra_pix_ctx, inter_pix_ctx;
} SliceContext;
typedef struct MSS12Context {
AVCodecContext *avctx;
uint32_t pal[256];
uint8_t *pal_pic;
uint8_t *last_pal_pic;
ptrdiff_t pal_stride;
uint8_t *mask;
ptrdiff_t mask_stride;
uint8_t *rgb_pic;
uint8_t *last_rgb_pic;
ptrdiff_t rgb_stride;
int free_colours;
int keyframe;
int mvX, mvY;
int corrupted;
int slice_split;
int full_model_syms;
} MSS12Context;
int ff_mss12_decode_rect(SliceContext *ctx, ArithCoder *acoder,
int x, int y, int width, int height);
void ff_mss12_model_update(Model *m, int val);
void ff_mss12_slicecontext_reset(SliceContext *sc);
int ff_mss12_decode_init(MSS12Context *c, int version,
SliceContext *sc1, SliceContext *sc2);
int ff_mss12_decode_end(MSS12Context *ctx);
#define ARITH_GET_BIT(prefix) static int prefix ##_get_bit(ArithCoder *c) { int range = c->high - c->low + 1; int bit = 2 * c->value - c->low >= c->high; if (bit) c->low += range >> 1; else c->high = c->low + (range >> 1) - 1; prefix ##_normalise(c); return bit; }
#define ARITH_GET_MODEL_SYM(prefix) static int prefix ##_get_model_sym(ArithCoder *c, Model *m) { int idx, val; idx = prefix ##_get_prob(c, m->cum_prob); val = m->idx2sym[idx]; ff_mss12_model_update(m, idx); prefix ##_normalise(c); return val; }
