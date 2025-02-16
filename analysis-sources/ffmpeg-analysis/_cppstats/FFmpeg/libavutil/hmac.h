#include <stdint.h>
#include "version.h"
enum AVHMACType {
AV_HMAC_MD5,
AV_HMAC_SHA1,
AV_HMAC_SHA224,
AV_HMAC_SHA256,
AV_HMAC_SHA384,
AV_HMAC_SHA512,
};
typedef struct AVHMAC AVHMAC;
AVHMAC *av_hmac_alloc(enum AVHMACType type);
void av_hmac_free(AVHMAC *ctx);
void av_hmac_init(AVHMAC *ctx, const uint8_t *key, unsigned int keylen);
void av_hmac_update(AVHMAC *ctx, const uint8_t *data, unsigned int len);
int av_hmac_final(AVHMAC *ctx, uint8_t *out, unsigned int outlen);
int av_hmac_calc(AVHMAC *ctx, const uint8_t *data, unsigned int len,
const uint8_t *key, unsigned int keylen,
uint8_t *out, unsigned int outlen);
