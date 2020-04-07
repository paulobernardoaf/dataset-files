



















#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"
#include "libavutil/murmur3.h"

int main(void)
{
int i;
uint8_t hash_result[16] = {0};
struct AVMurMur3 *ctx = av_murmur3_alloc();
#if 1
uint8_t in[256] = {0};
uint8_t *hashes = av_mallocz(256 * 16);
for (i = 0; i < 256; i++)
{
in[i] = i;
av_murmur3_init_seeded(ctx, 256 - i);

av_murmur3_update(ctx, in, i);
av_murmur3_final(ctx, hashes + 16 * i);
}
av_murmur3_init_seeded(ctx, 0);
av_murmur3_update(ctx, hashes, 256 * 16);
av_murmur3_final(ctx, hash_result);
av_free(hashes);
av_freep(&ctx);
printf("result: 0x%"PRIx64" 0x%"PRIx64"\n", AV_RL64(hash_result), AV_RL64(hash_result + 8));

return AV_RL32(hash_result) != 0x6384ba69;
#else
uint8_t *in = av_mallocz(512*1024);
av_murmur3_init(ctx);
for (i = 0; i < 40*1024; i++)
av_murmur3_update(ctx, in, 512*1024);
av_murmur3_final(ctx, hash_result);
av_free(in);
return hash_result[0];
#endif
}
