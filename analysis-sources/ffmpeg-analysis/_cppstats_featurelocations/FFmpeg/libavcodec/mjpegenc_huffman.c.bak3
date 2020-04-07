




















#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "libavutil/avassert.h"
#include "libavutil/common.h"
#include "libavutil/error.h"
#include "libavutil/qsort.h"
#include "mjpegenc_huffman.h"








static int compare_by_prob(const void *a, const void *b)
{
PTable a_val = *(PTable *) a;
PTable b_val = *(PTable *) b;
return a_val.prob - b_val.prob;
}








static int compare_by_length(const void *a, const void *b)
{
HuffTable a_val = *(HuffTable *) a;
HuffTable b_val = *(HuffTable *) b;
return a_val.length - b_val.length;
}






















void ff_mjpegenc_huffman_compute_bits(PTable *prob_table, HuffTable *distincts, int size, int max_length)
{
PackageMergerList list_a, list_b, *to = &list_a, *from = &list_b, *temp;

int times, i, j, k;

int nbits[257] = {0};

int min;

av_assert0(max_length > 0);

to->nitems = 0;
from->nitems = 0;
to->item_idx[0] = 0;
from->item_idx[0] = 0;
AV_QSORT(prob_table, size, PTable, compare_by_prob);

for (times = 0; times <= max_length; times++) {
to->nitems = 0;
to->item_idx[0] = 0;

j = 0;
k = 0;

if (times < max_length) {
i = 0;
}
while (i < size || j + 1 < from->nitems) {
to->nitems++;
to->item_idx[to->nitems] = to->item_idx[to->nitems - 1];
if (i < size &&
(j + 1 >= from->nitems ||
prob_table[i].prob <
from->probability[j] + from->probability[j + 1])) {
to->items[to->item_idx[to->nitems]++] = prob_table[i].value;
to->probability[to->nitems - 1] = prob_table[i].prob;
i++;
} else {
for (k = from->item_idx[j]; k < from->item_idx[j + 2]; k++) {
to->items[to->item_idx[to->nitems]++] = from->items[k];
}
to->probability[to->nitems - 1] =
from->probability[j] + from->probability[j + 1];
j += 2;
}
}
temp = to;
to = from;
from = temp;
}

min = (size - 1 < from->nitems) ? size - 1 : from->nitems;
for (i = 0; i < from->item_idx[min]; i++) {
nbits[from->items[i]]++;
}


j = 0;
for (i = 0; i < 256; i++) {
if (nbits[i] > 0) {
distincts[j].code = i;
distincts[j].length = nbits[i];
j++;
}
}
}

void ff_mjpeg_encode_huffman_init(MJpegEncHuffmanContext *s)
{
memset(s->val_count, 0, sizeof(s->val_count));
}









void ff_mjpeg_encode_huffman_close(MJpegEncHuffmanContext *s, uint8_t bits[17],
uint8_t val[], int max_nval)
{
int i, j;
int nval = 0;
PTable val_counts[257];
HuffTable distincts[256];

for (i = 0; i < 256; i++) {
if (s->val_count[i]) nval++;
}
av_assert0 (nval <= max_nval);

j = 0;
for (i = 0; i < 256; i++) {
if (s->val_count[i]) {
val_counts[j].value = i;
val_counts[j].prob = s->val_count[i];
j++;
}
}
val_counts[j].value = 256;
val_counts[j].prob = 0;
ff_mjpegenc_huffman_compute_bits(val_counts, distincts, nval + 1, 16);
AV_QSORT(distincts, nval, HuffTable, compare_by_length);

memset(bits, 0, sizeof(bits[0]) * 17);
for (i = 0; i < nval; i++) {
val[i] = distincts[i].code;
bits[distincts[i].length]++;
}
}
