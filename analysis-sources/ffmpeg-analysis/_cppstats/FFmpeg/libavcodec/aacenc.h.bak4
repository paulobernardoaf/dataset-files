




















#if !defined(AVCODEC_AACENC_H)
#define AVCODEC_AACENC_H

#include "libavutil/float_dsp.h"
#include "avcodec.h"
#include "put_bits.h"

#include "aac.h"
#include "audio_frame_queue.h"
#include "psymodel.h"

#include "lpc.h"

typedef enum AACCoder {
AAC_CODER_ANMR = 0,
AAC_CODER_TWOLOOP,
AAC_CODER_FAST,

AAC_CODER_NB,
}AACCoder;

typedef struct AACEncOptions {
int coder;
int pns;
int tns;
int ltp;
int pce;
int pred;
int mid_side;
int intensity_stereo;
} AACEncOptions;

struct AACEncContext;

typedef struct AACCoefficientsEncoder {
void (*search_for_quantizers)(AVCodecContext *avctx, struct AACEncContext *s,
SingleChannelElement *sce, const float lambda);
void (*encode_window_bands_info)(struct AACEncContext *s, SingleChannelElement *sce,
int win, int group_len, const float lambda);
void (*quantize_and_encode_band)(struct AACEncContext *s, PutBitContext *pb, const float *in, float *out, int size,
int scale_idx, int cb, const float lambda, int rtz);
void (*encode_tns_info)(struct AACEncContext *s, SingleChannelElement *sce);
void (*encode_ltp_info)(struct AACEncContext *s, SingleChannelElement *sce, int common_window);
void (*encode_main_pred)(struct AACEncContext *s, SingleChannelElement *sce);
void (*adjust_common_pred)(struct AACEncContext *s, ChannelElement *cpe);
void (*adjust_common_ltp)(struct AACEncContext *s, ChannelElement *cpe);
void (*apply_main_pred)(struct AACEncContext *s, SingleChannelElement *sce);
void (*apply_tns_filt)(struct AACEncContext *s, SingleChannelElement *sce);
void (*update_ltp)(struct AACEncContext *s, SingleChannelElement *sce);
void (*ltp_insert_new_frame)(struct AACEncContext *s);
void (*set_special_band_scalefactors)(struct AACEncContext *s, SingleChannelElement *sce);
void (*search_for_pns)(struct AACEncContext *s, AVCodecContext *avctx, SingleChannelElement *sce);
void (*mark_pns)(struct AACEncContext *s, AVCodecContext *avctx, SingleChannelElement *sce);
void (*search_for_tns)(struct AACEncContext *s, SingleChannelElement *sce);
void (*search_for_ltp)(struct AACEncContext *s, SingleChannelElement *sce, int common_window);
void (*search_for_ms)(struct AACEncContext *s, ChannelElement *cpe);
void (*search_for_is)(struct AACEncContext *s, AVCodecContext *avctx, ChannelElement *cpe);
void (*search_for_pred)(struct AACEncContext *s, SingleChannelElement *sce);
} AACCoefficientsEncoder;

extern const AACCoefficientsEncoder ff_aac_coders[];

typedef struct AACQuantizeBandCostCacheEntry {
float rd;
float energy;
int bits;
char cb;
char rtz;
uint16_t generation;
} AACQuantizeBandCostCacheEntry;

typedef struct AACPCEInfo {
int64_t layout;
int num_ele[4]; 
int pairing[3][8]; 
int index[4][8]; 
uint8_t config_map[16]; 
uint8_t reorder_map[16]; 
} AACPCEInfo;




































