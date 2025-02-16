#if defined(LTC_HMAC)
typedef struct Hmac_state {
hash_state md;
int hash;
unsigned char key[MAXBLOCKSIZE];
} hmac_state;
int hmac_init(hmac_state *hmac, int hash, const unsigned char *key, unsigned long keylen);
int hmac_process(hmac_state *hmac, const unsigned char *in, unsigned long inlen);
int hmac_done(hmac_state *hmac, unsigned char *out, unsigned long *outlen);
int hmac_test(void);
int hmac_memory(int hash,
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int hmac_memory_multi(int hash,
const unsigned char *key, unsigned long keylen,
unsigned char *out, unsigned long *outlen,
const unsigned char *in, unsigned long inlen, ...);
int hmac_file(int hash, const char *fname, const unsigned char *key,
unsigned long keylen,
unsigned char *out, unsigned long *outlen);
#endif
#if defined(LTC_OMAC)
typedef struct {
int cipher_idx,
buflen,
blklen;
unsigned char block[MAXBLOCKSIZE],
prev[MAXBLOCKSIZE],
Lu[2][MAXBLOCKSIZE];
symmetric_key key;
} omac_state;
int omac_init(omac_state *omac, int cipher, const unsigned char *key, unsigned long keylen);
int omac_process(omac_state *omac, const unsigned char *in, unsigned long inlen);
int omac_done(omac_state *omac, unsigned char *out, unsigned long *outlen);
int omac_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int omac_memory_multi(int cipher,
const unsigned char *key, unsigned long keylen,
unsigned char *out, unsigned long *outlen,
const unsigned char *in, unsigned long inlen, ...);
int omac_file(int cipher,
const unsigned char *key, unsigned long keylen,
const char *filename,
unsigned char *out, unsigned long *outlen);
int omac_test(void);
#endif 
#if defined(LTC_PMAC)
typedef struct {
unsigned char Ls[32][MAXBLOCKSIZE], 
Li[MAXBLOCKSIZE], 
Lr[MAXBLOCKSIZE], 
block[MAXBLOCKSIZE], 
checksum[MAXBLOCKSIZE]; 
symmetric_key key; 
unsigned long block_index; 
int cipher_idx, 
block_len, 
buflen; 
} pmac_state;
int pmac_init(pmac_state *pmac, int cipher, const unsigned char *key, unsigned long keylen);
int pmac_process(pmac_state *pmac, const unsigned char *in, unsigned long inlen);
int pmac_done(pmac_state *pmac, unsigned char *out, unsigned long *outlen);
int pmac_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int pmac_memory_multi(int cipher,
const unsigned char *key, unsigned long keylen,
unsigned char *out, unsigned long *outlen,
const unsigned char *in, unsigned long inlen, ...);
int pmac_file(int cipher,
const unsigned char *key, unsigned long keylen,
const char *filename,
unsigned char *out, unsigned long *outlen);
int pmac_test(void);
int pmac_ntz(unsigned long x);
void pmac_shift_xor(pmac_state *pmac);
#endif 
#if defined(LTC_POLY1305)
typedef struct {
ulong32 r[5];
ulong32 h[5];
ulong32 pad[4];
unsigned long leftover;
unsigned char buffer[16];
int final;
} poly1305_state;
int poly1305_init(poly1305_state *st, const unsigned char *key, unsigned long keylen);
int poly1305_process(poly1305_state *st, const unsigned char *in, unsigned long inlen);
int poly1305_done(poly1305_state *st, unsigned char *mac, unsigned long *maclen);
int poly1305_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen);
int poly1305_memory_multi(const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen, const unsigned char *in, unsigned long inlen, ...);
int poly1305_file(const char *fname, const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen);
int poly1305_test(void);
#endif 
#if defined(LTC_BLAKE2SMAC)
typedef hash_state blake2smac_state;
int blake2smac_init(blake2smac_state *st, unsigned long outlen, const unsigned char *key, unsigned long keylen);
int blake2smac_process(blake2smac_state *st, const unsigned char *in, unsigned long inlen);
int blake2smac_done(blake2smac_state *st, unsigned char *mac, unsigned long *maclen);
int blake2smac_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen);
int blake2smac_memory_multi(const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen, const unsigned char *in, unsigned long inlen, ...);
int blake2smac_file(const char *fname, const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen);
int blake2smac_test(void);
#endif 
#if defined(LTC_BLAKE2BMAC)
typedef hash_state blake2bmac_state;
int blake2bmac_init(blake2bmac_state *st, unsigned long outlen, const unsigned char *key, unsigned long keylen);
int blake2bmac_process(blake2bmac_state *st, const unsigned char *in, unsigned long inlen);
int blake2bmac_done(blake2bmac_state *st, unsigned char *mac, unsigned long *maclen);
int blake2bmac_memory(const unsigned char *key, unsigned long keylen, const unsigned char *in, unsigned long inlen, unsigned char *mac, unsigned long *maclen);
int blake2bmac_memory_multi(const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen, const unsigned char *in, unsigned long inlen, ...);
int blake2bmac_file(const char *fname, const unsigned char *key, unsigned long keylen, unsigned char *mac, unsigned long *maclen);
int blake2bmac_test(void);
#endif 
#if defined(LTC_PELICAN)
typedef struct pelican_state
{
symmetric_key K;
unsigned char state[16];
int buflen;
} pelican_state;
int pelican_init(pelican_state *pelmac, const unsigned char *key, unsigned long keylen);
int pelican_process(pelican_state *pelmac, const unsigned char *in, unsigned long inlen);
int pelican_done(pelican_state *pelmac, unsigned char *out);
int pelican_test(void);
int pelican_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out);
#endif
#if defined(LTC_XCBC)
#define LTC_XCBC_PURE 0x8000UL
typedef struct {
unsigned char K[3][MAXBLOCKSIZE],
IV[MAXBLOCKSIZE];
symmetric_key key;
int cipher,
buflen,
blocksize;
} xcbc_state;
int xcbc_init(xcbc_state *xcbc, int cipher, const unsigned char *key, unsigned long keylen);
int xcbc_process(xcbc_state *xcbc, const unsigned char *in, unsigned long inlen);
int xcbc_done(xcbc_state *xcbc, unsigned char *out, unsigned long *outlen);
int xcbc_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int xcbc_memory_multi(int cipher,
const unsigned char *key, unsigned long keylen,
unsigned char *out, unsigned long *outlen,
const unsigned char *in, unsigned long inlen, ...);
int xcbc_file(int cipher,
const unsigned char *key, unsigned long keylen,
const char *filename,
unsigned char *out, unsigned long *outlen);
int xcbc_test(void);
#endif
#if defined(LTC_F9_MODE)
typedef struct {
unsigned char akey[MAXBLOCKSIZE],
ACC[MAXBLOCKSIZE],
IV[MAXBLOCKSIZE];
symmetric_key key;
int cipher,
buflen,
keylen,
blocksize;
} f9_state;
int f9_init(f9_state *f9, int cipher, const unsigned char *key, unsigned long keylen);
int f9_process(f9_state *f9, const unsigned char *in, unsigned long inlen);
int f9_done(f9_state *f9, unsigned char *out, unsigned long *outlen);
int f9_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen);
int f9_memory_multi(int cipher,
const unsigned char *key, unsigned long keylen,
unsigned char *out, unsigned long *outlen,
const unsigned char *in, unsigned long inlen, ...);
int f9_file(int cipher,
const unsigned char *key, unsigned long keylen,
const char *fname,
unsigned char *out, unsigned long *outlen);
int f9_test(void);
#endif
#if defined(LTC_EAX_MODE)
#if !(defined(LTC_OMAC) && defined(LTC_CTR_MODE))
#error LTC_EAX_MODE requires LTC_OMAC and CTR
#endif
typedef struct {
unsigned char N[MAXBLOCKSIZE];
symmetric_CTR ctr;
omac_state headeromac, ctomac;
} eax_state;
int eax_init(eax_state *eax, int cipher, const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *header, unsigned long headerlen);
int eax_encrypt(eax_state *eax, const unsigned char *pt, unsigned char *ct, unsigned long length);
int eax_decrypt(eax_state *eax, const unsigned char *ct, unsigned char *pt, unsigned long length);
int eax_addheader(eax_state *eax, const unsigned char *header, unsigned long length);
int eax_done(eax_state *eax, unsigned char *tag, unsigned long *taglen);
int eax_encrypt_authenticate_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *header, unsigned long headerlen,
const unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen);
int eax_decrypt_verify_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *header, unsigned long headerlen,
const unsigned char *ct, unsigned long ctlen,
unsigned char *pt,
const unsigned char *tag, unsigned long taglen,
int *stat);
int eax_test(void);
#endif 
#if defined(LTC_OCB_MODE)
typedef struct {
unsigned char L[MAXBLOCKSIZE], 
Ls[32][MAXBLOCKSIZE], 
Li[MAXBLOCKSIZE], 
Lr[MAXBLOCKSIZE], 
R[MAXBLOCKSIZE], 
checksum[MAXBLOCKSIZE]; 
symmetric_key key; 
unsigned long block_index; 
int cipher, 
block_len; 
} ocb_state;
int ocb_init(ocb_state *ocb, int cipher,
const unsigned char *key, unsigned long keylen, const unsigned char *nonce);
int ocb_encrypt(ocb_state *ocb, const unsigned char *pt, unsigned char *ct);
int ocb_decrypt(ocb_state *ocb, const unsigned char *ct, unsigned char *pt);
int ocb_done_encrypt(ocb_state *ocb,
const unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen);
int ocb_done_decrypt(ocb_state *ocb,
const unsigned char *ct, unsigned long ctlen,
unsigned char *pt,
const unsigned char *tag, unsigned long taglen, int *stat);
int ocb_encrypt_authenticate_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce,
const unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen);
int ocb_decrypt_verify_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce,
const unsigned char *ct, unsigned long ctlen,
unsigned char *pt,
const unsigned char *tag, unsigned long taglen,
int *stat);
int ocb_test(void);
void ocb_shift_xor(ocb_state *ocb, unsigned char *Z);
int ocb_ntz(unsigned long x);
int s_ocb_done(ocb_state *ocb, const unsigned char *pt, unsigned long ptlen,
unsigned char *ct, unsigned char *tag, unsigned long *taglen, int mode);
#endif 
#if defined(LTC_OCB3_MODE)
typedef struct {
unsigned char Offset_0[MAXBLOCKSIZE], 
Offset_current[MAXBLOCKSIZE], 
L_dollar[MAXBLOCKSIZE], 
L_star[MAXBLOCKSIZE], 
L_[32][MAXBLOCKSIZE], 
tag_part[MAXBLOCKSIZE], 
checksum[MAXBLOCKSIZE]; 
unsigned char aSum_current[MAXBLOCKSIZE], 
aOffset_current[MAXBLOCKSIZE], 
adata_buffer[MAXBLOCKSIZE]; 
int adata_buffer_bytes; 
unsigned long ablock_index; 
symmetric_key key; 
unsigned long block_index; 
int cipher, 
tag_len, 
block_len; 
} ocb3_state;
int ocb3_init(ocb3_state *ocb, int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
unsigned long taglen);
int ocb3_encrypt(ocb3_state *ocb, const unsigned char *pt, unsigned long ptlen, unsigned char *ct);
int ocb3_decrypt(ocb3_state *ocb, const unsigned char *ct, unsigned long ctlen, unsigned char *pt);
int ocb3_encrypt_last(ocb3_state *ocb, const unsigned char *pt, unsigned long ptlen, unsigned char *ct);
int ocb3_decrypt_last(ocb3_state *ocb, const unsigned char *ct, unsigned long ctlen, unsigned char *pt);
int ocb3_add_aad(ocb3_state *ocb, const unsigned char *aad, unsigned long aadlen);
int ocb3_done(ocb3_state *ocb, unsigned char *tag, unsigned long *taglen);
int ocb3_encrypt_authenticate_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *adata, unsigned long adatalen,
const unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen);
int ocb3_decrypt_verify_memory(int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *adata, unsigned long adatalen,
const unsigned char *ct, unsigned long ctlen,
unsigned char *pt,
const unsigned char *tag, unsigned long taglen,
int *stat);
int ocb3_test(void);
#endif 
#if defined(LTC_CCM_MODE)
#define CCM_ENCRYPT LTC_ENCRYPT
#define CCM_DECRYPT LTC_DECRYPT
typedef struct {
symmetric_key K;
int cipher, 
taglen, 
x; 
unsigned long L, 
ptlen, 
current_ptlen, 
aadlen, 
current_aadlen, 
noncelen; 
unsigned char PAD[16],
ctr[16],
CTRPAD[16],
CTRlen;
} ccm_state;
int ccm_init(ccm_state *ccm, int cipher,
const unsigned char *key, int keylen, int ptlen, int taglen, int aadlen);
int ccm_reset(ccm_state *ccm);
int ccm_add_nonce(ccm_state *ccm,
const unsigned char *nonce, unsigned long noncelen);
int ccm_add_aad(ccm_state *ccm,
const unsigned char *adata, unsigned long adatalen);
int ccm_process(ccm_state *ccm,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
int direction);
int ccm_done(ccm_state *ccm,
unsigned char *tag, unsigned long *taglen);
int ccm_memory(int cipher,
const unsigned char *key, unsigned long keylen,
symmetric_key *uskey,
const unsigned char *nonce, unsigned long noncelen,
const unsigned char *header, unsigned long headerlen,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen,
int direction);
int ccm_test(void);
#endif 
#if defined(LRW_MODE) || defined(LTC_GCM_MODE)
void gcm_gf_mult(const unsigned char *a, const unsigned char *b, unsigned char *c);
#endif
#if defined(LTC_GCM_TABLES) || defined(LTC_LRW_TABLES) || ((defined(LTC_GCM_MODE) || defined(LTC_GCM_MODE)) && defined(LTC_FAST))
extern const unsigned char gcm_shift_table[];
#endif
#if defined(LTC_GCM_MODE)
#define GCM_ENCRYPT LTC_ENCRYPT
#define GCM_DECRYPT LTC_DECRYPT
#define LTC_GCM_MODE_IV 0
#define LTC_GCM_MODE_AAD 1
#define LTC_GCM_MODE_TEXT 2
typedef struct {
symmetric_key K;
unsigned char H[16], 
X[16], 
Y[16], 
Y_0[16], 
buf[16]; 
int cipher, 
ivmode, 
mode, 
buflen; 
ulong64 totlen, 
pttotlen; 
#if defined(LTC_GCM_TABLES)
unsigned char PC[16][256][16] 
#if defined(LTC_GCM_TABLES_SSE2)
__attribute__ ((aligned (16)))
#endif
;
#endif
} gcm_state;
void gcm_mult_h(const gcm_state *gcm, unsigned char *I);
int gcm_init(gcm_state *gcm, int cipher,
const unsigned char *key, int keylen);
int gcm_reset(gcm_state *gcm);
int gcm_add_iv(gcm_state *gcm,
const unsigned char *IV, unsigned long IVlen);
int gcm_add_aad(gcm_state *gcm,
const unsigned char *adata, unsigned long adatalen);
int gcm_process(gcm_state *gcm,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
int direction);
int gcm_done(gcm_state *gcm,
unsigned char *tag, unsigned long *taglen);
int gcm_memory( int cipher,
const unsigned char *key, unsigned long keylen,
const unsigned char *IV, unsigned long IVlen,
const unsigned char *adata, unsigned long adatalen,
unsigned char *pt, unsigned long ptlen,
unsigned char *ct,
unsigned char *tag, unsigned long *taglen,
int direction);
int gcm_test(void);
#endif 
#if defined(LTC_CHACHA20POLY1305_MODE)
typedef struct {
poly1305_state poly;
chacha_state chacha;
ulong64 aadlen;
ulong64 ctlen;
int aadflg;
} chacha20poly1305_state;
#define CHACHA20POLY1305_ENCRYPT LTC_ENCRYPT
#define CHACHA20POLY1305_DECRYPT LTC_DECRYPT
int chacha20poly1305_init(chacha20poly1305_state *st, const unsigned char *key, unsigned long keylen);
int chacha20poly1305_setiv(chacha20poly1305_state *st, const unsigned char *iv, unsigned long ivlen);
int chacha20poly1305_setiv_rfc7905(chacha20poly1305_state *st, const unsigned char *iv, unsigned long ivlen, ulong64 sequence_number);
int chacha20poly1305_add_aad(chacha20poly1305_state *st, const unsigned char *in, unsigned long inlen);
int chacha20poly1305_encrypt(chacha20poly1305_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int chacha20poly1305_decrypt(chacha20poly1305_state *st, const unsigned char *in, unsigned long inlen, unsigned char *out);
int chacha20poly1305_done(chacha20poly1305_state *st, unsigned char *tag, unsigned long *taglen);
int chacha20poly1305_memory(const unsigned char *key, unsigned long keylen,
const unsigned char *iv, unsigned long ivlen,
const unsigned char *aad, unsigned long aadlen,
const unsigned char *in, unsigned long inlen,
unsigned char *out,
unsigned char *tag, unsigned long *taglen,
int direction);
int chacha20poly1305_test(void);
#endif 
