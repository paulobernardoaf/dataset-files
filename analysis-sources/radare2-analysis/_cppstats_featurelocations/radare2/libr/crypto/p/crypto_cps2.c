


#define UPPER_LIMIT 0x400000

#define BIT(x,n) (((x)>>(n))&1)

#define BITSWAP8(val,B7,B6,B5,B4,B3,B2,B1,B0) ((BIT(val,B7) << 7) | (BIT(val,B6) << 6) | (BIT(val,B5) << 5) | (BIT(val,B4) << 4) | (BIT(val,B3) << 3) | (BIT(val,B2) << 2) | (BIT(val,B1) << 1) | (BIT(val,B0) << 0))



#include <r_lib.h>
#include <r_crypto.h>




















































































































static const int fn1_groupA[8] = { 10, 4, 6, 7, 2, 13, 15, 14 };
static const int fn1_groupB[8] = { 0, 1, 3, 5, 8, 9, 11, 12 };

static const int fn2_groupA[8] = { 6, 0, 2, 13, 1, 4, 14, 7 };
static const int fn2_groupB[8] = { 3, 5, 9, 10, 8, 15, 12, 11 };







struct sbox {
const ut8 table[64];
const int inputs[6]; 
const int outputs[2]; 
};




struct optimised_sbox {
ut8 input_lookup[256];
ut8 output[64];
};

static const struct sbox fn1_r1_boxes[4] = {
{ 
{
0,2,2,0,1,0,1,1,3,2,0,3,0,3,1,2,1,1,1,2,1,3,2,2,2,3,3,2,1,1,1,2,
2,2,0,0,3,1,3,1,1,1,3,0,0,1,0,0,1,2,2,1,2,3,2,2,2,3,1,3,2,0,1,3,
},
{ 3, 4, 5, 6, -1, -1 },
{ 3, 6 }
},
{ 
{
3,0,2,2,2,1,1,1,1,2,1,0,0,0,2,3,2,3,1,3,0,0,0,2,1,2,2,3,0,3,3,3,
0,1,3,2,3,3,3,1,1,1,1,2,0,1,2,1,3,2,3,1,1,3,2,2,2,3,1,3,2,3,0,0,
},
{ 0, 1, 2, 4, 7, -1 },
{ 2, 7 }
},
{ 
{
3,0,3,1,1,0,2,2,3,1,2,0,3,3,2,3,0,1,0,1,2,3,0,2,0,2,0,1,0,0,1,0,
2,3,1,2,1,0,2,0,2,1,0,1,0,2,1,0,3,1,2,3,1,3,1,1,1,2,0,2,2,0,0,0,
},
{ 0, 1, 2, 3, 6, 7 },
{ 0, 1 }
},
{ 
{
3,2,0,3,0,2,2,1,1,2,3,2,1,3,2,1,2,2,1,3,3,2,1,0,1,0,1,3,0,0,0,2,
2,1,0,1,0,1,0,1,3,1,1,2,2,3,2,0,3,3,2,0,2,1,3,3,0,0,3,0,1,1,3,3,
},
{ 0, 1, 3, 5, 6, 7 },
{ 4, 5 }
},
};

