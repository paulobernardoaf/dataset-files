#include <r_lib.h>
#include <r_crypto.h>
#include "crypto_aes_algo.h"
static struct aes_state st = {{0}};
static bool aes_set_key (RCrypto *cry, const ut8 *key, int keylen, int mode, int direction) {
if (!(keylen == 128 / 8 || keylen == 192 / 8 || keylen == 256 / 8)) {
return false;
}
st.key_size = keylen;
st.rounds = 6 + (int)(keylen / 4);
st.columns = (int)(keylen / 4);
memcpy (st.key, key, keylen);
cry->dir = direction;
return true;
}
static int aes_get_key_size (RCrypto *cry) {
return st.key_size;
}
static bool aes_use (const char *algo) {
return !strcmp (algo, "aes-ecb");
}
#define BLOCK_SIZE 16
static bool update (RCrypto *cry, const ut8 *buf, int len) {
const int diff = (BLOCK_SIZE - (len % BLOCK_SIZE)) % BLOCK_SIZE;
const int size = len + diff;
const int blocks = size / BLOCK_SIZE;
int i;
ut8 *const obuf = calloc (1, size);
if (!obuf) {
return false;
}
ut8 *const ibuf = calloc (1, size);
if (!ibuf) {
free (obuf);
return false;
}
memset (ibuf, 0, size);
memcpy (ibuf, buf, len);
if (diff) {
ibuf[len] = 8; 
}
if (cry->dir == 0) {
for (i = 0; i < blocks; i++) {
const int delta = BLOCK_SIZE * i;
aes_encrypt (&st, ibuf + delta, obuf + delta);
}
} else if (cry->dir > 0) {
for (i = 0; i < blocks; i++) {
const int delta = BLOCK_SIZE * i;
aes_decrypt (&st, ibuf + delta, obuf + delta);
}
}
r_crypto_append (cry, obuf, size);
free (obuf);
free (ibuf);
return true;
}
static bool final (RCrypto *cry, const ut8 *buf, int len) {
return update (cry, buf, len);
}
RCryptoPlugin r_crypto_plugin_aes = {
.name = "aes-ecb",
.set_key = aes_set_key,
.get_key_size = aes_get_key_size,
.use = aes_use,
.update = update,
.final = final
};
#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_CRYPTO,
.data = &r_crypto_plugin_aes,
.version = R2_VERSION
};
#endif
