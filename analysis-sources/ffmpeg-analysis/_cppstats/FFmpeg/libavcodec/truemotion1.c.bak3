






























#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avcodec.h"
#include "internal.h"
#include "libavutil/imgutils.h"
#include "libavutil/internal.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/mem.h"

#include "truemotion1data.h"

typedef struct TrueMotion1Context {
AVCodecContext *avctx;
AVFrame *frame;

const uint8_t *buf;
int size;

const uint8_t *mb_change_bits;
int mb_change_bits_row_size;
const uint8_t *index_stream;
int index_stream_size;

int flags;
int x, y, w, h;

uint32_t y_predictor_table[1024];
uint32_t c_predictor_table[1024];
uint32_t fat_y_predictor_table[1024];
uint32_t fat_c_predictor_table[1024];

int compression;
int block_type;
int block_width;
int block_height;

int16_t ydt[8];
int16_t cdt[8];
int16_t fat_ydt[8];
int16_t fat_cdt[8];

int last_deltaset, last_vectable;

unsigned int *vert_pred;
int vert_pred_size;

} TrueMotion1Context;

#define FLAG_SPRITE 32
#define FLAG_KEYFRAME 16
#define FLAG_INTERFRAME 8
#define FLAG_INTERPOLATED 4

struct frame_header {
uint8_t header_size;
uint8_t compression;
uint8_t deltaset;
uint8_t vectable;
uint16_t ysize;
uint16_t xsize;
uint16_t checksum;
uint8_t version;
uint8_t header_type;
uint8_t flags;
uint8_t control;
uint16_t xoffset;
uint16_t yoffset;
uint16_t width;
uint16_t height;
};

#define ALGO_NOP 0
#define ALGO_RGB16V 1
#define ALGO_RGB16H 2
#define ALGO_RGB24H 3


#define BLOCK_2x2 0
#define BLOCK_2x4 1
#define BLOCK_4x2 2
#define BLOCK_4x4 3

typedef struct comp_types {
int algorithm;
int block_width; 
int block_height; 
int block_type;
} comp_types;


static const comp_types compression_types[17] = {
{ ALGO_NOP, 0, 0, 0 },

{ ALGO_RGB16V, 4, 4, BLOCK_4x4 },
{ ALGO_RGB16H, 4, 4, BLOCK_4x4 },
{ ALGO_RGB16V, 4, 2, BLOCK_4x2 },
{ ALGO_RGB16H, 4, 2, BLOCK_4x2 },

{ ALGO_RGB16V, 2, 4, BLOCK_2x4 },
{ ALGO_RGB16H, 2, 4, BLOCK_2x4 },
{ ALGO_RGB16V, 2, 2, BLOCK_2x2 },
{ ALGO_RGB16H, 2, 2, BLOCK_2x2 },

{ ALGO_NOP, 4, 4, BLOCK_4x4 },
{ ALGO_RGB24H, 4, 4, BLOCK_4x4 },
{ ALGO_NOP, 4, 2, BLOCK_4x2 },
{ ALGO_RGB24H, 4, 2, BLOCK_4x2 },

{ ALGO_NOP, 2, 4, BLOCK_2x4 },
{ ALGO_RGB24H, 2, 4, BLOCK_2x4 },
{ ALGO_NOP, 2, 2, BLOCK_2x2 },
{ ALGO_RGB24H, 2, 2, BLOCK_2x2 }
};

static void select_delta_tables(TrueMotion1Context *s, int delta_table_index)
{
int i;

if (delta_table_index > 3)
return;

memcpy(s->ydt, ydts[delta_table_index], 8 * sizeof(int16_t));
memcpy(s->cdt, cdts[delta_table_index], 8 * sizeof(int16_t));
memcpy(s->fat_ydt, fat_ydts[delta_table_index], 8 * sizeof(int16_t));
memcpy(s->fat_cdt, fat_cdts[delta_table_index], 8 * sizeof(int16_t));



for (i = 0; i < 8; i++)
{


s->ydt[i] &= 0xFFFE;
s->ydt[i] /= 2;
}
}

