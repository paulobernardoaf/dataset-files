#include <stdint.h>
#include "avformat.h"
#include "rtspcodes.h"
#include "rtpdec.h"
#include "network.h"
#include "httpauth.h"
#include "libavutil/log.h"
#include "libavutil/opt.h"
enum RTSPLowerTransport {
RTSP_LOWER_TRANSPORT_UDP = 0, 
RTSP_LOWER_TRANSPORT_TCP = 1, 
RTSP_LOWER_TRANSPORT_UDP_MULTICAST = 2, 
RTSP_LOWER_TRANSPORT_NB,
RTSP_LOWER_TRANSPORT_HTTP = 8, 
RTSP_LOWER_TRANSPORT_HTTPS, 
RTSP_LOWER_TRANSPORT_CUSTOM = 16, 
};
enum RTSPTransport {
RTSP_TRANSPORT_RTP, 
RTSP_TRANSPORT_RDT, 
RTSP_TRANSPORT_RAW, 
RTSP_TRANSPORT_NB
};
enum RTSPControlTransport {
RTSP_MODE_PLAIN, 
RTSP_MODE_TUNNEL 
};
#define RTSP_DEFAULT_PORT 554
#define RTSPS_DEFAULT_PORT 322
#define RTSP_MAX_TRANSPORTS 8
#define RTSP_TCP_MAX_PACKET_SIZE 1472
#define RTSP_DEFAULT_NB_AUDIO_CHANNELS 1
#define RTSP_DEFAULT_AUDIO_SAMPLERATE 44100
#define RTSP_RTP_PORT_MIN 5000
#define RTSP_RTP_PORT_MAX 65000
typedef struct RTSPTransportField {
int interleaved_min, interleaved_max;
int port_min, port_max;
int client_port_min, client_port_max;
int server_port_min, server_port_max;
int ttl;
int mode_record;
struct sockaddr_storage destination; 
char source[INET6_ADDRSTRLEN + 1]; 
enum RTSPTransport transport;
enum RTSPLowerTransport lower_transport;
} RTSPTransportField;
typedef struct RTSPMessageHeader {
int content_length;
enum RTSPStatusCode status_code; 
int nb_transports;
int64_t range_start, range_end;
RTSPTransportField transports[RTSP_MAX_TRANSPORTS];
int seq; 
char session_id[512];
char location[4096];
char real_challenge[64];
char server[64];
int timeout;
int notice;
char reason[256];
char content_type[64];
} RTSPMessageHeader;
enum RTSPClientState {
RTSP_STATE_IDLE, 
RTSP_STATE_STREAMING, 
RTSP_STATE_PAUSED, 
RTSP_STATE_SEEKING, 
};
enum RTSPServerType {
RTSP_SERVER_RTP, 
RTSP_SERVER_REAL, 
RTSP_SERVER_WMS, 
RTSP_SERVER_NB
};
typedef struct RTSPState {
const AVClass *class; 
URLContext *rtsp_hd; 
int nb_rtsp_streams;
struct RTSPStream **rtsp_streams; 
enum RTSPClientState state;
int64_t seek_timestamp;
int seq; 
char session_id[512];
int timeout;
int64_t last_cmd_time;
enum RTSPTransport transport;
enum RTSPLowerTransport lower_transport;
enum RTSPServerType server_type;
char real_challenge[64];
char auth[128];
HTTPAuthState auth_state;
char last_reply[2048]; 
void *cur_transport_priv;
int need_subscription;
enum AVDiscard *real_setup_cache;
enum AVDiscard *real_setup;
char last_subscription[1024];
AVFormatContext *asf_ctx;
uint64_t asf_pb_pos;
char control_uri[1024];
struct MpegTSContext *ts;
int recvbuf_pos;
int recvbuf_len;
URLContext *rtsp_hd_out;
enum RTSPControlTransport control_transport;
int nb_byes;
uint8_t* recvbuf;
int lower_transport_mask;
uint64_t packets;
struct pollfd *p;
int max_p;
int get_parameter_supported;
int initial_pause;
int rtp_muxer_flags;
int accept_dynamic_rate;
int rtsp_flags;
int media_type_mask;
int rtp_port_min, rtp_port_max;
int initial_timeout;
int stimeout;
int reordering_queue_size;
char *user_agent;
char default_lang[4];
int buffer_size;
int pkt_size;
} RTSPState;
#define RTSP_FLAG_FILTER_SRC 0x1 
#define RTSP_FLAG_LISTEN 0x2 
#define RTSP_FLAG_CUSTOM_IO 0x4 
#define RTSP_FLAG_RTCP_TO_SOURCE 0x8 
#define RTSP_FLAG_PREFER_TCP 0x10 
typedef struct RTSPSource {
char addr[128]; 
} RTSPSource;
typedef struct RTSPStream {
URLContext *rtp_handle; 
void *transport_priv; 
int stream_index;
int interleaved_min, interleaved_max;
char control_url[1024]; 
int sdp_port; 
struct sockaddr_storage sdp_ip; 
int nb_include_source_addrs; 
struct RTSPSource **include_source_addrs; 
int nb_exclude_source_addrs; 
struct RTSPSource **exclude_source_addrs; 
int sdp_ttl; 
int sdp_payload_type; 
const RTPDynamicProtocolHandler *dynamic_handler;
PayloadContext *dynamic_protocol_context;
int feedback;
uint32_t ssrc;
char crypto_suite[40];
char crypto_params[100];
} RTSPStream;
void ff_rtsp_parse_line(AVFormatContext *s,
RTSPMessageHeader *reply, const char *buf,
RTSPState *rt, const char *method);
int ff_rtsp_send_cmd_async(AVFormatContext *s, const char *method,
const char *url, const char *headers);
int ff_rtsp_send_cmd_with_content(AVFormatContext *s,
const char *method, const char *url,
const char *headers,
RTSPMessageHeader *reply,
unsigned char **content_ptr,
const unsigned char *send_content,
int send_content_length);
int ff_rtsp_send_cmd(AVFormatContext *s, const char *method,
const char *url, const char *headers,
RTSPMessageHeader *reply, unsigned char **content_ptr);
int ff_rtsp_read_reply(AVFormatContext *s, RTSPMessageHeader *reply,
unsigned char **content_ptr,
int return_on_interleaved_data, const char *method);
void ff_rtsp_skip_packet(AVFormatContext *s);
int ff_rtsp_connect(AVFormatContext *s);
void ff_rtsp_close_streams(AVFormatContext *s);
void ff_rtsp_close_connections(AVFormatContext *s);
int ff_rtsp_setup_input_streams(AVFormatContext *s, RTSPMessageHeader *reply);
int ff_rtsp_setup_output_streams(AVFormatContext *s, const char *addr);
int ff_rtsp_parse_streaming_commands(AVFormatContext *s);
int ff_sdp_parse(AVFormatContext *s, const char *content);
int ff_rtsp_tcp_read_packet(AVFormatContext *s, RTSPStream **prtsp_st,
uint8_t *buf, int buf_size);
int ff_rtsp_tcp_write_packet(AVFormatContext *s, RTSPStream *rtsp_st);
int ff_rtsp_fetch_packet(AVFormatContext *s, AVPacket *pkt);
int ff_rtsp_make_setup_request(AVFormatContext *s, const char *host, int port,
int lower_transport, const char *real_challenge);
void ff_rtsp_undo_setup(AVFormatContext *s, int send_packets);
int ff_rtsp_open_transport_ctx(AVFormatContext *s, RTSPStream *rtsp_st);
extern const AVOption ff_rtsp_options[];
