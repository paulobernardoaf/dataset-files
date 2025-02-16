


























#include <inttypes.h>

#include "avcodec.h"
#include "get_bits.h"
#include "unary.h"
#include "mpeg4audio.h"
#include "bgmc.h"
#include "bswapdsp.h"
#include "internal.h"
#include "mlz.h"
#include "libavutil/samplefmt.h"
#include "libavutil/crc.h"
#include "libavutil/softfloat_ieee754.h"
#include "libavutil/intfloat.h"
#include "libavutil/intreadwrite.h"

#include <stdint.h>





static const int8_t parcor_rice_table[3][20][2] = {
{ {-52, 4}, {-29, 5}, {-31, 4}, { 19, 4}, {-16, 4},
{ 12, 3}, { -7, 3}, { 9, 3}, { -5, 3}, { 6, 3},
{ -4, 3}, { 3, 3}, { -3, 2}, { 3, 2}, { -2, 2},
{ 3, 2}, { -1, 2}, { 2, 2}, { -1, 2}, { 2, 2} },
{ {-58, 3}, {-42, 4}, {-46, 4}, { 37, 5}, {-36, 4},
{ 29, 4}, {-29, 4}, { 25, 4}, {-23, 4}, { 20, 4},
{-17, 4}, { 16, 4}, {-12, 4}, { 12, 3}, {-10, 4},
{ 7, 3}, { -4, 4}, { 3, 3}, { -1, 3}, { 1, 3} },
{ {-59, 3}, {-45, 5}, {-50, 4}, { 38, 4}, {-39, 4},
{ 32, 4}, {-30, 4}, { 25, 3}, {-23, 3}, { 20, 3},
{-20, 3}, { 16, 3}, {-13, 3}, { 10, 3}, { -7, 3},
{ 3, 3}, { 0, 3}, { -1, 3}, { 2, 3}, { -1, 2} }
};







static const int16_t parcor_scaled_values[] = {
-1048544 / 32, -1048288 / 32, -1047776 / 32, -1047008 / 32,
-1045984 / 32, -1044704 / 32, -1043168 / 32, -1041376 / 32,
-1039328 / 32, -1037024 / 32, -1034464 / 32, -1031648 / 32,
-1028576 / 32, -1025248 / 32, -1021664 / 32, -1017824 / 32,
-1013728 / 32, -1009376 / 32, -1004768 / 32, -999904 / 32,
-994784 / 32, -989408 / 32, -983776 / 32, -977888 / 32,
-971744 / 32, -965344 / 32, -958688 / 32, -951776 / 32,
-944608 / 32, -937184 / 32, -929504 / 32, -921568 / 32,
-913376 / 32, -904928 / 32, -896224 / 32, -887264 / 32,
-878048 / 32, -868576 / 32, -858848 / 32, -848864 / 32,
-838624 / 32, -828128 / 32, -817376 / 32, -806368 / 32,
-795104 / 32, -783584 / 32, -771808 / 32, -759776 / 32,
-747488 / 32, -734944 / 32, -722144 / 32, -709088 / 32,
-695776 / 32, -682208 / 32, -668384 / 32, -654304 / 32,
-639968 / 32, -625376 / 32, -610528 / 32, -595424 / 32,
-580064 / 32, -564448 / 32, -548576 / 32, -532448 / 32,
-516064 / 32, -499424 / 32, -482528 / 32, -465376 / 32,
-447968 / 32, -430304 / 32, -412384 / 32, -394208 / 32,
-375776 / 32, -357088 / 32, -338144 / 32, -318944 / 32,
-299488 / 32, -279776 / 32, -259808 / 32, -239584 / 32,
-219104 / 32, -198368 / 32, -177376 / 32, -156128 / 32,
-134624 / 32, -112864 / 32, -90848 / 32, -68576 / 32,
-46048 / 32, -23264 / 32, -224 / 32, 23072 / 32,
46624 / 32, 70432 / 32, 94496 / 32, 118816 / 32,
143392 / 32, 168224 / 32, 193312 / 32, 218656 / 32,
244256 / 32, 270112 / 32, 296224 / 32, 322592 / 32,
349216 / 32, 376096 / 32, 403232 / 32, 430624 / 32,
458272 / 32, 486176 / 32, 514336 / 32, 542752 / 32,
571424 / 32, 600352 / 32, 629536 / 32, 658976 / 32,
688672 / 32, 718624 / 32, 748832 / 32, 779296 / 32,
810016 / 32, 840992 / 32, 872224 / 32, 903712 / 32,
935456 / 32, 967456 / 32, 999712 / 32, 1032224 / 32
};





static const uint8_t ltp_gain_values [4][4] = {
{ 0, 8, 16, 24},
{32, 40, 48, 56},
{64, 70, 76, 82},
{88, 92, 96, 100}
};





static const int16_t mcc_weightings[] = {
204, 192, 179, 166, 153, 140, 128, 115,
102, 89, 76, 64, 51, 38, 25, 12,
0, -12, -25, -38, -51, -64, -76, -89,
-102, -115, -128, -140, -153, -166, -179, -192
};




static const uint8_t tail_code[16][6] = {
{ 74, 44, 25, 13, 7, 3},
{ 68, 42, 24, 13, 7, 3},
{ 58, 39, 23, 13, 7, 3},
{126, 70, 37, 19, 10, 5},
{132, 70, 37, 20, 10, 5},
{124, 70, 38, 20, 10, 5},
{120, 69, 37, 20, 11, 5},
{116, 67, 37, 20, 11, 5},
{108, 66, 36, 20, 10, 5},
{102, 62, 36, 20, 10, 5},
{ 88, 58, 34, 19, 10, 5},
{162, 89, 49, 25, 13, 7},
{156, 87, 49, 26, 14, 7},
{150, 86, 47, 26, 14, 7},
{142, 84, 47, 26, 14, 7},
{131, 79, 46, 26, 14, 7}
};


enum RA_Flag {
RA_FLAG_NONE,
RA_FLAG_FRAMES,
RA_FLAG_HEADER
};


typedef struct ALSSpecificConfig {
uint32_t samples; 
int resolution; 
int floating; 
int msb_first; 
int frame_length; 
int ra_distance; 
enum RA_Flag ra_flag; 
int adapt_order; 
int coef_table; 
int long_term_prediction; 
int max_order; 
int block_switching; 
int bgmc; 
int sb_part; 
int joint_stereo; 
int mc_coding; 
int chan_config; 
int chan_sort; 
int rlslms; 
int chan_config_info; 
int *chan_pos; 
int crc_enabled; 
} ALSSpecificConfig;


typedef struct ALSChannelData {
int stop_flag;
int master_channel;
int time_diff_flag;
int time_diff_sign;
int time_diff_index;
int weighting[6];
} ALSChannelData;


typedef struct ALSDecContext {
AVCodecContext *avctx;
ALSSpecificConfig sconf;
GetBitContext gb;
BswapDSPContext bdsp;
const AVCRC *crc_table;
uint32_t crc_org; 
uint32_t crc; 
unsigned int cur_frame_length; 
unsigned int frame_id; 
unsigned int js_switch; 
unsigned int cs_switch; 
unsigned int num_blocks; 
unsigned int s_max; 
uint8_t *bgmc_lut; 
int *bgmc_lut_status; 
int ltp_lag_length; 
int *const_block; 
unsigned int *shift_lsbs; 
unsigned int *opt_order; 
int *store_prev_samples; 
int *use_ltp; 
int *ltp_lag; 
int **ltp_gain; 
int *ltp_gain_buffer; 
int32_t **quant_cof; 
int32_t *quant_cof_buffer; 
int32_t **lpc_cof; 
int32_t *lpc_cof_buffer; 
int32_t *lpc_cof_reversed_buffer; 
ALSChannelData **chan_data; 
ALSChannelData *chan_data_buffer; 
int *reverted_channels; 
int32_t *prev_raw_samples; 
int32_t **raw_samples; 
int32_t *raw_buffer; 
uint8_t *crc_buffer; 
MLZ* mlz; 
SoftFloat_IEEE754 *acf; 
int *last_acf_mantissa; 
int *shift_value; 
int *last_shift_value; 
int **raw_mantissa; 
unsigned char *larray; 
int *nbits; 
int highest_decoded_channel;
} ALSDecContext;


typedef struct ALSBlockData {
unsigned int block_length; 
unsigned int ra_block; 
int *const_block; 
int js_blocks; 
unsigned int *shift_lsbs; 
unsigned int *opt_order; 
int *store_prev_samples;
int *use_ltp; 
int *ltp_lag; 
int *ltp_gain; 
int32_t *quant_cof; 
int32_t *lpc_cof; 
int32_t *raw_samples; 
int32_t *prev_raw_samples; 
int32_t *raw_other; 
} ALSBlockData;