static const struct sbox fn1_r2_boxes[4] = {
{ 
{
3,3,2,0,3,0,3,1,0,3,0,1,0,2,1,3,1,3,0,3,3,1,3,3,3,2,3,2,2,3,1,2,
0,2,2,1,0,1,2,0,3,3,0,1,3,2,1,2,3,0,1,3,0,1,2,2,1,2,1,2,0,1,3,0,
},
{ 0, 1, 2, 3, 6, -1 },
{ 1, 6 }
},
{ 
{
1,2,3,2,1,3,0,1,1,0,2,0,0,2,3,2,3,3,0,1,2,2,1,0,1,0,1,2,3,2,1,3,
2,2,2,0,1,0,2,3,2,1,2,1,2,1,0,3,0,1,2,3,1,2,1,3,2,0,3,2,3,0,2,0,
},
{ 2, 4, 5, 6, 7, -1 },
{ 5, 7 }
},
{ 
{
0,1,0,2,1,1,0,1,0,2,2,2,1,3,0,0,1,1,3,1,2,2,2,3,1,0,3,3,3,2,2,2,
1,1,3,0,3,1,3,0,1,3,3,2,1,1,0,0,1,2,2,2,1,1,1,2,2,0,0,3,2,3,1,3,
},
{ 1, 2, 3, 4, 5, 7 },
{ 0, 3 }
},
{ 
{
2,1,0,3,3,3,2,0,1,2,1,1,1,0,3,1,1,3,3,0,1,2,1,0,0,0,3,0,3,0,3,0,
1,3,3,3,0,3,2,0,2,1,2,2,2,1,1,3,0,1,0,1,0,1,1,1,1,3,1,0,1,2,3,3,
},
{ 0, 1, 3, 4, 6, 7 },
{ 2, 4 }
},
};

static const struct sbox fn1_r3_boxes[4] = {
{ 
{
0,0,0,3,3,1,1,0,2,0,2,0,0,0,3,2,0,1,2,3,2,2,1,0,3,0,0,0,0,0,2,3,
3,0,0,1,1,2,3,3,0,1,3,2,0,1,3,3,2,0,0,1,0,2,0,0,0,3,1,3,3,3,3,3,
},
{ 0, 1, 5, 6, 7, -1 },
{ 0, 5 }
},
{ 
{
2,3,2,3,0,2,3,0,2,2,3,0,3,2,0,2,1,0,2,3,1,1,1,0,0,1,0,2,1,2,2,1,
3,0,2,1,2,3,3,0,3,2,3,1,0,2,1,0,1,2,2,3,0,2,1,3,1,3,0,2,1,1,1,3,
},
{ 2, 3, 4, 6, 7, -1 },
{ 6, 7 }
},
{ 
{
3,0,2,1,1,3,1,2,2,1,2,2,2,0,0,1,2,3,1,0,2,0,0,2,3,1,2,0,0,0,3,0,
2,1,1,2,0,0,1,2,3,1,1,2,0,1,3,0,3,1,1,0,0,2,3,0,0,0,0,3,2,0,0,0,
},
{ 0, 2, 3, 4, 5, 6 },
{ 1, 4 }
},
{ 
{
0,1,0,0,2,1,3,2,3,3,2,1,0,1,1,1,1,1,0,3,3,1,1,0,0,2,2,1,0,3,3,2,
1,3,3,0,3,0,2,1,1,2,3,2,2,2,1,0,0,3,3,3,2,2,3,1,0,2,3,0,3,1,1,0,
},
{ 0, 1, 2, 3, 5, 7 },
{ 2, 3 }
},
};

static const struct sbox fn1_r4_boxes[4] = {
{ 
{
1,1,1,1,1,0,1,3,3,2,3,0,1,2,0,2,3,3,0,1,2,1,2,3,0,3,2,3,2,0,1,2,
0,1,0,3,2,1,3,2,3,1,2,3,2,0,1,2,2,0,0,0,2,1,3,0,3,1,3,0,1,3,3,0,
},
{ 1, 2, 3, 4, 5, 7 },
{ 0, 4 }
},
{ 
{
3,0,0,0,0,1,0,2,3,3,1,3,0,3,1,2,2,2,3,1,0,0,2,0,1,0,2,2,3,3,0,0,
1,1,3,0,2,3,0,3,0,3,0,2,0,2,0,1,0,3,0,1,3,1,1,0,0,1,3,3,2,2,1,0,
},
{ 0, 1, 2, 3, 5, 6 },
{ 1, 3 }
},
{ 
{
0,1,1,2,0,1,3,1,2,0,3,2,0,0,3,0,3,0,1,2,2,3,3,2,3,2,0,1,0,0,1,0,
3,0,2,3,0,2,2,2,1,1,0,2,2,0,0,1,2,1,1,1,2,3,0,3,1,2,3,3,1,1,3,0,
},
{ 0, 2, 4, 5, 6, 7 },
{ 2, 6 }
},
{ 
{
0,1,2,2,0,1,0,3,2,2,1,1,3,2,0,2,0,1,3,3,0,2,2,3,3,2,0,0,2,1,3,3,
1,1,1,3,1,2,1,1,0,3,3,2,3,2,3,0,3,1,0,0,3,0,0,0,2,2,2,1,2,3,0,0,
},
{ 0, 1, 3, 4, 6, 7 },
{ 5, 7 }
},
};



