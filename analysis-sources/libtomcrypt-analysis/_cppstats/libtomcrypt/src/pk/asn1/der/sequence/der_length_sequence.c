#include "tomcrypt_private.h"
#if defined(LTC_DER)
int der_length_sequence(const ltc_asn1_list *list, unsigned long inlen,
unsigned long *outlen)
{
return der_length_sequence_ex(list, inlen, outlen, NULL);
}
int der_length_sequence_ex(const ltc_asn1_list *list, unsigned long inlen,
unsigned long *outlen, unsigned long *payloadlen)
{
int err;
ltc_asn1_type type;
unsigned long size, x, y, i;
void *data;
LTC_ARGCHK(list != NULL);
LTC_ARGCHK(outlen != NULL);
y = 0;
for (i = 0; i < inlen; i++) {
type = list[i].type;
size = list[i].size;
data = list[i].data;
if (type == LTC_ASN1_EOL) {
break;
}
if (!list[i].used && list[i].optional) continue;
switch (type) {
case LTC_ASN1_BOOLEAN:
if ((err = der_length_boolean(&x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_INTEGER:
if ((err = der_length_integer(data, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_SHORT_INTEGER:
if ((err = der_length_short_integer(*((unsigned long *)data), &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_BIT_STRING:
case LTC_ASN1_RAW_BIT_STRING:
if ((err = der_length_bit_string(size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_OCTET_STRING:
if ((err = der_length_octet_string(size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_NULL:
y += 2;
break;
case LTC_ASN1_OBJECT_IDENTIFIER:
if ((err = der_length_object_identifier(data, size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_IA5_STRING:
if ((err = der_length_ia5_string(data, size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_TELETEX_STRING:
if ((err = der_length_teletex_string(data, size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_PRINTABLE_STRING:
if ((err = der_length_printable_string(data, size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_UTCTIME:
if ((err = der_length_utctime(data, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_GENERALIZEDTIME:
if ((err = der_length_generalizedtime(data, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_UTF8_STRING:
if ((err = der_length_utf8_string(data, size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_CUSTOM_TYPE:
if ((err = der_length_custom_type(&list[i], &x, NULL)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_SET:
case LTC_ASN1_SETOF:
case LTC_ASN1_SEQUENCE:
if ((err = der_length_sequence(data, size, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
y += x;
break;
case LTC_ASN1_CHOICE:
case LTC_ASN1_EOL:
err = CRYPT_INVALID_ARG;
goto LBL_ERR;
}
}
if ((err = der_length_asn1_length(y, &x)) != CRYPT_OK) {
goto LBL_ERR;
}
if (payloadlen != NULL) {
*payloadlen = y;
}
*outlen = y + x + 1;
err = CRYPT_OK;
LBL_ERR:
return err;
}
#endif
