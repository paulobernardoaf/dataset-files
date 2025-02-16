#include "avcodec.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "msmpeg4data.h"
#include "unary.h"
#include "vc1.h"
#include "vc1_pred.h"
#include "vc1acdata.h"
#include "vc1data.h"
#define MB_INTRA_VLC_BITS 9
#define DC_VLC_BITS 9
static const uint8_t offset_table[2][9] = {
{ 0, 1, 2, 4, 8, 16, 32, 64, 128 },
{ 0, 1, 3, 7, 15, 31, 63, 127, 255 },
};
static const int block_map[6] = {0, 2, 1, 3, 4, 5};
static inline void init_block_index(VC1Context *v)
{
MpegEncContext *s = &v->s;
ff_init_block_index(s);
if (v->field_mode && !(v->second_field ^ v->tff)) {
s->dest[0] += s->current_picture_ptr->f->linesize[0];
s->dest[1] += s->current_picture_ptr->f->linesize[1];
s->dest[2] += s->current_picture_ptr->f->linesize[2];
}
}
static void vc1_put_blocks_clamped(VC1Context *v, int put_signed)
{
MpegEncContext *s = &v->s;
uint8_t *dest;
int block_count = CONFIG_GRAY && (s->avctx->flags & AV_CODEC_FLAG_GRAY) ? 4 : 6;
int fieldtx = 0;
int i;
if (!s->first_slice_line && v->fcm != ILACE_FRAME) {
if (s->mb_x) {
for (i = 0; i < block_count; i++) {
if (i > 3 ? v->mb_type[0][s->block_index[i] - s->block_wrap[i] - 1] :
v->mb_type[0][s->block_index[i] - 2 * s->block_wrap[i] - 2]) {
dest = s->dest[0] + ((i & 2) - 4) * 4 * s->linesize + ((i & 1) - 2) * 8;
if (put_signed)
s->idsp.put_signed_pixels_clamped(v->block[v->topleft_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest,
i > 3 ? s->uvlinesize : s->linesize);
else
s->idsp.put_pixels_clamped(v->block[v->topleft_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize - 8 : dest,
i > 3 ? s->uvlinesize : s->linesize);
}
}
}
if (s->mb_x == v->end_mb_x - 1) {
for (i = 0; i < block_count; i++) {
if (i > 3 ? v->mb_type[0][s->block_index[i] - s->block_wrap[i]] :
v->mb_type[0][s->block_index[i] - 2 * s->block_wrap[i]]) {
dest = s->dest[0] + ((i & 2) - 4) * 4 * s->linesize + (i & 1) * 8;
if (put_signed)
s->idsp.put_signed_pixels_clamped(v->block[v->top_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest,
i > 3 ? s->uvlinesize : s->linesize);
else
s->idsp.put_pixels_clamped(v->block[v->top_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] - 8 * s->uvlinesize : dest,
i > 3 ? s->uvlinesize : s->linesize);
}
}
}
}
if (s->mb_y == s->end_mb_y - 1 || v->fcm == ILACE_FRAME) {
if (s->mb_x) {
if (v->fcm == ILACE_FRAME)
fieldtx = v->fieldtx_plane[s->mb_y * s->mb_stride + s->mb_x - 1];
for (i = 0; i < block_count; i++) {
if (i > 3 ? v->mb_type[0][s->block_index[i] - 1] :
v->mb_type[0][s->block_index[i] - 2]) {
if (fieldtx)
dest = s->dest[0] + ((i & 2) >> 1) * s->linesize + ((i & 1) - 2) * 8;
else
dest = s->dest[0] + (i & 2) * 4 * s->linesize + ((i & 1) - 2) * 8;
if (put_signed)
s->idsp.put_signed_pixels_clamped(v->block[v->left_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] - 8 : dest,
i > 3 ? s->uvlinesize : s->linesize << fieldtx);
else
s->idsp.put_pixels_clamped(v->block[v->left_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] - 8 : dest,
i > 3 ? s->uvlinesize : s->linesize << fieldtx);
}
}
}
if (s->mb_x == v->end_mb_x - 1) {
if (v->fcm == ILACE_FRAME)
fieldtx = v->fieldtx_plane[s->mb_y * s->mb_stride + s->mb_x];
for (i = 0; i < block_count; i++) {
if (v->mb_type[0][s->block_index[i]]) {
if (fieldtx)
dest = s->dest[0] + ((i & 2) >> 1) * s->linesize + (i & 1) * 8;
else
dest = s->dest[0] + (i & 2) * 4 * s->linesize + (i & 1) * 8;
if (put_signed)
s->idsp.put_signed_pixels_clamped(v->block[v->cur_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] : dest,
i > 3 ? s->uvlinesize : s->linesize << fieldtx);
else
s->idsp.put_pixels_clamped(v->block[v->cur_blk_idx][block_map[i]],
i > 3 ? s->dest[i - 3] : dest,
i > 3 ? s->uvlinesize : s->linesize << fieldtx);
}
}
}
}
}
#define inc_blk_idx(idx) do { idx++; if (idx >= v->n_allocated_blks) idx = 0; } while (0)
#define GET_MQUANT() if (v->dquantfrm) { int edges = 0; if (v->dqprofile == DQPROFILE_ALL_MBS) { if (v->dqbilevel) { mquant = (get_bits1(gb)) ? -v->altpq : v->pq; } else { mqdiff = get_bits(gb, 3); if (mqdiff != 7) mquant = -v->pq - mqdiff; else mquant = -get_bits(gb, 5); } } if (v->dqprofile == DQPROFILE_SINGLE_EDGE) edges = 1 << v->dqsbedge; else if (v->dqprofile == DQPROFILE_DOUBLE_EDGES) edges = (3 << v->dqsbedge) % 15; else if (v->dqprofile == DQPROFILE_FOUR_EDGES) edges = 15; if ((edges&1) && !s->mb_x) mquant = -v->altpq; if ((edges&2) && !s->mb_y) mquant = -v->altpq; if ((edges&4) && s->mb_x == (s->mb_width - 1)) mquant = -v->altpq; if ((edges&8) && s->mb_y == ((s->mb_height >> v->field_mode) - 1)) mquant = -v->altpq; if (!mquant || mquant > 31 || mquant < -31) { av_log(v->s.avctx, AV_LOG_ERROR, "Overriding invalid mquant %d\n", mquant); mquant = 1; } }
#define GET_MVDATA(_dmv_x, _dmv_y) index = 1 + get_vlc2(gb, ff_vc1_mv_diff_vlc[s->mv_table_index].table, VC1_MV_DIFF_VLC_BITS, 2); if (index > 36) { mb_has_coeffs = 1; index -= 37; } else mb_has_coeffs = 0; s->mb_intra = 0; if (!index) { _dmv_x = _dmv_y = 0; } else if (index == 35) { _dmv_x = get_bits(gb, v->k_x - 1 + s->quarter_sample); _dmv_y = get_bits(gb, v->k_y - 1 + s->quarter_sample); } else if (index == 36) { _dmv_x = 0; _dmv_y = 0; s->mb_intra = 1; } else { index1 = index % 6; _dmv_x = offset_table[1][index1]; val = size_table[index1] - (!s->quarter_sample && index1 == 5); if (val > 0) { val = get_bits(gb, val); sign = 0 - (val & 1); _dmv_x = (sign ^ ((val >> 1) + _dmv_x)) - sign; } index1 = index / 6; _dmv_y = offset_table[1][index1]; val = size_table[index1] - (!s->quarter_sample && index1 == 5); if (val > 0) { val = get_bits(gb, val); sign = 0 - (val & 1); _dmv_y = (sign ^ ((val >> 1) + _dmv_y)) - sign; } }
static av_always_inline void get_mvdata_interlaced(VC1Context *v, int *dmv_x,
int *dmv_y, int *pred_flag)
{
int index, index1;
int extend_x, extend_y;
GetBitContext *gb = &v->s.gb;
int bits, esc;
int val, sign;
if (v->numref) {
bits = VC1_2REF_MVDATA_VLC_BITS;
esc = 125;
} else {
bits = VC1_1REF_MVDATA_VLC_BITS;
esc = 71;
}
extend_x = v->dmvrange & 1;
extend_y = (v->dmvrange >> 1) & 1;
index = get_vlc2(gb, v->imv_vlc->table, bits, 3);
if (index == esc) {
*dmv_x = get_bits(gb, v->k_x);
*dmv_y = get_bits(gb, v->k_y);
if (v->numref) {
if (pred_flag)
*pred_flag = *dmv_y & 1;
*dmv_y = (*dmv_y + (*dmv_y & 1)) >> 1;
}
}
else {
av_assert0(index < esc);
index1 = (index + 1) % 9;
if (index1 != 0) {
val = get_bits(gb, index1 + extend_x);
sign = 0 - (val & 1);
*dmv_x = (sign ^ ((val >> 1) + offset_table[extend_x][index1])) - sign;
} else
*dmv_x = 0;
index1 = (index + 1) / 9;
if (index1 > v->numref) {
val = get_bits(gb, (index1 >> v->numref) + extend_y);
sign = 0 - (val & 1);
*dmv_y = (sign ^ ((val >> 1) + offset_table[extend_y][index1 >> v->numref])) - sign;
} else
*dmv_y = 0;
if (v->numref && pred_flag)
*pred_flag = index1 & 1;
}
}
static inline void vc1_b_mc(VC1Context *v, int dmv_x[2], int dmv_y[2],
int direct, int mode)
{
if (direct) {
ff_vc1_mc_1mv(v, 0);
ff_vc1_interp_mc(v);
return;
}
if (mode == BMV_TYPE_INTERPOLATED) {
ff_vc1_mc_1mv(v, 0);
ff_vc1_interp_mc(v);
return;
}
ff_vc1_mc_1mv(v, (mode == BMV_TYPE_BACKWARD));
}
static inline int vc1_i_pred_dc(MpegEncContext *s, int overlap, int pq, int n,
int16_t **dc_val_ptr, int *dir_ptr)
{
int a, b, c, wrap, pred, scale;
int16_t *dc_val;
static const uint16_t dcpred[32] = {
-1, 1024, 512, 341, 256, 205, 171, 146, 128,
114, 102, 93, 85, 79, 73, 68, 64,
60, 57, 54, 51, 49, 47, 45, 43,
41, 39, 38, 37, 35, 34, 33
};
if (n < 4) scale = s->y_dc_scale;
else scale = s->c_dc_scale;
wrap = s->block_wrap[n];
dc_val = s->dc_val[0] + s->block_index[n];
c = dc_val[ - 1];
b = dc_val[ - 1 - wrap];
a = dc_val[ - wrap];
if (pq < 9 || !overlap) {
if (s->first_slice_line && (n != 2 && n != 3))
b = a = dcpred[scale];
if (s->mb_x == 0 && (n != 1 && n != 3))
b = c = dcpred[scale];
} else {
if (s->first_slice_line && (n != 2 && n != 3))
b = a = 0;
if (s->mb_x == 0 && (n != 1 && n != 3))
b = c = 0;
}
if (abs(a - b) <= abs(b - c)) {
pred = c;
*dir_ptr = 1; 
} else {
pred = a;
*dir_ptr = 0; 
}
*dc_val_ptr = &dc_val[0];
return pred;
}
static inline int ff_vc1_pred_dc(MpegEncContext *s, int overlap, int pq, int n,
int a_avail, int c_avail,
int16_t **dc_val_ptr, int *dir_ptr)
{
int a, b, c, wrap, pred;
int16_t *dc_val;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int q1, q2 = 0;
int dqscale_index;
q1 = FFABS(s->current_picture.qscale_table[mb_pos]);
dqscale_index = s->y_dc_scale_table[q1] - 1;
if (dqscale_index < 0)
return 0;
wrap = s->block_wrap[n];
dc_val = s->dc_val[0] + s->block_index[n];
c = dc_val[ - 1];
b = dc_val[ - 1 - wrap];
a = dc_val[ - wrap];
if (c_avail && (n != 1 && n != 3)) {
q2 = FFABS(s->current_picture.qscale_table[mb_pos - 1]);
if (q2 && q2 != q1)
c = (int)((unsigned)c * s->y_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
}
if (a_avail && (n != 2 && n != 3)) {
q2 = FFABS(s->current_picture.qscale_table[mb_pos - s->mb_stride]);
if (q2 && q2 != q1)
a = (int)((unsigned)a * s->y_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
}
if (a_avail && c_avail && (n != 3)) {
int off = mb_pos;
if (n != 1)
off--;
if (n != 2)
off -= s->mb_stride;
q2 = FFABS(s->current_picture.qscale_table[off]);
if (q2 && q2 != q1)
b = (int)((unsigned)b * s->y_dc_scale_table[q2] * ff_vc1_dqscale[dqscale_index] + 0x20000) >> 18;
}
if (c_avail && (!a_avail || abs(a - b) <= abs(b - c))) {
pred = c;
*dir_ptr = 1; 
} else if (a_avail) {
pred = a;
*dir_ptr = 0; 
} else {
pred = 0;
*dir_ptr = 1; 
}
*dc_val_ptr = &dc_val[0];
return pred;
}
static inline int vc1_coded_block_pred(MpegEncContext * s, int n,
uint8_t **coded_block_ptr)
{
int xy, wrap, pred, a, b, c;
xy = s->block_index[n];
wrap = s->b8_stride;
a = s->coded_block[xy - 1 ];
b = s->coded_block[xy - 1 - wrap];
c = s->coded_block[xy - wrap];
if (b == c) {
pred = a;
} else {
pred = c;
}
*coded_block_ptr = &s->coded_block[xy];
return pred;
}
static int vc1_decode_ac_coeff(VC1Context *v, int *last, int *skip,
int *value, int codingset)
{
GetBitContext *gb = &v->s.gb;
int index, run, level, lst, sign;
index = get_vlc2(gb, ff_vc1_ac_coeff_table[codingset].table, AC_VLC_BITS, 3);
if (index < 0)
return index;
if (index != ff_vc1_ac_sizes[codingset] - 1) {
run = vc1_index_decode_table[codingset][index][0];
level = vc1_index_decode_table[codingset][index][1];
lst = index >= vc1_last_decode_table[codingset] || get_bits_left(gb) < 0;
sign = get_bits1(gb);
} else {
int escape = decode210(gb);
if (escape != 2) {
index = get_vlc2(gb, ff_vc1_ac_coeff_table[codingset].table, AC_VLC_BITS, 3);
if (index >= ff_vc1_ac_sizes[codingset] - 1U)
return AVERROR_INVALIDDATA;
run = vc1_index_decode_table[codingset][index][0];
level = vc1_index_decode_table[codingset][index][1];
lst = index >= vc1_last_decode_table[codingset];
if (escape == 0) {
if (lst)
level += vc1_last_delta_level_table[codingset][run];
else
level += vc1_delta_level_table[codingset][run];
} else {
if (lst)
run += vc1_last_delta_run_table[codingset][level] + 1;
else
run += vc1_delta_run_table[codingset][level] + 1;
}
sign = get_bits1(gb);
} else {
lst = get_bits1(gb);
if (v->s.esc3_level_length == 0) {
if (v->pq < 8 || v->dquantfrm) { 
v->s.esc3_level_length = get_bits(gb, 3);
if (!v->s.esc3_level_length)
v->s.esc3_level_length = get_bits(gb, 2) + 8;
} else { 
v->s.esc3_level_length = get_unary(gb, 1, 6) + 2;
}
v->s.esc3_run_length = 3 + get_bits(gb, 2);
}
run = get_bits(gb, v->s.esc3_run_length);
sign = get_bits1(gb);
level = get_bits(gb, v->s.esc3_level_length);
}
}
*last = lst;
*skip = run;
*value = (level ^ -sign) + sign;
return 0;
}
static int vc1_decode_i_block(VC1Context *v, int16_t block[64], int n,
int coded, int codingset)
{
GetBitContext *gb = &v->s.gb;
MpegEncContext *s = &v->s;
int dc_pred_dir = 0; 
int i;
int16_t *dc_val;
int16_t *ac_val, *ac_val2;
int dcdiff, scale;
if (n < 4) {
dcdiff = get_vlc2(&s->gb, ff_msmp4_dc_luma_vlc[s->dc_table_index].table, DC_VLC_BITS, 3);
} else {
dcdiff = get_vlc2(&s->gb, ff_msmp4_dc_chroma_vlc[s->dc_table_index].table, DC_VLC_BITS, 3);
}
if (dcdiff < 0) {
av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
return -1;
}
if (dcdiff) {
const int m = (v->pq == 1 || v->pq == 2) ? 3 - v->pq : 0;
if (dcdiff == 119 ) {
dcdiff = get_bits(gb, 8 + m);
} else {
if (m)
dcdiff = (dcdiff << m) + get_bits(gb, m) - ((1 << m) - 1);
}
if (get_bits1(gb))
dcdiff = -dcdiff;
}
dcdiff += vc1_i_pred_dc(&v->s, v->overlap, v->pq, n, &dc_val, &dc_pred_dir);
*dc_val = dcdiff;
if (n < 4)
scale = s->y_dc_scale;
else
scale = s->c_dc_scale;
block[0] = dcdiff * scale;
ac_val = s->ac_val[0][s->block_index[n]];
ac_val2 = ac_val;
if (dc_pred_dir) 
ac_val -= 16;
else 
ac_val -= 16 * s->block_wrap[n];
scale = v->pq * 2 + v->halfpq;
i = !!coded;
if (coded) {
int last = 0, skip, value;
const uint8_t *zz_table;
int k;
if (v->s.ac_pred) {
if (!dc_pred_dir)
zz_table = v->zz_8x8[2];
else
zz_table = v->zz_8x8[3];
} else
zz_table = v->zz_8x8[1];
while (!last) {
int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, codingset);
if (ret < 0)
return ret;
i += skip;
if (i > 63)
break;
block[zz_table[i++]] = value;
}
if (s->ac_pred) {
int sh;
if (dc_pred_dir) { 
sh = v->left_blk_sh;
} else { 
sh = v->top_blk_sh;
ac_val += 8;
}
for (k = 1; k < 8; k++)
block[k << sh] += ac_val[k];
}
for (k = 1; k < 8; k++) {
ac_val2[k] = block[k << v->left_blk_sh];
ac_val2[k + 8] = block[k << v->top_blk_sh];
}
for (k = 1; k < 64; k++)
if (block[k]) {
block[k] *= scale;
if (!v->pquantizer)
block[k] += (block[k] < 0) ? -v->pq : v->pq;
}
} else {
int k;
memset(ac_val2, 0, 16 * 2);
if (s->ac_pred) {
int sh;
if (dc_pred_dir) { 
sh = v->left_blk_sh;
} else { 
sh = v->top_blk_sh;
ac_val += 8;
ac_val2 += 8;
}
memcpy(ac_val2, ac_val, 8 * 2);
for (k = 1; k < 8; k++) {
block[k << sh] = ac_val[k] * scale;
if (!v->pquantizer && block[k << sh])
block[k << sh] += (block[k << sh] < 0) ? -v->pq : v->pq;
}
}
}
if (s->ac_pred) i = 63;
s->block_last_index[n] = i;
return 0;
}
static int vc1_decode_i_block_adv(VC1Context *v, int16_t block[64], int n,
int coded, int codingset, int mquant)
{
GetBitContext *gb = &v->s.gb;
MpegEncContext *s = &v->s;
int dc_pred_dir = 0; 
int i;
int16_t *dc_val = NULL;
int16_t *ac_val, *ac_val2;
int dcdiff;
int a_avail = v->a_avail, c_avail = v->c_avail;
int use_pred = s->ac_pred;
int scale;
int q1, q2 = 0;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int quant = FFABS(mquant);
if (n < 4) {
dcdiff = get_vlc2(&s->gb, ff_msmp4_dc_luma_vlc[s->dc_table_index].table, DC_VLC_BITS, 3);
} else {
dcdiff = get_vlc2(&s->gb, ff_msmp4_dc_chroma_vlc[s->dc_table_index].table, DC_VLC_BITS, 3);
}
if (dcdiff < 0) {
av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
return -1;
}
if (dcdiff) {
const int m = (quant == 1 || quant == 2) ? 3 - quant : 0;
if (dcdiff == 119 ) {
dcdiff = get_bits(gb, 8 + m);
} else {
if (m)
dcdiff = (dcdiff << m) + get_bits(gb, m) - ((1 << m) - 1);
}
if (get_bits1(gb))
dcdiff = -dcdiff;
}
dcdiff += ff_vc1_pred_dc(&v->s, v->overlap, quant, n, v->a_avail, v->c_avail, &dc_val, &dc_pred_dir);
*dc_val = dcdiff;
if (n < 4)
scale = s->y_dc_scale;
else
scale = s->c_dc_scale;
block[0] = dcdiff * scale;
if (!a_avail && !c_avail)
use_pred = 0;
scale = quant * 2 + ((mquant < 0) ? 0 : v->halfpq);
ac_val = s->ac_val[0][s->block_index[n]];
ac_val2 = ac_val;
if (dc_pred_dir) 
ac_val -= 16;
else 
ac_val -= 16 * s->block_wrap[n];
q1 = s->current_picture.qscale_table[mb_pos];
if (n == 3)
q2 = q1;
else if (dc_pred_dir) {
if (n == 1)
q2 = q1;
else if (c_avail && mb_pos)
q2 = s->current_picture.qscale_table[mb_pos - 1];
} else {
if (n == 2)
q2 = q1;
else if (a_avail && mb_pos >= s->mb_stride)
q2 = s->current_picture.qscale_table[mb_pos - s->mb_stride];
}
i = 1;
if (coded) {
int last = 0, skip, value;
const uint8_t *zz_table;
int k;
if (v->s.ac_pred) {
if (!use_pred && v->fcm == ILACE_FRAME) {
zz_table = v->zzi_8x8;
} else {
if (!dc_pred_dir) 
zz_table = v->zz_8x8[2];
else 
zz_table = v->zz_8x8[3];
}
} else {
if (v->fcm != ILACE_FRAME)
zz_table = v->zz_8x8[1];
else
zz_table = v->zzi_8x8;
}
while (!last) {
int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, codingset);
if (ret < 0)
return ret;
i += skip;
if (i > 63)
break;
block[zz_table[i++]] = value;
}
if (use_pred) {
int sh;
if (dc_pred_dir) { 
sh = v->left_blk_sh;
} else { 
sh = v->top_blk_sh;
ac_val += 8;
}
q1 = FFABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
if (q1 < 1)
return AVERROR_INVALIDDATA;
if (q2)
q2 = FFABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
if (q2 && q1 != q2) {
for (k = 1; k < 8; k++)
block[k << sh] += (int)(ac_val[k] * (unsigned)q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
} else {
for (k = 1; k < 8; k++)
block[k << sh] += ac_val[k];
}
}
for (k = 1; k < 8; k++) {
ac_val2[k ] = block[k << v->left_blk_sh];
ac_val2[k + 8] = block[k << v->top_blk_sh];
}
for (k = 1; k < 64; k++)
if (block[k]) {
block[k] *= scale;
if (!v->pquantizer)
block[k] += (block[k] < 0) ? -quant : quant;
}
} else { 
int k;
memset(ac_val2, 0, 16 * 2);
if (use_pred) {
int sh;
if (dc_pred_dir) { 
sh = v->left_blk_sh;
} else { 
sh = v->top_blk_sh;
ac_val += 8;
ac_val2 += 8;
}
memcpy(ac_val2, ac_val, 8 * 2);
q1 = FFABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
if (q1 < 1)
return AVERROR_INVALIDDATA;
if (q2)
q2 = FFABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
if (q2 && q1 != q2) {
for (k = 1; k < 8; k++)
ac_val2[k] = (int)(ac_val2[k] * q2 * (unsigned)ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
}
for (k = 1; k < 8; k++) {
block[k << sh] = ac_val2[k] * scale;
if (!v->pquantizer && block[k << sh])
block[k << sh] += (block[k << sh] < 0) ? -quant : quant;
}
}
}
if (use_pred) i = 63;
s->block_last_index[n] = i;
return 0;
}
static int vc1_decode_intra_block(VC1Context *v, int16_t block[64], int n,
int coded, int mquant, int codingset)
{
GetBitContext *gb = &v->s.gb;
MpegEncContext *s = &v->s;
int dc_pred_dir = 0; 
int i;
int16_t *dc_val = NULL;
int16_t *ac_val, *ac_val2;
int dcdiff;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int a_avail = v->a_avail, c_avail = v->c_avail;
int use_pred = s->ac_pred;
int scale;
int q1, q2 = 0;
int quant = FFABS(mquant);
s->bdsp.clear_block(block);
quant = av_clip_uintp2(quant, 5);
s->y_dc_scale = s->y_dc_scale_table[quant];
s->c_dc_scale = s->c_dc_scale_table[quant];
if (n < 4) {
dcdiff = get_vlc2(&s->gb, ff_msmp4_dc_luma_vlc[s->dc_table_index].table, DC_VLC_BITS, 3);
} else {
dcdiff = get_vlc2(&s->gb, ff_msmp4_dc_chroma_vlc[s->dc_table_index].table, DC_VLC_BITS, 3);
}
if (dcdiff < 0) {
av_log(s->avctx, AV_LOG_ERROR, "Illegal DC VLC\n");
return -1;
}
if (dcdiff) {
const int m = (quant == 1 || quant == 2) ? 3 - quant : 0;
if (dcdiff == 119 ) {
dcdiff = get_bits(gb, 8 + m);
} else {
if (m)
dcdiff = (dcdiff << m) + get_bits(gb, m) - ((1 << m) - 1);
}
if (get_bits1(gb))
dcdiff = -dcdiff;
}
dcdiff += ff_vc1_pred_dc(&v->s, v->overlap, quant, n, a_avail, c_avail, &dc_val, &dc_pred_dir);
*dc_val = dcdiff;
if (n < 4) {
block[0] = dcdiff * s->y_dc_scale;
} else {
block[0] = dcdiff * s->c_dc_scale;
}
i = 1;
if (!a_avail) dc_pred_dir = 1;
if (!c_avail) dc_pred_dir = 0;
if (!a_avail && !c_avail) use_pred = 0;
ac_val = s->ac_val[0][s->block_index[n]];
ac_val2 = ac_val;
scale = quant * 2 + ((mquant < 0) ? 0 : v->halfpq);
if (dc_pred_dir) 
ac_val -= 16;
else 
ac_val -= 16 * s->block_wrap[n];
q1 = s->current_picture.qscale_table[mb_pos];
if (dc_pred_dir && c_avail && mb_pos)
q2 = s->current_picture.qscale_table[mb_pos - 1];
if (!dc_pred_dir && a_avail && mb_pos >= s->mb_stride)
q2 = s->current_picture.qscale_table[mb_pos - s->mb_stride];
if (dc_pred_dir && n == 1)
q2 = q1;
if (!dc_pred_dir && n == 2)
q2 = q1;
if (n == 3) q2 = q1;
if (coded) {
int last = 0, skip, value;
int k;
while (!last) {
int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, codingset);
if (ret < 0)
return ret;
i += skip;
if (i > 63)
break;
if (v->fcm == PROGRESSIVE)
block[v->zz_8x8[0][i++]] = value;
else {
if (use_pred && (v->fcm == ILACE_FRAME)) {
if (!dc_pred_dir) 
block[v->zz_8x8[2][i++]] = value;
else 
block[v->zz_8x8[3][i++]] = value;
} else {
block[v->zzi_8x8[i++]] = value;
}
}
}
if (use_pred) {
q1 = FFABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
if (q1 < 1)
return AVERROR_INVALIDDATA;
if (q2)
q2 = FFABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
if (q2 && q1 != q2) {
if (dc_pred_dir) { 
for (k = 1; k < 8; k++)
block[k << v->left_blk_sh] += (int)(ac_val[k] * q2 * (unsigned)ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
} else { 
for (k = 1; k < 8; k++)
block[k << v->top_blk_sh] += (int)(ac_val[k + 8] * q2 * (unsigned)ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
}
} else {
if (dc_pred_dir) { 
for (k = 1; k < 8; k++)
block[k << v->left_blk_sh] += ac_val[k];
} else { 
for (k = 1; k < 8; k++)
block[k << v->top_blk_sh] += ac_val[k + 8];
}
}
}
for (k = 1; k < 8; k++) {
ac_val2[k ] = block[k << v->left_blk_sh];
ac_val2[k + 8] = block[k << v->top_blk_sh];
}
for (k = 1; k < 64; k++)
if (block[k]) {
block[k] *= scale;
if (!v->pquantizer)
block[k] += (block[k] < 0) ? -quant : quant;
}
if (use_pred) i = 63;
} else { 
int k;
memset(ac_val2, 0, 16 * 2);
if (dc_pred_dir) { 
if (use_pred) {
memcpy(ac_val2, ac_val, 8 * 2);
q1 = FFABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
if (q1 < 1)
return AVERROR_INVALIDDATA;
if (q2)
q2 = FFABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
if (q2 && q1 != q2) {
for (k = 1; k < 8; k++)
ac_val2[k] = (ac_val2[k] * q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
}
}
} else { 
if (use_pred) {
memcpy(ac_val2 + 8, ac_val + 8, 8 * 2);
q1 = FFABS(q1) * 2 + ((q1 < 0) ? 0 : v->halfpq) - 1;
if (q1 < 1)
return AVERROR_INVALIDDATA;
if (q2)
q2 = FFABS(q2) * 2 + ((q2 < 0) ? 0 : v->halfpq) - 1;
if (q2 && q1 != q2) {
for (k = 1; k < 8; k++)
ac_val2[k + 8] = (ac_val2[k + 8] * q2 * ff_vc1_dqscale[q1 - 1] + 0x20000) >> 18;
}
}
}
if (use_pred) {
if (dc_pred_dir) { 
for (k = 1; k < 8; k++) {
block[k << v->left_blk_sh] = ac_val2[k] * scale;
if (!v->pquantizer && block[k << v->left_blk_sh])
block[k << v->left_blk_sh] += (block[k << v->left_blk_sh] < 0) ? -quant : quant;
}
} else { 
for (k = 1; k < 8; k++) {
block[k << v->top_blk_sh] = ac_val2[k + 8] * scale;
if (!v->pquantizer && block[k << v->top_blk_sh])
block[k << v->top_blk_sh] += (block[k << v->top_blk_sh] < 0) ? -quant : quant;
}
}
i = 63;
}
}
s->block_last_index[n] = i;
return 0;
}
static int vc1_decode_p_block(VC1Context *v, int16_t block[64], int n,
int mquant, int ttmb, int first_block,
uint8_t *dst, int linesize, int skip_block,
int *ttmb_out)
{
MpegEncContext *s = &v->s;
GetBitContext *gb = &s->gb;
int i, j;
int subblkpat = 0;
int scale, off, idx, last, skip, value;
int ttblk = ttmb & 7;
int pat = 0;
int quant = FFABS(mquant);
s->bdsp.clear_block(block);
if (ttmb == -1) {
ttblk = ff_vc1_ttblk_to_tt[v->tt_index][get_vlc2(gb, ff_vc1_ttblk_vlc[v->tt_index].table, VC1_TTBLK_VLC_BITS, 1)];
}
if (ttblk == TT_4X4) {
subblkpat = ~(get_vlc2(gb, ff_vc1_subblkpat_vlc[v->tt_index].table, VC1_SUBBLKPAT_VLC_BITS, 1) + 1);
}
if ((ttblk != TT_8X8 && ttblk != TT_4X4)
&& ((v->ttmbf || (ttmb != -1 && (ttmb & 8) && !first_block))
|| (!v->res_rtm_flag && !first_block))) {
subblkpat = decode012(gb);
if (subblkpat)
subblkpat ^= 3; 
if (ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM)
ttblk = TT_8X4;
if (ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT)
ttblk = TT_4X8;
}
scale = quant * 2 + ((mquant < 0) ? 0 : v->halfpq);
if (ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM) {
subblkpat = 2 - (ttblk == TT_8X4_TOP);
ttblk = TT_8X4;
}
if (ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT) {
subblkpat = 2 - (ttblk == TT_4X8_LEFT);
ttblk = TT_4X8;
}
switch (ttblk) {
case TT_8X8:
pat = 0xF;
i = 0;
last = 0;
while (!last) {
int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
if (ret < 0)
return ret;
i += skip;
if (i > 63)
break;
if (!v->fcm)
idx = v->zz_8x8[0][i++];
else
idx = v->zzi_8x8[i++];
block[idx] = value * scale;
if (!v->pquantizer)
block[idx] += (block[idx] < 0) ? -quant : quant;
}
if (!skip_block) {
if (i == 1)
v->vc1dsp.vc1_inv_trans_8x8_dc(dst, linesize, block);
else {
v->vc1dsp.vc1_inv_trans_8x8(block);
s->idsp.add_pixels_clamped(block, dst, linesize);
}
}
break;
case TT_4X4:
pat = ~subblkpat & 0xF;
for (j = 0; j < 4; j++) {
last = subblkpat & (1 << (3 - j));
i = 0;
off = (j & 1) * 4 + (j & 2) * 16;
while (!last) {
int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
if (ret < 0)
return ret;
i += skip;
if (i > 15)
break;
if (!v->fcm)
idx = ff_vc1_simple_progressive_4x4_zz[i++];
else
idx = ff_vc1_adv_interlaced_4x4_zz[i++];
block[idx + off] = value * scale;
if (!v->pquantizer)
block[idx + off] += (block[idx + off] < 0) ? -quant : quant;
}
if (!(subblkpat & (1 << (3 - j))) && !skip_block) {
if (i == 1)
v->vc1dsp.vc1_inv_trans_4x4_dc(dst + (j & 1) * 4 + (j & 2) * 2 * linesize, linesize, block + off);
else
v->vc1dsp.vc1_inv_trans_4x4(dst + (j & 1) * 4 + (j & 2) * 2 * linesize, linesize, block + off);
}
}
break;
case TT_8X4:
pat = ~((subblkpat & 2) * 6 + (subblkpat & 1) * 3) & 0xF;
for (j = 0; j < 2; j++) {
last = subblkpat & (1 << (1 - j));
i = 0;
off = j * 32;
while (!last) {
int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
if (ret < 0)
return ret;
i += skip;
if (i > 31)
break;
if (!v->fcm)
idx = v->zz_8x4[i++] + off;
else
idx = ff_vc1_adv_interlaced_8x4_zz[i++] + off;
block[idx] = value * scale;
if (!v->pquantizer)
block[idx] += (block[idx] < 0) ? -quant : quant;
}
if (!(subblkpat & (1 << (1 - j))) && !skip_block) {
if (i == 1)
v->vc1dsp.vc1_inv_trans_8x4_dc(dst + j * 4 * linesize, linesize, block + off);
else
v->vc1dsp.vc1_inv_trans_8x4(dst + j * 4 * linesize, linesize, block + off);
}
}
break;
case TT_4X8:
pat = ~(subblkpat * 5) & 0xF;
for (j = 0; j < 2; j++) {
last = subblkpat & (1 << (1 - j));
i = 0;
off = j * 4;
while (!last) {
int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
if (ret < 0)
return ret;
i += skip;
if (i > 31)
break;
if (!v->fcm)
idx = v->zz_4x8[i++] + off;
else
idx = ff_vc1_adv_interlaced_4x8_zz[i++] + off;
block[idx] = value * scale;
if (!v->pquantizer)
block[idx] += (block[idx] < 0) ? -quant : quant;
}
if (!(subblkpat & (1 << (1 - j))) && !skip_block) {
if (i == 1)
v->vc1dsp.vc1_inv_trans_4x8_dc(dst + j * 4, linesize, block + off);
else
v->vc1dsp.vc1_inv_trans_4x8(dst + j*4, linesize, block + off);
}
}
break;
}
if (ttmb_out)
*ttmb_out |= ttblk << (n * 4);
return pat;
}
static const uint8_t size_table[6] = { 0, 2, 3, 4, 5, 8 };
static int vc1_decode_p_mb(VC1Context *v)
{
MpegEncContext *s = &v->s;
GetBitContext *gb = &s->gb;
int i, j;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int cbp; 
int mqdiff, mquant; 
int ttmb = v->ttfrm; 
int mb_has_coeffs = 1; 
int dmv_x, dmv_y; 
int index, index1; 
int val, sign; 
int first_block = 1;
int dst_idx, off;
int skipped, fourmv;
int block_cbp = 0, pat, block_tt = 0, block_intra = 0;
mquant = v->pq; 
if (v->mv_type_is_raw)
fourmv = get_bits1(gb);
else
fourmv = v->mv_type_mb_plane[mb_pos];
if (v->skip_is_raw)
skipped = get_bits1(gb);
else
skipped = v->s.mbskip_table[mb_pos];
if (!fourmv) { 
if (!skipped) {
GET_MVDATA(dmv_x, dmv_y);
if (s->mb_intra) {
s->current_picture.motion_val[1][s->block_index[0]][0] = 0;
s->current_picture.motion_val[1][s->block_index[0]][1] = 0;
}
s->current_picture.mb_type[mb_pos] = s->mb_intra ? MB_TYPE_INTRA : MB_TYPE_16x16;
ff_vc1_pred_mv(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type[0], 0, 0);
if (s->mb_intra && !mb_has_coeffs) {
GET_MQUANT();
s->ac_pred = get_bits1(gb);
cbp = 0;
} else if (mb_has_coeffs) {
if (s->mb_intra)
s->ac_pred = get_bits1(gb);
cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
GET_MQUANT();
} else {
mquant = v->pq;
cbp = 0;
}
s->current_picture.qscale_table[mb_pos] = mquant;
if (!v->ttmbf && !s->mb_intra && mb_has_coeffs)
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table,
VC1_TTMB_VLC_BITS, 2);
if (!s->mb_intra) ff_vc1_mc_1mv(v, 0);
dst_idx = 0;
for (i = 0; i < 6; i++) {
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
v->mb_type[0][s->block_index[i]] = s->mb_intra;
if (s->mb_intra) {
v->a_avail = v->c_avail = 0;
if (i == 2 || i == 3 || !s->first_slice_line)
v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
if (i == 1 || i == 3 || s->mb_x)
v->c_avail = v->mb_type[0][s->block_index[i] - 1];
vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, val, mquant,
(i & 4) ? v->codingset2 : v->codingset);
if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
if (v->rangeredfrm)
for (j = 0; j < 64; j++)
v->block[v->cur_blk_idx][block_map[i]][j] *= 2;
block_cbp |= 0xF << (i << 2);
block_intra |= 1 << i;
} else if (val) {
pat = vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb, first_block,
s->dest[dst_idx] + off, (i & 4) ? s->uvlinesize : s->linesize,
CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
if (pat < 0)
return pat;
block_cbp |= pat << (i << 2);
if (!v->ttmbf && ttmb < 8)
ttmb = -1;
first_block = 0;
}
}
} else { 
s->mb_intra = 0;
for (i = 0; i < 6; i++) {
v->mb_type[0][s->block_index[i]] = 0;
s->dc_val[0][s->block_index[i]] = 0;
}
s->current_picture.mb_type[mb_pos] = MB_TYPE_SKIP;
s->current_picture.qscale_table[mb_pos] = 0;
ff_vc1_pred_mv(v, 0, 0, 0, 1, v->range_x, v->range_y, v->mb_type[0], 0, 0);
ff_vc1_mc_1mv(v, 0);
}
} else { 
if (!skipped ) {
int intra_count = 0, coded_inter = 0;
int is_intra[6], is_coded[6];
cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
for (i = 0; i < 6; i++) {
val = ((cbp >> (5 - i)) & 1);
s->dc_val[0][s->block_index[i]] = 0;
s->mb_intra = 0;
if (i < 4) {
dmv_x = dmv_y = 0;
s->mb_intra = 0;
mb_has_coeffs = 0;
if (val) {
GET_MVDATA(dmv_x, dmv_y);
}
ff_vc1_pred_mv(v, i, dmv_x, dmv_y, 0, v->range_x, v->range_y, v->mb_type[0], 0, 0);
if (!s->mb_intra)
ff_vc1_mc_4mv_luma(v, i, 0, 0);
intra_count += s->mb_intra;
is_intra[i] = s->mb_intra;
is_coded[i] = mb_has_coeffs;
}
if (i & 4) {
is_intra[i] = (intra_count >= 3);
is_coded[i] = val;
}
if (i == 4)
ff_vc1_mc_4mv_chroma(v, 0);
v->mb_type[0][s->block_index[i]] = is_intra[i];
if (!coded_inter)
coded_inter = !is_intra[i] & is_coded[i];
}
dst_idx = 0;
if (!intra_count && !coded_inter)
goto end;
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
{
int intrapred = 0;
for (i = 0; i < 6; i++)
if (is_intra[i]) {
if (((!s->first_slice_line || (i == 2 || i == 3)) && v->mb_type[0][s->block_index[i] - s->block_wrap[i]])
|| ((s->mb_x || (i == 1 || i == 3)) && v->mb_type[0][s->block_index[i] - 1])) {
intrapred = 1;
break;
}
}
if (intrapred)
s->ac_pred = get_bits1(gb);
else
s->ac_pred = 0;
}
if (!v->ttmbf && coded_inter)
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
for (i = 0; i < 6; i++) {
dst_idx += i >> 2;
off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
s->mb_intra = is_intra[i];
if (is_intra[i]) {
v->a_avail = v->c_avail = 0;
if (i == 2 || i == 3 || !s->first_slice_line)
v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
if (i == 1 || i == 3 || s->mb_x)
v->c_avail = v->mb_type[0][s->block_index[i] - 1];
vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, is_coded[i], mquant,
(i & 4) ? v->codingset2 : v->codingset);
if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
if (v->rangeredfrm)
for (j = 0; j < 64; j++)
v->block[v->cur_blk_idx][block_map[i]][j] *= 2;
block_cbp |= 0xF << (i << 2);
block_intra |= 1 << i;
} else if (is_coded[i]) {
pat = vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb,
first_block, s->dest[dst_idx] + off,
(i & 4) ? s->uvlinesize : s->linesize,
CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY),
&block_tt);
if (pat < 0)
return pat;
block_cbp |= pat << (i << 2);
if (!v->ttmbf && ttmb < 8)
ttmb = -1;
first_block = 0;
}
}
} else { 
s->mb_intra = 0;
s->current_picture.qscale_table[mb_pos] = 0;
for (i = 0; i < 6; i++) {
v->mb_type[0][s->block_index[i]] = 0;
s->dc_val[0][s->block_index[i]] = 0;
}
for (i = 0; i < 4; i++) {
ff_vc1_pred_mv(v, i, 0, 0, 0, v->range_x, v->range_y, v->mb_type[0], 0, 0);
ff_vc1_mc_4mv_luma(v, i, 0, 0);
}
ff_vc1_mc_4mv_chroma(v, 0);
s->current_picture.qscale_table[mb_pos] = 0;
}
}
end:
if (v->overlap && v->pq >= 9)
ff_vc1_p_overlap_filter(v);
vc1_put_blocks_clamped(v, 1);
v->cbp[s->mb_x] = block_cbp;
v->ttblk[s->mb_x] = block_tt;
v->is_intra[s->mb_x] = block_intra;
return 0;
}
static int vc1_decode_p_mb_intfr(VC1Context *v)
{
MpegEncContext *s = &v->s;
GetBitContext *gb = &s->gb;
int i;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int cbp = 0; 
int mqdiff, mquant; 
int ttmb = v->ttfrm; 
int mb_has_coeffs = 1; 
int dmv_x, dmv_y; 
int val; 
int first_block = 1;
int dst_idx, off;
int skipped, fourmv = 0, twomv = 0;
int block_cbp = 0, pat, block_tt = 0;
int idx_mbmode = 0, mvbp;
int fieldtx;
mquant = v->pq; 
if (v->skip_is_raw)
skipped = get_bits1(gb);
else
skipped = v->s.mbskip_table[mb_pos];
if (!skipped) {
if (v->fourmvswitch)
idx_mbmode = get_vlc2(gb, v->mbmode_vlc->table, VC1_INTFR_4MV_MBMODE_VLC_BITS, 2); 
else
idx_mbmode = get_vlc2(gb, v->mbmode_vlc->table, VC1_INTFR_NON4MV_MBMODE_VLC_BITS, 2); 
switch (ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0]) {
case MV_PMODE_INTFR_4MV:
fourmv = 1;
v->blk_mv_type[s->block_index[0]] = 0;
v->blk_mv_type[s->block_index[1]] = 0;
v->blk_mv_type[s->block_index[2]] = 0;
v->blk_mv_type[s->block_index[3]] = 0;
break;
case MV_PMODE_INTFR_4MV_FIELD:
fourmv = 1;
v->blk_mv_type[s->block_index[0]] = 1;
v->blk_mv_type[s->block_index[1]] = 1;
v->blk_mv_type[s->block_index[2]] = 1;
v->blk_mv_type[s->block_index[3]] = 1;
break;
case MV_PMODE_INTFR_2MV_FIELD:
twomv = 1;
v->blk_mv_type[s->block_index[0]] = 1;
v->blk_mv_type[s->block_index[1]] = 1;
v->blk_mv_type[s->block_index[2]] = 1;
v->blk_mv_type[s->block_index[3]] = 1;
break;
case MV_PMODE_INTFR_1MV:
v->blk_mv_type[s->block_index[0]] = 0;
v->blk_mv_type[s->block_index[1]] = 0;
v->blk_mv_type[s->block_index[2]] = 0;
v->blk_mv_type[s->block_index[3]] = 0;
break;
}
if (ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_INTRA) { 
for (i = 0; i < 4; i++) {
s->current_picture.motion_val[1][s->block_index[i]][0] = 0;
s->current_picture.motion_val[1][s->block_index[i]][1] = 0;
}
v->is_intra[s->mb_x] = 0x3f; 
s->mb_intra = 1;
s->current_picture.mb_type[mb_pos] = MB_TYPE_INTRA;
fieldtx = v->fieldtx_plane[mb_pos] = get_bits1(gb);
mb_has_coeffs = get_bits1(gb);
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
s->y_dc_scale = s->y_dc_scale_table[FFABS(mquant)];
s->c_dc_scale = s->c_dc_scale_table[FFABS(mquant)];
dst_idx = 0;
for (i = 0; i < 6; i++) {
v->a_avail = v->c_avail = 0;
v->mb_type[0][s->block_index[i]] = 1;
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
if (i == 2 || i == 3 || !s->first_slice_line)
v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
if (i == 1 || i == 3 || s->mb_x)
v->c_avail = v->mb_type[0][s->block_index[i] - 1];
vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, val, mquant,
(i & 4) ? v->codingset2 : v->codingset);
if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
if (i < 4)
off = (fieldtx) ? ((i & 1) * 8) + ((i & 2) >> 1) * s->linesize : (i & 1) * 8 + 4 * (i & 2) * s->linesize;
else
off = 0;
block_cbp |= 0xf << (i << 2);
}
} else { 
mb_has_coeffs = ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][3];
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
if (ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_2MV_FIELD) {
v->twomvbp = get_vlc2(gb, v->twomvbp_vlc->table, VC1_2MV_BLOCK_PATTERN_VLC_BITS, 1);
} else {
if ((ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_4MV)
|| (ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][0] == MV_PMODE_INTFR_4MV_FIELD)) {
v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc->table, VC1_4MV_BLOCK_PATTERN_VLC_BITS, 1);
}
}
s->mb_intra = v->is_intra[s->mb_x] = 0;
for (i = 0; i < 6; i++)
v->mb_type[0][s->block_index[i]] = 0;
fieldtx = v->fieldtx_plane[mb_pos] = ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][1];
dst_idx = 0;
if (fourmv) {
mvbp = v->fourmvbp;
for (i = 0; i < 4; i++) {
dmv_x = dmv_y = 0;
if (mvbp & (8 >> i))
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
ff_vc1_pred_mv_intfr(v, i, dmv_x, dmv_y, 0, v->range_x, v->range_y, v->mb_type[0], 0);
ff_vc1_mc_4mv_luma(v, i, 0, 0);
}
ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
} else if (twomv) {
mvbp = v->twomvbp;
dmv_x = dmv_y = 0;
if (mvbp & 2) {
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
}
ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 2, v->range_x, v->range_y, v->mb_type[0], 0);
ff_vc1_mc_4mv_luma(v, 0, 0, 0);
ff_vc1_mc_4mv_luma(v, 1, 0, 0);
dmv_x = dmv_y = 0;
if (mvbp & 1) {
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
}
ff_vc1_pred_mv_intfr(v, 2, dmv_x, dmv_y, 2, v->range_x, v->range_y, v->mb_type[0], 0);
ff_vc1_mc_4mv_luma(v, 2, 0, 0);
ff_vc1_mc_4mv_luma(v, 3, 0, 0);
ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
} else {
mvbp = ff_vc1_mbmode_intfrp[v->fourmvswitch][idx_mbmode][2];
dmv_x = dmv_y = 0;
if (mvbp) {
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
}
ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type[0], 0);
ff_vc1_mc_1mv(v, 0);
}
if (cbp)
GET_MQUANT(); 
s->current_picture.qscale_table[mb_pos] = mquant;
if (!v->ttmbf && cbp)
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
for (i = 0; i < 6; i++) {
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
if (!fieldtx)
off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
else
off = (i & 4) ? 0 : ((i & 1) * 8 + ((i > 1) * s->linesize));
if (val) {
pat = vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb,
first_block, s->dest[dst_idx] + off,
(i & 4) ? s->uvlinesize : (s->linesize << fieldtx),
CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
if (pat < 0)
return pat;
block_cbp |= pat << (i << 2);
if (!v->ttmbf && ttmb < 8)
ttmb = -1;
first_block = 0;
}
}
}
} else { 
s->mb_intra = v->is_intra[s->mb_x] = 0;
for (i = 0; i < 6; i++) {
v->mb_type[0][s->block_index[i]] = 0;
s->dc_val[0][s->block_index[i]] = 0;
}
s->current_picture.mb_type[mb_pos] = MB_TYPE_SKIP;
s->current_picture.qscale_table[mb_pos] = 0;
v->blk_mv_type[s->block_index[0]] = 0;
v->blk_mv_type[s->block_index[1]] = 0;
v->blk_mv_type[s->block_index[2]] = 0;
v->blk_mv_type[s->block_index[3]] = 0;
ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, v->mb_type[0], 0);
ff_vc1_mc_1mv(v, 0);
v->fieldtx_plane[mb_pos] = 0;
}
if (v->overlap && v->pq >= 9)
ff_vc1_p_overlap_filter(v);
vc1_put_blocks_clamped(v, 1);
v->cbp[s->mb_x] = block_cbp;
v->ttblk[s->mb_x] = block_tt;
return 0;
}
static int vc1_decode_p_mb_intfi(VC1Context *v)
{
MpegEncContext *s = &v->s;
GetBitContext *gb = &s->gb;
int i;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int cbp = 0; 
int mqdiff, mquant; 
int ttmb = v->ttfrm; 
int mb_has_coeffs = 1; 
int dmv_x, dmv_y; 
int val; 
int first_block = 1;
int dst_idx, off;
int pred_flag = 0;
int block_cbp = 0, pat, block_tt = 0;
int idx_mbmode = 0;
mquant = v->pq; 
idx_mbmode = get_vlc2(gb, v->mbmode_vlc->table, VC1_IF_MBMODE_VLC_BITS, 2);
if (idx_mbmode <= 1) { 
v->is_intra[s->mb_x] = 0x3f; 
s->mb_intra = 1;
s->current_picture.motion_val[1][s->block_index[0] + v->blocks_off][0] = 0;
s->current_picture.motion_val[1][s->block_index[0] + v->blocks_off][1] = 0;
s->current_picture.mb_type[mb_pos + v->mb_off] = MB_TYPE_INTRA;
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
s->y_dc_scale = s->y_dc_scale_table[FFABS(mquant)];
s->c_dc_scale = s->c_dc_scale_table[FFABS(mquant)];
v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
mb_has_coeffs = idx_mbmode & 1;
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_ICBPCY_VLC_BITS, 2);
dst_idx = 0;
for (i = 0; i < 6; i++) {
v->a_avail = v->c_avail = 0;
v->mb_type[0][s->block_index[i]] = 1;
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
if (i == 2 || i == 3 || !s->first_slice_line)
v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
if (i == 1 || i == 3 || s->mb_x)
v->c_avail = v->mb_type[0][s->block_index[i] - 1];
vc1_decode_intra_block(v, v->block[v->cur_blk_idx][block_map[i]], i, val, mquant,
(i & 4) ? v->codingset2 : v->codingset);
if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[i]]);
off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
block_cbp |= 0xf << (i << 2);
}
} else {
s->mb_intra = v->is_intra[s->mb_x] = 0;
s->current_picture.mb_type[mb_pos + v->mb_off] = MB_TYPE_16x16;
for (i = 0; i < 6; i++)
v->mb_type[0][s->block_index[i]] = 0;
if (idx_mbmode <= 5) { 
dmv_x = dmv_y = pred_flag = 0;
if (idx_mbmode & 1) {
get_mvdata_interlaced(v, &dmv_x, &dmv_y, &pred_flag);
}
ff_vc1_pred_mv(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type[0], pred_flag, 0);
ff_vc1_mc_1mv(v, 0);
mb_has_coeffs = !(idx_mbmode & 2);
} else { 
v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc->table, VC1_4MV_BLOCK_PATTERN_VLC_BITS, 1);
for (i = 0; i < 4; i++) {
dmv_x = dmv_y = pred_flag = 0;
if (v->fourmvbp & (8 >> i))
get_mvdata_interlaced(v, &dmv_x, &dmv_y, &pred_flag);
ff_vc1_pred_mv(v, i, dmv_x, dmv_y, 0, v->range_x, v->range_y, v->mb_type[0], pred_flag, 0);
ff_vc1_mc_4mv_luma(v, i, 0, 0);
}
ff_vc1_mc_4mv_chroma(v, 0);
mb_has_coeffs = idx_mbmode & 1;
}
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
if (cbp) {
GET_MQUANT();
}
s->current_picture.qscale_table[mb_pos] = mquant;
if (!v->ttmbf && cbp) {
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
}
dst_idx = 0;
for (i = 0; i < 6; i++) {
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
off = (i & 4) ? 0 : (i & 1) * 8 + (i & 2) * 4 * s->linesize;
if (val) {
pat = vc1_decode_p_block(v, v->block[v->cur_blk_idx][block_map[i]], i, mquant, ttmb,
first_block, s->dest[dst_idx] + off,
(i & 4) ? s->uvlinesize : s->linesize,
CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY),
&block_tt);
if (pat < 0)
return pat;
block_cbp |= pat << (i << 2);
if (!v->ttmbf && ttmb < 8)
ttmb = -1;
first_block = 0;
}
}
}
if (v->overlap && v->pq >= 9)
ff_vc1_p_overlap_filter(v);
vc1_put_blocks_clamped(v, 1);
v->cbp[s->mb_x] = block_cbp;
v->ttblk[s->mb_x] = block_tt;
return 0;
}
static int vc1_decode_b_mb(VC1Context *v)
{
MpegEncContext *s = &v->s;
GetBitContext *gb = &s->gb;
int i, j;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int cbp = 0; 
int mqdiff, mquant; 
int ttmb = v->ttfrm; 
int mb_has_coeffs = 0; 
int index, index1; 
int val, sign; 
int first_block = 1;
int dst_idx, off;
int skipped, direct;
int dmv_x[2], dmv_y[2];
int bmvtype = BMV_TYPE_BACKWARD;
mquant = v->pq; 
s->mb_intra = 0;
if (v->dmb_is_raw)
direct = get_bits1(gb);
else
direct = v->direct_mb_plane[mb_pos];
if (v->skip_is_raw)
skipped = get_bits1(gb);
else
skipped = v->s.mbskip_table[mb_pos];
dmv_x[0] = dmv_x[1] = dmv_y[0] = dmv_y[1] = 0;
for (i = 0; i < 6; i++) {
v->mb_type[0][s->block_index[i]] = 0;
s->dc_val[0][s->block_index[i]] = 0;
}
s->current_picture.qscale_table[mb_pos] = 0;
if (!direct) {
if (!skipped) {
GET_MVDATA(dmv_x[0], dmv_y[0]);
dmv_x[1] = dmv_x[0];
dmv_y[1] = dmv_y[0];
}
if (skipped || !s->mb_intra) {
bmvtype = decode012(gb);
switch (bmvtype) {
case 0:
bmvtype = (v->bfraction >= (B_FRACTION_DEN/2)) ? BMV_TYPE_BACKWARD : BMV_TYPE_FORWARD;
break;
case 1:
bmvtype = (v->bfraction >= (B_FRACTION_DEN/2)) ? BMV_TYPE_FORWARD : BMV_TYPE_BACKWARD;
break;
case 2:
bmvtype = BMV_TYPE_INTERPOLATED;
dmv_x[0] = dmv_y[0] = 0;
}
}
}
for (i = 0; i < 6; i++)
v->mb_type[0][s->block_index[i]] = s->mb_intra;
if (skipped) {
if (direct)
bmvtype = BMV_TYPE_INTERPOLATED;
ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
return 0;
}
if (direct) {
cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
GET_MQUANT();
s->mb_intra = 0;
s->current_picture.qscale_table[mb_pos] = mquant;
if (!v->ttmbf)
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
dmv_x[0] = dmv_y[0] = dmv_x[1] = dmv_y[1] = 0;
ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
} else {
if (!mb_has_coeffs && !s->mb_intra) {
ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
return 0;
}
if (s->mb_intra && !mb_has_coeffs) {
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
s->ac_pred = get_bits1(gb);
cbp = 0;
ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
} else {
if (bmvtype == BMV_TYPE_INTERPOLATED) {
GET_MVDATA(dmv_x[0], dmv_y[0]);
if (!mb_has_coeffs) {
ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
return 0;
}
}
ff_vc1_pred_b_mv(v, dmv_x, dmv_y, direct, bmvtype);
if (!s->mb_intra) {
vc1_b_mc(v, dmv_x, dmv_y, direct, bmvtype);
}
if (s->mb_intra)
s->ac_pred = get_bits1(gb);
cbp = get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
if (!v->ttmbf && !s->mb_intra && mb_has_coeffs)
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
}
}
dst_idx = 0;
for (i = 0; i < 6; i++) {
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
v->mb_type[0][s->block_index[i]] = s->mb_intra;
if (s->mb_intra) {
v->a_avail = v->c_avail = 0;
if (i == 2 || i == 3 || !s->first_slice_line)
v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
if (i == 1 || i == 3 || s->mb_x)
v->c_avail = v->mb_type[0][s->block_index[i] - 1];
vc1_decode_intra_block(v, s->block[i], i, val, mquant,
(i & 4) ? v->codingset2 : v->codingset);
if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(s->block[i]);
if (v->rangeredfrm)
for (j = 0; j < 64; j++)
s->block[i][j] *= 2;
s->idsp.put_signed_pixels_clamped(s->block[i],
s->dest[dst_idx] + off,
i & 4 ? s->uvlinesize
: s->linesize);
} else if (val) {
int pat = vc1_decode_p_block(v, s->block[i], i, mquant, ttmb,
first_block, s->dest[dst_idx] + off,
(i & 4) ? s->uvlinesize : s->linesize,
CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), NULL);
if (pat < 0)
return pat;
if (!v->ttmbf && ttmb < 8)
ttmb = -1;
first_block = 0;
}
}
return 0;
}
static int vc1_decode_b_mb_intfi(VC1Context *v)
{
MpegEncContext *s = &v->s;
GetBitContext *gb = &s->gb;
int i, j;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int cbp = 0; 
int mqdiff, mquant; 
int ttmb = v->ttfrm; 
int mb_has_coeffs = 0; 
int val; 
int first_block = 1;
int dst_idx, off;
int fwd;
int dmv_x[2], dmv_y[2], pred_flag[2];
int bmvtype = BMV_TYPE_BACKWARD;
int block_cbp = 0, pat, block_tt = 0;
int idx_mbmode;
mquant = v->pq; 
s->mb_intra = 0;
idx_mbmode = get_vlc2(gb, v->mbmode_vlc->table, VC1_IF_MBMODE_VLC_BITS, 2);
if (idx_mbmode <= 1) { 
v->is_intra[s->mb_x] = 0x3f; 
s->mb_intra = 1;
s->current_picture.motion_val[1][s->block_index[0]][0] = 0;
s->current_picture.motion_val[1][s->block_index[0]][1] = 0;
s->current_picture.mb_type[mb_pos + v->mb_off] = MB_TYPE_INTRA;
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
s->y_dc_scale = s->y_dc_scale_table[FFABS(mquant)];
s->c_dc_scale = s->c_dc_scale_table[FFABS(mquant)];
v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
mb_has_coeffs = idx_mbmode & 1;
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_ICBPCY_VLC_BITS, 2);
dst_idx = 0;
for (i = 0; i < 6; i++) {
v->a_avail = v->c_avail = 0;
v->mb_type[0][s->block_index[i]] = 1;
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
if (i == 2 || i == 3 || !s->first_slice_line)
v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
if (i == 1 || i == 3 || s->mb_x)
v->c_avail = v->mb_type[0][s->block_index[i] - 1];
vc1_decode_intra_block(v, s->block[i], i, val, mquant,
(i & 4) ? v->codingset2 : v->codingset);
if (CONFIG_GRAY && (i > 3) && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(s->block[i]);
if (v->rangeredfrm)
for (j = 0; j < 64; j++)
s->block[i][j] <<= 1;
off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
s->idsp.put_signed_pixels_clamped(s->block[i],
s->dest[dst_idx] + off,
(i & 4) ? s->uvlinesize
: s->linesize);
}
} else {
s->mb_intra = v->is_intra[s->mb_x] = 0;
s->current_picture.mb_type[mb_pos + v->mb_off] = MB_TYPE_16x16;
for (i = 0; i < 6; i++)
v->mb_type[0][s->block_index[i]] = 0;
if (v->fmb_is_raw)
fwd = v->forward_mb_plane[mb_pos] = get_bits1(gb);
else
fwd = v->forward_mb_plane[mb_pos];
if (idx_mbmode <= 5) { 
int interpmvp = 0;
dmv_x[0] = dmv_x[1] = dmv_y[0] = dmv_y[1] = 0;
pred_flag[0] = pred_flag[1] = 0;
if (fwd)
bmvtype = BMV_TYPE_FORWARD;
else {
bmvtype = decode012(gb);
switch (bmvtype) {
case 0:
bmvtype = BMV_TYPE_BACKWARD;
break;
case 1:
bmvtype = BMV_TYPE_DIRECT;
break;
case 2:
bmvtype = BMV_TYPE_INTERPOLATED;
interpmvp = get_bits1(gb);
}
}
v->bmvtype = bmvtype;
if (bmvtype != BMV_TYPE_DIRECT && idx_mbmode & 1) {
get_mvdata_interlaced(v, &dmv_x[bmvtype == BMV_TYPE_BACKWARD], &dmv_y[bmvtype == BMV_TYPE_BACKWARD], &pred_flag[bmvtype == BMV_TYPE_BACKWARD]);
}
if (interpmvp) {
get_mvdata_interlaced(v, &dmv_x[1], &dmv_y[1], &pred_flag[1]);
}
if (bmvtype == BMV_TYPE_DIRECT) {
dmv_x[0] = dmv_y[0] = pred_flag[0] = 0;
dmv_x[1] = dmv_y[1] = pred_flag[0] = 0;
if (!s->next_picture_ptr->field_picture) {
av_log(s->avctx, AV_LOG_ERROR, "Mixed field/frame direct mode not supported\n");
return AVERROR_INVALIDDATA;
}
}
ff_vc1_pred_b_mv_intfi(v, 0, dmv_x, dmv_y, 1, pred_flag);
vc1_b_mc(v, dmv_x, dmv_y, (bmvtype == BMV_TYPE_DIRECT), bmvtype);
mb_has_coeffs = !(idx_mbmode & 2);
} else { 
if (fwd)
bmvtype = BMV_TYPE_FORWARD;
v->bmvtype = bmvtype;
v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc->table, VC1_4MV_BLOCK_PATTERN_VLC_BITS, 1);
for (i = 0; i < 4; i++) {
dmv_x[0] = dmv_y[0] = pred_flag[0] = 0;
dmv_x[1] = dmv_y[1] = pred_flag[1] = 0;
if (v->fourmvbp & (8 >> i)) {
get_mvdata_interlaced(v, &dmv_x[bmvtype == BMV_TYPE_BACKWARD],
&dmv_y[bmvtype == BMV_TYPE_BACKWARD],
&pred_flag[bmvtype == BMV_TYPE_BACKWARD]);
}
ff_vc1_pred_b_mv_intfi(v, i, dmv_x, dmv_y, 0, pred_flag);
ff_vc1_mc_4mv_luma(v, i, bmvtype == BMV_TYPE_BACKWARD, 0);
}
ff_vc1_mc_4mv_chroma(v, bmvtype == BMV_TYPE_BACKWARD);
mb_has_coeffs = idx_mbmode & 1;
}
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
if (cbp) {
GET_MQUANT();
}
s->current_picture.qscale_table[mb_pos] = mquant;
if (!v->ttmbf && cbp) {
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
}
dst_idx = 0;
for (i = 0; i < 6; i++) {
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
off = (i & 4) ? 0 : (i & 1) * 8 + (i & 2) * 4 * s->linesize;
if (val) {
pat = vc1_decode_p_block(v, s->block[i], i, mquant, ttmb,
first_block, s->dest[dst_idx] + off,
(i & 4) ? s->uvlinesize : s->linesize,
CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
if (pat < 0)
return pat;
block_cbp |= pat << (i << 2);
if (!v->ttmbf && ttmb < 8)
ttmb = -1;
first_block = 0;
}
}
}
v->cbp[s->mb_x] = block_cbp;
v->ttblk[s->mb_x] = block_tt;
return 0;
}
static int vc1_decode_b_mb_intfr(VC1Context *v)
{
MpegEncContext *s = &v->s;
GetBitContext *gb = &s->gb;
int i, j;
int mb_pos = s->mb_x + s->mb_y * s->mb_stride;
int cbp = 0; 
int mqdiff, mquant; 
int ttmb = v->ttfrm; 
int mvsw = 0; 
int mb_has_coeffs = 1; 
int dmv_x, dmv_y; 
int val; 
int first_block = 1;
int dst_idx, off;
int skipped, direct, twomv = 0;
int block_cbp = 0, pat, block_tt = 0;
int idx_mbmode = 0, mvbp;
int stride_y, fieldtx;
int bmvtype = BMV_TYPE_BACKWARD;
int dir, dir2;
mquant = v->pq; 
s->mb_intra = 0;
if (v->skip_is_raw)
skipped = get_bits1(gb);
else
skipped = v->s.mbskip_table[mb_pos];
if (!skipped) {
idx_mbmode = get_vlc2(gb, v->mbmode_vlc->table, VC1_INTFR_NON4MV_MBMODE_VLC_BITS, 2);
if (ff_vc1_mbmode_intfrp[0][idx_mbmode][0] == MV_PMODE_INTFR_2MV_FIELD) {
twomv = 1;
v->blk_mv_type[s->block_index[0]] = 1;
v->blk_mv_type[s->block_index[1]] = 1;
v->blk_mv_type[s->block_index[2]] = 1;
v->blk_mv_type[s->block_index[3]] = 1;
} else {
v->blk_mv_type[s->block_index[0]] = 0;
v->blk_mv_type[s->block_index[1]] = 0;
v->blk_mv_type[s->block_index[2]] = 0;
v->blk_mv_type[s->block_index[3]] = 0;
}
}
if (ff_vc1_mbmode_intfrp[0][idx_mbmode][0] == MV_PMODE_INTFR_INTRA) { 
for (i = 0; i < 4; i++) {
s->mv[0][i][0] = s->current_picture.motion_val[0][s->block_index[i]][0] = 0;
s->mv[0][i][1] = s->current_picture.motion_val[0][s->block_index[i]][1] = 0;
s->mv[1][i][0] = s->current_picture.motion_val[1][s->block_index[i]][0] = 0;
s->mv[1][i][1] = s->current_picture.motion_val[1][s->block_index[i]][1] = 0;
}
v->is_intra[s->mb_x] = 0x3f; 
s->mb_intra = 1;
s->current_picture.mb_type[mb_pos] = MB_TYPE_INTRA;
fieldtx = v->fieldtx_plane[mb_pos] = get_bits1(gb);
mb_has_coeffs = get_bits1(gb);
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
v->s.ac_pred = v->acpred_plane[mb_pos] = get_bits1(gb);
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
s->y_dc_scale = s->y_dc_scale_table[FFABS(mquant)];
s->c_dc_scale = s->c_dc_scale_table[FFABS(mquant)];
dst_idx = 0;
for (i = 0; i < 6; i++) {
v->a_avail = v->c_avail = 0;
v->mb_type[0][s->block_index[i]] = 1;
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
if (i == 2 || i == 3 || !s->first_slice_line)
v->a_avail = v->mb_type[0][s->block_index[i] - s->block_wrap[i]];
if (i == 1 || i == 3 || s->mb_x)
v->c_avail = v->mb_type[0][s->block_index[i] - 1];
vc1_decode_intra_block(v, s->block[i], i, val, mquant,
(i & 4) ? v->codingset2 : v->codingset);
if (CONFIG_GRAY && i > 3 && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(s->block[i]);
if (i < 4) {
stride_y = s->linesize << fieldtx;
off = (fieldtx) ? ((i & 1) * 8) + ((i & 2) >> 1) * s->linesize : (i & 1) * 8 + 4 * (i & 2) * s->linesize;
} else {
stride_y = s->uvlinesize;
off = 0;
}
s->idsp.put_signed_pixels_clamped(s->block[i],
s->dest[dst_idx] + off,
stride_y);
}
} else {
s->mb_intra = v->is_intra[s->mb_x] = 0;
if (v->dmb_is_raw)
direct = get_bits1(gb);
else
direct = v->direct_mb_plane[mb_pos];
if (direct) {
if (s->next_picture_ptr->field_picture)
av_log(s->avctx, AV_LOG_WARNING, "Mixed frame/field direct mode not supported\n");
s->mv[0][0][0] = s->current_picture.motion_val[0][s->block_index[0]][0] = scale_mv(s->next_picture.motion_val[1][s->block_index[0]][0], v->bfraction, 0, s->quarter_sample);
s->mv[0][0][1] = s->current_picture.motion_val[0][s->block_index[0]][1] = scale_mv(s->next_picture.motion_val[1][s->block_index[0]][1], v->bfraction, 0, s->quarter_sample);
s->mv[1][0][0] = s->current_picture.motion_val[1][s->block_index[0]][0] = scale_mv(s->next_picture.motion_val[1][s->block_index[0]][0], v->bfraction, 1, s->quarter_sample);
s->mv[1][0][1] = s->current_picture.motion_val[1][s->block_index[0]][1] = scale_mv(s->next_picture.motion_val[1][s->block_index[0]][1], v->bfraction, 1, s->quarter_sample);
if (twomv) {
s->mv[0][2][0] = s->current_picture.motion_val[0][s->block_index[2]][0] = scale_mv(s->next_picture.motion_val[1][s->block_index[2]][0], v->bfraction, 0, s->quarter_sample);
s->mv[0][2][1] = s->current_picture.motion_val[0][s->block_index[2]][1] = scale_mv(s->next_picture.motion_val[1][s->block_index[2]][1], v->bfraction, 0, s->quarter_sample);
s->mv[1][2][0] = s->current_picture.motion_val[1][s->block_index[2]][0] = scale_mv(s->next_picture.motion_val[1][s->block_index[2]][0], v->bfraction, 1, s->quarter_sample);
s->mv[1][2][1] = s->current_picture.motion_val[1][s->block_index[2]][1] = scale_mv(s->next_picture.motion_val[1][s->block_index[2]][1], v->bfraction, 1, s->quarter_sample);
for (i = 1; i < 4; i += 2) {
s->mv[0][i][0] = s->current_picture.motion_val[0][s->block_index[i]][0] = s->mv[0][i-1][0];
s->mv[0][i][1] = s->current_picture.motion_val[0][s->block_index[i]][1] = s->mv[0][i-1][1];
s->mv[1][i][0] = s->current_picture.motion_val[1][s->block_index[i]][0] = s->mv[1][i-1][0];
s->mv[1][i][1] = s->current_picture.motion_val[1][s->block_index[i]][1] = s->mv[1][i-1][1];
}
} else {
for (i = 1; i < 4; i++) {
s->mv[0][i][0] = s->current_picture.motion_val[0][s->block_index[i]][0] = s->mv[0][0][0];
s->mv[0][i][1] = s->current_picture.motion_val[0][s->block_index[i]][1] = s->mv[0][0][1];
s->mv[1][i][0] = s->current_picture.motion_val[1][s->block_index[i]][0] = s->mv[1][0][0];
s->mv[1][i][1] = s->current_picture.motion_val[1][s->block_index[i]][1] = s->mv[1][0][1];
}
}
}
if (!direct) {
if (skipped || !s->mb_intra) {
bmvtype = decode012(gb);
switch (bmvtype) {
case 0:
bmvtype = (v->bfraction >= (B_FRACTION_DEN/2)) ? BMV_TYPE_BACKWARD : BMV_TYPE_FORWARD;
break;
case 1:
bmvtype = (v->bfraction >= (B_FRACTION_DEN/2)) ? BMV_TYPE_FORWARD : BMV_TYPE_BACKWARD;
break;
case 2:
bmvtype = BMV_TYPE_INTERPOLATED;
}
}
if (twomv && bmvtype != BMV_TYPE_INTERPOLATED)
mvsw = get_bits1(gb);
}
if (!skipped) { 
mb_has_coeffs = ff_vc1_mbmode_intfrp[0][idx_mbmode][3];
if (mb_has_coeffs)
cbp = 1 + get_vlc2(&v->s.gb, v->cbpcy_vlc->table, VC1_CBPCY_P_VLC_BITS, 2);
if (!direct) {
if (bmvtype == BMV_TYPE_INTERPOLATED && twomv) {
v->fourmvbp = get_vlc2(gb, v->fourmvbp_vlc->table, VC1_4MV_BLOCK_PATTERN_VLC_BITS, 1);
} else if (bmvtype == BMV_TYPE_INTERPOLATED || twomv) {
v->twomvbp = get_vlc2(gb, v->twomvbp_vlc->table, VC1_2MV_BLOCK_PATTERN_VLC_BITS, 1);
}
}
for (i = 0; i < 6; i++)
v->mb_type[0][s->block_index[i]] = 0;
fieldtx = v->fieldtx_plane[mb_pos] = ff_vc1_mbmode_intfrp[0][idx_mbmode][1];
dst_idx = 0;
if (direct) {
if (twomv) {
for (i = 0; i < 4; i++) {
ff_vc1_mc_4mv_luma(v, i, 0, 0);
ff_vc1_mc_4mv_luma(v, i, 1, 1);
}
ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
ff_vc1_mc_4mv_chroma4(v, 1, 1, 1);
} else {
ff_vc1_mc_1mv(v, 0);
ff_vc1_interp_mc(v);
}
} else if (twomv && bmvtype == BMV_TYPE_INTERPOLATED) {
mvbp = v->fourmvbp;
for (i = 0; i < 4; i++) {
dir = i==1 || i==3;
dmv_x = dmv_y = 0;
val = ((mvbp >> (3 - i)) & 1);
if (val)
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
j = i > 1 ? 2 : 0;
ff_vc1_pred_mv_intfr(v, j, dmv_x, dmv_y, 2, v->range_x, v->range_y, v->mb_type[0], dir);
ff_vc1_mc_4mv_luma(v, j, dir, dir);
ff_vc1_mc_4mv_luma(v, j+1, dir, dir);
}
ff_vc1_mc_4mv_chroma4(v, 0, 0, 0);
ff_vc1_mc_4mv_chroma4(v, 1, 1, 1);
} else if (bmvtype == BMV_TYPE_INTERPOLATED) {
mvbp = v->twomvbp;
dmv_x = dmv_y = 0;
if (mvbp & 2)
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type[0], 0);
ff_vc1_mc_1mv(v, 0);
dmv_x = dmv_y = 0;
if (mvbp & 1)
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type[0], 1);
ff_vc1_interp_mc(v);
} else if (twomv) {
dir = bmvtype == BMV_TYPE_BACKWARD;
dir2 = dir;
if (mvsw)
dir2 = !dir;
mvbp = v->twomvbp;
dmv_x = dmv_y = 0;
if (mvbp & 2)
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 2, v->range_x, v->range_y, v->mb_type[0], dir);
dmv_x = dmv_y = 0;
if (mvbp & 1)
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
ff_vc1_pred_mv_intfr(v, 2, dmv_x, dmv_y, 2, v->range_x, v->range_y, v->mb_type[0], dir2);
if (mvsw) {
for (i = 0; i < 2; i++) {
s->mv[dir][i+2][0] = s->mv[dir][i][0] = s->current_picture.motion_val[dir][s->block_index[i+2]][0] = s->current_picture.motion_val[dir][s->block_index[i]][0];
s->mv[dir][i+2][1] = s->mv[dir][i][1] = s->current_picture.motion_val[dir][s->block_index[i+2]][1] = s->current_picture.motion_val[dir][s->block_index[i]][1];
s->mv[dir2][i+2][0] = s->mv[dir2][i][0] = s->current_picture.motion_val[dir2][s->block_index[i]][0] = s->current_picture.motion_val[dir2][s->block_index[i+2]][0];
s->mv[dir2][i+2][1] = s->mv[dir2][i][1] = s->current_picture.motion_val[dir2][s->block_index[i]][1] = s->current_picture.motion_val[dir2][s->block_index[i+2]][1];
}
} else {
ff_vc1_pred_mv_intfr(v, 0, 0, 0, 2, v->range_x, v->range_y, v->mb_type[0], !dir);
ff_vc1_pred_mv_intfr(v, 2, 0, 0, 2, v->range_x, v->range_y, v->mb_type[0], !dir);
}
ff_vc1_mc_4mv_luma(v, 0, dir, 0);
ff_vc1_mc_4mv_luma(v, 1, dir, 0);
ff_vc1_mc_4mv_luma(v, 2, dir2, 0);
ff_vc1_mc_4mv_luma(v, 3, dir2, 0);
ff_vc1_mc_4mv_chroma4(v, dir, dir2, 0);
} else {
dir = bmvtype == BMV_TYPE_BACKWARD;
mvbp = ff_vc1_mbmode_intfrp[0][idx_mbmode][2];
dmv_x = dmv_y = 0;
if (mvbp)
get_mvdata_interlaced(v, &dmv_x, &dmv_y, 0);
ff_vc1_pred_mv_intfr(v, 0, dmv_x, dmv_y, 1, v->range_x, v->range_y, v->mb_type[0], dir);
v->blk_mv_type[s->block_index[0]] = 1;
v->blk_mv_type[s->block_index[1]] = 1;
v->blk_mv_type[s->block_index[2]] = 1;
v->blk_mv_type[s->block_index[3]] = 1;
ff_vc1_pred_mv_intfr(v, 0, 0, 0, 2, v->range_x, v->range_y, 0, !dir);
for (i = 0; i < 2; i++) {
s->mv[!dir][i+2][0] = s->mv[!dir][i][0] = s->current_picture.motion_val[!dir][s->block_index[i+2]][0] = s->current_picture.motion_val[!dir][s->block_index[i]][0];
s->mv[!dir][i+2][1] = s->mv[!dir][i][1] = s->current_picture.motion_val[!dir][s->block_index[i+2]][1] = s->current_picture.motion_val[!dir][s->block_index[i]][1];
}
ff_vc1_mc_1mv(v, dir);
}
if (cbp)
GET_MQUANT(); 
s->current_picture.qscale_table[mb_pos] = mquant;
if (!v->ttmbf && cbp)
ttmb = get_vlc2(gb, ff_vc1_ttmb_vlc[v->tt_index].table, VC1_TTMB_VLC_BITS, 2);
for (i = 0; i < 6; i++) {
s->dc_val[0][s->block_index[i]] = 0;
dst_idx += i >> 2;
val = ((cbp >> (5 - i)) & 1);
if (!fieldtx)
off = (i & 4) ? 0 : ((i & 1) * 8 + (i & 2) * 4 * s->linesize);
else
off = (i & 4) ? 0 : ((i & 1) * 8 + ((i > 1) * s->linesize));
if (val) {
pat = vc1_decode_p_block(v, s->block[i], i, mquant, ttmb,
first_block, s->dest[dst_idx] + off,
(i & 4) ? s->uvlinesize : (s->linesize << fieldtx),
CONFIG_GRAY && (i & 4) && (s->avctx->flags & AV_CODEC_FLAG_GRAY), &block_tt);
if (pat < 0)
return pat;
block_cbp |= pat << (i << 2);
if (!v->ttmbf && ttmb < 8)
ttmb = -1;
first_block = 0;
}
}
} else { 
dir = 0;
for (i = 0; i < 6; i++) {
v->mb_type[0][s->block_index[i]] = 0;
s->dc_val[0][s->block_index[i]] = 0;
}
s->current_picture.mb_type[mb_pos] = MB_TYPE_SKIP;
s->current_picture.qscale_table[mb_pos] = 0;
v->blk_mv_type[s->block_index[0]] = 0;
v->blk_mv_type[s->block_index[1]] = 0;
v->blk_mv_type[s->block_index[2]] = 0;
v->blk_mv_type[s->block_index[3]] = 0;
if (!direct) {
if (bmvtype == BMV_TYPE_INTERPOLATED) {
ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, v->mb_type[0], 0);
ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, v->mb_type[0], 1);
} else {
dir = bmvtype == BMV_TYPE_BACKWARD;
ff_vc1_pred_mv_intfr(v, 0, 0, 0, 1, v->range_x, v->range_y, v->mb_type[0], dir);
if (mvsw) {
int dir2 = dir;
if (mvsw)
dir2 = !dir;
for (i = 0; i < 2; i++) {
s->mv[dir][i+2][0] = s->mv[dir][i][0] = s->current_picture.motion_val[dir][s->block_index[i+2]][0] = s->current_picture.motion_val[dir][s->block_index[i]][0];
s->mv[dir][i+2][1] = s->mv[dir][i][1] = s->current_picture.motion_val[dir][s->block_index[i+2]][1] = s->current_picture.motion_val[dir][s->block_index[i]][1];
s->mv[dir2][i+2][0] = s->mv[dir2][i][0] = s->current_picture.motion_val[dir2][s->block_index[i]][0] = s->current_picture.motion_val[dir2][s->block_index[i+2]][0];
s->mv[dir2][i+2][1] = s->mv[dir2][i][1] = s->current_picture.motion_val[dir2][s->block_index[i]][1] = s->current_picture.motion_val[dir2][s->block_index[i+2]][1];
}
} else {
v->blk_mv_type[s->block_index[0]] = 1;
v->blk_mv_type[s->block_index[1]] = 1;
v->blk_mv_type[s->block_index[2]] = 1;
v->blk_mv_type[s->block_index[3]] = 1;
ff_vc1_pred_mv_intfr(v, 0, 0, 0, 2, v->range_x, v->range_y, 0, !dir);
for (i = 0; i < 2; i++) {
s->mv[!dir][i+2][0] = s->mv[!dir][i][0] = s->current_picture.motion_val[!dir][s->block_index[i+2]][0] = s->current_picture.motion_val[!dir][s->block_index[i]][0];
s->mv[!dir][i+2][1] = s->mv[!dir][i][1] = s->current_picture.motion_val[!dir][s->block_index[i+2]][1] = s->current_picture.motion_val[!dir][s->block_index[i]][1];
}
}
}
}
ff_vc1_mc_1mv(v, dir);
if (direct || bmvtype == BMV_TYPE_INTERPOLATED) {
ff_vc1_interp_mc(v);
}
v->fieldtx_plane[mb_pos] = 0;
}
}
v->cbp[s->mb_x] = block_cbp;
v->ttblk[s->mb_x] = block_tt;
return 0;
}
static void vc1_decode_i_blocks(VC1Context *v)
{
int k, j;
MpegEncContext *s = &v->s;
int cbp, val;
uint8_t *coded_val;
int mb_pos;
switch (v->y_ac_table_index) {
case 0:
v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
break;
case 1:
v->codingset = CS_HIGH_MOT_INTRA;
break;
case 2:
v->codingset = CS_MID_RATE_INTRA;
break;
}
switch (v->c_ac_table_index) {
case 0:
v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
break;
case 1:
v->codingset2 = CS_HIGH_MOT_INTER;
break;
case 2:
v->codingset2 = CS_MID_RATE_INTER;
break;
}
s->y_dc_scale = s->y_dc_scale_table[v->pq];
s->c_dc_scale = s->c_dc_scale_table[v->pq];
s->mb_x = s->mb_y = 0;
s->mb_intra = 1;
s->first_slice_line = 1;
for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
s->mb_x = 0;
init_block_index(v);
for (; s->mb_x < v->end_mb_x; s->mb_x++) {
ff_update_block_index(s);
s->bdsp.clear_blocks(v->block[v->cur_blk_idx][0]);
mb_pos = s->mb_x + s->mb_y * s->mb_width;
s->current_picture.mb_type[mb_pos] = MB_TYPE_INTRA;
s->current_picture.qscale_table[mb_pos] = v->pq;
for (int i = 0; i < 4; i++) {
s->current_picture.motion_val[1][s->block_index[i]][0] = 0;
s->current_picture.motion_val[1][s->block_index[i]][1] = 0;
}
cbp = get_vlc2(&v->s.gb, ff_msmp4_mb_i_vlc.table, MB_INTRA_VLC_BITS, 2);
v->s.ac_pred = get_bits1(&v->s.gb);
for (k = 0; k < 6; k++) {
v->mb_type[0][s->block_index[k]] = 1;
val = ((cbp >> (5 - k)) & 1);
if (k < 4) {
int pred = vc1_coded_block_pred(&v->s, k, &coded_val);
val = val ^ pred;
*coded_val = val;
}
cbp |= val << (5 - k);
vc1_decode_i_block(v, v->block[v->cur_blk_idx][block_map[k]], k, val, (k < 4) ? v->codingset : v->codingset2);
if (CONFIG_GRAY && k > 3 && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[k]]);
}
if (v->overlap && v->pq >= 9) {
ff_vc1_i_overlap_filter(v);
if (v->rangeredfrm)
for (k = 0; k < 6; k++)
for (j = 0; j < 64; j++)
v->block[v->cur_blk_idx][block_map[k]][j] *= 2;
vc1_put_blocks_clamped(v, 1);
} else {
if (v->rangeredfrm)
for (k = 0; k < 6; k++)
for (j = 0; j < 64; j++)
v->block[v->cur_blk_idx][block_map[k]][j] = (v->block[v->cur_blk_idx][block_map[k]][j] - 64) * 2;
vc1_put_blocks_clamped(v, 0);
}
if (v->s.loop_filter)
ff_vc1_i_loop_filter(v);
if (get_bits_left(&s->gb) < 0) {
ff_er_add_slice(&s->er, 0, 0, s->mb_x, s->mb_y, ER_MB_ERROR);
av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i\n",
get_bits_count(&s->gb), s->gb.size_in_bits);
return;
}
v->topleft_blk_idx = (v->topleft_blk_idx + 1) % (v->end_mb_x + 2);
v->top_blk_idx = (v->top_blk_idx + 1) % (v->end_mb_x + 2);
v->left_blk_idx = (v->left_blk_idx + 1) % (v->end_mb_x + 2);
v->cur_blk_idx = (v->cur_blk_idx + 1) % (v->end_mb_x + 2);
}
if (!v->s.loop_filter)
ff_mpeg_draw_horiz_band(s, s->mb_y * 16, 16);
else if (s->mb_y)
ff_mpeg_draw_horiz_band(s, (s->mb_y - 1) * 16, 16);
s->first_slice_line = 0;
}
if (v->s.loop_filter)
ff_mpeg_draw_horiz_band(s, (s->end_mb_y - 1) * 16, 16);
ff_er_add_slice(&s->er, 0, 0, s->mb_width - 1, s->mb_height - 1, ER_MB_END);
}
static int vc1_decode_i_blocks_adv(VC1Context *v)
{
int k;
MpegEncContext *s = &v->s;
int cbp, val;
uint8_t *coded_val;
int mb_pos;
int mquant;
int mqdiff;
GetBitContext *gb = &s->gb;
if (get_bits_left(gb) <= 1)
return AVERROR_INVALIDDATA;
switch (v->y_ac_table_index) {
case 0:
v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
break;
case 1:
v->codingset = CS_HIGH_MOT_INTRA;
break;
case 2:
v->codingset = CS_MID_RATE_INTRA;
break;
}
switch (v->c_ac_table_index) {
case 0:
v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
break;
case 1:
v->codingset2 = CS_HIGH_MOT_INTER;
break;
case 2:
v->codingset2 = CS_MID_RATE_INTER;
break;
}
s->mb_x = s->mb_y = 0;
s->mb_intra = 1;
s->first_slice_line = 1;
s->mb_y = s->start_mb_y;
if (s->start_mb_y) {
s->mb_x = 0;
init_block_index(v);
memset(&s->coded_block[s->block_index[0] - s->b8_stride], 0,
(1 + s->b8_stride) * sizeof(*s->coded_block));
}
for (; s->mb_y < s->end_mb_y; s->mb_y++) {
s->mb_x = 0;
init_block_index(v);
for (;s->mb_x < s->mb_width; s->mb_x++) {
mquant = v->pq;
ff_update_block_index(s);
s->bdsp.clear_blocks(v->block[v->cur_blk_idx][0]);
mb_pos = s->mb_x + s->mb_y * s->mb_stride;
s->current_picture.mb_type[mb_pos + v->mb_off] = MB_TYPE_INTRA;
for (int i = 0; i < 4; i++) {
s->current_picture.motion_val[1][s->block_index[i] + v->blocks_off][0] = 0;
s->current_picture.motion_val[1][s->block_index[i] + v->blocks_off][1] = 0;
}
if (v->fieldtx_is_raw)
v->fieldtx_plane[mb_pos] = get_bits1(&v->s.gb);
if (get_bits_left(&v->s.gb) <= 1) {
ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
return 0;
}
cbp = get_vlc2(&v->s.gb, ff_msmp4_mb_i_vlc.table, MB_INTRA_VLC_BITS, 2);
if (v->acpred_is_raw)
v->s.ac_pred = get_bits1(&v->s.gb);
else
v->s.ac_pred = v->acpred_plane[mb_pos];
if (v->condover == CONDOVER_SELECT && v->overflg_is_raw)
v->over_flags_plane[mb_pos] = get_bits1(&v->s.gb);
GET_MQUANT();
s->current_picture.qscale_table[mb_pos] = mquant;
s->y_dc_scale = s->y_dc_scale_table[FFABS(mquant)];
s->c_dc_scale = s->c_dc_scale_table[FFABS(mquant)];
for (k = 0; k < 6; k++) {
v->mb_type[0][s->block_index[k]] = 1;
val = ((cbp >> (5 - k)) & 1);
if (k < 4) {
int pred = vc1_coded_block_pred(&v->s, k, &coded_val);
val = val ^ pred;
*coded_val = val;
}
cbp |= val << (5 - k);
v->a_avail = !s->first_slice_line || (k == 2 || k == 3);
v->c_avail = !!s->mb_x || (k == 1 || k == 3);
vc1_decode_i_block_adv(v, v->block[v->cur_blk_idx][block_map[k]], k, val,
(k < 4) ? v->codingset : v->codingset2, mquant);
if (CONFIG_GRAY && k > 3 && (s->avctx->flags & AV_CODEC_FLAG_GRAY))
continue;
v->vc1dsp.vc1_inv_trans_8x8(v->block[v->cur_blk_idx][block_map[k]]);
}
if (v->overlap && (v->pq >= 9 || v->condover != CONDOVER_NONE))
ff_vc1_i_overlap_filter(v);
vc1_put_blocks_clamped(v, 1);
if (v->s.loop_filter)
ff_vc1_i_loop_filter(v);
if (get_bits_left(&s->gb) < 0) {
ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i\n",
get_bits_count(&s->gb), s->gb.size_in_bits);
return 0;
}
inc_blk_idx(v->topleft_blk_idx);
inc_blk_idx(v->top_blk_idx);
inc_blk_idx(v->left_blk_idx);
inc_blk_idx(v->cur_blk_idx);
}
if (!v->s.loop_filter)
ff_mpeg_draw_horiz_band(s, s->mb_y * 16, 16);
else if (s->mb_y)
ff_mpeg_draw_horiz_band(s, (s->mb_y-1) * 16, 16);
s->first_slice_line = 0;
}
if (v->s.loop_filter)
ff_mpeg_draw_horiz_band(s, (s->end_mb_y - 1) * 16, 16);
ff_er_add_slice(&s->er, 0, s->start_mb_y << v->field_mode, s->mb_width - 1,
(s->end_mb_y << v->field_mode) - 1, ER_MB_END);
return 0;
}
static void vc1_decode_p_blocks(VC1Context *v)
{
MpegEncContext *s = &v->s;
int apply_loop_filter;
switch (v->c_ac_table_index) {
case 0:
v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
break;
case 1:
v->codingset = CS_HIGH_MOT_INTRA;
break;
case 2:
v->codingset = CS_MID_RATE_INTRA;
break;
}
switch (v->c_ac_table_index) {
case 0:
v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
break;
case 1:
v->codingset2 = CS_HIGH_MOT_INTER;
break;
case 2:
v->codingset2 = CS_MID_RATE_INTER;
break;
}
apply_loop_filter = s->loop_filter && !(s->avctx->skip_loop_filter >= AVDISCARD_NONKEY);
s->first_slice_line = 1;
memset(v->cbp_base, 0, sizeof(v->cbp_base[0]) * 3 * s->mb_stride);
for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
s->mb_x = 0;
init_block_index(v);
for (; s->mb_x < s->mb_width; s->mb_x++) {
ff_update_block_index(s);
if (v->fcm == ILACE_FIELD || (v->fcm == PROGRESSIVE && v->mv_type_is_raw) || v->skip_is_raw)
if (get_bits_left(&v->s.gb) <= 1) {
ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
return;
}
if (v->fcm == ILACE_FIELD) {
vc1_decode_p_mb_intfi(v);
if (apply_loop_filter)
ff_vc1_p_loop_filter(v);
} else if (v->fcm == ILACE_FRAME) {
vc1_decode_p_mb_intfr(v);
if (apply_loop_filter)
ff_vc1_p_intfr_loop_filter(v);
} else {
vc1_decode_p_mb(v);
if (apply_loop_filter)
ff_vc1_p_loop_filter(v);
}
if (get_bits_left(&s->gb) < 0 || get_bits_count(&s->gb) < 0) {
ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i at %ix%i\n",
get_bits_count(&s->gb), s->gb.size_in_bits, s->mb_x, s->mb_y);
return;
}
inc_blk_idx(v->topleft_blk_idx);
inc_blk_idx(v->top_blk_idx);
inc_blk_idx(v->left_blk_idx);
inc_blk_idx(v->cur_blk_idx);
}
memmove(v->cbp_base,
v->cbp - s->mb_stride,
sizeof(v->cbp_base[0]) * 2 * s->mb_stride);
memmove(v->ttblk_base,
v->ttblk - s->mb_stride,
sizeof(v->ttblk_base[0]) * 2 * s->mb_stride);
memmove(v->is_intra_base,
v->is_intra - s->mb_stride,
sizeof(v->is_intra_base[0]) * 2 * s->mb_stride);
memmove(v->luma_mv_base,
v->luma_mv - s->mb_stride,
sizeof(v->luma_mv_base[0]) * 2 * s->mb_stride);
if (s->mb_y != s->start_mb_y)
ff_mpeg_draw_horiz_band(s, (s->mb_y - 1) * 16, 16);
s->first_slice_line = 0;
}
if (s->end_mb_y >= s->start_mb_y)
ff_mpeg_draw_horiz_band(s, (s->end_mb_y - 1) * 16, 16);
ff_er_add_slice(&s->er, 0, s->start_mb_y << v->field_mode, s->mb_width - 1,
(s->end_mb_y << v->field_mode) - 1, ER_MB_END);
}
static void vc1_decode_b_blocks(VC1Context *v)
{
MpegEncContext *s = &v->s;
switch (v->c_ac_table_index) {
case 0:
v->codingset = (v->pqindex <= 8) ? CS_HIGH_RATE_INTRA : CS_LOW_MOT_INTRA;
break;
case 1:
v->codingset = CS_HIGH_MOT_INTRA;
break;
case 2:
v->codingset = CS_MID_RATE_INTRA;
break;
}
switch (v->c_ac_table_index) {
case 0:
v->codingset2 = (v->pqindex <= 8) ? CS_HIGH_RATE_INTER : CS_LOW_MOT_INTER;
break;
case 1:
v->codingset2 = CS_HIGH_MOT_INTER;
break;
case 2:
v->codingset2 = CS_MID_RATE_INTER;
break;
}
s->first_slice_line = 1;
for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
s->mb_x = 0;
init_block_index(v);
for (; s->mb_x < s->mb_width; s->mb_x++) {
ff_update_block_index(s);
if (v->fcm == ILACE_FIELD || v->skip_is_raw || v->dmb_is_raw)
if (get_bits_left(&v->s.gb) <= 1) {
ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
return;
}
if (v->fcm == ILACE_FIELD) {
vc1_decode_b_mb_intfi(v);
if (v->s.loop_filter)
ff_vc1_b_intfi_loop_filter(v);
} else if (v->fcm == ILACE_FRAME) {
vc1_decode_b_mb_intfr(v);
if (v->s.loop_filter)
ff_vc1_p_intfr_loop_filter(v);
} else {
vc1_decode_b_mb(v);
if (v->s.loop_filter)
ff_vc1_i_loop_filter(v);
}
if (get_bits_left(&s->gb) < 0 || get_bits_count(&s->gb) < 0) {
ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_x, s->mb_y, ER_MB_ERROR);
av_log(s->avctx, AV_LOG_ERROR, "Bits overconsumption: %i > %i at %ix%i\n",
get_bits_count(&s->gb), s->gb.size_in_bits, s->mb_x, s->mb_y);
return;
}
}
memmove(v->cbp_base,
v->cbp - s->mb_stride,
sizeof(v->cbp_base[0]) * 2 * s->mb_stride);
memmove(v->ttblk_base,
v->ttblk - s->mb_stride,
sizeof(v->ttblk_base[0]) * 2 * s->mb_stride);
memmove(v->is_intra_base,
v->is_intra - s->mb_stride,
sizeof(v->is_intra_base[0]) * 2 * s->mb_stride);
if (!v->s.loop_filter)
ff_mpeg_draw_horiz_band(s, s->mb_y * 16, 16);
else if (s->mb_y)
ff_mpeg_draw_horiz_band(s, (s->mb_y - 1) * 16, 16);
s->first_slice_line = 0;
}
if (v->s.loop_filter)
ff_mpeg_draw_horiz_band(s, (s->end_mb_y - 1) * 16, 16);
ff_er_add_slice(&s->er, 0, s->start_mb_y << v->field_mode, s->mb_width - 1,
(s->end_mb_y << v->field_mode) - 1, ER_MB_END);
}
static void vc1_decode_skip_blocks(VC1Context *v)
{
MpegEncContext *s = &v->s;
if (!v->s.last_picture.f->data[0])
return;
ff_er_add_slice(&s->er, 0, s->start_mb_y, s->mb_width - 1, s->end_mb_y - 1, ER_MB_END);
s->first_slice_line = 1;
for (s->mb_y = s->start_mb_y; s->mb_y < s->end_mb_y; s->mb_y++) {
s->mb_x = 0;
init_block_index(v);
ff_update_block_index(s);
memcpy(s->dest[0], s->last_picture.f->data[0] + s->mb_y * 16 * s->linesize, s->linesize * 16);
memcpy(s->dest[1], s->last_picture.f->data[1] + s->mb_y * 8 * s->uvlinesize, s->uvlinesize * 8);
memcpy(s->dest[2], s->last_picture.f->data[2] + s->mb_y * 8 * s->uvlinesize, s->uvlinesize * 8);
ff_mpeg_draw_horiz_band(s, s->mb_y * 16, 16);
s->first_slice_line = 0;
}
s->pict_type = AV_PICTURE_TYPE_P;
}
void ff_vc1_decode_blocks(VC1Context *v)
{
v->s.esc3_level_length = 0;
if (v->x8_type) {
ff_intrax8_decode_picture(&v->x8, &v->s.current_picture,
&v->s.gb, &v->s.mb_x, &v->s.mb_y,
2 * v->pq + v->halfpq, v->pq * !v->pquantizer,
v->s.loop_filter, v->s.low_delay);
ff_er_add_slice(&v->s.er, 0, 0,
(v->s.mb_x >> 1) - 1, (v->s.mb_y >> 1) - 1,
ER_MB_END);
} else {
v->cur_blk_idx = 0;
v->left_blk_idx = -1;
v->topleft_blk_idx = 1;
v->top_blk_idx = 2;
switch (v->s.pict_type) {
case AV_PICTURE_TYPE_I:
if (v->profile == PROFILE_ADVANCED)
vc1_decode_i_blocks_adv(v);
else
vc1_decode_i_blocks(v);
break;
case AV_PICTURE_TYPE_P:
if (v->p_frame_skipped)
vc1_decode_skip_blocks(v);
else
vc1_decode_p_blocks(v);
break;
case AV_PICTURE_TYPE_B:
if (v->bi_type) {
if (v->profile == PROFILE_ADVANCED)
vc1_decode_i_blocks_adv(v);
else
vc1_decode_i_blocks(v);
} else
vc1_decode_b_blocks(v);
break;
}
}
}
