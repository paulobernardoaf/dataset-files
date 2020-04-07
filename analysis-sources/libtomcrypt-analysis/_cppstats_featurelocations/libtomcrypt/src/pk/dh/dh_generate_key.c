








#include "tomcrypt_private.h"

#if defined(LTC_MDH)

static int _dh_groupsize_to_keysize(int groupsize)
{



if (groupsize <= 0) {
return 0;
}
if (groupsize <= 192) {
return 30; 
}
if (groupsize <= 256) {
return 40; 
}
if (groupsize <= 384) {
return 52; 
}
if (groupsize <= 512) {
return 60; 
}
if (groupsize <= 768) {
return 67; 
}
if (groupsize <= 1024) {
return 77; 
}
return 0;
}

int dh_generate_key(prng_state *prng, int wprng, dh_key *key)
{
unsigned char *buf;
unsigned long keysize;
int err, max_iterations = LTC_PK_MAX_RETRIES;

LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);


if ((err = prng_is_valid(wprng)) != CRYPT_OK) {
return err;
}

keysize = _dh_groupsize_to_keysize(mp_unsigned_bin_size(key->prime));
if (keysize == 0) {
err = CRYPT_INVALID_KEYSIZE;
goto freemp;
}


buf = XMALLOC(keysize);
if (buf == NULL) {
err = CRYPT_MEM;
goto freemp;
}

key->type = PK_PRIVATE;
do {

if (prng_descriptor[wprng].read(buf, keysize, prng) != keysize) {
err = CRYPT_ERROR_READPRNG;
goto freebuf;
}

if ((err = mp_read_unsigned_bin(key->x, buf, keysize)) != CRYPT_OK) {
goto freebuf;
}

if ((err = mp_exptmod(key->base, key->x, key->prime, key->y)) != CRYPT_OK) {
goto freebuf;
}
err = dh_check_pubkey(key);
} while (err != CRYPT_OK && max_iterations-- > 0);

freebuf:
zeromem(buf, keysize);
XFREE(buf);
freemp:
if (err != CRYPT_OK) dh_free(key);
return err;
}

#endif 




