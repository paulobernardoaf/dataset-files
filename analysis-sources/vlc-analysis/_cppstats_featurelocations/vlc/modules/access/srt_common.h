





















#if !defined(SRT_COMMON_H)
#define SRT_COMMON_H 1

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <vlc_common.h>
#include <srt/srt.h>



#define SRT_PARAM_LATENCY "latency"
#define SRT_PARAM_PASSPHRASE "passphrase"
#define SRT_PARAM_PAYLOAD_SIZE "payload-size"
#define SRT_PARAM_BANDWIDTH_OVERHEAD_LIMIT "bandwidth-overhead-limit"
#define SRT_PARAM_CHUNK_SIZE "chunk-size"
#define SRT_PARAM_POLL_TIMEOUT "poll-timeout"
#define SRT_PARAM_KEY_LENGTH "key-length"


#define SRT_DEFAULT_BANDWIDTH_OVERHEAD_LIMIT 25


#define SRT_DEFAULT_CHUNK_SIZE SRT_LIVE_DEF_PLSIZE

#define SRT_DEFAULT_PORT 9000

#define SRT_MIN_CHUNKS_TRYREAD 10
#define SRT_MAX_CHUNKS_TRYREAD 100

#define SRT_DEFAULT_POLL_TIMEOUT -1

#define SRT_DEFAULT_LATENCY SRT_LIVE_DEF_LATENCY_MS
#define SRT_DEFAULT_PAYLOAD_SIZE SRT_LIVE_DEF_PLSIZE

#define SRT_KEY_LENGTH_TEXT N_("Crypto key length in bytes")
#define SRT_DEFAULT_KEY_LENGTH 16
static const int srt_key_lengths[] = { 16, 24, 32, };

extern const char * const srt_key_length_names[];

typedef struct srt_params {
int latency;
const char* passphrase;
int key_length;
int payload_size;
int bandwidth_overhead_limit;
} srt_params_t;

bool srt_parse_url(char* url, srt_params_t* params);

int srt_set_socket_option(vlc_object_t *this, const char *srt_param,
SRTSOCKET u, SRT_SOCKOPT opt, const void *optval, int optlen);

#endif
