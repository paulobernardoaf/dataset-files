










enum public_key_type {

PK_PUBLIC = 0x0000,

PK_PRIVATE = 0x0001,


PK_STD = 0x1000,

PK_COMPRESSED = 0x2000,

PK_CURVEOID = 0x4000
};

int rand_prime(void *N, long len, prng_state *prng, int wprng);


#if defined(LTC_MRSA)


typedef struct Rsa_key {

int type;

void *e;

void *d;

void *N;

void *p;

void *q;

void *qP;

void *dP;

void *dQ;
} rsa_key;

int rsa_make_key(prng_state *prng, int wprng, int size, long e, rsa_key *key);

int rsa_get_size(const rsa_key *key);

int rsa_exptmod(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen, int which,
const rsa_key *key);

void rsa_free(rsa_key *key);


#define rsa_encrypt_key(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _prng, _prng_idx, _hash_idx, _key) rsa_encrypt_key_ex(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _prng, _prng_idx, _hash_idx, LTC_PKCS_1_OAEP, _key)


#define rsa_decrypt_key(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _hash_idx, _stat, _key) rsa_decrypt_key_ex(_in, _inlen, _out, _outlen, _lparam, _lparamlen, _hash_idx, LTC_PKCS_1_OAEP, _stat, _key)


#define rsa_sign_hash(_in, _inlen, _out, _outlen, _prng, _prng_idx, _hash_idx, _saltlen, _key) rsa_sign_hash_ex(_in, _inlen, _out, _outlen, LTC_PKCS_1_PSS, _prng, _prng_idx, _hash_idx, _saltlen, _key)


#define rsa_verify_hash(_sig, _siglen, _hash, _hashlen, _hash_idx, _saltlen, _stat, _key) rsa_verify_hash_ex(_sig, _siglen, _hash, _hashlen, LTC_PKCS_1_PSS, _hash_idx, _saltlen, _stat, _key)


#define rsa_sign_saltlen_get_max(_hash_idx, _key) rsa_sign_saltlen_get_max_ex(LTC_PKCS_1_PSS, _hash_idx, _key)



int rsa_encrypt_key_ex(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
const unsigned char *lparam, unsigned long lparamlen,
prng_state *prng, int prng_idx,
int hash_idx, int padding,
const rsa_key *key);

int rsa_decrypt_key_ex(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
const unsigned char *lparam, unsigned long lparamlen,
int hash_idx, int padding,
int *stat, const rsa_key *key);

int rsa_sign_hash_ex(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
int padding,
prng_state *prng, int prng_idx,
int hash_idx, unsigned long saltlen,
const rsa_key *key);

int rsa_verify_hash_ex(const unsigned char *sig, unsigned long siglen,
const unsigned char *hash, unsigned long hashlen,
int padding,
int hash_idx, unsigned long saltlen,
int *stat, const rsa_key *key);

int rsa_sign_saltlen_get_max_ex(int padding, int hash_idx, const rsa_key *key);


int rsa_export(unsigned char *out, unsigned long *outlen, int type, const rsa_key *key);
int rsa_import(const unsigned char *in, unsigned long inlen, rsa_key *key);

int rsa_import_x509(const unsigned char *in, unsigned long inlen, rsa_key *key);
int rsa_import_pkcs8(const unsigned char *in, unsigned long inlen,
const void *passwd, unsigned long passwdlen, rsa_key *key);

int rsa_set_key(const unsigned char *N, unsigned long Nlen,
const unsigned char *e, unsigned long elen,
const unsigned char *d, unsigned long dlen,
rsa_key *key);
int rsa_set_factors(const unsigned char *p, unsigned long plen,
const unsigned char *q, unsigned long qlen,
rsa_key *key);
int rsa_set_crt_params(const unsigned char *dP, unsigned long dPlen,
const unsigned char *dQ, unsigned long dQlen,
const unsigned char *qP, unsigned long qPlen,
rsa_key *key);
#endif


#if defined(LTC_MDH)

typedef struct {
int type;
void *x;
void *y;
void *base;
void *prime;
} dh_key;

int dh_get_groupsize(const dh_key *key);

int dh_export(unsigned char *out, unsigned long *outlen, int type, const dh_key *key);
int dh_import(const unsigned char *in, unsigned long inlen, dh_key *key);

