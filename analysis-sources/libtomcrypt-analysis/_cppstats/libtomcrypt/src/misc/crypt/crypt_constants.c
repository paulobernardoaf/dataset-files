#include "tomcrypt_private.h"
typedef struct {
const char *name;
const int value;
} crypt_constant;
#define _C_STRINGIFY(s) { #s, s }
static const crypt_constant _crypt_constants[] = {
_C_STRINGIFY(CRYPT_OK),
_C_STRINGIFY(CRYPT_ERROR),
_C_STRINGIFY(CRYPT_NOP),
_C_STRINGIFY(CRYPT_INVALID_KEYSIZE),
_C_STRINGIFY(CRYPT_INVALID_ROUNDS),
_C_STRINGIFY(CRYPT_FAIL_TESTVECTOR),
_C_STRINGIFY(CRYPT_BUFFER_OVERFLOW),
_C_STRINGIFY(CRYPT_INVALID_PACKET),
_C_STRINGIFY(CRYPT_INVALID_PRNGSIZE),
_C_STRINGIFY(CRYPT_ERROR_READPRNG),
_C_STRINGIFY(CRYPT_INVALID_CIPHER),
_C_STRINGIFY(CRYPT_INVALID_HASH),
_C_STRINGIFY(CRYPT_INVALID_PRNG),
_C_STRINGIFY(CRYPT_MEM),
_C_STRINGIFY(CRYPT_PK_TYPE_MISMATCH),
_C_STRINGIFY(CRYPT_PK_NOT_PRIVATE),
_C_STRINGIFY(CRYPT_INVALID_ARG),
_C_STRINGIFY(CRYPT_FILE_NOTFOUND),
_C_STRINGIFY(CRYPT_PK_INVALID_TYPE),
_C_STRINGIFY(CRYPT_OVERFLOW),
_C_STRINGIFY(CRYPT_PK_ASN1_ERROR),
_C_STRINGIFY(CRYPT_INPUT_TOO_LONG),
_C_STRINGIFY(CRYPT_PK_INVALID_SIZE),
_C_STRINGIFY(CRYPT_INVALID_PRIME_SIZE),
_C_STRINGIFY(CRYPT_PK_INVALID_PADDING),
_C_STRINGIFY(CRYPT_HASH_OVERFLOW),
_C_STRINGIFY(PK_PUBLIC),
_C_STRINGIFY(PK_PRIVATE),
_C_STRINGIFY(LTC_ENCRYPT),
_C_STRINGIFY(LTC_DECRYPT),
#if defined(LTC_PKCS_1)
{"LTC_PKCS_1", 1},
_C_STRINGIFY(LTC_PKCS_1_EMSA),
_C_STRINGIFY(LTC_PKCS_1_EME),
_C_STRINGIFY(LTC_PKCS_1_V1_5),
_C_STRINGIFY(LTC_PKCS_1_OAEP),
_C_STRINGIFY(LTC_PKCS_1_PSS),
_C_STRINGIFY(LTC_PKCS_1_V1_5_NA1),
#else
{"LTC_PKCS_1", 0},
#endif
#if defined(LTC_PADDING)
{"LTC_PADDING", 1},
_C_STRINGIFY(LTC_PAD_PKCS7),
#if defined(LTC_RNG_GET_BYTES)
_C_STRINGIFY(LTC_PAD_ISO_10126),
#endif
_C_STRINGIFY(LTC_PAD_ANSI_X923),
_C_STRINGIFY(LTC_PAD_ONE_AND_ZERO),
_C_STRINGIFY(LTC_PAD_ZERO),
_C_STRINGIFY(LTC_PAD_ZERO_ALWAYS),
#else
{"LTC_PADDING", 0},
#endif
#if defined(LTC_MRSA)
{"LTC_MRSA", 1},
#else
{"LTC_MRSA", 0},
#endif
#if defined(LTC_MECC)
{"LTC_MECC", 1},
_C_STRINGIFY(ECC_BUF_SIZE),
_C_STRINGIFY(ECC_MAXSIZE),
#else
{"LTC_MECC", 0},
#endif
#if defined(LTC_MDSA)
{"LTC_MDSA", 1},
_C_STRINGIFY(LTC_MDSA_DELTA),
_C_STRINGIFY(LTC_MDSA_MAX_GROUP),
#else
{"LTC_MDSA", 0},
#endif
#if defined(LTC_MILLER_RABIN_REPS)
_C_STRINGIFY(LTC_MILLER_RABIN_REPS),
#endif
#if defined(LTC_DER)
{"LTC_DER", 1},
_C_STRINGIFY(LTC_ASN1_EOL),
_C_STRINGIFY(LTC_ASN1_BOOLEAN),
_C_STRINGIFY(LTC_ASN1_INTEGER),
_C_STRINGIFY(LTC_ASN1_SHORT_INTEGER),
_C_STRINGIFY(LTC_ASN1_BIT_STRING),
_C_STRINGIFY(LTC_ASN1_OCTET_STRING),
_C_STRINGIFY(LTC_ASN1_NULL),
_C_STRINGIFY(LTC_ASN1_OBJECT_IDENTIFIER),
_C_STRINGIFY(LTC_ASN1_IA5_STRING),
_C_STRINGIFY(LTC_ASN1_PRINTABLE_STRING),
_C_STRINGIFY(LTC_ASN1_UTF8_STRING),
_C_STRINGIFY(LTC_ASN1_UTCTIME),
_C_STRINGIFY(LTC_ASN1_CHOICE),
_C_STRINGIFY(LTC_ASN1_SEQUENCE),
_C_STRINGIFY(LTC_ASN1_SET),
_C_STRINGIFY(LTC_ASN1_SETOF),
_C_STRINGIFY(LTC_ASN1_RAW_BIT_STRING),
_C_STRINGIFY(LTC_ASN1_TELETEX_STRING),
_C_STRINGIFY(LTC_ASN1_GENERALIZEDTIME),
_C_STRINGIFY(LTC_ASN1_CUSTOM_TYPE),
_C_STRINGIFY(LTC_DER_MAX_RECURSION),
#else
{"LTC_DER", 0},
#endif
#if defined(LTC_CTR_MODE)
{"LTC_CTR_MODE", 1},
_C_STRINGIFY(CTR_COUNTER_LITTLE_ENDIAN),
_C_STRINGIFY(CTR_COUNTER_BIG_ENDIAN),
_C_STRINGIFY(LTC_CTR_RFC3686),
#else
{"LTC_CTR_MODE", 0},
#endif
#if defined(LTC_GCM_MODE)
_C_STRINGIFY(LTC_GCM_MODE_IV),
_C_STRINGIFY(LTC_GCM_MODE_AAD),
_C_STRINGIFY(LTC_GCM_MODE_TEXT),
#endif
_C_STRINGIFY(LTC_MP_LT),
_C_STRINGIFY(LTC_MP_EQ),
_C_STRINGIFY(LTC_MP_GT),
_C_STRINGIFY(LTC_MP_NO),
_C_STRINGIFY(LTC_MP_YES),
_C_STRINGIFY(MAXBLOCKSIZE),
_C_STRINGIFY(TAB_SIZE),
_C_STRINGIFY(ARGTYPE),
#if defined(LTM_DESC)
{"LTM_DESC", 1},
#else
{"LTM_DESC", 0},
#endif
#if defined(TFM_DESC)
{"TFM_DESC", 1},
#else
{"TFM_DESC", 0},
#endif
#if defined(GMP_DESC)
{"GMP_DESC", 1},
#else
{"GMP_DESC", 0},
#endif
#if defined(LTC_FAST)
{"LTC_FAST", 1},
#else
{"LTC_FAST", 0},
#endif
#if defined(LTC_NO_FILE)
{"LTC_NO_FILE", 1},
#else
{"LTC_NO_FILE", 0},
#endif
#if defined(ENDIAN_LITTLE)
{"ENDIAN_LITTLE", 1},
#else
{"ENDIAN_LITTLE", 0},
#endif
#if defined(ENDIAN_BIG)
{"ENDIAN_BIG", 1},
#else
{"ENDIAN_BIG", 0},
#endif
#if defined(ENDIAN_32BITWORD)
{"ENDIAN_32BITWORD", 1},
#else
{"ENDIAN_32BITWORD", 0},
#endif
#if defined(ENDIAN_64BITWORD)
{"ENDIAN_64BITWORD", 1},
#else
{"ENDIAN_64BITWORD", 0},
#endif
#if defined(ENDIAN_NEUTRAL)
{"ENDIAN_NEUTRAL", 1},
#else
{"ENDIAN_NEUTRAL", 0},
#endif
};
int crypt_get_constant(const char* namein, int *valueout) {
int i;
int _crypt_constants_len = sizeof(_crypt_constants) / sizeof(_crypt_constants[0]);
for (i=0; i<_crypt_constants_len; i++) {
if (XSTRCMP(_crypt_constants[i].name, namein) == 0) {
*valueout = _crypt_constants[i].value;
return 0;
}
}
return 1;
}
int crypt_list_all_constants(char *names_list, unsigned int *names_list_size) {
int i;
unsigned int total_len = 0;
char *ptr;
int number_len;
int count = sizeof(_crypt_constants) / sizeof(_crypt_constants[0]);
for (i=0; i<count; i++) {
number_len = snprintf(NULL, 0, "%s,%d\n", _crypt_constants[i].name, _crypt_constants[i].value);
if (number_len < 0) {
return -1;
}
total_len += number_len;
}
if (names_list == NULL) {
*names_list_size = total_len;
} else {
if (total_len > *names_list_size) {
return -1;
}
ptr = names_list;
for (i=0; i<count; i++) {
number_len = snprintf(ptr, total_len, "%s,%d\n", _crypt_constants[i].name, _crypt_constants[i].value);
if (number_len < 0) return -1;
if ((unsigned int)number_len > total_len) return -1;
total_len -= number_len;
ptr += number_len;
}
ptr -= 1;
*ptr = 0;
}
return 0;
}