static av_cold void dprint_specific_config(ALSDecContext *ctx)
{
#if defined(DEBUG)
AVCodecContext *avctx = ctx->avctx;
ALSSpecificConfig *sconf = &ctx->sconf;

ff_dlog(avctx, "resolution = %i\n", sconf->resolution);
ff_dlog(avctx, "floating = %i\n", sconf->floating);
ff_dlog(avctx, "frame_length = %i\n", sconf->frame_length);
ff_dlog(avctx, "ra_distance = %i\n", sconf->ra_distance);
ff_dlog(avctx, "ra_flag = %i\n", sconf->ra_flag);
ff_dlog(avctx, "adapt_order = %i\n", sconf->adapt_order);
ff_dlog(avctx, "coef_table = %i\n", sconf->coef_table);
ff_dlog(avctx, "long_term_prediction = %i\n", sconf->long_term_prediction);
ff_dlog(avctx, "max_order = %i\n", sconf->max_order);
ff_dlog(avctx, "block_switching = %i\n", sconf->block_switching);
ff_dlog(avctx, "bgmc = %i\n", sconf->bgmc);
ff_dlog(avctx, "sb_part = %i\n", sconf->sb_part);
ff_dlog(avctx, "joint_stereo = %i\n", sconf->joint_stereo);
ff_dlog(avctx, "mc_coding = %i\n", sconf->mc_coding);
ff_dlog(avctx, "chan_config = %i\n", sconf->chan_config);
ff_dlog(avctx, "chan_sort = %i\n", sconf->chan_sort);
ff_dlog(avctx, "RLSLMS = %i\n", sconf->rlslms);
ff_dlog(avctx, "chan_config_info = %i\n", sconf->chan_config_info);
#endif
}




static av_cold int read_specific_config(ALSDecContext *ctx)
{
GetBitContext gb;
uint64_t ht_size;
int i, config_offset;
MPEG4AudioConfig m4ac = {0};
ALSSpecificConfig *sconf = &ctx->sconf;
AVCodecContext *avctx = ctx->avctx;
uint32_t als_id, header_size, trailer_size;
int ret;

if ((ret = init_get_bits8(&gb, avctx->extradata, avctx->extradata_size)) < 0)
return ret;

config_offset = avpriv_mpeg4audio_get_config2(&m4ac, avctx->extradata,
avctx->extradata_size, 1, avctx);

if (config_offset < 0)
return AVERROR_INVALIDDATA;

skip_bits_long(&gb, config_offset);

if (get_bits_left(&gb) < (30 << 3))
return AVERROR_INVALIDDATA;


als_id = get_bits_long(&gb, 32);
avctx->sample_rate = m4ac.sample_rate;
skip_bits_long(&gb, 32); 
sconf->samples = get_bits_long(&gb, 32);
avctx->channels = m4ac.channels;
skip_bits(&gb, 16); 
skip_bits(&gb, 3); 
sconf->resolution = get_bits(&gb, 3);
sconf->floating = get_bits1(&gb);
sconf->msb_first = get_bits1(&gb);
sconf->frame_length = get_bits(&gb, 16) + 1;
sconf->ra_distance = get_bits(&gb, 8);
sconf->ra_flag = get_bits(&gb, 2);
sconf->adapt_order = get_bits1(&gb);
sconf->coef_table = get_bits(&gb, 2);
sconf->long_term_prediction = get_bits1(&gb);
sconf->max_order = get_bits(&gb, 10);
sconf->block_switching = get_bits(&gb, 2);
sconf->bgmc = get_bits1(&gb);
sconf->sb_part = get_bits1(&gb);
sconf->joint_stereo = get_bits1(&gb);
sconf->mc_coding = get_bits1(&gb);
sconf->chan_config = get_bits1(&gb);
sconf->chan_sort = get_bits1(&gb);
sconf->crc_enabled = get_bits1(&gb);
sconf->rlslms = get_bits1(&gb);
skip_bits(&gb, 5); 
skip_bits1(&gb); 



if (als_id != MKBETAG('A','L','S','\0'))
return AVERROR_INVALIDDATA;

if (avctx->channels > FF_SANE_NB_CHANNELS) {
avpriv_request_sample(avctx, "Huge number of channels\n");
return AVERROR_PATCHWELCOME;
}

ctx->cur_frame_length = sconf->frame_length;


if (sconf->chan_config)
sconf->chan_config_info = get_bits(&gb, 16);




if (sconf->chan_sort && avctx->channels > 1) {
int chan_pos_bits = av_ceil_log2(avctx->channels);
int bits_needed = avctx->channels * chan_pos_bits + 7;
if (get_bits_left(&gb) < bits_needed)
return AVERROR_INVALIDDATA;

if (!(sconf->chan_pos = av_malloc_array(avctx->channels, sizeof(*sconf->chan_pos))))
return AVERROR(ENOMEM);

ctx->cs_switch = 1;

for (i = 0; i < avctx->channels; i++) {
sconf->chan_pos[i] = -1;
}

for (i = 0; i < avctx->channels; i++) {
int idx;

idx = get_bits(&gb, chan_pos_bits);
if (idx >= avctx->channels || sconf->chan_pos[idx] != -1) {
av_log(avctx, AV_LOG_WARNING, "Invalid channel reordering.\n");
ctx->cs_switch = 0;
break;
}
sconf->chan_pos[idx] = i;
}

align_get_bits(&gb);
}




if (get_bits_left(&gb) < 64)
return AVERROR_INVALIDDATA;

header_size = get_bits_long(&gb, 32);
trailer_size = get_bits_long(&gb, 32);
if (header_size == 0xFFFFFFFF)
header_size = 0;
if (trailer_size == 0xFFFFFFFF)
trailer_size = 0;

ht_size = ((int64_t)(header_size) + (int64_t)(trailer_size)) << 3;



if (get_bits_left(&gb) < ht_size)
return AVERROR_INVALIDDATA;

if (ht_size > INT32_MAX)
return AVERROR_PATCHWELCOME;

skip_bits_long(&gb, ht_size);



if (sconf->crc_enabled) {
if (get_bits_left(&gb) < 32)
return AVERROR_INVALIDDATA;

if (avctx->err_recognition & (AV_EF_CRCCHECK|AV_EF_CAREFUL)) {
ctx->crc_table = av_crc_get_table(AV_CRC_32_IEEE_LE);
ctx->crc = 0xFFFFFFFF;
ctx->crc_org = ~get_bits_long(&gb, 32);
} else
skip_bits_long(&gb, 32);
}




dprint_specific_config(ctx);

return 0;
}




static int check_specific_config(ALSDecContext *ctx)
{
ALSSpecificConfig *sconf = &ctx->sconf;
int error = 0;


#define MISSING_ERR(cond, str, errval) { if (cond) { avpriv_report_missing_feature(ctx->avctx, str); error = errval; } }








MISSING_ERR(sconf->rlslms, "Adaptive RLS-LMS prediction", AVERROR_PATCHWELCOME);

return error;
}





static void parse_bs_info(const uint32_t bs_info, unsigned int n,
unsigned int div, unsigned int **div_blocks,
unsigned int *num_blocks)
{
if (n < 31 && ((bs_info << n) & 0x40000000)) {


n *= 2;
div += 1;
parse_bs_info(bs_info, n + 1, div, div_blocks, num_blocks);
parse_bs_info(bs_info, n + 2, div, div_blocks, num_blocks);
} else {


**div_blocks = div;
(*div_blocks)++;
(*num_blocks)++;
}
}




static int32_t decode_rice(GetBitContext *gb, unsigned int k)
{
int max = get_bits_left(gb) - k;
unsigned q = get_unary(gb, 0, max);
int r = k ? get_bits1(gb) : !(q & 1);

if (k > 1) {
q <<= (k - 1);
q += get_bits_long(gb, k - 1);
} else if (!k) {
q >>= 1;
}
return r ? q : ~q;
}




static void parcor_to_lpc(unsigned int k, const int32_t *par, int32_t *cof)
{
int i, j;

for (i = 0, j = k - 1; i < j; i++, j--) {
unsigned tmp1 = ((MUL64(par[k], cof[j]) + (1 << 19)) >> 20);
cof[j] += ((MUL64(par[k], cof[i]) + (1 << 19)) >> 20);
cof[i] += tmp1;
}
if (i == j)
cof[i] += ((MUL64(par[k], cof[j]) + (1 << 19)) >> 20);

cof[k] = par[k];
}






static void get_block_sizes(ALSDecContext *ctx, unsigned int *div_blocks,
uint32_t *bs_info)
{
ALSSpecificConfig *sconf = &ctx->sconf;
GetBitContext *gb = &ctx->gb;
unsigned int *ptr_div_blocks = div_blocks;
unsigned int b;

if (sconf->block_switching) {
unsigned int bs_info_len = 1 << (sconf->block_switching + 2);
*bs_info = get_bits_long(gb, bs_info_len);
*bs_info <<= (32 - bs_info_len);
}

ctx->num_blocks = 0;
parse_bs_info(*bs_info, 0, 0, &ptr_div_blocks, &ctx->num_blocks);
















for (b = 0; b < ctx->num_blocks; b++)
div_blocks[b] = ctx->sconf.frame_length >> div_blocks[b];

if (ctx->cur_frame_length != ctx->sconf.frame_length) {
unsigned int remaining = ctx->cur_frame_length;

for (b = 0; b < ctx->num_blocks; b++) {
if (remaining <= div_blocks[b]) {
div_blocks[b] = remaining;
ctx->num_blocks = b + 1;
break;
}

remaining -= div_blocks[b];
}
}
}




