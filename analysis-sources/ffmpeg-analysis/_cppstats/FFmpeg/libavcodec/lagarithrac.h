#include <stdint.h>
#include "libavutil/common.h"
#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "get_bits.h"
typedef struct lag_rac {
AVCodecContext *avctx;
unsigned low;
unsigned range;
unsigned scale; 
unsigned hash_shift; 
const uint8_t *bytestream_start; 
const uint8_t *bytestream; 
const uint8_t *bytestream_end; 
int overread;
#define MAX_OVERREAD 4
uint32_t prob[258]; 
uint8_t range_hash[1024]; 
} lag_rac;
void ff_lag_rac_init(lag_rac *l, GetBitContext *gb, int length);
static inline void lag_rac_refill(lag_rac *l)
{
while (l->range <= 0x800000) {
l->low <<= 8;
l->range <<= 8;
l->low |= 0xff & (AV_RB16(l->bytestream) >> 1);
if (l->bytestream < l->bytestream_end)
l->bytestream++;
else
l->overread++;
}
}
static inline uint8_t lag_get_rac(lag_rac *l)
{
unsigned range_scaled, low_scaled;
int val;
lag_rac_refill(l);
range_scaled = l->range >> l->scale;
if (l->low < range_scaled * l->prob[255]) {
if (l->low < range_scaled * l->prob[1]) {
val = 0;
} else {
low_scaled = l->low / (range_scaled<<(l->hash_shift));
val = l->range_hash[low_scaled];
while (l->low >= range_scaled * l->prob[val + 1])
val++;
}
l->range = range_scaled * (l->prob[val + 1] - l->prob[val]);
} else {
val = 255;
l->range -= range_scaled * l->prob[255];
}
if (!l->range)
l->range = 0x80;
l->low -= range_scaled * l->prob[val];
return val;
}
