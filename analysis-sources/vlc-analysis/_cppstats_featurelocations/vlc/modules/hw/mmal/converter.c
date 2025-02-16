























#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdatomic.h>

#include <vlc_common.h>
#include <vlc_codec.h>
#include <vlc_filter.h>
#include <vlc_plugin.h>

#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/util/mmal_default_components.h>

#include "mmal_picture.h"

#include "subpic.h"

typedef enum filter_resizer_e {
FILTER_RESIZER_HVS,
FILTER_RESIZER_ISP,
FILTER_RESIZER_RESIZER,
} filter_resizer_t;

#if !defined(NDEBUG)
#define MMAL_CONVERTER_TYPE_NAME "mmal-hw-converter"
#define MMAL_CONVERTER_TYPE_TEXT "Hardware used for MMAL conversions"
#define MMAL_CONVERTER_TYPE_LONGTEXT "Hardware component used for MMAL conversions. Hardware Video Scaler"" (default) gives the best result and allows blending, Resizer is slower but uses less memory."

#endif

static const int pi_converter_modes[] = {
FILTER_RESIZER_HVS, FILTER_RESIZER_ISP, FILTER_RESIZER_RESIZER,
};
static const char * const ppsz_converter_text[] = {
"Hardware Video Scaler", "ISP", "Resizer"
};

int OpenConverter(vlc_object_t *);
void CloseConverter(vlc_object_t *);

vlc_module_begin()
add_submodule()
set_category( CAT_VIDEO )
set_subcategory( SUBCAT_VIDEO_VFILTER )
set_shortname(N_("MMAL resizer"))
set_description(N_("MMAL resizing conversion filter"))
add_shortcut("mmal_converter")
set_capability( "video converter", 900 )
#if !defined(NDEBUG)
add_integer( MMAL_CONVERTER_TYPE_NAME, FILTER_RESIZER_HVS, MMAL_CONVERTER_TYPE_TEXT, MMAL_CONVERTER_TYPE_LONGTEXT, true )
change_integer_list( pi_converter_modes, ppsz_converter_text )
#endif
set_callbacks(OpenConverter, CloseConverter)
vlc_module_end()

#define MMAL_SLICE_HEIGHT 16

#define CONV_MAX_LATENCY 1 

static MMAL_FOURCC_T pic_to_slice_mmal_fourcc(MMAL_FOURCC_T fcc)
{
switch (fcc){
case MMAL_ENCODING_I420:
return MMAL_ENCODING_I420_SLICE;
case MMAL_ENCODING_I422:
return MMAL_ENCODING_I422_SLICE;
case MMAL_ENCODING_ARGB:
return MMAL_ENCODING_ARGB_SLICE;
case MMAL_ENCODING_RGBA:
return MMAL_ENCODING_RGBA_SLICE;
case MMAL_ENCODING_ABGR:
return MMAL_ENCODING_ABGR_SLICE;
case MMAL_ENCODING_BGRA:
return MMAL_ENCODING_BGRA_SLICE;
case MMAL_ENCODING_RGB16:
return MMAL_ENCODING_RGB16_SLICE;
case MMAL_ENCODING_RGB24:
return MMAL_ENCODING_RGB24_SLICE;
case MMAL_ENCODING_RGB32:
return MMAL_ENCODING_RGB32_SLICE;
case MMAL_ENCODING_BGR16:
return MMAL_ENCODING_BGR16_SLICE;
case MMAL_ENCODING_BGR24:
return MMAL_ENCODING_BGR24_SLICE;
case MMAL_ENCODING_BGR32:
return MMAL_ENCODING_BGR32_SLICE;
default:
break;
}
return 0;
}

typedef struct pic_fifo_s {
picture_t * head;
picture_t * tail;
} pic_fifo_t;

static picture_t * pic_fifo_get(pic_fifo_t * const pf)
{
picture_t * const pic = pf->head;;
if (pic != NULL) {
pf->head = pic->p_next;
pic->p_next = NULL;
}
return pic;
}