#if HAVE_BIGENDIAN
static int make_ydt15_entry(int p2, int p1, int16_t *ydt)
#else
static int make_ydt15_entry(int p1, int p2, int16_t *ydt)
#endif
{
int lo, hi;

lo = ydt[p1];
lo += (lo * 32) + (lo * 1024);
hi = ydt[p2];
hi += (hi * 32) + (hi * 1024);
return (lo + (hi * (1U << 16))) * 2;
}

static int make_cdt15_entry(int p1, int p2, int16_t *cdt)
{
int r, b, lo;

b = cdt[p2];
r = cdt[p1] * 1024;
lo = b + r;
return (lo + (lo * (1U << 16))) * 2;
}

#if HAVE_BIGENDIAN
static int make_ydt16_entry(int p2, int p1, int16_t *ydt)
#else
static int make_ydt16_entry(int p1, int p2, int16_t *ydt)
#endif
{
int lo, hi;

lo = ydt[p1];
lo += (lo << 6) + (lo << 11);
hi = ydt[p2];
hi += (hi << 6) + (hi << 11);
return (lo + (hi << 16)) << 1;
}

static int make_cdt16_entry(int p1, int p2, int16_t *cdt)
{
int r, b, lo;

b = cdt[p2];
r = cdt[p1] << 11;
lo = b + r;
return (lo + (lo * (1 << 16))) * 2;
}

static int make_ydt24_entry(int p1, int p2, int16_t *ydt)
{
int lo, hi;

lo = ydt[p1];
hi = ydt[p2];
return (lo + (hi * (1 << 8)) + (hi * (1 << 16))) * 2;
}

static int make_cdt24_entry(int p1, int p2, int16_t *cdt)
{
int r, b;

b = cdt[p2];
r = cdt[p1] * (1 << 16);
return (b+r) * 2;
}

static void gen_vector_table15(TrueMotion1Context *s, const uint8_t *sel_vector_table)
{
int len, i, j;
unsigned char delta_pair;

for (i = 0; i < 1024; i += 4)
{
len = *sel_vector_table++ / 2;
for (j = 0; j < len; j++)
{
delta_pair = *sel_vector_table++;
s->y_predictor_table[i+j] = 0xfffffffe &
make_ydt15_entry(delta_pair >> 4, delta_pair & 0xf, s->ydt);
s->c_predictor_table[i+j] = 0xfffffffe &
make_cdt15_entry(delta_pair >> 4, delta_pair & 0xf, s->cdt);
}
s->y_predictor_table[i+(j-1)] |= 1;
s->c_predictor_table[i+(j-1)] |= 1;
}
}

static void gen_vector_table16(TrueMotion1Context *s, const uint8_t *sel_vector_table)
{
int len, i, j;
unsigned char delta_pair;

for (i = 0; i < 1024; i += 4)
{
len = *sel_vector_table++ / 2;
for (j = 0; j < len; j++)
{
delta_pair = *sel_vector_table++;
s->y_predictor_table[i+j] = 0xfffffffe &
make_ydt16_entry(delta_pair >> 4, delta_pair & 0xf, s->ydt);
s->c_predictor_table[i+j] = 0xfffffffe &
make_cdt16_entry(delta_pair >> 4, delta_pair & 0xf, s->cdt);
}
s->y_predictor_table[i+(j-1)] |= 1;
s->c_predictor_table[i+(j-1)] |= 1;
}
}

