typedef struct
{
uint16_t i_type;
uint16_t i_size;
uint32_t i_sequence;
uint16_t i_unknown;
uint16_t i_size2;
int i_data;
uint8_t *p_data;
} chunk_t;
#define BUFFER_SIZE 65536
typedef struct
{
int i_proto;
struct vlc_tls *stream;
vlc_url_t url;
bool b_proxy;
vlc_url_t proxy;
int i_request_context;
uint8_t buffer[BUFFER_SIZE + 1];
bool b_broadcast;
uint8_t *p_header;
int i_header;
uint8_t *p_packet;
uint32_t i_packet_sequence;
unsigned int i_packet_used;
unsigned int i_packet_length;
uint64_t i_start;
uint64_t i_position;
asf_header_t asfh;
vlc_guid_t guid;
} access_sys_t;
