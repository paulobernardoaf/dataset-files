#include "tomcrypt_private.h"
#if defined(LTC_BASE64) || defined (LTC_BASE64_URL)
#if defined(LTC_BASE64)
static const unsigned char map_base64[256] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 253, 255,
255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63,
52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
255, 254, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6,
7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255,
255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255 };
#endif 
static const unsigned char map_base64url[] = {
#if defined(LTC_BASE64_URL)
255, 255, 255, 255, 255, 255, 255, 255, 255, 253, 253, 255,
255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 253, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255,
52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,
255, 254, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6,
7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 63,
255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255
#endif 
};
enum {
insane = 0,
strict = 1,
relaxed = 2
};
static int _base64_decode_internal(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen,
const unsigned char *map, int mode)
{
unsigned long t, x, y, z;
unsigned char c;
int g;
LTC_ARGCHK(in != NULL);
LTC_ARGCHK(out != NULL);
LTC_ARGCHK(outlen != NULL);
g = 0; 
for (x = y = z = t = 0; x < inlen; x++) {
if ((in[x] == 0) && (x == (inlen - 1)) && (mode != strict)) {
continue; 
}
c = map[(unsigned char)in[x]&0xFF];
if (c == 254) {
g++;
continue;
}
if (c == 253) {
if (mode == strict) {
return CRYPT_INVALID_PACKET;
}
continue; 
}
if (c == 255) {
if (mode == insane) {
continue; 
}
return CRYPT_INVALID_PACKET;
}
if ((g > 0) && (mode != insane)) {
return CRYPT_INVALID_PACKET;
}
t = (t<<6)|c;
if (++y == 4) {
if (z + 3 > *outlen) return CRYPT_BUFFER_OVERFLOW;
out[z++] = (unsigned char)((t>>16)&255);
out[z++] = (unsigned char)((t>>8)&255);
out[z++] = (unsigned char)(t&255);
y = t = 0;
}
}
if (y != 0) {
if (y == 1) return CRYPT_INVALID_PACKET;
if (((y + g) != 4) && (mode == strict) && (map != map_base64url)) return CRYPT_INVALID_PACKET;
t = t << (6 * (4 - y));
if (z + y - 1 > *outlen) return CRYPT_BUFFER_OVERFLOW;
if (y >= 2) out[z++] = (unsigned char) ((t >> 16) & 255);
if (y == 3) out[z++] = (unsigned char) ((t >> 8) & 255);
}
*outlen = z;
return CRYPT_OK;
}
#if defined(LTC_BASE64)
int base64_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
return _base64_decode_internal(in, inlen, out, outlen, map_base64, insane);
}
int base64_strict_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
return _base64_decode_internal(in, inlen, out, outlen, map_base64, strict);
}
int base64_sane_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
return _base64_decode_internal(in, inlen, out, outlen, map_base64, relaxed);
}
#endif 
#if defined(LTC_BASE64_URL)
int base64url_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
return _base64_decode_internal(in, inlen, out, outlen, map_base64url, insane);
}
int base64url_strict_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
return _base64_decode_internal(in, inlen, out, outlen, map_base64url, strict);
}
int base64url_sane_decode(const char *in, unsigned long inlen,
unsigned char *out, unsigned long *outlen)
{
return _base64_decode_internal(in, inlen, out, outlen, map_base64url, relaxed);
}
#endif 
#endif