static void gen_vector_table24(TrueMotion1Context *s, const uint8_t *sel_vector_table)
{
int len, i, j;
unsigned char delta_pair;

for (i = 0; i < 1024; i += 4)
{
len = *sel_vector_table++ / 2;
for (j = 0; j < len; j++)
{
delta_pair = *sel_vector_table++;
s->y_predictor_table[i+j] = 0xfffffffe &
make_ydt24_entry(delta_pair >> 4, delta_pair & 0xf, s->ydt);
s->c_predictor_table[i+j] = 0xfffffffe &
make_cdt24_entry(delta_pair >> 4, delta_pair & 0xf, s->cdt);
s->fat_y_predictor_table[i+j] = 0xfffffffe &
make_ydt24_entry(delta_pair >> 4, delta_pair & 0xf, s->fat_ydt);
s->fat_c_predictor_table[i+j] = 0xfffffffe &
make_cdt24_entry(delta_pair >> 4, delta_pair & 0xf, s->fat_cdt);
}
s->y_predictor_table[i+(j-1)] |= 1;
s->c_predictor_table[i+(j-1)] |= 1;
s->fat_y_predictor_table[i+(j-1)] |= 1;
s->fat_c_predictor_table[i+(j-1)] |= 1;
}
}



static int truemotion1_decode_header(TrueMotion1Context *s)
{
int i, ret;
int width_shift = 0;
int new_pix_fmt;
struct frame_header header;
uint8_t header_buffer[128] = { 0 }; 
const uint8_t *sel_vector_table;

header.header_size = ((s->buf[0] >> 5) | (s->buf[0] << 3)) & 0x7f;
if (s->buf[0] < 0x10)
{
av_log(s->avctx, AV_LOG_ERROR, "invalid header size (%d)\n", s->buf[0]);
return AVERROR_INVALIDDATA;
}

if (header.header_size + 1 > s->size) {
av_log(s->avctx, AV_LOG_ERROR, "Input packet too small.\n");
return AVERROR_INVALIDDATA;
}


for (i = 1; i < header.header_size; i++)
header_buffer[i - 1] = s->buf[i] ^ s->buf[i + 1];

header.compression = header_buffer[0];
header.deltaset = header_buffer[1];
header.vectable = header_buffer[2];
header.ysize = AV_RL16(&header_buffer[3]);
header.xsize = AV_RL16(&header_buffer[5]);
header.checksum = AV_RL16(&header_buffer[7]);
header.version = header_buffer[9];
header.header_type = header_buffer[10];
header.flags = header_buffer[11];
header.control = header_buffer[12];


if (header.version >= 2)
{
if (header.header_type > 3)
{
av_log(s->avctx, AV_LOG_ERROR, "invalid header type (%d)\n", header.header_type);
return AVERROR_INVALIDDATA;
} else if ((header.header_type == 2) || (header.header_type == 3)) {
s->flags = header.flags;
if (!(s->flags & FLAG_INTERFRAME))
s->flags |= FLAG_KEYFRAME;
} else
s->flags = FLAG_KEYFRAME;
} else 
s->flags = FLAG_KEYFRAME;

if (s->flags & FLAG_SPRITE) {
avpriv_request_sample(s->avctx, "Frame with sprite");

return AVERROR_PATCHWELCOME;
} else {
s->w = header.xsize;
s->h = header.ysize;
if (header.header_type < 2) {
if ((s->w < 213) && (s->h >= 176))
{
s->flags |= FLAG_INTERPOLATED;
avpriv_request_sample(s->avctx, "Interpolated frame");
}
}
}

if (header.compression >= 17) {
av_log(s->avctx, AV_LOG_ERROR, "invalid compression type (%d)\n", header.compression);
return AVERROR_INVALIDDATA;
}

if ((header.deltaset != s->last_deltaset) ||
(header.vectable != s->last_vectable))
select_delta_tables(s, header.deltaset);

if ((header.compression & 1) && header.header_type)
sel_vector_table = pc_tbl2;
else {
if (header.vectable > 0 && header.vectable < 4)
sel_vector_table = tables[header.vectable - 1];
else {
av_log(s->avctx, AV_LOG_ERROR, "invalid vector table id (%d)\n", header.vectable);
return AVERROR_INVALIDDATA;
}
}

if (compression_types[header.compression].algorithm == ALGO_RGB24H) {
new_pix_fmt = AV_PIX_FMT_0RGB32;
width_shift = 1;
} else
new_pix_fmt = AV_PIX_FMT_RGB555; 

s->w >>= width_shift;
if (s->w & 1) {
avpriv_request_sample(s->avctx, "Frame with odd width");
return AVERROR_PATCHWELCOME;
}

if (s->w != s->avctx->width || s->h != s->avctx->height ||
new_pix_fmt != s->avctx->pix_fmt) {
av_frame_unref(s->frame);
s->avctx->sample_aspect_ratio = (AVRational){ 1 << width_shift, 1 };
s->avctx->pix_fmt = new_pix_fmt;

if ((ret = ff_set_dimensions(s->avctx, s->w, s->h)) < 0)
return ret;

ff_set_sar(s->avctx, s->avctx->sample_aspect_ratio);

av_fast_malloc(&s->vert_pred, &s->vert_pred_size, s->avctx->width * sizeof(unsigned int));
if (!s->vert_pred)
return AVERROR(ENOMEM);
}




s->mb_change_bits_row_size = ((s->avctx->width >> (2 - width_shift)) + 7) >> 3;

if ((header.deltaset != s->last_deltaset) || (header.vectable != s->last_vectable))
{
if (compression_types[header.compression].algorithm == ALGO_RGB24H)
gen_vector_table24(s, sel_vector_table);
else
if (s->avctx->pix_fmt == AV_PIX_FMT_RGB555)
gen_vector_table15(s, sel_vector_table);
else
gen_vector_table16(s, sel_vector_table);
}


s->mb_change_bits = s->buf + header.header_size;
if (s->flags & FLAG_KEYFRAME) {

s->index_stream = s->mb_change_bits;
if (s->avctx->width * s->avctx->height / 2048 + header.header_size > s->size)
return AVERROR_INVALIDDATA;
} else {

s->index_stream = s->mb_change_bits +
(s->mb_change_bits_row_size * (s->avctx->height >> 2));
}
s->index_stream_size = s->size - (s->index_stream - s->buf);

s->last_deltaset = header.deltaset;
s->last_vectable = header.vectable;
s->compression = header.compression;
s->block_width = compression_types[header.compression].block_width;
s->block_height = compression_types[header.compression].block_height;
s->block_type = compression_types[header.compression].block_type;

if (s->avctx->debug & FF_DEBUG_PICT_INFO)
av_log(s->avctx, AV_LOG_INFO, "tables: %d / %d c:%d %dx%d t:%d %s%s%s%s\n",
s->last_deltaset, s->last_vectable, s->compression, s->block_width,
s->block_height, s->block_type,
s->flags & FLAG_KEYFRAME ? " KEY" : "",
s->flags & FLAG_INTERFRAME ? " INTER" : "",
s->flags & FLAG_SPRITE ? " SPRITE" : "",
s->flags & FLAG_INTERPOLATED ? " INTERPOL" : "");

return header.header_size;
}

