#include "tomcrypt_private.h"
#if defined(LTC_MRSA)
int rsa_import_pkcs8(const unsigned char *in, unsigned long inlen,
const void *passwd, unsigned long passwdlen,
rsa_key *key)
{
int err;
void *zero, *iter;
unsigned char *buf1 = NULL, *buf2 = NULL;
unsigned long buf1len, buf2len;
unsigned long oid[16];
const char *rsaoid;
ltc_asn1_list alg_seq[2], top_seq[3];
ltc_asn1_list *l = NULL;
unsigned char *decrypted = NULL;
unsigned long decryptedlen;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(ltc_mp.name != NULL);
err = pk_get_oid(PKA_RSA, &rsaoid);
if (err != CRYPT_OK) { goto LBL_NOFREE; }
buf1len = inlen; 
buf1 = XMALLOC(buf1len);
if (buf1 == NULL) { err = CRYPT_MEM; goto LBL_NOFREE; }
buf2len = inlen; 
buf2 = XMALLOC(buf2len);
if (buf2 == NULL) { err = CRYPT_MEM; goto LBL_FREE1; }
if ((err = mp_init_multi(&zero, &iter, NULL)) != CRYPT_OK) { goto LBL_FREE2; }
if ((err = rsa_init(key)) != CRYPT_OK) { goto LBL_FREE3; }
if ((err = pkcs8_decode_flexi(in, inlen, passwd, passwdlen, &l)) != CRYPT_OK) {
goto LBL_ERR;
}
decrypted = l->data;
decryptedlen = l->size;
LTC_SET_ASN1(alg_seq, 0, LTC_ASN1_OBJECT_IDENTIFIER, oid, 16UL);
LTC_SET_ASN1(alg_seq, 1, LTC_ASN1_NULL, NULL, 0UL);
LTC_SET_ASN1(top_seq, 0, LTC_ASN1_INTEGER, zero, 1UL);
LTC_SET_ASN1(top_seq, 1, LTC_ASN1_SEQUENCE, alg_seq, 2UL);
LTC_SET_ASN1(top_seq, 2, LTC_ASN1_OCTET_STRING, buf1, buf1len);
err=der_decode_sequence(decrypted, decryptedlen, top_seq, 3UL);
if (err != CRYPT_OK) { goto LBL_ERR; }
if ((err = pk_oid_cmp_with_asn1(rsaoid, &alg_seq[0])) != CRYPT_OK) {
goto LBL_ERR;
}
err = der_decode_sequence_multi(buf1, top_seq[2].size,
LTC_ASN1_INTEGER, 1UL, zero,
LTC_ASN1_INTEGER, 1UL, key->N,
LTC_ASN1_INTEGER, 1UL, key->e,
LTC_ASN1_INTEGER, 1UL, key->d,
LTC_ASN1_INTEGER, 1UL, key->p,
LTC_ASN1_INTEGER, 1UL, key->q,
LTC_ASN1_INTEGER, 1UL, key->dP,
LTC_ASN1_INTEGER, 1UL, key->dQ,
LTC_ASN1_INTEGER, 1UL, key->qP,
LTC_ASN1_EOL, 0UL, NULL);
if (err != CRYPT_OK) { goto LBL_ERR; }
key->type = PK_PRIVATE;
err = CRYPT_OK;
goto LBL_FREE3;
LBL_ERR:
rsa_free(key);
LBL_FREE3:
mp_clear_multi(iter, zero, NULL);
if (l) der_free_sequence_flexi(l);
LBL_FREE2:
XFREE(buf2);
LBL_FREE1:
XFREE(buf1);
LBL_NOFREE:
return err;
}
#endif 
