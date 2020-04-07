#include "libavcodec/avcodec.h"
#include "avformat.h"
#include "rtp.h"
#include "url.h"
#include "srtp.h"
typedef struct PayloadContext PayloadContext;
typedef struct RTPDynamicProtocolHandler RTPDynamicProtocolHandler;
#define RTP_MIN_PACKET_LENGTH 12
#define RTP_MAX_PACKET_LENGTH 8192
#define RTP_REORDER_QUEUE_DEFAULT_SIZE 500
#define RTP_NOTS_VALUE ((uint32_t)-1)
typedef struct RTPDemuxContext RTPDemuxContext;
RTPDemuxContext *ff_rtp_parse_open(AVFormatContext *s1, AVStream *st,
int payload_type, int queue_size);
void ff_rtp_parse_set_dynamic_protocol(RTPDemuxContext *s, PayloadContext *ctx,
const RTPDynamicProtocolHandler *handler);
void ff_rtp_parse_set_crypto(RTPDemuxContext *s, const char *suite,
const char *params);
int ff_rtp_parse_packet(RTPDemuxContext *s, AVPacket *pkt,
uint8_t **buf, int len);
void ff_rtp_parse_close(RTPDemuxContext *s);
int64_t ff_rtp_queued_packet_time(RTPDemuxContext *s);
void ff_rtp_reset_packet_queue(RTPDemuxContext *s);
void ff_rtp_send_punch_packets(URLContext* rtp_handle);
int ff_rtp_check_and_send_back_rr(RTPDemuxContext *s, URLContext *fd,
AVIOContext *avio, int count);
int ff_rtp_send_rtcp_feedback(RTPDemuxContext *s, URLContext *fd,
AVIOContext *avio);
typedef struct RTPStatistics {
uint16_t max_seq; 
uint32_t cycles; 
uint32_t base_seq; 
uint32_t bad_seq; 
int probation; 
uint32_t received; 
uint32_t expected_prior; 
uint32_t received_prior; 
uint32_t transit; 
uint32_t jitter; 
} RTPStatistics;
#define RTP_FLAG_KEY 0x1 
#define RTP_FLAG_MARKER 0x2 
typedef int (*DynamicPayloadPacketHandlerProc)(AVFormatContext *ctx,
PayloadContext *s,
AVStream *st, AVPacket *pkt,
uint32_t *timestamp,
const uint8_t * buf,
int len, uint16_t seq, int flags);
struct RTPDynamicProtocolHandler {
const char *enc_name;
enum AVMediaType codec_type;
enum AVCodecID codec_id;
enum AVStreamParseType need_parsing;
int static_payload_id; 
int priv_data_size;
int (*init)(AVFormatContext *s, int st_index, PayloadContext *priv_data);
int (*parse_sdp_a_line)(AVFormatContext *s, int st_index,
PayloadContext *priv_data, const char *line);
void (*close)(PayloadContext *protocol_data);
DynamicPayloadPacketHandlerProc parse_packet;
int (*need_keyframe)(PayloadContext *context);
struct RTPDynamicProtocolHandler *next;
};
typedef struct RTPPacket {
uint16_t seq;
uint8_t *buf;
int len;
int64_t recvtime;
struct RTPPacket *next;
} RTPPacket;
struct RTPDemuxContext {
AVFormatContext *ic;
AVStream *st;
int payload_type;
uint32_t ssrc;
uint16_t seq;
uint32_t timestamp;
uint32_t base_timestamp;
int64_t unwrapped_timestamp;
int64_t range_start_offset;
int max_payload_size;
char hostname[256];
int srtp_enabled;
struct SRTPContext srtp;
RTPStatistics statistics;
int prev_ret; 
RTPPacket* queue; 
int queue_len; 
int queue_size; 
uint64_t last_rtcp_ntp_time;
int64_t last_rtcp_reception_time;
uint64_t first_rtcp_ntp_time;
uint32_t last_rtcp_timestamp;
int64_t rtcp_ts_offset;
unsigned int packet_count;
unsigned int octet_count;
unsigned int last_octet_count;
int64_t last_feedback_time;
const RTPDynamicProtocolHandler *handler;
PayloadContext *dynamic_protocol_context;
};
const RTPDynamicProtocolHandler *ff_rtp_handler_iterate(void **opaque);
const RTPDynamicProtocolHandler *ff_rtp_handler_find_by_name(const char *name,
enum AVMediaType codec_type);
const RTPDynamicProtocolHandler *ff_rtp_handler_find_by_id(int id,
enum AVMediaType codec_type);
int ff_rtsp_next_attr_and_value(const char **p, char *attr, int attr_size,
char *value, int value_size);
int ff_parse_fmtp(AVFormatContext *s,
AVStream *stream, PayloadContext *data, const char *p,
int (*parse_fmtp)(AVFormatContext *s,
AVStream *stream,
PayloadContext *data,
const char *attr, const char *value));
int ff_rtp_finalize_packet(AVPacket *pkt, AVIOContext **dyn_buf, int stream_idx);
