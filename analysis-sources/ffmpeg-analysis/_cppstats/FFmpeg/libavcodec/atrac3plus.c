#include "libavutil/avassert.h"
#include "avcodec.h"
#include "get_bits.h"
#include "atrac3plus.h"
#include "atrac3plus_data.h"
static VLC_TYPE tables_data[154276][2];
static VLC wl_vlc_tabs[4];
static VLC sf_vlc_tabs[8];
static VLC ct_vlc_tabs[4];
static VLC spec_vlc_tabs[112];
static VLC gain_vlc_tabs[11];
static VLC tone_vlc_tabs[7];
static av_cold void build_canonical_huff(const uint8_t *cb, const uint8_t *xlat,
int *tab_offset, VLC *out_vlc)
{
int i, b;
uint16_t codes[256];
uint8_t bits[256];
unsigned code = 0;
int index = 0;
int min_len = *cb++; 
int max_len = *cb++; 
for (b = min_len; b <= max_len; b++) {
for (i = *cb++; i > 0; i--) {
av_assert0(index < 256);
bits[index] = b;
codes[index] = code++;
index++;
}
code <<= 1;
}
out_vlc->table = &tables_data[*tab_offset];
out_vlc->table_allocated = 1 << max_len;
ff_init_vlc_sparse(out_vlc, max_len, index, bits, 1, 1, codes, 2, 2,
xlat, 1, 1, INIT_VLC_USE_NEW_STATIC);
*tab_offset += 1 << max_len;
}
av_cold void ff_atrac3p_init_vlcs(void)
{
int i, wl_vlc_offs, ct_vlc_offs, sf_vlc_offs, tab_offset;
static const uint8_t wl_nb_bits[4] = { 2, 3, 5, 5 };
static const uint8_t wl_nb_codes[4] = { 3, 5, 8, 8 };
static const uint8_t * const wl_bits[4] = {
atrac3p_wl_huff_bits1, atrac3p_wl_huff_bits2,
atrac3p_wl_huff_bits3, atrac3p_wl_huff_bits4
};
static const uint8_t * const wl_codes[4] = {
atrac3p_wl_huff_code1, atrac3p_wl_huff_code2,
atrac3p_wl_huff_code3, atrac3p_wl_huff_code4
};
static const uint8_t * const wl_xlats[4] = {
atrac3p_wl_huff_xlat1, atrac3p_wl_huff_xlat2, NULL, NULL
};
static const uint8_t ct_nb_bits[4] = { 3, 4, 4, 4 };
static const uint8_t ct_nb_codes[4] = { 4, 8, 8, 8 };
static const uint8_t * const ct_bits[4] = {
atrac3p_ct_huff_bits1, atrac3p_ct_huff_bits2,
atrac3p_ct_huff_bits2, atrac3p_ct_huff_bits3
};
static const uint8_t * const ct_codes[4] = {
atrac3p_ct_huff_code1, atrac3p_ct_huff_code2,
atrac3p_ct_huff_code2, atrac3p_ct_huff_code3
};
static const uint8_t * const ct_xlats[4] = {
NULL, NULL, atrac3p_ct_huff_xlat1, NULL
};
static const uint8_t sf_nb_bits[8] = { 9, 9, 9, 9, 6, 6, 7, 7 };
static const uint8_t sf_nb_codes[8] = { 64, 64, 64, 64, 16, 16, 16, 16 };
static const uint8_t * const sf_bits[8] = {
atrac3p_sf_huff_bits1, atrac3p_sf_huff_bits1, atrac3p_sf_huff_bits2,
atrac3p_sf_huff_bits3, atrac3p_sf_huff_bits4, atrac3p_sf_huff_bits4,
atrac3p_sf_huff_bits5, atrac3p_sf_huff_bits6
};
static const uint16_t * const sf_codes[8] = {
atrac3p_sf_huff_code1, atrac3p_sf_huff_code1, atrac3p_sf_huff_code2,
atrac3p_sf_huff_code3, atrac3p_sf_huff_code4, atrac3p_sf_huff_code4,
atrac3p_sf_huff_code5, atrac3p_sf_huff_code6
};
static const uint8_t * const sf_xlats[8] = {
atrac3p_sf_huff_xlat1, atrac3p_sf_huff_xlat2, NULL, NULL,
atrac3p_sf_huff_xlat4, atrac3p_sf_huff_xlat5, NULL, NULL
};
static const uint8_t * const gain_cbs[11] = {
atrac3p_huff_gain_npoints1_cb, atrac3p_huff_gain_npoints1_cb,
atrac3p_huff_gain_lev1_cb, atrac3p_huff_gain_lev2_cb,
atrac3p_huff_gain_lev3_cb, atrac3p_huff_gain_lev4_cb,
atrac3p_huff_gain_loc3_cb, atrac3p_huff_gain_loc1_cb,
atrac3p_huff_gain_loc4_cb, atrac3p_huff_gain_loc2_cb,
atrac3p_huff_gain_loc5_cb
};
static const uint8_t * const gain_xlats[11] = {
NULL, atrac3p_huff_gain_npoints2_xlat, atrac3p_huff_gain_lev1_xlat,
atrac3p_huff_gain_lev2_xlat, atrac3p_huff_gain_lev3_xlat,
atrac3p_huff_gain_lev4_xlat, atrac3p_huff_gain_loc3_xlat,
atrac3p_huff_gain_loc1_xlat, atrac3p_huff_gain_loc4_xlat,
atrac3p_huff_gain_loc2_xlat, atrac3p_huff_gain_loc5_xlat
};
static const uint8_t * const tone_cbs[7] = {
atrac3p_huff_tonebands_cb, atrac3p_huff_numwavs1_cb,
atrac3p_huff_numwavs2_cb, atrac3p_huff_wav_ampsf1_cb,
atrac3p_huff_wav_ampsf2_cb, atrac3p_huff_wav_ampsf3_cb,
atrac3p_huff_freq_cb
};
static const uint8_t * const tone_xlats[7] = {
NULL, NULL, atrac3p_huff_numwavs2_xlat, atrac3p_huff_wav_ampsf1_xlat,
atrac3p_huff_wav_ampsf2_xlat, atrac3p_huff_wav_ampsf3_xlat,
atrac3p_huff_freq_xlat
};
for (i = 0, wl_vlc_offs = 0, ct_vlc_offs = 2508; i < 4; i++) {
wl_vlc_tabs[i].table = &tables_data[wl_vlc_offs];
wl_vlc_tabs[i].table_allocated = 1 << wl_nb_bits[i];
ct_vlc_tabs[i].table = &tables_data[ct_vlc_offs];
ct_vlc_tabs[i].table_allocated = 1 << ct_nb_bits[i];
ff_init_vlc_sparse(&wl_vlc_tabs[i], wl_nb_bits[i], wl_nb_codes[i],
wl_bits[i], 1, 1,
wl_codes[i], 1, 1,
wl_xlats[i], 1, 1,
INIT_VLC_USE_NEW_STATIC);
ff_init_vlc_sparse(&ct_vlc_tabs[i], ct_nb_bits[i], ct_nb_codes[i],
ct_bits[i], 1, 1,
ct_codes[i], 1, 1,
ct_xlats[i], 1, 1,
INIT_VLC_USE_NEW_STATIC);
wl_vlc_offs += wl_vlc_tabs[i].table_allocated;
ct_vlc_offs += ct_vlc_tabs[i].table_allocated;
}
for (i = 0, sf_vlc_offs = 76; i < 8; i++) {
sf_vlc_tabs[i].table = &tables_data[sf_vlc_offs];
sf_vlc_tabs[i].table_allocated = 1 << sf_nb_bits[i];
ff_init_vlc_sparse(&sf_vlc_tabs[i], sf_nb_bits[i], sf_nb_codes[i],
sf_bits[i], 1, 1,
sf_codes[i], 2, 2,
sf_xlats[i], 1, 1,
INIT_VLC_USE_NEW_STATIC);
sf_vlc_offs += sf_vlc_tabs[i].table_allocated;
}
tab_offset = 2564;
for (i = 0; i < 112; i++) {
if (atrac3p_spectra_tabs[i].cb)
build_canonical_huff(atrac3p_spectra_tabs[i].cb,
atrac3p_spectra_tabs[i].xlat,
&tab_offset, &spec_vlc_tabs[i]);
else
spec_vlc_tabs[i].table = 0;
}
for (i = 0; i < 11; i++)
build_canonical_huff(gain_cbs[i], gain_xlats[i], &tab_offset, &gain_vlc_tabs[i]);
for (i = 0; i < 7; i++)
build_canonical_huff(tone_cbs[i], tone_xlats[i], &tab_offset, &tone_vlc_tabs[i]);
}
static int num_coded_units(GetBitContext *gb, Atrac3pChanParams *chan,
Atrac3pChanUnitCtx *ctx, AVCodecContext *avctx)
{
chan->fill_mode = get_bits(gb, 2);
if (!chan->fill_mode) {
chan->num_coded_vals = ctx->num_quant_units;
} else {
chan->num_coded_vals = get_bits(gb, 5);
if (chan->num_coded_vals > ctx->num_quant_units) {
av_log(avctx, AV_LOG_ERROR,
"Invalid number of transmitted units!\n");
return AVERROR_INVALIDDATA;
}
if (chan->fill_mode == 3)
chan->split_point = get_bits(gb, 2) + (chan->ch_num << 1) + 1;
}
return 0;
}
static int add_wordlen_weights(Atrac3pChanUnitCtx *ctx,
Atrac3pChanParams *chan, int wtab_idx,
AVCodecContext *avctx)
{
int i;
const int8_t *weights_tab =
&atrac3p_wl_weights[chan->ch_num * 3 + wtab_idx - 1][0];
for (i = 0; i < ctx->num_quant_units; i++) {
chan->qu_wordlen[i] += weights_tab[i];
if (chan->qu_wordlen[i] < 0 || chan->qu_wordlen[i] > 7) {
av_log(avctx, AV_LOG_ERROR,
"WL index out of range: pos=%d, val=%d!\n",
i, chan->qu_wordlen[i]);
return AVERROR_INVALIDDATA;
}
}
return 0;
}
static int subtract_sf_weights(Atrac3pChanUnitCtx *ctx,
Atrac3pChanParams *chan, int wtab_idx,
AVCodecContext *avctx)
{
int i;
const int8_t *weights_tab = &atrac3p_sf_weights[wtab_idx - 1][0];
for (i = 0; i < ctx->used_quant_units; i++) {
chan->qu_sf_idx[i] -= weights_tab[i];
if (chan->qu_sf_idx[i] < 0 || chan->qu_sf_idx[i] > 63) {
av_log(avctx, AV_LOG_ERROR,
"SF index out of range: pos=%d, val=%d!\n",
i, chan->qu_sf_idx[i]);
return AVERROR_INVALIDDATA;
}
}
return 0;
}
static inline void unpack_vq_shape(int start_val, const int8_t *shape_vec,
int *dst, int num_values)
{
int i;
if (num_values) {
dst[0] = dst[1] = dst[2] = start_val;
for (i = 3; i < num_values; i++)
dst[i] = start_val - shape_vec[atrac3p_qu_num_to_seg[i] - 1];
}
}
#define UNPACK_SF_VQ_SHAPE(gb, dst, num_vals) start_val = get_bits((gb), 6); unpack_vq_shape(start_val, &atrac3p_sf_shapes[get_bits((gb), 6)][0], (dst), (num_vals))
static int decode_channel_wordlen(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, AVCodecContext *avctx)
{
int i, weight_idx = 0, delta, diff, pos, delta_bits, min_val, flag,
ret, start_val;
VLC *vlc_tab;
Atrac3pChanParams *chan = &ctx->channels[ch_num];
Atrac3pChanParams *ref_chan = &ctx->channels[0];
chan->fill_mode = 0;
switch (get_bits(gb, 2)) { 
case 0: 
for (i = 0; i < ctx->num_quant_units; i++)
chan->qu_wordlen[i] = get_bits(gb, 3);
break;
case 1:
if (ch_num) {
if ((ret = num_coded_units(gb, chan, ctx, avctx)) < 0)
return ret;
if (chan->num_coded_vals) {
vlc_tab = &wl_vlc_tabs[get_bits(gb, 2)];
for (i = 0; i < chan->num_coded_vals; i++) {
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_wordlen[i] = (ref_chan->qu_wordlen[i] + delta) & 7;
}
}
} else {
weight_idx = get_bits(gb, 2);
if ((ret = num_coded_units(gb, chan, ctx, avctx)) < 0)
return ret;
if (chan->num_coded_vals) {
pos = get_bits(gb, 5);
if (pos > chan->num_coded_vals) {
av_log(avctx, AV_LOG_ERROR,
"WL mode 1: invalid position!\n");
return AVERROR_INVALIDDATA;
}
delta_bits = get_bits(gb, 2);
min_val = get_bits(gb, 3);
for (i = 0; i < pos; i++)
chan->qu_wordlen[i] = get_bits(gb, 3);
for (i = pos; i < chan->num_coded_vals; i++)
chan->qu_wordlen[i] = (min_val + get_bitsz(gb, delta_bits)) & 7;
}
}
break;
case 2:
if ((ret = num_coded_units(gb, chan, ctx, avctx)) < 0)
return ret;
if (ch_num && chan->num_coded_vals) {
vlc_tab = &wl_vlc_tabs[get_bits(gb, 2)];
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_wordlen[0] = (ref_chan->qu_wordlen[0] + delta) & 7;
for (i = 1; i < chan->num_coded_vals; i++) {
diff = ref_chan->qu_wordlen[i] - ref_chan->qu_wordlen[i - 1];
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_wordlen[i] = (chan->qu_wordlen[i - 1] + diff + delta) & 7;
}
} else if (chan->num_coded_vals) {
flag = get_bits(gb, 1);
vlc_tab = &wl_vlc_tabs[get_bits(gb, 1)];
start_val = get_bits(gb, 3);
unpack_vq_shape(start_val,
&atrac3p_wl_shapes[start_val][get_bits(gb, 4)][0],
chan->qu_wordlen, chan->num_coded_vals);
if (!flag) {
for (i = 0; i < chan->num_coded_vals; i++) {
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_wordlen[i] = (chan->qu_wordlen[i] + delta) & 7;
}
} else {
for (i = 0; i < (chan->num_coded_vals & - 2); i += 2)
if (!get_bits1(gb)) {
chan->qu_wordlen[i] = (chan->qu_wordlen[i] +
get_vlc2(gb, vlc_tab->table,
vlc_tab->bits, 1)) & 7;
chan->qu_wordlen[i + 1] = (chan->qu_wordlen[i + 1] +
get_vlc2(gb, vlc_tab->table,
vlc_tab->bits, 1)) & 7;
}
if (chan->num_coded_vals & 1)
chan->qu_wordlen[i] = (chan->qu_wordlen[i] +
get_vlc2(gb, vlc_tab->table,
vlc_tab->bits, 1)) & 7;
}
}
break;
case 3:
weight_idx = get_bits(gb, 2);
if ((ret = num_coded_units(gb, chan, ctx, avctx)) < 0)
return ret;
if (chan->num_coded_vals) {
vlc_tab = &wl_vlc_tabs[get_bits(gb, 2)];
chan->qu_wordlen[0] = get_bits(gb, 3);
for (i = 1; i < chan->num_coded_vals; i++) {
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_wordlen[i] = (chan->qu_wordlen[i - 1] + delta) & 7;
}
}
break;
}
if (chan->fill_mode == 2) {
for (i = chan->num_coded_vals; i < ctx->num_quant_units; i++)
chan->qu_wordlen[i] = ch_num ? get_bits1(gb) : 1;
} else if (chan->fill_mode == 3) {
pos = ch_num ? chan->num_coded_vals + chan->split_point
: ctx->num_quant_units - chan->split_point;
if (pos > FF_ARRAY_ELEMS(chan->qu_wordlen)) {
av_log(avctx, AV_LOG_ERROR, "Split point beyond array\n");
pos = FF_ARRAY_ELEMS(chan->qu_wordlen);
}
for (i = chan->num_coded_vals; i < pos; i++)
chan->qu_wordlen[i] = 1;
}
if (weight_idx)
return add_wordlen_weights(ctx, chan, weight_idx, avctx);
return 0;
}
static int decode_channel_sf_idx(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, AVCodecContext *avctx)
{
int i, weight_idx = 0, delta, diff, num_long_vals,
delta_bits, min_val, vlc_sel, start_val;
VLC *vlc_tab;
Atrac3pChanParams *chan = &ctx->channels[ch_num];
Atrac3pChanParams *ref_chan = &ctx->channels[0];
switch (get_bits(gb, 2)) { 
case 0: 
for (i = 0; i < ctx->used_quant_units; i++)
chan->qu_sf_idx[i] = get_bits(gb, 6);
break;
case 1:
if (ch_num) {
vlc_tab = &sf_vlc_tabs[get_bits(gb, 2)];
for (i = 0; i < ctx->used_quant_units; i++) {
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_sf_idx[i] = (ref_chan->qu_sf_idx[i] + delta) & 0x3F;
}
} else {
weight_idx = get_bits(gb, 2);
if (weight_idx == 3) {
UNPACK_SF_VQ_SHAPE(gb, chan->qu_sf_idx, ctx->used_quant_units);
num_long_vals = get_bits(gb, 5);
delta_bits = get_bits(gb, 2);
min_val = get_bits(gb, 4) - 7;
for (i = 0; i < num_long_vals; i++)
chan->qu_sf_idx[i] = (chan->qu_sf_idx[i] +
get_bits(gb, 4) - 7) & 0x3F;
for (i = num_long_vals; i < ctx->used_quant_units; i++)
chan->qu_sf_idx[i] = (chan->qu_sf_idx[i] + min_val +
get_bitsz(gb, delta_bits)) & 0x3F;
} else {
num_long_vals = get_bits(gb, 5);
delta_bits = get_bits(gb, 3);
min_val = get_bits(gb, 6);
if (num_long_vals > ctx->used_quant_units || delta_bits == 7) {
av_log(avctx, AV_LOG_ERROR,
"SF mode 1: invalid parameters!\n");
return AVERROR_INVALIDDATA;
}
for (i = 0; i < num_long_vals; i++)
chan->qu_sf_idx[i] = get_bits(gb, 6);
for (i = num_long_vals; i < ctx->used_quant_units; i++)
chan->qu_sf_idx[i] = (min_val +
get_bitsz(gb, delta_bits)) & 0x3F;
}
}
break;
case 2:
if (ch_num) {
vlc_tab = &sf_vlc_tabs[get_bits(gb, 2)];
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_sf_idx[0] = (ref_chan->qu_sf_idx[0] + delta) & 0x3F;
for (i = 1; i < ctx->used_quant_units; i++) {
diff = ref_chan->qu_sf_idx[i] - ref_chan->qu_sf_idx[i - 1];
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_sf_idx[i] = (chan->qu_sf_idx[i - 1] + diff + delta) & 0x3F;
}
} else {
vlc_tab = &sf_vlc_tabs[get_bits(gb, 2) + 4];
UNPACK_SF_VQ_SHAPE(gb, chan->qu_sf_idx, ctx->used_quant_units);
for (i = 0; i < ctx->used_quant_units; i++) {
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_sf_idx[i] = (chan->qu_sf_idx[i] +
sign_extend(delta, 4)) & 0x3F;
}
}
break;
case 3:
if (ch_num) {
for (i = 0; i < ctx->used_quant_units; i++)
chan->qu_sf_idx[i] = ref_chan->qu_sf_idx[i];
} else {
weight_idx = get_bits(gb, 2);
vlc_sel = get_bits(gb, 2);
vlc_tab = &sf_vlc_tabs[vlc_sel];
if (weight_idx == 3) {
vlc_tab = &sf_vlc_tabs[vlc_sel + 4];
UNPACK_SF_VQ_SHAPE(gb, chan->qu_sf_idx, ctx->used_quant_units);
diff = (get_bits(gb, 4) + 56) & 0x3F;
chan->qu_sf_idx[0] = (chan->qu_sf_idx[0] + diff) & 0x3F;
for (i = 1; i < ctx->used_quant_units; i++) {
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
diff = (diff + sign_extend(delta, 4)) & 0x3F;
chan->qu_sf_idx[i] = (diff + chan->qu_sf_idx[i]) & 0x3F;
}
} else {
chan->qu_sf_idx[0] = get_bits(gb, 6);
for (i = 1; i < ctx->used_quant_units; i++) {
delta = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
chan->qu_sf_idx[i] = (chan->qu_sf_idx[i - 1] + delta) & 0x3F;
}
}
}
break;
}
if (weight_idx && weight_idx < 3)
return subtract_sf_weights(ctx, chan, weight_idx, avctx);
return 0;
}
static int decode_quant_wordlen(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx)
{
int ch_num, i, ret;
for (ch_num = 0; ch_num < num_channels; ch_num++) {
memset(ctx->channels[ch_num].qu_wordlen, 0,
sizeof(ctx->channels[ch_num].qu_wordlen));
if ((ret = decode_channel_wordlen(gb, ctx, ch_num, avctx)) < 0)
return ret;
}
for (i = ctx->num_quant_units - 1; i >= 0; i--)
if (ctx->channels[0].qu_wordlen[i] ||
(num_channels == 2 && ctx->channels[1].qu_wordlen[i]))
break;
ctx->used_quant_units = i + 1;
return 0;
}
static int decode_scale_factors(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx)
{
int ch_num, ret;
if (!ctx->used_quant_units)
return 0;
for (ch_num = 0; ch_num < num_channels; ch_num++) {
memset(ctx->channels[ch_num].qu_sf_idx, 0,
sizeof(ctx->channels[ch_num].qu_sf_idx));
if ((ret = decode_channel_sf_idx(gb, ctx, ch_num, avctx)) < 0)
return ret;
}
return 0;
}
static int get_num_ct_values(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
AVCodecContext *avctx)
{
int num_coded_vals;
if (get_bits1(gb)) {
num_coded_vals = get_bits(gb, 5);
if (num_coded_vals > ctx->used_quant_units) {
av_log(avctx, AV_LOG_ERROR,
"Invalid number of code table indexes: %d!\n", num_coded_vals);
return AVERROR_INVALIDDATA;
}
return num_coded_vals;
} else
return ctx->used_quant_units;
}
#define DEC_CT_IDX_COMMON(OP) num_vals = get_num_ct_values(gb, ctx, avctx); if (num_vals < 0) return num_vals; for (i = 0; i < num_vals; i++) { if (chan->qu_wordlen[i]) { chan->qu_tab_idx[i] = OP; } else if (ch_num && ref_chan->qu_wordlen[i]) chan->qu_tab_idx[i] = get_bits1(gb); }
#define CODING_DIRECT get_bits(gb, num_bits)
#define CODING_VLC get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1)
#define CODING_VLC_DELTA (!i) ? CODING_VLC : (pred + get_vlc2(gb, delta_vlc->table, delta_vlc->bits, 1)) & mask; pred = chan->qu_tab_idx[i]
#define CODING_VLC_DIFF (ref_chan->qu_tab_idx[i] + get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1)) & mask
static int decode_channel_code_tab(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, AVCodecContext *avctx)
{
int i, num_vals, num_bits, pred;
int mask = ctx->use_full_table ? 7 : 3; 
VLC *vlc_tab, *delta_vlc;
Atrac3pChanParams *chan = &ctx->channels[ch_num];
Atrac3pChanParams *ref_chan = &ctx->channels[0];
chan->table_type = get_bits1(gb);
switch (get_bits(gb, 2)) { 
case 0: 
num_bits = ctx->use_full_table + 2;
DEC_CT_IDX_COMMON(CODING_DIRECT);
break;
case 1: 
vlc_tab = ctx->use_full_table ? &ct_vlc_tabs[1]
: ct_vlc_tabs;
DEC_CT_IDX_COMMON(CODING_VLC);
break;
case 2: 
if (ctx->use_full_table) {
vlc_tab = &ct_vlc_tabs[1];
delta_vlc = &ct_vlc_tabs[2];
} else {
vlc_tab = ct_vlc_tabs;
delta_vlc = ct_vlc_tabs;
}
pred = 0;
DEC_CT_IDX_COMMON(CODING_VLC_DELTA);
break;
case 3: 
if (ch_num) {
vlc_tab = ctx->use_full_table ? &ct_vlc_tabs[3]
: ct_vlc_tabs;
DEC_CT_IDX_COMMON(CODING_VLC_DIFF);
}
break;
}
return 0;
}
static int decode_code_table_indexes(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx)
{
int ch_num, ret;
if (!ctx->used_quant_units)
return 0;
ctx->use_full_table = get_bits1(gb);
for (ch_num = 0; ch_num < num_channels; ch_num++) {
memset(ctx->channels[ch_num].qu_tab_idx, 0,
sizeof(ctx->channels[ch_num].qu_tab_idx));
if ((ret = decode_channel_code_tab(gb, ctx, ch_num, avctx)) < 0)
return ret;
}
return 0;
}
static void decode_qu_spectra(GetBitContext *gb, const Atrac3pSpecCodeTab *tab,
VLC *vlc_tab, int16_t *out, const int num_specs)
{
int i, j, pos, cf;
int group_size = tab->group_size;
int num_coeffs = tab->num_coeffs;
int bits = tab->bits;
int is_signed = tab->is_signed;
unsigned val;
for (pos = 0; pos < num_specs;) {
if (group_size == 1 || get_bits1(gb)) {
for (j = 0; j < group_size; j++) {
val = get_vlc2(gb, vlc_tab->table, vlc_tab->bits, 1);
for (i = 0; i < num_coeffs; i++) {
cf = av_mod_uintp2(val, bits);
if (is_signed)
cf = sign_extend(cf, bits);
else if (cf && get_bits1(gb))
cf = -cf;
out[pos++] = cf;
val >>= bits;
}
}
} else 
pos += group_size * num_coeffs;
}
}
static void decode_spectrum(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx)
{
int i, ch_num, qu, wordlen, codetab, tab_index, num_specs;
const Atrac3pSpecCodeTab *tab;
Atrac3pChanParams *chan;
for (ch_num = 0; ch_num < num_channels; ch_num++) {
chan = &ctx->channels[ch_num];
memset(chan->spectrum, 0, sizeof(chan->spectrum));
memset(chan->power_levs, ATRAC3P_POWER_COMP_OFF, sizeof(chan->power_levs));
for (qu = 0; qu < ctx->used_quant_units; qu++) {
num_specs = ff_atrac3p_qu_to_spec_pos[qu + 1] -
ff_atrac3p_qu_to_spec_pos[qu];
wordlen = chan->qu_wordlen[qu];
codetab = chan->qu_tab_idx[qu];
if (wordlen) {
if (!ctx->use_full_table)
codetab = atrac3p_ct_restricted_to_full[chan->table_type][wordlen - 1][codetab];
tab_index = (chan->table_type * 8 + codetab) * 7 + wordlen - 1;
tab = &atrac3p_spectra_tabs[tab_index];
if (tab->redirect >= 0)
tab_index = tab->redirect;
decode_qu_spectra(gb, tab, &spec_vlc_tabs[tab_index],
&chan->spectrum[ff_atrac3p_qu_to_spec_pos[qu]],
num_specs);
} else if (ch_num && ctx->channels[0].qu_wordlen[qu] && !codetab) {
memcpy(&chan->spectrum[ff_atrac3p_qu_to_spec_pos[qu]],
&ctx->channels[0].spectrum[ff_atrac3p_qu_to_spec_pos[qu]],
num_specs *
sizeof(chan->spectrum[ff_atrac3p_qu_to_spec_pos[qu]]));
chan->qu_wordlen[qu] = ctx->channels[0].qu_wordlen[qu];
}
}
if (ctx->used_quant_units > 2) {
num_specs = atrac3p_subband_to_num_powgrps[ctx->num_coded_subbands - 1];
for (i = 0; i < num_specs; i++)
chan->power_levs[i] = get_bits(gb, 4);
}
}
}
static int get_subband_flags(GetBitContext *gb, uint8_t *out, int num_flags)
{
int i, result;
memset(out, 0, num_flags);
result = get_bits1(gb);
if (result) {
if (get_bits1(gb))
for (i = 0; i < num_flags; i++)
out[i] = get_bits1(gb);
else
memset(out, 1, num_flags);
}
return result;
}
static void decode_window_shape(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels)
{
int ch_num;
for (ch_num = 0; ch_num < num_channels; ch_num++)
get_subband_flags(gb, ctx->channels[ch_num].wnd_shape,
ctx->num_subbands);
}
static int decode_gainc_npoints(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int coded_subbands)
{
int i, delta, delta_bits, min_val;
Atrac3pChanParams *chan = &ctx->channels[ch_num];
Atrac3pChanParams *ref_chan = &ctx->channels[0];
switch (get_bits(gb, 2)) { 
case 0: 
for (i = 0; i < coded_subbands; i++)
chan->gain_data[i].num_points = get_bits(gb, 3);
break;
case 1: 
for (i = 0; i < coded_subbands; i++)
chan->gain_data[i].num_points =
get_vlc2(gb, gain_vlc_tabs[0].table,
gain_vlc_tabs[0].bits, 1);
break;
case 2:
if (ch_num) { 
for (i = 0; i < coded_subbands; i++) {
delta = get_vlc2(gb, gain_vlc_tabs[1].table,
gain_vlc_tabs[1].bits, 1);
chan->gain_data[i].num_points =
(ref_chan->gain_data[i].num_points + delta) & 7;
}
} else { 
chan->gain_data[0].num_points =
get_vlc2(gb, gain_vlc_tabs[0].table,
gain_vlc_tabs[0].bits, 1);
for (i = 1; i < coded_subbands; i++) {
delta = get_vlc2(gb, gain_vlc_tabs[1].table,
gain_vlc_tabs[1].bits, 1);
chan->gain_data[i].num_points =
(chan->gain_data[i - 1].num_points + delta) & 7;
}
}
break;
case 3:
if (ch_num) { 
for (i = 0; i < coded_subbands; i++)
chan->gain_data[i].num_points =
ref_chan->gain_data[i].num_points;
} else { 
delta_bits = get_bits(gb, 2);
min_val = get_bits(gb, 3);
for (i = 0; i < coded_subbands; i++) {
chan->gain_data[i].num_points = min_val + get_bitsz(gb, delta_bits);
if (chan->gain_data[i].num_points > 7)
return AVERROR_INVALIDDATA;
}
}
}
return 0;
}
static inline void gainc_level_mode3s(AtracGainInfo *dst, AtracGainInfo *ref)
{
int i;
for (i = 0; i < dst->num_points; i++)
dst->lev_code[i] = (i >= ref->num_points) ? 7 : ref->lev_code[i];
}
static inline void gainc_level_mode1m(GetBitContext *gb,
Atrac3pChanUnitCtx *ctx,
AtracGainInfo *dst)
{
int i, delta;
if (dst->num_points > 0)
dst->lev_code[0] = get_vlc2(gb, gain_vlc_tabs[2].table,
gain_vlc_tabs[2].bits, 1);
for (i = 1; i < dst->num_points; i++) {
delta = get_vlc2(gb, gain_vlc_tabs[3].table,
gain_vlc_tabs[3].bits, 1);
dst->lev_code[i] = (dst->lev_code[i - 1] + delta) & 0xF;
}
}
static int decode_gainc_levels(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int coded_subbands)
{
int sb, i, delta, delta_bits, min_val, pred;
Atrac3pChanParams *chan = &ctx->channels[ch_num];
Atrac3pChanParams *ref_chan = &ctx->channels[0];
switch (get_bits(gb, 2)) { 
case 0: 
for (sb = 0; sb < coded_subbands; sb++)
for (i = 0; i < chan->gain_data[sb].num_points; i++)
chan->gain_data[sb].lev_code[i] = get_bits(gb, 4);
break;
case 1:
if (ch_num) { 
for (sb = 0; sb < coded_subbands; sb++)
for (i = 0; i < chan->gain_data[sb].num_points; i++) {
delta = get_vlc2(gb, gain_vlc_tabs[5].table,
gain_vlc_tabs[5].bits, 1);
pred = (i >= ref_chan->gain_data[sb].num_points)
? 7 : ref_chan->gain_data[sb].lev_code[i];
chan->gain_data[sb].lev_code[i] = (pred + delta) & 0xF;
}
} else { 
for (sb = 0; sb < coded_subbands; sb++)
gainc_level_mode1m(gb, ctx, &chan->gain_data[sb]);
}
break;
case 2:
if (ch_num) { 
for (sb = 0; sb < coded_subbands; sb++)
if (chan->gain_data[sb].num_points > 0) {
if (get_bits1(gb))
gainc_level_mode1m(gb, ctx, &chan->gain_data[sb]);
else
gainc_level_mode3s(&chan->gain_data[sb],
&ref_chan->gain_data[sb]);
}
} else { 
if (chan->gain_data[0].num_points > 0)
gainc_level_mode1m(gb, ctx, &chan->gain_data[0]);
for (sb = 1; sb < coded_subbands; sb++)
for (i = 0; i < chan->gain_data[sb].num_points; i++) {
delta = get_vlc2(gb, gain_vlc_tabs[4].table,
gain_vlc_tabs[4].bits, 1);
pred = (i >= chan->gain_data[sb - 1].num_points)
? 7 : chan->gain_data[sb - 1].lev_code[i];
chan->gain_data[sb].lev_code[i] = (pred + delta) & 0xF;
}
}
break;
case 3:
if (ch_num) { 
for (sb = 0; sb < coded_subbands; sb++)
gainc_level_mode3s(&chan->gain_data[sb],
&ref_chan->gain_data[sb]);
} else { 
delta_bits = get_bits(gb, 2);
min_val = get_bits(gb, 4);
for (sb = 0; sb < coded_subbands; sb++)
for (i = 0; i < chan->gain_data[sb].num_points; i++) {
chan->gain_data[sb].lev_code[i] = min_val + get_bitsz(gb, delta_bits);
if (chan->gain_data[sb].lev_code[i] > 15)
return AVERROR_INVALIDDATA;
}
}
break;
}
return 0;
}
static inline void gainc_loc_mode0(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
AtracGainInfo *dst, int pos)
{
int delta_bits;
if (!pos || dst->loc_code[pos - 1] < 15)
dst->loc_code[pos] = get_bits(gb, 5);
else if (dst->loc_code[pos - 1] >= 30)
dst->loc_code[pos] = 31;
else {
delta_bits = av_log2(30 - dst->loc_code[pos - 1]) + 1;
dst->loc_code[pos] = dst->loc_code[pos - 1] +
get_bits(gb, delta_bits) + 1;
}
}
static inline void gainc_loc_mode1(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
AtracGainInfo *dst)
{
int i;
VLC *tab;
if (dst->num_points > 0) {
dst->loc_code[0] = get_bits(gb, 5);
for (i = 1; i < dst->num_points; i++) {
tab = (dst->lev_code[i] <= dst->lev_code[i - 1])
? &gain_vlc_tabs[7]
: &gain_vlc_tabs[9];
dst->loc_code[i] = dst->loc_code[i - 1] +
get_vlc2(gb, tab->table, tab->bits, 1);
}
}
}
static int decode_gainc_loc_codes(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int coded_subbands,
AVCodecContext *avctx)
{
int sb, i, delta, delta_bits, min_val, pred, more_than_ref;
AtracGainInfo *dst, *ref;
VLC *tab;
Atrac3pChanParams *chan = &ctx->channels[ch_num];
Atrac3pChanParams *ref_chan = &ctx->channels[0];
switch (get_bits(gb, 2)) { 
case 0: 
for (sb = 0; sb < coded_subbands; sb++)
for (i = 0; i < chan->gain_data[sb].num_points; i++)
gainc_loc_mode0(gb, ctx, &chan->gain_data[sb], i);
break;
case 1:
if (ch_num) {
for (sb = 0; sb < coded_subbands; sb++) {
if (chan->gain_data[sb].num_points <= 0)
continue;
dst = &chan->gain_data[sb];
ref = &ref_chan->gain_data[sb];
delta = get_vlc2(gb, gain_vlc_tabs[10].table,
gain_vlc_tabs[10].bits, 1);
pred = ref->num_points > 0 ? ref->loc_code[0] : 0;
dst->loc_code[0] = (pred + delta) & 0x1F;
for (i = 1; i < dst->num_points; i++) {
more_than_ref = i >= ref->num_points;
if (dst->lev_code[i] > dst->lev_code[i - 1]) {
if (more_than_ref) {
delta =
get_vlc2(gb, gain_vlc_tabs[9].table,
gain_vlc_tabs[9].bits, 1);
dst->loc_code[i] = dst->loc_code[i - 1] + delta;
} else {
if (get_bits1(gb))
gainc_loc_mode0(gb, ctx, dst, i); 
else
dst->loc_code[i] = ref->loc_code[i]; 
}
} else { 
tab = more_than_ref ? &gain_vlc_tabs[7]
: &gain_vlc_tabs[10];
delta = get_vlc2(gb, tab->table, tab->bits, 1);
if (more_than_ref)
dst->loc_code[i] = dst->loc_code[i - 1] + delta;
else
dst->loc_code[i] = (ref->loc_code[i] + delta) & 0x1F;
}
}
}
} else 
for (sb = 0; sb < coded_subbands; sb++)
gainc_loc_mode1(gb, ctx, &chan->gain_data[sb]);
break;
case 2:
if (ch_num) {
for (sb = 0; sb < coded_subbands; sb++) {
if (chan->gain_data[sb].num_points <= 0)
continue;
dst = &chan->gain_data[sb];
ref = &ref_chan->gain_data[sb];
if (dst->num_points > ref->num_points || get_bits1(gb))
gainc_loc_mode1(gb, ctx, dst);
else 
for (i = 0; i < chan->gain_data[sb].num_points; i++)
dst->loc_code[i] = ref->loc_code[i];
}
} else {
for (i = 0; i < chan->gain_data[0].num_points; i++)
gainc_loc_mode0(gb, ctx, &chan->gain_data[0], i);
for (sb = 1; sb < coded_subbands; sb++) {
if (chan->gain_data[sb].num_points <= 0)
continue;
dst = &chan->gain_data[sb];
delta = get_vlc2(gb, gain_vlc_tabs[6].table,
gain_vlc_tabs[6].bits, 1);
pred = dst[-1].num_points > 0
? dst[-1].loc_code[0] : 0;
dst->loc_code[0] = (pred + delta) & 0x1F;
for (i = 1; i < dst->num_points; i++) {
more_than_ref = i >= dst[-1].num_points;
tab = &gain_vlc_tabs[(dst->lev_code[i] > dst->lev_code[i - 1]) *
2 + more_than_ref + 6];
delta = get_vlc2(gb, tab->table, tab->bits, 1);
if (more_than_ref)
dst->loc_code[i] = dst->loc_code[i - 1] + delta;
else
dst->loc_code[i] = (dst[-1].loc_code[i] + delta) & 0x1F;
}
}
}
break;
case 3:
if (ch_num) { 
for (sb = 0; sb < coded_subbands; sb++)
for (i = 0; i < chan->gain_data[sb].num_points; i++) {
if (i >= ref_chan->gain_data[sb].num_points)
gainc_loc_mode0(gb, ctx, &chan->gain_data[sb], i);
else
chan->gain_data[sb].loc_code[i] =
ref_chan->gain_data[sb].loc_code[i];
}
} else { 
delta_bits = get_bits(gb, 2) + 1;
min_val = get_bits(gb, 5);
for (sb = 0; sb < coded_subbands; sb++)
for (i = 0; i < chan->gain_data[sb].num_points; i++)
chan->gain_data[sb].loc_code[i] = min_val + i +
get_bits(gb, delta_bits);
}
break;
}
for (sb = 0; sb < coded_subbands; sb++) {
dst = &chan->gain_data[sb];
for (i = 0; i < chan->gain_data[sb].num_points; i++) {
if (dst->loc_code[i] < 0 || dst->loc_code[i] > 31 ||
(i && dst->loc_code[i] <= dst->loc_code[i - 1])) {
av_log(avctx, AV_LOG_ERROR,
"Invalid gain location: ch=%d, sb=%d, pos=%d, val=%d\n",
ch_num, sb, i, dst->loc_code[i]);
return AVERROR_INVALIDDATA;
}
}
}
return 0;
}
static int decode_gainc_data(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx)
{
int ch_num, coded_subbands, sb, ret;
for (ch_num = 0; ch_num < num_channels; ch_num++) {
memset(ctx->channels[ch_num].gain_data, 0,
sizeof(*ctx->channels[ch_num].gain_data) * ATRAC3P_SUBBANDS);
if (get_bits1(gb)) { 
coded_subbands = get_bits(gb, 4) + 1;
if (get_bits1(gb)) 
ctx->channels[ch_num].num_gain_subbands = get_bits(gb, 4) + 1;
else
ctx->channels[ch_num].num_gain_subbands = coded_subbands;
if ((ret = decode_gainc_npoints(gb, ctx, ch_num, coded_subbands)) < 0 ||
(ret = decode_gainc_levels(gb, ctx, ch_num, coded_subbands)) < 0 ||
(ret = decode_gainc_loc_codes(gb, ctx, ch_num, coded_subbands, avctx)) < 0)
return ret;
if (coded_subbands > 0) { 
for (sb = coded_subbands; sb < ctx->channels[ch_num].num_gain_subbands; sb++)
ctx->channels[ch_num].gain_data[sb] =
ctx->channels[ch_num].gain_data[sb - 1];
}
} else {
ctx->channels[ch_num].num_gain_subbands = 0;
}
}
return 0;
}
static void decode_tones_envelope(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int band_has_tones[])
{
int sb;
Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
Atrac3pWavesData *ref = ctx->channels[0].tones_info;
if (!ch_num || !get_bits1(gb)) { 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb])
continue;
dst[sb].pend_env.has_start_point = get_bits1(gb);
dst[sb].pend_env.start_pos = dst[sb].pend_env.has_start_point
? get_bits(gb, 5) : -1;
dst[sb].pend_env.has_stop_point = get_bits1(gb);
dst[sb].pend_env.stop_pos = dst[sb].pend_env.has_stop_point
? get_bits(gb, 5) : 32;
}
} else { 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb])
continue;
dst[sb].pend_env.has_start_point = ref[sb].pend_env.has_start_point;
dst[sb].pend_env.has_stop_point = ref[sb].pend_env.has_stop_point;
dst[sb].pend_env.start_pos = ref[sb].pend_env.start_pos;
dst[sb].pend_env.stop_pos = ref[sb].pend_env.stop_pos;
}
}
}
static int decode_band_numwavs(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int band_has_tones[],
AVCodecContext *avctx)
{
int mode, sb, delta;
Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
Atrac3pWavesData *ref = ctx->channels[0].tones_info;
mode = get_bits(gb, ch_num + 1);
switch (mode) {
case 0: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
if (band_has_tones[sb])
dst[sb].num_wavs = get_bits(gb, 4);
break;
case 1: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
if (band_has_tones[sb])
dst[sb].num_wavs =
get_vlc2(gb, tone_vlc_tabs[1].table,
tone_vlc_tabs[1].bits, 1);
break;
case 2: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
if (band_has_tones[sb]) {
delta = get_vlc2(gb, tone_vlc_tabs[2].table,
tone_vlc_tabs[2].bits, 1);
delta = sign_extend(delta, 3);
dst[sb].num_wavs = (ref[sb].num_wavs + delta) & 0xF;
}
break;
case 3: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
if (band_has_tones[sb])
dst[sb].num_wavs = ref[sb].num_wavs;
break;
}
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++)
if (band_has_tones[sb]) {
if (ctx->waves_info->tones_index + dst[sb].num_wavs > 48) {
av_log(avctx, AV_LOG_ERROR,
"Too many tones: %d (max. 48), frame: %d!\n",
ctx->waves_info->tones_index + dst[sb].num_wavs,
avctx->frame_number);
return AVERROR_INVALIDDATA;
}
dst[sb].start_index = ctx->waves_info->tones_index;
ctx->waves_info->tones_index += dst[sb].num_wavs;
}
return 0;
}
static void decode_tones_frequency(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int band_has_tones[])
{
int sb, i, direction, nbits, pred, delta;
Atrac3pWaveParam *iwav, *owav;
Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
Atrac3pWavesData *ref = ctx->channels[0].tones_info;
if (!ch_num || !get_bits1(gb)) { 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb] || !dst[sb].num_wavs)
continue;
iwav = &ctx->waves_info->waves[dst[sb].start_index];
direction = (dst[sb].num_wavs > 1) ? get_bits1(gb) : 0;
if (direction) { 
if (dst[sb].num_wavs)
iwav[dst[sb].num_wavs - 1].freq_index = get_bits(gb, 10);
for (i = dst[sb].num_wavs - 2; i >= 0 ; i--) {
nbits = av_log2(iwav[i+1].freq_index) + 1;
iwav[i].freq_index = get_bits(gb, nbits);
}
} else { 
for (i = 0; i < dst[sb].num_wavs; i++) {
if (!i || iwav[i - 1].freq_index < 512)
iwav[i].freq_index = get_bits(gb, 10);
else {
nbits = av_log2(1023 - iwav[i - 1].freq_index) + 1;
iwav[i].freq_index = get_bits(gb, nbits) +
1024 - (1 << nbits);
}
}
}
}
} else { 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb] || !dst[sb].num_wavs)
continue;
iwav = &ctx->waves_info->waves[ref[sb].start_index];
owav = &ctx->waves_info->waves[dst[sb].start_index];
for (i = 0; i < dst[sb].num_wavs; i++) {
delta = get_vlc2(gb, tone_vlc_tabs[6].table,
tone_vlc_tabs[6].bits, 1);
delta = sign_extend(delta, 8);
pred = (i < ref[sb].num_wavs) ? iwav[i].freq_index :
(ref[sb].num_wavs ? iwav[ref[sb].num_wavs - 1].freq_index : 0);
owav[i].freq_index = (pred + delta) & 0x3FF;
}
}
}
}
static void decode_tones_amplitude(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int band_has_tones[])
{
int mode, sb, j, i, diff, maxdiff, fi, delta, pred;
Atrac3pWaveParam *wsrc, *wref;
int refwaves[48] = { 0 };
Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
Atrac3pWavesData *ref = ctx->channels[0].tones_info;
if (ch_num) {
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb] || !dst[sb].num_wavs)
continue;
wsrc = &ctx->waves_info->waves[dst[sb].start_index];
wref = &ctx->waves_info->waves[ref[sb].start_index];
for (j = 0; j < dst[sb].num_wavs; j++) {
for (i = 0, fi = 0, maxdiff = 1024; i < ref[sb].num_wavs; i++) {
diff = FFABS(wsrc[j].freq_index - wref[i].freq_index);
if (diff < maxdiff) {
maxdiff = diff;
fi = i;
}
}
if (maxdiff < 8)
refwaves[dst[sb].start_index + j] = fi + ref[sb].start_index;
else if (j < ref[sb].num_wavs)
refwaves[dst[sb].start_index + j] = j + ref[sb].start_index;
else
refwaves[dst[sb].start_index + j] = -1;
}
}
}
mode = get_bits(gb, ch_num + 1);
switch (mode) {
case 0: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb] || !dst[sb].num_wavs)
continue;
if (ctx->waves_info->amplitude_mode)
for (i = 0; i < dst[sb].num_wavs; i++)
ctx->waves_info->waves[dst[sb].start_index + i].amp_sf = get_bits(gb, 6);
else
ctx->waves_info->waves[dst[sb].start_index].amp_sf = get_bits(gb, 6);
}
break;
case 1: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb] || !dst[sb].num_wavs)
continue;
if (ctx->waves_info->amplitude_mode)
for (i = 0; i < dst[sb].num_wavs; i++)
ctx->waves_info->waves[dst[sb].start_index + i].amp_sf =
get_vlc2(gb, tone_vlc_tabs[3].table,
tone_vlc_tabs[3].bits, 1) + 20;
else
ctx->waves_info->waves[dst[sb].start_index].amp_sf =
get_vlc2(gb, tone_vlc_tabs[4].table,
tone_vlc_tabs[4].bits, 1) + 24;
}
break;
case 2: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb] || !dst[sb].num_wavs)
continue;
for (i = 0; i < dst[sb].num_wavs; i++) {
delta = get_vlc2(gb, tone_vlc_tabs[5].table,
tone_vlc_tabs[5].bits, 1);
delta = sign_extend(delta, 5);
pred = refwaves[dst[sb].start_index + i] >= 0 ?
ctx->waves_info->waves[refwaves[dst[sb].start_index + i]].amp_sf : 34;
ctx->waves_info->waves[dst[sb].start_index + i].amp_sf = (pred + delta) & 0x3F;
}
}
break;
case 3: 
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb])
continue;
for (i = 0; i < dst[sb].num_wavs; i++)
ctx->waves_info->waves[dst[sb].start_index + i].amp_sf =
refwaves[dst[sb].start_index + i] >= 0
? ctx->waves_info->waves[refwaves[dst[sb].start_index + i]].amp_sf
: 32;
}
break;
}
}
static void decode_tones_phase(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int ch_num, int band_has_tones[])
{
int sb, i;
Atrac3pWaveParam *wparam;
Atrac3pWavesData *dst = ctx->channels[ch_num].tones_info;
for (sb = 0; sb < ctx->waves_info->num_tone_bands; sb++) {
if (!band_has_tones[sb])
continue;
wparam = &ctx->waves_info->waves[dst[sb].start_index];
for (i = 0; i < dst[sb].num_wavs; i++)
wparam[i].phase_index = get_bits(gb, 5);
}
}
static int decode_tones_info(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx)
{
int ch_num, i, ret;
int band_has_tones[16];
for (ch_num = 0; ch_num < num_channels; ch_num++)
memset(ctx->channels[ch_num].tones_info, 0,
sizeof(*ctx->channels[ch_num].tones_info) * ATRAC3P_SUBBANDS);
ctx->waves_info->tones_present = get_bits1(gb);
if (!ctx->waves_info->tones_present)
return 0;
memset(ctx->waves_info->waves, 0, sizeof(ctx->waves_info->waves));
ctx->waves_info->amplitude_mode = get_bits1(gb);
if (!ctx->waves_info->amplitude_mode) {
avpriv_report_missing_feature(avctx, "GHA amplitude mode 0");
return AVERROR_PATCHWELCOME;
}
ctx->waves_info->num_tone_bands =
get_vlc2(gb, tone_vlc_tabs[0].table,
tone_vlc_tabs[0].bits, 1) + 1;
if (num_channels == 2) {
get_subband_flags(gb, ctx->waves_info->tone_sharing, ctx->waves_info->num_tone_bands);
get_subband_flags(gb, ctx->waves_info->tone_master, ctx->waves_info->num_tone_bands);
get_subband_flags(gb, ctx->waves_info->invert_phase, ctx->waves_info->num_tone_bands);
}
ctx->waves_info->tones_index = 0;
for (ch_num = 0; ch_num < num_channels; ch_num++) {
for (i = 0; i < ctx->waves_info->num_tone_bands; i++)
band_has_tones[i] = !ch_num ? 1 : !ctx->waves_info->tone_sharing[i];
decode_tones_envelope(gb, ctx, ch_num, band_has_tones);
if ((ret = decode_band_numwavs(gb, ctx, ch_num, band_has_tones,
avctx)) < 0)
return ret;
decode_tones_frequency(gb, ctx, ch_num, band_has_tones);
decode_tones_amplitude(gb, ctx, ch_num, band_has_tones);
decode_tones_phase(gb, ctx, ch_num, band_has_tones);
}
if (num_channels == 2) {
for (i = 0; i < ctx->waves_info->num_tone_bands; i++) {
if (ctx->waves_info->tone_sharing[i])
ctx->channels[1].tones_info[i] = ctx->channels[0].tones_info[i];
if (ctx->waves_info->tone_master[i])
FFSWAP(Atrac3pWavesData, ctx->channels[0].tones_info[i],
ctx->channels[1].tones_info[i]);
}
}
return 0;
}
int ff_atrac3p_decode_channel_unit(GetBitContext *gb, Atrac3pChanUnitCtx *ctx,
int num_channels, AVCodecContext *avctx)
{
int ret;
ctx->num_quant_units = get_bits(gb, 5) + 1;
if (ctx->num_quant_units > 28 && ctx->num_quant_units < 32) {
av_log(avctx, AV_LOG_ERROR,
"Invalid number of quantization units: %d!\n",
ctx->num_quant_units);
return AVERROR_INVALIDDATA;
}
ctx->mute_flag = get_bits1(gb);
if ((ret = decode_quant_wordlen(gb, ctx, num_channels, avctx)) < 0)
return ret;
ctx->num_subbands = atrac3p_qu_to_subband[ctx->num_quant_units - 1] + 1;
ctx->num_coded_subbands = ctx->used_quant_units
? atrac3p_qu_to_subband[ctx->used_quant_units - 1] + 1
: 0;
if ((ret = decode_scale_factors(gb, ctx, num_channels, avctx)) < 0)
return ret;
if ((ret = decode_code_table_indexes(gb, ctx, num_channels, avctx)) < 0)
return ret;
decode_spectrum(gb, ctx, num_channels, avctx);
if (num_channels == 2) {
get_subband_flags(gb, ctx->swap_channels, ctx->num_coded_subbands);
get_subband_flags(gb, ctx->negate_coeffs, ctx->num_coded_subbands);
}
decode_window_shape(gb, ctx, num_channels);
if ((ret = decode_gainc_data(gb, ctx, num_channels, avctx)) < 0)
return ret;
if ((ret = decode_tones_info(gb, ctx, num_channels, avctx)) < 0)
return ret;
ctx->noise_present = get_bits1(gb);
if (ctx->noise_present) {
ctx->noise_level_index = get_bits(gb, 4);
ctx->noise_table_index = get_bits(gb, 4);
}
return 0;
}
