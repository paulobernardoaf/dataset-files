




















#include <limits.h>
#include <X11/extensions/XvMC.h>

#include "avcodec.h"
#include "mpegutils.h"
#include "mpegvideo.h"

#undef NDEBUG
#include <assert.h>

#include "xvmc.h"
#include "xvmc_internal.h"
#include "version.h"








void ff_xvmc_init_block(MpegEncContext *s)
{
struct xvmc_pix_fmt *render = (struct xvmc_pix_fmt*)s->current_picture.f->data[2];
assert(render && render->xvmc_id == AV_XVMC_ID);

s->block = (int16_t (*)[64])(render->data_blocks + render->next_free_data_block_num * 64);
}

static void exchange_uv(MpegEncContext *s)
{
int16_t (*tmp)[64];

tmp = s->pblocks[4];
s->pblocks[4] = s->pblocks[5];
s->pblocks[5] = tmp;
}





void ff_xvmc_pack_pblocks(MpegEncContext *s, int cbp)
{
int i, j = 0;
const int mb_block_count = 4 + (1 << s->chroma_format);

cbp <<= 12-mb_block_count;
for (i = 0; i < mb_block_count; i++) {
if (cbp & (1 << 11))
s->pblocks[i] = &s->block[j++];
else
s->pblocks[i] = NULL;
cbp += cbp;
}
if (s->swap_uv) {
exchange_uv(s);
}
}






static int ff_xvmc_field_start(AVCodecContext *avctx, const uint8_t *buf, uint32_t buf_size)
{
struct MpegEncContext *s = avctx->priv_data;
struct xvmc_pix_fmt *last, *next, *render = (struct xvmc_pix_fmt*)s->current_picture.f->data[2];
const int mb_block_count = 4 + (1 << s->chroma_format);

assert(avctx);
if (!render || render->xvmc_id != AV_XVMC_ID ||
!render->data_blocks || !render->mv_blocks ||
(unsigned int)render->allocated_mv_blocks > INT_MAX/(64*6) ||
(unsigned int)render->allocated_data_blocks > INT_MAX/64 ||
!render->p_surface) {
av_log(avctx, AV_LOG_ERROR,
"Render token doesn't look as expected.\n");
return -1; 
}

if (render->filled_mv_blocks_num) {
av_log(avctx, AV_LOG_ERROR,
"Rendering surface contains %i unprocessed blocks.\n",
render->filled_mv_blocks_num);
return -1;
}
if (render->allocated_mv_blocks < 1 ||
render->allocated_data_blocks < render->allocated_mv_blocks*mb_block_count ||
render->start_mv_blocks_num >= render->allocated_mv_blocks ||
render->next_free_data_block_num >
render->allocated_data_blocks -
mb_block_count*(render->allocated_mv_blocks-render->start_mv_blocks_num)) {
av_log(avctx, AV_LOG_ERROR,
"Rendering surface doesn't provide enough block structures to work with.\n");
return -1;
}

render->picture_structure = s->picture_structure;
render->flags = s->first_field ? 0 : XVMC_SECOND_FIELD;
render->p_future_surface = NULL;
render->p_past_surface = NULL;

switch(s->pict_type) {
case AV_PICTURE_TYPE_I:
return 0; 
case AV_PICTURE_TYPE_B:
next = (struct xvmc_pix_fmt*)s->next_picture.f->data[2];
if (!next)
return -1;
if (next->xvmc_id != AV_XVMC_ID)
return -1;
render->p_future_surface = next->p_surface;

case AV_PICTURE_TYPE_P:
last = (struct xvmc_pix_fmt*)s->last_picture.f->data[2];
if (!last)
last = render; 
if (last->xvmc_id != AV_XVMC_ID)
return -1;
render->p_past_surface = last->p_surface;
return 0;
}

return -1;
}







static int ff_xvmc_field_end(AVCodecContext *avctx)
{
struct MpegEncContext *s = avctx->priv_data;
struct xvmc_pix_fmt *render = (struct xvmc_pix_fmt*)s->current_picture.f->data[2];
assert(render);

if (render->filled_mv_blocks_num > 0)
ff_mpeg_draw_horiz_band(s, 0, 0);
return 0;
}





