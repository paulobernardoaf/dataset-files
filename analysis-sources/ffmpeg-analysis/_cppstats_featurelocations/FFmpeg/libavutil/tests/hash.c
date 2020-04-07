



















#include <stdio.h>
#include <string.h>

#include "libavutil/hash.h"

#define SRC_BUF_SIZE 64
#define DST_BUF_SIZE (AV_HASH_MAX_SIZE * 8)

int main(void)
{
struct AVHashContext *ctx = NULL;
int i, j, numhashes = 0;
static const uint8_t src[SRC_BUF_SIZE] = { 0 };
uint8_t dst[DST_BUF_SIZE];

while (av_hash_names(numhashes))
numhashes++;

for (i = 0; i < numhashes; i++) {
if (av_hash_alloc(&ctx, av_hash_names(i)) < 0)
return 1;

av_hash_init(ctx);
av_hash_update(ctx, src, SRC_BUF_SIZE);
memset(dst, 0, DST_BUF_SIZE);
av_hash_final_hex(ctx, dst, DST_BUF_SIZE);
printf("%s hex: %s\n", av_hash_get_name(ctx), dst);

av_hash_init(ctx);
av_hash_update(ctx, src, SRC_BUF_SIZE);
av_hash_final_bin(ctx, dst, DST_BUF_SIZE);
printf("%s bin: ", av_hash_get_name(ctx));
for (j = 0; j < av_hash_get_size(ctx); j++) {
printf("%#x ", dst[j]);
}
printf("\n");

av_hash_init(ctx);
av_hash_update(ctx, src, SRC_BUF_SIZE);
av_hash_final_b64(ctx, dst, DST_BUF_SIZE);
printf("%s b64: %s\n", av_hash_get_name(ctx), dst);
av_hash_freep(&ctx);
}
return 0;
}
