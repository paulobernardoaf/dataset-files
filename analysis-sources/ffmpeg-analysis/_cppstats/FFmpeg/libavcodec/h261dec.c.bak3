


























#include "libavutil/avassert.h"
#include "avcodec.h"
#include "mpeg_er.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "h263.h"
#include "h261.h"
#include "internal.h"

#define H261_MBA_VLC_BITS 9
#define H261_MTYPE_VLC_BITS 6
#define H261_MV_VLC_BITS 7
#define H261_CBP_VLC_BITS 9
#define TCOEFF_VLC_BITS 9
#define MBA_STUFFING 33
#define MBA_STARTCODE 34

static VLC h261_mba_vlc;
static VLC h261_mtype_vlc;
static VLC h261_mv_vlc;
static VLC h261_cbp_vlc;

static av_cold void h261_decode_init_vlc(H261Context *h)
{
static int done = 0;

if (!done) {
done = 1;
INIT_VLC_STATIC(&h261_mba_vlc, H261_MBA_VLC_BITS, 35,
ff_h261_mba_bits, 1, 1,
ff_h261_mba_code, 1, 1, 662);
INIT_VLC_STATIC(&h261_mtype_vlc, H261_MTYPE_VLC_BITS, 10,
ff_h261_mtype_bits, 1, 1,
ff_h261_mtype_code, 1, 1, 80);
INIT_VLC_STATIC(&h261_mv_vlc, H261_MV_VLC_BITS, 17,
&ff_h261_mv_tab[0][1], 2, 1,
&ff_h261_mv_tab[0][0], 2, 1, 144);
INIT_VLC_STATIC(&h261_cbp_vlc, H261_CBP_VLC_BITS, 63,
&ff_h261_cbp_tab[0][1], 2, 1,
&ff_h261_cbp_tab[0][0], 2, 1, 512);
INIT_VLC_RL(ff_h261_rl_tcoeff, 552);
}
}

static av_cold int h261_decode_init(AVCodecContext *avctx)
{
H261Context *h = avctx->priv_data;
MpegEncContext *const s = &h->s;


ff_mpv_decode_defaults(s);
ff_mpv_decode_init(s, avctx);

s->out_format = FMT_H261;
s->low_delay = 1;
avctx->pix_fmt = AV_PIX_FMT_YUV420P;

ff_h261_common_init();
h261_decode_init_vlc(h);

h->gob_start_code_skipped = 0;

return 0;
}





static int h261_decode_gob_header(H261Context *h)
{
unsigned int val;
MpegEncContext *const s = &h->s;

if (!h->gob_start_code_skipped) {

val = show_bits(&s->gb, 15);
if (val)
return -1;


skip_bits(&s->gb, 16);
}

h->gob_start_code_skipped = 0;

h->gob_number = get_bits(&s->gb, 4); 
s->qscale = get_bits(&s->gb, 5); 


if (s->mb_height == 18) { 
if ((h->gob_number <= 0) || (h->gob_number > 12))
return -1;
} else { 
if ((h->gob_number != 1) && (h->gob_number != 3) &&
(h->gob_number != 5))
return -1;
}


if (skip_1stop_8data_bits(&s->gb) < 0)
return AVERROR_INVALIDDATA;

if (s->qscale == 0) {
av_log(s->avctx, AV_LOG_ERROR, "qscale has forbidden 0 value\n");
if (s->avctx->err_recognition & (AV_EF_BITSTREAM | AV_EF_COMPLIANT))
return -1;
}





h->current_mba = 0;
h->mba_diff = 0;

return 0;
}





static int h261_resync(H261Context *h)
{
MpegEncContext *const s = &h->s;
int left, ret;

if (h->gob_start_code_skipped) {
ret = h261_decode_gob_header(h);
if (ret >= 0)
return 0;
} else {
if (show_bits(&s->gb, 15) == 0) {
ret = h261_decode_gob_header(h);
if (ret >= 0)
return 0;
}

s->gb = s->last_resync_gb;
align_get_bits(&s->gb);
left = get_bits_left(&s->gb);

for (; left > 15 + 1 + 4 + 5; left -= 8) {
if (show_bits(&s->gb, 15) == 0) {
GetBitContext bak = s->gb;

ret = h261_decode_gob_header(h);
if (ret >= 0)
return 0;

s->gb = bak;
}
skip_bits(&s->gb, 8);
}
}

return -1;
}





