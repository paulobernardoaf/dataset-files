typedef struct rtsp_stream_t rtsp_stream_t;
typedef struct rtsp_stream_id_t rtsp_stream_id_t;
typedef struct sout_stream_id_sys_t sout_stream_id_sys_t;
rtsp_stream_t *RtspSetup( vlc_object_t *owner, const char *path );
void RtspUnsetup( rtsp_stream_t *rtsp );
rtsp_stream_id_t *RtspAddId( rtsp_stream_t *rtsp, sout_stream_id_sys_t *sid,
uint32_t ssrc, unsigned clock_rate,
int mcast_fd );
void RtspDelId( rtsp_stream_t *rtsp, rtsp_stream_id_t * );
char *RtspAppendTrackPath( rtsp_stream_id_t *id, const char *base );
int RtspTrackAttach( rtsp_stream_t *rtsp, const char *name,
rtsp_stream_id_t *id, sout_stream_id_sys_t *sout_id,
uint32_t *ssrc, uint16_t *seq_init );
void RtspTrackDetach( rtsp_stream_t *rtsp, const char *name,
sout_stream_id_sys_t *sout_id);
char *SDPGenerate( sout_stream_t *p_stream, const char *rtsp_url );
uint32_t rtp_compute_ts( unsigned i_clock_rate, vlc_tick_t i_pts );
int rtp_add_sink( sout_stream_id_sys_t *id, int fd, bool rtcp_mux, uint16_t *seq );
void rtp_del_sink( sout_stream_id_sys_t *id, int fd );
uint16_t rtp_get_seq( sout_stream_id_sys_t *id );
vlc_tick_t rtp_get_ts( const sout_stream_t *p_stream, const sout_stream_id_sys_t *id,
vlc_tick_t *p_npt );
void rtp_packetize_common (sout_stream_id_sys_t *id, block_t *out,
bool b_m_bit, vlc_tick_t i_pts);
void rtp_packetize_send (sout_stream_id_sys_t *id, block_t *out);
size_t rtp_mtu (const sout_stream_id_sys_t *id);
int rtp_packetize_xiph_config( sout_stream_id_sys_t *id, const char *fmtp,
vlc_tick_t i_pts );
typedef struct rtcp_sender_t rtcp_sender_t;
rtcp_sender_t *OpenRTCP (vlc_object_t *obj, int rtp_fd, int proto,
bool mux);
void CloseRTCP (rtcp_sender_t *rtcp);
void SendRTCP (rtcp_sender_t *restrict rtcp, const block_t *rtp);
typedef int (*pf_rtp_packetizer_t)( sout_stream_id_sys_t *, block_t * );
typedef struct rtp_format_t
{
uint8_t payload_type;
unsigned clock_rate;
unsigned channels;
enum es_format_category_e cat;
unsigned bitrate;
const char *ptname;
char *fmtp;
pf_rtp_packetizer_t pf_packetize;
} rtp_format_t;
int rtp_get_fmt( vlc_object_t *obj, const es_format_t *p_fmt, const char *mux,
rtp_format_t *p_rtp_fmt );
void rtp_get_video_geometry( sout_stream_id_sys_t *id, int *width, int *height );
uint16_t rtp_get_extended_sequence( sout_stream_id_sys_t *id );