static int read_const_block_data(ALSDecContext *ctx, ALSBlockData *bd)
{
ALSSpecificConfig *sconf = &ctx->sconf;
AVCodecContext *avctx = ctx->avctx;
GetBitContext *gb = &ctx->gb;

if (bd->block_length <= 0)
return AVERROR_INVALIDDATA;

*bd->raw_samples = 0;
*bd->const_block = get_bits1(gb); 
bd->js_blocks = get_bits1(gb);


skip_bits(gb, 5);

if (*bd->const_block) {
unsigned int const_val_bits = sconf->floating ? 24 : avctx->bits_per_raw_sample;
*bd->raw_samples = get_sbits_long(gb, const_val_bits);
}


*bd->const_block = 1;

return 0;
}




static void decode_const_block_data(ALSDecContext *ctx, ALSBlockData *bd)
{
int smp = bd->block_length - 1;
int32_t val = *bd->raw_samples;
int32_t *dst = bd->raw_samples + 1;


for (; smp; smp--)
*dst++ = val;
}




static int read_var_block_data(ALSDecContext *ctx, ALSBlockData *bd)
{
ALSSpecificConfig *sconf = &ctx->sconf;
AVCodecContext *avctx = ctx->avctx;
GetBitContext *gb = &ctx->gb;
unsigned int k;
unsigned int s[8];
unsigned int sx[8];
unsigned int sub_blocks, log2_sub_blocks, sb_length;
unsigned int start = 0;
unsigned int opt_order;
int sb;
int32_t *quant_cof = bd->quant_cof;
int32_t *current_res;



*bd->const_block = 0;

*bd->opt_order = 1;
bd->js_blocks = get_bits1(gb);

opt_order = *bd->opt_order;


if (!sconf->bgmc && !sconf->sb_part) {
log2_sub_blocks = 0;
} else {
if (sconf->bgmc && sconf->sb_part)
log2_sub_blocks = get_bits(gb, 2);
else
log2_sub_blocks = 2 * get_bits1(gb);
}

sub_blocks = 1 << log2_sub_blocks;



if (bd->block_length & (sub_blocks - 1) || bd->block_length <= 0) {
av_log(avctx, AV_LOG_WARNING,
"Block length is not evenly divisible by the number of subblocks.\n");
return AVERROR_INVALIDDATA;
}

sb_length = bd->block_length >> log2_sub_blocks;

if (sconf->bgmc) {
s[0] = get_bits(gb, 8 + (sconf->resolution > 1));
for (k = 1; k < sub_blocks; k++)
s[k] = s[k - 1] + decode_rice(gb, 2);

for (k = 0; k < sub_blocks; k++) {
sx[k] = s[k] & 0x0F;
s [k] >>= 4;
}
} else {
s[0] = get_bits(gb, 4 + (sconf->resolution > 1));
for (k = 1; k < sub_blocks; k++)
s[k] = s[k - 1] + decode_rice(gb, 0);
}
for (k = 1; k < sub_blocks; k++)
if (s[k] > 32) {
av_log(avctx, AV_LOG_ERROR, "k invalid for rice code.\n");
return AVERROR_INVALIDDATA;
}

if (get_bits1(gb))
*bd->shift_lsbs = get_bits(gb, 4) + 1;

*bd->store_prev_samples = (bd->js_blocks && bd->raw_other) || *bd->shift_lsbs;


if (!sconf->rlslms) {
if (sconf->adapt_order && sconf->max_order) {
int opt_order_length = av_ceil_log2(av_clip((bd->block_length >> 3) - 1,
2, sconf->max_order + 1));
*bd->opt_order = get_bits(gb, opt_order_length);
if (*bd->opt_order > sconf->max_order) {
*bd->opt_order = sconf->max_order;
av_log(avctx, AV_LOG_ERROR, "Predictor order too large.\n");
return AVERROR_INVALIDDATA;
}
} else {
*bd->opt_order = sconf->max_order;
}
opt_order = *bd->opt_order;

if (opt_order) {
int add_base;

if (sconf->coef_table == 3) {
add_base = 0x7F;


quant_cof[0] = 32 * parcor_scaled_values[get_bits(gb, 7)];


if (opt_order > 1)
quant_cof[1] = -32 * parcor_scaled_values[get_bits(gb, 7)];


for (k = 2; k < opt_order; k++)
quant_cof[k] = get_bits(gb, 7);
} else {
int k_max;
add_base = 1;


k_max = FFMIN(opt_order, 20);
for (k = 0; k < k_max; k++) {
int rice_param = parcor_rice_table[sconf->coef_table][k][1];
int offset = parcor_rice_table[sconf->coef_table][k][0];
quant_cof[k] = decode_rice(gb, rice_param) + offset;
if (quant_cof[k] < -64 || quant_cof[k] > 63) {
av_log(avctx, AV_LOG_ERROR,
"quant_cof %"PRId32" is out of range.\n",
quant_cof[k]);
return AVERROR_INVALIDDATA;
}
}


k_max = FFMIN(opt_order, 127);
for (; k < k_max; k++)
quant_cof[k] = decode_rice(gb, 2) + (k & 1);


for (; k < opt_order; k++)
quant_cof[k] = decode_rice(gb, 1);

quant_cof[0] = 32 * parcor_scaled_values[quant_cof[0] + 64];

if (opt_order > 1)
quant_cof[1] = -32 * parcor_scaled_values[quant_cof[1] + 64];
}

for (k = 2; k < opt_order; k++)
quant_cof[k] = (quant_cof[k] * (1 << 14)) + (add_base << 13);
}
}


if (sconf->long_term_prediction) {
*bd->use_ltp = get_bits1(gb);

if (*bd->use_ltp) {
int r, c;

bd->ltp_gain[0] = decode_rice(gb, 1) * 8;
bd->ltp_gain[1] = decode_rice(gb, 2) * 8;

r = get_unary(gb, 0, 4);
c = get_bits(gb, 2);
if (r >= 4) {
av_log(avctx, AV_LOG_ERROR, "r overflow\n");
return AVERROR_INVALIDDATA;
}

bd->ltp_gain[2] = ltp_gain_values[r][c];

bd->ltp_gain[3] = decode_rice(gb, 2) * 8;
bd->ltp_gain[4] = decode_rice(gb, 1) * 8;

*bd->ltp_lag = get_bits(gb, ctx->ltp_lag_length);
*bd->ltp_lag += FFMAX(4, opt_order + 1);
}
}


if (bd->ra_block) {
start = FFMIN(opt_order, 3);
av_assert0(sb_length <= sconf->frame_length);
if (sb_length <= start) {

av_log(avctx, AV_LOG_ERROR, "Sub block length smaller or equal start\n");
return AVERROR_PATCHWELCOME;
}

if (opt_order)
bd->raw_samples[0] = decode_rice(gb, avctx->bits_per_raw_sample - 4);
if (opt_order > 1)
bd->raw_samples[1] = decode_rice(gb, FFMIN(s[0] + 3, ctx->s_max));
if (opt_order > 2)
bd->raw_samples[2] = decode_rice(gb, FFMIN(s[0] + 1, ctx->s_max));
}


if (sconf->bgmc) {
int delta[8];
unsigned int k [8];
unsigned int b = av_clip((av_ceil_log2(bd->block_length) - 3) >> 1, 0, 5);


unsigned int high;
unsigned int low;
unsigned int value;

int ret = ff_bgmc_decode_init(gb, &high, &low, &value);
if (ret < 0)
return ret;

current_res = bd->raw_samples + start;

for (sb = 0; sb < sub_blocks; sb++) {
unsigned int sb_len = sb_length - (sb ? 0 : start);

k [sb] = s[sb] > b ? s[sb] - b : 0;
delta[sb] = 5 - s[sb] + k[sb];

if (k[sb] >= 32)
return AVERROR_INVALIDDATA;

ff_bgmc_decode(gb, sb_len, current_res,
delta[sb], sx[sb], &high, &low, &value, ctx->bgmc_lut, ctx->bgmc_lut_status);

current_res += sb_len;
}

ff_bgmc_decode_end(gb);



current_res = bd->raw_samples + start;

for (sb = 0; sb < sub_blocks; sb++, start = 0) {
unsigned int cur_tail_code = tail_code[sx[sb]][delta[sb]];
unsigned int cur_k = k[sb];
unsigned int cur_s = s[sb];

for (; start < sb_length; start++) {
int32_t res = *current_res;

if (res == cur_tail_code) {
unsigned int max_msb = (2 + (sx[sb] > 2) + (sx[sb] > 10))
<< (5 - delta[sb]);

res = decode_rice(gb, cur_s);

if (res >= 0) {
res += (max_msb ) << cur_k;
} else {
res -= (max_msb - 1) << cur_k;
}
} else {
if (res > cur_tail_code)
res--;

if (res & 1)
res = -res;

res >>= 1;

if (cur_k) {
res *= 1U << cur_k;
res |= get_bits_long(gb, cur_k);
}
}

*current_res++ = res;
}
}
} else {
current_res = bd->raw_samples + start;

for (sb = 0; sb < sub_blocks; sb++, start = 0)
for (; start < sb_length; start++)
*current_res++ = decode_rice(gb, s[sb]);
}

return 0;
}




