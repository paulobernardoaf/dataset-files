#include <r_hash.h>
void crc_init (R_CRC_CTX *ctx, utcrc crc, ut32 size, int reflect, utcrc poly, utcrc xout) {
ctx->crc = crc;
ctx->size = size;
ctx->reflect = reflect;
ctx->poly = poly;
ctx->xout = xout;
}
void crc_update (R_CRC_CTX *ctx, const ut8 *data, ut32 sz) {
utcrc crc, d;
int i, j;
crc = ctx->crc;
for (i = 0; i < sz; i++) {
d = data[i];
if (ctx->reflect) {
for (j = 0; j < 4; j++) {
if (((d >> j) ^ (d >> (7 - j))) & 1) {
d ^= (1 << j) ^ (1 << (7 - j));
}
}
}
crc ^= d << (ctx->size - 8);
for (j = 0; j < 8; j++) {
crc = ((crc >> (ctx->size - 1)) & 1? ctx->poly: 0) ^ (crc << 1);
}
}
ctx->crc = crc;
}
static void crc_final (R_CRC_CTX *ctx, utcrc *r) {
utcrc crc;
int i;
crc = ctx->crc;
crc &= (((UTCRC_C(1) << (ctx->size - 1)) - 1) << 1) | 1;
if (ctx->reflect) {
for (i = 0; i < (ctx->size >> 1); i++) {
if (((crc >> i) ^ (crc >> (ctx->size - 1 - i))) & 1) {
crc ^= (UTCRC_C(1) << i) ^ (UTCRC_C(1) << (ctx->size - 1 - i));
}
}
}
*r = crc ^ ctx->xout;
}
#define CRC_PRESET(crc, size, reflect, poly, xout) { UTCRC_C(crc), (size), (reflect), UTCRC_C(poly), UTCRC_C(xout) }
R_CRC_CTX crc_presets[] = {
CRC_PRESET (0x00 , 8, 0, 0x07 , 0x00 ), 
#if R_HAVE_CRC8_EXTRA
CRC_PRESET (0xFF , 8, 0, 0x9B , 0x00 ), 
CRC_PRESET (0x00 , 8, 1, 0x39 , 0x00 ), 
CRC_PRESET (0x00 , 8, 0, 0xD5 , 0x00 ), 
CRC_PRESET (0xFF , 8, 1, 0x1D , 0x00 ), 
CRC_PRESET (0xFD , 8, 0, 0x1D , 0x00 ), 
CRC_PRESET (0x00 , 8, 0, 0x07 , 0x55 ), 
CRC_PRESET (0x00 , 8, 1, 0x31 , 0x00 ), 
CRC_PRESET (0xFF , 8, 1, 0x07 , 0x00 ), 
CRC_PRESET (0x00 , 8, 1, 0x9B , 0x00 ), 
#endif 
#if R_HAVE_CRC15_EXTRA
CRC_PRESET (0x0000 , 15, 0, 0x4599 , 0x0000 ), 
#endif 
CRC_PRESET (0x0000 , 16, 1, 0x8005 , 0x0000 ), 
CRC_PRESET (0xFFFF , 16, 0, 0x1021 , 0x0000 ), 
CRC_PRESET (0xFFFF , 16, 1, 0x8005 , 0xFFFF ), 
CRC_PRESET (0xFFFF , 16, 1, 0x1021 , 0xFFFF ), 
#if R_HAVE_CRC16_EXTRA
CRC_PRESET (0x1D0F , 16, 0, 0x1021 , 0x0000 ), 
CRC_PRESET (0x0000 , 16, 0, 0x8005 , 0x0000 ), 
CRC_PRESET (0xFFFF , 16, 0, 0xC867 , 0x0000 ), 
CRC_PRESET (0x800D , 16, 0, 0x8005 , 0x0000 ), 
CRC_PRESET (0x0000 , 16, 0, 0x0589 , 0x0001 ), 
CRC_PRESET (0x0000 , 16, 0, 0x0589 , 0x0000 ), 
CRC_PRESET (0x0000 , 16, 1, 0x3D65 , 0xFFFF ), 
CRC_PRESET (0x0000 , 16, 0, 0x3D65 , 0xFFFF ), 
CRC_PRESET (0xFFFF , 16, 0, 0x1021 , 0xFFFF ), 
CRC_PRESET (0x0000 , 16, 1, 0x8005 , 0xFFFF ), 
CRC_PRESET (0xFFFF , 16, 1, 0x1021 , 0x0000 ), 
CRC_PRESET (0xB2AA , 16, 1, 0x1021 , 0x0000 ), 
CRC_PRESET (0x0000 , 16, 0, 0x8BB7 , 0x0000 ), 
CRC_PRESET (0x0000 , 16, 0, 0xA097 , 0x0000 ), 
CRC_PRESET (0x89EC , 16, 1, 0x1021 , 0x0000 ), 
CRC_PRESET (0xC6C6 , 16, 1, 0x1021 , 0x0000 ), 
CRC_PRESET (0x0000 , 16, 1, 0x1021 , 0x0000 ), 
CRC_PRESET (0xFFFF , 16, 1, 0x8005 , 0x0000 ), 
CRC_PRESET (0xFFFF , 16, 1, 0x1021 , 0xFFFF ), 
CRC_PRESET (0x0000 , 16, 0, 0x1021 , 0x0000 ), 
#endif 
#if R_HAVE_CRC24
CRC_PRESET (0xB704CE , 24, 0, 0x864CFB , 0x000000 ), 
#endif 
CRC_PRESET (0xFFFFFFFF, 32, 1, 0x04C11DB7, 0xFFFFFFFF ), 
CRC_PRESET (0x00000000, 32, 0, 0x80000011, 0x00000000 ), 
CRC_PRESET (0xFFFFFFFF, 32, 1, 0x1EDC6F41, 0xFFFFFFFF ), 
#if R_HAVE_CRC32_EXTRA
CRC_PRESET (0xFFFFFFFF, 32, 0, 0x04C11DB7, 0xFFFFFFFF ), 
CRC_PRESET (0xFFFFFFFF, 32, 1, 0xA833982B, 0xFFFFFFFF ), 
CRC_PRESET (0xFFFFFFFF, 32, 0, 0x04C11DB7, 0x00000000 ), 
CRC_PRESET (0x00000000, 32, 0, 0x04C11DB7, 0xFFFFFFFF ), 
CRC_PRESET (0x00000000, 32, 0, 0x814141AB, 0x00000000 ), 
CRC_PRESET (0xFFFFFFFF, 32, 1, 0x04C11DB7, 0x00000000 ), 
CRC_PRESET (0x00000000, 32, 0, 0x000000AF, 0x00000000 ), 
#endif 
#if R_HAVE_CRC64
CRC_PRESET (0x0000000000000000, 64, 0, 0x42F0E1EBA9EA3693, 0x0000000000000000 ), 
#endif 
#if R_HAVE_CRC64_EXTRA
CRC_PRESET (0x0000000000000000, 64, 0, 0x42F0E1EBA9EA3693, 0x0000000000000000 ), 
CRC_PRESET (0xFFFFFFFFFFFFFFFF, 64, 0, 0x42F0E1EBA9EA3693, 0xFFFFFFFFFFFFFFFF ), 
CRC_PRESET (0xFFFFFFFFFFFFFFFF, 64, 1, 0x42F0E1EBA9EA3693, 0xFFFFFFFFFFFFFFFF ), 
CRC_PRESET (0xFFFFFFFFFFFFFFFF, 64, 1, 0x000000000000001b, 0xFFFFFFFFFFFFFFFF ), 
#endif 
};
void crc_init_preset (R_CRC_CTX *ctx, enum CRC_PRESETS preset) {
ctx->crc = crc_presets[preset].crc;
ctx->size = crc_presets[preset].size;
ctx->reflect = crc_presets[preset].reflect;
ctx->poly = crc_presets[preset].poly;
ctx->xout = crc_presets[preset].xout;
}
utcrc r_hash_crc_preset (const ut8 *data, ut32 size, enum CRC_PRESETS preset) {
if (!data || !size || preset >= CRC_PRESET_SIZE) {
return 0;
}
utcrc r;
R_CRC_CTX crcctx;
crc_init_preset (&crcctx, preset);
crc_update (&crcctx, data, size);
crc_final (&crcctx, &r);
return r;
}
