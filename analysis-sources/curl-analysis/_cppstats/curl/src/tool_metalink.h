#include "tool_setup.h"
#include "tool_sdecls.h"
struct GlobalConfig;
struct OperationConfig;
typedef int (* Curl_digest_init_func)(void *context);
typedef void (* Curl_digest_update_func)(void *context,
const unsigned char *data,
unsigned int len);
typedef void (* Curl_digest_final_func)(unsigned char *result, void *context);
typedef struct {
Curl_digest_init_func digest_init; 
Curl_digest_update_func digest_update; 
Curl_digest_final_func digest_final; 
unsigned int digest_ctxtsize; 
unsigned int digest_resultlen; 
} digest_params;
typedef struct {
const digest_params *digest_hash; 
void *digest_hashctx; 
} digest_context;
digest_context * Curl_digest_init(const digest_params *dparams);
int Curl_digest_update(digest_context *context,
const unsigned char *data,
unsigned int len);
int Curl_digest_final(digest_context *context, unsigned char *result);
typedef struct {
const char *hash_name;
const digest_params *dparams;
} metalink_digest_def;
typedef struct {
const char *alias_name;
const metalink_digest_def *digest_def;
} metalink_digest_alias;
typedef struct metalink_checksum {
const metalink_digest_def *digest_def;
unsigned char *digest;
} metalink_checksum;
typedef struct metalink_resource {
struct metalink_resource *next;
char *url;
} metalink_resource;
typedef struct metalinkfile {
struct metalinkfile *next;
char *filename;
metalink_checksum *checksum;
metalink_resource *resource;
} metalinkfile;
#if defined(USE_METALINK)
#define CURL_REQ_LIBMETALINK_MAJOR 0
#define CURL_REQ_LIBMETALINK_MINOR 1
#define CURL_REQ_LIBMETALINK_PATCH 0
#define CURL_REQ_LIBMETALINK_VERS ((CURL_REQ_LIBMETALINK_MAJOR * 10000) + (CURL_REQ_LIBMETALINK_MINOR * 100) + CURL_REQ_LIBMETALINK_PATCH)
extern const digest_params MD5_DIGEST_PARAMS[1];
extern const digest_params SHA1_DIGEST_PARAMS[1];
extern const digest_params SHA256_DIGEST_PARAMS[1];
#include <metalink/metalink.h>
int count_next_metalink_resource(metalinkfile *mlfile);
void delete_metalinkfile(metalinkfile *mlfile);
void clean_metalink(struct OperationConfig *config);
int parse_metalink(struct OperationConfig *config, struct OutStruct *outs,
const char *metalink_url);
size_t metalink_write_cb(void *buffer, size_t sz, size_t nmemb,
void *userdata);
int check_metalink_content_type(const char *content_type);
int metalink_check_hash(struct GlobalConfig *config,
metalinkfile *mlfile,
const char *filename);
void metalink_cleanup(void);
#else 
#define count_next_metalink_resource(x) 0
#define delete_metalinkfile(x) (void)x
#define clean_metalink(x) (void)x
#define metalink_cleanup() Curl_nop_stmt
#endif 