static int decode_var_block_data(ALSDecContext *ctx, ALSBlockData *bd)
{
ALSSpecificConfig *sconf = &ctx->sconf;
unsigned int block_length = bd->block_length;
unsigned int smp = 0;
unsigned int k;
int opt_order = *bd->opt_order;
int sb;
int64_t y;
int32_t *quant_cof = bd->quant_cof;
int32_t *lpc_cof = bd->lpc_cof;
int32_t *raw_samples = bd->raw_samples;
int32_t *raw_samples_end = bd->raw_samples + bd->block_length;
int32_t *lpc_cof_reversed = ctx->lpc_cof_reversed_buffer;


if (*bd->use_ltp) {
int ltp_smp;

for (ltp_smp = FFMAX(*bd->ltp_lag - 2, 0); ltp_smp < block_length; ltp_smp++) {
int center = ltp_smp - *bd->ltp_lag;
int begin = FFMAX(0, center - 2);
int end = center + 3;
int tab = 5 - (end - begin);
int base;

y = 1 << 6;

for (base = begin; base < end; base++, tab++)
y += (uint64_t)MUL64(bd->ltp_gain[tab], raw_samples[base]);

raw_samples[ltp_smp] += y >> 7;
}
}


if (bd->ra_block) {
for (smp = 0; smp < FFMIN(opt_order, block_length); smp++) {
y = 1 << 19;

for (sb = 0; sb < smp; sb++)
y += (uint64_t)MUL64(lpc_cof[sb], raw_samples[-(sb + 1)]);

*raw_samples++ -= y >> 20;
parcor_to_lpc(smp, quant_cof, lpc_cof);
}
} else {
for (k = 0; k < opt_order; k++)
parcor_to_lpc(k, quant_cof, lpc_cof);


if (*bd->store_prev_samples)
memcpy(bd->prev_raw_samples, raw_samples - sconf->max_order,
sizeof(*bd->prev_raw_samples) * sconf->max_order);


if (bd->js_blocks && bd->raw_other) {
uint32_t *left, *right;

if (bd->raw_other > raw_samples) { 
left = raw_samples;
right = bd->raw_other;
} else { 
left = bd->raw_other;
right = raw_samples;
}

for (sb = -1; sb >= -sconf->max_order; sb--)
raw_samples[sb] = right[sb] - left[sb];
}


if (*bd->shift_lsbs)
for (sb = -1; sb >= -sconf->max_order; sb--)
raw_samples[sb] >>= *bd->shift_lsbs;
}


lpc_cof = lpc_cof + opt_order;

for (sb = 0; sb < opt_order; sb++)
lpc_cof_reversed[sb] = lpc_cof[-(sb + 1)];


raw_samples = bd->raw_samples + smp;
lpc_cof = lpc_cof_reversed + opt_order;

for (; raw_samples < raw_samples_end; raw_samples++) {
y = 1 << 19;

for (sb = -opt_order; sb < 0; sb++)
y += (uint64_t)MUL64(lpc_cof[sb], raw_samples[sb]);

*raw_samples -= y >> 20;
}

raw_samples = bd->raw_samples;


if (*bd->store_prev_samples)
memcpy(raw_samples - sconf->max_order, bd->prev_raw_samples,
sizeof(*raw_samples) * sconf->max_order);

return 0;
}




static int read_block(ALSDecContext *ctx, ALSBlockData *bd)
{
int ret;
GetBitContext *gb = &ctx->gb;
ALSSpecificConfig *sconf = &ctx->sconf;

*bd->shift_lsbs = 0;

if (get_bits1(gb)) {
ret = read_var_block_data(ctx, bd);
} else {
ret = read_const_block_data(ctx, bd);
}

if (!sconf->mc_coding || ctx->js_switch)
align_get_bits(gb);

return ret;
}




static int decode_block(ALSDecContext *ctx, ALSBlockData *bd)
{
unsigned int smp;
int ret = 0;


if (*bd->const_block)
decode_const_block_data(ctx, bd);
else
ret = decode_var_block_data(ctx, bd); 

if (ret < 0)
return ret;



if (*bd->shift_lsbs)
for (smp = 0; smp < bd->block_length; smp++)
bd->raw_samples[smp] = (unsigned)bd->raw_samples[smp] << *bd->shift_lsbs;

return 0;
}




static int read_decode_block(ALSDecContext *ctx, ALSBlockData *bd)
{
int ret;

if ((ret = read_block(ctx, bd)) < 0)
return ret;

return decode_block(ctx, bd);
}





static void zero_remaining(unsigned int b, unsigned int b_max,
const unsigned int *div_blocks, int32_t *buf)
{
unsigned int count = 0;

while (b < b_max)
count += div_blocks[b++];

if (count)
memset(buf, 0, sizeof(*buf) * count);
}




static int decode_blocks_ind(ALSDecContext *ctx, unsigned int ra_frame,
unsigned int c, const unsigned int *div_blocks,
unsigned int *js_blocks)
{
int ret;
unsigned int b;
ALSBlockData bd = { 0 };

bd.ra_block = ra_frame;
bd.const_block = ctx->const_block;
bd.shift_lsbs = ctx->shift_lsbs;
bd.opt_order = ctx->opt_order;
bd.store_prev_samples = ctx->store_prev_samples;
bd.use_ltp = ctx->use_ltp;
bd.ltp_lag = ctx->ltp_lag;
bd.ltp_gain = ctx->ltp_gain[0];
bd.quant_cof = ctx->quant_cof[0];
bd.lpc_cof = ctx->lpc_cof[0];
bd.prev_raw_samples = ctx->prev_raw_samples;
bd.raw_samples = ctx->raw_samples[c];


for (b = 0; b < ctx->num_blocks; b++) {
bd.block_length = div_blocks[b];

if ((ret = read_decode_block(ctx, &bd)) < 0) {

zero_remaining(b, ctx->num_blocks, div_blocks, bd.raw_samples);
return ret;
}
bd.raw_samples += div_blocks[b];
bd.ra_block = 0;
}

return 0;
}




static int decode_blocks(ALSDecContext *ctx, unsigned int ra_frame,
unsigned int c, const unsigned int *div_blocks,
unsigned int *js_blocks)
{
ALSSpecificConfig *sconf = &ctx->sconf;
unsigned int offset = 0;
unsigned int b;
int ret;
ALSBlockData bd[2] = { { 0 } };

bd[0].ra_block = ra_frame;
bd[0].const_block = ctx->const_block;
bd[0].shift_lsbs = ctx->shift_lsbs;
bd[0].opt_order = ctx->opt_order;
bd[0].store_prev_samples = ctx->store_prev_samples;
bd[0].use_ltp = ctx->use_ltp;
bd[0].ltp_lag = ctx->ltp_lag;
bd[0].ltp_gain = ctx->ltp_gain[0];
bd[0].quant_cof = ctx->quant_cof[0];
bd[0].lpc_cof = ctx->lpc_cof[0];
bd[0].prev_raw_samples = ctx->prev_raw_samples;
bd[0].js_blocks = *js_blocks;

bd[1].ra_block = ra_frame;
bd[1].const_block = ctx->const_block;
bd[1].shift_lsbs = ctx->shift_lsbs;
bd[1].opt_order = ctx->opt_order;
bd[1].store_prev_samples = ctx->store_prev_samples;
bd[1].use_ltp = ctx->use_ltp;
bd[1].ltp_lag = ctx->ltp_lag;
bd[1].ltp_gain = ctx->ltp_gain[0];
bd[1].quant_cof = ctx->quant_cof[0];
bd[1].lpc_cof = ctx->lpc_cof[0];
bd[1].prev_raw_samples = ctx->prev_raw_samples;
bd[1].js_blocks = *(js_blocks + 1);


for (b = 0; b < ctx->num_blocks; b++) {
unsigned int s;

bd[0].block_length = div_blocks[b];
bd[1].block_length = div_blocks[b];

bd[0].raw_samples = ctx->raw_samples[c ] + offset;
bd[1].raw_samples = ctx->raw_samples[c + 1] + offset;

bd[0].raw_other = bd[1].raw_samples;
bd[1].raw_other = bd[0].raw_samples;

if ((ret = read_decode_block(ctx, &bd[0])) < 0 ||
(ret = read_decode_block(ctx, &bd[1])) < 0)
goto fail;


if (bd[0].js_blocks) {
if (bd[1].js_blocks)
av_log(ctx->avctx, AV_LOG_WARNING, "Invalid channel pair.\n");

for (s = 0; s < div_blocks[b]; s++)
bd[0].raw_samples[s] = bd[1].raw_samples[s] - (unsigned)bd[0].raw_samples[s];
} else if (bd[1].js_blocks) {
for (s = 0; s < div_blocks[b]; s++)
bd[1].raw_samples[s] = bd[1].raw_samples[s] + (unsigned)bd[0].raw_samples[s];
}

offset += div_blocks[b];
bd[0].ra_block = 0;
bd[1].ra_block = 0;
}



memmove(ctx->raw_samples[c] - sconf->max_order,
ctx->raw_samples[c] - sconf->max_order + sconf->frame_length,
sizeof(*ctx->raw_samples[c]) * sconf->max_order);

return 0;
fail:

zero_remaining(b, ctx->num_blocks, div_blocks, bd[0].raw_samples);
zero_remaining(b, ctx->num_blocks, div_blocks, bd[1].raw_samples);
return ret;
}