static void pic_fifo_release_all(pic_fifo_t * const pf)
{
picture_t * pic;
while ((pic = pic_fifo_get(pf)) != NULL) {
picture_Release(pic);
}
}

static void pic_fifo_init(pic_fifo_t * const pf)
{
pf->head = NULL;
pf->tail = NULL; 
}

static void pic_fifo_put(pic_fifo_t * const pf, picture_t * pic)
{
pic->p_next = NULL;
if (pf->head == NULL)
pf->head = pic;
else
pf->tail->p_next = pic;
pf->tail = pic;
}

#define SUBS_MAX 3

typedef struct conv_frame_stash_s
{
mtime_t pts;
MMAL_BUFFER_HEADER_T * sub_bufs[SUBS_MAX];
} conv_frame_stash_t;

typedef struct
{
filter_resizer_t resizer_type;
MMAL_COMPONENT_T *component;
MMAL_PORT_T *input;
MMAL_PORT_T *output;
MMAL_POOL_T *out_pool; 
MMAL_POOL_T *in_pool; 

cma_buf_pool_t * cma_in_pool;
cma_buf_pool_t * cma_out_pool;

subpic_reg_stash_t subs[SUBS_MAX];

pic_fifo_t ret_pics;

unsigned int pic_n;
vlc_sem_t sem;
vlc_mutex_t lock;

MMAL_STATUS_T err_stream;

bool needs_copy_in;
bool is_sliced;
bool out_fmt_set;
MMAL_PORT_BH_CB_T in_port_cb_fn;
MMAL_PORT_BH_CB_T out_port_cb_fn;

uint64_t frame_seq;
conv_frame_stash_t stash[16];


struct {
pic_fifo_t pics;
unsigned int line; 
} slice;

vlc_decoder_device *dec_dev;
} converter_sys_t;


static void cma_buf_pool_deletez(cma_buf_pool_t ** const pp)
{
cma_buf_pool_t * const p = *pp;
if (p != NULL) {
*pp = NULL;
cma_buf_pool_delete(p);
}
}

static MMAL_STATUS_T pic_to_format(MMAL_ES_FORMAT_T * const es_fmt, const picture_t * const pic)
{
unsigned int bpp = (pic->format.i_bits_per_pixel + 7) >> 3;
MMAL_VIDEO_FORMAT_T * const v_fmt = &es_fmt->es->video;

es_fmt->type = MMAL_ES_TYPE_VIDEO;
es_fmt->encoding = vlc_to_mmal_video_fourcc(&pic->format);
es_fmt->encoding_variant = 0;


hw_mmal_vlc_fmt_to_mmal_fmt(es_fmt, &pic->format);

if (bpp != 0) {
v_fmt->width = pic->p[0].i_pitch / bpp;
v_fmt->height = pic->p[0].i_lines;
}
return MMAL_SUCCESS;
}


static MMAL_STATUS_T conv_enable_in(filter_t * const p_filter, converter_sys_t * const sys)
{
MMAL_STATUS_T err = MMAL_SUCCESS;

if (!sys->input->is_enabled &&
(err = mmal_port_enable(sys->input, sys->in_port_cb_fn)) != MMAL_SUCCESS)
{
msg_Err(p_filter, "Failed to enable input port %s (status=%"PRIx32" %s)",
sys->input->name, err, mmal_status_to_string(err));
}
return err;
}

static MMAL_STATUS_T conv_enable_out(filter_t * const p_filter, converter_sys_t * const sys)
{
MMAL_STATUS_T err = MMAL_SUCCESS;

{
cma_buf_pool_deletez(&sys->cma_out_pool);
}

if (!sys->output->is_enabled &&
(err = mmal_port_enable(sys->output, sys->out_port_cb_fn)) != MMAL_SUCCESS)
{
msg_Err(p_filter, "Failed to enable output port %s (status=%"PRIx32" %s)",
sys->output->name, err, mmal_status_to_string(err));
}
return err;
}