static av_cold int truemotion1_decode_init(AVCodecContext *avctx)
{
TrueMotion1Context *s = avctx->priv_data;

s->avctx = avctx;







s->frame = av_frame_alloc();
if (!s->frame)
return AVERROR(ENOMEM);



av_fast_malloc(&s->vert_pred, &s->vert_pred_size, s->avctx->width * sizeof(unsigned int));
if (!s->vert_pred) {
av_frame_free(&s->frame);
return AVERROR(ENOMEM);
}

return 0;
}



























#define GET_NEXT_INDEX() {if (index_stream_index >= s->index_stream_size) { av_log(s->avctx, AV_LOG_INFO, " help! truemotion1 decoder went out of bounds\n"); return; } index = s->index_stream[index_stream_index++] * 4; }








#define INC_INDEX do { if (index >= 1023) { av_log(s->avctx, AV_LOG_ERROR, "Invalid index value.\n"); return; } index++; } while (0)








#define APPLY_C_PREDICTOR() predictor_pair = s->c_predictor_table[index]; horiz_pred += (predictor_pair >> 1); if (predictor_pair & 1) { GET_NEXT_INDEX() if (!index) { GET_NEXT_INDEX() predictor_pair = s->c_predictor_table[index]; horiz_pred += ((predictor_pair >> 1) * 5); if (predictor_pair & 1) GET_NEXT_INDEX() else INC_INDEX; } } else INC_INDEX;
















