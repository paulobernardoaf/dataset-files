#include "avcodec.h"
#include "sbc.h"
static const int sbc_offset4[4][4] = {
{ -1, 0, 0, 0 },
{ -2, 0, 0, 1 },
{ -2, 0, 0, 1 },
{ -2, 0, 0, 1 }
};
static const int sbc_offset8[4][8] = {
{ -2, 0, 0, 0, 0, 0, 0, 1 },
{ -3, 0, 0, 0, 0, 0, 1, 2 },
{ -4, 0, 0, 0, 0, 0, 1, 2 },
{ -4, 0, 0, 0, 0, 0, 1, 2 }
};
uint8_t ff_sbc_crc8(const AVCRC *ctx, const uint8_t *data, size_t len)
{
size_t byte_length = len >> 3;
int bit_length = len & 7;
uint8_t crc;
crc = av_crc(ctx, 0x0F, data, byte_length);
if (bit_length) {
uint8_t bits = data[byte_length];
while (bit_length--) {
int8_t mask = bits ^ crc;
crc = (crc << 1) ^ ((mask >> 7) & 0x1D);
bits <<= 1;
}
}
return crc;
}
void ff_sbc_calculate_bits(const struct sbc_frame *frame, int (*bits)[8])
{
int subbands = frame->subbands;
uint8_t sf = frame->frequency;
if (frame->mode == MONO || frame->mode == DUAL_CHANNEL) {
int bitneed[2][8], loudness, max_bitneed, bitcount, slicecount, bitslice;
int ch, sb;
for (ch = 0; ch < frame->channels; ch++) {
max_bitneed = 0;
if (frame->allocation == SNR) {
for (sb = 0; sb < subbands; sb++) {
bitneed[ch][sb] = frame->scale_factor[ch][sb];
if (bitneed[ch][sb] > max_bitneed)
max_bitneed = bitneed[ch][sb];
}
} else {
for (sb = 0; sb < subbands; sb++) {
if (frame->scale_factor[ch][sb] == 0)
bitneed[ch][sb] = -5;
else {
if (subbands == 4)
loudness = frame->scale_factor[ch][sb] - sbc_offset4[sf][sb];
else
loudness = frame->scale_factor[ch][sb] - sbc_offset8[sf][sb];
if (loudness > 0)
bitneed[ch][sb] = loudness / 2;
else
bitneed[ch][sb] = loudness;
}
if (bitneed[ch][sb] > max_bitneed)
max_bitneed = bitneed[ch][sb];
}
}
bitcount = 0;
slicecount = 0;
bitslice = max_bitneed + 1;
do {
bitslice--;
bitcount += slicecount;
slicecount = 0;
for (sb = 0; sb < subbands; sb++) {
if ((bitneed[ch][sb] > bitslice + 1) && (bitneed[ch][sb] < bitslice + 16))
slicecount++;
else if (bitneed[ch][sb] == bitslice + 1)
slicecount += 2;
}
} while (bitcount + slicecount < frame->bitpool);
if (bitcount + slicecount == frame->bitpool) {
bitcount += slicecount;
bitslice--;
}
for (sb = 0; sb < subbands; sb++) {
if (bitneed[ch][sb] < bitslice + 2)
bits[ch][sb] = 0;
else {
bits[ch][sb] = bitneed[ch][sb] - bitslice;
if (bits[ch][sb] > 16)
bits[ch][sb] = 16;
}
}
for (sb = 0; bitcount < frame->bitpool &&
sb < subbands; sb++) {
if ((bits[ch][sb] >= 2) && (bits[ch][sb] < 16)) {
bits[ch][sb]++;
bitcount++;
} else if ((bitneed[ch][sb] == bitslice + 1) && (frame->bitpool > bitcount + 1)) {
bits[ch][sb] = 2;
bitcount += 2;
}
}
for (sb = 0; bitcount < frame->bitpool &&
sb < subbands; sb++) {
if (bits[ch][sb] < 16) {
bits[ch][sb]++;
bitcount++;
}
}
}
} else if (frame->mode == STEREO || frame->mode == JOINT_STEREO) {
int bitneed[2][8], loudness, max_bitneed, bitcount, slicecount, bitslice;
int ch, sb;
max_bitneed = 0;
if (frame->allocation == SNR) {
for (ch = 0; ch < 2; ch++) {
for (sb = 0; sb < subbands; sb++) {
bitneed[ch][sb] = frame->scale_factor[ch][sb];
if (bitneed[ch][sb] > max_bitneed)
max_bitneed = bitneed[ch][sb];
}
}
} else {
for (ch = 0; ch < 2; ch++) {
for (sb = 0; sb < subbands; sb++) {
if (frame->scale_factor[ch][sb] == 0)
bitneed[ch][sb] = -5;
else {
if (subbands == 4)
loudness = frame->scale_factor[ch][sb] - sbc_offset4[sf][sb];
else
loudness = frame->scale_factor[ch][sb] - sbc_offset8[sf][sb];
if (loudness > 0)
bitneed[ch][sb] = loudness / 2;
else
bitneed[ch][sb] = loudness;
}
if (bitneed[ch][sb] > max_bitneed)
max_bitneed = bitneed[ch][sb];
}
}
}
bitcount = 0;
slicecount = 0;
bitslice = max_bitneed + 1;
do {
bitslice--;
bitcount += slicecount;
slicecount = 0;
for (ch = 0; ch < 2; ch++) {
for (sb = 0; sb < subbands; sb++) {
if ((bitneed[ch][sb] > bitslice + 1) && (bitneed[ch][sb] < bitslice + 16))
slicecount++;
else if (bitneed[ch][sb] == bitslice + 1)
slicecount += 2;
}
}
} while (bitcount + slicecount < frame->bitpool);
if (bitcount + slicecount == frame->bitpool) {
bitcount += slicecount;
bitslice--;
}
for (ch = 0; ch < 2; ch++) {
for (sb = 0; sb < subbands; sb++) {
if (bitneed[ch][sb] < bitslice + 2) {
bits[ch][sb] = 0;
} else {
bits[ch][sb] = bitneed[ch][sb] - bitslice;
if (bits[ch][sb] > 16)
bits[ch][sb] = 16;
}
}
}
ch = 0;
sb = 0;
while (bitcount < frame->bitpool) {
if ((bits[ch][sb] >= 2) && (bits[ch][sb] < 16)) {
bits[ch][sb]++;
bitcount++;
} else if ((bitneed[ch][sb] == bitslice + 1) && (frame->bitpool > bitcount + 1)) {
bits[ch][sb] = 2;
bitcount += 2;
}
if (ch == 1) {
ch = 0;
sb++;
if (sb >= subbands)
break;
} else
ch = 1;
}
ch = 0;
sb = 0;
while (bitcount < frame->bitpool) {
if (bits[ch][sb] < 16) {
bits[ch][sb]++;
bitcount++;
}
if (ch == 1) {
ch = 0;
sb++;
if (sb >= subbands)
break;
} else
ch = 1;
}
}
}
