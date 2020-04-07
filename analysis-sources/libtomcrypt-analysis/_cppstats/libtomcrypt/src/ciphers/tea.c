#include "tomcrypt_private.h"
#if defined(LTC_TEA)
const struct ltc_cipher_descriptor tea_desc =
{
"tea",
26,
16, 16, 8, 32,
&tea_setup,
&tea_ecb_encrypt,
&tea_ecb_decrypt,
&tea_test,
&tea_done,
&tea_keysize,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
#define DELTA 0x9E3779B9uL
#define SUM 0xC6EF3720uL
int tea_setup(const unsigned char *key, int keylen, int num_rounds, symmetric_key *skey)
{
LTC_ARGCHK(key != NULL);
LTC_ARGCHK(skey != NULL);
if (keylen != 16) {
return CRYPT_INVALID_KEYSIZE;
}
if (num_rounds != 0 && num_rounds != 32) {
return CRYPT_INVALID_ROUNDS;
}
LOAD32H(skey->tea.k[0], key+0);
LOAD32H(skey->tea.k[1], key+4);
LOAD32H(skey->tea.k[2], key+8);
LOAD32H(skey->tea.k[3], key+12);
return CRYPT_OK;
}
int tea_ecb_encrypt(const unsigned char *pt, unsigned char *ct, const symmetric_key *skey)
{
ulong32 y, z, sum = 0;
const ulong32 delta = DELTA;
int r;
LTC_ARGCHK(pt != NULL);
LTC_ARGCHK(ct != NULL);
LTC_ARGCHK(skey != NULL);
LOAD32H(y, &pt[0]);
LOAD32H(z, &pt[4]);
for (r = 0; r < 32; r += 4) {
sum += delta;
y += ((z<<4) + skey->tea.k[0]) ^ (z + sum) ^ ((z>>5) + skey->tea.k[1]);
z += ((y<<4) + skey->tea.k[2]) ^ (y + sum) ^ ((y>>5) + skey->tea.k[3]);
}
STORE32H(y, &ct[0]);
STORE32H(z, &ct[4]);
return CRYPT_OK;
}
int tea_ecb_decrypt(const unsigned char *ct, unsigned char *pt, const symmetric_key *skey)
{
ulong32 v0, v1, sum = SUM;
const ulong32 delta = DELTA;
int r;
LTC_ARGCHK(pt != NULL);
LTC_ARGCHK(ct != NULL);
LTC_ARGCHK(skey != NULL);
LOAD32H(v0, &ct[0]);
LOAD32H(v1, &ct[4]);
for (r = 0; r < 32; r++) {
v1 -= ((v0 << 4) + skey->tea.k[2]) ^ (v0 + sum) ^ ((v0 >> 5) + skey->tea.k[3]);
v0 -= ((v1 << 4) + skey->tea.k[0]) ^ (v1 + sum) ^ ((v1 >> 5) + skey->tea.k[1]);
sum -= delta;
}
STORE32H(v0, &pt[0]);
STORE32H(v1, &pt[4]);
return CRYPT_OK;
}
int tea_test(void)
{
#if !defined(LTC_TEST)
return CRYPT_NOP;
#else
static const struct {
const char *key, *pt, *ct;
} tests[] = {
{
"00000000000000000000000000000000",
"0000000000000000",
"41ea3a0a94baa940"
}, {
"32a1e65408b63bb9214105744ec5d2e2",
"5ada1d89a9c3801a",
"dd46249e28aa0b4b"
}, {
"60388adadf70a1f5d9cb4e097d2c6c57",
"7a6adb4d69c53e0f",
"44b71215cf25368a"
}, {
"4368d2249bd0321eb7c56d5b63a1bfac",
"5a5d7ca2e186c41a",
"91f56dff7281794f"
}, {
"5c60bff27072d01c4513c5eb8f3a38ab",
"80d9c4adcf899635",
"2bb0f1b3c023ed11"
}
};
unsigned char ptct[2][8];
unsigned char tmp[2][8];
unsigned char key[16];
unsigned long l;
symmetric_key skey;
size_t i;
int err, y;
for (i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
zeromem(&skey, sizeof(skey));
l = sizeof(key);
if ((err = base16_decode(tests[i].key, XSTRLEN(tests[i].key), key, &l)) != CRYPT) return err;
l = sizeof(ptct[0]);
if ((err = base16_decode(tests[i].pt, XSTRLEN(tests[i].pt), ptct[0], &l)) != CRYPT) return err;
l = sizeof(ptct[1]);
if ((err = base16_decode(tests[i].ct, XSTRLEN(tests[i].ct), ptct[1], &l)) != CRYPT) return err;
if ((err = tea_setup(key, 16, 0, &skey)) != CRYPT_OK) {
return err;
}
tea_ecb_encrypt(ptct[0], tmp[0], &skey);
tea_ecb_decrypt(tmp[0], tmp[1], &skey);
if (compare_testvector(tmp[0], 8, ptct[0], 8, "TEA Encrypt", i) != 0 ||
compare_testvector(tmp[1], 8, ptct[1], 8, "TEA Decrypt", i) != 0) {
return CRYPT_FAIL_TESTVECTOR;
}
for (y = 0; y < 8; y++) tmp[0][y] = 0;
for (y = 0; y < 1000; y++) tea_ecb_encrypt(tmp[0], tmp[0], &skey);
for (y = 0; y < 1000; y++) tea_ecb_decrypt(tmp[0], tmp[0], &skey);
for (y = 0; y < 8; y++) if (tmp[0][y] != 0) return CRYPT_FAIL_TESTVECTOR;
} 
return CRYPT_OK;
#endif
}
void tea_done(symmetric_key *skey)
{
LTC_UNUSED_PARAM(skey);
}
int tea_keysize(int *keysize)
{
LTC_ARGCHK(keysize != NULL);
if (*keysize < 16) {
return CRYPT_INVALID_KEYSIZE;
}
*keysize = 16;
return CRYPT_OK;
}
#endif