#define APPLY_C_PREDICTOR_24() predictor_pair = s->c_predictor_table[index]; horiz_pred += (predictor_pair >> 1); if (predictor_pair & 1) { GET_NEXT_INDEX() if (!index) { GET_NEXT_INDEX() predictor_pair = s->fat_c_predictor_table[index]; horiz_pred += (predictor_pair >> 1); if (predictor_pair & 1) GET_NEXT_INDEX() else INC_INDEX; } } else INC_INDEX;

















#define APPLY_Y_PREDICTOR() predictor_pair = s->y_predictor_table[index]; horiz_pred += (predictor_pair >> 1); if (predictor_pair & 1) { GET_NEXT_INDEX() if (!index) { GET_NEXT_INDEX() predictor_pair = s->y_predictor_table[index]; horiz_pred += ((predictor_pair >> 1) * 5); if (predictor_pair & 1) GET_NEXT_INDEX() else INC_INDEX; } } else INC_INDEX;
















#define APPLY_Y_PREDICTOR_24() predictor_pair = s->y_predictor_table[index]; horiz_pred += (predictor_pair >> 1); if (predictor_pair & 1) { GET_NEXT_INDEX() if (!index) { GET_NEXT_INDEX() predictor_pair = s->fat_y_predictor_table[index]; horiz_pred += (predictor_pair >> 1); if (predictor_pair & 1) GET_NEXT_INDEX() else INC_INDEX; } } else INC_INDEX;
















#define OUTPUT_PIXEL_PAIR() *current_pixel_pair = *vert_pred + horiz_pred; *vert_pred++ = *current_pixel_pair++;



static void truemotion1_decode_16bit(TrueMotion1Context *s)
{
int y;
int pixels_left; 
unsigned int predictor_pair;
unsigned int horiz_pred;
unsigned int *vert_pred;
unsigned int *current_pixel_pair;
unsigned char *current_line = s->frame->data[0];
int keyframe = s->flags & FLAG_KEYFRAME;


const unsigned char *mb_change_bits = s->mb_change_bits;
unsigned char mb_change_byte;
unsigned char mb_change_byte_mask;
int mb_change_index;


int index_stream_index = 0; 
int index;


memset(s->vert_pred, 0, s->avctx->width * sizeof(unsigned int));

GET_NEXT_INDEX();

for (y = 0; y < s->avctx->height; y++) {


horiz_pred = 0;
current_pixel_pair = (unsigned int *)current_line;
vert_pred = s->vert_pred;
mb_change_index = 0;
if (!keyframe)
mb_change_byte = mb_change_bits[mb_change_index++];
mb_change_byte_mask = 0x01;
pixels_left = s->avctx->width;

while (pixels_left > 0) {

if (keyframe || ((mb_change_byte & mb_change_byte_mask) == 0)) {

switch (y & 3) {
case 0:


if (s->block_width == 2) {
APPLY_C_PREDICTOR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
APPLY_C_PREDICTOR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
} else {
APPLY_C_PREDICTOR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
}
break;

case 1:
case 3:

APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
break;

case 2:


if (s->block_type == BLOCK_2x2) {
APPLY_C_PREDICTOR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
APPLY_C_PREDICTOR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
} else if (s->block_type == BLOCK_4x2) {
APPLY_C_PREDICTOR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
} else {
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR();
OUTPUT_PIXEL_PAIR();
}
break;
}

} else {



*vert_pred++ = *current_pixel_pair++;
horiz_pred = *current_pixel_pair - *vert_pred;
*vert_pred++ = *current_pixel_pair++;

}

if (!keyframe) {
mb_change_byte_mask <<= 1;


if (!mb_change_byte_mask) {
mb_change_byte = mb_change_bits[mb_change_index++];
mb_change_byte_mask = 0x01;
}
}

pixels_left -= 4;
}


if (((y + 1) & 3) == 0)
mb_change_bits += s->mb_change_bits_row_size;

current_line += s->frame->linesize[0];
}
}