int dh_set_pg(const unsigned char *p, unsigned long plen,
const unsigned char *g, unsigned long glen,
dh_key *key);
int dh_set_pg_dhparam(const unsigned char *dhparam, unsigned long dhparamlen, dh_key *key);
int dh_set_pg_groupsize(int groupsize, dh_key *key);

int dh_set_key(const unsigned char *in, unsigned long inlen, int type, dh_key *key);
int dh_generate_key(prng_state *prng, int wprng, dh_key *key);

int dh_shared_secret(const dh_key *private_key, const dh_key *public_key,
unsigned char *out, unsigned long *outlen);

void dh_free(dh_key *key);

int dh_export_key(void *out, unsigned long *outlen, int type, const dh_key *key);
#endif 



#if defined(LTC_MECC)


#define ECC_BUF_SIZE 256


#define ECC_MAXSIZE 66


typedef struct {

const char *prime;


const char *A;


const char *B;


const char *order;


const char *Gx;


const char *Gy;


unsigned long cofactor;


const char *OID;
} ltc_ecc_curve;


typedef struct {

void *x;


void *y;


void *z;
} ecc_point;


typedef struct {

int size;

void *prime;

void *A;

void *B;

void *order;

ecc_point base;

unsigned long cofactor;

unsigned long oid[16];
unsigned long oidlen;
} ltc_ecc_dp;


typedef struct {

int type;


ltc_ecc_dp dp;


ecc_point pubkey;


void *k;
} ecc_key;


typedef enum ecc_signature_type_ {

LTC_ECCSIG_ANSIX962 = 0x0,

LTC_ECCSIG_RFC7518 = 0x1,

LTC_ECCSIG_ETH27 = 0x2,

LTC_ECCSIG_RFC5656 = 0x3,
} ecc_signature_type;


extern const ltc_ecc_curve ltc_ecc_curves[];

void ecc_sizes(int *low, int *high);
int ecc_get_size(const ecc_key *key);

int ecc_find_curve(const char* name_or_oid, const ltc_ecc_curve** cu);
int ecc_set_curve(const ltc_ecc_curve *cu, ecc_key *key);
int ecc_generate_key(prng_state *prng, int wprng, ecc_key *key);
int ecc_set_key(const unsigned char *in, unsigned long inlen, int type, ecc_key *key);
int ecc_get_key(unsigned char *out, unsigned long *outlen, int type, const ecc_key *key);
int ecc_get_oid_str(char *out, unsigned long *outlen, const ecc_key *key);

int ecc_make_key(prng_state *prng, int wprng, int keysize, ecc_key *key);
int ecc_make_key_ex(prng_state *prng, int wprng, ecc_key *key, const ltc_ecc_curve *cu);
void ecc_free(ecc_key *key);

int ecc_export(unsigned char *out, unsigned long *outlen, int type, const ecc_key *key);
int ecc_import(const unsigned char *in, unsigned long inlen, ecc_key *key);
int ecc_import_ex(const unsigned char *in, unsigned long inlen, ecc_key *key, const ltc_ecc_curve *cu);

int ecc_ansi_x963_export(const ecc_key *key, unsigned char *out, unsigned long *outlen);
int ecc_ansi_x963_import(const unsigned char *in, unsigned long inlen, ecc_key *key);
int ecc_ansi_x963_import_ex(const unsigned char *in, unsigned long inlen, ecc_key *key, const ltc_ecc_curve *cu);

int ecc_export_openssl(unsigned char *out, unsigned long *outlen, int type, const ecc_key *key);
int ecc_import_openssl(const unsigned char *in, unsigned long inlen, ecc_key *key);
int ecc_import_pkcs8(const unsigned char *in, unsigned long inlen, const void *pwd, unsigned long pwdlen, ecc_key *key);
int ecc_import_x509(const unsigned char *in, unsigned long inlen, ecc_key *key);

int ecc_shared_secret(const ecc_key *private_key, const ecc_key *public_key,
unsigned char *out, unsigned long *outlen);

int ecc_encrypt_key(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
prng_state *prng, int wprng, int hash,
const ecc_key *key);

int ecc_decrypt_key(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
const ecc_key *key);

#define ecc_sign_hash_rfc7518(in_, inlen_, out_, outlen_, prng_, wprng_, key_) ecc_sign_hash_ex(in_, inlen_, out_, outlen_, prng_, wprng_, LTC_ECCSIG_RFC7518, NULL, key_)


