







#include "tomcrypt_private.h"






#if defined(LTC_CURVE25519)










int ec25519_import_pkcs8(const unsigned char *in, unsigned long inlen,
const void *pwd, unsigned long pwdlen,
enum ltc_oid_id id, sk_to_pk fp,
curve25519_key *key)
{
int err;
ltc_asn1_list *l = NULL;
const char *oid;
ltc_asn1_list alg_id[1];
unsigned char private_key[34];
unsigned long version, key_len;
unsigned long tmpoid[16];

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(fp != NULL);

if ((err = pkcs8_decode_flexi(in, inlen, pwd, pwdlen, &l)) == CRYPT_OK) {

LTC_SET_ASN1(alg_id, 0, LTC_ASN1_OBJECT_IDENTIFIER, tmpoid, sizeof(tmpoid) / sizeof(tmpoid[0]));

key_len = sizeof(private_key);
if ((err = der_decode_sequence_multi(l->data, l->size,
LTC_ASN1_SHORT_INTEGER, 1uL, &version,
LTC_ASN1_SEQUENCE, 1uL, alg_id,
LTC_ASN1_OCTET_STRING, key_len, private_key,
LTC_ASN1_EOL, 0uL, NULL))
!= CRYPT_OK) {




if ((err == CRYPT_INPUT_TOO_LONG) && (version == 1)) {
version = 0;
} else {
goto out;
}
}

if ((err = pk_get_oid(id, &oid)) != CRYPT_OK) {
goto out;
}
if ((err = pk_oid_cmp_with_asn1(oid, &alg_id[0])) != CRYPT_OK) {
goto out;
}

if (version == 0) {
key_len = sizeof(key->priv);
if ((err = der_decode_octet_string(private_key, sizeof(private_key), key->priv, &key_len)) == CRYPT_OK) {
fp(key->pub, key->priv);
key->type = PK_PRIVATE;
key->algo = id;
}
} else {
err = CRYPT_PK_INVALID_TYPE;
}
}
out:
if (l) der_free_sequence_flexi(l);
#if defined(LTC_CLEAN_STACK)
zeromem(private_key, sizeof(private_key));
#endif

return err;
}

#endif




