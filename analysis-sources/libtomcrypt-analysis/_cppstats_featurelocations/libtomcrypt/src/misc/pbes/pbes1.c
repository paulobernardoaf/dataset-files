







#include "tomcrypt_private.h"

#if defined(LTC_PBES)

static int _pkcs_5_alg1_wrap(const unsigned char *password, unsigned long password_len,
const unsigned char *salt, unsigned long salt_len,
int iteration_count, int hash_idx,
unsigned char *out, unsigned long *outlen)
{
LTC_UNUSED_PARAM(salt_len);
return pkcs_5_alg1(password, password_len, salt, iteration_count, hash_idx, out, outlen);
}

static int _pkcs_12_wrap(const unsigned char *password, unsigned long password_len,
const unsigned char *salt, unsigned long salt_len,
int iteration_count, int hash_idx,
unsigned char *out, unsigned long *outlen)
{
int err;

unsigned long pwlen = password_len * 2;
unsigned char* pw;
if (*outlen < 32) return CRYPT_INVALID_ARG;
pw = XMALLOC(pwlen + 2);
if (pw == NULL) return CRYPT_MEM;
if ((err = pkcs12_utf8_to_utf16(password, password_len, pw, &pwlen)) != CRYPT_OK) goto LBL_ERROR;
pw[pwlen++] = 0;
pw[pwlen++] = 0;

if ((err = pkcs12_kdf(hash_idx, pw, pwlen, salt, salt_len, iteration_count, 1, out, 24)) != CRYPT_OK) goto LBL_ERROR;

if ((err = pkcs12_kdf(hash_idx, pw, pwlen, salt, salt_len, iteration_count, 2, out+24, 8)) != CRYPT_OK) goto LBL_ERROR;

*outlen = 32;
LBL_ERROR:
zeromem(pw, pwlen);
XFREE(pw);
return err;
}

static const pbes_properties _pbes1_types[] = {
{ _pkcs_5_alg1_wrap, "md2", "des", 8, 8 },
{ _pkcs_5_alg1_wrap, "md2", "rc2", 8, 8 },
{ _pkcs_5_alg1_wrap, "md5", "des", 8, 8 },
{ _pkcs_5_alg1_wrap, "md5", "rc2", 8, 8 },
{ _pkcs_5_alg1_wrap, "sha1", "des", 8, 8 },
{ _pkcs_5_alg1_wrap, "sha1", "rc2", 8, 8 },
{ _pkcs_12_wrap, "sha1", "3des", 24, 8 },
};

typedef struct {
const pbes_properties *data;
const char *oid;
} oid_to_pbes;

static const oid_to_pbes _pbes1_list[] = {
{ &_pbes1_types[0], "1.2.840.113549.1.5.1" }, 
{ &_pbes1_types[1], "1.2.840.113549.1.5.4" }, 
{ &_pbes1_types[2], "1.2.840.113549.1.5.3" }, 
{ &_pbes1_types[3], "1.2.840.113549.1.5.6" }, 
{ &_pbes1_types[4], "1.2.840.113549.1.5.10" }, 
{ &_pbes1_types[5], "1.2.840.113549.1.5.11" }, 
{ &_pbes1_types[6], "1.2.840.113549.1.12.1.3" }, 
{ 0 },
};

static int _pbes1_from_oid(const ltc_asn1_list *oid, pbes_properties *res)
{
unsigned int i;
for (i = 0; _pbes1_list[i].data != NULL; ++i) {
if (pk_oid_cmp_with_asn1(_pbes1_list[i].oid, oid) == CRYPT_OK) {
if (res != NULL) *res = *_pbes1_list[i].data;
return CRYPT_OK;
}
}
return CRYPT_INVALID_ARG;
}








int pbes1_extract(const ltc_asn1_list *s, pbes_arg *res)
{
int err;

LTC_ARGCHK(s != NULL);
LTC_ARGCHK(res != NULL);

if ((err = _pbes1_from_oid(s, &res->type)) != CRYPT_OK) return err;

if (!LTC_ASN1_IS_TYPE(s->next, LTC_ASN1_SEQUENCE) ||
!LTC_ASN1_IS_TYPE(s->next->child, LTC_ASN1_OCTET_STRING) ||
!LTC_ASN1_IS_TYPE(s->next->child->next, LTC_ASN1_INTEGER)) {
return CRYPT_INVALID_PACKET;
}









res->salt = s->next->child;
res->iterations = mp_get_int(s->next->child->next->data);

return CRYPT_OK;
}

#endif




