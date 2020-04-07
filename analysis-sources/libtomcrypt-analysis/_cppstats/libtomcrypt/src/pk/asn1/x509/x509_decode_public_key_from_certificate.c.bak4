







#include "tomcrypt_private.h"






#if defined(LTC_DER)


#define LOOKS_LIKE_SPKI(l) ((l) != NULL) && ((l)->type == LTC_ASN1_SEQUENCE) && ((l)->child != NULL) && ((l)->child->type == LTC_ASN1_OBJECT_IDENTIFIER) && ((l)->next != NULL) && ((l)->next->type == LTC_ASN1_BIT_STRING)


















int x509_decode_public_key_from_certificate(const unsigned char *in, unsigned long inlen,
enum ltc_oid_id algorithm, ltc_asn1_type param_type,
ltc_asn1_list* parameters, unsigned long *parameters_len,
public_key_decode_cb callback, void *ctx)
{
int err;
unsigned char *tmpbuf;
unsigned long tmpbuf_len, tmp_inlen;
ltc_asn1_list *decoded_list = NULL, *l;

LTC_ARGCHK(in != NULL);
LTC_ARGCHK(inlen != 0);

tmpbuf_len = inlen;
tmpbuf = XCALLOC(1, tmpbuf_len);
if (tmpbuf == NULL) {
err = CRYPT_MEM;
goto LBL_OUT;
}

tmp_inlen = inlen;
if ((err = der_decode_sequence_flexi(in, &tmp_inlen, &decoded_list)) == CRYPT_OK) {
l = decoded_list;

err = CRYPT_NOP;






if ((l->type == LTC_ASN1_SEQUENCE) && (l->child != NULL)) {
l = l->child;
if ((l->type == LTC_ASN1_SEQUENCE) && (l->child != NULL)) {
l = l->child;









do {



if ((l->type == LTC_ASN1_SEQUENCE)
&& (l->data != NULL)
&& LOOKS_LIKE_SPKI(l->child)) {
if (algorithm == PKA_EC) {
err = ecc_import_subject_public_key_info(l->data, l->size, ctx);
} else {
err = x509_decode_subject_public_key_info(l->data, l->size,
algorithm, tmpbuf, &tmpbuf_len,
param_type, parameters, parameters_len);
if (err == CRYPT_OK) {
err = callback(tmpbuf, tmpbuf_len, ctx);
goto LBL_OUT;
}
}
}
l = l->next;
} while(l);
}
}
}

LBL_OUT:
if (decoded_list) der_free_sequence_flexi(decoded_list);
if (tmpbuf != NULL) XFREE(tmpbuf);

return err;
}

#endif




