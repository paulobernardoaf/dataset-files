













































#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"
#include "libavutil/internal.h"

#define HUFFMAN_TABLE_SIZE 64 * 1024
#define HUF_TOKENS 256
#define PALETTE_COUNT 256

typedef struct hnode {
int count;
unsigned char used;
int children[2];
} hnode;

typedef struct IdcinContext {

AVCodecContext *avctx;

const unsigned char *buf;
int size;

hnode huff_nodes[256][HUF_TOKENS*2];
int num_huff_nodes[256];

uint32_t pal[256];
} IdcinContext;







static int huff_smallest_node(hnode *hnodes, int num_hnodes) {
int i;
int best, best_node;

best = 99999999;
best_node = -1;
for(i = 0; i < num_hnodes; i++) {
if(hnodes[i].used)
continue;
if(!hnodes[i].count)
continue;
if(hnodes[i].count < best) {
best = hnodes[i].count;
best_node = i;
}
}

if(best_node == -1)
return -1;
hnodes[best_node].used = 1;
return best_node;
}










static av_cold void huff_build_tree(IdcinContext *s, int prev) {
hnode *node, *hnodes;
int num_hnodes, i;

num_hnodes = HUF_TOKENS;
hnodes = s->huff_nodes[prev];
for(i = 0; i < HUF_TOKENS * 2; i++)
hnodes[i].used = 0;

while (1) {
node = &hnodes[num_hnodes]; 


node->children[0] = huff_smallest_node(hnodes, num_hnodes);
if(node->children[0] == -1)
break; 

node->children[1] = huff_smallest_node(hnodes, num_hnodes);
if(node->children[1] == -1)
break; 


node->count = hnodes[node->children[0]].count +
hnodes[node->children[1]].count;
num_hnodes++;
}

s->num_huff_nodes[prev] = num_hnodes - 1;
}

static av_cold int idcin_decode_init(AVCodecContext *avctx)
{
IdcinContext *s = avctx->priv_data;
int i, j, histogram_index = 0;
unsigned char *histograms;

s->avctx = avctx;
avctx->pix_fmt = AV_PIX_FMT_PAL8;


if (s->avctx->extradata_size != HUFFMAN_TABLE_SIZE) {
av_log(s->avctx, AV_LOG_ERROR, " id CIN video: expected extradata size of %d\n", HUFFMAN_TABLE_SIZE);
return -1;
}


histograms = (unsigned char *)s->avctx->extradata;
for (i = 0; i < 256; i++) {
for(j = 0; j < HUF_TOKENS; j++)
s->huff_nodes[i][j].count = histograms[histogram_index++];
huff_build_tree(s, i);
}

return 0;
}

static int idcin_decode_vlcs(IdcinContext *s, AVFrame *frame)
{
hnode *hnodes;
long x, y;
int prev;
unsigned char v = 0;
int bit_pos, node_num, dat_pos;

prev = bit_pos = dat_pos = 0;
for (y = 0; y < (frame->linesize[0] * s->avctx->height);
y += frame->linesize[0]) {
for (x = y; x < y + s->avctx->width; x++) {
node_num = s->num_huff_nodes[prev];
hnodes = s->huff_nodes[prev];

while(node_num >= HUF_TOKENS) {
if(!bit_pos) {
if(dat_pos >= s->size) {
av_log(s->avctx, AV_LOG_ERROR, "Huffman decode error.\n");
return -1;
}
bit_pos = 8;
v = s->buf[dat_pos++];
}

node_num = hnodes[node_num].children[v & 0x01];
v = v >> 1;
bit_pos--;
}

frame->data[0][x] = node_num;
prev = node_num;
}
}

return 0;
}

static int idcin_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
IdcinContext *s = avctx->priv_data;
int pal_size;
const uint8_t *pal = av_packet_get_side_data(avpkt, AV_PKT_DATA_PALETTE, &pal_size);
AVFrame *frame = data;
int ret;

s->buf = buf;
s->size = buf_size;

if ((ret = ff_get_buffer(avctx, frame, 0)) < 0)
return ret;

if (idcin_decode_vlcs(s, frame))
return AVERROR_INVALIDDATA;

if (pal && pal_size == AVPALETTE_SIZE) {
frame->palette_has_changed = 1;
memcpy(s->pal, pal, AVPALETTE_SIZE);
} else if (pal) {
av_log(avctx, AV_LOG_ERROR, "Palette size %d is wrong\n", pal_size);
}

memcpy(frame->data[1], s->pal, AVPALETTE_SIZE);

*got_frame = 1;


return buf_size;
}

static const AVCodecDefault idcin_defaults[] = {
{ "max_pixels", "320*240" },
{ NULL },
};

AVCodec ff_idcin_decoder = {
.name = "idcinvideo",
.long_name = NULL_IF_CONFIG_SMALL("id Quake II CIN video"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_IDCIN,
.priv_data_size = sizeof(IdcinContext),
.init = idcin_decode_init,
.decode = idcin_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
.defaults = idcin_defaults,
};
