


























#include "libavutil/avassert.h"
#include "libavutil/imgutils.h"
#include "internal.h"
#include "cabac.h"
#include "cabac_functions.h"
#include "error_resilience.h"
#include "avcodec.h"
#include "h264dec.h"
#include "h264data.h"
#include "h264chroma.h"
#include "h264_mvpred.h"
#include "mathops.h"
#include "mpegutils.h"
#include "rectangle.h"
#include "thread.h"

void ff_h264_unref_picture(H264Context *h, H264Picture *pic)
{
int off = offsetof(H264Picture, tf) + sizeof(pic->tf);
int i;

if (!pic->f || !pic->f->buf[0])
return;

ff_thread_release_buffer(h->avctx, &pic->tf);
av_buffer_unref(&pic->hwaccel_priv_buf);

av_buffer_unref(&pic->qscale_table_buf);
av_buffer_unref(&pic->mb_type_buf);
for (i = 0; i < 2; i++) {
av_buffer_unref(&pic->motion_val_buf[i]);
av_buffer_unref(&pic->ref_index_buf[i]);
}

memset((uint8_t*)pic + off, 0, sizeof(*pic) - off);
}

int ff_h264_ref_picture(H264Context *h, H264Picture *dst, H264Picture *src)
{
int ret, i;

av_assert0(!dst->f->buf[0]);
av_assert0(src->f->buf[0]);
av_assert0(src->tf.f == src->f);

dst->tf.f = dst->f;
ret = ff_thread_ref_frame(&dst->tf, &src->tf);
if (ret < 0)
goto fail;

dst->qscale_table_buf = av_buffer_ref(src->qscale_table_buf);
dst->mb_type_buf = av_buffer_ref(src->mb_type_buf);
if (!dst->qscale_table_buf || !dst->mb_type_buf) {
ret = AVERROR(ENOMEM);
goto fail;
}
dst->qscale_table = src->qscale_table;
dst->mb_type = src->mb_type;

for (i = 0; i < 2; i++) {
dst->motion_val_buf[i] = av_buffer_ref(src->motion_val_buf[i]);
dst->ref_index_buf[i] = av_buffer_ref(src->ref_index_buf[i]);
if (!dst->motion_val_buf[i] || !dst->ref_index_buf[i]) {
ret = AVERROR(ENOMEM);
goto fail;
}
dst->motion_val[i] = src->motion_val[i];
dst->ref_index[i] = src->ref_index[i];
}

if (src->hwaccel_picture_private) {
dst->hwaccel_priv_buf = av_buffer_ref(src->hwaccel_priv_buf);
if (!dst->hwaccel_priv_buf) {
ret = AVERROR(ENOMEM);
goto fail;
}
dst->hwaccel_picture_private = dst->hwaccel_priv_buf->data;
}

for (i = 0; i < 2; i++)
dst->field_poc[i] = src->field_poc[i];

memcpy(dst->ref_poc, src->ref_poc, sizeof(src->ref_poc));
memcpy(dst->ref_count, src->ref_count, sizeof(src->ref_count));

dst->poc = src->poc;
dst->frame_num = src->frame_num;
dst->mmco_reset = src->mmco_reset;
dst->long_ref = src->long_ref;
dst->mbaff = src->mbaff;
dst->field_picture = src->field_picture;
dst->reference = src->reference;
dst->recovered = src->recovered;
dst->invalid_gap = src->invalid_gap;
dst->sei_recovery_frame_cnt = src->sei_recovery_frame_cnt;

return 0;
fail:
ff_h264_unref_picture(h, dst);
return ret;
}

void ff_h264_set_erpic(ERPicture *dst, H264Picture *src)
{
#if CONFIG_ERROR_RESILIENCE
int i;

memset(dst, 0, sizeof(*dst));

if (!src)
return;

dst->f = src->f;
dst->tf = &src->tf;

for (i = 0; i < 2; i++) {
dst->motion_val[i] = src->motion_val[i];
dst->ref_index[i] = src->ref_index[i];
}

dst->mb_type = src->mb_type;
dst->field_picture = src->field_picture;
#endif 
}

int ff_h264_field_end(H264Context *h, H264SliceContext *sl, int in_setup)
{
AVCodecContext *const avctx = h->avctx;
int err = 0;
h->mb_y = 0;

if (in_setup || !(avctx->active_thread_type & FF_THREAD_FRAME)) {
if (!h->droppable) {
err = ff_h264_execute_ref_pic_marking(h);
h->poc.prev_poc_msb = h->poc.poc_msb;
h->poc.prev_poc_lsb = h->poc.poc_lsb;
}
h->poc.prev_frame_num_offset = h->poc.frame_num_offset;
h->poc.prev_frame_num = h->poc.frame_num;
}

if (avctx->hwaccel) {
err = avctx->hwaccel->end_frame(avctx);
if (err < 0)
av_log(avctx, AV_LOG_ERROR,
"hardware accelerator failed to decode picture\n");
}

if (!in_setup && !h->droppable)
ff_thread_report_progress(&h->cur_pic_ptr->tf, INT_MAX,
h->picture_structure == PICT_BOTTOM_FIELD);
emms_c();

h->current_slice = 0;

return err;
}
