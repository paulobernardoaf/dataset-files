#if defined(LTC_BLOWFISH)
struct blowfish_key {
ulong32 S[4][256];
ulong32 K[18];
};
#endif
#if defined(LTC_RC5)
struct rc5_key {
int rounds;
ulong32 K[50];
};
#endif
#if defined(LTC_RC6)
struct rc6_key {
ulong32 K[44];
};
#endif
#if defined(LTC_SAFERP)
struct saferp_key {
unsigned char K[33][16];
long rounds;
};
#endif
#if defined(LTC_RIJNDAEL)
struct rijndael_key {
ulong32 eK[60], dK[60];
int Nr;
};
#endif
#if defined(LTC_KSEED)
struct kseed_key {
ulong32 K[32], dK[32];
};
#endif
#if defined(LTC_KASUMI)
struct kasumi_key {
ulong32 KLi1[8], KLi2[8],
KOi1[8], KOi2[8], KOi3[8],
KIi1[8], KIi2[8], KIi3[8];
};
#endif
#if defined(LTC_XTEA)
struct xtea_key {
unsigned long A[32], B[32];
};
#endif
#if defined(LTC_TWOFISH)
#if !defined(LTC_TWOFISH_SMALL)
struct twofish_key {
ulong32 S[4][256], K[40];
};
#else
struct twofish_key {
ulong32 K[40];
unsigned char S[32], start;
};
#endif
#endif
#if defined(LTC_SAFER)
#define LTC_SAFER_K64_DEFAULT_NOF_ROUNDS 6
#define LTC_SAFER_K128_DEFAULT_NOF_ROUNDS 10
#define LTC_SAFER_SK64_DEFAULT_NOF_ROUNDS 8
#define LTC_SAFER_SK128_DEFAULT_NOF_ROUNDS 10
#define LTC_SAFER_MAX_NOF_ROUNDS 13
#define LTC_SAFER_BLOCK_LEN 8
#define LTC_SAFER_KEY_LEN (1 + LTC_SAFER_BLOCK_LEN * (1 + 2 * LTC_SAFER_MAX_NOF_ROUNDS))
typedef unsigned char safer_block_t[LTC_SAFER_BLOCK_LEN];
typedef unsigned char safer_key_t[LTC_SAFER_KEY_LEN];
struct safer_key { safer_key_t key; };
#endif
#if defined(LTC_RC2)
struct rc2_key { unsigned xkey[64]; };
#endif
#if defined(LTC_DES)
struct des_key {
ulong32 ek[32], dk[32];
};
struct des3_key {
ulong32 ek[3][32], dk[3][32];
};
#endif
#if defined(LTC_CAST5)
struct cast5_key {
ulong32 K[32], keylen;
};
#endif
#if defined(LTC_NOEKEON)
struct noekeon_key {
ulong32 K[4], dK[4];
};
#endif
#if defined(LTC_SKIPJACK)
struct skipjack_key {
unsigned char key[10];
};
#endif
#if defined(LTC_KHAZAD)
struct khazad_key {
ulong64 roundKeyEnc[8 + 1];
ulong64 roundKeyDec[8 + 1];
};
#endif
#if defined(LTC_ANUBIS)
struct anubis_key {
int keyBits;
int R;
ulong32 roundKeyEnc[18 + 1][4];
ulong32 roundKeyDec[18 + 1][4];
};
#endif
#if defined(LTC_MULTI2)
struct multi2_key {
int N;
ulong32 uk[8];
};
#endif
#if defined(LTC_CAMELLIA)
struct camellia_key {
int R;
ulong64 kw[4], k[24], kl[6];
};
#endif
#if defined(LTC_IDEA)
#define LTC_IDEA_ROUNDS 8
#define LTC_IDEA_KEYLEN 6*LTC_IDEA_ROUNDS+4
struct idea_key {
unsigned short int ek[LTC_IDEA_KEYLEN]; 
unsigned short int dk[LTC_IDEA_KEYLEN]; 
};
#endif
#if defined(LTC_SERPENT)
struct serpent_key {
ulong32 k[33*4];
};
#endif
#if defined(LTC_TEA)
struct tea_key {
ulong32 k[4];
};
#endif
typedef union Symmetric_key {
#if defined(LTC_DES)
struct des_key des;
struct des3_key des3;
#endif
#if defined(LTC_RC2)
struct rc2_key rc2;
#endif
#if defined(LTC_SAFER)
struct safer_key safer;
#endif
#if defined(LTC_TWOFISH)
struct twofish_key twofish;
#endif
#if defined(LTC_BLOWFISH)
struct blowfish_key blowfish;
#endif
#if defined(LTC_RC5)
struct rc5_key rc5;
#endif
#if defined(LTC_RC6)
struct rc6_key rc6;
#endif
#if defined(LTC_SAFERP)
struct saferp_key saferp;
#endif
#if defined(LTC_RIJNDAEL)
struct rijndael_key rijndael;
#endif
#if defined(LTC_XTEA)
struct xtea_key xtea;
#endif
#if defined(LTC_CAST5)
struct cast5_key cast5;
#endif
#if defined(LTC_NOEKEON)
struct noekeon_key noekeon;
#endif
#if defined(LTC_SKIPJACK)
struct skipjack_key skipjack;
#endif
#if defined(LTC_KHAZAD)
struct khazad_key khazad;
#endif
#if defined(LTC_ANUBIS)
struct anubis_key anubis;
#endif
#if defined(LTC_KSEED)
struct kseed_key kseed;
#endif
#if defined(LTC_KASUMI)
struct kasumi_key kasumi;
#endif
#if defined(LTC_MULTI2)
struct multi2_key multi2;
#endif
#if defined(LTC_CAMELLIA)
struct camellia_key camellia;
#endif
#if defined(LTC_IDEA)
struct idea_key idea;
#endif
#if defined(LTC_SERPENT)
struct serpent_key serpent;
#endif
#if defined(LTC_TEA)
struct tea_key tea;
#endif
void *data;
} symmetric_key;
#if defined(LTC_ECB_MODE)
typedef struct {
int cipher,
blocklen;
symmetric_key key;
} symmetric_ECB;
#endif
#if defined(LTC_CFB_MODE)
typedef struct {
int cipher,
blocklen,
padlen;
unsigned char IV[MAXBLOCKSIZE],
pad[MAXBLOCKSIZE];
symmetric_key key;
} symmetric_CFB;
#endif
#if defined(LTC_OFB_MODE)
typedef struct {
int cipher,
blocklen,
padlen;
unsigned char IV[MAXBLOCKSIZE];
symmetric_key key;
} symmetric_OFB;
#endif
#if defined(LTC_CBC_MODE)
typedef struct {
int cipher,
blocklen;
unsigned char IV[MAXBLOCKSIZE];
symmetric_key key;
} symmetric_CBC;
#endif
#if defined(LTC_CTR_MODE)
typedef struct {
int cipher,
blocklen,
padlen,
mode,
ctrlen;
unsigned char ctr[MAXBLOCKSIZE],
pad[MAXBLOCKSIZE];
symmetric_key key;
} symmetric_CTR;
#endif
#if defined(LTC_LRW_MODE)
typedef struct {
int cipher;
unsigned char IV[16],
tweak[16],
pad[16];
symmetric_key key;
#if defined(LTC_LRW_TABLES)
unsigned char PC[16][256][16];
#endif
} symmetric_LRW;
#endif
#if defined(LTC_F8_MODE)
typedef struct {
int cipher,
blocklen,
padlen;
unsigned char IV[MAXBLOCKSIZE],
MIV[MAXBLOCKSIZE];
ulong32 blockcnt;
symmetric_key key;
} symmetric_F8;
#endif
extern struct ltc_cipher_descriptor {
const char *name;
unsigned char ID;
int min_key_length,
max_key_length,
block_length,
default_rounds;
int (*setup)(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int (*ecb_encrypt)(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int (*ecb_decrypt)(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int (*test)(void);
void (*done)(symmetric_key *skey);
int (*keysize)(int *keysize);
int (*accel_ecb_encrypt)(const unsigned char *pt, unsigned char *ct, unsigned long blocks, symmetric_key *skey);
int (*accel_ecb_decrypt)(const unsigned char *ct, unsigned char *pt, unsigned long blocks, symmetric_key *skey);
int (*accel_cbc_encrypt)(const unsigned char *pt, unsigned char *ct, unsigned long blocks, unsigned char *IV, symmetric_key *skey);
int (*accel_cbc_decrypt)(const unsigned char *ct, unsigned char *pt, unsigned long blocks, unsigned char *IV, symmetric_key *skey);
int (*accel_ctr_encrypt)(const unsigned char *pt, unsigned char *ct, unsigned long blocks, unsigned char *IV, int mode, symmetric_key *skey);
int (*accel_lrw_encrypt)(const unsigned char *pt, unsigned char *ct, unsigned long blocks, unsigned char *IV, const unsigned char *tweak, symmetric_key *skey);
int (*accel_lrw_decrypt)(const unsigned char *ct, unsigned char *pt, unsigned long blocks, unsigned char *IV, const unsigned char *tweak, symmetric_key *skey);
int (*accel_ccm_memory)(
const unsigned char *key, unsigned long keylen,
symmetric_key *uskey,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *header, unsigned long headerlen,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen,
int direction);
int (*accel_gcm_memory)(
const unsigned char *key, unsigned long keylen,
const unsigned char *IV, unsigned long IVlen,
const unsigned char *adata, unsigned long adatalen,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen,
int direction);
int (*omac_memory)(
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int (*xcbc_memory)(
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int (*f9_memory)(
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int (*accel_xts_encrypt)(const unsigned char *pt, unsigned char *ct,
unsigned long blocks, unsigned char *tweak,
const symmetric_key *skey1, const symmetric_key *skey2);
int (*accel_xts_decrypt)(const unsigned char *ct, unsigned char *pt,
unsigned long blocks, unsigned char *tweak,
const symmetric_key *skey1, const symmetric_key *skey2);
} cipher_descriptor[];
#if defined(LTC_BLOWFISH)
int blowfish_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int blowfish_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int blowfish_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int blowfish_test(void);
void blowfish_done(symmetric_key *skey);
int blowfish_keysize(int *keysize);
extern const struct ltc_cipher_descriptor blowfish_desc;
#endif
#if defined(LTC_RC5)
int rc5_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int rc5_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int rc5_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int rc5_test(void);
void rc5_done(symmetric_key *skey);
int rc5_keysize(int *keysize);
extern const struct ltc_cipher_descriptor rc5_desc;
#endif
#if defined(LTC_RC6)
int rc6_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int rc6_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int rc6_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int rc6_test(void);
void rc6_done(symmetric_key *skey);
int rc6_keysize(int *keysize);
extern const struct ltc_cipher_descriptor rc6_desc;
#endif
#if defined(LTC_RC2)
int rc2_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int rc2_setup_ex(const unsigned char *key, int keylen, int bits, int num_rounds, symmetric_key *skey);
int rc2_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int rc2_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int rc2_test(void);
void rc2_done(symmetric_key *skey);
int rc2_keysize(int *keysize);
extern const struct ltc_cipher_descriptor rc2_desc;
#endif
#if defined(LTC_SAFERP)
int saferp_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int saferp_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int saferp_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int saferp_test(void);
void saferp_done(symmetric_key *skey);
int saferp_keysize(int *keysize);
extern const struct ltc_cipher_descriptor saferp_desc;
#endif
#if defined(LTC_SAFER)
int safer_k64_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int safer_sk64_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int safer_k128_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int safer_sk128_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int safer_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int safer_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int safer_k64_test(void);
int safer_sk64_test(void);
int safer_sk128_test(void);
void safer_done(symmetric_key *skey);
int safer_64_keysize(int *keysize);
int safer_128_keysize(int *keysize);
extern const struct ltc_cipher_descriptor safer_k64_desc, safer_k128_desc, safer_sk64_desc, safer_sk128_desc;
#endif
#if defined(LTC_RIJNDAEL)
#define aes_setup rijndael_setup
#define aes_ecb_encrypt rijndael_ecb_encrypt
#define aes_ecb_decrypt rijndael_ecb_decrypt
#define aes_test rijndael_test
#define aes_done rijndael_done
#define aes_keysize rijndael_keysize
#define aes_enc_setup rijndael_enc_setup
#define aes_enc_ecb_encrypt rijndael_enc_ecb_encrypt
#define aes_enc_keysize rijndael_enc_keysize
int rijndael_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int rijndael_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int rijndael_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int rijndael_test(void);
void rijndael_done(symmetric_key *skey);
int rijndael_keysize(int *keysize);
int rijndael_enc_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int rijndael_enc_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
void rijndael_enc_done(symmetric_key *skey);
int rijndael_enc_keysize(int *keysize);
extern const struct ltc_cipher_descriptor rijndael_desc, aes_desc;
extern const struct ltc_cipher_descriptor rijndael_enc_desc, aes_enc_desc;
#endif
#if defined(LTC_XTEA)
int xtea_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int xtea_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int xtea_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int xtea_test(void);
void xtea_done(symmetric_key *skey);
int xtea_keysize(int *keysize);
extern const struct ltc_cipher_descriptor xtea_desc;
#endif
#if defined(LTC_TWOFISH)
int twofish_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int twofish_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int twofish_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int twofish_test(void);
void twofish_done(symmetric_key *skey);
int twofish_keysize(int *keysize);
extern const struct ltc_cipher_descriptor twofish_desc;
#endif
#if defined(LTC_DES)
int des_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int des_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int des_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int des_test(void);
void des_done(symmetric_key *skey);
int des_keysize(int *keysize);
int des3_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int des3_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int des3_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int des3_test(void);
void des3_done(symmetric_key *skey);
int des3_keysize(int *keysize);
extern const struct ltc_cipher_descriptor des_desc, des3_desc;
#endif
#if defined(LTC_CAST5)
int cast5_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int cast5_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int cast5_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int cast5_test(void);
void cast5_done(symmetric_key *skey);
int cast5_keysize(int *keysize);
extern const struct ltc_cipher_descriptor cast5_desc;
#endif
#if defined(LTC_NOEKEON)
int noekeon_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int noekeon_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int noekeon_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int noekeon_test(void);
void noekeon_done(symmetric_key *skey);
int noekeon_keysize(int *keysize);
extern const struct ltc_cipher_descriptor noekeon_desc;
#endif
#if defined(LTC_SKIPJACK)
int skipjack_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int skipjack_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int skipjack_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int skipjack_test(void);
void skipjack_done(symmetric_key *skey);
int skipjack_keysize(int *keysize);
extern const struct ltc_cipher_descriptor skipjack_desc;
#endif
#if defined(LTC_KHAZAD)
int khazad_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int khazad_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int khazad_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int khazad_test(void);
void khazad_done(symmetric_key *skey);
int khazad_keysize(int *keysize);
extern const struct ltc_cipher_descriptor khazad_desc;
#endif
#if defined(LTC_ANUBIS)
int anubis_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int anubis_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int anubis_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int anubis_test(void);
void anubis_done(symmetric_key *skey);
int anubis_keysize(int *keysize);
extern const struct ltc_cipher_descriptor anubis_desc;
#endif
#if defined(LTC_KSEED)
int kseed_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int kseed_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int kseed_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int kseed_test(void);
void kseed_done(symmetric_key *skey);
int kseed_keysize(int *keysize);
extern const struct ltc_cipher_descriptor kseed_desc;
#endif
#if defined(LTC_KASUMI)
int kasumi_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int kasumi_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int kasumi_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int kasumi_test(void);
void kasumi_done(symmetric_key *skey);
int kasumi_keysize(int *keysize);
extern const struct ltc_cipher_descriptor kasumi_desc;
#endif
#if defined(LTC_MULTI2)
int multi2_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int multi2_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int multi2_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int multi2_test(void);
void multi2_done(symmetric_key *skey);
int multi2_keysize(int *keysize);
extern const struct ltc_cipher_descriptor multi2_desc;
#endif
#if defined(LTC_CAMELLIA)
int camellia_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int camellia_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int camellia_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int camellia_test(void);
void camellia_done(symmetric_key *skey);
int camellia_keysize(int *keysize);
extern const struct ltc_cipher_descriptor camellia_desc;
#endif
#if defined(LTC_IDEA)
int idea_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int idea_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int idea_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int idea_test(void);
void idea_done(symmetric_key *skey);
int idea_keysize(int *keysize);
extern const struct ltc_cipher_descriptor idea_desc;
#endif
#if defined(LTC_SERPENT)
int serpent_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int serpent_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int serpent_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int serpent_test(void);
void serpent_done(symmetric_key *skey);
int serpent_keysize(int *keysize);
extern const struct ltc_cipher_descriptor serpent_desc;
#endif
#if defined(LTC_TEA)
int tea_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey);
int tea_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey);
int tea_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey);
int tea_test(void);
void tea_done(symmetric_key *skey);
int tea_keysize(int *keysize);
extern const struct ltc_cipher_descriptor tea_desc;
#endif
#if defined(LTC_ECB_MODE)
int ecb_start(int cipher, const unsigned char *key,
int keylen, int num_rounds, symmetric_ECB *ecb);
int ecb_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_ECB *ecb);
int ecb_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_ECB *ecb);
int ecb_done(symmetric_ECB *ecb);
#endif
#if defined(LTC_CFB_MODE)
int cfb_start(int cipher, const unsigned char *IV, const unsigned char *key,
int keylen, int num_rounds, symmetric_CFB *cfb);
int cfb_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CFB *cfb);
int cfb_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CFB *cfb);
int cfb_getiv(unsigned char *IV, unsigned long *len, const symmetric_CFB *cfb);
int cfb_setiv(const unsigned char *IV, unsigned long len, symmetric_CFB *cfb);
int cfb_done(symmetric_CFB *cfb);
#endif
#if defined(LTC_OFB_MODE)
int ofb_start(int cipher, const unsigned char *IV, const unsigned char *key,
int keylen, int num_rounds, symmetric_OFB *ofb);
int ofb_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_OFB *ofb);
int ofb_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_OFB *ofb);
int ofb_getiv(unsigned char *IV, unsigned long *len, const symmetric_OFB *ofb);
int ofb_setiv(const unsigned char *IV, unsigned long len, symmetric_OFB *ofb);
int ofb_done(symmetric_OFB *ofb);
#endif
#if defined(LTC_CBC_MODE)
int cbc_start(int cipher, const unsigned char *IV, const unsigned char *key,
int keylen, int num_rounds, symmetric_CBC *cbc);
int cbc_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CBC *cbc);
int cbc_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CBC *cbc);
int cbc_getiv(unsigned char *IV, unsigned long *len, const symmetric_CBC *cbc);
int cbc_setiv(const unsigned char *IV, unsigned long len, symmetric_CBC *cbc);
int cbc_done(symmetric_CBC *cbc);
#endif
#if defined(LTC_CTR_MODE)
#define CTR_COUNTER_LITTLE_ENDIAN 0x0000
#define CTR_COUNTER_BIG_ENDIAN 0x1000
#define LTC_CTR_RFC3686 0x2000
int ctr_start( int cipher,
const unsigned char *IV,
const unsigned char *key, int keylen,
int num_rounds, int ctr_mode,
symmetric_CTR *ctr);
int ctr_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_CTR *ctr);
int ctr_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_CTR *ctr);
int ctr_getiv(unsigned char *IV, unsigned long *len, const symmetric_CTR *ctr);
int ctr_setiv(const unsigned char *IV, unsigned long len, symmetric_CTR *ctr);
int ctr_done(symmetric_CTR *ctr);
int ctr_test(void);
#endif
#if defined(LTC_LRW_MODE)
#define LRW_ENCRYPT LTC_ENCRYPT
#define LRW_DECRYPT LTC_DECRYPT
int lrw_start( int cipher,
const unsigned char *IV,
const unsigned char *key, int keylen,
const unsigned char *tweak,
int num_rounds,
symmetric_LRW *lrw);
int lrw_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_LRW *lrw);
int lrw_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_LRW *lrw);
int lrw_getiv(unsigned char *IV, unsigned long *len, const symmetric_LRW *lrw);
int lrw_setiv(const unsigned char *IV, unsigned long len, symmetric_LRW *lrw);
int lrw_done(symmetric_LRW *lrw);
int lrw_test(void);
int lrw_process(const unsigned char *pt, unsigned char *ct, unsigned long len, int mode, symmetric_LRW *lrw);
#endif
#if defined(LTC_F8_MODE)
int f8_start( int cipher, const unsigned char *IV,
const unsigned char *key, int keylen,
const unsigned char *salt_key, int skeylen,
int num_rounds, symmetric_F8 *f8);
int f8_encrypt(const unsigned char *pt, unsigned char *ct, unsigned long len, symmetric_F8 *f8);
int f8_decrypt(const unsigned char *ct, unsigned char *pt, unsigned long len, symmetric_F8 *f8);
int f8_getiv(unsigned char *IV, unsigned long *len, const symmetric_F8 *f8);
int f8_setiv(const unsigned char *IV, unsigned long len, symmetric_F8 *f8);
int f8_done(symmetric_F8 *f8);
int f8_test_mode(void);
#endif
#if defined(LTC_XTS_MODE)
typedef struct {
symmetric_key key1, key2;
int cipher;
} symmetric_xts;
int xts_start( int cipher,
const unsigned char *key1,
const unsigned char *key2,
unsigned long keylen,
int num_rounds,
symmetric_xts *xts);
int xts_encrypt(
const unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tweak,
const symmetric_xts *xts);
int xts_decrypt(
const unsigned char *ct, unsigned long ptlen,
unsigned char *pt,
unsigned char *tweak,
const symmetric_xts *xts);
void xts_done(symmetric_xts *xts);
int xts_test(void);
void xts_mult_x(unsigned char *I);
#endif
int find_cipher(const char *name);
int find_cipher_any(const char *name, int blocklen, int keylen);
int find_cipher_id(unsigned char ID);
int register_cipher(const struct ltc_cipher_descriptor *cipher);
int unregister_cipher(const struct ltc_cipher_descriptor *cipher);
int register_all_ciphers(void);
int cipher_is_valid(int idx);
LTC_MUTEX_PROTO(ltc_cipher_mutex)
#if defined(LTC_CHACHA)
typedef struct {
ulong32 input[16];
unsigned char kstream[64];
unsigned long ksleft;
unsigned long ivlen;
int rounds;
} chacha_state;
int chacha_setup(chacha_state *st, const unsigned char *key, unsigned long keylen, int rounds);
int chacha_ivctr32(chacha_state *st, const unsigned char *iv, unsigned long ivlen, ulong32 counter);
int chacha_ivctr64(chacha_state *st, const unsigned char *iv, unsigned long ivlen, ulong64 counter);
int chacha_crypt(chacha_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int chacha_keystream(chacha_state *st, unsigned char *out, unsigned long outlen);
int chacha_done(chacha_state *st);
int chacha_test(void);
int chacha_memory(const unsigned char *key, unsigned long keylen, unsigned long rounds,
const unsigned char *iv, unsigned long ivlen, ulong64 counter,
const unsigned char *datain, unsigned long datalen, unsigned char *dataout);
#endif 
#if defined(LTC_SALSA20)
typedef struct {
ulong32 input[16];
unsigned char kstream[64];
unsigned long ksleft;
unsigned long ivlen;
int rounds;
} salsa20_state;
int salsa20_setup(salsa20_state *st, const unsigned char *key, unsigned long keylen, int rounds);
int salsa20_ivctr64(salsa20_state *st, const unsigned char *iv, unsigned long ivlen, ulong64 counter);
int salsa20_crypt(salsa20_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int salsa20_keystream(salsa20_state *st, unsigned char *out, unsigned long outlen);
int salsa20_done(salsa20_state *st);
int salsa20_test(void);
int salsa20_memory(const unsigned char *key, unsigned long keylen, unsigned long rounds,
const unsigned char *iv, unsigned long ivlen, ulong64 counter,
const unsigned char *datain, unsigned long datalen, unsigned char *dataout);
#endif 
#if defined(LTC_XSALSA20)
int xsalsa20_setup(salsa20_state *st, const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
int rounds);
int xsalsa20_test(void);
int xsalsa20_memory(const unsigned char *key, unsigned long keylen, unsigned long rounds,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *datain, unsigned long datalen, unsigned char *dataout);
#endif 
#if defined(LTC_SOSEMANUK)
typedef struct {
ulong32 kc[100]; 
ulong32 s00, s01, s02, s03, s04, s05, s06, s07, s08, s09;
ulong32 r1, r2;
unsigned char buf[80];
unsigned ptr;
} sosemanuk_state;
int sosemanuk_setup(sosemanuk_state *st, const unsigned char *key, unsigned long keylen);
int sosemanuk_setiv(sosemanuk_state *st, const unsigned char *iv, unsigned long ivlen);
int sosemanuk_crypt(sosemanuk_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int sosemanuk_keystream(sosemanuk_state *st, unsigned char *out, unsigned long outlen);
int sosemanuk_done(sosemanuk_state *st);
int sosemanuk_test(void);
int sosemanuk_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *iv, unsigned long ivlen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout);
#endif 
#if defined(LTC_RABBIT)
typedef struct {
ulong32 x[8];
ulong32 c[8];
ulong32 carry;
} rabbit_ctx;
typedef struct {
rabbit_ctx master_ctx;
rabbit_ctx work_ctx;
unsigned char block[16]; 
ulong32 unused; 
} rabbit_state;
int rabbit_setup(rabbit_state* st, const unsigned char *key, unsigned long keylen);
int rabbit_setiv(rabbit_state* st, const unsigned char *iv, unsigned long ivlen);
int rabbit_crypt(rabbit_state* st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int rabbit_keystream(rabbit_state* st, unsigned char *out, unsigned long outlen);
int rabbit_done(rabbit_state *st);
int rabbit_test(void);
int rabbit_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *iv, unsigned long ivlen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout);
#endif 
#if defined(LTC_RC4_STREAM)
typedef struct {
unsigned int x, y;
unsigned char buf[256];
} rc4_state;
int rc4_stream_setup(rc4_state *st, const unsigned char *key, unsigned long keylen);
int rc4_stream_crypt(rc4_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int rc4_stream_keystream(rc4_state *st, unsigned char *out, unsigned long outlen);
int rc4_stream_done(rc4_state *st);
int rc4_stream_test(void);
int rc4_stream_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout);
#endif 
#if defined(LTC_SOBER128_STREAM)
typedef struct {
ulong32 R[17], 
initR[17], 
konst, 
sbuf; 
int nbuf; 
} sober128_state;
int sober128_stream_setup(sober128_state *st, const unsigned char *key, unsigned long keylen);
int sober128_stream_setiv(sober128_state *st, const unsigned char *iv, unsigned long ivlen);
int sober128_stream_crypt(sober128_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int sober128_stream_keystream(sober128_state *st, unsigned char *out, unsigned long outlen);
int sober128_stream_done(sober128_state *st);
int sober128_stream_test(void);
int sober128_stream_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *iv, unsigned long ivlen,
const unsigned char *datain, unsigned long datalen,
unsigned char *dataout);
#endif 
