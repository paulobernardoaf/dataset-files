








#include "tomcrypt_private.h"

#if defined(LTC_MDH)









int dh_shared_secret(const dh_key *private_key, const dh_key *public_key,
unsigned char *out, unsigned long *outlen)
{
void *tmp;
unsigned long x;
int err;

LTC_ARGCHK(private_key != NULL);
LTC_ARGCHK(public_key != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);


if (private_key->type != PK_PRIVATE) {
return CRYPT_PK_NOT_PRIVATE;
}


if (mp_cmp(private_key->prime, public_key->prime) != LTC_MP_EQ) { return CRYPT_PK_TYPE_MISMATCH; }
if (mp_cmp(private_key->base, public_key->base) != LTC_MP_EQ) { return CRYPT_PK_TYPE_MISMATCH; }


if ((err = mp_init(&tmp)) != CRYPT_OK) {
return err;
}


if ((err = dh_check_pubkey(public_key)) != CRYPT_OK) {
goto error;
}


if ((err = mp_exptmod(public_key->y, private_key->x, private_key->prime, tmp)) != CRYPT_OK) {
goto error;
}


x = (unsigned long)mp_unsigned_bin_size(tmp);
if (*outlen < x) {
*outlen = x;
err = CRYPT_BUFFER_OVERFLOW;
goto error;
}
if ((err = mp_to_unsigned_bin(tmp, out)) != CRYPT_OK) {
goto error;
}
*outlen = x;
err = CRYPT_OK;

error:
mp_clear(tmp);
return err;
}

#endif 




