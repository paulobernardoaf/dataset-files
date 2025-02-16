#include "tomcrypt_private.h"
#if defined(LTC_MDSA)
int dsa_import(const unsigned char *in, unsigned long inlen, dsa_key *key)
{
int err, stat;
unsigned long zero = 0, len;
unsigned char* tmpbuf = NULL;
unsigned char flags[1];
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);
if (mp_init_multi(&key->p, &key->g, &key->q, &key->x, &key->y, NULL) != CRYPT_OK) {
return CRYPT_MEM;
}
err = der_decode_sequence_multi(in, inlen, LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_EOL, 0UL, NULL);
if (err == CRYPT_OK || err == CRYPT_INPUT_TOO_LONG) {
if (flags[0] == 1) {
if ((err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_INTEGER, 1UL, key->g,
LTC_ASN1_INTEGER, 1UL, key->p,
LTC_ASN1_INTEGER, 1UL, key->q,
LTC_ASN1_INTEGER, 1UL, key->y,
LTC_ASN1_INTEGER, 1UL, key->x,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto LBL_ERR;
}
key->type = PK_PRIVATE;
goto LBL_OK;
}
else if (flags[0] == 0) {
if ((err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_BIT_STRING, 1UL, flags,
LTC_ASN1_INTEGER, 1UL, key->g,
LTC_ASN1_INTEGER, 1UL, key->p,
LTC_ASN1_INTEGER, 1UL, key->q,
LTC_ASN1_INTEGER, 1UL, key->y,
LTC_ASN1_EOL, 0UL, NULL)) != CRYPT_OK) {
goto LBL_ERR;
}
key->type = PK_PUBLIC;
goto LBL_OK;
}
else {
err = CRYPT_INVALID_PACKET;
goto LBL_ERR;
}
}
if ((err = der_decode_sequence_multi(in, inlen,
LTC_ASN1_SHORT_INTEGER, 1UL, &zero,
LTC_ASN1_INTEGER, 1UL, key->p,
LTC_ASN1_INTEGER, 1UL, key->q,
LTC_ASN1_INTEGER, 1UL, key->g,
LTC_ASN1_INTEGER, 1UL, key->y,
LTC_ASN1_INTEGER, 1UL, key->x,
LTC_ASN1_EOL, 0UL, NULL)) == CRYPT_OK) {
key->type = PK_PRIVATE;
} else { 
ltc_asn1_list params[3];
unsigned long tmpbuf_len = inlen;
LTC_SET_ASN1(params, 0, LTC_ASN1_INTEGER, key->p, 1UL);
LTC_SET_ASN1(params, 1, LTC_ASN1_INTEGER, key->q, 1UL);
LTC_SET_ASN1(params, 2, LTC_ASN1_INTEGER, key->g, 1UL);
tmpbuf = XCALLOC(1, tmpbuf_len);
if (tmpbuf == NULL) {
err = CRYPT_MEM;
goto LBL_ERR;
}
len = 3;
err = x509_decode_subject_public_key_info(in, inlen, PKA_DSA,
tmpbuf, &tmpbuf_len,
LTC_ASN1_SEQUENCE, params, &len);
if (err != CRYPT_OK) {
XFREE(tmpbuf);
goto LBL_ERR;
}
if ((err=der_decode_integer(tmpbuf, tmpbuf_len, key->y)) != CRYPT_OK) {
XFREE(tmpbuf);
goto LBL_ERR;
}
XFREE(tmpbuf);
key->type = PK_PUBLIC;
}
LBL_OK:
key->qord = mp_unsigned_bin_size(key->q);
if ((err = dsa_int_validate_pqg(key, &stat)) != CRYPT_OK) {
goto LBL_ERR;
}
if (stat == 0) {
err = CRYPT_INVALID_PACKET;
goto LBL_ERR;
}
if ((err = dsa_int_validate_xy(key, &stat)) != CRYPT_OK) {
goto LBL_ERR;
}
if (stat == 0) {
err = CRYPT_INVALID_PACKET;
goto LBL_ERR;
}
return CRYPT_OK;
LBL_ERR:
dsa_free(key);
return err;
}
#endif
