








#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "tomcrypt_private.h"

#if defined(LTC_HKDF)


int hkdf_extract(int hash_idx, const unsigned char *salt, unsigned long saltlen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{








if (salt == NULL || saltlen == 0) {
return hmac_memory(hash_idx, (const unsigned char *)"", 1, in, inlen, out, outlen);
}
return hmac_memory(hash_idx, salt, saltlen, in, inlen, out, outlen);
}

int hkdf_expand(int hash_idx, const unsigned char *info, unsigned long infolen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long outlen)
{
unsigned long hashsize;
int err;
unsigned char N;
unsigned long Noutlen, outoff;

unsigned char *T, *dat;
unsigned long Tlen, datlen;


if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
return err;
}

hashsize = hash_descriptor[hash_idx].hashsize;


if (inlen < hashsize || outlen > hashsize * 255) {
return CRYPT_INVALID_ARG;
}
if (info == NULL && infolen != 0) {
return CRYPT_INVALID_ARG;
}
LTC_ARGCHK(out != NULL);

Tlen = hashsize + infolen + 1;
T = XMALLOC(Tlen); 
if (T == NULL) {
return CRYPT_MEM;
}
if (info != NULL) {
XMEMCPY(T + hashsize, info, infolen);
}


dat = T + hashsize;
datlen = Tlen - hashsize;

N = 0;
outoff = 0; 
while (1) { 
Noutlen = MIN(hashsize, outlen - outoff);
T[Tlen - 1] = ++N;
if ((err = hmac_memory(hash_idx, in, inlen, dat, datlen,
out + outoff, &Noutlen)) != CRYPT_OK) {
zeromem(T, Tlen);
XFREE(T);
return err;
}
outoff += Noutlen;

if (outoff >= outlen) { 
break;
}


XMEMCPY(T, out + hashsize * (N-1), hashsize);
if (N == 1) {
dat = T;
datlen = Tlen;
}
}
zeromem(T, Tlen);
XFREE(T);
return CRYPT_OK;
}


int hkdf(int hash_idx, const unsigned char *salt, unsigned long saltlen,
const unsigned char *info, unsigned long infolen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long outlen)
{
unsigned long hashsize;
int err;
unsigned char *extracted;


if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
return err;
}

hashsize = hash_descriptor[hash_idx].hashsize;

extracted = XMALLOC(hashsize); 
if (extracted == NULL) {
return CRYPT_MEM;
}
if ((err = hkdf_extract(hash_idx, salt, saltlen, in, inlen, extracted, &hashsize)) != 0) {
zeromem(extracted, hashsize);
XFREE(extracted);
return err;
}
err = hkdf_expand(hash_idx, info, infolen, extracted, hashsize, out, outlen);
zeromem(extracted, hashsize);
XFREE(extracted);
return err;
}
#endif 