static void conv_control_port_cb(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
filter_t * const p_filter = (filter_t *)port->userdata;

if (buffer->cmd == MMAL_EVENT_ERROR) {
MMAL_STATUS_T status = *(uint32_t *)buffer->data;

converter_sys_t * sys = p_filter->p_sys;
sys->err_stream = status;

msg_Err(p_filter, "MMAL error %"PRIx32" \"%s\"", status,
mmal_status_to_string(status));
}

mmal_buffer_header_release(buffer);
}

static void conv_input_port_cb(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf)
{
VLC_UNUSED(port);

mmal_buffer_header_release(buf);
}

static void conv_out_q_pic(converter_sys_t * const sys, picture_t * const pic)
{
pic->p_next = NULL;

vlc_mutex_lock(&sys->lock);
pic_fifo_put(&sys->ret_pics, pic);
vlc_mutex_unlock(&sys->lock);

vlc_sem_post(&sys->sem);
}

static void conv_output_port_cb(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf)
{
filter_t * const p_filter = (filter_t *)port->userdata;
converter_sys_t * const sys = p_filter->p_sys;

if (buf->cmd == 0) {
picture_t * const pic = (picture_t *)buf->user_data;

if (pic == NULL) {
msg_Err(p_filter, "Buffer has no attached picture");
}
else if (buf->data != NULL && buf->length != 0)
{
buf_to_pic_copy_props(pic, buf);

buf->user_data = NULL; 
conv_out_q_pic(sys, pic);
}
}

mmal_buffer_header_release(buf);
}


static void slice_output_port_cb(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buf)
{
filter_t * const p_filter = (filter_t *)port->userdata;
converter_sys_t * const sys = p_filter->p_sys;

if (buf->cmd != 0)
{
mmal_buffer_header_release(buf);
return;
}

if (buf->data != NULL && buf->length != 0)
{

picture_t *pic = sys->slice.pics.head;
const unsigned int scale_lines = sys->output->format->es->video.height; 

if (pic == NULL) {
msg_Err(p_filter, "No output picture");
goto fail;
}



{
const unsigned int scale_n = __MIN(scale_lines - sys->slice.line, MMAL_SLICE_HEIGHT);
const unsigned int pic_lines = pic->p[0].i_lines;
const unsigned int copy_n = sys->slice.line + scale_n <= pic_lines ? scale_n :
sys->slice.line >= pic_lines ? 0 :
pic_lines - sys->slice.line;

const unsigned int src_stride = buf->type->video.pitch[0];
const unsigned int dst_stride = pic->p[0].i_pitch;
uint8_t *dst = pic->p[0].p_pixels + sys->slice.line * dst_stride;
const uint8_t *src = buf->data + buf->type->video.offset[0];

if (src_stride == dst_stride) {
if (copy_n != 0)
memcpy(dst, src, src_stride * copy_n);
}
else {
unsigned int i;
for (i = 0; i != copy_n; ++i) {
memcpy(dst, src, __MIN(dst_stride, src_stride));
dst += dst_stride;
src += src_stride;
}
}
sys->slice.line += scale_n;
}

if ((buf->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END) != 0 || sys->slice.line >= scale_lines) {

if ((buf->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END) == 0 || sys->slice.line != scale_lines) {

msg_Err(p_filter, "Line count (%d/%d) & EOF disagree (flags=%#x)", sys->slice.line, scale_lines, buf->flags);
goto fail;
}
else {
sys->slice.line = 0;

vlc_mutex_lock(&sys->lock);
pic_fifo_get(&sys->slice.pics); 
vlc_mutex_unlock(&sys->lock);

buf_to_pic_copy_props(pic, buf);
conv_out_q_pic(sys, pic);
}
}
}


buf->user_data = NULL; 
mmal_buffer_header_reset(buf);

if (mmal_port_send_buffer(sys->output, buf) != MMAL_SUCCESS) {
mmal_buffer_header_release(buf);
}
return;

fail:
sys->err_stream = MMAL_EIO;
vlc_sem_post(&sys->sem); 
}