static inline int als_weighting(GetBitContext *gb, int k, int off)
{
int idx = av_clip(decode_rice(gb, k) + off,
0, FF_ARRAY_ELEMS(mcc_weightings) - 1);
return mcc_weightings[idx];
}



static int read_channel_data(ALSDecContext *ctx, ALSChannelData *cd, int c)
{
GetBitContext *gb = &ctx->gb;
ALSChannelData *current = cd;
unsigned int channels = ctx->avctx->channels;
int entries = 0;

while (entries < channels && !(current->stop_flag = get_bits1(gb))) {
current->master_channel = get_bits_long(gb, av_ceil_log2(channels));

if (current->master_channel >= channels) {
av_log(ctx->avctx, AV_LOG_ERROR, "Invalid master channel.\n");
return AVERROR_INVALIDDATA;
}

if (current->master_channel != c) {
current->time_diff_flag = get_bits1(gb);
current->weighting[0] = als_weighting(gb, 1, 16);
current->weighting[1] = als_weighting(gb, 2, 14);
current->weighting[2] = als_weighting(gb, 1, 16);

if (current->time_diff_flag) {
current->weighting[3] = als_weighting(gb, 1, 16);
current->weighting[4] = als_weighting(gb, 1, 16);
current->weighting[5] = als_weighting(gb, 1, 16);

current->time_diff_sign = get_bits1(gb);
current->time_diff_index = get_bits(gb, ctx->ltp_lag_length - 3) + 3;
}
}

current++;
entries++;
}

if (entries == channels) {
av_log(ctx->avctx, AV_LOG_ERROR, "Damaged channel data.\n");
return AVERROR_INVALIDDATA;
}

align_get_bits(gb);
return 0;
}




static int revert_channel_correlation(ALSDecContext *ctx, ALSBlockData *bd,
ALSChannelData **cd, int *reverted,
unsigned int offset, int c)
{
ALSChannelData *ch = cd[c];
unsigned int dep = 0;
unsigned int channels = ctx->avctx->channels;
unsigned int channel_size = ctx->sconf.frame_length + ctx->sconf.max_order;

if (reverted[c])
return 0;

reverted[c] = 1;

while (dep < channels && !ch[dep].stop_flag) {
revert_channel_correlation(ctx, bd, cd, reverted, offset,
ch[dep].master_channel);

dep++;
}

if (dep == channels) {
av_log(ctx->avctx, AV_LOG_WARNING, "Invalid channel correlation.\n");
return AVERROR_INVALIDDATA;
}

bd->const_block = ctx->const_block + c;
bd->shift_lsbs = ctx->shift_lsbs + c;
bd->opt_order = ctx->opt_order + c;
bd->store_prev_samples = ctx->store_prev_samples + c;
bd->use_ltp = ctx->use_ltp + c;
bd->ltp_lag = ctx->ltp_lag + c;
bd->ltp_gain = ctx->ltp_gain[c];
bd->lpc_cof = ctx->lpc_cof[c];
bd->quant_cof = ctx->quant_cof[c];
bd->raw_samples = ctx->raw_samples[c] + offset;

for (dep = 0; !ch[dep].stop_flag; dep++) {
ptrdiff_t smp;
ptrdiff_t begin = 1;
ptrdiff_t end = bd->block_length - 1;
int64_t y;
int32_t *master = ctx->raw_samples[ch[dep].master_channel] + offset;

if (ch[dep].master_channel == c)
continue;

if (ch[dep].time_diff_flag) {
int t = ch[dep].time_diff_index;

if (ch[dep].time_diff_sign) {
t = -t;
if (begin < t) {
av_log(ctx->avctx, AV_LOG_ERROR, "begin %"PTRDIFF_SPECIFIER" smaller than time diff index %d.\n", begin, t);
return AVERROR_INVALIDDATA;
}
begin -= t;
} else {
if (end < t) {
av_log(ctx->avctx, AV_LOG_ERROR, "end %"PTRDIFF_SPECIFIER" smaller than time diff index %d.\n", end, t);
return AVERROR_INVALIDDATA;
}
end -= t;
}

if (FFMIN(begin - 1, begin - 1 + t) < ctx->raw_buffer - master ||
FFMAX(end + 1, end + 1 + t) > ctx->raw_buffer + channels * channel_size - master) {
av_log(ctx->avctx, AV_LOG_ERROR,
"sample pointer range [%p, %p] not contained in raw_buffer [%p, %p].\n",
master + FFMIN(begin - 1, begin - 1 + t), master + FFMAX(end + 1, end + 1 + t),
ctx->raw_buffer, ctx->raw_buffer + channels * channel_size);
return AVERROR_INVALIDDATA;
}

for (smp = begin; smp < end; smp++) {
y = (1 << 6) +
MUL64(ch[dep].weighting[0], master[smp - 1 ]) +
MUL64(ch[dep].weighting[1], master[smp ]) +
MUL64(ch[dep].weighting[2], master[smp + 1 ]) +
MUL64(ch[dep].weighting[3], master[smp - 1 + t]) +
MUL64(ch[dep].weighting[4], master[smp + t]) +
MUL64(ch[dep].weighting[5], master[smp + 1 + t]);

bd->raw_samples[smp] += y >> 7;
}
} else {

if (begin - 1 < ctx->raw_buffer - master ||
end + 1 > ctx->raw_buffer + channels * channel_size - master) {
av_log(ctx->avctx, AV_LOG_ERROR,
"sample pointer range [%p, %p] not contained in raw_buffer [%p, %p].\n",
master + begin - 1, master + end + 1,
ctx->raw_buffer, ctx->raw_buffer + channels * channel_size);
return AVERROR_INVALIDDATA;
}

for (smp = begin; smp < end; smp++) {
y = (1 << 6) +
MUL64(ch[dep].weighting[0], master[smp - 1]) +
MUL64(ch[dep].weighting[1], master[smp ]) +
MUL64(ch[dep].weighting[2], master[smp + 1]);

bd->raw_samples[smp] += y >> 7;
}
}
}

return 0;
}




static SoftFloat_IEEE754 multiply(SoftFloat_IEEE754 a, SoftFloat_IEEE754 b) {
uint64_t mantissa_temp;
uint64_t mask_64;
int cutoff_bit_count;
unsigned char last_2_bits;
unsigned int mantissa;
int32_t sign;
uint32_t return_val = 0;
int bit_count = 48;

sign = a.sign ^ b.sign;


mantissa_temp = (uint64_t)a.mant * (uint64_t)b.mant;
mask_64 = (uint64_t)0x1 << 47;

if (!mantissa_temp)
return FLOAT_0;


while (!(mantissa_temp & mask_64) && mask_64) {
bit_count--;
mask_64 >>= 1;
}


cutoff_bit_count = bit_count - 24;
if (cutoff_bit_count > 0) {
last_2_bits = (unsigned char)(((unsigned int)mantissa_temp >> (cutoff_bit_count - 1)) & 0x3 );
if ((last_2_bits == 0x3) || ((last_2_bits == 0x1) && ((unsigned int)mantissa_temp & ((0x1UL << (cutoff_bit_count - 1)) - 1)))) {

mantissa_temp += (uint64_t)0x1 << cutoff_bit_count;
}
}

if (cutoff_bit_count >= 0) {
mantissa = (unsigned int)(mantissa_temp >> cutoff_bit_count);
} else {
mantissa = (unsigned int)(mantissa_temp <<-cutoff_bit_count);
}


if (mantissa & 0x01000000ul) {
bit_count++;
mantissa >>= 1;
}

if (!sign) {
return_val = 0x80000000U;
}

return_val |= ((unsigned)av_clip(a.exp + b.exp + bit_count - 47, -126, 127) << 23) & 0x7F800000;
return_val |= mantissa;
return av_bits2sf_ieee754(return_val);
}




