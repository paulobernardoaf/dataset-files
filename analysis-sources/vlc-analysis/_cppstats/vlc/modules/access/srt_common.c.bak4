






















#include "srt_common.h"

const char * const srt_key_length_names[] = { N_( "16 bytes" ), N_(
"24 bytes" ), N_( "32 bytes" ), };

typedef struct parsed_param {
char *key;
char *val;
} parsed_param_t;

static inline char*
find(char *str, char find)
{
str = strchr( str, find );
return str != NULL ? str + 1 : NULL;
}


















static int srt_url_parse_query(char *query, const char* delimiter,
parsed_param_t *params, int max_params)
{
int i = 0;
char *token = NULL;

if (!query || *query == '\0')
return -1;
if (!params || max_params == 0)
return 0;

token = strtok( query, delimiter );
while (token != NULL && i < max_params) {
params[i].key = token;
params[i].val = NULL;
if ((params[i].val = strchr( params[i].key, '=' )) != NULL) {
size_t val_len = strlen( params[i].val );


*(params[i].val) = '\0';


if (val_len > 1) {
params[i].val++;


if (params[i].key[0])
i++;
};
}
token = strtok( NULL, delimiter );
}
return i;
}

bool srt_parse_url(char* url, srt_params_t* params)
{
char* query = NULL;
struct parsed_param local_params[32];
int num_params = 0;
int i = 0;
bool rc = false;

if (!url || !url[0] || !params)
return false;


params->latency = -1;
params->passphrase = NULL;
params->key_length = -1;
params->payload_size = -1;
params->bandwidth_overhead_limit = -1;


query = find( url, '?' );
if (query) {
num_params = srt_url_parse_query( query, "&", local_params,
sizeof(local_params) / sizeof(struct parsed_param) );
if (num_params > 0) {
rc = true;
for (i = 0; i < num_params; ++i) {
char* val = local_params[i].val;
if (!val)
continue;

if (strcmp( local_params[i].key, SRT_PARAM_LATENCY ) == 0) {
int temp = atoi( val );
if (temp >= 0)
params->latency = temp;
} else if (strcmp( local_params[i].key, SRT_PARAM_PASSPHRASE )
== 0) {
params->passphrase = val;
} else if (strcmp( local_params[i].key, SRT_PARAM_PAYLOAD_SIZE )
== 0) {
int temp = atoi( val );
if (temp >= 0)
params->payload_size = temp;
} else if (strcmp( local_params[i].key, SRT_PARAM_KEY_LENGTH )
== 0) {
int temp = atoi( val );
if (temp == srt_key_lengths[0] || temp == srt_key_lengths[1]
|| temp == srt_key_lengths[2]) {
params->key_length = temp;
}
} else if (strcmp( local_params[i].key,
SRT_PARAM_BANDWIDTH_OVERHEAD_LIMIT ) == 0) {
int temp = atoi( val );
if (temp >= 0)
params->bandwidth_overhead_limit = temp;

}
}
}
}

return rc;
}

int srt_set_socket_option(vlc_object_t *this, const char *srt_param,
SRTSOCKET u, SRT_SOCKOPT opt, const void *optval, int optlen)
{
int stat = 0;

stat = srt_setsockopt( u, 0, opt, optval, optlen );
if (stat)
msg_Err( this, "Failed to set socket option %s (reason: %s)", srt_param,
srt_getlasterror_str() );

return stat;
}

