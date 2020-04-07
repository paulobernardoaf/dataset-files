




























#if defined(OGG_DEMUX_DEBUG)
#define DemuxDebug(code) code
#else
#define DemuxDebug(code)
#endif


#define PACKET_TYPE_HEADER 0x01
#define PACKET_TYPE_BITS 0x07
#define PACKET_LEN_BITS01 0xc0
#define PACKET_LEN_BITS2 0x02
#define PACKET_IS_SYNCPOINT 0x08

#define OGGDS_RESOLUTION 10000000

typedef struct oggseek_index_entry demux_index_entry_t;
typedef struct ogg_skeleton_t ogg_skeleton_t;

typedef struct backup_queue
{
block_t *p_block;
vlc_tick_t i_duration;
} backup_queue_t;

typedef struct logical_stream_s
{
ogg_stream_state os; 

es_format_t fmt;
es_format_t fmt_old; 
es_out_id_t *p_es;
date_t dts;
bool b_contiguous; 
bool b_interpolation_failed; 

int i_serial_no;




bool b_force_backup;
int i_packets_backup;
int32_t i_extra_headers_packets;
void *p_headers;
int i_headers;
ogg_int64_t i_granulepos_offset;



vlc_tick_t i_pcr;


bool b_initializing;
bool b_finished;
bool b_reinit;
bool b_oggds;
int i_granule_shift;
int i_next_block_flags;


int i_pre_skip;


int8_t i_first_frame_index;


demux_index_entry_t *idx;


ogg_skeleton_t *p_skel;


unsigned int i_skip_frames;


int64_t i_data_start;


int i_secondary_header_packets;


struct
{
block_t *p_blocks;
block_t **pp_append;
} queue;

union
{
#if defined(HAVE_LIBVORBIS)
struct
{
vorbis_info *p_info;
vorbis_comment *p_comment;
int i_headers_flags;
int i_prev_blocksize;
} vorbis;
#endif
struct
{

int i_num_headers;
} kate;
struct
{
bool b_interlaced;
bool b_old;
} dirac;
struct
{
int32_t i_framesize;
int32_t i_framesperpacket;
} speex;
struct
{
bool b_old;
} flac;
} special;

} logical_stream_t;

struct ogg_skeleton_t
{
int i_messages;
char **ppsz_messages;
unsigned char *p_index;
uint64_t i_index;
uint64_t i_index_size;
int64_t i_indexstampden;
int64_t i_indexfirstnum;
int64_t i_indexlastnum;
};

typedef struct
{
ogg_sync_state oy; 

int i_streams; 
logical_stream_t **pp_stream; 
logical_stream_t *p_skelstream; 

logical_stream_t *p_old_stream; 



vlc_tick_t i_pcr;
vlc_tick_t i_nzpcr_offset;


bool b_chained_boundary;


int i_bitrate;
bool b_partial_bitrate;


bool b_page_waiting;


int64_t i_total_frames;


int64_t i_total_length;


int64_t i_input_position;


ogg_page current_page;


vlc_meta_t *p_meta;
int cur_seekpoint;
int i_seekpoints;
seekpoint_t **pp_seekpoints;
unsigned updates;


struct
{
uint16_t major;
uint16_t minor;
} skeleton;


int i_attachments;
input_attachment_t **attachments;


bool b_preparsing_done;
bool b_es_created;


int64_t i_length;

bool b_slave;

} demux_sys_t;


unsigned const char * Read7BitsVariableLE( unsigned const char *,
unsigned const char *,
uint64_t * );
bool Ogg_GetBoundsUsingSkeletonIndex( logical_stream_t *p_stream, vlc_tick_t i_time,
int64_t *pi_lower, int64_t *pi_upper );
