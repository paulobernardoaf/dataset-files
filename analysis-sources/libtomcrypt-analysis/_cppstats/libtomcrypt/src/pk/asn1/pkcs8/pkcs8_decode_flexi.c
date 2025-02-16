#include "tomcrypt_private.h"
#if defined(LTC_PKCS_8)
int pkcs8_decode_flexi(const unsigned char *in, unsigned long inlen,
const void *pwd, unsigned long pwdlen,
ltc_asn1_list **decoded_list)
{
unsigned long len = inlen;
unsigned long dec_size;
unsigned char *dec_data = NULL;
ltc_asn1_list *l = NULL;
int err;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(decoded_list != NULL);
*decoded_list = NULL;
if ((err = der_decode_sequence_flexi(in, &len, &l)) == CRYPT_OK) {
if (l->type == LTC_ASN1_SEQUENCE &&
LTC_ASN1_IS_TYPE(l->child, LTC_ASN1_SEQUENCE) &&
LTC_ASN1_IS_TYPE(l->child->child, LTC_ASN1_OBJECT_IDENTIFIER) &&
LTC_ASN1_IS_TYPE(l->child->child->next, LTC_ASN1_SEQUENCE) &&
LTC_ASN1_IS_TYPE(l->child->next, LTC_ASN1_OCTET_STRING)) {
ltc_asn1_list *lalgoid = l->child->child;
pbes_arg pbes;
XMEMSET(&pbes, 0, sizeof(pbes));
if (pbes1_extract(lalgoid, &pbes) == CRYPT_OK) {
} else if (pbes2_extract(lalgoid, &pbes) == CRYPT_OK) {
} else {
err = CRYPT_INVALID_PACKET;
goto LBL_DONE;
}
pbes.enc_data = l->child->next;
pbes.pwd = pwd;
pbes.pwdlen = pwdlen;
dec_size = pbes.enc_data->size;
if ((dec_data = XMALLOC(dec_size)) == NULL) {
err = CRYPT_MEM;
goto LBL_DONE;
}
if ((err = pbes_decrypt(&pbes, dec_data, &dec_size)) != CRYPT_OK) goto LBL_DONE;
der_free_sequence_flexi(l);
l = NULL;
err = der_decode_sequence_flexi(dec_data, &dec_size, &l);
if (err != CRYPT_OK) goto LBL_DONE;
*decoded_list = l;
}
else {
err = CRYPT_OK;
*decoded_list = l;
}
l = NULL;
}
LBL_DONE:
if (l) der_free_sequence_flexi(l);
if (dec_data) {
zeromem(dec_data, dec_size);
XFREE(dec_data);
}
return err;
}
#endif
