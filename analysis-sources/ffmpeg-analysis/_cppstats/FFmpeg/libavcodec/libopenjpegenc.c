#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/opt.h"
#include "avcodec.h"
#include "internal.h"
#include <openjpeg.h>
typedef struct LibOpenJPEGContext {
AVClass *avclass;
opj_cparameters_t enc_params;
int format;
int profile;
int prog_order;
int cinema_mode;
int numresolution;
int irreversible;
int disto_alloc;
int fixed_quality;
} LibOpenJPEGContext;
static void error_callback(const char *msg, void *data)
{
av_log(data, AV_LOG_ERROR, "%s\n", msg);
}
static void warning_callback(const char *msg, void *data)
{
av_log(data, AV_LOG_WARNING, "%s\n", msg);
}
static void info_callback(const char *msg, void *data)
{
av_log(data, AV_LOG_DEBUG, "%s\n", msg);
}
typedef struct PacketWriter {
int pos;
AVPacket *packet;
} PacketWriter;
static OPJ_SIZE_T stream_write(void *out_buffer, OPJ_SIZE_T nb_bytes, void *user_data)
{
PacketWriter *writer = user_data;
AVPacket *packet = writer->packet;
int remaining = packet->size - writer->pos;
if (nb_bytes > remaining) {
OPJ_SIZE_T needed = nb_bytes - remaining;
int max_growth = INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE - packet->size;
if (needed > max_growth) {
return (OPJ_SIZE_T)-1;
}
if (av_grow_packet(packet, (int)needed)) {
return (OPJ_SIZE_T)-1;
}
}
memcpy(packet->data + writer->pos, out_buffer, nb_bytes);
writer->pos += (int)nb_bytes;
return nb_bytes;
}
static OPJ_OFF_T stream_skip(OPJ_OFF_T nb_bytes, void *user_data)
{
PacketWriter *writer = user_data;
AVPacket *packet = writer->packet;
if (nb_bytes < 0) {
if (writer->pos == 0) {
return (OPJ_SIZE_T)-1;
}
if (nb_bytes + writer->pos < 0) {
nb_bytes = -writer->pos;
}
} else {
int remaining = packet->size - writer->pos;
if (nb_bytes > remaining) {
OPJ_SIZE_T needed = nb_bytes - remaining;
int max_growth = INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE - packet->size;
if (needed > max_growth) {
return (OPJ_SIZE_T)-1;
}
if (av_grow_packet(packet, (int)needed)) {
return (OPJ_SIZE_T)-1;
}
}
}
writer->pos += (int)nb_bytes;
return nb_bytes;
}
static OPJ_BOOL stream_seek(OPJ_OFF_T nb_bytes, void *user_data)
{
PacketWriter *writer = user_data;
AVPacket *packet = writer->packet;
if (nb_bytes < 0) {
return OPJ_FALSE;
}
if (nb_bytes > packet->size) {
if (nb_bytes > INT_MAX - AV_INPUT_BUFFER_PADDING_SIZE ||
av_grow_packet(packet, (int)nb_bytes - packet->size)) {
return OPJ_FALSE;
}
}
writer->pos = (int)nb_bytes;
return OPJ_TRUE;
}
static void cinema_parameters(opj_cparameters_t *p)
{
p->tile_size_on = 0;
p->cp_tdx = 1;
p->cp_tdy = 1;
p->tp_flag = 'C';
p->tp_on = 1;
p->cp_tx0 = 0;
p->cp_ty0 = 0;
p->image_offset_x0 = 0;
p->image_offset_y0 = 0;
p->cblockw_init = 32;
p->cblockh_init = 32;
p->csty |= 0x01;
p->prog_order = OPJ_CPRL;
p->roi_compno = -1;
p->subsampling_dx = 1;
p->subsampling_dy = 1;
p->irreversible = 1;
p->tcp_mct = 1;
}
static opj_image_t *mj2_create_image(AVCodecContext *avctx, opj_cparameters_t *parameters)
{
const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(avctx->pix_fmt);
opj_image_cmptparm_t cmptparm[4] = {{0}};
opj_image_t *img;
int i;
int sub_dx[4];
int sub_dy[4];
int numcomps;
OPJ_COLOR_SPACE color_space = OPJ_CLRSPC_UNKNOWN;
sub_dx[0] = sub_dx[3] = 1;
sub_dy[0] = sub_dy[3] = 1;
sub_dx[1] = sub_dx[2] = 1 << desc->log2_chroma_w;
sub_dy[1] = sub_dy[2] = 1 << desc->log2_chroma_h;
numcomps = desc->nb_components;
switch (avctx->pix_fmt) {
case AV_PIX_FMT_GRAY8:
case AV_PIX_FMT_YA8:
case AV_PIX_FMT_GRAY10:
case AV_PIX_FMT_GRAY12:
case AV_PIX_FMT_GRAY14:
case AV_PIX_FMT_GRAY16:
case AV_PIX_FMT_YA16:
color_space = OPJ_CLRSPC_GRAY;
break;
case AV_PIX_FMT_RGB24:
case AV_PIX_FMT_RGBA:
case AV_PIX_FMT_RGB48:
case AV_PIX_FMT_RGBA64:
case AV_PIX_FMT_GBR24P:
case AV_PIX_FMT_GBRP9:
case AV_PIX_FMT_GBRP10:
case AV_PIX_FMT_GBRP12:
case AV_PIX_FMT_GBRP14:
case AV_PIX_FMT_GBRP16:
case AV_PIX_FMT_XYZ12:
color_space = OPJ_CLRSPC_SRGB;
break;
case AV_PIX_FMT_YUV410P:
case AV_PIX_FMT_YUV411P:
case AV_PIX_FMT_YUV420P:
case AV_PIX_FMT_YUV422P:
case AV_PIX_FMT_YUV440P:
case AV_PIX_FMT_YUV444P:
case AV_PIX_FMT_YUVA420P:
case AV_PIX_FMT_YUVA422P:
case AV_PIX_FMT_YUVA444P:
case AV_PIX_FMT_YUV420P9:
case AV_PIX_FMT_YUV422P9:
case AV_PIX_FMT_YUV444P9:
case AV_PIX_FMT_YUVA420P9:
case AV_PIX_FMT_YUVA422P9:
case AV_PIX_FMT_YUVA444P9:
case AV_PIX_FMT_YUV420P10:
case AV_PIX_FMT_YUV422P10:
case AV_PIX_FMT_YUV444P10:
case AV_PIX_FMT_YUVA420P10:
case AV_PIX_FMT_YUVA422P10:
case AV_PIX_FMT_YUVA444P10:
case AV_PIX_FMT_YUV420P12:
case AV_PIX_FMT_YUV422P12:
case AV_PIX_FMT_YUV444P12:
case AV_PIX_FMT_YUV420P14:
case AV_PIX_FMT_YUV422P14:
case AV_PIX_FMT_YUV444P14:
case AV_PIX_FMT_YUV420P16:
case AV_PIX_FMT_YUV422P16:
case AV_PIX_FMT_YUV444P16:
case AV_PIX_FMT_YUVA420P16:
case AV_PIX_FMT_YUVA422P16:
case AV_PIX_FMT_YUVA444P16:
color_space = OPJ_CLRSPC_SYCC;
break;
default:
av_log(avctx, AV_LOG_ERROR,
"The requested pixel format '%s' is not supported\n",
av_get_pix_fmt_name(avctx->pix_fmt));
return NULL;
}
for (i = 0; i < numcomps; i++) {
cmptparm[i].prec = desc->comp[i].depth;
cmptparm[i].bpp = desc->comp[i].depth;
cmptparm[i].sgnd = 0;
cmptparm[i].dx = sub_dx[i];
cmptparm[i].dy = sub_dy[i];
cmptparm[i].w = (avctx->width + sub_dx[i] - 1) / sub_dx[i];
cmptparm[i].h = (avctx->height + sub_dy[i] - 1) / sub_dy[i];
}
img = opj_image_create(numcomps, cmptparm, color_space);
if (!img)
return NULL;
img->x0 = 0;
img->y0 = 0;
img->x1 = (avctx->width - 1) * parameters->subsampling_dx + 1;
img->y1 = (avctx->height - 1) * parameters->subsampling_dy + 1;
return img;
}
static av_cold int libopenjpeg_encode_init(AVCodecContext *avctx)
{
LibOpenJPEGContext *ctx = avctx->priv_data;
int err = 0;
opj_set_default_encoder_parameters(&ctx->enc_params);
switch (ctx->cinema_mode) {
case OPJ_CINEMA2K_24:
ctx->enc_params.rsiz = OPJ_PROFILE_CINEMA_2K;
ctx->enc_params.max_cs_size = OPJ_CINEMA_24_CS;
ctx->enc_params.max_comp_size = OPJ_CINEMA_24_COMP;
break;
case OPJ_CINEMA2K_48:
ctx->enc_params.rsiz = OPJ_PROFILE_CINEMA_2K;
ctx->enc_params.max_cs_size = OPJ_CINEMA_48_CS;
ctx->enc_params.max_comp_size = OPJ_CINEMA_48_COMP;
break;
case OPJ_CINEMA4K_24:
ctx->enc_params.rsiz = OPJ_PROFILE_CINEMA_4K;
ctx->enc_params.max_cs_size = OPJ_CINEMA_24_CS;
ctx->enc_params.max_comp_size = OPJ_CINEMA_24_COMP;
break;
}
switch (ctx->profile) {
case OPJ_CINEMA2K:
if (ctx->enc_params.rsiz == OPJ_PROFILE_CINEMA_4K) {
err = AVERROR(EINVAL);
break;
}
ctx->enc_params.rsiz = OPJ_PROFILE_CINEMA_2K;
break;
case OPJ_CINEMA4K:
if (ctx->enc_params.rsiz == OPJ_PROFILE_CINEMA_2K) {
err = AVERROR(EINVAL);
break;
}
ctx->enc_params.rsiz = OPJ_PROFILE_CINEMA_4K;
break;
}
if (err) {
av_log(avctx, AV_LOG_ERROR,
"Invalid parameter pairing: cinema_mode and profile conflict.\n");
return err;
}
if (!ctx->numresolution) {
ctx->numresolution = 6;
while (FFMIN(avctx->width, avctx->height) >> ctx->numresolution < 1)
ctx->numresolution --;
}
ctx->enc_params.prog_order = ctx->prog_order;
ctx->enc_params.numresolution = ctx->numresolution;
ctx->enc_params.irreversible = ctx->irreversible;
ctx->enc_params.cp_disto_alloc = ctx->disto_alloc;
ctx->enc_params.cp_fixed_quality = ctx->fixed_quality;
ctx->enc_params.tcp_numlayers = 1;
ctx->enc_params.tcp_rates[0] = FFMAX(avctx->compression_level, 0) * 2;
if (ctx->cinema_mode > 0) {
cinema_parameters(&ctx->enc_params);
}
return 0;
}
static int libopenjpeg_copy_packed8(AVCodecContext *avctx, const AVFrame *frame, opj_image_t *image)
{
int compno;
int x;
int y;
int *image_line;
int frame_index;
const int numcomps = image->numcomps;
for (compno = 0; compno < numcomps; ++compno) {
if (image->comps[compno].w > frame->linesize[0] / numcomps) {
av_log(avctx, AV_LOG_ERROR, "Error: frame's linesize is too small for the image\n");
return 0;
}
}
for (compno = 0; compno < numcomps; ++compno) {
for (y = 0; y < avctx->height; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
frame_index = y * frame->linesize[0] + compno;
for (x = 0; x < avctx->width; ++x) {
image_line[x] = frame->data[0][frame_index];
frame_index += numcomps;
}
for (; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - 1];
}
}
for (; y < image->comps[compno].h; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
for (x = 0; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - (int)image->comps[compno].w];
}
}
}
return 1;
}
static int libopenjpeg_copy_packed12(AVCodecContext *avctx, const AVFrame *frame, opj_image_t *image)
{
int compno;
int x, y;
int *image_line;
int frame_index;
const int numcomps = image->numcomps;
uint16_t *frame_ptr = (uint16_t *)frame->data[0];
for (compno = 0; compno < numcomps; ++compno) {
if (image->comps[compno].w > frame->linesize[0] / numcomps) {
av_log(avctx, AV_LOG_ERROR, "Error: frame's linesize is too small for the image\n");
return 0;
}
}
for (compno = 0; compno < numcomps; ++compno) {
for (y = 0; y < avctx->height; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
frame_index = y * (frame->linesize[0] / 2) + compno;
for (x = 0; x < avctx->width; ++x) {
image_line[x] = frame_ptr[frame_index] >> 4;
frame_index += numcomps;
}
for (; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - 1];
}
}
for (; y < image->comps[compno].h; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
for (x = 0; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - (int)image->comps[compno].w];
}
}
}
return 1;
}
static int libopenjpeg_copy_packed16(AVCodecContext *avctx, const AVFrame *frame, opj_image_t *image)
{
int compno;
int x;
int y;
int *image_line;
int frame_index;
const int numcomps = image->numcomps;
uint16_t *frame_ptr = (uint16_t*)frame->data[0];
for (compno = 0; compno < numcomps; ++compno) {
if (image->comps[compno].w > frame->linesize[0] / numcomps) {
av_log(avctx, AV_LOG_ERROR, "Error: frame's linesize is too small for the image\n");
return 0;
}
}
for (compno = 0; compno < numcomps; ++compno) {
for (y = 0; y < avctx->height; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
frame_index = y * (frame->linesize[0] / 2) + compno;
for (x = 0; x < avctx->width; ++x) {
image_line[x] = frame_ptr[frame_index];
frame_index += numcomps;
}
for (; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - 1];
}
}
for (; y < image->comps[compno].h; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
for (x = 0; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - (int)image->comps[compno].w];
}
}
}
return 1;
}
static int libopenjpeg_copy_unpacked8(AVCodecContext *avctx, const AVFrame *frame, opj_image_t *image)
{
int compno;
int x;
int y;
int width;
int height;
int *image_line;
int frame_index;
const int numcomps = image->numcomps;
for (compno = 0; compno < numcomps; ++compno) {
if (image->comps[compno].w > frame->linesize[compno]) {
av_log(avctx, AV_LOG_ERROR, "Error: frame's linesize is too small for the image\n");
return 0;
}
}
for (compno = 0; compno < numcomps; ++compno) {
width = (avctx->width + image->comps[compno].dx - 1) / image->comps[compno].dx;
height = (avctx->height + image->comps[compno].dy - 1) / image->comps[compno].dy;
for (y = 0; y < height; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
frame_index = y * frame->linesize[compno];
for (x = 0; x < width; ++x)
image_line[x] = frame->data[compno][frame_index++];
for (; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - 1];
}
}
for (; y < image->comps[compno].h; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
for (x = 0; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - (int)image->comps[compno].w];
}
}
}
return 1;
}
static int libopenjpeg_copy_unpacked16(AVCodecContext *avctx, const AVFrame *frame, opj_image_t *image)
{
int compno;
int x;
int y;
int width;
int height;
int *image_line;
int frame_index;
const int numcomps = image->numcomps;
uint16_t *frame_ptr;
for (compno = 0; compno < numcomps; ++compno) {
if (image->comps[compno].w > frame->linesize[compno]) {
av_log(avctx, AV_LOG_ERROR, "Error: frame's linesize is too small for the image\n");
return 0;
}
}
for (compno = 0; compno < numcomps; ++compno) {
width = (avctx->width + image->comps[compno].dx - 1) / image->comps[compno].dx;
height = (avctx->height + image->comps[compno].dy - 1) / image->comps[compno].dy;
frame_ptr = (uint16_t *)frame->data[compno];
for (y = 0; y < height; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
frame_index = y * (frame->linesize[compno] / 2);
for (x = 0; x < width; ++x)
image_line[x] = frame_ptr[frame_index++];
for (; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - 1];
}
}
for (; y < image->comps[compno].h; ++y) {
image_line = image->comps[compno].data + y * image->comps[compno].w;
for (x = 0; x < image->comps[compno].w; ++x) {
image_line[x] = image_line[x - (int)image->comps[compno].w];
}
}
}
return 1;
}
static int libopenjpeg_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
const AVFrame *frame, int *got_packet)
{
LibOpenJPEGContext *ctx = avctx->priv_data;
int ret;
AVFrame *gbrframe;
int cpyresult = 0;
PacketWriter writer = { 0 };
opj_codec_t *compress = NULL;
opj_stream_t *stream = NULL;
opj_image_t *image = mj2_create_image(avctx, &ctx->enc_params);
if (!image) {
av_log(avctx, AV_LOG_ERROR, "Error creating the mj2 image\n");
ret = AVERROR(EINVAL);
goto done;
}
switch (avctx->pix_fmt) {
case AV_PIX_FMT_RGB24:
case AV_PIX_FMT_RGBA:
case AV_PIX_FMT_YA8:
cpyresult = libopenjpeg_copy_packed8(avctx, frame, image);
break;
case AV_PIX_FMT_XYZ12:
cpyresult = libopenjpeg_copy_packed12(avctx, frame, image);
break;
case AV_PIX_FMT_RGB48:
case AV_PIX_FMT_RGBA64:
case AV_PIX_FMT_YA16:
cpyresult = libopenjpeg_copy_packed16(avctx, frame, image);
break;
case AV_PIX_FMT_GBR24P:
case AV_PIX_FMT_GBRP9:
case AV_PIX_FMT_GBRP10:
case AV_PIX_FMT_GBRP12:
case AV_PIX_FMT_GBRP14:
case AV_PIX_FMT_GBRP16:
gbrframe = av_frame_clone(frame);
if (!gbrframe) {
ret = AVERROR(ENOMEM);
goto done;
}
gbrframe->data[0] = frame->data[2]; 
gbrframe->data[1] = frame->data[0];
gbrframe->data[2] = frame->data[1];
gbrframe->linesize[0] = frame->linesize[2];
gbrframe->linesize[1] = frame->linesize[0];
gbrframe->linesize[2] = frame->linesize[1];
if (avctx->pix_fmt == AV_PIX_FMT_GBR24P) {
cpyresult = libopenjpeg_copy_unpacked8(avctx, gbrframe, image);
} else {
cpyresult = libopenjpeg_copy_unpacked16(avctx, gbrframe, image);
}
av_frame_free(&gbrframe);
break;
case AV_PIX_FMT_GRAY8:
case AV_PIX_FMT_YUV410P:
case AV_PIX_FMT_YUV411P:
case AV_PIX_FMT_YUV420P:
case AV_PIX_FMT_YUV422P:
case AV_PIX_FMT_YUV440P:
case AV_PIX_FMT_YUV444P:
case AV_PIX_FMT_YUVA420P:
case AV_PIX_FMT_YUVA422P:
case AV_PIX_FMT_YUVA444P:
cpyresult = libopenjpeg_copy_unpacked8(avctx, frame, image);
break;
case AV_PIX_FMT_GRAY10:
case AV_PIX_FMT_GRAY12:
case AV_PIX_FMT_GRAY14:
case AV_PIX_FMT_GRAY16:
case AV_PIX_FMT_YUV420P9:
case AV_PIX_FMT_YUV422P9:
case AV_PIX_FMT_YUV444P9:
case AV_PIX_FMT_YUVA420P9:
case AV_PIX_FMT_YUVA422P9:
case AV_PIX_FMT_YUVA444P9:
case AV_PIX_FMT_YUV444P10:
case AV_PIX_FMT_YUV422P10:
case AV_PIX_FMT_YUV420P10:
case AV_PIX_FMT_YUVA444P10:
case AV_PIX_FMT_YUVA422P10:
case AV_PIX_FMT_YUVA420P10:
case AV_PIX_FMT_YUV420P12:
case AV_PIX_FMT_YUV422P12:
case AV_PIX_FMT_YUV444P12:
case AV_PIX_FMT_YUV420P14:
case AV_PIX_FMT_YUV422P14:
case AV_PIX_FMT_YUV444P14:
case AV_PIX_FMT_YUV444P16:
case AV_PIX_FMT_YUV422P16:
case AV_PIX_FMT_YUV420P16:
case AV_PIX_FMT_YUVA444P16:
case AV_PIX_FMT_YUVA422P16:
case AV_PIX_FMT_YUVA420P16:
cpyresult = libopenjpeg_copy_unpacked16(avctx, frame, image);
break;
default:
av_log(avctx, AV_LOG_ERROR,
"The frame's pixel format '%s' is not supported\n",
av_get_pix_fmt_name(avctx->pix_fmt));
ret = AVERROR(EINVAL);
goto done;
break;
}
if (!cpyresult) {
av_log(avctx, AV_LOG_ERROR,
"Could not copy the frame data to the internal image buffer\n");
ret = -1;
goto done;
}
if ((ret = ff_alloc_packet2(avctx, pkt, 1024, 0)) < 0) {
goto done;
}
compress = opj_create_compress(ctx->format);
if (!compress) {
av_log(avctx, AV_LOG_ERROR, "Error creating the compressor\n");
ret = AVERROR(ENOMEM);
goto done;
}
if (!opj_set_error_handler(compress, error_callback, avctx) ||
!opj_set_warning_handler(compress, warning_callback, avctx) ||
!opj_set_info_handler(compress, info_callback, avctx)) {
av_log(avctx, AV_LOG_ERROR, "Error setting the compressor handlers\n");
ret = AVERROR_EXTERNAL;
goto done;
}
if (!opj_setup_encoder(compress, &ctx->enc_params, image)) {
av_log(avctx, AV_LOG_ERROR, "Error setting up the compressor\n");
ret = AVERROR_EXTERNAL;
goto done;
}
stream = opj_stream_default_create(OPJ_STREAM_WRITE);
if (!stream) {
av_log(avctx, AV_LOG_ERROR, "Error creating the cio stream\n");
ret = AVERROR(ENOMEM);
goto done;
}
writer.packet = pkt;
opj_stream_set_write_function(stream, stream_write);
opj_stream_set_skip_function(stream, stream_skip);
opj_stream_set_seek_function(stream, stream_seek);
opj_stream_set_user_data(stream, &writer, NULL);
if (!opj_start_compress(compress, image, stream) ||
!opj_encode(compress, stream) ||
!opj_end_compress(compress, stream)) {
av_log(avctx, AV_LOG_ERROR, "Error during the opj encode\n");
ret = AVERROR_EXTERNAL;
goto done;
}
av_shrink_packet(pkt, writer.pos);
pkt->flags |= AV_PKT_FLAG_KEY;
*got_packet = 1;
ret = 0;
done:
opj_stream_destroy(stream);
opj_destroy_codec(compress);
opj_image_destroy(image);
return ret;
}
#define OFFSET(x) offsetof(LibOpenJPEGContext, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
{ "format", "Codec Format", OFFSET(format), AV_OPT_TYPE_INT, { .i64 = OPJ_CODEC_JP2 }, OPJ_CODEC_J2K, OPJ_CODEC_JP2, VE, "format" },
{ "j2k", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CODEC_J2K }, 0, 0, VE, "format" },
{ "jp2", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CODEC_JP2 }, 0, 0, VE, "format" },
{ "profile", NULL, OFFSET(profile), AV_OPT_TYPE_INT, { .i64 = OPJ_STD_RSIZ }, OPJ_STD_RSIZ, OPJ_CINEMA4K, VE, "profile" },
{ "jpeg2000", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_STD_RSIZ }, 0, 0, VE, "profile" },
{ "cinema2k", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CINEMA2K }, 0, 0, VE, "profile" },
{ "cinema4k", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CINEMA4K }, 0, 0, VE, "profile" },
{ "cinema_mode", "Digital Cinema", OFFSET(cinema_mode), AV_OPT_TYPE_INT, { .i64 = OPJ_OFF }, OPJ_OFF, OPJ_CINEMA4K_24, VE, "cinema_mode" },
{ "off", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_OFF }, 0, 0, VE, "cinema_mode" },
{ "2k_24", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CINEMA2K_24 }, 0, 0, VE, "cinema_mode" },
{ "2k_48", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CINEMA2K_48 }, 0, 0, VE, "cinema_mode" },
{ "4k_24", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CINEMA4K_24 }, 0, 0, VE, "cinema_mode" },
{ "prog_order", "Progression Order", OFFSET(prog_order), AV_OPT_TYPE_INT, { .i64 = OPJ_LRCP }, OPJ_LRCP, OPJ_CPRL, VE, "prog_order" },
{ "lrcp", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_LRCP }, 0, 0, VE, "prog_order" },
{ "rlcp", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_RLCP }, 0, 0, VE, "prog_order" },
{ "rpcl", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_RPCL }, 0, 0, VE, "prog_order" },
{ "pcrl", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_PCRL }, 0, 0, VE, "prog_order" },
{ "cprl", NULL, 0, AV_OPT_TYPE_CONST, { .i64 = OPJ_CPRL }, 0, 0, VE, "prog_order" },
{ "numresolution", NULL, OFFSET(numresolution), AV_OPT_TYPE_INT, { .i64 = 6 }, 0, 33, VE },
{ "irreversible", NULL, OFFSET(irreversible), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, VE },
{ "disto_alloc", NULL, OFFSET(disto_alloc), AV_OPT_TYPE_INT, { .i64 = 1 }, 0, 1, VE },
{ "fixed_quality", NULL, OFFSET(fixed_quality), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, VE },
{ NULL },
};
static const AVClass openjpeg_class = {
.class_name = "libopenjpeg",
.item_name = av_default_item_name,
.option = options,
.version = LIBAVUTIL_VERSION_INT,
};
AVCodec ff_libopenjpeg_encoder = {
.name = "libopenjpeg",
.long_name = NULL_IF_CONFIG_SMALL("OpenJPEG JPEG 2000"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_JPEG2000,
.priv_data_size = sizeof(LibOpenJPEGContext),
.init = libopenjpeg_encode_init,
.encode2 = libopenjpeg_encode_frame,
.capabilities = AV_CODEC_CAP_FRAME_THREADS | AV_CODEC_CAP_INTRA_ONLY,
.pix_fmts = (const enum AVPixelFormat[]) {
AV_PIX_FMT_RGB24, AV_PIX_FMT_RGBA, AV_PIX_FMT_RGB48,
AV_PIX_FMT_RGBA64, AV_PIX_FMT_GBR24P,
AV_PIX_FMT_GBRP9, AV_PIX_FMT_GBRP10, AV_PIX_FMT_GBRP12, AV_PIX_FMT_GBRP14, AV_PIX_FMT_GBRP16,
AV_PIX_FMT_GRAY8, AV_PIX_FMT_YA8, AV_PIX_FMT_GRAY16, AV_PIX_FMT_YA16,
AV_PIX_FMT_GRAY10, AV_PIX_FMT_GRAY12, AV_PIX_FMT_GRAY14,
AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUV422P, AV_PIX_FMT_YUVA420P,
AV_PIX_FMT_YUV440P, AV_PIX_FMT_YUV444P, AV_PIX_FMT_YUVA422P,
AV_PIX_FMT_YUV411P, AV_PIX_FMT_YUV410P, AV_PIX_FMT_YUVA444P,
AV_PIX_FMT_YUV420P9, AV_PIX_FMT_YUV422P9, AV_PIX_FMT_YUV444P9,
AV_PIX_FMT_YUVA420P9, AV_PIX_FMT_YUVA422P9, AV_PIX_FMT_YUVA444P9,
AV_PIX_FMT_YUV420P10, AV_PIX_FMT_YUV422P10, AV_PIX_FMT_YUV444P10,
AV_PIX_FMT_YUVA420P10, AV_PIX_FMT_YUVA422P10, AV_PIX_FMT_YUVA444P10,
AV_PIX_FMT_YUV420P12, AV_PIX_FMT_YUV422P12, AV_PIX_FMT_YUV444P12,
AV_PIX_FMT_YUV420P14, AV_PIX_FMT_YUV422P14, AV_PIX_FMT_YUV444P14,
AV_PIX_FMT_YUV420P16, AV_PIX_FMT_YUV422P16, AV_PIX_FMT_YUV444P16,
AV_PIX_FMT_YUVA420P16, AV_PIX_FMT_YUVA422P16, AV_PIX_FMT_YUVA444P16,
AV_PIX_FMT_XYZ12,
AV_PIX_FMT_NONE
},
.priv_class = &openjpeg_class,
.wrapper_name = "libopenjpeg",
};