#define ecc_sign_hash(in_, inlen_, out_, outlen_, prng_, wprng_, key_) ecc_sign_hash_ex(in_, inlen_, out_, outlen_, prng_, wprng_, LTC_ECCSIG_ANSIX962, NULL, key_)


#define ecc_verify_hash_rfc7518(sig_, siglen_, hash_, hashlen_, stat_, key_) ecc_verify_hash_ex(sig_, siglen_, hash_, hashlen_, LTC_ECCSIG_RFC7518, stat_, key_)


#define ecc_verify_hash(sig_, siglen_, hash_, hashlen_, stat_, key_) ecc_verify_hash_ex(sig_, siglen_, hash_, hashlen_, LTC_ECCSIG_ANSIX962, stat_, key_)


int ecc_sign_hash_ex(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
prng_state *prng, int wprng, ecc_signature_type sigformat,
int *recid, const ecc_key *key);

int ecc_verify_hash_ex(const unsigned char *sig, unsigned long siglen,
const unsigned char *hash, unsigned long hashlen,
ecc_signature_type sigformat, int *stat, const ecc_key *key);

int ecc_recover_key(const unsigned char *sig, unsigned long siglen,
const unsigned char *hash, unsigned long hashlen,
int recid, ecc_signature_type sigformat, ecc_key *key);

#endif

#if defined(LTC_CURVE25519)

typedef struct {

enum public_key_type type;






int algo;


unsigned char priv[32];


unsigned char pub[32];
} curve25519_key;



int ed25519_make_key(prng_state *prng, int wprng, curve25519_key *key);

int ed25519_export( unsigned char *out, unsigned long *outlen,
int which,
const curve25519_key *key);

int ed25519_import(const unsigned char *in, unsigned long inlen, curve25519_key *key);
int ed25519_import_raw(const unsigned char *in, unsigned long inlen, int which, curve25519_key *key);
int ed25519_import_x509(const unsigned char *in, unsigned long inlen, curve25519_key *key);
int ed25519_import_pkcs8(const unsigned char *in, unsigned long inlen,
const void *pwd, unsigned long pwdlen,
curve25519_key *key);

int ed25519_sign(const unsigned char *msg, unsigned long msglen,
unsigned char *sig, unsigned long *siglen,
const curve25519_key *private_key);

int ed25519_verify(const unsigned char *msg, unsigned long msglen,
const unsigned char *sig, unsigned long siglen,
int *stat, const curve25519_key *public_key);


int x25519_make_key(prng_state *prng, int wprng, curve25519_key *key);

int x25519_export( unsigned char *out, unsigned long *outlen,
int which,
const curve25519_key *key);

int x25519_import(const unsigned char *in, unsigned long inlen, curve25519_key *key);
int x25519_import_raw(const unsigned char *in, unsigned long inlen, int which, curve25519_key *key);
int x25519_import_x509(const unsigned char *in, unsigned long inlen, curve25519_key *key);
int x25519_import_pkcs8(const unsigned char *in, unsigned long inlen,
const void *pwd, unsigned long pwdlen,
curve25519_key *key);

int x25519_shared_secret(const curve25519_key *private_key,
const curve25519_key *public_key,
unsigned char *out, unsigned long *outlen);

#endif 

#if defined(LTC_MDSA)


#define LTC_MDSA_DELTA 512


#define LTC_MDSA_MAX_GROUP 512


typedef struct {

int type;


int qord;


void *g;


void *q;


void *p;


void *x;


void *y;
} dsa_key;

int dsa_make_key(prng_state *prng, int wprng, int group_size, int modulus_size, dsa_key *key);

int dsa_set_pqg(const unsigned char *p, unsigned long plen,
const unsigned char *q, unsigned long qlen,
const unsigned char *g, unsigned long glen,
dsa_key *key);
int dsa_set_pqg_dsaparam(const unsigned char *dsaparam, unsigned long dsaparamlen, dsa_key *key);
int dsa_generate_pqg(prng_state *prng, int wprng, int group_size, int modulus_size, dsa_key *key);

int dsa_set_key(const unsigned char *in, unsigned long inlen, int type, dsa_key *key);
int dsa_generate_key(prng_state *prng, int wprng, dsa_key *key);

