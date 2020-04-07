


























#include "libavutil/attributes.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "h263.h"
#include "h261.h"
#include "mpegvideodata.h"

static uint8_t uni_h261_rl_len [64*64*2*2];
#define UNI_ENC_INDEX(last,run,level) ((last)*128*64 + (run)*128 + (level))

int ff_h261_get_picture_format(int width, int height)
{

if (width == 176 && height == 144)
return 0;

else if (width == 352 && height == 288)
return 1;

else
return AVERROR(EINVAL);
}

void ff_h261_encode_picture_header(MpegEncContext *s, int picture_number)
{
H261Context *h = (H261Context *)s;
int format, temp_ref;

avpriv_align_put_bits(&s->pb);


s->ptr_lastgob = put_bits_ptr(&s->pb);

put_bits(&s->pb, 20, 0x10); 

temp_ref = s->picture_number * 30000LL * s->avctx->time_base.num /
(1001LL * s->avctx->time_base.den); 
put_sbits(&s->pb, 5, temp_ref); 

put_bits(&s->pb, 1, 0); 
put_bits(&s->pb, 1, 0); 
put_bits(&s->pb, 1, s->pict_type == AV_PICTURE_TYPE_I); 

format = ff_h261_get_picture_format(s->width, s->height);

put_bits(&s->pb, 1, format); 

put_bits(&s->pb, 1, 1); 
put_bits(&s->pb, 1, 1); 

put_bits(&s->pb, 1, 0); 
if (format == 0)
h->gob_number = -1;
else
h->gob_number = 0;
s->mb_skip_run = 0;
}




static void h261_encode_gob_header(MpegEncContext *s, int mb_line)
{
H261Context *h = (H261Context *)s;
if (ff_h261_get_picture_format(s->width, s->height) == 0) {
h->gob_number += 2; 
} else {
h->gob_number++; 
}
put_bits(&s->pb, 16, 1); 
put_bits(&s->pb, 4, h->gob_number); 
put_bits(&s->pb, 5, s->qscale); 
put_bits(&s->pb, 1, 0); 
s->mb_skip_run = 0;
s->last_mv[0][0][0] = 0;
s->last_mv[0][0][1] = 0;
}

void ff_h261_reorder_mb_index(MpegEncContext *s)
{
int index = s->mb_x + s->mb_y * s->mb_width;

if (index % 11 == 0) {
if (index % 33 == 0)
h261_encode_gob_header(s, 0);
s->last_mv[0][0][0] = 0;
s->last_mv[0][0][1] = 0;
}



if (ff_h261_get_picture_format(s->width, s->height) == 1) { 
s->mb_x = index % 11;
index /= 11;
s->mb_y = index % 3;
index /= 3;
s->mb_x += 11 * (index % 2);
index /= 2;
s->mb_y += 3 * index;

ff_init_block_index(s);
ff_update_block_index(s);
}
}

static void h261_encode_motion(H261Context *h, int val)
{
MpegEncContext *const s = &h->s;
int sign, code;
if (val == 0) {
code = 0;
put_bits(&s->pb, ff_h261_mv_tab[code][1], ff_h261_mv_tab[code][0]);
} else {
if (val > 15)
val -= 32;
if (val < -16)
val += 32;
sign = val < 0;
code = sign ? -val : val;
put_bits(&s->pb, ff_h261_mv_tab[code][1], ff_h261_mv_tab[code][0]);
put_bits(&s->pb, 1, sign);
}
}

static inline int get_cbp(MpegEncContext *s, int16_t block[6][64])
{
int i, cbp;
cbp = 0;
for (i = 0; i < 6; i++)
if (s->block_last_index[i] >= 0)
cbp |= 1 << (5 - i);
return cbp;
}






static void h261_encode_block(H261Context *h, int16_t *block, int n)
{
MpegEncContext *const s = &h->s;
int level, run, i, j, last_index, last_non_zero, sign, slevel, code;
RLTable *rl;

rl = &ff_h261_rl_tcoeff;
if (s->mb_intra) {

level = block[0];

if (level > 254) {
level = 254;
block[0] = 254;
}

else if (level < 1) {
level = 1;
block[0] = 1;
}
if (level == 128)
put_bits(&s->pb, 8, 0xff);
else
put_bits(&s->pb, 8, level);
i = 1;
} else if ((block[0] == 1 || block[0] == -1) &&
(s->block_last_index[n] > -1)) {

put_bits(&s->pb, 2, block[0] > 0 ? 2 : 3);
i = 1;
} else {
i = 0;
}


last_index = s->block_last_index[n];
last_non_zero = i - 1;
for (; i <= last_index; i++) {
j = s->intra_scantable.permutated[i];
level = block[j];
if (level) {
run = i - last_non_zero - 1;
sign = 0;
slevel = level;
if (level < 0) {
sign = 1;
level = -level;
}
code = get_rl_index(rl, 0 ,
run, level);
if (run == 0 && level < 16)
code += 1;
put_bits(&s->pb, rl->table_vlc[code][1], rl->table_vlc[code][0]);
if (code == rl->n) {
put_bits(&s->pb, 6, run);
av_assert1(slevel != 0);
av_assert1(level <= 127);
put_sbits(&s->pb, 8, slevel);
} else {
put_bits(&s->pb, 1, sign);
}
last_non_zero = i;
}
}
if (last_index > -1)
put_bits(&s->pb, rl->table_vlc[0][1], rl->table_vlc[0][0]); 
}