static int read_diff_float_data(ALSDecContext *ctx, unsigned int ra_frame) {
AVCodecContext *avctx = ctx->avctx;
GetBitContext *gb = &ctx->gb;
SoftFloat_IEEE754 *acf = ctx->acf;
int *shift_value = ctx->shift_value;
int *last_shift_value = ctx->last_shift_value;
int *last_acf_mantissa = ctx->last_acf_mantissa;
int **raw_mantissa = ctx->raw_mantissa;
int *nbits = ctx->nbits;
unsigned char *larray = ctx->larray;
int frame_length = ctx->cur_frame_length;
SoftFloat_IEEE754 scale = av_int2sf_ieee754(0x1u, 23);
unsigned int partA_flag;
unsigned int highest_byte;
unsigned int shift_amp;
uint32_t tmp_32;
int use_acf;
int nchars;
int i;
int c;
long k;
long nbits_aligned;
unsigned long acc;
unsigned long j;
uint32_t sign;
uint32_t e;
uint32_t mantissa;

skip_bits_long(gb, 32); 
use_acf = get_bits1(gb);

if (ra_frame) {
memset(last_acf_mantissa, 0, avctx->channels * sizeof(*last_acf_mantissa));
memset(last_shift_value, 0, avctx->channels * sizeof(*last_shift_value) );
ff_mlz_flush_dict(ctx->mlz);
}

if (avctx->channels * 8 > get_bits_left(gb))
return AVERROR_INVALIDDATA;

for (c = 0; c < avctx->channels; ++c) {
if (use_acf) {

if (get_bits1(gb)) {
tmp_32 = get_bits(gb, 23);
last_acf_mantissa[c] = tmp_32;
} else {
tmp_32 = last_acf_mantissa[c];
}
acf[c] = av_bits2sf_ieee754(tmp_32);
} else {
acf[c] = FLOAT_1;
}

highest_byte = get_bits(gb, 2);
partA_flag = get_bits1(gb);
shift_amp = get_bits1(gb);

if (shift_amp) {
shift_value[c] = get_bits(gb, 8);
last_shift_value[c] = shift_value[c];
} else {
shift_value[c] = last_shift_value[c];
}

if (partA_flag) {
if (!get_bits1(gb)) { 
for (i = 0; i < frame_length; ++i) {
if (ctx->raw_samples[c][i] == 0) {
ctx->raw_mantissa[c][i] = get_bits_long(gb, 32);
}
}
} else { 
nchars = 0;
for (i = 0; i < frame_length; ++i) {
if (ctx->raw_samples[c][i] == 0) {
nchars += 4;
}
}

tmp_32 = ff_mlz_decompression(ctx->mlz, gb, nchars, larray);
if(tmp_32 != nchars) {
av_log(ctx->avctx, AV_LOG_ERROR, "Error in MLZ decompression (%"PRId32", %d).\n", tmp_32, nchars);
return AVERROR_INVALIDDATA;
}

for (i = 0; i < frame_length; ++i) {
ctx->raw_mantissa[c][i] = AV_RB32(larray);
}
}
}


if (highest_byte) {
for (i = 0; i < frame_length; ++i) {
if (ctx->raw_samples[c][i] != 0) {

if (av_cmp_sf_ieee754(acf[c], FLOAT_1)) {
nbits[i] = 23 - av_log2(abs(ctx->raw_samples[c][i]));
} else {
nbits[i] = 23;
}
nbits[i] = FFMIN(nbits[i], highest_byte*8);
}
}

if (!get_bits1(gb)) { 
for (i = 0; i < frame_length; ++i) {
if (ctx->raw_samples[c][i] != 0) {
raw_mantissa[c][i] = get_bitsz(gb, nbits[i]);
}
}
} else { 
nchars = 0;
for (i = 0; i < frame_length; ++i) {
if (ctx->raw_samples[c][i]) {
nchars += (int) nbits[i] / 8;
if (nbits[i] & 7) {
++nchars;
}
}
}

tmp_32 = ff_mlz_decompression(ctx->mlz, gb, nchars, larray);
if(tmp_32 != nchars) {
av_log(ctx->avctx, AV_LOG_ERROR, "Error in MLZ decompression (%"PRId32", %d).\n", tmp_32, nchars);
return AVERROR_INVALIDDATA;
}

j = 0;
for (i = 0; i < frame_length; ++i) {
if (ctx->raw_samples[c][i]) {
if (nbits[i] & 7) {
nbits_aligned = 8 * ((unsigned int)(nbits[i] / 8) + 1);
} else {
nbits_aligned = nbits[i];
}
acc = 0;
for (k = 0; k < nbits_aligned/8; ++k) {
acc = (acc << 8) + larray[j++];
}
acc >>= (nbits_aligned - nbits[i]);
raw_mantissa[c][i] = acc;
}
}
}
}

for (i = 0; i < frame_length; ++i) {
SoftFloat_IEEE754 pcm_sf = av_int2sf_ieee754(ctx->raw_samples[c][i], 0);
pcm_sf = av_div_sf_ieee754(pcm_sf, scale);

if (ctx->raw_samples[c][i] != 0) {
if (!av_cmp_sf_ieee754(acf[c], FLOAT_1)) {
pcm_sf = multiply(acf[c], pcm_sf);
}

sign = pcm_sf.sign;
e = pcm_sf.exp;
mantissa = (pcm_sf.mant | 0x800000) + raw_mantissa[c][i];

while(mantissa >= 0x1000000) {
e++;
mantissa >>= 1;
}

if (mantissa) e += (shift_value[c] - 127);
mantissa &= 0x007fffffUL;

tmp_32 = (sign << 31) | ((e + EXP_BIAS) << 23) | (mantissa);
ctx->raw_samples[c][i] = tmp_32;
} else {
ctx->raw_samples[c][i] = raw_mantissa[c][i] & 0x007fffffUL;
}
}
align_get_bits(gb);
}
return 0;
}




static int read_frame_data(ALSDecContext *ctx, unsigned int ra_frame)
{
ALSSpecificConfig *sconf = &ctx->sconf;
AVCodecContext *avctx = ctx->avctx;
GetBitContext *gb = &ctx->gb;
unsigned int div_blocks[32]; 
unsigned int c;
unsigned int js_blocks[2];
uint32_t bs_info = 0;
int ret;


if (sconf->ra_flag == RA_FLAG_FRAMES && ra_frame)
skip_bits_long(gb, 32);

if (sconf->mc_coding && sconf->joint_stereo) {
ctx->js_switch = get_bits1(gb);
align_get_bits(gb);
}

if (!sconf->mc_coding || ctx->js_switch) {
int independent_bs = !sconf->joint_stereo;

for (c = 0; c < avctx->channels; c++) {
js_blocks[0] = 0;
js_blocks[1] = 0;

get_block_sizes(ctx, div_blocks, &bs_info);



if (sconf->joint_stereo && sconf->block_switching)
if (bs_info >> 31)
independent_bs = 2;


if (c == avctx->channels - 1 || (c & 1))
independent_bs = 1;

if (independent_bs) {
ret = decode_blocks_ind(ctx, ra_frame, c,
div_blocks, js_blocks);
if (ret < 0)
return ret;
independent_bs--;
} else {
ret = decode_blocks(ctx, ra_frame, c, div_blocks, js_blocks);
if (ret < 0)
return ret;

c++;
}


memmove(ctx->raw_samples[c] - sconf->max_order,
ctx->raw_samples[c] - sconf->max_order + sconf->frame_length,
sizeof(*ctx->raw_samples[c]) * sconf->max_order);
ctx->highest_decoded_channel = c;
}
} else { 
ALSBlockData bd = { 0 };
int b, ret;
int *reverted_channels = ctx->reverted_channels;
unsigned int offset = 0;

for (c = 0; c < avctx->channels; c++)
if (ctx->chan_data[c] < ctx->chan_data_buffer) {
av_log(ctx->avctx, AV_LOG_ERROR, "Invalid channel data.\n");
return AVERROR_INVALIDDATA;
}

memset(reverted_channels, 0, sizeof(*reverted_channels) * avctx->channels);

bd.ra_block = ra_frame;
bd.prev_raw_samples = ctx->prev_raw_samples;

get_block_sizes(ctx, div_blocks, &bs_info);

for (b = 0; b < ctx->num_blocks; b++) {
bd.block_length = div_blocks[b];
if (bd.block_length <= 0) {
av_log(ctx->avctx, AV_LOG_WARNING,
"Invalid block length %u in channel data!\n",
bd.block_length);
continue;
}

for (c = 0; c < avctx->channels; c++) {
bd.const_block = ctx->const_block + c;
bd.shift_lsbs = ctx->shift_lsbs + c;
bd.opt_order = ctx->opt_order + c;
bd.store_prev_samples = ctx->store_prev_samples + c;
bd.use_ltp = ctx->use_ltp + c;
bd.ltp_lag = ctx->ltp_lag + c;
bd.ltp_gain = ctx->ltp_gain[c];
bd.lpc_cof = ctx->lpc_cof[c];
bd.quant_cof = ctx->quant_cof[c];
bd.raw_samples = ctx->raw_samples[c] + offset;
bd.raw_other = NULL;

if ((ret = read_block(ctx, &bd)) < 0)
return ret;
if ((ret = read_channel_data(ctx, ctx->chan_data[c], c)) < 0)
return ret;
}

for (c = 0; c < avctx->channels; c++) {
ret = revert_channel_correlation(ctx, &bd, ctx->chan_data,
reverted_channels, offset, c);
if (ret < 0)
return ret;
}
for (c = 0; c < avctx->channels; c++) {
bd.const_block = ctx->const_block + c;
bd.shift_lsbs = ctx->shift_lsbs + c;
bd.opt_order = ctx->opt_order + c;
bd.store_prev_samples = ctx->store_prev_samples + c;
bd.use_ltp = ctx->use_ltp + c;
bd.ltp_lag = ctx->ltp_lag + c;
bd.ltp_gain = ctx->ltp_gain[c];
bd.lpc_cof = ctx->lpc_cof[c];
bd.quant_cof = ctx->quant_cof[c];
bd.raw_samples = ctx->raw_samples[c] + offset;

if ((ret = decode_block(ctx, &bd)) < 0)
return ret;

ctx->highest_decoded_channel = FFMAX(ctx->highest_decoded_channel, c);
}

memset(reverted_channels, 0, avctx->channels * sizeof(*reverted_channels));
offset += div_blocks[b];
bd.ra_block = 0;
}


for (c = 0; c < avctx->channels; c++)
memmove(ctx->raw_samples[c] - sconf->max_order,
ctx->raw_samples[c] - sconf->max_order + sconf->frame_length,
sizeof(*ctx->raw_samples[c]) * sconf->max_order);
}

if (sconf->floating) {
read_diff_float_data(ctx, ra_frame);
}

if (get_bits_left(gb) < 0) {
av_log(ctx->avctx, AV_LOG_ERROR, "Overread %d\n", -get_bits_left(gb));
return AVERROR_INVALIDDATA;
}

return 0;
}




