








#include "tomcrypt_private.h"

#if defined(LTC_CHACHA20POLY1305_MODE)








int chacha20poly1305_setiv(chacha20poly1305_state *st, const unsigned char *iv, unsigned long ivlen)
{
chacha_state tmp_st;
int i, err;
unsigned char polykey[32];

LTC_ARGCHK(st != NULL);
LTC_ARGCHK(iv != NULL);
LTC_ARGCHK(ivlen == 12 || ivlen == 8);


if (ivlen == 12) {

if ((err = chacha_ivctr32(&st->chacha, iv, ivlen, 1)) != CRYPT_OK) return err;
}
else {

if ((err = chacha_ivctr64(&st->chacha, iv, ivlen, 1)) != CRYPT_OK) return err;
}


for(i = 0; i < 12; i++) tmp_st.input[i] = st->chacha.input[i];
tmp_st.rounds = 20;

if (ivlen == 12) {

if ((err = chacha_ivctr32(&tmp_st, iv, ivlen, 0)) != CRYPT_OK) return err;
}
else {

if ((err = chacha_ivctr64(&tmp_st, iv, ivlen, 0)) != CRYPT_OK) return err;
}

if ((err = chacha_keystream(&tmp_st, polykey, 32)) != CRYPT_OK) return err;

if ((err = poly1305_init(&st->poly, polykey, 32)) != CRYPT_OK) return err;
st->ctlen = 0;
st->aadlen = 0;
st->aadflg = 1;

return CRYPT_OK;
}

#endif




