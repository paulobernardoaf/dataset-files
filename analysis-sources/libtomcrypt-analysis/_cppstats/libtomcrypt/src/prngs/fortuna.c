#include "tomcrypt_private.h"
#if defined(LTC_FORTUNA_RESEED_RATELIMIT_TIMED)
#if defined(_WIN32)
#include <windows.h>
#elif defined(LTC_CLOCK_GETTIME)
#include <time.h> 
#else
#include <sys/time.h> 
#endif
#endif
#if defined(LTC_FORTUNA)
#if !(defined(LTC_RIJNDAEL) && defined(LTC_SHA256))
#error LTC_FORTUNA requires LTC_SHA256 and LTC_RIJNDAEL (AES)
#endif
#if !defined(LTC_FORTUNA_POOLS)
#warning LTC_FORTUNA_POOLS was not previously defined (old headers?)
#define LTC_FORTUNA_POOLS 32
#endif
#if LTC_FORTUNA_POOLS < 4 || LTC_FORTUNA_POOLS > 32
#error LTC_FORTUNA_POOLS must be in [4..32]
#endif
const struct ltc_prng_descriptor fortuna_desc = {
"fortuna",
64,
&fortuna_start,
&fortuna_add_entropy,
&fortuna_ready,
&fortuna_read,
&fortuna_done,
&fortuna_export,
&fortuna_import,
&fortuna_test
};
static void _fortuna_update_iv(prng_state *prng)
{
int x;
unsigned char *IV;
IV = prng->u.fortuna.IV;
for (x = 0; x < 16; x++) {
IV[x] = (IV[x] + 1) & 255;
if (IV[x] != 0) break;
}
}
#if defined(LTC_FORTUNA_RESEED_RATELIMIT_TIMED)
static ulong64 _fortuna_current_time(void)
{
ulong64 cur_time;
#if defined(_WIN32)
FILETIME CurrentTime;
ULARGE_INTEGER ul;
GetSystemTimeAsFileTime(&CurrentTime);
ul.LowPart = CurrentTime.dwLowDateTime;
ul.HighPart = CurrentTime.dwHighDateTime;
cur_time = ul.QuadPart; 
cur_time -= CONST64(116444736000000000); 
cur_time /= 10; 
#elif defined(LTC_CLOCK_GETTIME)
struct timespec ts;
clock_gettime(CLOCK_MONOTONIC, &ts);
cur_time = (ulong64)(ts.tv_sec) * 1000000 + (ulong64)(ts.tv_nsec) / 1000; 
#else
struct timeval tv;
gettimeofday(&tv, NULL);
cur_time = (ulong64)(tv.tv_sec) * 1000000 + (ulong64)(tv.tv_usec); 
#endif
return cur_time / 100;
}
#endif
static int _fortuna_reseed(prng_state *prng)
{
unsigned char tmp[MAXBLOCKSIZE];
hash_state md;
ulong64 reset_cnt;
int err, x;
#if defined(LTC_FORTUNA_RESEED_RATELIMIT_TIMED)
ulong64 now = _fortuna_current_time();
if (now == prng->u.fortuna.wd) {
return CRYPT_OK;
}
#else
if (++prng->u.fortuna.wd < LTC_FORTUNA_WD) {
return CRYPT_OK;
}
#endif
sha256_init(&md);
if ((err = sha256_process(&md, prng->u.fortuna.K, 32)) != CRYPT_OK) {
sha256_done(&md, tmp);
return err;
}
reset_cnt = prng->u.fortuna.reset_cnt + 1;
for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
if (x == 0 || ((reset_cnt >> (x-1)) & 1) == 0) {
if ((err = sha256_done(&prng->u.fortuna.pool[x], tmp)) != CRYPT_OK) {
sha256_done(&md, tmp);
return err;
}
if ((err = sha256_process(&md, tmp, 32)) != CRYPT_OK) {
sha256_done(&md, tmp);
return err;
}
if ((err = sha256_init(&prng->u.fortuna.pool[x])) != CRYPT_OK) {
sha256_done(&md, tmp);
return err;
}
} else {
break;
}
}
if ((err = sha256_done(&md, prng->u.fortuna.K)) != CRYPT_OK) {
return err;
}
if ((err = rijndael_setup(prng->u.fortuna.K, 32, 0, &prng->u.fortuna.skey)) != CRYPT_OK) {
return err;
}
_fortuna_update_iv(prng);
prng->u.fortuna.pool0_len = 0;
#if defined(LTC_FORTUNA_RESEED_RATELIMIT_TIMED)
prng->u.fortuna.wd = now;
#else
prng->u.fortuna.wd = 0;
#endif
prng->u.fortuna.reset_cnt = reset_cnt;
#if defined(LTC_CLEAN_STACK)
zeromem(&md, sizeof(md));
zeromem(tmp, sizeof(tmp));
#endif
return CRYPT_OK;
}
int fortuna_update_seed(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
int err;
unsigned char tmp[MAXBLOCKSIZE];
hash_state md;
LTC_MUTEX_LOCK(&prng->lock);
sha256_init(&md);
if ((err = sha256_process(&md, prng->u.fortuna.K, 32)) != CRYPT_OK) {
sha256_done(&md, tmp);
goto LBL_UNLOCK;
}
if ((err = sha256_process(&md, in, inlen)) != CRYPT_OK) {
sha256_done(&md, tmp);
goto LBL_UNLOCK;
}
if ((err = sha256_done(&md, prng->u.fortuna.K)) != CRYPT_OK) {
goto LBL_UNLOCK;
}
_fortuna_update_iv(prng);
LBL_UNLOCK:
LTC_MUTEX_UNLOCK(&prng->lock);
#if defined(LTC_CLEAN_STACK)
zeromem(&md, sizeof(md));
#endif
return err;
}
int fortuna_start(prng_state *prng)
{
int err, x, y;
unsigned char tmp[MAXBLOCKSIZE];
LTC_ARGCHK(prng != NULL);
prng->ready = 0;
for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
if ((err = sha256_init(&prng->u.fortuna.pool[x])) != CRYPT_OK) {
for (y = 0; y < x; y++) {
sha256_done(&prng->u.fortuna.pool[y], tmp);
}
return err;
}
}
prng->u.fortuna.pool_idx = prng->u.fortuna.pool0_len = 0;
prng->u.fortuna.reset_cnt = prng->u.fortuna.wd = 0;
zeromem(prng->u.fortuna.K, 32);
if ((err = rijndael_setup(prng->u.fortuna.K, 32, 0, &prng->u.fortuna.skey)) != CRYPT_OK) {
for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
sha256_done(&prng->u.fortuna.pool[x], tmp);
}
return err;
}
zeromem(prng->u.fortuna.IV, 16);
LTC_MUTEX_INIT(&prng->lock)
return CRYPT_OK;
}
static int _fortuna_add(unsigned long source, unsigned long pool, const unsigned char *in, unsigned long inlen, prng_state *prng)
{
unsigned char tmp[2];
int err;
if (inlen > 32) {
inlen = 32;
}
tmp[0] = (unsigned char)source;
tmp[1] = (unsigned char)inlen;
if ((err = sha256_process(&prng->u.fortuna.pool[pool], tmp, 2)) != CRYPT_OK) {
return err;
}
if ((err = sha256_process(&prng->u.fortuna.pool[pool], in, inlen)) != CRYPT_OK) {
return err;
}
if (pool == 0) {
prng->u.fortuna.pool0_len += inlen;
}
return CRYPT_OK; 
}
int fortuna_add_random_event(unsigned long source, unsigned long pool, const unsigned char *in, unsigned long inlen, prng_state *prng)
{
int err;
LTC_ARGCHK(prng != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen > 0);
LTC_ARGCHK(source <= 255);
LTC_ARGCHK(pool < LTC_FORTUNA_POOLS);
LTC_MUTEX_LOCK(&prng->lock);
err = _fortuna_add(source, pool, in, inlen, prng);
LTC_MUTEX_UNLOCK(&prng->lock);
return err;
}
int fortuna_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
int err;
LTC_ARGCHK(prng != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen > 0);
LTC_MUTEX_LOCK(&prng->lock);
err = _fortuna_add(0, prng->u.fortuna.pool_idx, in, inlen, prng);
if (err == CRYPT_OK) {
++(prng->u.fortuna.pool_idx);
prng->u.fortuna.pool_idx %= LTC_FORTUNA_POOLS;
}
LTC_MUTEX_UNLOCK(&prng->lock);
return err;
}
int fortuna_ready(prng_state *prng)
{
int err;
LTC_ARGCHK(prng != NULL);
LTC_MUTEX_LOCK(&prng->lock);
#if defined(LTC_FORTUNA_RESEED_RATELIMIT_TIMED)
prng->u.fortuna.wd = _fortuna_current_time() - 1;
#else
prng->u.fortuna.wd = LTC_FORTUNA_WD;
#endif
err = _fortuna_reseed(prng);
prng->ready = (err == CRYPT_OK) ? 1 : 0;
LTC_MUTEX_UNLOCK(&prng->lock);
return err;
}
unsigned long fortuna_read(unsigned char *out, unsigned long outlen, prng_state *prng)
{
unsigned char tmp[16];
unsigned long tlen = 0;
if (outlen == 0 || prng == NULL || out == NULL) return 0;
LTC_MUTEX_LOCK(&prng->lock);
if (!prng->ready) {
goto LBL_UNLOCK;
}
if (prng->u.fortuna.pool0_len >= 64) {
if (_fortuna_reseed(prng) != CRYPT_OK) {
goto LBL_UNLOCK;
}
}
if (prng->u.fortuna.reset_cnt == 0) {
goto LBL_UNLOCK;
}
tlen = outlen;
while (outlen >= 16) {
rijndael_ecb_encrypt(prng->u.fortuna.IV, out, &prng->u.fortuna.skey);
out += 16;
outlen -= 16;
_fortuna_update_iv(prng);
}
if (outlen > 0) {
rijndael_ecb_encrypt(prng->u.fortuna.IV, tmp, &prng->u.fortuna.skey);
XMEMCPY(out, tmp, outlen);
_fortuna_update_iv(prng);
}
rijndael_ecb_encrypt(prng->u.fortuna.IV, prng->u.fortuna.K , &prng->u.fortuna.skey);
_fortuna_update_iv(prng);
rijndael_ecb_encrypt(prng->u.fortuna.IV, prng->u.fortuna.K+16, &prng->u.fortuna.skey);
_fortuna_update_iv(prng);
if (rijndael_setup(prng->u.fortuna.K, 32, 0, &prng->u.fortuna.skey) != CRYPT_OK) {
tlen = 0;
}
LBL_UNLOCK:
#if defined(LTC_CLEAN_STACK)
zeromem(tmp, sizeof(tmp));
#endif
LTC_MUTEX_UNLOCK(&prng->lock);
return tlen;
}
int fortuna_done(prng_state *prng)
{
int err, x;
unsigned char tmp[32];
LTC_ARGCHK(prng != NULL);
LTC_MUTEX_LOCK(&prng->lock);
prng->ready = 0;
for (x = 0; x < LTC_FORTUNA_POOLS; x++) {
if ((err = sha256_done(&(prng->u.fortuna.pool[x]), tmp)) != CRYPT_OK) {
goto LBL_UNLOCK;
}
}
err = CRYPT_OK; 
LBL_UNLOCK:
#if defined(LTC_CLEAN_STACK)
zeromem(tmp, sizeof(tmp));
#endif
LTC_MUTEX_UNLOCK(&prng->lock);
LTC_MUTEX_DESTROY(&prng->lock);
return err;
}
_LTC_PRNG_EXPORT(fortuna)
int fortuna_import(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(prng != NULL);
if (inlen < (unsigned long)fortuna_desc.export_size) {
return CRYPT_INVALID_ARG;
}
if ((err = fortuna_start(prng)) != CRYPT_OK) {
return err;
}
if ((err = fortuna_update_seed(in, inlen, prng)) != CRYPT_OK) {
return err;
}
return err;
}
int fortuna_test(void)
{
#if !defined(LTC_TEST)
return CRYPT_NOP;
#else
int err;
if ((err = sha256_test()) != CRYPT_OK) {
return err;
}
return rijndael_test();
#endif
}
#endif
