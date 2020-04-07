




















#if !defined(AVFORMAT_TLS_H)
#define AVFORMAT_TLS_H

#include "libavutil/opt.h"

#include "url.h"

typedef struct TLSShared {
char *ca_file;
int verify;
char *cert_file;
char *key_file;
int listen;

char *host;

char underlying_host[200];
int numerichost;

URLContext *tcp;
} TLSShared;

#define TLS_OPTFL (AV_OPT_FLAG_DECODING_PARAM | AV_OPT_FLAG_ENCODING_PARAM)
#define TLS_COMMON_OPTIONS(pstruct, options_field) {"ca_file", "Certificate Authority database file", offsetof(pstruct, options_field . ca_file), AV_OPT_TYPE_STRING, .flags = TLS_OPTFL }, {"cafile", "Certificate Authority database file", offsetof(pstruct, options_field . ca_file), AV_OPT_TYPE_STRING, .flags = TLS_OPTFL }, {"tls_verify", "Verify the peer certificate", offsetof(pstruct, options_field . verify), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, .flags = TLS_OPTFL }, {"cert_file", "Certificate file", offsetof(pstruct, options_field . cert_file), AV_OPT_TYPE_STRING, .flags = TLS_OPTFL }, {"key_file", "Private key file", offsetof(pstruct, options_field . key_file), AV_OPT_TYPE_STRING, .flags = TLS_OPTFL }, {"listen", "Listen for incoming connections", offsetof(pstruct, options_field . listen), AV_OPT_TYPE_INT, { .i64 = 0 }, 0, 1, .flags = TLS_OPTFL }, {"verifyhost", "Verify against a specific hostname", offsetof(pstruct, options_field . host), AV_OPT_TYPE_STRING, .flags = TLS_OPTFL }








int ff_tls_open_underlying(TLSShared *c, URLContext *parent, const char *uri, AVDictionary **options);

void ff_gnutls_init(void);
void ff_gnutls_deinit(void);

int ff_openssl_init(void);
void ff_openssl_deinit(void);

#endif 