static void conv_flush(filter_t * p_filter)
{
converter_sys_t * const sys = p_filter->p_sys;
unsigned int i;

if (sys->resizer_type == FILTER_RESIZER_HVS)
{
for (i = 0; i != SUBS_MAX; ++i) {
hw_mmal_subpic_flush(sys->subs + i);
}
}

if (sys->input != NULL && sys->input->is_enabled)
mmal_port_disable(sys->input);

if (sys->output != NULL && sys->output->is_enabled)
mmal_port_disable(sys->output);







for (i = 0; i != ARRAY_SIZE(sys->stash); ++i) {
conv_frame_stash_t *const stash = sys->stash + i;
unsigned int sub_no;

stash->pts = MMAL_TIME_UNKNOWN;
for (sub_no = 0; sub_no != SUBS_MAX; ++sub_no) {
if (stash->sub_bufs[sub_no] != NULL) {
mmal_buffer_header_release(stash->sub_bufs[sub_no]);
stash->sub_bufs[sub_no] = NULL;
}
}
}

pic_fifo_release_all(&sys->slice.pics);
pic_fifo_release_all(&sys->ret_pics);


vlc_sem_init(&sys->sem, 0);
sys->pic_n = 0;


sys->err_stream = MMAL_SUCCESS;
}

static void conv_stash_fixup(filter_t * const p_filter, converter_sys_t * const sys, picture_t * const p_pic)
{
conv_frame_stash_t * const stash = sys->stash + (p_pic->date & 0xf);
unsigned int sub_no;
VLC_UNUSED(p_filter);

p_pic->date = stash->pts;
for (sub_no = 0; sub_no != SUBS_MAX; ++sub_no) {
if (stash->sub_bufs[sub_no] != NULL) {



mmal_buffer_header_release(stash->sub_bufs[sub_no]);
stash->sub_bufs[sub_no] = NULL;
}
}
}



static MMAL_BOOL_T out_buffer_pre_release_cb(MMAL_BUFFER_HEADER_T *header, void *userdata)
{
VLC_UNUSED(userdata);

picture_t * const pic = header->user_data;
header->user_data = NULL;

if (pic != NULL)
picture_Release(pic);

return MMAL_FALSE;
}

static MMAL_STATUS_T conv_set_output(filter_t * const p_filter, converter_sys_t * const sys, picture_t * const pic)
{
MMAL_STATUS_T status;

sys->output->userdata = (struct MMAL_PORT_USERDATA_T *)p_filter;
sys->output->format->type = MMAL_ES_TYPE_VIDEO;
sys->output->format->encoding = vlc_to_mmal_video_fourcc(&p_filter->fmt_out.video);
sys->output->format->encoding_variant = 0;
hw_mmal_vlc_fmt_to_mmal_fmt(sys->output->format, &p_filter->fmt_out.video);

if (pic != NULL)
{

if ((status = pic_to_format(sys->output->format, pic)) != MMAL_SUCCESS)
{
msg_Err(p_filter, "Bad format desc: %4.4s, pic=%p, bits=%d", (const char*)&pic->format.i_chroma, pic, pic->format.i_bits_per_pixel);
return status;
}
}

if (sys->is_sliced) {

sys->output->format->es->video.height = MMAL_SLICE_HEIGHT;
}

mmal_log_dump_format(sys->output->format);

status = mmal_port_format_commit(sys->output);
if (status != MMAL_SUCCESS) {
msg_Err(p_filter, "Failed to commit format for output port %s (status=%"PRIx32" %s)",
sys->output->name, status, mmal_status_to_string(status));
return status;
}

sys->output->buffer_num = __MAX(sys->is_sliced ? 16 : 2, sys->output->buffer_num_recommended);
sys->output->buffer_size = sys->output->buffer_size_recommended;

if ((status = conv_enable_out(p_filter, sys)) != MMAL_SUCCESS)
return status;

return MMAL_SUCCESS;
}