static const struct sbox fn2_r1_boxes[4] = {
{ 
{
2,0,2,0,3,0,0,3,1,1,0,1,3,2,0,1,2,0,1,2,0,2,0,2,2,2,3,0,2,1,3,0,
0,1,0,1,2,2,3,3,0,3,0,2,3,0,1,2,1,1,0,2,0,3,1,1,2,2,1,3,1,1,3,1,
},
{ 0, 3, 4, 5, 7, -1 },
{ 6, 7 }
},
{ 
{
1,1,0,3,0,2,0,1,3,0,2,0,1,1,0,0,1,3,2,2,0,2,2,2,2,0,1,3,3,3,1,1,
1,3,1,3,2,2,2,2,2,2,0,1,0,1,1,2,3,1,1,2,0,3,3,3,2,2,3,1,1,1,3,0,
},
{ 1, 2, 3, 4, 6, -1 },
{ 3, 5 }
},
{ 
{
1,0,2,2,3,3,3,3,1,2,2,1,0,1,2,1,1,2,3,1,2,0,0,1,2,3,1,2,0,0,0,2,
2,0,1,1,0,0,2,0,0,0,2,3,2,3,0,1,3,0,0,0,2,3,2,0,1,3,2,1,3,1,1,3,
},
{ 1, 2, 4, 5, 6, 7 },
{ 1, 4 }
},
{ 
{
1,3,3,0,3,2,3,1,3,2,1,1,3,3,2,1,2,3,0,3,1,0,0,2,3,0,0,0,3,3,0,1,
2,3,0,0,0,1,2,1,3,0,0,1,0,2,2,2,3,3,1,2,1,3,0,0,0,3,0,1,3,2,2,0,
},
{ 0, 2, 3, 5, 6, 7 },
{ 0, 2 }
},
};

static const struct sbox fn2_r2_boxes[4] = {
{ 
{
3,1,3,0,3,0,3,1,3,0,0,1,1,3,0,3,1,1,0,1,2,3,2,3,3,1,2,2,2,0,2,3,
2,2,2,1,1,3,3,0,3,1,2,1,1,1,0,2,0,3,3,0,0,2,0,0,1,1,2,1,2,1,1,0,
},
{ 0, 2, 4, 6, -1, -1 },
{ 4, 6 }
},
{ 
{
0,3,0,3,3,2,1,2,3,1,1,1,2,0,2,3,0,3,1,2,2,1,3,3,3,2,1,2,2,0,1,0,
2,3,0,1,2,0,1,1,2,0,2,1,2,0,2,3,3,1,0,2,3,3,0,3,1,1,3,0,0,1,2,0,
},
{ 1, 3, 4, 5, 6, 7 },
{ 0, 3 }
},
{ 
{
0,0,2,1,3,2,1,0,1,2,2,2,1,1,0,3,1,2,2,3,2,1,1,0,3,0,0,1,1,2,3,1,
3,3,2,2,1,0,1,1,1,2,0,1,2,3,0,3,3,0,3,2,2,0,2,2,1,2,3,2,1,0,2,1,
},
{ 0, 1, 3, 4, 5, 7 },
{ 1, 7 }
},
{ 
{
0,2,1,2,0,2,2,0,1,3,2,0,3,2,3,0,3,3,2,3,1,2,3,1,2,2,0,0,2,2,1,2,
2,3,3,3,1,1,0,0,0,3,2,0,3,2,3,1,1,1,1,0,1,0,1,3,0,0,1,2,2,3,2,0,
},
{ 1, 2, 3, 5, 6, 7 },
{ 2, 5 }
},
};