static void truemotion1_decode_24bit(TrueMotion1Context *s)
{
int y;
int pixels_left; 
unsigned int predictor_pair;
unsigned int horiz_pred;
unsigned int *vert_pred;
unsigned int *current_pixel_pair;
unsigned char *current_line = s->frame->data[0];
int keyframe = s->flags & FLAG_KEYFRAME;


const unsigned char *mb_change_bits = s->mb_change_bits;
unsigned char mb_change_byte;
unsigned char mb_change_byte_mask;
int mb_change_index;


int index_stream_index = 0; 
int index;


memset(s->vert_pred, 0, s->avctx->width * sizeof(unsigned int));

GET_NEXT_INDEX();

for (y = 0; y < s->avctx->height; y++) {


horiz_pred = 0;
current_pixel_pair = (unsigned int *)current_line;
vert_pred = s->vert_pred;
mb_change_index = 0;
mb_change_byte = mb_change_bits[mb_change_index++];
mb_change_byte_mask = 0x01;
pixels_left = s->avctx->width;

while (pixels_left > 0) {

if (keyframe || ((mb_change_byte & mb_change_byte_mask) == 0)) {

switch (y & 3) {
case 0:


if (s->block_width == 2) {
APPLY_C_PREDICTOR_24();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
APPLY_C_PREDICTOR_24();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
} else {
APPLY_C_PREDICTOR_24();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
}
break;

case 1:
case 3:

APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
break;

case 2:


if (s->block_type == BLOCK_2x2) {
APPLY_C_PREDICTOR_24();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
APPLY_C_PREDICTOR_24();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
} else if (s->block_type == BLOCK_4x2) {
APPLY_C_PREDICTOR_24();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
} else {
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
APPLY_Y_PREDICTOR_24();
OUTPUT_PIXEL_PAIR();
}
break;
}

} else {



*vert_pred++ = *current_pixel_pair++;
horiz_pred = *current_pixel_pair - *vert_pred;
*vert_pred++ = *current_pixel_pair++;

}

if (!keyframe) {
mb_change_byte_mask <<= 1;


if (!mb_change_byte_mask) {
mb_change_byte = mb_change_bits[mb_change_index++];
mb_change_byte_mask = 0x01;
}
}

pixels_left -= 2;
}


if (((y + 1) & 3) == 0)
mb_change_bits += s->mb_change_bits_row_size;

current_line += s->frame->linesize[0];
}
}


static int truemotion1_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int ret, buf_size = avpkt->size;
TrueMotion1Context *s = avctx->priv_data;

s->buf = buf;
s->size = buf_size;

if ((ret = truemotion1_decode_header(s)) < 0)
return ret;

if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;

if (compression_types[s->compression].algorithm == ALGO_RGB24H) {
truemotion1_decode_24bit(s);
} else if (compression_types[s->compression].algorithm != ALGO_NOP) {
truemotion1_decode_16bit(s);
}

if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;

*got_frame = 1;


return buf_size;
}

static av_cold int truemotion1_decode_end(AVCodecContext *avctx)
{
TrueMotion1Context *s = avctx->priv_data;

av_frame_free(&s->frame);
av_freep(&s->vert_pred);

return 0;
}

AVCodec ff_truemotion1_decoder = {
.name = "truemotion1",
.long_name = NULL_IF_CONFIG_SMALL("Duck TrueMotion 1.0"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_TRUEMOTION1,
.priv_data_size = sizeof(TrueMotion1Context),
.init = truemotion1_decode_init,
.close = truemotion1_decode_end,
.decode = truemotion1_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
