#include "mpegutils.h"
#include "mpegvideo.h"
#include "mpeg4video.h"
#include "mpeg4data.h"
uint8_t ff_mpeg4_static_rl_table_store[3][2][2 * MAX_RUN + MAX_LEVEL + 3];
int ff_mpeg4_get_video_packet_prefix_length(MpegEncContext *s)
{
switch (s->pict_type) {
case AV_PICTURE_TYPE_I:
return 16;
case AV_PICTURE_TYPE_P:
case AV_PICTURE_TYPE_S:
return s->f_code + 15;
case AV_PICTURE_TYPE_B:
return FFMAX3(s->f_code, s->b_code, 2) + 15;
default:
return -1;
}
}
void ff_mpeg4_clean_buffers(MpegEncContext *s)
{
int c_wrap, c_xy, l_wrap, l_xy;
l_wrap = s->b8_stride;
l_xy = (2 * s->mb_y - 1) * l_wrap + s->mb_x * 2 - 1;
c_wrap = s->mb_stride;
c_xy = (s->mb_y - 1) * c_wrap + s->mb_x - 1;
memset(s->ac_val[0] + l_xy, 0, (l_wrap * 2 + 1) * 16 * sizeof(int16_t));
memset(s->ac_val[1] + c_xy, 0, (c_wrap + 1) * 16 * sizeof(int16_t));
memset(s->ac_val[2] + c_xy, 0, (c_wrap + 1) * 16 * sizeof(int16_t));
s->last_mv[0][0][0] =
s->last_mv[0][0][1] =
s->last_mv[1][0][0] =
s->last_mv[1][0][1] = 0;
}
#define tab_size ((signed)FF_ARRAY_ELEMS(s->direct_scale_mv[0]))
#define tab_bias (tab_size / 2)
void ff_mpeg4_init_direct_mv(MpegEncContext *s)
{
int i;
for (i = 0; i < tab_size; i++) {
s->direct_scale_mv[0][i] = (i - tab_bias) * s->pb_time / s->pp_time;
s->direct_scale_mv[1][i] = (i - tab_bias) * (s->pb_time - s->pp_time) /
s->pp_time;
}
}
static inline void ff_mpeg4_set_one_direct_mv(MpegEncContext *s, int mx,
int my, int i)
{
int xy = s->block_index[i];
uint16_t time_pp = s->pp_time;
uint16_t time_pb = s->pb_time;
int p_mx, p_my;
p_mx = s->next_picture.motion_val[0][xy][0];
if ((unsigned)(p_mx + tab_bias) < tab_size) {
s->mv[0][i][0] = s->direct_scale_mv[0][p_mx + tab_bias] + mx;
s->mv[1][i][0] = mx ? s->mv[0][i][0] - p_mx
: s->direct_scale_mv[1][p_mx + tab_bias];
} else {
s->mv[0][i][0] = p_mx * time_pb / time_pp + mx;
s->mv[1][i][0] = mx ? s->mv[0][i][0] - p_mx
: p_mx * (time_pb - time_pp) / time_pp;
}
p_my = s->next_picture.motion_val[0][xy][1];
if ((unsigned)(p_my + tab_bias) < tab_size) {
s->mv[0][i][1] = s->direct_scale_mv[0][p_my + tab_bias] + my;
s->mv[1][i][1] = my ? s->mv[0][i][1] - p_my
: s->direct_scale_mv[1][p_my + tab_bias];
} else {
s->mv[0][i][1] = p_my * time_pb / time_pp + my;
s->mv[1][i][1] = my ? s->mv[0][i][1] - p_my
: p_my * (time_pb - time_pp) / time_pp;
}
}
#undef tab_size
#undef tab_bias
int ff_mpeg4_set_direct_mv(MpegEncContext *s, int mx, int my)
{
const int mb_index = s->mb_x + s->mb_y * s->mb_stride;
const int colocated_mb_type = s->next_picture.mb_type[mb_index];
uint16_t time_pp;
uint16_t time_pb;
int i;
if (IS_8X8(colocated_mb_type)) {
s->mv_type = MV_TYPE_8X8;
for (i = 0; i < 4; i++)
ff_mpeg4_set_one_direct_mv(s, mx, my, i);
return MB_TYPE_DIRECT2 | MB_TYPE_8x8 | MB_TYPE_L0L1;
} else if (IS_INTERLACED(colocated_mb_type)) {
s->mv_type = MV_TYPE_FIELD;
for (i = 0; i < 2; i++) {
int field_select = s->next_picture.ref_index[0][4 * mb_index + 2 * i];
s->field_select[0][i] = field_select;
s->field_select[1][i] = i;
if (s->top_field_first) {
time_pp = s->pp_field_time - field_select + i;
time_pb = s->pb_field_time - field_select + i;
} else {
time_pp = s->pp_field_time + field_select - i;
time_pb = s->pb_field_time + field_select - i;
}
s->mv[0][i][0] = s->p_field_mv_table[i][0][mb_index][0] *
time_pb / time_pp + mx;
s->mv[0][i][1] = s->p_field_mv_table[i][0][mb_index][1] *
time_pb / time_pp + my;
s->mv[1][i][0] = mx ? s->mv[0][i][0] -
s->p_field_mv_table[i][0][mb_index][0]
: s->p_field_mv_table[i][0][mb_index][0] *
(time_pb - time_pp) / time_pp;
s->mv[1][i][1] = my ? s->mv[0][i][1] -
s->p_field_mv_table[i][0][mb_index][1]
: s->p_field_mv_table[i][0][mb_index][1] *
(time_pb - time_pp) / time_pp;
}
return MB_TYPE_DIRECT2 | MB_TYPE_16x8 |
MB_TYPE_L0L1 | MB_TYPE_INTERLACED;
} else {
ff_mpeg4_set_one_direct_mv(s, mx, my, 0);
s->mv[0][1][0] =
s->mv[0][2][0] =
s->mv[0][3][0] = s->mv[0][0][0];
s->mv[0][1][1] =
s->mv[0][2][1] =
s->mv[0][3][1] = s->mv[0][0][1];
s->mv[1][1][0] =
s->mv[1][2][0] =
s->mv[1][3][0] = s->mv[1][0][0];
s->mv[1][1][1] =
s->mv[1][2][1] =
s->mv[1][3][1] = s->mv[1][0][1];
if ((s->avctx->workaround_bugs & FF_BUG_DIRECT_BLOCKSIZE) ||
!s->quarter_sample)
s->mv_type = MV_TYPE_16X16;
else
s->mv_type = MV_TYPE_8X8;
return MB_TYPE_DIRECT2 | MB_TYPE_16x16 | MB_TYPE_L0L1;
}
}