void dsa_free(dsa_key *key);

int dsa_sign_hash_raw(const unsigned char *in, unsigned long inlen,
void *r, void *s,
prng_state *prng, int wprng, const dsa_key *key);

int dsa_sign_hash(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
prng_state *prng, int wprng, const dsa_key *key);

int dsa_verify_hash_raw( void *r, void *s,
const unsigned char *hash, unsigned long hashlen,
int *stat, const dsa_key *key);

int dsa_verify_hash(const unsigned char *sig, unsigned long siglen,
const unsigned char *hash, unsigned long hashlen,
int *stat, const dsa_key *key);

int dsa_encrypt_key(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
prng_state *prng, int wprng, int hash,
const dsa_key *key);

int dsa_decrypt_key(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
const dsa_key *key);

int dsa_import(const unsigned char *in, unsigned long inlen, dsa_key *key);
int dsa_export(unsigned char *out, unsigned long *outlen, int type, const dsa_key *key);
int dsa_verify_key(const dsa_key *key, int *stat);
int dsa_shared_secret(void *private_key, void *base,
const dsa_key *public_key,
unsigned char *out, unsigned long *outlen);
#endif 

#if defined(LTC_DER)


typedef enum ltc_asn1_type_ {

LTC_ASN1_EOL,
LTC_ASN1_BOOLEAN,
LTC_ASN1_INTEGER,
LTC_ASN1_SHORT_INTEGER,
LTC_ASN1_BIT_STRING,

LTC_ASN1_OCTET_STRING,
LTC_ASN1_NULL,
LTC_ASN1_OBJECT_IDENTIFIER,
LTC_ASN1_IA5_STRING,
LTC_ASN1_PRINTABLE_STRING,

LTC_ASN1_UTF8_STRING,
LTC_ASN1_UTCTIME,
LTC_ASN1_CHOICE,
LTC_ASN1_SEQUENCE,
LTC_ASN1_SET,

LTC_ASN1_SETOF,
LTC_ASN1_RAW_BIT_STRING,
LTC_ASN1_TELETEX_STRING,
LTC_ASN1_GENERALIZEDTIME,
LTC_ASN1_CUSTOM_TYPE,
} ltc_asn1_type;

typedef enum {
LTC_ASN1_CL_UNIVERSAL = 0x0,
LTC_ASN1_CL_APPLICATION = 0x1,
LTC_ASN1_CL_CONTEXT_SPECIFIC = 0x2,
LTC_ASN1_CL_PRIVATE = 0x3,
} ltc_asn1_class;

typedef enum {
LTC_ASN1_PC_PRIMITIVE = 0x0,
LTC_ASN1_PC_CONSTRUCTED = 0x1,
} ltc_asn1_pc;


typedef struct ltc_asn1_list_ {

ltc_asn1_type type;

void *data;

unsigned long size;




int used;

int optional;

ltc_asn1_class klass;
ltc_asn1_pc pc;
ulong64 tag;

struct ltc_asn1_list_ *prev, *next, *child, *parent;
} ltc_asn1_list;

#define LTC_SET_ASN1(list, index, Type, Data, Size) do { int LTC_MACRO_temp = (index); ltc_asn1_list *LTC_MACRO_list = (list); LTC_MACRO_list[LTC_MACRO_temp].type = (Type); LTC_MACRO_list[LTC_MACRO_temp].data = (void*)(Data); LTC_MACRO_list[LTC_MACRO_temp].size = (Size); LTC_MACRO_list[LTC_MACRO_temp].used = 0; LTC_MACRO_list[LTC_MACRO_temp].optional = 0; LTC_MACRO_list[LTC_MACRO_temp].klass = 0; LTC_MACRO_list[LTC_MACRO_temp].pc = 0; LTC_MACRO_list[LTC_MACRO_temp].tag = 0; } while (0)













#define __LTC_SET_ASN1_IDENTIFIER(list, index, Class, Pc, Tag) do { int LTC_MACRO_temp = (index); ltc_asn1_list *LTC_MACRO_list = (list); LTC_MACRO_list[LTC_MACRO_temp].type = LTC_ASN1_CUSTOM_TYPE; LTC_MACRO_list[LTC_MACRO_temp].klass = (Class); LTC_MACRO_list[LTC_MACRO_temp].pc = (Pc); LTC_MACRO_list[LTC_MACRO_temp].tag = (Tag); } while (0)