static int h261_decode_mb_skipped(H261Context *h, int mba1, int mba2)
{
MpegEncContext *const s = &h->s;
int i;

s->mb_intra = 0;

for (i = mba1; i < mba2; i++) {
int j, xy;

s->mb_x = ((h->gob_number - 1) % 2) * 11 + i % 11;
s->mb_y = ((h->gob_number - 1) / 2) * 3 + i / 11;
xy = s->mb_x + s->mb_y * s->mb_stride;
ff_init_block_index(s);
ff_update_block_index(s);

for (j = 0; j < 6; j++)
s->block_last_index[j] = -1;

s->mv_dir = MV_DIR_FORWARD;
s->mv_type = MV_TYPE_16X16;
s->current_picture.mb_type[xy] = MB_TYPE_SKIP | MB_TYPE_16x16 | MB_TYPE_L0;
s->mv[0][0][0] = 0;
s->mv[0][0][1] = 0;
s->mb_skipped = 1;
h->mtype &= ~MB_TYPE_H261_FIL;

if (s->current_picture.motion_val[0]) {
int b_stride = 2*s->mb_width + 1;
int b_xy = 2 * s->mb_x + (2 * s->mb_y) * b_stride;
s->current_picture.motion_val[0][b_xy][0] = s->mv[0][0][0];
s->current_picture.motion_val[0][b_xy][1] = s->mv[0][0][1];
}

ff_mpv_reconstruct_mb(s, s->block);
}

return 0;
}

static const int mvmap[17] = {
0, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16
};

static int decode_mv_component(GetBitContext *gb, int v)
{
int mv_diff = get_vlc2(gb, h261_mv_vlc.table, H261_MV_VLC_BITS, 2);


if (mv_diff < 0)
return v;

mv_diff = mvmap[mv_diff];

if (mv_diff && !get_bits1(gb))
mv_diff = -mv_diff;

v += mv_diff;
if (v <= -16)
v += 32;
else if (v >= 16)
v -= 32;

return v;
}





static int h261_decode_block(H261Context *h, int16_t *block, int n, int coded)
{
MpegEncContext *const s = &h->s;
int level, i, j, run;
RLTable *rl = &ff_h261_rl_tcoeff;
const uint8_t *scan_table;







scan_table = s->intra_scantable.permutated;
if (s->mb_intra) {

level = get_bits(&s->gb, 8);

if ((level & 0x7F) == 0) {
av_log(s->avctx, AV_LOG_ERROR, "illegal dc %d at %d %d\n",
level, s->mb_x, s->mb_y);
return -1;
}


if (level == 255)
level = 128;
block[0] = level;
i = 1;
} else if (coded) {




int check = show_bits(&s->gb, 2);
i = 0;
if (check & 0x2) {
skip_bits(&s->gb, 2);
block[0] = (check & 0x1) ? -1 : 1;
i = 1;
}
} else {
i = 0;
}
if (!coded) {
s->block_last_index[n] = i - 1;
return 0;
}
{
OPEN_READER(re, &s->gb);
i--; 
for (;;) {
UPDATE_CACHE(re, &s->gb);
GET_RL_VLC(level, run, re, &s->gb, rl->rl_vlc[0], TCOEFF_VLC_BITS, 2, 0);
if (run == 66) {
if (level) {
CLOSE_READER(re, &s->gb);
av_log(s->avctx, AV_LOG_ERROR, "illegal ac vlc code at %dx%d\n",
s->mb_x, s->mb_y);
return -1;
}




run = SHOW_UBITS(re, &s->gb, 6) + 1;
SKIP_CACHE(re, &s->gb, 6);
level = SHOW_SBITS(re, &s->gb, 8);
SKIP_COUNTER(re, &s->gb, 6 + 8);
} else if (level == 0) {
break;
} else {
if (SHOW_UBITS(re, &s->gb, 1))
level = -level;
SKIP_COUNTER(re, &s->gb, 1);
}
i += run;
if (i >= 64) {
CLOSE_READER(re, &s->gb);
av_log(s->avctx, AV_LOG_ERROR, "run overflow at %dx%d\n",
s->mb_x, s->mb_y);
return -1;
}
j = scan_table[i];
block[j] = level;
}
CLOSE_READER(re, &s->gb);
}
s->block_last_index[n] = i;
return 0;
}