static const struct sbox fn2_r3_boxes[4] = {
{ 
{
2,1,2,1,2,3,1,3,2,2,1,3,3,0,0,1,0,2,0,3,3,1,0,0,1,1,0,2,3,2,1,2,
1,1,2,1,1,3,2,2,0,2,2,3,3,3,2,0,0,0,0,0,3,3,3,0,1,2,1,0,2,3,3,1,
},
{ 2, 3, 4, 6, -1, -1 },
{ 3, 5 }
},
{ 
{
3,2,3,3,1,0,3,0,2,0,1,1,1,0,3,0,3,1,3,1,0,1,2,3,2,2,3,2,0,1,1,2,
3,0,0,2,1,0,0,2,2,0,1,0,0,2,0,0,1,3,1,3,2,0,3,3,1,0,2,2,2,3,0,0,
},
{ 0, 1, 3, 5, 7, -1 },
{ 0, 2 }
},
{ 
{
2,2,1,0,2,3,3,0,0,0,1,3,1,2,3,2,2,3,1,3,0,3,0,3,3,2,2,1,0,0,0,2,
1,2,2,2,0,0,1,2,0,1,3,0,2,3,2,1,3,2,2,2,3,1,3,0,2,0,2,1,0,3,3,1,
},
{ 0, 1, 2, 3, 5, 7 },
{ 1, 6 }
},
{ 
{
1,2,3,2,0,2,1,3,3,1,0,1,1,2,2,0,0,1,1,1,2,1,1,2,0,1,3,3,1,1,1,2,
3,3,1,0,2,1,1,1,2,1,0,0,2,2,3,2,3,2,2,0,2,2,3,3,0,2,3,0,2,2,1,1,
},
{ 0, 2, 4, 5, 6, 7 },
{ 4, 7 }
},
};

static const struct sbox fn2_r4_boxes[4] = {
{ 
{
2,0,1,1,2,1,3,3,1,1,1,2,0,1,0,2,0,1,2,0,2,3,0,2,3,3,2,2,3,2,0,1,
3,0,2,0,2,3,1,3,2,0,0,1,1,2,3,1,1,1,0,1,2,0,3,3,1,1,1,3,3,1,1,0,
},
{ 0, 1, 3, 6, 7, -1 },
{ 0, 3 }
},
{ 
{
1,2,2,1,0,3,3,1,0,2,2,2,1,0,1,0,1,1,0,1,0,2,1,0,2,1,0,2,3,2,3,3,
2,2,1,2,2,3,1,3,3,3,0,1,0,1,3,0,0,0,1,2,0,3,3,2,3,2,1,3,2,1,0,2,
},
{ 0, 1, 2, 4, 5, 6 },
{ 4, 7 }
},
{ 
{
2,3,2,1,3,2,3,0,0,2,1,1,0,0,3,2,3,1,0,1,2,2,2,1,3,2,2,1,0,2,1,2,
0,3,1,0,0,3,1,1,3,3,2,0,1,0,1,3,0,0,1,2,1,2,3,2,1,0,0,3,2,1,1,3,
},
{ 0, 2, 3, 4, 5, 7 },
{ 1, 2 }
},
{ 
{
2,0,0,3,2,2,2,1,3,3,1,1,2,0,0,3,1,0,3,2,1,0,2,0,3,2,2,3,2,0,3,0,
1,3,0,2,2,1,3,3,0,1,0,3,1,1,3,2,0,3,0,2,3,2,1,3,2,3,0,0,1,3,2,1,
},
{ 2, 3, 4, 5, 6, 7 },
{ 5, 6 }
},
};



