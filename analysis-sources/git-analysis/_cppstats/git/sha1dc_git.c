#include "cache.h"
#if defined(DC_SHA1_EXTERNAL)
void git_SHA1DCInit(SHA1_CTX *ctx)
{
SHA1DCInit(ctx);
SHA1DCSetSafeHash(ctx, 0);
}
#endif
void git_SHA1DCFinal(unsigned char hash[20], SHA1_CTX *ctx)
{
if (!SHA1DCFinal(hash, ctx))
return;
die("SHA-1 appears to be part of a collision attack: %s",
hash_to_hex_algop(hash, &hash_algos[GIT_HASH_SHA1]));
}
void git_SHA1DCUpdate(SHA1_CTX *ctx, const void *vdata, unsigned long len)
{
const char *data = vdata;
while (len > INT_MAX) {
SHA1DCUpdate(ctx, data, INT_MAX);
data += INT_MAX;
len -= INT_MAX;
}
SHA1DCUpdate(ctx, data, len);
}
