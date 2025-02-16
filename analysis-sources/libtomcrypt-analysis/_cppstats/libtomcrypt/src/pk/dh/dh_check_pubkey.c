#include "tomcrypt_private.h"
#if defined(LTC_MDH)
int dh_check_pubkey(const dh_key *key)
{
void *p_minus1;
ltc_mp_digit digit;
int i, digit_count, bits_set = 0, err;
LTC_ARGCHK(key != NULL);
if ((err = mp_init(&p_minus1)) != CRYPT_OK) {
return err;
}
if ((err = mp_sub_d(key->prime, 1, p_minus1)) != CRYPT_OK) {
goto error;
}
if (mp_cmp(key->y, p_minus1) != LTC_MP_LT || mp_cmp_d(key->y, 1) != LTC_MP_GT) {
err = CRYPT_INVALID_ARG;
goto error;
}
digit_count = mp_get_digit_count(key->y);
for (i = 0; i < digit_count && bits_set < 2; i++) {
digit = mp_get_digit(key->y, i);
while (digit > 0) {
if (digit & 1) bits_set++;
digit >>= 1;
}
}
if (bits_set > 1) {
err = CRYPT_OK;
}
else {
err = CRYPT_INVALID_ARG;
}
error:
mp_clear(p_minus1);
return err;
}
#endif 
