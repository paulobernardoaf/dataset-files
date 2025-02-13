#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
FILE *outfile)
{
int ret;
if (frame)
printf("Send frame %3"PRId64"\n", frame->pts);
ret = avcodec_send_frame(enc_ctx, frame);
if (ret < 0) {
fprintf(stderr, "Error sending a frame for encoding\n");
exit(1);
}
while (ret >= 0) {
ret = avcodec_receive_packet(enc_ctx, pkt);
if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
return;
else if (ret < 0) {
fprintf(stderr, "Error during encoding\n");
exit(1);
}
printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
fwrite(pkt->data, 1, pkt->size, outfile);
av_packet_unref(pkt);
}
}
int main(int argc, char **argv)
{
const char *filename, *codec_name;
const AVCodec *codec;
AVCodecContext *c= NULL;
int i, ret, x, y;
FILE *f;
AVFrame *frame;
AVPacket *pkt;
uint8_t endcode[] = { 0, 0, 1, 0xb7 };
if (argc <= 2) {
fprintf(stderr, "Usage: %s <output file> <codec name>\n", argv[0]);
exit(0);
}
filename = argv[1];
codec_name = argv[2];
codec = avcodec_find_encoder_by_name(codec_name);
if (!codec) {
fprintf(stderr, "Codec '%s' not found\n", codec_name);
exit(1);
}
c = avcodec_alloc_context3(codec);
if (!c) {
fprintf(stderr, "Could not allocate video codec context\n");
exit(1);
}
pkt = av_packet_alloc();
if (!pkt)
exit(1);
c->bit_rate = 400000;
c->width = 352;
c->height = 288;
c->time_base = (AVRational){1, 25};
c->framerate = (AVRational){25, 1};
c->gop_size = 10;
c->max_b_frames = 1;
c->pix_fmt = AV_PIX_FMT_YUV420P;
if (codec->id == AV_CODEC_ID_H264)
av_opt_set(c->priv_data, "preset", "slow", 0);
ret = avcodec_open2(c, codec, NULL);
if (ret < 0) {
fprintf(stderr, "Could not open codec: %s\n", av_err2str(ret));
exit(1);
}
f = fopen(filename, "wb");
if (!f) {
fprintf(stderr, "Could not open %s\n", filename);
exit(1);
}
frame = av_frame_alloc();
if (!frame) {
fprintf(stderr, "Could not allocate video frame\n");
exit(1);
}
frame->format = c->pix_fmt;
frame->width = c->width;
frame->height = c->height;
ret = av_frame_get_buffer(frame, 32);
if (ret < 0) {
fprintf(stderr, "Could not allocate the video frame data\n");
exit(1);
}
for (i = 0; i < 25; i++) {
fflush(stdout);
ret = av_frame_make_writable(frame);
if (ret < 0)
exit(1);
for (y = 0; y < c->height; y++) {
for (x = 0; x < c->width; x++) {
frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
}
}
for (y = 0; y < c->height/2; y++) {
for (x = 0; x < c->width/2; x++) {
frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
}
}
frame->pts = i;
encode(c, frame, pkt, f);
}
encode(c, NULL, pkt, f);
if (codec->id == AV_CODEC_ID_MPEG1VIDEO || codec->id == AV_CODEC_ID_MPEG2VIDEO)
fwrite(endcode, 1, sizeof(endcode), f);
fclose(f);
avcodec_free_context(&c);
av_frame_free(&frame);
av_packet_free(&pkt);
return 0;
}