static ut8 fn(ut8 in, const struct optimised_sbox *sboxes, ut32 key) {
const struct optimised_sbox *sbox1 = &sboxes[0];
const struct optimised_sbox *sbox2 = &sboxes[1];
const struct optimised_sbox *sbox3 = &sboxes[2];
const struct optimised_sbox *sbox4 = &sboxes[3];

return
sbox1->output[sbox1->input_lookup[in] ^ ((key >> 0) & 0x3f)] |
sbox2->output[sbox2->input_lookup[in] ^ ((key >> 6) & 0x3f)] |
sbox3->output[sbox3->input_lookup[in] ^ ((key >> 12) & 0x3f)] |
sbox4->output[sbox4->input_lookup[in] ^ ((key >> 18) & 0x3f)];
}



static void expand_1st_key(ut32 *dstkey, const ut32 *srckey) {
static const int bits[96] = {
33, 58, 49, 36, 0, 31,
22, 30, 3, 16, 5, 53,
10, 41, 23, 19, 27, 39,
43, 6, 34, 12, 61, 21,
48, 13, 32, 35, 6, 42,
43, 14, 21, 41, 52, 25,
18, 47, 46, 37, 57, 53,
20, 8, 55, 54, 59, 60,
27, 33, 35, 18, 8, 15,
63, 1, 50, 44, 16, 46,
5, 4, 45, 51, 38, 25,
13, 11, 62, 29, 48, 2,
59, 61, 62, 56, 51, 57,
54, 9, 24, 63, 22, 7,
26, 42, 45, 40, 23, 14,
2, 31, 52, 28, 44, 17,
};
int i;

dstkey[0] = 0;
dstkey[1] = 0;
dstkey[2] = 0;
dstkey[3] = 0;

for (i = 0; i < 96; i++) {
dstkey[i / 24] |= BIT (srckey[bits[i] / 32], bits[i] % 32) << (i % 24);
}
}



static void expand_2nd_key(ut32 *dstkey, const ut32 *srckey) {
static const int bits[96] = {
34, 9, 32, 24, 44, 54,
38, 61, 47, 13, 28, 7,
29, 58, 18, 1, 20, 60,
15, 6, 11, 43, 39, 19,
63, 23, 16, 62, 54, 40,
31, 3, 56, 61, 17, 25,
47, 38, 55, 57, 5, 4,
15, 42, 22, 7, 2, 19,
46, 37, 29, 39, 12, 30,
49, 57, 31, 41, 26, 27,
24, 36, 11, 63, 33, 16,
56, 62, 48, 60, 59, 32,
12, 30, 53, 48, 10, 0,
50, 35, 3, 59, 14, 49,
51, 45, 44, 2, 21, 33,
55, 52, 23, 28, 8, 26,
};
int i;

dstkey[0] = 0;
dstkey[1] = 0;
dstkey[2] = 0;
dstkey[3] = 0;

for (i = 0; i < 96; i++) {
dstkey[i / 24] |= BIT(srckey[bits[i] / 32], bits[i] % 32) << (i % 24);
}
}




static void expand_subkey(ut32* subkey, ut16 seed) {

static const int bits[64] = {
5, 10, 14, 9, 4, 0, 15, 6, 1, 8, 3, 2, 12, 7, 13, 11,
5, 12, 7, 2, 13, 11, 9, 14, 4, 1, 6, 10, 8, 0, 15, 3,
4, 10, 2, 0, 6, 9, 12, 1, 11, 7, 15, 8, 13, 5, 14, 3,
14, 11, 12, 7, 4, 5, 2, 10, 1, 15, 0, 9, 8, 6, 13, 3,
};
int i;

subkey[0] = 0;
subkey[1] = 0;

for (i = 0; i < 64; i++) {
subkey[i / 32] |= BIT(seed, bits[i]) << (i % 32);
}
}

