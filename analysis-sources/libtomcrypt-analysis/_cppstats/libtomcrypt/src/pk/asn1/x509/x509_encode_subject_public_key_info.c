#include "tomcrypt_private.h"
#if defined(LTC_DER)
int x509_encode_subject_public_key_info(unsigned char *out, unsigned long *outlen,
unsigned int algorithm, const void* public_key, unsigned long public_key_len,
ltc_asn1_type parameters_type, ltc_asn1_list* parameters, unsigned long parameters_len)
{
int err;
ltc_asn1_list alg_id[2];
const char *OID;
unsigned long oid[16], oidlen;
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
if ((err = pk_get_oid(algorithm, &OID)) != CRYPT_OK) {
return err;
}
oidlen = sizeof(oid)/sizeof(oid[0]);
if ((err = pk_oid_str_to_num(OID, oid, &oidlen)) != CRYPT_OK) {
return err;
}
LTC_SET_ASN1(alg_id, 0, LTC_ASN1_OBJECT_IDENTIFIER, oid, oidlen);
LTC_SET_ASN1(alg_id, 1, parameters_type, parameters, parameters_len);
return der_encode_sequence_multi(out, outlen,
LTC_ASN1_SEQUENCE, (unsigned long)sizeof(alg_id)/sizeof(alg_id[0]), alg_id,
LTC_ASN1_RAW_BIT_STRING, public_key_len*8U, public_key,
LTC_ASN1_EOL, 0UL, NULL);
}
#endif