#define LTC_SET_ASN1_CUSTOM_CONSTRUCTED(list, index, Class, Tag, Data) do { int LTC_MACRO_temp##__LINE__ = (index); LTC_SET_ASN1(list, LTC_MACRO_temp##__LINE__, LTC_ASN1_CUSTOM_TYPE, Data, 1); __LTC_SET_ASN1_IDENTIFIER(list, LTC_MACRO_temp##__LINE__, Class, LTC_ASN1_PC_CONSTRUCTED, Tag); } while (0)






#define LTC_SET_ASN1_CUSTOM_PRIMITIVE(list, index, Class, Tag, Type, Data, Size) do { int LTC_MACRO_temp##__LINE__ = (index); LTC_SET_ASN1(list, LTC_MACRO_temp##__LINE__, LTC_ASN1_CUSTOM_TYPE, Data, Size); __LTC_SET_ASN1_IDENTIFIER(list, LTC_MACRO_temp##__LINE__, Class, LTC_ASN1_PC_PRIMITIVE, Tag); list[LTC_MACRO_temp##__LINE__].used = (int)(Type); } while (0)







extern const char* der_asn1_class_to_string_map[];
extern const unsigned long der_asn1_class_to_string_map_sz;

extern const char* der_asn1_pc_to_string_map[];
extern const unsigned long der_asn1_pc_to_string_map_sz;

extern const char* der_asn1_tag_to_string_map[];
extern const unsigned long der_asn1_tag_to_string_map_sz;


int der_encode_sequence_ex(const ltc_asn1_list *list, unsigned long inlen,
unsigned char *out, unsigned long *outlen, int type_of);

#define der_encode_sequence(list, inlen, out, outlen) der_encode_sequence_ex(list, inlen, out, outlen, LTC_ASN1_SEQUENCE)




enum ltc_der_seq {
LTC_DER_SEQ_ZERO = 0x0u,



LTC_DER_SEQ_UNORDERED = LTC_DER_SEQ_ZERO,
LTC_DER_SEQ_ORDERED = 0x1u,



LTC_DER_SEQ_RELAXED = LTC_DER_SEQ_ZERO,
LTC_DER_SEQ_STRICT = 0x2u,


LTC_DER_SEQ_SET = LTC_DER_SEQ_UNORDERED,
LTC_DER_SEQ_SEQUENCE = LTC_DER_SEQ_ORDERED,
};

int der_decode_sequence_ex(const unsigned char *in, unsigned long inlen,
ltc_asn1_list *list, unsigned long outlen, unsigned int flags);

#define der_decode_sequence(in, inlen, list, outlen) der_decode_sequence_ex(in, inlen, list, outlen, LTC_DER_SEQ_SEQUENCE | LTC_DER_SEQ_RELAXED)
#define der_decode_sequence_strict(in, inlen, list, outlen) der_decode_sequence_ex(in, inlen, list, outlen, LTC_DER_SEQ_SEQUENCE | LTC_DER_SEQ_STRICT)

int der_length_sequence(const ltc_asn1_list *list, unsigned long inlen,
unsigned long *outlen);



int der_encode_custom_type(const ltc_asn1_list *root,
unsigned char *out, unsigned long *outlen);

int der_decode_custom_type(const unsigned char *in, unsigned long inlen,
ltc_asn1_list *root);

int der_length_custom_type(const ltc_asn1_list *root,
unsigned long *outlen,
unsigned long *payloadlen);


#define der_decode_set(in, inlen, list, outlen) der_decode_sequence_ex(in, inlen, list, outlen, LTC_DER_SEQ_SET)
#define der_length_set der_length_sequence
int der_encode_set(const ltc_asn1_list *list, unsigned long inlen,
unsigned char *out, unsigned long *outlen);

int der_encode_setof(const ltc_asn1_list *list, unsigned long inlen,
unsigned char *out, unsigned long *outlen);


int der_encode_sequence_multi(unsigned char *out, unsigned long *outlen, ...);
int der_decode_sequence_multi(const unsigned char *in, unsigned long inlen, ...);


