#include <r_util.h>
#include <zlib.h>
#define MAXOUT 50000000
static const char *gzerr(int n) {
const char *errors[] = {
"",
"file error", 
"stream error", 
"data error", 
"insufficient memory", 
"buffer error", 
"incompatible version",
};
if (n<1 || n>6) {
return "unknown";
}
return errors[n];
}
R_API ut8 *r_inflate(const ut8 *src, int srcLen, int *srcConsumed, int *dstLen) {
int err = 0;
int out_size = 0;
ut8 *dst = NULL;
ut8 *tmp_ptr;
z_stream stream;
if (srcLen <= 0) {
return NULL;
}
memset (&stream, 0, sizeof (z_stream));
stream.avail_in = srcLen;
stream.next_in = (Bytef *) src;
stream.zalloc = Z_NULL;
stream.zfree = Z_NULL;
stream.opaque = Z_NULL;
if (inflateInit2 (&stream, MAX_WBITS + 32) != Z_OK) {
return NULL;
}
do {
if (stream.avail_out == 0) {
tmp_ptr = realloc (dst, stream.total_out + srcLen * 2);
if (!tmp_ptr) {
goto err_exit;
}
dst = tmp_ptr;
out_size += srcLen*2;
if (out_size > MAXOUT) {
goto err_exit;
}
stream.next_out = dst + stream.total_out;
stream.avail_out = srcLen * 2;
}
err = inflate (&stream, Z_NO_FLUSH);
if (err<0) {
eprintf ("inflate error: %d %s\n",
err, gzerr (-err));
goto err_exit;
}
} while (err != Z_STREAM_END);
if (dstLen) {
*dstLen = stream.total_out;
}
if (srcConsumed) {
*srcConsumed = (const ut8*)stream.next_in-(const ut8*)src;
}
inflateEnd (&stream);
return dst;
err_exit:
inflateEnd (&stream);
free (dst);
return NULL;
}
