








#include "tomcrypt_private.h"







#if defined(LTC_SOBER128)

const struct ltc_prng_descriptor sober128_desc =
{
"sober128",
40,
&sober128_start,
&sober128_add_entropy,
&sober128_ready,
&sober128_read,
&sober128_done,
&sober128_export,
&sober128_import,
&sober128_test
};






int sober128_start(prng_state *prng)
{
LTC_ARGCHK(prng != NULL);
prng->ready = 0;
XMEMSET(&prng->u.sober128.ent, 0, sizeof(prng->u.sober128.ent));
prng->u.sober128.idx = 0;
LTC_MUTEX_INIT(&prng->lock)
return CRYPT_OK;
}








int sober128_add_entropy(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
unsigned char buf[40];
unsigned long i;
int err;

LTC_ARGCHK(prng != NULL);
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen > 0);

LTC_MUTEX_LOCK(&prng->lock);
if (prng->ready) {

if ((err = sober128_stream_keystream(&prng->u.sober128.s, buf, sizeof(buf))) != CRYPT_OK) goto LBL_UNLOCK;
for(i = 0; i < inlen; i++) buf[i % sizeof(buf)] ^= in[i];

if ((err = sober128_stream_setup(&prng->u.sober128.s, buf, 32)) != CRYPT_OK) goto LBL_UNLOCK;

if ((err = sober128_stream_setiv(&prng->u.sober128.s, buf + 32, 8)) != CRYPT_OK) goto LBL_UNLOCK;

zeromem(buf, sizeof(buf));
}
else {

while (inlen--) prng->u.sober128.ent[prng->u.sober128.idx++ % sizeof(prng->u.sober128.ent)] ^= *in++;
}
err = CRYPT_OK;
LBL_UNLOCK:
LTC_MUTEX_UNLOCK(&prng->lock);
return err;
}






int sober128_ready(prng_state *prng)
{
int err;

LTC_ARGCHK(prng != NULL);

LTC_MUTEX_LOCK(&prng->lock);
if (prng->ready) { err = CRYPT_OK; goto LBL_UNLOCK; }

if ((err = sober128_stream_setup(&prng->u.sober128.s, prng->u.sober128.ent, 32)) != CRYPT_OK) goto LBL_UNLOCK;

if ((err = sober128_stream_setiv(&prng->u.sober128.s, prng->u.sober128.ent + 32, 8)) != CRYPT_OK) goto LBL_UNLOCK;
XMEMSET(&prng->u.sober128.ent, 0, sizeof(prng->u.sober128.ent));
prng->u.sober128.idx = 0;
prng->ready = 1;
LBL_UNLOCK:
LTC_MUTEX_UNLOCK(&prng->lock);
return err;
}








unsigned long sober128_read(unsigned char *out, unsigned long outlen, prng_state *prng)
{
if (outlen == 0 || prng == NULL || out == NULL) return 0;
LTC_MUTEX_LOCK(&prng->lock);
if (!prng->ready) { outlen = 0; goto LBL_UNLOCK; }
if (sober128_stream_keystream(&prng->u.sober128.s, out, outlen) != CRYPT_OK) outlen = 0;
LBL_UNLOCK:
LTC_MUTEX_UNLOCK(&prng->lock);
return outlen;
}






int sober128_done(prng_state *prng)
{
int err;
LTC_ARGCHK(prng != NULL);
LTC_MUTEX_LOCK(&prng->lock);
prng->ready = 0;
err = sober128_stream_done(&prng->u.sober128.s);
LTC_MUTEX_UNLOCK(&prng->lock);
LTC_MUTEX_DESTROY(&prng->lock);
return err;
}








_LTC_PRNG_EXPORT(sober128)








int sober128_import(const unsigned char *in, unsigned long inlen, prng_state *prng)
{
int err;

LTC_ARGCHK(prng != NULL);
LTC_ARGCHK(in != NULL);
if (inlen < (unsigned long)sober128_desc.export_size) return CRYPT_INVALID_ARG;

if ((err = sober128_start(prng)) != CRYPT_OK) return err;
if ((err = sober128_add_entropy(in, inlen, prng)) != CRYPT_OK) return err;
return CRYPT_OK;
}





int sober128_test(void)
{
#if !defined(LTC_TEST)
return CRYPT_NOP;
#else
prng_state st;
unsigned char en[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32 };
unsigned char dmp[300];
unsigned long dmplen = sizeof(dmp);
unsigned char out[500];
unsigned char t1[] = { 0x31, 0x82, 0xA7, 0xA5, 0x8B, 0xD7, 0xCB, 0x39, 0x86, 0x1A };
unsigned char t2[] = { 0x6B, 0x43, 0x9E, 0xBC, 0xE7, 0x62, 0x9B, 0xE6, 0x9B, 0x83 };
unsigned char t3[] = { 0x4A, 0x0E, 0x6C, 0xC1, 0xCF, 0xB4, 0x73, 0x49, 0x99, 0x05 };
int err;

if ((err = sober128_start(&st)) != CRYPT_OK) return err;

if ((err = sober128_add_entropy(en, sizeof(en), &st)) != CRYPT_OK) return err;
if ((err = sober128_ready(&st)) != CRYPT_OK) return err;
if (sober128_read(out, 10, &st) != 10) return CRYPT_ERROR_READPRNG; 
if (compare_testvector(out, 10, t1, sizeof(t1), "SOBER128-PRNG", 1)) return CRYPT_FAIL_TESTVECTOR;
if (sober128_read(out, 500, &st) != 500) return CRYPT_ERROR_READPRNG; 

if ((err = sober128_add_entropy(en, sizeof(en), &st)) != CRYPT_OK) return err;
if (sober128_read(out, 500, &st) != 500) return CRYPT_ERROR_READPRNG; 
if ((err = sober128_export(dmp, &dmplen, &st)) != CRYPT_OK) return err;
if (sober128_read(out, 500, &st) != 500) return CRYPT_ERROR_READPRNG; 
if (sober128_read(out, 10, &st) != 10) return CRYPT_ERROR_READPRNG; 
if (compare_testvector(out, 10, t2, sizeof(t2), "SOBER128-PRNG", 2)) return CRYPT_FAIL_TESTVECTOR;
if ((err = sober128_done(&st)) != CRYPT_OK) return err;
if ((err = sober128_import(dmp, dmplen, &st)) != CRYPT_OK) return err;
if ((err = sober128_ready(&st)) != CRYPT_OK) return err;
if (sober128_read(out, 500, &st) != 500) return CRYPT_ERROR_READPRNG; 
if (sober128_read(out, 10, &st) != 10) return CRYPT_ERROR_READPRNG; 
if (compare_testvector(out, 10, t3, sizeof(t3), "SOBER128-PRNG", 3)) return CRYPT_FAIL_TESTVECTOR;
if ((err = sober128_done(&st)) != CRYPT_OK) return err;

return CRYPT_OK;
#endif
}

#endif




