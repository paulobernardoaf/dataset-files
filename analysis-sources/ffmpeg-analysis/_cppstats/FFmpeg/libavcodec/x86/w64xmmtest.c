#include "libavcodec/avcodec.h"
#include "libavutil/x86/w64xmmtest.h"
wrap(avcodec_open2(AVCodecContext *avctx,
const AVCodec *codec,
AVDictionary **options))
{
testxmmclobbers(avcodec_open2, avctx, codec, options);
}
wrap(avcodec_decode_audio4(AVCodecContext *avctx,
AVFrame *frame,
int *got_frame_ptr,
AVPacket *avpkt))
{
testxmmclobbers(avcodec_decode_audio4, avctx, frame,
got_frame_ptr, avpkt);
}
wrap(avcodec_decode_video2(AVCodecContext *avctx,
AVFrame *picture,
int *got_picture_ptr,
AVPacket *avpkt))
{
testxmmclobbers(avcodec_decode_video2, avctx, picture,
got_picture_ptr, avpkt);
}
wrap(avcodec_decode_subtitle2(AVCodecContext *avctx,
AVSubtitle *sub,
int *got_sub_ptr,
AVPacket *avpkt))
{
testxmmclobbers(avcodec_decode_subtitle2, avctx, sub,
got_sub_ptr, avpkt);
}
wrap(avcodec_encode_audio2(AVCodecContext *avctx,
AVPacket *avpkt,
const AVFrame *frame,
int *got_packet_ptr))
{
testxmmclobbers(avcodec_encode_audio2, avctx, avpkt, frame,
got_packet_ptr);
}
wrap(avcodec_encode_subtitle(AVCodecContext *avctx,
uint8_t *buf, int buf_size,
const AVSubtitle *sub))
{
testxmmclobbers(avcodec_encode_subtitle, avctx, buf, buf_size, sub);
}
wrap(avcodec_encode_video2(AVCodecContext *avctx, AVPacket *avpkt,
const AVFrame *frame, int *got_packet_ptr))
{
testxmmclobbers(avcodec_encode_video2, avctx, avpkt, frame, got_packet_ptr);
}
wrap(avcodec_send_packet(AVCodecContext *avctx, const AVPacket *avpkt))
{
testxmmclobbers(avcodec_send_packet, avctx, avpkt);
}
wrap(avcodec_receive_packet(AVCodecContext *avctx, AVPacket *avpkt))
{
testxmmclobbers(avcodec_receive_packet, avctx, avpkt);
}
wrap(avcodec_send_frame(AVCodecContext *avctx, const AVFrame *frame))
{
testxmmclobbers(avcodec_send_frame, avctx, frame);
}
wrap(avcodec_receive_frame(AVCodecContext *avctx, AVFrame *frame))
{
testxmmclobbers(avcodec_receive_frame, avctx, frame);
}