int der_decode_sequence_flexi(const unsigned char *in, unsigned long *inlen, ltc_asn1_list **out);
#define der_free_sequence_flexi der_sequence_free
void der_sequence_free(ltc_asn1_list *in);
void der_sequence_shrink(ltc_asn1_list *in);


int der_length_boolean(unsigned long *outlen);
int der_encode_boolean(int in,
unsigned char *out, unsigned long *outlen);
int der_decode_boolean(const unsigned char *in, unsigned long inlen,
int *out);

int der_encode_integer(void *num, unsigned char *out, unsigned long *outlen);
int der_decode_integer(const unsigned char *in, unsigned long inlen, void *num);
int der_length_integer(void *num, unsigned long *outlen);


int der_decode_short_integer(const unsigned char *in, unsigned long inlen, unsigned long *num);
int der_encode_short_integer(unsigned long num, unsigned char *out, unsigned long *outlen);
int der_length_short_integer(unsigned long num, unsigned long *outlen);


int der_encode_bit_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_decode_bit_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_encode_raw_bit_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_decode_raw_bit_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_length_bit_string(unsigned long nbits, unsigned long *outlen);


int der_encode_octet_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_decode_octet_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_length_octet_string(unsigned long noctets, unsigned long *outlen);


int der_encode_object_identifier(const unsigned long *words, unsigned long nwords,
unsigned char *out, unsigned long *outlen);
int der_decode_object_identifier(const unsigned char *in, unsigned long inlen,
unsigned long *words, unsigned long *outlen);
int der_length_object_identifier(const unsigned long *words, unsigned long nwords, unsigned long *outlen);
unsigned long der_object_identifier_bits(unsigned long x);


int der_encode_ia5_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_decode_ia5_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_length_ia5_string(const unsigned char *octets, unsigned long noctets, unsigned long *outlen);

int der_ia5_char_encode(int c);
int der_ia5_value_decode(int v);


int der_decode_teletex_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_length_teletex_string(const unsigned char *octets, unsigned long noctets, unsigned long *outlen);


int der_encode_printable_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_decode_printable_string(const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int der_length_printable_string(const unsigned char *octets, unsigned long noctets, unsigned long *outlen);

int der_printable_char_encode(int c);
int der_printable_value_decode(int v);


#if (defined(SIZE_MAX) || __STDC_VERSION__ >= 199901L || defined(WCHAR_MAX) || defined(__WCHAR_MAX__) || defined(_WCHAR_T) || defined(_WCHAR_T_DEFINED) || defined (__WCHAR_TYPE__)) && !defined(LTC_NO_WCHAR)
#if defined(__WCHAR_MAX__)
#define LTC_WCHAR_MAX __WCHAR_MAX__
#else
#include <wchar.h>
#define LTC_WCHAR_MAX WCHAR_MAX
#endif

#else
typedef ulong32 wchar_t;
#define LTC_WCHAR_MAX 0xFFFFFFFF
#endif

int der_encode_utf8_string(const wchar_t *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);

int der_decode_utf8_string(const unsigned char *in, unsigned long inlen,
wchar_t *out, unsigned long *outlen);
unsigned long der_utf8_charsize(const wchar_t c);
int der_length_utf8_string(const wchar_t *in, unsigned long noctets, unsigned long *outlen);



int der_decode_choice(const unsigned char *in, unsigned long *inlen,
ltc_asn1_list *list, unsigned long outlen);


typedef struct {
unsigned YY, 
MM, 
DD, 
hh, 
mm, 
ss, 
off_dir, 
off_hh, 
off_mm; 
} ltc_utctime;

int der_encode_utctime(const ltc_utctime *utctime,
unsigned char *out, unsigned long *outlen);

int der_decode_utctime(const unsigned char *in, unsigned long *inlen,
ltc_utctime *out);

int der_length_utctime(const ltc_utctime *utctime, unsigned long *outlen);


typedef struct {
unsigned YYYY, 
MM, 
DD, 
hh, 
mm, 
ss, 
fs, 
off_dir, 
off_hh, 
off_mm; 
} ltc_generalizedtime;

int der_encode_generalizedtime(const ltc_generalizedtime *gtime,
unsigned char *out, unsigned long *outlen);

int der_decode_generalizedtime(const unsigned char *in, unsigned long *inlen,
ltc_generalizedtime *out);

int der_length_generalizedtime(const ltc_generalizedtime *gtime, unsigned long *outlen);

#endif