static int decode_frame(AVCodecContext *avctx, void *data, int *got_frame_ptr,
AVPacket *avpkt)
{
ALSDecContext *ctx = avctx->priv_data;
AVFrame *frame = data;
ALSSpecificConfig *sconf = &ctx->sconf;
const uint8_t *buffer = avpkt->data;
int buffer_size = avpkt->size;
int invalid_frame, ret;
unsigned int c, sample, ra_frame, bytes_read, shift;

if ((ret = init_get_bits8(&ctx->gb, buffer, buffer_size)) < 0)
return ret;





ra_frame = sconf->ra_distance && !(ctx->frame_id % sconf->ra_distance);


if (sconf->samples != 0xFFFFFFFF)
ctx->cur_frame_length = FFMIN(sconf->samples - ctx->frame_id * (uint64_t) sconf->frame_length,
sconf->frame_length);
else
ctx->cur_frame_length = sconf->frame_length;

ctx->highest_decoded_channel = 0;

if ((invalid_frame = read_frame_data(ctx, ra_frame)) < 0)
av_log(ctx->avctx, AV_LOG_WARNING,
"Reading frame data failed. Skipping RA unit.\n");

if (ctx->highest_decoded_channel == 0)
return AVERROR_INVALIDDATA;

ctx->frame_id++;


frame->nb_samples = ctx->cur_frame_length;
if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;


#define INTERLEAVE_OUTPUT(bps) { int##bps##_t *dest = (int##bps##_t*)frame->data[0]; int channels = avctx->channels; int32_t *raw_samples = ctx->raw_samples[0]; int raw_step = channels > 1 ? ctx->raw_samples[1] - raw_samples : 1; shift = bps - ctx->avctx->bits_per_raw_sample; if (!ctx->cs_switch) { for (sample = 0; sample < ctx->cur_frame_length; sample++) for (c = 0; c < channels; c++) *dest++ = raw_samples[c*raw_step + sample] * (1U << shift); } else { for (sample = 0; sample < ctx->cur_frame_length; sample++) for (c = 0; c < channels; c++) *dest++ = raw_samples[sconf->chan_pos[c]*raw_step + sample] * (1U << shift);} }

















if (ctx->avctx->bits_per_raw_sample <= 16) {
INTERLEAVE_OUTPUT(16)
} else {
INTERLEAVE_OUTPUT(32)
}


if (sconf->crc_enabled && (avctx->err_recognition & (AV_EF_CRCCHECK|AV_EF_CAREFUL))) {
int swap = HAVE_BIGENDIAN != sconf->msb_first;

if (ctx->avctx->bits_per_raw_sample == 24) {
int32_t *src = (int32_t *)frame->data[0];

for (sample = 0;
sample < ctx->cur_frame_length * avctx->channels;
sample++) {
int32_t v;

if (swap)
v = av_bswap32(src[sample]);
else
v = src[sample];
if (!HAVE_BIGENDIAN)
v >>= 8;

ctx->crc = av_crc(ctx->crc_table, ctx->crc, (uint8_t*)(&v), 3);
}
} else {
uint8_t *crc_source;

if (swap) {
if (ctx->avctx->bits_per_raw_sample <= 16) {
int16_t *src = (int16_t*) frame->data[0];
int16_t *dest = (int16_t*) ctx->crc_buffer;
for (sample = 0;
sample < ctx->cur_frame_length * avctx->channels;
sample++)
*dest++ = av_bswap16(src[sample]);
} else {
ctx->bdsp.bswap_buf((uint32_t *) ctx->crc_buffer,
(uint32_t *) frame->data[0],
ctx->cur_frame_length * avctx->channels);
}
crc_source = ctx->crc_buffer;
} else {
crc_source = frame->data[0];
}

ctx->crc = av_crc(ctx->crc_table, ctx->crc, crc_source,
ctx->cur_frame_length * avctx->channels *
av_get_bytes_per_sample(avctx->sample_fmt));
}



if (ctx->cur_frame_length != sconf->frame_length &&
ctx->crc_org != ctx->crc) {
av_log(avctx, AV_LOG_ERROR, "CRC error.\n");
if (avctx->err_recognition & AV_EF_EXPLODE)
return AVERROR_INVALIDDATA;
}
}

*got_frame_ptr = 1;

bytes_read = invalid_frame ? buffer_size :
(get_bits_count(&ctx->gb) + 7) >> 3;

return bytes_read;
}




static av_cold int decode_end(AVCodecContext *avctx)
{
ALSDecContext *ctx = avctx->priv_data;
int i;

av_freep(&ctx->sconf.chan_pos);

ff_bgmc_end(&ctx->bgmc_lut, &ctx->bgmc_lut_status);

av_freep(&ctx->const_block);
av_freep(&ctx->shift_lsbs);
av_freep(&ctx->opt_order);
av_freep(&ctx->store_prev_samples);
av_freep(&ctx->use_ltp);
av_freep(&ctx->ltp_lag);
av_freep(&ctx->ltp_gain);
av_freep(&ctx->ltp_gain_buffer);
av_freep(&ctx->quant_cof);
av_freep(&ctx->lpc_cof);
av_freep(&ctx->quant_cof_buffer);
av_freep(&ctx->lpc_cof_buffer);
av_freep(&ctx->lpc_cof_reversed_buffer);
av_freep(&ctx->prev_raw_samples);
av_freep(&ctx->raw_samples);
av_freep(&ctx->raw_buffer);
av_freep(&ctx->chan_data);
av_freep(&ctx->chan_data_buffer);
av_freep(&ctx->reverted_channels);
av_freep(&ctx->crc_buffer);
if (ctx->mlz) {
av_freep(&ctx->mlz->dict);
av_freep(&ctx->mlz);
}
av_freep(&ctx->acf);
av_freep(&ctx->last_acf_mantissa);
av_freep(&ctx->shift_value);
av_freep(&ctx->last_shift_value);
if (ctx->raw_mantissa) {
for (i = 0; i < avctx->channels; i++) {
av_freep(&ctx->raw_mantissa[i]);
}
av_freep(&ctx->raw_mantissa);
}
av_freep(&ctx->larray);
av_freep(&ctx->nbits);

return 0;
}