static picture_t *conv_filter(filter_t *p_filter, picture_t *p_pic)
{
converter_sys_t * const sys = p_filter->p_sys;
picture_t * ret_pics;
MMAL_STATUS_T err;
const uint64_t frame_seq = ++sys->frame_seq;
conv_frame_stash_t * const stash = sys->stash + (frame_seq & 0xf);
MMAL_BUFFER_HEADER_T * out_buf = NULL;

if (sys->err_stream != MMAL_SUCCESS) {
goto stream_fail;
}




if (hw_mmal_vlc_pic_to_mmal_fmt_update(sys->input->format, p_pic))
{
msg_Dbg(p_filter, "Reset input port format");
mmal_port_format_commit(sys->input);
}

if (p_pic->context == NULL) {

if (!sys->needs_copy_in)
msg_Dbg(p_filter, "No context");
}
else if (sys->resizer_type == FILTER_RESIZER_HVS)
{
unsigned int sub_no = 0;

for (sub_no = 0; sub_no != SUBS_MAX; ++sub_no) {
int rv;
if ((rv = hw_mmal_subpic_update(VLC_OBJECT(p_filter),
hw_mmal_pic_sub_buf_get(p_pic, sub_no),
sys->subs + sub_no,
&p_pic->format,
&sys->output->format->es->video.crop,
frame_seq)) == 0)
break;
else if (rv < 0)
goto fail;
}
}
else
{
unsigned int sub_no = 0;
for (sub_no = 0; sub_no != SUBS_MAX; ++sub_no) {
if ((stash->sub_bufs[sub_no] = hw_mmal_pic_sub_buf_get(p_pic, sub_no)) != NULL) {
mmal_buffer_header_acquire(stash->sub_bufs[sub_no]);
}
}
}

if (!sys->out_fmt_set) {
sys->out_fmt_set = true;

if (sys->is_sliced) {


if ((err = conv_set_output(p_filter, sys, NULL)) != MMAL_SUCCESS)
goto fail;

sys->out_pool = mmal_port_pool_create(sys->output, sys->output->buffer_num, sys->output->buffer_size);
}
else {
picture_t *pic = filter_NewPicture(p_filter);
err = conv_set_output(p_filter, sys, pic);
picture_Release(pic);
if (err != MMAL_SUCCESS)
goto fail;

sys->out_pool = mmal_pool_create(sys->output->buffer_num, 0);
}

if (sys->out_pool == NULL) {
msg_Err(p_filter, "Failed to create output pool");
goto fail;
}
}


if ((err = conv_enable_out(p_filter, sys)) != MMAL_SUCCESS ||
(err = conv_enable_in(p_filter, sys)) != MMAL_SUCCESS)
goto fail;




{
picture_t * const out_pic = filter_NewPicture(p_filter);

if (out_pic == NULL)
{
msg_Err(p_filter, "Failed to alloc required filter output pic");
goto fail;
}

out_pic->format.i_sar_den = p_filter->fmt_out.video.i_sar_den;
out_pic->format.i_sar_num = p_filter->fmt_out.video.i_sar_num;

if (sys->is_sliced) {
vlc_mutex_lock(&sys->lock);
pic_fifo_put(&sys->slice.pics, out_pic);
vlc_mutex_unlock(&sys->lock);



while ((out_buf = mmal_queue_get(sys->out_pool->queue)) != NULL)
mmal_port_send_buffer(sys->output, out_buf);
}
else
{

if ((out_buf = mmal_queue_wait(sys->out_pool->queue)) == NULL)
{
msg_Err(p_filter, "Failed to get output buffer");
picture_Release(out_pic);
goto fail;
}
mmal_buffer_header_reset(out_buf);



out_buf->user_data = out_pic;
mmal_buffer_header_pre_release_cb_set(out_buf, out_buffer_pre_release_cb, NULL);

{
out_buf->data = out_pic->p[0].p_pixels;
out_buf->alloc_size = out_pic->p[0].i_pitch * out_pic->p[0].i_lines;

}

if ((err = mmal_port_send_buffer(sys->output, out_buf)) != MMAL_SUCCESS)
{
msg_Err(p_filter, "Send buffer to output failed");
goto fail;
}
out_buf = NULL;
}
}




stash->pts = p_pic->date;
{
mmal_decoder_device_t *devsys = GetMMALDeviceOpaque(sys->dec_dev);
MMAL_BUFFER_HEADER_T *const pic_buf = sys->needs_copy_in ?
hw_mmal_pic_buf_copied(p_pic, sys->in_pool, sys->input, sys->cma_in_pool,
devsys->is_cma) :
hw_mmal_pic_buf_replicated(p_pic, sys->in_pool);


picture_Release(p_pic);
p_pic = NULL;

if (pic_buf == NULL) {
msg_Err(p_filter, "Pic has no attached buffer");
goto fail;
}

pic_buf->pts = frame_seq;

if ((err = mmal_port_send_buffer(sys->input, pic_buf)) != MMAL_SUCCESS)
{
msg_Err(p_filter, "Send buffer to input failed");
mmal_buffer_header_release(pic_buf);
goto fail;
}
}




if (sys->pic_n++ == 1) {
return NULL;
}
vlc_sem_wait(&sys->sem);


vlc_mutex_lock(&sys->lock);
ret_pics = pic_fifo_get(&sys->ret_pics);
vlc_mutex_unlock(&sys->lock);

if (sys->err_stream != MMAL_SUCCESS)
goto stream_fail;

conv_stash_fixup(p_filter, sys, ret_pics);

return ret_pics;

stream_fail:
msg_Err(p_filter, "MMAL error reported by callback");
fail:
if (out_buf != NULL)
mmal_buffer_header_release(out_buf);
if (p_pic != NULL)
picture_Release(p_pic);
conv_flush(p_filter);
return NULL;
}

