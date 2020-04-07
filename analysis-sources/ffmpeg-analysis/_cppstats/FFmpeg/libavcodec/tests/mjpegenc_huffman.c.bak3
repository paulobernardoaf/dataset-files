
























#include "libavcodec/avcodec.h"
#include <stdlib.h>
#include "libavcodec/mjpegenc.h"
#include "libavcodec/mjpegenc_huffman.h"
#include "libavcodec/mjpegenc_common.h"
#include "libavcodec/mpegvideo.h"


static int check_lengths(int L, int expected_length,
const int *probs, int nprobs)
{
HuffTable lengths[256];
PTable val_counts[256];
int actual_length = 0, i, j, k, prob, length;
int ret = 0;
double cantor_measure = 0;
av_assert0(nprobs <= 256);

for (i = 0; i < nprobs; i++) {
val_counts[i] = (PTable){.value = i, .prob = probs[i]};
}

ff_mjpegenc_huffman_compute_bits(val_counts, lengths, nprobs, L);

for (i = 0; i < nprobs; i++) {

for (j = 0; j < nprobs; j++)
if (val_counts[j].value == i) break;
for (k = 0; k < nprobs; k++)
if (lengths[k].code == i) break;
if (!(j < nprobs && k < nprobs)) return 1;
prob = val_counts[j].prob;
length = lengths[k].length;

if (prob) {
actual_length += prob * length;
cantor_measure += 1. / (1 << length);
}

if (length > L || length < 1) return 1;
}

if (cantor_measure > 1) ret = 1;

if (actual_length != expected_length) ret = 1;

if (ret == 1) {
fprintf(stderr,
"Cantor measure: %f\n"
"Actual length: %d\n"
"Expected length: %d\n",
cantor_measure, actual_length, expected_length);
}

return ret;
}

static const int probs_zeroes[] = {
6, 6, 0, 0, 0
};

static const int probs_skewed[] = {
2, 0, 0, 0, 0, 1, 0, 0, 20, 0, 2, 0, 10, 5, 1, 1, 9, 1, 1, 6, 0, 5, 0, 1, 0, 7, 6,
1, 1, 5, 0, 0, 0, 0, 11, 0, 0, 0, 51, 1, 0, 20, 0, 1, 0, 0, 0, 0, 6, 106, 1, 0, 1,
0, 2, 1, 16, 0, 0, 5, 0, 0, 0, 4, 3, 15, 4, 4, 0, 0, 0, 3, 0, 0, 1, 0, 3, 0, 3, 2,
2, 0, 0, 4, 3, 40, 1, 2, 0, 22, 0, 0, 0, 9, 0, 0, 0, 0, 1, 1, 0, 1, 6, 11, 4, 10,
28, 6, 1, 0, 0, 9, 9, 4, 0, 0, 0, 0, 8, 33844, 2, 0, 2, 1, 1, 5, 0, 0, 1, 9, 1, 0,
4, 14, 4, 0, 0, 3, 8, 0, 51, 9, 6, 1, 1, 2, 2, 3, 1, 5, 5, 29, 0, 0, 0, 0, 14, 29,
6, 4, 13, 12, 2, 3, 1, 0, 5, 4, 1, 1, 0, 0, 29, 1, 0, 0, 0, 0, 4, 0, 0, 1, 0, 1,
7, 0, 42, 0, 0, 0, 0, 0, 2, 0, 3, 9, 0, 0, 0, 2, 1, 0, 0, 6, 5, 6, 1, 2, 3, 0, 0,
0, 3, 0, 0, 28, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 23, 0, 0, 0, 0,
0, 21, 1, 0, 3, 24, 2, 0, 0, 7, 0, 0, 1, 5, 1, 2, 0, 5
};

static const int probs_sat[] = {
74, 8, 14, 7, 9345, 40, 0, 2014, 2, 1, 115, 0, 2, 1, 194, 388, 20, 0, 0, 2, 1, 121,
1, 1583, 0, 16, 21, 2, 132, 2, 15, 9, 13, 1, 0, 2293, 2, 8, 5, 2, 30, 0, 0, 4, 54,
783, 4, 1, 2, 4, 0, 22, 93, 1, 143, 19, 0, 36, 32, 4, 6, 33, 3, 45, 0, 8, 1, 0, 18,
17, 1, 0, 1, 0, 0, 1, 1004, 38, 3, 8, 90, 23, 0, 2819, 3, 0, 970, 158, 9, 6, 4, 48,
4, 0, 1, 0, 0, 60, 3, 62, 0, 2, 2, 2, 279, 66, 16, 1, 20, 0, 7, 9, 32, 1411, 6, 3,
27, 1, 5, 49, 0, 0, 0, 0, 0, 2, 10, 1, 1, 2, 3, 801, 3, 25, 5, 1, 1, 0, 632, 0, 14,
18, 5, 8, 200, 4, 4, 22, 12, 0, 4, 1, 0, 2, 4, 9, 3, 16, 7, 2, 2, 213, 0, 2, 620,
39303, 0, 1, 0, 2, 1, 183781, 1, 0, 0, 0, 94, 7, 3, 4, 0, 4, 306, 43, 352, 76, 34,
13, 11, 0, 51, 1, 13, 19, 0, 26, 0, 7276, 4, 207, 31, 1, 2, 4, 6, 19, 8, 17, 4, 6,
0, 1085, 0, 0, 0, 3, 489, 36, 1, 0, 1, 9420, 294, 28, 0, 57, 5, 0, 9, 2, 0, 1, 2,
2, 0, 0, 9, 2, 29, 2, 2, 7, 0, 5, 490, 0, 7, 5, 0, 1, 8, 0, 0, 23255, 0, 1
};



int main(int argc, char **argv)
{
int i, ret = 0;

PTable val_counts[] = {
{.value = 0, .prob = 1},
{.value = 1, .prob = 2},
{.value = 2, .prob = 5},
{.value = 3, .prob = 10},
{.value = 4, .prob = 21},
};

static const HuffTable expected[] = {
{.code = 0, .length = 3},
{.code = 1, .length = 3},
{.code = 2, .length = 3},
{.code = 3, .length = 3},
{.code = 4, .length = 1},
};

HuffTable distincts[5];



ff_mjpegenc_huffman_compute_bits(val_counts, distincts,
FF_ARRAY_ELEMS(distincts), 3);

for (i = 0; i < FF_ARRAY_ELEMS(distincts); i++) {
if (distincts[i].code != expected[i].code ||
distincts[i].length != expected[i].length) {
fprintf(stderr,
"Built huffman does not equal expectations. "
"Expected: code %d probability %d, "
"Actual: code %d probability %d\n",
expected[i].code, expected[i].length,
distincts[i].code, distincts[i].length);
ret = 1;
}
}


if (check_lengths(16, 18, probs_zeroes, FF_ARRAY_ELEMS(probs_zeroes)))
ret = 1;

if (check_lengths(16, 41282, probs_skewed, FF_ARRAY_ELEMS(probs_skewed)))
ret = 1;

if (check_lengths(16, 669904, probs_sat, FF_ARRAY_ELEMS(probs_sat)))
ret = 1;

return ret;
}