static int h261_decode_mb(H261Context *h)
{
MpegEncContext *const s = &h->s;
int i, cbp, xy;

cbp = 63;

do {
h->mba_diff = get_vlc2(&s->gb, h261_mba_vlc.table,
H261_MBA_VLC_BITS, 2);



if (h->mba_diff == MBA_STARTCODE) { 
h->gob_start_code_skipped = 1;
return SLICE_END;
}
} while (h->mba_diff == MBA_STUFFING); 

if (h->mba_diff < 0) {
if (get_bits_left(&s->gb) <= 7)
return SLICE_END;

av_log(s->avctx, AV_LOG_ERROR, "illegal mba at %d %d\n", s->mb_x, s->mb_y);
return SLICE_ERROR;
}

h->mba_diff += 1;
h->current_mba += h->mba_diff;

if (h->current_mba > MBA_STUFFING)
return SLICE_ERROR;

s->mb_x = ((h->gob_number - 1) % 2) * 11 + ((h->current_mba - 1) % 11);
s->mb_y = ((h->gob_number - 1) / 2) * 3 + ((h->current_mba - 1) / 11);
xy = s->mb_x + s->mb_y * s->mb_stride;
ff_init_block_index(s);
ff_update_block_index(s);


h->mtype = get_vlc2(&s->gb, h261_mtype_vlc.table, H261_MTYPE_VLC_BITS, 2);
if (h->mtype < 0) {
av_log(s->avctx, AV_LOG_ERROR, "Invalid mtype index %d\n",
h->mtype);
return SLICE_ERROR;
}
av_assert0(h->mtype < FF_ARRAY_ELEMS(ff_h261_mtype_map));
h->mtype = ff_h261_mtype_map[h->mtype];


if (IS_QUANT(h->mtype))
ff_set_qscale(s, get_bits(&s->gb, 5));

s->mb_intra = IS_INTRA4x4(h->mtype);


if (IS_16X16(h->mtype)) {








if ((h->current_mba == 1) || (h->current_mba == 12) ||
(h->current_mba == 23) || (h->mba_diff != 1)) {
h->current_mv_x = 0;
h->current_mv_y = 0;
}

h->current_mv_x = decode_mv_component(&s->gb, h->current_mv_x);
h->current_mv_y = decode_mv_component(&s->gb, h->current_mv_y);
} else {
h->current_mv_x = 0;
h->current_mv_y = 0;
}


if (HAS_CBP(h->mtype))
cbp = get_vlc2(&s->gb, h261_cbp_vlc.table, H261_CBP_VLC_BITS, 2) + 1;

if (s->mb_intra) {
s->current_picture.mb_type[xy] = MB_TYPE_INTRA;
goto intra;
}


s->mv_dir = MV_DIR_FORWARD;
s->mv_type = MV_TYPE_16X16;
s->current_picture.mb_type[xy] = MB_TYPE_16x16 | MB_TYPE_L0;
s->mv[0][0][0] = h->current_mv_x * 2; 
s->mv[0][0][1] = h->current_mv_y * 2;

if (s->current_picture.motion_val[0]) {
int b_stride = 2*s->mb_width + 1;
int b_xy = 2 * s->mb_x + (2 * s->mb_y) * b_stride;
s->current_picture.motion_val[0][b_xy][0] = s->mv[0][0][0];
s->current_picture.motion_val[0][b_xy][1] = s->mv[0][0][1];
}

intra:

if (s->mb_intra || HAS_CBP(h->mtype)) {
s->bdsp.clear_blocks(s->block[0]);
for (i = 0; i < 6; i++) {
if (h261_decode_block(h, s->block[i], i, cbp & 32) < 0)
return SLICE_ERROR;
cbp += cbp;
}
} else {
for (i = 0; i < 6; i++)
s->block_last_index[i] = -1;
}

ff_mpv_reconstruct_mb(s, s->block);

return SLICE_OK;
}





static int h261_decode_picture_header(H261Context *h)
{
MpegEncContext *const s = &h->s;
int format, i;
uint32_t startcode = 0;

for (i = get_bits_left(&s->gb); i > 24; i -= 1) {
startcode = ((startcode << 1) | get_bits(&s->gb, 1)) & 0x000FFFFF;

if (startcode == 0x10)
break;
}

if (startcode != 0x10) {
av_log(s->avctx, AV_LOG_ERROR, "Bad picture start code\n");
return -1;
}


i = get_bits(&s->gb, 5); 
if (i < (s->picture_number & 31))
i += 32;
s->picture_number = (s->picture_number & ~31) + i;

s->avctx->framerate = (AVRational) { 30000, 1001 };


skip_bits1(&s->gb); 
skip_bits1(&s->gb); 
skip_bits1(&s->gb); 

format = get_bits1(&s->gb);


if (format == 0) { 
s->width = 176;
s->height = 144;
s->mb_width = 11;
s->mb_height = 9;
} else { 
s->width = 352;
s->height = 288;
s->mb_width = 22;
s->mb_height = 18;
}

s->mb_num = s->mb_width * s->mb_height;

skip_bits1(&s->gb); 
skip_bits1(&s->gb); 


if (skip_1stop_8data_bits(&s->gb) < 0)
return AVERROR_INVALIDDATA;




s->pict_type = AV_PICTURE_TYPE_P;

h->gob_number = 0;
return 0;
}