void CloseConverter(vlc_object_t * obj)
{
filter_t * const p_filter = (filter_t *)obj;
converter_sys_t * const sys = p_filter->p_sys;

if (sys == NULL)
return;


conv_flush(p_filter);

cma_buf_pool_deletez(&sys->cma_in_pool);
cma_buf_pool_deletez(&sys->cma_out_pool);

if (sys->component && sys->component->control->is_enabled)
mmal_port_disable(sys->component->control);

if (sys->component && sys->component->is_enabled)
mmal_component_disable(sys->component);

if (sys->resizer_type == FILTER_RESIZER_HVS)
{
for (size_t i = 0; i != SUBS_MAX; ++i) {
hw_mmal_subpic_close(sys->subs + i);
}
}

if (sys->out_pool)
{
if (sys->is_sliced)
mmal_port_pool_destroy(sys->output, sys->out_pool);
else
mmal_pool_destroy(sys->out_pool);
}

if (sys->in_pool != NULL)
mmal_pool_destroy(sys->in_pool);

if (sys->component)
mmal_component_release(sys->component);

if (p_filter->vctx_out)
vlc_video_context_Release(p_filter->vctx_out);

if (sys->dec_dev)
vlc_decoder_device_Release(sys->dec_dev);

p_filter->p_sys = NULL;
free(sys);
}


static MMAL_FOURCC_T filter_enc_in(const video_format_t * const fmt)
{
if (hw_mmal_chroma_is_mmal(fmt->i_chroma) || fmt->i_chroma == VLC_CODEC_I420)
return vlc_to_mmal_video_fourcc(fmt);

if (fmt->i_chroma == VLC_CODEC_I420_10L)
return MMAL_ENCODING_I420;

return 0;
}