static const AACPCEInfo aac_pce_configs[] = {
{
.layout = AV_CH_LAYOUT_MONO,
.num_ele = { 1, 0, 0, 0 },
.pairing = { { 0 }, },
.index = { { 0 }, },
.config_map = { 1, TYPE_SCE, },
.reorder_map = { 0 },
},
{
.layout = AV_CH_LAYOUT_STEREO,
.num_ele = { 1, 0, 0, 0 },
.pairing = { { 1 }, },
.index = { { 0 }, },
.config_map = { 1, TYPE_CPE, },
.reorder_map = { 0, 1 },
},
{
.layout = AV_CH_LAYOUT_2POINT1,
.num_ele = { 1, 0, 0, 1 },
.pairing = { { 1 }, },
.index = { { 0 },{ 0 },{ 0 },{ 0 } },
.config_map = { 2, TYPE_CPE, TYPE_LFE },
.reorder_map = { 0, 1, 2 },
},
{
.layout = AV_CH_LAYOUT_2_1,
.num_ele = { 1, 0, 1, 0 },
.pairing = { { 1 },{ 0 },{ 0 } },
.index = { { 0 },{ 0 },{ 0 }, },
.config_map = { 2, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2 },
},
{
.layout = AV_CH_LAYOUT_SURROUND,
.num_ele = { 2, 0, 0, 0 },
.pairing = { { 1, 0 }, },
.index = { { 0, 0 }, },
.config_map = { 2, TYPE_CPE, TYPE_SCE, },
.reorder_map = { 0, 1, 2 },
},
{
.layout = AV_CH_LAYOUT_3POINT1,
.num_ele = { 2, 0, 0, 1 },
.pairing = { { 1, 0 }, },
.index = { { 0, 0 }, { 0 }, { 0 }, { 0 }, },
.config_map = { 3, TYPE_CPE, TYPE_SCE, TYPE_LFE },
.reorder_map = { 0, 1, 2, 3 },
},
{
.layout = AV_CH_LAYOUT_4POINT0,
.num_ele = { 2, 0, 1, 0 },
.pairing = { { 1, 0 }, { 0 }, { 0 }, },
.index = { { 0, 0 }, { 0 }, { 1 } },
.config_map = { 3, TYPE_CPE, TYPE_SCE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3 },
},
{
.layout = AV_CH_LAYOUT_4POINT1,
.num_ele = { 2, 1, 1, 0 },
.pairing = { { 1, 0 }, { 0 }, { 0 }, },
.index = { { 0, 0 }, { 1 }, { 2 }, { 0 } },
.config_map = { 4, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4 },
},
{
.layout = AV_CH_LAYOUT_2_2,
.num_ele = { 1, 1, 0, 0 },
.pairing = { { 1 }, { 1 }, },
.index = { { 0 }, { 1 }, },
.config_map = { 2, TYPE_CPE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3 },
},
{
.layout = AV_CH_LAYOUT_QUAD,
.num_ele = { 1, 0, 1, 0 },
.pairing = { { 1 }, { 0 }, { 1 }, },
.index = { { 0 }, { 0 }, { 1 } },
.config_map = { 2, TYPE_CPE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3 },
},
{
.layout = AV_CH_LAYOUT_5POINT0,
.num_ele = { 2, 1, 0, 0 },
.pairing = { { 1, 0 }, { 1 }, },
.index = { { 0, 0 }, { 1 } },
.config_map = { 3, TYPE_CPE, TYPE_SCE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4 },
},
{
.layout = AV_CH_LAYOUT_5POINT1,
.num_ele = { 2, 1, 1, 0 },
.pairing = { { 1, 0 }, { 0 }, { 1 }, },
.index = { { 0, 0 }, { 1 }, { 1 } },
.config_map = { 4, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4, 5 },
},
{
.layout = AV_CH_LAYOUT_5POINT0_BACK,
.num_ele = { 2, 0, 1, 0 },
.pairing = { { 1, 0 }, { 0 }, { 1 } },
.index = { { 0, 0 }, { 0 }, { 1 } },
.config_map = { 3, TYPE_CPE, TYPE_SCE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4 },
},
{
.layout = AV_CH_LAYOUT_5POINT1_BACK,
.num_ele = { 2, 1, 1, 0 },
.pairing = { { 1, 0 }, { 0 }, { 1 }, },
.index = { { 0, 0 }, { 1 }, { 1 } },
.config_map = { 4, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4, 5 },
},
{
.layout = AV_CH_LAYOUT_6POINT0,
.num_ele = { 2, 1, 1, 0 },
.pairing = { { 1, 0 }, { 1 }, { 0 }, },
.index = { { 0, 0 }, { 1 }, { 1 } },
.config_map = { 4, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5 },
},
{
.layout = AV_CH_LAYOUT_6POINT0_FRONT,
.num_ele = { 2, 1, 0, 0 },
.pairing = { { 1, 1 }, { 1 } },
.index = { { 1, 0 }, { 2 }, },
.config_map = { 3, TYPE_CPE, TYPE_CPE, TYPE_CPE, },
.reorder_map = { 0, 1, 2, 3, 4, 5 },
},
{
.layout = AV_CH_LAYOUT_HEXAGONAL,
.num_ele = { 2, 0, 2, 0 },
.pairing = { { 1, 0 },{ 0 },{ 1, 0 }, },
.index = { { 0, 0 },{ 0 },{ 1, 1 } },
.config_map = { 4, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_SCE, },
.reorder_map = { 0, 1, 2, 3, 4, 5 },
},
{
.layout = AV_CH_LAYOUT_6POINT1,
.num_ele = { 2, 1, 2, 0 },
.pairing = { { 1, 0 },{ 0 },{ 1, 0 }, },
.index = { { 0, 0 },{ 1 },{ 1, 2 } },
.config_map = { 5, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6 },
},
{
.layout = AV_CH_LAYOUT_6POINT1_BACK,
.num_ele = { 2, 1, 2, 0 },
.pairing = { { 1, 0 }, { 0 }, { 1, 0 }, },
.index = { { 0, 0 }, { 1 }, { 1, 2 } },
.config_map = { 5, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6 },
},
{
.layout = AV_CH_LAYOUT_6POINT1_FRONT,
.num_ele = { 2, 1, 2, 0 },
.pairing = { { 1, 0 }, { 0 }, { 1, 0 }, },
.index = { { 0, 0 }, { 1 }, { 1, 2 } },
.config_map = { 5, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6 },
},
{
.layout = AV_CH_LAYOUT_7POINT0,
.num_ele = { 2, 1, 1, 0 },
.pairing = { { 1, 0 }, { 1 }, { 1 }, },
.index = { { 0, 0 }, { 1 }, { 2 }, },
.config_map = { 4, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6 },
},
{
.layout = AV_CH_LAYOUT_7POINT0_FRONT,
.num_ele = { 2, 1, 1, 0 },
.pairing = { { 1, 0 }, { 1 }, { 1 }, },
.index = { { 0, 0 }, { 1 }, { 2 }, },
.config_map = { 4, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6 },
},
{
.layout = AV_CH_LAYOUT_7POINT1,
.num_ele = { 2, 1, 2, 0 },
.pairing = { { 1, 0 }, { 0 }, { 1, 1 }, },
.index = { { 0, 0 }, { 1 }, { 1, 2 }, { 0 } },
.config_map = { 5, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6, 7 },
},
{
.layout = AV_CH_LAYOUT_7POINT1_WIDE,
.num_ele = { 2, 1, 2, 0 },
.pairing = { { 1, 0 }, { 0 },{ 1, 1 }, },
.index = { { 0, 0 }, { 1 }, { 1, 2 }, { 0 } },
.config_map = { 5, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6, 7 },
},
{
.layout = AV_CH_LAYOUT_7POINT1_WIDE_BACK,
.num_ele = { 2, 1, 2, 0 },
.pairing = { { 1, 0 }, { 0 }, { 1, 1 }, },
.index = { { 0, 0 }, { 1 }, { 1, 2 }, { 0 } },
.config_map = { 5, TYPE_CPE, TYPE_SCE, TYPE_SCE, TYPE_CPE, TYPE_CPE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6, 7 },
},
{
.layout = AV_CH_LAYOUT_OCTAGONAL,
.num_ele = { 2, 1, 2, 0 },
.pairing = { { 1, 0 }, { 1 }, { 1, 0 }, },
.index = { { 0, 0 }, { 1 }, { 2, 1 } },
.config_map = { 5, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6, 7 },
},
{ 
.layout = AV_CH_LAYOUT_OCTAGONAL | AV_CH_TOP_CENTER,
.num_ele = { 2, 2, 2, 0 },
.pairing = { { 1, 0 }, { 1, 0 }, { 1, 0 }, },
.index = { { 0, 0 }, { 1, 1 }, { 2, 2 } },
.config_map = { 6, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6, 7, 8 },
},
{ 
.layout = AV_CH_LAYOUT_6POINT0_FRONT | AV_CH_BACK_CENTER |
AV_CH_BACK_LEFT | AV_CH_BACK_RIGHT | AV_CH_TOP_CENTER,
.num_ele = { 2, 2, 2, 0 },
.pairing = { { 1, 1 }, { 1, 0 }, { 1, 0 }, },
.index = { { 0, 1 }, { 2, 0 }, { 3, 1 } },
.config_map = { 6, TYPE_CPE, TYPE_CPE, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
},
{
.layout = AV_CH_LAYOUT_HEXADECAGONAL,
.num_ele = { 4, 2, 4, 0 },
.pairing = { { 1, 0, 1, 0 }, { 1, 1 }, { 1, 0, 1, 0 }, },
.index = { { 0, 0, 1, 1 }, { 2, 3 }, { 4, 2, 5, 3 } },
.config_map = { 10, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_CPE, TYPE_CPE, TYPE_SCE, TYPE_CPE, TYPE_SCE },
.reorder_map = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
},
};




typedef struct AACEncContext {
AVClass *av_class;
AACEncOptions options; 
PutBitContext pb;
FFTContext mdct1024; 
FFTContext mdct128; 
AVFloatDSPContext *fdsp;
AACPCEInfo pce; 
float *planar_samples[16]; 

int profile; 
int needs_pce; 
LPCContext lpc; 
int samplerate_index; 
int channels; 
const uint8_t *reorder_map; 
const uint8_t *chan_map; 

ChannelElement *cpe; 
FFPsyContext psy;
struct FFPsyPreprocessContext* psypp;
const AACCoefficientsEncoder *coder;
int cur_channel; 
int random_state;
float lambda;
int last_frame_pb_count; 
float lambda_sum; 
int lambda_count; 
enum RawDataBlockType cur_type; 

AudioFrameQueue afq;
DECLARE_ALIGNED(16, int, qcoefs)[96]; 
DECLARE_ALIGNED(32, float, scoefs)[1024]; 

uint16_t quantize_band_cost_cache_generation;
AACQuantizeBandCostCacheEntry quantize_band_cost_cache[256][128]; 

void (*abs_pow34)(float *out, const float *in, const int size);
void (*quant_bands)(int *out, const float *in, const float *scaled,
int size, int is_signed, int maxval, const float Q34,
const float rounding);

struct {
float *samples;
} buffer;
} AACEncContext;

void ff_aac_dsp_init_x86(AACEncContext *s);
void ff_aac_coder_init_mips(AACEncContext *c);
void ff_quantize_band_cost_cache_init(struct AACEncContext *s);


#endif 
