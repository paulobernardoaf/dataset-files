



















#include "libavutil/imgutils.h"
#include "lavfutils.h"

int ff_load_image(uint8_t *data[4], int linesize[4],
int *w, int *h, enum AVPixelFormat *pix_fmt,
const char *filename, void *log_ctx)
{
AVInputFormat *iformat = NULL;
AVFormatContext *format_ctx = NULL;
AVCodec *codec;
AVCodecContext *codec_ctx;
AVCodecParameters *par;
AVFrame *frame;
int frame_decoded, ret = 0;
AVPacket pkt;
AVDictionary *opt=NULL;

av_init_packet(&pkt);

iformat = av_find_input_format("image2pipe");
if ((ret = avformat_open_input(&format_ctx, filename, iformat, NULL)) < 0) {
av_log(log_ctx, AV_LOG_ERROR,
"Failed to open input file '%s'\n", filename);
return ret;
}

if ((ret = avformat_find_stream_info(format_ctx, NULL)) < 0) {
av_log(log_ctx, AV_LOG_ERROR, "Find stream info failed\n");
return ret;
}

par = format_ctx->streams[0]->codecpar;
codec = avcodec_find_decoder(par->codec_id);
if (!codec) {
av_log(log_ctx, AV_LOG_ERROR, "Failed to find codec\n");
ret = AVERROR(EINVAL);
goto end;
}

codec_ctx = avcodec_alloc_context3(codec);
if (!codec_ctx) {
av_log(log_ctx, AV_LOG_ERROR, "Failed to alloc video decoder context\n");
ret = AVERROR(ENOMEM);
goto end;
}

ret = avcodec_parameters_to_context(codec_ctx, par);
if (ret < 0) {
av_log(log_ctx, AV_LOG_ERROR, "Failed to copy codec parameters to decoder context\n");
goto end;
}

av_dict_set(&opt, "thread_type", "slice", 0);
if ((ret = avcodec_open2(codec_ctx, codec, &opt)) < 0) {
av_log(log_ctx, AV_LOG_ERROR, "Failed to open codec\n");
goto end;
}

if (!(frame = av_frame_alloc()) ) {
av_log(log_ctx, AV_LOG_ERROR, "Failed to alloc frame\n");
ret = AVERROR(ENOMEM);
goto end;
}

ret = av_read_frame(format_ctx, &pkt);
if (ret < 0) {
av_log(log_ctx, AV_LOG_ERROR, "Failed to read frame from file\n");
goto end;
}

ret = avcodec_decode_video2(codec_ctx, frame, &frame_decoded, &pkt);
if (ret < 0 || !frame_decoded) {
av_log(log_ctx, AV_LOG_ERROR, "Failed to decode image from file\n");
if (ret >= 0)
ret = -1;
goto end;
}

*w = frame->width;
*h = frame->height;
*pix_fmt = frame->format;

if ((ret = av_image_alloc(data, linesize, *w, *h, *pix_fmt, 16)) < 0)
goto end;
ret = 0;

av_image_copy(data, linesize, (const uint8_t **)frame->data, frame->linesize, *pix_fmt, *w, *h);

end:
av_packet_unref(&pkt);
avcodec_free_context(&codec_ctx);
avformat_close_input(&format_ctx);
av_frame_free(&frame);
av_dict_free(&opt);

if (ret < 0)
av_log(log_ctx, AV_LOG_ERROR, "Error loading image file '%s'\n", filename);
return ret;
}