void ff_h261_encode_mb(MpegEncContext *s, int16_t block[6][64],
int motion_x, int motion_y)
{
H261Context *h = (H261Context *)s;
int mvd, mv_diff_x, mv_diff_y, i, cbp;
cbp = 63; 
mvd = 0;

h->mtype = 0;

if (!s->mb_intra) {

cbp = get_cbp(s, block);


mvd = motion_x | motion_y;

if ((cbp | mvd) == 0) {

s->skip_count++;
s->mb_skip_run++;
s->last_mv[0][0][0] = 0;
s->last_mv[0][0][1] = 0;
s->qscale -= s->dquant;
return;
}
}


put_bits(&s->pb,
ff_h261_mba_bits[s->mb_skip_run],
ff_h261_mba_code[s->mb_skip_run]);
s->mb_skip_run = 0;


if (!s->mb_intra) {
h->mtype++;

if (mvd || s->loop_filter)
h->mtype += 3;
if (s->loop_filter)
h->mtype += 3;
if (cbp)
h->mtype++;
av_assert1(h->mtype > 1);
}

if (s->dquant && cbp) {
h->mtype++;
} else
s->qscale -= s->dquant;

put_bits(&s->pb,
ff_h261_mtype_bits[h->mtype],
ff_h261_mtype_code[h->mtype]);

h->mtype = ff_h261_mtype_map[h->mtype];

if (IS_QUANT(h->mtype)) {
ff_set_qscale(s, s->qscale + s->dquant);
put_bits(&s->pb, 5, s->qscale);
}

if (IS_16X16(h->mtype)) {
mv_diff_x = (motion_x >> 1) - s->last_mv[0][0][0];
mv_diff_y = (motion_y >> 1) - s->last_mv[0][0][1];
s->last_mv[0][0][0] = (motion_x >> 1);
s->last_mv[0][0][1] = (motion_y >> 1);
h261_encode_motion(h, mv_diff_x);
h261_encode_motion(h, mv_diff_y);
}

if (HAS_CBP(h->mtype)) {
av_assert1(cbp > 0);
put_bits(&s->pb,
ff_h261_cbp_tab[cbp - 1][1],
ff_h261_cbp_tab[cbp - 1][0]);
}
for (i = 0; i < 6; i++)

h261_encode_block(h, block[i], i);

if (!IS_16X16(h->mtype)) {
s->last_mv[0][0][0] = 0;
s->last_mv[0][0][1] = 0;
}
}

static av_cold void init_uni_h261_rl_tab(RLTable *rl, uint32_t *bits_tab,
uint8_t *len_tab)
{
int slevel, run, last;

av_assert0(MAX_LEVEL >= 64);
av_assert0(MAX_RUN >= 63);

for(slevel=-64; slevel<64; slevel++){
if(slevel==0) continue;
for(run=0; run<64; run++){
for(last=0; last<=1; last++){
const int index= UNI_ENC_INDEX(last, run, slevel+64);
int level= slevel < 0 ? -slevel : slevel;
int len, code;

len_tab[index]= 100;


code= get_rl_index(rl, 0, run, level);
len= rl->table_vlc[code][1] + 1;
if(last)
len += 2;

if(code!=rl->n && len < len_tab[index]){
len_tab [index]= len;
}

len = rl->table_vlc[rl->n][1];
if(last)
len += 2;

if(len < len_tab[index]){
len_tab [index]= len;
}
}
}
}
}

av_cold void ff_h261_encode_init(MpegEncContext *s)
{
ff_h261_common_init();

s->min_qcoeff = -127;
s->max_qcoeff = 127;
s->y_dc_scale_table =
s->c_dc_scale_table = ff_mpeg1_dc_scale_table;
s->ac_esc_length = 6+6+8;

init_uni_h261_rl_tab(&ff_h261_rl_tcoeff, NULL, uni_h261_rl_len);

s->intra_ac_vlc_length = s->inter_ac_vlc_length = uni_h261_rl_len;
s->intra_ac_vlc_last_length = s->inter_ac_vlc_last_length = uni_h261_rl_len + 128*64;
}

static const AVClass h261_class = {
.class_name = "h261 encoder",
.item_name = av_default_item_name,
.option = ff_mpv_generic_options,
.version = LIBAVUTIL_VERSION_INT,
};

AVCodec ff_h261_encoder = {
.name = "h261",
.long_name = NULL_IF_CONFIG_SMALL("H.261"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_H261,
.priv_data_size = sizeof(H261Context),
.init = ff_mpv_encode_init,
.encode2 = ff_mpv_encode_picture,
.close = ff_mpv_encode_end,
.pix_fmts = (const enum AVPixelFormat[]) { AV_PIX_FMT_YUV420P,
AV_PIX_FMT_NONE },
.priv_class = &h261_class,
};