static inline ut16 feistel(ut16 val, const int *bitsA, const int *bitsB,
const struct optimised_sbox* boxes1, const struct optimised_sbox* boxes2,
const struct optimised_sbox* boxes3, const struct optimised_sbox* boxes4,
ut32 key1, ut32 key2, ut32 key3, ut32 key4)
{
ut8 l = BITSWAP8(val, bitsB[7],bitsB[6],bitsB[5],bitsB[4],bitsB[3],bitsB[2],bitsB[1],bitsB[0]);
ut8 r = BITSWAP8(val, bitsA[7],bitsA[6],bitsA[5],bitsA[4],bitsA[3],bitsA[2],bitsA[1],bitsA[0]);

l ^= fn(r, boxes1, key1);
r ^= fn(l, boxes2, key2);
l ^= fn(r, boxes3, key3);
r ^= fn(l, boxes4, key4);

return
(BIT(l, 0) << bitsA[0]) |
(BIT(l, 1) << bitsA[1]) |
(BIT(l, 2) << bitsA[2]) |
(BIT(l, 3) << bitsA[3]) |
(BIT(l, 4) << bitsA[4]) |
(BIT(l, 5) << bitsA[5]) |
(BIT(l, 6) << bitsA[6]) |
(BIT(l, 7) << bitsA[7]) |
(BIT(r, 0) << bitsB[0]) |
(BIT(r, 1) << bitsB[1]) |
(BIT(r, 2) << bitsB[2]) |
(BIT(r, 3) << bitsB[3]) |
(BIT(r, 4) << bitsB[4]) |
(BIT(r, 5) << bitsB[5]) |
(BIT(r, 6) << bitsB[6]) |
(BIT(r, 7) << bitsB[7]);
}

static int extract_inputs(ut32 val, const int *inputs) {
int i, res = 0;
for (i = 0; i < 6; i++) {
if (inputs[i] != -1) {
res |= BIT (val, inputs[i]) << i;
}
}
return res;
}

static void optimise_sboxes(struct optimised_sbox* out, const struct sbox* in) {
int i, box;

for (box = 0; box < 4; box++) {

for (i = 0; i < 256; i++) {
out[box].input_lookup[i] = extract_inputs(i, in[box].inputs);
}

for (i = 0; i < 64; i++) {
int o = in[box].table[i];
out[box].output[i] = 0;
if (o & 1) {
out[box].output[i] |= 1 << in[box].outputs[0];
}
if (o & 2) {
out[box].output[i] |= 1 << in[box].outputs[1];
}
}
}
}

