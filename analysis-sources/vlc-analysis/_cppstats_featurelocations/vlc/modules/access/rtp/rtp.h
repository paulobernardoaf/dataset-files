





















typedef struct rtp_pt_t rtp_pt_t;
typedef struct rtp_session_t rtp_session_t;

struct vlc_demux_chained_t;


struct rtp_pt_t
{
void *(*init) (demux_t *);
void (*destroy) (demux_t *, void *);
void (*header) (demux_t *, void *, block_t *);
void (*decode) (demux_t *, void *, block_t *);
uint32_t frequency; 
uint8_t number;
};
void rtp_autodetect (demux_t *, rtp_session_t *, const block_t *);

static inline uint8_t rtp_ptype (const block_t *block)
{
return block->p_buffer[1] & 0x7F;
}

void *codec_init (demux_t *demux, es_format_t *fmt);
void codec_destroy (demux_t *demux, void *data);
void codec_decode (demux_t *demux, void *data, block_t *block);

void *theora_init (demux_t *demux);
void xiph_destroy (demux_t *demux, void *data);
void xiph_decode (demux_t *demux, void *data, block_t *block);


rtp_session_t *rtp_session_create (demux_t *);
void rtp_session_destroy (demux_t *, rtp_session_t *);
void rtp_queue (demux_t *, rtp_session_t *, block_t *);
bool rtp_dequeue (demux_t *, const rtp_session_t *, vlc_tick_t *);
void rtp_dequeue_force (demux_t *, const rtp_session_t *);
int rtp_add_type (demux_t *demux, rtp_session_t *ses, const rtp_pt_t *pt);

void *rtp_dgram_thread (void *data);
void *rtp_stream_thread (void *data);


typedef struct
{
rtp_session_t *session;
struct vlc_demux_chained_t *chained_demux;
#if defined(HAVE_SRTP)
struct srtp_session_t *srtp;
#endif
int fd;
int rtcp_fd;
vlc_thread_t thread;

vlc_tick_t timeout;
uint16_t max_dropout; 
uint16_t max_misorder; 
uint8_t max_src; 
bool thread_ready;
bool autodetect; 
} demux_sys_t;