int OpenConverter(vlc_object_t * obj)
{
filter_t * const p_filter = (filter_t *)obj;
int ret = VLC_EGENERIC;
converter_sys_t *sys;
MMAL_STATUS_T status;
if (p_filter->fmt_out.video.i_chroma == VLC_CODEC_I420)
return VLC_EGENERIC;
const MMAL_FOURCC_T enc_out = vlc_to_mmal_video_fourcc(&p_filter->fmt_out.video);
const MMAL_FOURCC_T enc_in = filter_enc_in(&p_filter->fmt_in.video);


if (enc_in == 0 || enc_out == 0)
return VLC_EGENERIC;

filter_resizer_t resizer_type =
#if defined(NDEBUG)
FILTER_RESIZER_HVS;
#else 
var_GetInteger(p_filter, MMAL_CONVERTER_TYPE_NAME);
#endif 

retry:



if (enc_in == MMAL_ENCODING_YUVUV64_10) {

if (resizer_type == FILTER_RESIZER_RESIZER)
return VLC_EGENERIC;

resizer_type = FILTER_RESIZER_ISP;
}

if (enc_out == MMAL_ENCODING_I420 && resizer_type == FILTER_RESIZER_HVS) {
resizer_type = FILTER_RESIZER_ISP;
}

if (enc_in == MMAL_ENCODING_YUV10_COL) {
if (resizer_type != FILTER_RESIZER_HVS)
return VLC_EGENERIC;
}


if (resizer_type == FILTER_RESIZER_RESIZER && pic_to_slice_mmal_fourcc(enc_out) == 0) {
return VLC_EGENERIC;
}

sys = calloc(1, sizeof(converter_sys_t));
if (!sys) {
ret = VLC_ENOMEM;
goto fail;
}
p_filter->p_sys = sys;


vlc_mutex_init(&sys->lock);
vlc_sem_init(&sys->sem, 0);
sys->err_stream = MMAL_SUCCESS;
pic_fifo_init(&sys->ret_pics);
pic_fifo_init(&sys->slice.pics);

sys->needs_copy_in = !hw_mmal_chroma_is_mmal(p_filter->fmt_in.video.i_chroma);
sys->in_port_cb_fn = conv_input_port_cb;

if (hw_mmal_chroma_is_mmal(p_filter->fmt_in.video.i_chroma))
{
assert(p_filter->vctx_in);
sys->dec_dev = vlc_video_context_HoldDevice(p_filter->vctx_in);
assert(sys->dec_dev->type == VLC_DECODER_DEVICE_MMAL);
}
else
sys->dec_dev = filter_HoldDecoderDeviceType(p_filter, VLC_DECODER_DEVICE_MMAL);
if (sys->dec_dev == NULL) {
msg_Err(p_filter, "missing MMAL decoder device");
goto fail;
}

if (hw_mmal_chroma_is_mmal(p_filter->fmt_out.video.i_chroma))
{
if (hw_mmal_chroma_is_mmal(p_filter->fmt_in.video.i_chroma))
p_filter->vctx_out = vlc_video_context_Hold(p_filter->vctx_in);
else {
p_filter->vctx_out = vlc_video_context_Create(sys->dec_dev, VLC_VIDEO_CONTEXT_MMAL, 0, NULL);
if (unlikely(p_filter->vctx_out == NULL)) {
msg_Err(p_filter, "failed to create the output video context");
goto fail;
}
}
}

const char *component_name;
if (resizer_type == FILTER_RESIZER_RESIZER) {
sys->is_sliced = true;
sys->out_port_cb_fn = slice_output_port_cb;
component_name = MMAL_COMPONENT_DEFAULT_RESIZER;
}
else {
sys->is_sliced = false; 
sys->out_port_cb_fn = conv_output_port_cb;
if (resizer_type == FILTER_RESIZER_ISP)
component_name = MMAL_COMPONENT_ISP_RESIZER;
else
component_name = MMAL_COMPONENT_HVS;
}
sys->resizer_type = resizer_type;

status = mmal_component_create(component_name, &sys->component);
if (status != MMAL_SUCCESS) {
if (resizer_type == FILTER_RESIZER_HVS) {
msg_Warn(p_filter, "Failed to create HVS resizer - retrying with ISP");
CloseConverter(obj);
resizer_type = FILTER_RESIZER_ISP;
goto retry;
}
msg_Err(p_filter, "Failed to create MMAL component %s (status=%"PRIx32" %s)",
component_name, status, mmal_status_to_string(status));
goto fail;
}
sys->output = sys->component->output[0];
sys->input = sys->component->input[0];

sys->component->control->userdata = (struct MMAL_PORT_USERDATA_T *)p_filter;
status = mmal_port_enable(sys->component->control, conv_control_port_cb);
if (status != MMAL_SUCCESS) {
msg_Err(p_filter, "Failed to enable control port %s (status=%"PRIx32" %s)",
sys->component->control->name, status, mmal_status_to_string(status));
goto fail;
}

mmal_decoder_device_t *devsys = GetMMALDeviceOpaque(sys->dec_dev);
assert(devsys != NULL);

if (sys->needs_copy_in &&
(sys->cma_in_pool = cma_buf_pool_new(2, 2, devsys->is_cma, "conv-copy-in")) == NULL)
{
msg_Err(p_filter, "Failed to allocate input CMA pool");
goto fail;
}

sys->input->userdata = (struct MMAL_PORT_USERDATA_T *)p_filter;
sys->input->format->type = MMAL_ES_TYPE_VIDEO;
sys->input->format->encoding = enc_in;
sys->input->format->encoding_variant = MMAL_ENCODING_I420;
hw_mmal_vlc_fmt_to_mmal_fmt(sys->input->format, &p_filter->fmt_in.video);
port_parameter_set_bool(sys->input, MMAL_PARAMETER_ZERO_COPY, 1);

mmal_log_dump_format(sys->input->format);

status = mmal_port_format_commit(sys->input);
if (status != MMAL_SUCCESS) {
msg_Err(p_filter, "Failed to commit format for input port %s (status=%"PRIx32" %s)",
sys->input->name, status, mmal_status_to_string(status));
goto fail;
}
sys->input->buffer_size = sys->input->buffer_size_recommended;
sys->input->buffer_num = NUM_DECODER_BUFFER_HEADERS;

if ((status = conv_enable_in(p_filter, sys)) != MMAL_SUCCESS)
goto fail;

port_parameter_set_bool(sys->output, MMAL_PARAMETER_ZERO_COPY, sys->is_sliced);

status = mmal_component_enable(sys->component);
if (status != MMAL_SUCCESS) {
msg_Err(p_filter, "Failed to enable component %s (status=%"PRIx32" %s)",
sys->component->name, status, mmal_status_to_string(status));
goto fail;
}

if ((sys->in_pool = mmal_pool_create(sys->input->buffer_num, 0)) == NULL)
{
msg_Err(p_filter, "Failed to create input pool");
goto fail;
}

if (sys->resizer_type == FILTER_RESIZER_HVS)
{
unsigned int i;
for (i = 0; i != SUBS_MAX; ++i) {
if (hw_mmal_subpic_open(VLC_OBJECT(p_filter), sys->subs + i, sys->component->input[i + 1], -1, i + 1) != MMAL_SUCCESS)
{
msg_Err(p_filter, "Failed to open subpic %d", i);
goto fail;
}
}
}

p_filter->pf_video_filter = conv_filter;
p_filter->pf_flush = conv_flush;


return VLC_SUCCESS;

fail:
CloseConverter(obj);

if (resizer_type != FILTER_RESIZER_RESIZER && status == MMAL_ENOMEM) {
resizer_type = FILTER_RESIZER_RESIZER;
msg_Warn(p_filter, "Lack of memory to use HVS/ISP: trying resizer");
goto retry;
}

return ret;
}

