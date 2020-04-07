







#include "tomcrypt_private.h"

#if defined(LTC_PBES)

static const char * const _oid_pbes2 = "1.2.840.113549.1.5.13";
static const char * const _oid_pbkdf2 = "1.2.840.113549.1.5.12";

typedef struct {
const char *oid;
const char *id;
} oid_id_st;

static const oid_id_st _hmac_oid_names[] = {
{ "1.2.840.113549.2.7", "sha1" },
{ "1.2.840.113549.2.8", "sha224" },
{ "1.2.840.113549.2.9", "sha256" },
{ "1.2.840.113549.2.10", "sha384" },
{ "1.2.840.113549.2.11", "sha512" },
{ "1.2.840.113549.2.12", "sha512-224" },
{ "1.2.840.113549.2.13", "sha512-256" },
};

static const pbes_properties _pbes2_default_types[] = {
{ pkcs_5_alg2, "sha1", "des", 8, 0 },
{ pkcs_5_alg2, "sha1", "rc2", 4, 0 },
{ pkcs_5_alg2, "sha1", "3des", 24, 0 },
{ pkcs_5_alg2, "sha1", "aes", 16, 0 },
{ pkcs_5_alg2, "sha1", "aes", 24, 0 },
{ pkcs_5_alg2, "sha1", "aes", 32, 0 },
};

typedef struct {
const pbes_properties *data;
const char* oid;
} oid_to_pbes;

static const oid_to_pbes _pbes2_list[] = {
{ &_pbes2_default_types[0], "1.3.14.3.2.7" }, 
{ &_pbes2_default_types[1], "1.2.840.113549.3.2" }, 
{ &_pbes2_default_types[2], "1.2.840.113549.3.7" }, 
{ &_pbes2_default_types[3], "2.16.840.1.101.3.4.1.2" }, 
{ &_pbes2_default_types[4], "2.16.840.1.101.3.4.1.22" }, 
{ &_pbes2_default_types[5], "2.16.840.1.101.3.4.1.42" }, 
};

static int _pbes2_from_oid(const ltc_asn1_list *cipher_oid, const ltc_asn1_list *hmac_oid, pbes_properties *res)
{
unsigned int i;
for (i = 0; i < sizeof(_pbes2_list)/sizeof(_pbes2_list[0]); ++i) {
if (pk_oid_cmp_with_asn1(_pbes2_list[i].oid, cipher_oid) == CRYPT_OK) {
*res = *_pbes2_list[i].data;
break;
}
}
if (res->c == NULL) return CRYPT_INVALID_CIPHER;
if (hmac_oid != NULL) {
for (i = 0; i < sizeof(_hmac_oid_names)/sizeof(_hmac_oid_names[0]); ++i) {
if (pk_oid_cmp_with_asn1(_hmac_oid_names[i].oid, hmac_oid) == CRYPT_OK) {
res->h = _hmac_oid_names[i].id;
return CRYPT_OK;
}
}
return CRYPT_INVALID_HASH;
}
return CRYPT_OK;
}









int pbes2_extract(const ltc_asn1_list *s, pbes_arg *res)
{
unsigned long klen;
ltc_asn1_list *lkdf, *lenc, *loptseq, *liter, *lhmac;
int err;

LTC_ARGCHK(s != NULL);
LTC_ARGCHK(res != NULL);

if ((err = pk_oid_cmp_with_asn1(_oid_pbes2, s)) != CRYPT_OK) return err;

if (!LTC_ASN1_IS_TYPE(s->next, LTC_ASN1_SEQUENCE) ||
!LTC_ASN1_IS_TYPE(s->next->child, LTC_ASN1_SEQUENCE) ||
!LTC_ASN1_IS_TYPE(s->next->child->child, LTC_ASN1_OBJECT_IDENTIFIER) ||
!LTC_ASN1_IS_TYPE(s->next->child->child->next, LTC_ASN1_SEQUENCE) ||
!LTC_ASN1_IS_TYPE(s->next->child->next, LTC_ASN1_SEQUENCE) ||
!LTC_ASN1_IS_TYPE(s->next->child->next->child, LTC_ASN1_OBJECT_IDENTIFIER)) {
return CRYPT_INVALID_PACKET;
}


















lkdf = s->next->child->child;
lenc = s->next->child->next->child;

if ((err = pk_oid_cmp_with_asn1(_oid_pbkdf2, lkdf)) != CRYPT_OK) return err;

if (!LTC_ASN1_IS_TYPE(lkdf->next, LTC_ASN1_SEQUENCE) ||
!LTC_ASN1_IS_TYPE(lkdf->next->child, LTC_ASN1_OCTET_STRING) ||
!LTC_ASN1_IS_TYPE(lkdf->next->child->next, LTC_ASN1_INTEGER)) {
return CRYPT_INVALID_PACKET;
}

liter = lkdf->next->child->next;
loptseq = liter->next;
res->salt = lkdf->next->child;
res->iterations = mp_get_int(liter->data);



if(LTC_ASN1_IS_TYPE(loptseq, LTC_ASN1_INTEGER)) {
loptseq = loptseq->next;
}


lhmac = NULL;
if (LTC_ASN1_IS_TYPE(loptseq, LTC_ASN1_SEQUENCE) &&
LTC_ASN1_IS_TYPE(loptseq->child, LTC_ASN1_OBJECT_IDENTIFIER)) {
lhmac = loptseq->child;
}
if ((err = _pbes2_from_oid(lenc, lhmac, &res->type)) != CRYPT_OK) return err;

if (LTC_ASN1_IS_TYPE(lenc->next, LTC_ASN1_OCTET_STRING)) {

res->iv = lenc->next;
} else if (LTC_ASN1_IS_TYPE(lenc->next, LTC_ASN1_SEQUENCE)) {






if (LTC_ASN1_IS_TYPE(lenc->next->child, LTC_ASN1_INTEGER) &&
LTC_ASN1_IS_TYPE(lenc->next->child->next, LTC_ASN1_OCTET_STRING)) {
klen = mp_get_int(lenc->next->child->data);
res->iv = lenc->next->child->next;







switch (klen) {
case 160:
res->key_bits = 40;
break;
case 120:
res->key_bits = 64;
break;
case 58:
res->key_bits = 128;
break;
default:

if (klen < 256) return CRYPT_INVALID_KEYSIZE;

res->key_bits = klen;
break;
}
} else if (LTC_ASN1_IS_TYPE(lenc->next->child, LTC_ASN1_OCTET_STRING)) {
res->iv = lenc->next->child;




res->key_bits = 32;
} else {
return CRYPT_INVALID_PACKET;
}
}

return CRYPT_OK;
}

#endif




