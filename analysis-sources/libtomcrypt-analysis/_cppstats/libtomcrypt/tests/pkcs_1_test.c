#include <tomcrypt_test.h>
#if defined(LTC_PKCS_1)
#if defined(LTC_TEST_REAL_RAND)
#define LTC_TEST_RAND_SEED time(NULL)
#else
#define LTC_TEST_RAND_SEED 23
#endif
int pkcs_1_test(void)
{
unsigned char buf[3][128];
int res1, res2, res3, prng_idx, hash_idx;
unsigned long x, y, l1, l2, l3, i1, i2, lparamlen, saltlen, modlen;
static const unsigned char lparam[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
hash_idx = find_hash("sha1");
prng_idx = find_prng("yarrow");
if (hash_idx == -1 || prng_idx == -1) {
fprintf(stderr, "pkcs_1 tests require sha1/yarrow");
return 1;
}
srand(LTC_TEST_RAND_SEED);
for (x = 0; x < 100; x++) {
zeromem(buf, sizeof(buf));
l3 = (rand() & 31) + 8;
for (y = 0; y < l3; y++) buf[0][y] = rand() & 255;
lparamlen = abs(rand()) % 17;
saltlen = abs(rand()) % 17;
modlen = 800 + (abs(rand()) % 224);
l1 = sizeof(buf[1]);
DO(pkcs_1_oaep_encode(buf[0], l3, lparam, lparamlen, modlen, &yarrow_prng, prng_idx, hash_idx, buf[1], &l1));
l2 = sizeof(buf[2]);
DO(pkcs_1_oaep_decode(buf[1], l1, lparam, lparamlen, modlen, hash_idx, buf[2], &l2, &res1));
if (res1 != 1 || l2 != l3 || memcmp(buf[2], buf[0], l3) != 0) {
fprintf(stderr, "Outsize == %lu, should have been %lu, res1 = %d, lparamlen = %lu, msg contents follow.\n", l2, l3, res1, lparamlen);
fprintf(stderr, "ORIGINAL:\n");
for (x = 0; x < l3; x++) {
fprintf(stderr, "%02x ", buf[0][x]);
}
fprintf(stderr, "\nRESULT:\n");
for (x = 0; x < l2; x++) {
fprintf(stderr, "%02x ", buf[2][x]);
}
fprintf(stderr, "\n\n");
return 1;
}
l1 = sizeof(buf[1]);
DO(pkcs_1_pss_encode(buf[0], l3, saltlen, &yarrow_prng, prng_idx, hash_idx, modlen, buf[1], &l1));
DO(pkcs_1_pss_decode(buf[0], l3, buf[1], l1, saltlen, hash_idx, modlen, &res1));
buf[0][i1 = abs(rand()) % l3] ^= 1;
DO(pkcs_1_pss_decode(buf[0], l3, buf[1], l1, saltlen, hash_idx, modlen, &res2));
buf[0][i1] ^= 1;
buf[1][i2 = abs(rand()) % (l1 - 1)] ^= 1;
pkcs_1_pss_decode(buf[0], l3, buf[1], l1, saltlen, hash_idx, modlen, &res3);
if (!(res1 == 1 && res2 == 0 && res3 == 0)) {
fprintf(stderr, "PSS failed: %d, %d, %d, %lu, %lu\n", res1, res2, res3, l3, saltlen);
return 1;
}
}
return 0;
}
#else
int pkcs_1_test(void)
{
return CRYPT_NOP;
}
#endif
