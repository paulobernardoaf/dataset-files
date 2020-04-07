

















#include <stdint.h>
#include <string.h>

#include "libavutil/lfg.h"
#include "libavutil/log.h"

#include "libavcodec/rangecoder.h"

#define SIZE 1240

int main(void)
{
RangeCoder c;
uint8_t b[9 * SIZE] = {0};
uint8_t r[9 * SIZE];
int i, p, actual_length, version;
uint8_t state[10];
AVLFG prng;

av_lfg_init(&prng, 1);
for (version = 0; version < 2; version++) {
for (p = 0; p< 1024; p++) {
ff_init_range_encoder(&c, b, SIZE);
ff_build_rac_states(&c, (1LL << 32) / 20, 128 + 64 + 32 + 16);

memset(state, 128, sizeof(state));

for (i = 0; i < SIZE; i++)
r[i] = av_lfg_get(&prng) % 7;

for (i = 0; i < SIZE; i++)
put_rac(&c, state, r[i] & 1);

actual_length = ff_rac_terminate(&c, version);

ff_init_range_decoder(&c, b, version ? SIZE : actual_length);

memset(state, 128, sizeof(state));

for (i = 0; i < SIZE; i++)
if ((r[i] & 1) != get_rac(&c, state)) {
av_log(NULL, AV_LOG_ERROR, "rac failure at %d pass %d version %d\n", i, p, version);
return 1;
}

if (ff_rac_check_termination(&c, version) < 0) {
av_log(NULL, AV_LOG_ERROR, "rac failure at termination pass %d version %d\n", p, version);
return 1;
}
if (c.bytestream - c.bytestream_start - actual_length != version) {
av_log(NULL, AV_LOG_ERROR, "rac failure at pass %d version %d\n", p, version);
return 1;
}
}
}

return 0;
}