static void ff_xvmc_decode_mb(struct MpegEncContext *s)
{
XvMCMacroBlock *mv_block;
struct xvmc_pix_fmt *render;
int i, cbp, blocks_per_mb;

const int mb_xy = s->mb_y * s->mb_stride + s->mb_x;


if (s->encoding) {
av_log(s->avctx, AV_LOG_ERROR, "XVMC doesn't support encoding!!!\n");
return;
}


if (!s->mb_intra) {
s->last_dc[0] =
s->last_dc[1] =
s->last_dc[2] = 128 << s->intra_dc_precision;
}


s->mb_skipped = 0;




s->current_picture.qscale_table[mb_xy] = s->qscale;


render = (struct xvmc_pix_fmt*)s->current_picture.f->data[2];
assert(render);
assert(render->xvmc_id == AV_XVMC_ID);
assert(render->mv_blocks);


mv_block = &render->mv_blocks[render->start_mv_blocks_num +
render->filled_mv_blocks_num];

mv_block->x = s->mb_x;
mv_block->y = s->mb_y;
mv_block->dct_type = s->interlaced_dct; 
if (s->mb_intra) {
mv_block->macroblock_type = XVMC_MB_TYPE_INTRA; 
} else {
mv_block->macroblock_type = XVMC_MB_TYPE_PATTERN;

if (s->mv_dir & MV_DIR_FORWARD) {
mv_block->macroblock_type |= XVMC_MB_TYPE_MOTION_FORWARD;

mv_block->PMV[0][0][0] = s->mv[0][0][0];
mv_block->PMV[0][0][1] = s->mv[0][0][1];
mv_block->PMV[1][0][0] = s->mv[0][1][0];
mv_block->PMV[1][0][1] = s->mv[0][1][1];
}
if (s->mv_dir & MV_DIR_BACKWARD) {
mv_block->macroblock_type |= XVMC_MB_TYPE_MOTION_BACKWARD;
mv_block->PMV[0][1][0] = s->mv[1][0][0];
mv_block->PMV[0][1][1] = s->mv[1][0][1];
mv_block->PMV[1][1][0] = s->mv[1][1][0];
mv_block->PMV[1][1][1] = s->mv[1][1][1];
}

switch(s->mv_type) {
case MV_TYPE_16X16:
mv_block->motion_type = XVMC_PREDICTION_FRAME;
break;
case MV_TYPE_16X8:
mv_block->motion_type = XVMC_PREDICTION_16x8;
break;
case MV_TYPE_FIELD:
mv_block->motion_type = XVMC_PREDICTION_FIELD;
if (s->picture_structure == PICT_FRAME) {
mv_block->PMV[0][0][1] <<= 1;
mv_block->PMV[1][0][1] <<= 1;
mv_block->PMV[0][1][1] <<= 1;
mv_block->PMV[1][1][1] <<= 1;
}
break;
case MV_TYPE_DMV:
mv_block->motion_type = XVMC_PREDICTION_DUAL_PRIME;
if (s->picture_structure == PICT_FRAME) {

mv_block->PMV[0][0][0] = s->mv[0][0][0]; 
mv_block->PMV[0][0][1] = s->mv[0][0][1] << 1;

mv_block->PMV[0][1][0] = s->mv[0][0][0]; 
mv_block->PMV[0][1][1] = s->mv[0][0][1] << 1;

mv_block->PMV[1][0][0] = s->mv[0][2][0]; 
mv_block->PMV[1][0][1] = s->mv[0][2][1] << 1; 

mv_block->PMV[1][1][0] = s->mv[0][3][0]; 
mv_block->PMV[1][1][1] = s->mv[0][3][1] << 1; 

} else {
mv_block->PMV[0][1][0] = s->mv[0][2][0]; 
mv_block->PMV[0][1][1] = s->mv[0][2][1]; 
}
break;
default:
assert(0);
}

mv_block->motion_vertical_field_select = 0;


if (s->mv_type == MV_TYPE_FIELD || s->mv_type == MV_TYPE_16X8) {
mv_block->motion_vertical_field_select |= s->field_select[0][0];
mv_block->motion_vertical_field_select |= s->field_select[1][0] << 1;
mv_block->motion_vertical_field_select |= s->field_select[0][1] << 2;
mv_block->motion_vertical_field_select |= s->field_select[1][1] << 3;
}
} 

mv_block->index = render->next_free_data_block_num;

blocks_per_mb = 6;
if (s->chroma_format >= 2) {
blocks_per_mb = 4 + (1 << s->chroma_format);
}


cbp = 0;
for (i = 0; i < blocks_per_mb; i++) {
cbp += cbp;
if (s->block_last_index[i] >= 0)
cbp++;
}

if (s->avctx->flags & AV_CODEC_FLAG_GRAY) {
if (s->mb_intra) { 
for (i = 4; i < blocks_per_mb; i++) {
memset(s->pblocks[i], 0, sizeof(*s->pblocks[i])); 
if (!render->unsigned_intra)
*s->pblocks[i][0] = 1 << 10;
}
} else {
cbp &= 0xf << (blocks_per_mb - 4);
blocks_per_mb = 4; 
}
}
mv_block->coded_block_pattern = cbp;
if (cbp == 0)
mv_block->macroblock_type &= ~XVMC_MB_TYPE_PATTERN;

for (i = 0; i < blocks_per_mb; i++) {
if (s->block_last_index[i] >= 0) {

if (s->mb_intra && (render->idct || !render->unsigned_intra))
*s->pblocks[i][0] -= 1 << 10;
if (!render->idct) {
s->idsp.idct(*s->pblocks[i]);




}

if (!s->pack_pblocks) {
memcpy(&render->data_blocks[render->next_free_data_block_num*64],
s->pblocks[i], sizeof(*s->pblocks[i]));
}
render->next_free_data_block_num++;
}
}
render->filled_mv_blocks_num++;

assert(render->filled_mv_blocks_num <= render->allocated_mv_blocks);
assert(render->next_free_data_block_num <= render->allocated_data_blocks);





if (render->filled_mv_blocks_num == render->allocated_mv_blocks)
ff_mpeg_draw_horiz_band(s, 0, 0);
}

#if CONFIG_MPEG1_XVMC_HWACCEL
const AVHWAccel ff_mpeg1_xvmc_hwaccel = {
.name = "mpeg1_xvmc",
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_MPEG1VIDEO,
.pix_fmt = AV_PIX_FMT_XVMC,
.start_frame = ff_xvmc_field_start,
.end_frame = ff_xvmc_field_end,
.decode_slice = NULL,
.decode_mb = ff_xvmc_decode_mb,
.priv_data_size = 0,
};
#endif

#if CONFIG_MPEG2_XVMC_HWACCEL
const AVHWAccel ff_mpeg2_xvmc_hwaccel = {
.name = "mpeg2_xvmc",
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_MPEG2VIDEO,
.pix_fmt = AV_PIX_FMT_XVMC,
.start_frame = ff_xvmc_field_start,
.end_frame = ff_xvmc_field_end,
.decode_slice = NULL,
.decode_mb = ff_xvmc_decode_mb,
.priv_data_size = 0,
};
#endif