static int h261_decode_gob(H261Context *h)
{
MpegEncContext *const s = &h->s;

ff_set_qscale(s, s->qscale);


while (h->current_mba <= MBA_STUFFING) {
int ret;

ret = h261_decode_mb(h);
if (ret < 0) {
if (ret == SLICE_END) {
h261_decode_mb_skipped(h, h->current_mba, 33);
return 0;
}
av_log(s->avctx, AV_LOG_ERROR, "Error at MB: %d\n",
s->mb_x + s->mb_y * s->mb_stride);
return -1;
}

h261_decode_mb_skipped(h,
h->current_mba - h->mba_diff,
h->current_mba - 1);
}

return -1;
}




static int get_consumed_bytes(MpegEncContext *s, int buf_size)
{
int pos = get_bits_count(&s->gb) >> 3;
if (pos == 0)
pos = 1; 
if (pos + 10 > buf_size)
pos = buf_size; 

return pos;
}

static int h261_decode_frame(AVCodecContext *avctx, void *data,
int *got_frame, AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
H261Context *h = avctx->priv_data;
MpegEncContext *s = &h->s;
int ret;
AVFrame *pict = data;

ff_dlog(avctx, "*****frame %d size=%d\n", avctx->frame_number, buf_size);
ff_dlog(avctx, "bytes=%x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

h->gob_start_code_skipped = 0;

retry:
init_get_bits(&s->gb, buf, buf_size * 8);

if (!s->context_initialized)

ff_mpv_idct_init(s);

ret = h261_decode_picture_header(h);


if (ret < 0) {
av_log(s->avctx, AV_LOG_ERROR, "header damaged\n");
return -1;
}

if (s->width != avctx->coded_width || s->height != avctx->coded_height) {
ParseContext pc = s->parse_context; 
s->parse_context.buffer = 0;
ff_mpv_common_end(s);
s->parse_context = pc;
}

if (!s->context_initialized) {
if ((ret = ff_mpv_common_init(s)) < 0)
return ret;

ret = ff_set_dimensions(avctx, s->width, s->height);
if (ret < 0)
return ret;

goto retry;
}


s->current_picture.f->pict_type = s->pict_type;
s->current_picture.f->key_frame = s->pict_type == AV_PICTURE_TYPE_I;

if ((avctx->skip_frame >= AVDISCARD_NONREF && s->pict_type == AV_PICTURE_TYPE_B) ||
(avctx->skip_frame >= AVDISCARD_NONKEY && s->pict_type != AV_PICTURE_TYPE_I) ||
avctx->skip_frame >= AVDISCARD_ALL)
return get_consumed_bytes(s, buf_size);

if (ff_mpv_frame_start(s, avctx) < 0)
return -1;

ff_mpeg_er_frame_start(s);


s->mb_x = 0;
s->mb_y = 0;

while (h->gob_number < (s->mb_height == 18 ? 12 : 5)) {
if (h261_resync(h) < 0)
break;
h261_decode_gob(h);
}
ff_mpv_frame_end(s);

av_assert0(s->current_picture.f->pict_type == s->current_picture_ptr->f->pict_type);
av_assert0(s->current_picture.f->pict_type == s->pict_type);

if ((ret = av_frame_ref(pict, s->current_picture_ptr->f)) < 0)
return ret;
ff_print_debug_info(s, s->current_picture_ptr, pict);

*got_frame = 1;

return get_consumed_bytes(s, buf_size);
}

static av_cold int h261_decode_end(AVCodecContext *avctx)
{
H261Context *h = avctx->priv_data;
MpegEncContext *s = &h->s;

ff_mpv_common_end(s);
return 0;
}

AVCodec ff_h261_decoder = {
.name = "h261",
.long_name = NULL_IF_CONFIG_SMALL("H.261"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_H261,
.priv_data_size = sizeof(H261Context),
.init = h261_decode_init,
.close = h261_decode_end,
.decode = h261_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.max_lowres = 3,
};
