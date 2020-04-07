




























#include "get_bits.h"
#include "lagarithrac.h"

void ff_lag_rac_init(lag_rac *l, GetBitContext *gb, int length)
{
int i, j, left;




align_get_bits(gb);
left = get_bits_left(gb) >> 3;
l->bytestream_start =
l->bytestream = gb->buffer + get_bits_count(gb) / 8;
l->bytestream_end = l->bytestream_start + left;

l->range = 0x80;
l->low = *l->bytestream >> 1;
l->hash_shift = FFMAX(l->scale, 10) - 10;
l->overread = 0;

for (i = j = 0; i < 1024; i++) {
unsigned r = i << l->hash_shift;
while (l->prob[j + 1] <= r)
j++;
l->range_hash[i] = j;
}
}
