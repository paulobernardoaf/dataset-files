#include "tomcrypt_private.h"
#if defined(LTC_IDEA)
const struct ltc_cipher_descriptor idea_desc = {
"idea",
24, 
16, 16, 8, 8, 
&idea_setup,
&idea_ecb_encrypt,
&idea_ecb_decrypt,
&idea_test,
&idea_done,
&idea_keysize,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
typedef unsigned short int ushort16;
#define _LOW16(x) ((x)&0xffff) 
#define _HIGH16(x) ((x)>>16)
#define _MUL(a,b) { ulong32 p = (ulong32)_LOW16(a) * b; if (p) { p = _LOW16(p) - _HIGH16(p); a = (ushort16)p - (ushort16)_HIGH16(p); } else a = 1 - a - b; }
#define _STORE16(x,y) { (y)[0] = (unsigned char)(((x)>>8)&255); (y)[1] = (unsigned char)((x)&255); }
#define _LOAD16(x,y) { x = ((ushort16)((y)[0] & 255)<<8) | ((ushort16)((y)[1] & 255)); }
static ushort16 _mul_inv(ushort16 x)
{
ushort16 y = x;
unsigned i;
for (i = 0; i < 15; i++) {
_MUL(y, _LOW16(y));
_MUL(y, x);
}
return _LOW16(y);
}
static ushort16 _add_inv(ushort16 x)
{
return _LOW16(0 - x);
}
static int _setup_key(const unsigned char *key, symmetric_key *skey)
{
int i, j;
ushort16 *e_key = skey->idea.ek;
ushort16 *d_key = skey->idea.dk;
for (i = 0; i < 8; i++) {
_LOAD16(e_key[i], key + 2 * i);
}
for (; i < LTC_IDEA_KEYLEN; i++) {
j = (i - i % 8) - 8;
e_key[i] = _LOW16((e_key[j+(i+1)%8] << 9) | (e_key[j+(i+2)%8] >> 7));
}
for (i = 0; i < LTC_IDEA_ROUNDS; i++) {
d_key[i*6+0] = _mul_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+0]);
d_key[i*6+1] = _add_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+1+(i>0 ? 1 : 0)]);
d_key[i*6+2] = _add_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+2-(i>0 ? 1 : 0)]);
d_key[i*6+3] = _mul_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+3]);
d_key[i*6+4] = e_key[(LTC_IDEA_ROUNDS-1-i)*6+4];
d_key[i*6+5] = e_key[(LTC_IDEA_ROUNDS-1-i)*6+5];
}
d_key[i*6+0] = _mul_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+0]);
d_key[i*6+1] = _add_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+1]);
d_key[i*6+2] = _add_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+2]);
d_key[i*6+3] = _mul_inv(e_key[(LTC_IDEA_ROUNDS-i)*6+3]);
return CRYPT_OK;
}
static int _process_block(const unsigned char *in, unsigned char *out, const ushort16 *m_key)
{
int i;
ushort16 x0, x1, x2, x3, t0, t1;
_LOAD16(x0, in + 0);
_LOAD16(x1, in + 2);
_LOAD16(x2, in + 4);
_LOAD16(x3, in + 6);
for (i = 0; i < LTC_IDEA_ROUNDS; i++) {
_MUL(x0, m_key[i*6+0]);
x1 += m_key[i*6+1];
x2 += m_key[i*6+2];
_MUL(x3, m_key[i*6+3]);
t0 = x0^x2;
_MUL(t0, m_key[i*6+4]);
t1 = t0 + (x1^x3);
_MUL(t1, m_key[i*6+5]);
t0 += t1;
x0 ^= t1;
x3 ^= t0;
t0 ^= x1;
x1 = x2^t1;
x2 = t0;
}
_MUL(x0, m_key[LTC_IDEA_ROUNDS*6+0]);
x2 += m_key[LTC_IDEA_ROUNDS*6+1];
x1 += m_key[LTC_IDEA_ROUNDS*6+2];
_MUL(x3, m_key[LTC_IDEA_ROUNDS*6+3]);
_STORE16(x0, out + 0);
_STORE16(x2, out + 2);
_STORE16(x1, out + 4);
_STORE16(x3, out + 6);
return CRYPT_OK;
}
int idea_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
{
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(skey != NULL);
if (num_rounds != 0 && num_rounds != 8) return CRYPT_INVALID_ROUNDS;
if (keylen != 16) return CRYPT_INVALID_KEYSIZE;
return _setup_key(key, skey);
}
int idea_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey)
{
int err = _process_block(pt, ct, skey->idea.ek);
#if defined(LTC_CLEAN_STACK)
burn_stack(sizeof(ushort16) * 6 + sizeof(int));
#endif
return err;
}
int idea_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey)
{
int err = _process_block(ct, pt, skey->idea.dk);
#if defined(LTC_CLEAN_STACK)
burn_stack(sizeof(ushort16) * 6 + sizeof(int));
#endif
return err;
}
void idea_done(symmetric_key *skey)
{
LTC_UNUSED_PARAM(skey);
}
int idea_keysize(int *keysize)
{
LTC_ARGCHK(keysize != NULL);
if (*keysize < 16) {
return CRYPT_INVALID_KEYSIZE;
}
*keysize = 16;
return CRYPT_OK;
}
int idea_test(void)
{
#if !defined(LTC_TEST)
return CRYPT_NOP;
#else
static const struct {
unsigned char key[16], pt[8], ct[8];
} tests[] = {
{
{ 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xB1, 0xF5, 0xF7, 0xF8, 0x79, 0x01, 0x37, 0x0F }
},
{
{ 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xB3, 0x92, 0x7D, 0xFF, 0xB6, 0x35, 0x86, 0x26 }
},
{
{ 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xE9, 0x87, 0xE0, 0x02, 0x9F, 0xB9, 0x97, 0x85 }
},
{
{ 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x75, 0x4A, 0x03, 0xCE, 0x08, 0xDB, 0x7D, 0xAA }
},
{
{ 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
{ 0xF0, 0x15, 0xF9, 0xFB, 0x0C, 0xFC, 0x7E, 0x1C }
},
};
unsigned char buf[2][8];
symmetric_key key;
int err, x;
if (sizeof(ushort16) != 2) {
return CRYPT_FAIL_TESTVECTOR;
}
for (x = 0; x < (int)(sizeof(tests)/sizeof(tests[0])); x++) {
if ((err = idea_setup(tests[x].key, 16, 8, &key)) != CRYPT_OK) {
return err;
}
if ((err = idea_ecb_encrypt(tests[x].pt, buf[0], &key)) != CRYPT_OK) {
return err;
}
if (compare_testvector(buf[0], 8, tests[x].ct, 8, "IDEA Encrypt", x)) {
return CRYPT_FAIL_TESTVECTOR;
}
if ((err = idea_ecb_decrypt(tests[x].ct, buf[1], &key)) != CRYPT_OK) {
return err;
}
if (compare_testvector(buf[1], 8, tests[x].pt, 8, "IDEA Decrypt", x)) {
return CRYPT_FAIL_TESTVECTOR;
}
}
return CRYPT_OK;
#endif
}
#endif
