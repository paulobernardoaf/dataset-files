







#include "tomcrypt_private.h"





#if defined(LTC_PKCS_5)






















static int _pkcs_5_alg1_common(const unsigned char *password,
unsigned long password_len,
const unsigned char *salt,
int iteration_count, int hash_idx,
unsigned char *out, unsigned long *outlen,
int openssl_compat)
{
int err;
unsigned long x;
hash_state *md;
unsigned char *buf;

unsigned long block = 0, iter;

unsigned long outidx = 0, nb = 0;

LTC_ARGCHK(password != NULL);
LTC_ARGCHK(salt != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


if ((err = hash_is_valid(hash_idx)) != CRYPT_OK) {
return err;
}


md = XMALLOC(sizeof(hash_state));
buf = XMALLOC(MAXBLOCKSIZE);
if (md == NULL || buf == NULL) {
if (md != NULL) {
XFREE(md);
}
if (buf != NULL) {
XFREE(buf);
}
return CRYPT_MEM;
}

while(block * hash_descriptor[hash_idx].hashsize < *outlen) {


if ((err = hash_descriptor[hash_idx].init(md)) != CRYPT_OK) {
goto LBL_ERR;
}

if (openssl_compat && block) {
if ((err = hash_descriptor[hash_idx].process(md, buf, hash_descriptor[hash_idx].hashsize)) != CRYPT_OK) {
goto LBL_ERR;
}
}
if ((err = hash_descriptor[hash_idx].process(md, password, password_len)) != CRYPT_OK) {
goto LBL_ERR;
}
if ((err = hash_descriptor[hash_idx].process(md, salt, 8)) != CRYPT_OK) {
goto LBL_ERR;
}
if ((err = hash_descriptor[hash_idx].done(md, buf)) != CRYPT_OK) {
goto LBL_ERR;
}

iter = iteration_count;
while (--iter) {

x = MAXBLOCKSIZE;
if ((err = hash_memory(hash_idx, buf, hash_descriptor[hash_idx].hashsize, buf, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
}



outidx = block*hash_descriptor[hash_idx].hashsize;
nb = hash_descriptor[hash_idx].hashsize;
if(outidx+nb > *outlen) {
nb = *outlen - outidx;
}
if(nb > 0) {
XMEMCPY(out+outidx, buf, nb);
}

block++;
if (!openssl_compat) {
break;
}
}


if(!openssl_compat) {
*outlen = hash_descriptor[hash_idx].hashsize;
}

err = CRYPT_OK;
LBL_ERR:
#if defined(LTC_CLEAN_STACK)
zeromem(buf, MAXBLOCKSIZE);
zeromem(md, sizeof(hash_state));
#endif

XFREE(buf);
XFREE(md);

return err;
}












int pkcs_5_alg1(const unsigned char *password, unsigned long password_len,
const unsigned char *salt,
int iteration_count, int hash_idx,
unsigned char *out, unsigned long *outlen)
{
return _pkcs_5_alg1_common(password, password_len, salt, iteration_count,
hash_idx, out, outlen, 0);
}















int pkcs_5_alg1_openssl(const unsigned char *password,
unsigned long password_len,
const unsigned char *salt,
int iteration_count, int hash_idx,
unsigned char *out, unsigned long *outlen)
{
return _pkcs_5_alg1_common(password, password_len, salt, iteration_count,
hash_idx, out, outlen, 1);
}

#endif