static void cps2_crypt(int dir, const ut16 *rom, ut16 *dec, int length, const ut32 *master_key, ut32 upper_limit) {
int i;
ut32 key1[4];
struct optimised_sbox sboxes1[4*4];
struct optimised_sbox sboxes2[4*4];

optimise_sboxes(&sboxes1[0*4], fn1_r1_boxes);
optimise_sboxes(&sboxes1[1*4], fn1_r2_boxes);
optimise_sboxes(&sboxes1[2*4], fn1_r3_boxes);
optimise_sboxes(&sboxes1[3*4], fn1_r4_boxes);
optimise_sboxes(&sboxes2[0*4], fn2_r1_boxes);
optimise_sboxes(&sboxes2[1*4], fn2_r2_boxes);
optimise_sboxes(&sboxes2[2*4], fn2_r3_boxes);
optimise_sboxes(&sboxes2[3*4], fn2_r4_boxes);



expand_1st_key(key1, master_key);


key1[0] ^= BIT(key1[0], 1) << 4;
key1[0] ^= BIT(key1[0], 2) << 5;
key1[0] ^= BIT(key1[0], 8) << 11;
key1[1] ^= BIT(key1[1], 0) << 5;
key1[1] ^= BIT(key1[1], 8) << 11;
key1[2] ^= BIT(key1[2], 1) << 5;
key1[2] ^= BIT(key1[2], 8) << 11;

for (i = 0; i < 0x10000; i++) {
int a;
ut16 seed;
ut32 subkey[2];
ut32 key2[4];

if ((i & 0xff) == 0) {
eprintf ("Crypting %d%%\r", i*100/0x10000);
}


seed = feistel(i, fn1_groupA, fn1_groupB,
&sboxes1[0*4], &sboxes1[1*4], &sboxes1[2*4], &sboxes1[3*4],
key1[0], key1[1], key1[2], key1[3]);


expand_subkey (subkey, seed);


subkey[0] ^= master_key[0];
subkey[1] ^= master_key[1];


expand_2nd_key (key2, subkey);


key2[0] ^= BIT(key2[0], 0) << 5;
key2[0] ^= BIT(key2[0], 6) << 11;
key2[1] ^= BIT(key2[1], 0) << 5;
key2[1] ^= BIT(key2[1], 1) << 4;
key2[2] ^= BIT(key2[2], 2) << 5;
key2[2] ^= BIT(key2[2], 3) << 4;
key2[2] ^= BIT(key2[2], 7) << 11;
key2[3] ^= BIT(key2[3], 1) << 5;


for (a = i; a < length/2 && a < upper_limit/2; a += 0x10000) {
if (dir) {

dec[a] = feistel (rom[a], fn2_groupA, fn2_groupB,
&sboxes2[0*4], &sboxes2[1*4], &sboxes2[2*4], &sboxes2[3*4],
key2[0], key2[1], key2[2], key2[3]);
dec[a] = r_read_be16 (&dec[a]);
} else {

dec[a] = r_read_be16 (&rom[a]);
dec[a] = feistel (dec[a], fn2_groupA, fn2_groupB,
&sboxes2[3*4], &sboxes2[2*4], &sboxes2[1*4], &sboxes2[0*4],
key2[3], key2[2], key2[1], key2[0]);
}
}

while (a < length/2) {
dec[a] = r_read_be16 (&rom[a]);
a += 0x10000;
}
}
}

#if 0
main(cps_state,cps2crypt) {
ut32 key[2];
ut32 lower;
ut32 upper;

std::string skey1 = parameter("cryptkey1");;
key[0] = strtoll(skey1.c_str(), nullptr, 16);

std::string skey2 = parameter("cryptkey2");
key[1] = strtoll(skey2.c_str(), nullptr, 16);

std::string slower = parameter("cryptlower");
lower = strtoll(slower.c_str(), nullptr, 16); 

std::string supper = parameter("cryptupper");
upper = strtoll(supper.c_str(), nullptr, 16);


if (lower != 0xff0000) {
cps2_decrypt( (ut16 *)memregion("maincpu")->base(), m_decrypted_opcodes, memregion("maincpu")->bytes(), key, lower,upper);
}
}
#endif

static ut32 cps2key[2] = {0};

static bool set_key(RCrypto *cry, const ut8 *key, int keylen, int mode, int direction) {
cry->dir = direction;
if (keylen == 8) {

const ut32 *key32 = (const ut32*)key;
cps2key[0] = r_read_be32 (key32);
cps2key[1] = r_read_be32 (key32 + 1);
return true;
}
return false;
}

static int get_key_size(RCrypto *cry) {

return 8;
}

static bool cps2_use(const char *algo) {
return !strcmp (algo, "cps2");
}

static bool update(RCrypto *cry, const ut8 *buf, int len) {
ut8 *output = calloc (1, len);

cps2_crypt (cry->dir, (const ut16 *)buf, (ut16*)output, len, cps2key, UPPER_LIMIT);
r_crypto_append (cry, output, len);
free (output);
return true;
}

RCryptoPlugin r_crypto_plugin_cps2 = {
.name = "cps2",
.set_key = set_key,
.get_key_size = get_key_size,
.use = cps2_use,
.update = update
};

#if !defined(R2_PLUGIN_INCORE)
R_API RLibStruct radare_plugin = {
.type = R_LIB_TYPE_CRYPTO,
.data = &r_crypto_plugin_rol,
.version = R2_VERSION
};
#endif
