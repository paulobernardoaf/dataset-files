







#include "tomcrypt_private.h"






#if defined(LTC_DER)


static int _ltc_to_asn1(ltc_asn1_type v)
{
return der_asn1_type_to_identifier_map[v];
}


static int _qsort_helper(const void *a, const void *b)
{
ltc_asn1_list *A = (ltc_asn1_list *)a, *B = (ltc_asn1_list *)b;
int r;

r = _ltc_to_asn1(A->type) - _ltc_to_asn1(B->type);


if (r == 0) {

return A->used - B->used;
}
return r;
}









int der_encode_set(const ltc_asn1_list *list, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
ltc_asn1_list *copy;
unsigned long x;
int err;


copy = XCALLOC(inlen, sizeof(*copy));
if (copy == NULL) {
return CRYPT_MEM;
}


for (x = 0; x < inlen; x++) {
copy[x] = list[x];
copy[x].used = x;
}


XQSORT(copy, inlen, sizeof(*copy), &_qsort_helper);


err = der_encode_sequence_ex(copy, inlen, out, outlen, LTC_ASN1_SET);


XFREE(copy);

return err;
}


#endif




