#include <r_hash.h>
#include "r_util.h"
#if USE_LIB_XXHASH
#include <xxhash.h>
#else
#include "xxhash.h"
#endif
R_LIB_VERSION (r_hash);
static const struct {
const char *name;
ut64 bit;
} hash_name_bytes[] = {
{ "all", UT64_MAX },
{ "xor", R_HASH_XOR },
{ "xorpair", R_HASH_XORPAIR },
{ "md4", R_HASH_MD4 },
{ "md5", R_HASH_MD5 },
{ "sha1", R_HASH_SHA1 },
{ "sha256", R_HASH_SHA256 },
{ "sha384", R_HASH_SHA384 },
{ "sha512", R_HASH_SHA512 },
{ "adler32", R_HASH_ADLER32 },
{ "xxhash", R_HASH_XXHASH },
{ "parity", R_HASH_PARITY },
{ "entropy", R_HASH_ENTROPY },
{ "hamdist", R_HASH_HAMDIST },
{ "pcprint", R_HASH_PCPRINT },
{ "mod255", R_HASH_MOD255 },
{ "luhn", R_HASH_LUHN },
{ "fletcher8", R_HASH_FLETCHER8 },
{ "fletcher16", R_HASH_FLETCHER16 },
{ "fletcher32", R_HASH_FLETCHER32 },
{ "fletcher64", R_HASH_FLETCHER64 },
{ "crc8smbus", R_HASH_CRC8_SMBUS },
#if R_HAVE_CRC8_EXTRA
{ "crc8cdma2000", R_HASH_CRC8_CDMA2000 },
{ "crc8darc", R_HASH_CRC8_DARC },
{ "crc8dvbs2", R_HASH_CRC8_DVB_S2 },
{ "crc8ebu", R_HASH_CRC8_EBU },
{ "crc8icode", R_HASH_CRC8_ICODE },
{ "crc8itu", R_HASH_CRC8_ITU },
{ "crc8maxim", R_HASH_CRC8_MAXIM },
{ "crc8rohc", R_HASH_CRC8_ROHC },
{ "crc8wcdma", R_HASH_CRC8_WCDMA },
#endif 
#if R_HAVE_CRC15_EXTRA
{ "crc15can", R_HASH_CRC15_CAN },
#endif 
{ "crc16", R_HASH_CRC16 },
{ "crc16hdlc", R_HASH_CRC16_HDLC },
{ "crc16usb", R_HASH_CRC16_USB },
{ "crc16citt", R_HASH_CRC16_CITT },
#if R_HAVE_CRC16_EXTRA
{ "crc16augccitt", R_HASH_CRC16_AUG_CCITT },
{ "crc16buypass", R_HASH_CRC16_BUYPASS },
{ "crc16cdma2000", R_HASH_CRC16_CDMA2000 },
{ "crc16dds110", R_HASH_CRC16_DDS110 },
{ "crc16dectr", R_HASH_CRC16_DECT_R },
{ "crc16dectx", R_HASH_CRC16_DECT_X },
{ "crc16dnp", R_HASH_CRC16_DNP },
{ "crc16en13757", R_HASH_CRC16_EN13757 },
{ "crc16genibus", R_HASH_CRC16_GENIBUS },
{ "crc16maxim", R_HASH_CRC16_MAXIM },
{ "crc16mcrf4xx", R_HASH_CRC16_MCRF4XX },
{ "crc16riello", R_HASH_CRC16_RIELLO },
{ "crc16t10dif", R_HASH_CRC16_T10_DIF },
{ "crc16teledisk", R_HASH_CRC16_TELEDISK },
{ "crc16tms37157", R_HASH_CRC16_TMS37157 },
{ "crca", R_HASH_CRCA },
{ "crc16kermit", R_HASH_CRC16_KERMIT },
{ "crc16modbus", R_HASH_CRC16_MODBUS },
{ "crc16x25", R_HASH_CRC16_X25 },
{ "crc16xmodem", R_HASH_CRC16_XMODEM },
#endif 
#if R_HAVE_CRC24
{ "crc24", R_HASH_CRC24 },
#endif 
{ "crc32", R_HASH_CRC32 },
{ "crc32c", R_HASH_CRC32C },
{ "crc32ecma267", R_HASH_CRC32_ECMA_267 },
#if R_HAVE_CRC32_EXTRA
{ "crc32bzip2", R_HASH_CRC32_BZIP2 },
{ "crc32d", R_HASH_CRC32D },
{ "crc32mpeg2", R_HASH_CRC32_MPEG2 },
{ "crc32posix", R_HASH_CRC32_POSIX },
{ "crc32q", R_HASH_CRC32Q },
{ "crc32jamcrc", R_HASH_CRC32_JAMCRC },
{ "crc32xfer", R_HASH_CRC32_XFER },
#endif 
#if R_HAVE_CRC64
{ "crc64", R_HASH_CRC64 },
#endif 
#if R_HAVE_CRC64_EXTRA
{ "crc64ecma", R_HASH_CRC64_ECMA182 },
{ "crc64we", R_HASH_CRC64_WE },
{ "crc64xz", R_HASH_CRC64_XZ },
{ "crc64iso", R_HASH_CRC64_ISO },
#endif 
{ NULL, 0 }
};
R_API int r_hash_pcprint(const ut8 *buffer, ut64 len) {
const ut8 *end = buffer + len;
int n;
if (len < 1) {
return 0;
}
for (n = 0; buffer < end; buffer++) {
if (IS_PRINTABLE (*buffer)) {
n++;
}
}
return ((100 * n) / len);
}
R_API int r_hash_parity(const ut8 *buf, ut64 len) {
const ut8 *end = buf + len;
ut32 ones = 0;
for (; buf < end; buf++) {
ut8 x = buf[0];
ones += ((x & 128) ? 1 : 0) + ((x & 64) ? 1 : 0) + ((x & 32) ? 1 : 0) + ((x & 16) ? 1 : 0) +
((x & 8) ? 1 : 0) + ((x & 4) ? 1 : 0) + ((x & 2) ? 1 : 0) + ((x & 1) ? 1 : 0);
}
return ones % 2;
}
R_API ut16 r_hash_xorpair(const ut8 *a, ut64 len) {
ut16 result = 0, *b = (ut16 *)a;
for (len >>= 1; len--; b++) {
result ^= *b;
}
return result;
}
R_API ut8 r_hash_xor(const ut8 *b, ut64 len) {
ut8 res = 0;
for (; len--; b++) {
res ^= *b;
}
return res;
}
R_API ut8 r_hash_mod255(const ut8 *b, ut64 len) {
int i, c = 0;
for (i = 0; i < len; i++) {
c += b[i];
}
return c % 255;
}
R_API ut32 r_hash_xxhash(const ut8 *buf, ut64 len) {
return XXH32 (buf, (size_t)len, 0);
}
R_API ut8 r_hash_deviation(const ut8 *b, ut64 len) {
int i, c;
for (c = i = 0, len--; i < len; i++) {
c += R_ABS (b[i + 1] - b[i]);
}
return c;
}
R_API const char *r_hash_name(ut64 bit) {
int i;
for (i = 1; hash_name_bytes[i].bit; i++) {
if (bit & hash_name_bytes[i].bit) {
return hash_name_bytes[i].name;
}
}
return "";
}
R_API int r_hash_size(ut64 algo) {
#define ALGOBIT(x)if (algo & R_HASH_##x) {return R_HASH_SIZE_##x;}
ALGOBIT (FLETCHER8);
ALGOBIT (FLETCHER16);
ALGOBIT (FLETCHER32);
ALGOBIT (FLETCHER64);
ALGOBIT (MD4);
ALGOBIT (MD5);
ALGOBIT (SHA1);
ALGOBIT (SHA256);
ALGOBIT (SHA384);
ALGOBIT (SHA512);
ALGOBIT (XXHASH);
ALGOBIT (ADLER32);
ALGOBIT (PARITY);
ALGOBIT (ENTROPY);
ALGOBIT (HAMDIST);
ALGOBIT (XOR);
ALGOBIT (XORPAIR);
ALGOBIT (MOD255);
ALGOBIT (PCPRINT);
ALGOBIT (LUHN);
ALGOBIT (CRC8_SMBUS);
#if R_HAVE_CRC8_EXTRA
ALGOBIT (CRC8_CDMA2000);
ALGOBIT (CRC8_DARC);
ALGOBIT (CRC8_DVB_S2);
ALGOBIT (CRC8_EBU);
ALGOBIT (CRC8_ICODE);
ALGOBIT (CRC8_ITU);
ALGOBIT (CRC8_MAXIM);
ALGOBIT (CRC8_ROHC);
ALGOBIT (CRC8_WCDMA);
#endif 
#if R_HAVE_CRC15_EXTRA
ALGOBIT (CRC15_CAN);
#endif 
ALGOBIT (CRC16);
ALGOBIT (CRC16_HDLC);
ALGOBIT (CRC16_USB);
ALGOBIT (CRC16_CITT);
#if R_HAVE_CRC16_EXTRA
ALGOBIT (CRC16_AUG_CCITT);
ALGOBIT (CRC16_BUYPASS)
ALGOBIT (CRC16_CDMA2000);
ALGOBIT (CRC16_DDS110);
ALGOBIT (CRC16_DECT_R);
ALGOBIT (CRC16_DECT_X);
ALGOBIT (CRC16_DNP);
ALGOBIT (CRC16_EN13757);
ALGOBIT (CRC16_GENIBUS);
ALGOBIT (CRC16_MAXIM);
ALGOBIT (CRC16_MCRF4XX);
ALGOBIT (CRC16_RIELLO);
ALGOBIT (CRC16_T10_DIF);
ALGOBIT (CRC16_TELEDISK);
ALGOBIT (CRC16_TMS37157);
ALGOBIT (CRCA);
ALGOBIT (CRC16_KERMIT);
ALGOBIT (CRC16_MODBUS);
ALGOBIT (CRC16_X25);
ALGOBIT (CRC16_XMODEM);
#endif 
#if R_HAVE_CRC24
ALGOBIT (CRC24);
#endif 
ALGOBIT (CRC32);
ALGOBIT (CRC32C);
ALGOBIT (CRC32_ECMA_267);
#if R_HAVE_CRC32_EXTRA
ALGOBIT (CRC32_BZIP2);
ALGOBIT (CRC32D);
ALGOBIT (CRC32_MPEG2);
ALGOBIT (CRC32_POSIX);
ALGOBIT (CRC32Q);
ALGOBIT (CRC32_JAMCRC);
ALGOBIT (CRC32_XFER);
#endif 
#if R_HAVE_CRC64
ALGOBIT (CRC64);
#endif 
#if R_HAVE_CRC64_EXTRA
ALGOBIT (CRC64_ECMA182);
ALGOBIT (CRC64_WE);
ALGOBIT (CRC64_XZ);
ALGOBIT (CRC64_ISO);
#endif 
return 0;
}
R_API ut64 r_hash_name_to_bits(const char *name) {
char tmp[128];
int i;
const char *ptr = name;
ut64 ret = 0;
if (!ptr) {
return ret;
}
do {
for (i = 0; *ptr && *ptr != ',' && i < sizeof (tmp) - 1; i++) {
tmp[i] = tolower ((ut8)*ptr++);
}
tmp[i] = '\0';
for (i = 0; hash_name_bytes[i].name; i++) {
if (!strcmp (tmp, hash_name_bytes[i].name)) {
ret |= hash_name_bytes[i].bit;
break;
}
}
if (*ptr) {
ptr++;
}
} while (*ptr);
return ret;
}
R_API void r_hash_do_spice(RHash *ctx, ut64 algo, int loops, RHashSeed *seed) {
ut8 buf[1024];
int i, len, hlen = r_hash_size (algo);
for (i = 0; i < loops; i++) {
if (seed) {
if (seed->prefix) {
memcpy (buf, seed->buf, seed->len);
memcpy (buf + seed->len, ctx->digest, hlen);
} else {
memcpy (buf, ctx->digest, hlen);
memcpy (buf + hlen, seed->buf, seed->len);
}
len = hlen + seed->len;
} else {
memcpy (buf, ctx->digest, hlen);
len = hlen;
}
(void)r_hash_calculate (ctx, algo, buf, len);
}
}
R_API char *r_hash_to_string(RHash *ctx, const char *name, const ut8 *data, int len) {
ut64 algo = r_hash_name_to_bits (name);
char *digest_hex = NULL;
RHash *myctx = NULL;
int i, digest_size;
if (!algo || !data) {
return NULL;
}
if (!ctx) {
myctx = ctx = r_hash_new (true, algo);
}
r_hash_do_begin (ctx, algo);
digest_size = r_hash_calculate (ctx, algo, data, len);
r_hash_do_end (ctx, algo);
if (digest_size == 0) {
digest_hex = calloc (16, 1);
snprintf (digest_hex, 15, "%02.8f", ctx->entropy);
} else if (digest_size > 0) {
if (digest_size * 2 < digest_size) {
digest_hex = NULL;
} else {
digest_hex = malloc ((digest_size * 2) + 1);
if (digest_hex) {
for (i = 0; i < digest_size; i++) {
sprintf (digest_hex + (i * 2), "%02x", ctx->digest[i]);
}
digest_hex[digest_size * 2] = 0;
}
}
}
r_hash_free (myctx);
return digest_hex;
}
