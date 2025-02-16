#include "tomcrypt_private.h"
#if defined(LTC_DER)
void der_sequence_shrink(ltc_asn1_list *in)
{
if (!in) return;
while (in != NULL) {
if (in->child) {
der_sequence_shrink(in->child);
}
switch (in->type) {
case LTC_ASN1_CUSTOM_TYPE:
case LTC_ASN1_SET:
case LTC_ASN1_SEQUENCE : if (in->data != NULL) { XFREE(in->data); in->data = NULL; } break;
default: break;
}
in = in->next;
}
}
#endif
