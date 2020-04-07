#include "libavutil/bswap.h"
#include "libavutil/common.h"
#include "libavutil/des.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/rc4.h"
#include "asfcrypt.h"
static uint32_t inverse(uint32_t v)
{
uint32_t inverse = v * v * v;
inverse *= 2 - v * inverse;
inverse *= 2 - v * inverse;
inverse *= 2 - v * inverse;
return inverse;
}
static void multiswap_init(const uint8_t keybuf[48], uint32_t keys[12])
{
int i;
for (i = 0; i < 12; i++)
keys[i] = AV_RL32(keybuf + (i << 2)) | 1;
}
static void multiswap_invert_keys(uint32_t keys[12])
{
int i;
for (i = 0; i < 5; i++)
keys[i] = inverse(keys[i]);
for (i = 6; i < 11; i++)
keys[i] = inverse(keys[i]);
}
static uint32_t multiswap_step(const uint32_t keys[12], uint32_t v)
{
int i;
v *= keys[0];
for (i = 1; i < 5; i++) {
v = (v >> 16) | (v << 16);
v *= keys[i];
}
v += keys[5];
return v;
}
static uint32_t multiswap_inv_step(const uint32_t keys[12], uint32_t v)
{
int i;
v -= keys[5];
for (i = 4; i > 0; i--) {
v *= keys[i];
v = (v >> 16) | (v << 16);
}
v *= keys[0];
return v;
}
static uint64_t multiswap_enc(const uint32_t keys[12],
uint64_t key, uint64_t data)
{
uint32_t a = data;
uint32_t b = data >> 32;
uint32_t c;
uint32_t tmp;
a += key;
tmp = multiswap_step(keys, a);
b += tmp;
c = (key >> 32) + tmp;
tmp = multiswap_step(keys + 6, b);
c += tmp;
return ((uint64_t)c << 32) | tmp;
}
static uint64_t multiswap_dec(const uint32_t keys[12],
uint64_t key, uint64_t data)
{
uint32_t a;
uint32_t b;
uint32_t c = data >> 32;
uint32_t tmp = data;
c -= tmp;
b = multiswap_inv_step(keys + 6, tmp);
tmp = c - (key >> 32);
b -= tmp;
a = multiswap_inv_step(keys, tmp);
a -= key;
return ((uint64_t)b << 32) | a;
}
void ff_asfcrypt_dec(const uint8_t key[20], uint8_t *data, int len)
{
struct AVDES *des;
struct AVRC4 *rc4;
int num_qwords = len >> 3;
uint8_t *qwords = data;
uint64_t rc4buff[8] = { 0 };
uint64_t packetkey;
uint32_t ms_keys[12];
uint64_t ms_state;
int i;
if (len < 16) {
for (i = 0; i < len; i++)
data[i] ^= key[i];
return;
}
des = av_des_alloc();
rc4 = av_rc4_alloc();
if (!des || !rc4) {
av_freep(&des);
av_freep(&rc4);
return;
}
av_rc4_init(rc4, key, 12 * 8, 1);
av_rc4_crypt(rc4, (uint8_t *)rc4buff, NULL, sizeof(rc4buff), NULL, 1);
multiswap_init((uint8_t *)rc4buff, ms_keys);
packetkey = AV_RN64(&qwords[num_qwords * 8 - 8]);
packetkey ^= rc4buff[7];
av_des_init(des, key + 12, 64, 1);
av_des_crypt(des, (uint8_t *)&packetkey, (uint8_t *)&packetkey, 1, NULL, 1);
packetkey ^= rc4buff[6];
av_rc4_init(rc4, (uint8_t *)&packetkey, 64, 1);
av_rc4_crypt(rc4, data, data, len, NULL, 1);
ms_state = 0;
for (i = 0; i < num_qwords - 1; i++, qwords += 8)
ms_state = multiswap_enc(ms_keys, ms_state, AV_RL64(qwords));
multiswap_invert_keys(ms_keys);
packetkey = (packetkey << 32) | (packetkey >> 32);
packetkey = av_le2ne64(packetkey);
packetkey = multiswap_dec(ms_keys, ms_state, packetkey);
AV_WL64(qwords, packetkey);
av_free(rc4);
av_free(des);
}