static av_cold int decode_init(AVCodecContext *avctx)
{
unsigned int c;
unsigned int channel_size;
int num_buffers, ret;
ALSDecContext *ctx = avctx->priv_data;
ALSSpecificConfig *sconf = &ctx->sconf;
ctx->avctx = avctx;

if (!avctx->extradata) {
av_log(avctx, AV_LOG_ERROR, "Missing required ALS extradata.\n");
return AVERROR_INVALIDDATA;
}

if ((ret = read_specific_config(ctx)) < 0) {
av_log(avctx, AV_LOG_ERROR, "Reading ALSSpecificConfig failed.\n");
goto fail;
}

if ((ret = check_specific_config(ctx)) < 0) {
goto fail;
}

if (sconf->bgmc) {
ret = ff_bgmc_init(avctx, &ctx->bgmc_lut, &ctx->bgmc_lut_status);
if (ret < 0)
goto fail;
}
if (sconf->floating) {
avctx->sample_fmt = AV_SAMPLE_FMT_FLT;
avctx->bits_per_raw_sample = 32;
} else {
avctx->sample_fmt = sconf->resolution > 1
? AV_SAMPLE_FMT_S32 : AV_SAMPLE_FMT_S16;
avctx->bits_per_raw_sample = (sconf->resolution + 1) * 8;
if (avctx->bits_per_raw_sample > 32) {
av_log(avctx, AV_LOG_ERROR, "Bits per raw sample %d larger than 32.\n",
avctx->bits_per_raw_sample);
ret = AVERROR_INVALIDDATA;
goto fail;
}
}




ctx->s_max = sconf->resolution > 1 ? 31 : 15;


ctx->ltp_lag_length = 8 + (avctx->sample_rate >= 96000) +
(avctx->sample_rate >= 192000);


num_buffers = sconf->mc_coding ? avctx->channels : 1;
if (num_buffers * (uint64_t)num_buffers > INT_MAX) 
return AVERROR_INVALIDDATA;

ctx->quant_cof = av_malloc_array(num_buffers, sizeof(*ctx->quant_cof));
ctx->lpc_cof = av_malloc_array(num_buffers, sizeof(*ctx->lpc_cof));
ctx->quant_cof_buffer = av_malloc_array(num_buffers * sconf->max_order,
sizeof(*ctx->quant_cof_buffer));
ctx->lpc_cof_buffer = av_malloc_array(num_buffers * sconf->max_order,
sizeof(*ctx->lpc_cof_buffer));
ctx->lpc_cof_reversed_buffer = av_malloc_array(sconf->max_order,
sizeof(*ctx->lpc_cof_buffer));

if (!ctx->quant_cof || !ctx->lpc_cof ||
!ctx->quant_cof_buffer || !ctx->lpc_cof_buffer ||
!ctx->lpc_cof_reversed_buffer) {
av_log(avctx, AV_LOG_ERROR, "Allocating buffer memory failed.\n");
ret = AVERROR(ENOMEM);
goto fail;
}


for (c = 0; c < num_buffers; c++) {
ctx->quant_cof[c] = ctx->quant_cof_buffer + c * sconf->max_order;
ctx->lpc_cof[c] = ctx->lpc_cof_buffer + c * sconf->max_order;
}


ctx->const_block = av_malloc_array(num_buffers, sizeof(*ctx->const_block));
ctx->shift_lsbs = av_malloc_array(num_buffers, sizeof(*ctx->shift_lsbs));
ctx->opt_order = av_malloc_array(num_buffers, sizeof(*ctx->opt_order));
ctx->store_prev_samples = av_malloc_array(num_buffers, sizeof(*ctx->store_prev_samples));
ctx->use_ltp = av_mallocz_array(num_buffers, sizeof(*ctx->use_ltp));
ctx->ltp_lag = av_malloc_array(num_buffers, sizeof(*ctx->ltp_lag));
ctx->ltp_gain = av_malloc_array(num_buffers, sizeof(*ctx->ltp_gain));
ctx->ltp_gain_buffer = av_malloc_array(num_buffers * 5, sizeof(*ctx->ltp_gain_buffer));

if (!ctx->const_block || !ctx->shift_lsbs ||
!ctx->opt_order || !ctx->store_prev_samples ||
!ctx->use_ltp || !ctx->ltp_lag ||
!ctx->ltp_gain || !ctx->ltp_gain_buffer) {
av_log(avctx, AV_LOG_ERROR, "Allocating buffer memory failed.\n");
ret = AVERROR(ENOMEM);
goto fail;
}

for (c = 0; c < num_buffers; c++)
ctx->ltp_gain[c] = ctx->ltp_gain_buffer + c * 5;


if (sconf->mc_coding) {
ctx->chan_data_buffer = av_mallocz_array(num_buffers * num_buffers,
sizeof(*ctx->chan_data_buffer));
ctx->chan_data = av_mallocz_array(num_buffers,
sizeof(*ctx->chan_data));
ctx->reverted_channels = av_malloc_array(num_buffers,
sizeof(*ctx->reverted_channels));

if (!ctx->chan_data_buffer || !ctx->chan_data || !ctx->reverted_channels) {
av_log(avctx, AV_LOG_ERROR, "Allocating buffer memory failed.\n");
ret = AVERROR(ENOMEM);
goto fail;
}

for (c = 0; c < num_buffers; c++)
ctx->chan_data[c] = ctx->chan_data_buffer + c * num_buffers;
} else {
ctx->chan_data = NULL;
ctx->chan_data_buffer = NULL;
ctx->reverted_channels = NULL;
}

channel_size = sconf->frame_length + sconf->max_order;

ctx->prev_raw_samples = av_malloc_array(sconf->max_order, sizeof(*ctx->prev_raw_samples));
ctx->raw_buffer = av_mallocz_array(avctx->channels * channel_size, sizeof(*ctx->raw_buffer));
ctx->raw_samples = av_malloc_array(avctx->channels, sizeof(*ctx->raw_samples));

if (sconf->floating) {
ctx->acf = av_malloc_array(avctx->channels, sizeof(*ctx->acf));
ctx->shift_value = av_malloc_array(avctx->channels, sizeof(*ctx->shift_value));
ctx->last_shift_value = av_malloc_array(avctx->channels, sizeof(*ctx->last_shift_value));
ctx->last_acf_mantissa = av_malloc_array(avctx->channels, sizeof(*ctx->last_acf_mantissa));
ctx->raw_mantissa = av_mallocz_array(avctx->channels, sizeof(*ctx->raw_mantissa));

ctx->larray = av_malloc_array(ctx->cur_frame_length * 4, sizeof(*ctx->larray));
ctx->nbits = av_malloc_array(ctx->cur_frame_length, sizeof(*ctx->nbits));
ctx->mlz = av_mallocz(sizeof(*ctx->mlz));

if (!ctx->mlz || !ctx->acf || !ctx->shift_value || !ctx->last_shift_value
|| !ctx->last_acf_mantissa || !ctx->raw_mantissa) {
av_log(avctx, AV_LOG_ERROR, "Allocating buffer memory failed.\n");
ret = AVERROR(ENOMEM);
goto fail;
}

ff_mlz_init_dict(avctx, ctx->mlz);
ff_mlz_flush_dict(ctx->mlz);

for (c = 0; c < avctx->channels; ++c) {
ctx->raw_mantissa[c] = av_mallocz_array(ctx->cur_frame_length, sizeof(**ctx->raw_mantissa));
}
}


if (!ctx->prev_raw_samples || !ctx->raw_buffer|| !ctx->raw_samples) {
av_log(avctx, AV_LOG_ERROR, "Allocating buffer memory failed.\n");
ret = AVERROR(ENOMEM);
goto fail;
}


ctx->raw_samples[0] = ctx->raw_buffer + sconf->max_order;
for (c = 1; c < avctx->channels; c++)
ctx->raw_samples[c] = ctx->raw_samples[c - 1] + channel_size;


if (HAVE_BIGENDIAN != sconf->msb_first && sconf->crc_enabled &&
(avctx->err_recognition & (AV_EF_CRCCHECK|AV_EF_CAREFUL))) {
ctx->crc_buffer = av_malloc_array(ctx->cur_frame_length *
avctx->channels *
av_get_bytes_per_sample(avctx->sample_fmt),
sizeof(*ctx->crc_buffer));
if (!ctx->crc_buffer) {
av_log(avctx, AV_LOG_ERROR, "Allocating buffer memory failed.\n");
ret = AVERROR(ENOMEM);
goto fail;
}
}

ff_bswapdsp_init(&ctx->bdsp);

return 0;

fail:
return ret;
}




static av_cold void flush(AVCodecContext *avctx)
{
ALSDecContext *ctx = avctx->priv_data;

ctx->frame_id = 0;
}


AVCodec ff_als_decoder = {
.name = "als",
.long_name = NULL_IF_CONFIG_SMALL("MPEG-4 Audio Lossless Coding (ALS)"),
.type = AVMEDIA_TYPE_AUDIO,
.id = AV_CODEC_ID_MP4ALS,
.priv_data_size = sizeof(ALSDecContext),
.init = decode_init,
.close = decode_end,
.decode = decode_frame,
.flush = flush,
.capabilities = AV_CODEC_CAP_SUBFRAMES | AV_CODEC_CAP_DR1,
.caps_internal = FF_CODEC_CAP_INIT_CLEANUP,
};
