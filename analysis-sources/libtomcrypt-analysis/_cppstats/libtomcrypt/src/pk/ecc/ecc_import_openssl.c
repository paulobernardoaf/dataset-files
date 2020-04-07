#include "tomcrypt_private.h"
#if defined(LTC_MECC)
static int _ecc_import_private_with_oid(const unsigned char *in, unsigned long inlen, ecc_key *key)
{
ltc_asn1_list seq_priv[4], custom[2];
unsigned char bin_xy[2*ECC_MAXSIZE+2], bin_k[ECC_MAXSIZE];
unsigned long len, pkver = 0, curveoid[16];
char OID[256];
const ltc_ecc_curve *curve;
int err;
LTC_SET_ASN1(custom, 0, LTC_ASN1_OBJECT_IDENTIFIER, curveoid, 16UL);
LTC_SET_ASN1(custom, 1, LTC_ASN1_RAW_BIT_STRING, bin_xy, 8UL*sizeof(bin_xy));
LTC_SET_ASN1(seq_priv, 0, LTC_ASN1_SHORT_INTEGER, &pkver, 1UL);
LTC_SET_ASN1(seq_priv, 1, LTC_ASN1_OCTET_STRING, bin_k, sizeof(bin_k));
LTC_SET_ASN1_CUSTOM_CONSTRUCTED(seq_priv, 2, LTC_ASN1_CL_CONTEXT_SPECIFIC, 0, custom); 
LTC_SET_ASN1_CUSTOM_CONSTRUCTED(seq_priv, 3, LTC_ASN1_CL_CONTEXT_SPECIFIC, 1, custom + 1); 
err = der_decode_sequence(in, inlen, seq_priv, 4);
if (err == CRYPT_OK) {
len = sizeof(OID);
if ((err = pk_oid_num_to_str(curveoid, custom[0].size, OID, &len)) != CRYPT_OK) { goto error; }
if ((err = ecc_find_curve(OID, &curve)) != CRYPT_OK) { goto error; }
if ((err = ecc_set_curve(curve, key)) != CRYPT_OK) { goto error; }
err = ecc_set_key(bin_k, seq_priv[1].size, PK_PRIVATE, key);
}
error:
return err;
}
static int _ecc_import_private_with_curve(const unsigned char *in, unsigned long inlen, ecc_key *key)
{
void *prime, *order, *a, *b, *gx, *gy;
ltc_asn1_list seq_fieldid[2], seq_curve[3], seq_ecparams[6], seq_priv[4], custom[2];
unsigned char bin_a[ECC_MAXSIZE], bin_b[ECC_MAXSIZE], bin_k[ECC_MAXSIZE];
unsigned char bin_g[2*ECC_MAXSIZE+1], bin_xy[2*ECC_MAXSIZE+2], bin_seed[128];
unsigned long len_a, len_b, len_k, len_g;
unsigned long cofactor = 0, ecver = 0, pkver = 0, tmpoid[16];
int err;
if ((err = mp_init_multi(&prime, &order, &a, &b, &gx, &gy, NULL)) != CRYPT_OK) {
return err;
}
LTC_SET_ASN1(custom, 0, LTC_ASN1_SEQUENCE, seq_ecparams, 6UL);
LTC_SET_ASN1(custom, 1, LTC_ASN1_RAW_BIT_STRING, bin_xy, 8UL*sizeof(bin_xy));
LTC_SET_ASN1(seq_priv, 0, LTC_ASN1_SHORT_INTEGER, &pkver, 1UL);
LTC_SET_ASN1(seq_priv, 1, LTC_ASN1_OCTET_STRING, bin_k, sizeof(bin_k));
LTC_SET_ASN1_CUSTOM_CONSTRUCTED(seq_priv, 2, LTC_ASN1_CL_CONTEXT_SPECIFIC, 0, custom); 
LTC_SET_ASN1_CUSTOM_CONSTRUCTED(seq_priv, 3, LTC_ASN1_CL_CONTEXT_SPECIFIC, 1, custom + 1); 
LTC_SET_ASN1(seq_ecparams, 0, LTC_ASN1_SHORT_INTEGER, &ecver, 1UL);
LTC_SET_ASN1(seq_ecparams, 1, LTC_ASN1_SEQUENCE, seq_fieldid, 2UL);
LTC_SET_ASN1(seq_ecparams, 2, LTC_ASN1_SEQUENCE, seq_curve, 3UL);
LTC_SET_ASN1(seq_ecparams, 3, LTC_ASN1_OCTET_STRING, bin_g, sizeof(bin_g));
LTC_SET_ASN1(seq_ecparams, 4, LTC_ASN1_INTEGER, order, 1UL);
LTC_SET_ASN1(seq_ecparams, 5, LTC_ASN1_SHORT_INTEGER, &cofactor, 1UL);
seq_ecparams[5].optional = 1;
LTC_SET_ASN1(seq_fieldid, 0, LTC_ASN1_OBJECT_IDENTIFIER, tmpoid, 16UL);
LTC_SET_ASN1(seq_fieldid, 1, LTC_ASN1_INTEGER, prime, 1UL);
LTC_SET_ASN1(seq_curve, 0, LTC_ASN1_OCTET_STRING, bin_a, sizeof(bin_a));
LTC_SET_ASN1(seq_curve, 1, LTC_ASN1_OCTET_STRING, bin_b, sizeof(bin_b));
LTC_SET_ASN1(seq_curve, 2, LTC_ASN1_RAW_BIT_STRING, bin_seed, 8UL*sizeof(bin_seed));
seq_curve[2].optional = 1;
err = der_decode_sequence(in, inlen, seq_priv, 4);
if (err == CRYPT_OK) {
len_k = seq_priv[1].size;
len_a = seq_curve[0].size;
len_b = seq_curve[1].size;
len_g = seq_ecparams[3].size;
if ((err = mp_read_unsigned_bin(a, bin_a, len_a)) != CRYPT_OK) { goto error; }
if ((err = mp_read_unsigned_bin(b, bin_b, len_b)) != CRYPT_OK) { goto error; }
if ((err = ltc_ecc_import_point(bin_g, len_g, prime, a, b, gx, gy)) != CRYPT_OK) { goto error; }
if ((err = ecc_set_curve_from_mpis(a, b, prime, order, gx, gy, cofactor, key)) != CRYPT_OK) { goto error; }
err = ecc_set_key(bin_k, len_k, PK_PRIVATE, key);
}
error:
mp_clear_multi(prime, order, a, b, gx, gy, NULL);
return err;
}
int ecc_import_openssl(const unsigned char *in, unsigned long inlen, ecc_key *key)
{
int err;
if ((err = ecc_import_subject_public_key_info(in, inlen, key)) == CRYPT_OK) {
goto success;
}
if ((err = _ecc_import_private_with_oid(in, inlen, key)) == CRYPT_OK) {
goto success;
}
err = _ecc_import_private_with_curve(in, inlen, key);
success:
return err;
}
#endif
