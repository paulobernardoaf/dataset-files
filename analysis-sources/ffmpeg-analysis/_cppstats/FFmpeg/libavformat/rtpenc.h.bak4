



















#if !defined(AVFORMAT_RTPENC_H)
#define AVFORMAT_RTPENC_H

#include "avformat.h"
#include "rtp.h"

struct RTPMuxContext {
const AVClass *av_class;
AVFormatContext *ic;
AVStream *st;
int payload_type;
uint32_t ssrc;
const char *cname;
int seq;
uint32_t timestamp;
uint32_t base_timestamp;
uint32_t cur_timestamp;
int max_payload_size;
int num_frames;


int64_t last_rtcp_ntp_time;
int64_t first_rtcp_ntp_time;
unsigned int packet_count;
unsigned int octet_count;
unsigned int last_octet_count;
int first_packet;

uint8_t *buf;
uint8_t *buf_ptr;

int max_frames_per_packet;





int nal_length_size;
int buffered_nals;

int flags;

unsigned int frame_count;
};

typedef struct RTPMuxContext RTPMuxContext;

#define FF_RTP_FLAG_MP4A_LATM 1
#define FF_RTP_FLAG_RFC2190 2
#define FF_RTP_FLAG_SKIP_RTCP 4
#define FF_RTP_FLAG_H264_MODE0 8
#define FF_RTP_FLAG_SEND_BYE 16

#define FF_RTP_FLAG_OPTS(ctx, fieldname) { "rtpflags", "RTP muxer flags", offsetof(ctx, fieldname), AV_OPT_TYPE_FLAGS, {.i64 = 0}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM, "rtpflags" }, { "latm", "Use MP4A-LATM packetization instead of MPEG4-GENERIC for AAC", 0, AV_OPT_TYPE_CONST, {.i64 = FF_RTP_FLAG_MP4A_LATM}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM, "rtpflags" }, { "rfc2190", "Use RFC 2190 packetization instead of RFC 4629 for H.263", 0, AV_OPT_TYPE_CONST, {.i64 = FF_RTP_FLAG_RFC2190}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM, "rtpflags" }, { "skip_rtcp", "Don't send RTCP sender reports", 0, AV_OPT_TYPE_CONST, {.i64 = FF_RTP_FLAG_SKIP_RTCP}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM, "rtpflags" }, { "h264_mode0", "Use mode 0 for H.264 in RTP", 0, AV_OPT_TYPE_CONST, {.i64 = FF_RTP_FLAG_H264_MODE0}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM, "rtpflags" }, { "send_bye", "Send RTCP BYE packets when finishing", 0, AV_OPT_TYPE_CONST, {.i64 = FF_RTP_FLAG_SEND_BYE}, INT_MIN, INT_MAX, AV_OPT_FLAG_ENCODING_PARAM, "rtpflags" } 







void ff_rtp_send_data(AVFormatContext *s1, const uint8_t *buf1, int len, int m);

void ff_rtp_send_h264_hevc(AVFormatContext *s1, const uint8_t *buf1, int size);
void ff_rtp_send_h261(AVFormatContext *s1, const uint8_t *buf1, int size);
void ff_rtp_send_h263(AVFormatContext *s1, const uint8_t *buf1, int size);
void ff_rtp_send_h263_rfc2190(AVFormatContext *s1, const uint8_t *buf1, int size,
const uint8_t *mb_info, int mb_info_size);
void ff_rtp_send_aac(AVFormatContext *s1, const uint8_t *buff, int size);
void ff_rtp_send_latm(AVFormatContext *s1, const uint8_t *buff, int size);
void ff_rtp_send_amr(AVFormatContext *s1, const uint8_t *buff, int size);
void ff_rtp_send_mpegvideo(AVFormatContext *s1, const uint8_t *buf1, int size);
void ff_rtp_send_xiph(AVFormatContext *s1, const uint8_t *buff, int size);
void ff_rtp_send_vc2hq(AVFormatContext *s1, const uint8_t *buf, int size, int interlaced);
void ff_rtp_send_vp8(AVFormatContext *s1, const uint8_t *buff, int size);
void ff_rtp_send_vp9(AVFormatContext *s1, const uint8_t *buff, int size);
void ff_rtp_send_jpeg(AVFormatContext *s1, const uint8_t *buff, int size);

const uint8_t *ff_h263_find_resync_marker_reverse(const uint8_t *av_restrict start,
const uint8_t *av_restrict end);

#endif 
