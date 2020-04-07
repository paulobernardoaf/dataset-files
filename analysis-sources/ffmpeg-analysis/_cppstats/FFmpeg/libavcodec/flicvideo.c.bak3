



































#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavutil/intreadwrite.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "mathops.h"

#define FLI_256_COLOR 4
#define FLI_DELTA 7
#define FLI_COLOR 11
#define FLI_LC 12
#define FLI_BLACK 13
#define FLI_BRUN 15
#define FLI_COPY 16
#define FLI_MINI 18
#define FLI_DTA_BRUN 25
#define FLI_DTA_COPY 26
#define FLI_DTA_LC 27

#define FLI_TYPE_CODE (0xAF11)
#define FLC_FLX_TYPE_CODE (0xAF12)
#define FLC_DTA_TYPE_CODE (0xAF44) 
#define FLC_MAGIC_CARPET_SYNTHETIC_TYPE_CODE (0xAF13)

#define CHECK_PIXEL_PTR(n) if (pixel_ptr + n > pixel_limit) { av_log (s->avctx, AV_LOG_ERROR, "Invalid pixel_ptr = %d > pixel_limit = %d\n", pixel_ptr + n, pixel_limit); return AVERROR_INVALIDDATA; } 






typedef struct FlicDecodeContext {
AVCodecContext *avctx;
AVFrame *frame;

unsigned int palette[256];
int new_palette;
int fli_type; 
} FlicDecodeContext;

static av_cold int flic_decode_init(AVCodecContext *avctx)
{
FlicDecodeContext *s = avctx->priv_data;
unsigned char *fli_header = (unsigned char *)avctx->extradata;
int depth;

if (avctx->extradata_size != 0 &&
avctx->extradata_size != 12 &&
avctx->extradata_size != 128 &&
avctx->extradata_size != 256 &&
avctx->extradata_size != 904 &&
avctx->extradata_size != 1024) {
av_log(avctx, AV_LOG_ERROR, "Unexpected extradata size %d\n", avctx->extradata_size);
return AVERROR_INVALIDDATA;
}

s->avctx = avctx;

if (s->avctx->extradata_size == 12) {

s->fli_type = FLC_MAGIC_CARPET_SYNTHETIC_TYPE_CODE;
depth = 8;
} else if (avctx->extradata_size == 1024) {
uint8_t *ptr = avctx->extradata;
int i;

for (i = 0; i < 256; i++) {
s->palette[i] = AV_RL32(ptr);
ptr += 4;
}
depth = 8;

} else if (avctx->extradata_size == 0 ||
avctx->extradata_size == 256 ||

avctx->extradata_size == 904) {
s->fli_type = FLI_TYPE_CODE;
depth = 8;
} else {
s->fli_type = AV_RL16(&fli_header[4]);
depth = AV_RL16(&fli_header[12]);
}

if (depth == 0) {
depth = 8; 
}

if ((s->fli_type == FLC_FLX_TYPE_CODE) && (depth == 16)) {
depth = 15; 
}

switch (depth) {
case 8 : avctx->pix_fmt = AV_PIX_FMT_PAL8; break;
case 15 : avctx->pix_fmt = AV_PIX_FMT_RGB555; break;
case 16 : avctx->pix_fmt = AV_PIX_FMT_RGB565; break;
case 24 : avctx->pix_fmt = AV_PIX_FMT_BGR24; break;
default :
av_log(avctx, AV_LOG_ERROR, "Unknown FLC/FLX depth of %d Bpp is unsupported.\n",depth);
return AVERROR_INVALIDDATA;
}

s->frame = av_frame_alloc();
if (!s->frame)
return AVERROR(ENOMEM);

s->new_palette = 0;

return 0;
}

static int flic_decode_frame_8BPP(AVCodecContext *avctx,
void *data, int *got_frame,
const uint8_t *buf, int buf_size)
{
FlicDecodeContext *s = avctx->priv_data;

GetByteContext g2;
int pixel_ptr;
int palette_ptr;
unsigned char palette_idx1;
unsigned char palette_idx2;

unsigned int frame_size;
int num_chunks;

unsigned int chunk_size;
int chunk_type;

int i, j, ret;

int color_packets;
int color_changes;
int color_shift;
unsigned char r, g, b;

int lines;
int compressed_lines;
int starting_line;
int line_packets;
int y_ptr;
int byte_run;
int pixel_skip;
int pixel_countdown;
unsigned char *pixels;
unsigned int pixel_limit;

bytestream2_init(&g2, buf, buf_size);

if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;

pixels = s->frame->data[0];
pixel_limit = s->avctx->height * s->frame->linesize[0];
if (buf_size < 16 || buf_size > INT_MAX - (3 * 256 + AV_INPUT_BUFFER_PADDING_SIZE))
return AVERROR_INVALIDDATA;
frame_size = bytestream2_get_le32(&g2);
if (frame_size > buf_size)
frame_size = buf_size;
bytestream2_skip(&g2, 2); 
num_chunks = bytestream2_get_le16(&g2);
bytestream2_skip(&g2, 8); 

if (frame_size < 16)
return AVERROR_INVALIDDATA;

frame_size -= 16;


while ((frame_size >= 6) && (num_chunks > 0) &&
bytestream2_get_bytes_left(&g2) >= 4) {
int stream_ptr_after_chunk;
chunk_size = bytestream2_get_le32(&g2);
if (chunk_size > frame_size) {
av_log(avctx, AV_LOG_WARNING,
"Invalid chunk_size = %u > frame_size = %u\n", chunk_size, frame_size);
chunk_size = frame_size;
}
stream_ptr_after_chunk = bytestream2_tell(&g2) - 4 + chunk_size;

chunk_type = bytestream2_get_le16(&g2);

switch (chunk_type) {
case FLI_256_COLOR:
case FLI_COLOR:




if ((chunk_type == FLI_256_COLOR) && (s->fli_type != FLC_MAGIC_CARPET_SYNTHETIC_TYPE_CODE))
color_shift = 0;
else
color_shift = 2;

color_packets = bytestream2_get_le16(&g2);
palette_ptr = 0;
for (i = 0; i < color_packets; i++) {

palette_ptr += bytestream2_get_byte(&g2);


color_changes = bytestream2_get_byte(&g2);


if (color_changes == 0)
color_changes = 256;

if (bytestream2_tell(&g2) + color_changes * 3 > stream_ptr_after_chunk)
break;

for (j = 0; j < color_changes; j++) {
unsigned int entry;


if ((unsigned)palette_ptr >= 256)
palette_ptr = 0;

r = bytestream2_get_byte(&g2) << color_shift;
g = bytestream2_get_byte(&g2) << color_shift;
b = bytestream2_get_byte(&g2) << color_shift;
entry = 0xFFU << 24 | r << 16 | g << 8 | b;
if (color_shift == 2)
entry |= entry >> 6 & 0x30303;
if (s->palette[palette_ptr] != entry)
s->new_palette = 1;
s->palette[palette_ptr++] = entry;
}
}
break;

case FLI_DELTA:
y_ptr = 0;
compressed_lines = bytestream2_get_le16(&g2);
while (compressed_lines > 0) {
if (bytestream2_tell(&g2) + 2 > stream_ptr_after_chunk)
break;
if (y_ptr > pixel_limit)
return AVERROR_INVALIDDATA;
line_packets = sign_extend(bytestream2_get_le16(&g2), 16);
if ((line_packets & 0xC000) == 0xC000) {

line_packets = -line_packets;
if (line_packets > s->avctx->height)
return AVERROR_INVALIDDATA;
y_ptr += line_packets * s->frame->linesize[0];
} else if ((line_packets & 0xC000) == 0x4000) {
av_log(avctx, AV_LOG_ERROR, "Undefined opcode (%x) in DELTA_FLI\n", line_packets);
} else if ((line_packets & 0xC000) == 0x8000) {

pixel_ptr= y_ptr + s->frame->linesize[0] - 1;
CHECK_PIXEL_PTR(0);
pixels[pixel_ptr] = line_packets & 0xff;
} else {
compressed_lines--;
pixel_ptr = y_ptr;
CHECK_PIXEL_PTR(0);
pixel_countdown = s->avctx->width;
for (i = 0; i < line_packets; i++) {
if (bytestream2_tell(&g2) + 2 > stream_ptr_after_chunk)
break;

pixel_skip = bytestream2_get_byte(&g2);
pixel_ptr += pixel_skip;
pixel_countdown -= pixel_skip;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (byte_run < 0) {
byte_run = -byte_run;
palette_idx1 = bytestream2_get_byte(&g2);
palette_idx2 = bytestream2_get_byte(&g2);
CHECK_PIXEL_PTR(byte_run * 2);
for (j = 0; j < byte_run; j++, pixel_countdown -= 2) {
pixels[pixel_ptr++] = palette_idx1;
pixels[pixel_ptr++] = palette_idx2;
}
} else {
CHECK_PIXEL_PTR(byte_run * 2);
if (bytestream2_tell(&g2) + byte_run * 2 > stream_ptr_after_chunk)
break;
for (j = 0; j < byte_run * 2; j++, pixel_countdown--) {
pixels[pixel_ptr++] = bytestream2_get_byte(&g2);
}
}
}

y_ptr += s->frame->linesize[0];
}
}
break;

case FLI_LC:

starting_line = bytestream2_get_le16(&g2);
if (starting_line >= s->avctx->height)
return AVERROR_INVALIDDATA;
y_ptr = 0;
y_ptr += starting_line * s->frame->linesize[0];

compressed_lines = bytestream2_get_le16(&g2);
while (compressed_lines > 0) {
pixel_ptr = y_ptr;
CHECK_PIXEL_PTR(0);
pixel_countdown = s->avctx->width;
if (bytestream2_tell(&g2) + 1 > stream_ptr_after_chunk)
break;
line_packets = bytestream2_get_byte(&g2);
if (line_packets > 0) {
for (i = 0; i < line_packets; i++) {

if (bytestream2_tell(&g2) + 1 > stream_ptr_after_chunk)
break;
pixel_skip = bytestream2_get_byte(&g2);
pixel_ptr += pixel_skip;
pixel_countdown -= pixel_skip;
byte_run = sign_extend(bytestream2_get_byte(&g2),8);
if (byte_run > 0) {
CHECK_PIXEL_PTR(byte_run);
if (bytestream2_tell(&g2) + byte_run > stream_ptr_after_chunk)
break;
for (j = 0; j < byte_run; j++, pixel_countdown--) {
pixels[pixel_ptr++] = bytestream2_get_byte(&g2);
}
} else if (byte_run < 0) {
byte_run = -byte_run;
palette_idx1 = bytestream2_get_byte(&g2);
CHECK_PIXEL_PTR(byte_run);
for (j = 0; j < byte_run; j++, pixel_countdown--) {
pixels[pixel_ptr++] = palette_idx1;
}
}
}
}

y_ptr += s->frame->linesize[0];
compressed_lines--;
}
break;

case FLI_BLACK:

memset(pixels, 0,
s->frame->linesize[0] * s->avctx->height);
break;

case FLI_BRUN:


y_ptr = 0;
for (lines = 0; lines < s->avctx->height; lines++) {
pixel_ptr = y_ptr;


bytestream2_skip(&g2, 1);
pixel_countdown = s->avctx->width;
while (pixel_countdown > 0) {
if (bytestream2_tell(&g2) + 1 > stream_ptr_after_chunk)
break;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (!byte_run) {
av_log(avctx, AV_LOG_ERROR, "Invalid byte run value.\n");
return AVERROR_INVALIDDATA;
}

if (byte_run > 0) {
palette_idx1 = bytestream2_get_byte(&g2);
CHECK_PIXEL_PTR(byte_run);
for (j = 0; j < byte_run; j++) {
pixels[pixel_ptr++] = palette_idx1;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) at line %d\n",
pixel_countdown, lines);
}
} else { 
byte_run = -byte_run;
CHECK_PIXEL_PTR(byte_run);
if (bytestream2_tell(&g2) + byte_run > stream_ptr_after_chunk)
break;
for (j = 0; j < byte_run; j++) {
pixels[pixel_ptr++] = bytestream2_get_byte(&g2);
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) at line %d\n",
pixel_countdown, lines);
}
}
}

y_ptr += s->frame->linesize[0];
}
break;

case FLI_COPY:

if (chunk_size - 6 != FFALIGN(s->avctx->width, 4) * s->avctx->height) {
av_log(avctx, AV_LOG_ERROR, "In chunk FLI_COPY : source data (%d bytes) " \
"has incorrect size, skipping chunk\n", chunk_size - 6);
bytestream2_skip(&g2, chunk_size - 6);
} else {
for (y_ptr = 0; y_ptr < s->frame->linesize[0] * s->avctx->height;
y_ptr += s->frame->linesize[0]) {
bytestream2_get_buffer(&g2, &pixels[y_ptr],
s->avctx->width);
if (s->avctx->width & 3)
bytestream2_skip(&g2, 4 - (s->avctx->width & 3));
}
}
break;

case FLI_MINI:

break;

default:
av_log(avctx, AV_LOG_ERROR, "Unrecognized chunk type: %d\n", chunk_type);
break;
}

if (stream_ptr_after_chunk - bytestream2_tell(&g2) >= 0) {
bytestream2_skip(&g2, stream_ptr_after_chunk - bytestream2_tell(&g2));
} else {
av_log(avctx, AV_LOG_ERROR, "Chunk overread\n");
break;
}

frame_size -= chunk_size;
num_chunks--;
}



if (bytestream2_get_bytes_left(&g2) > 2)
av_log(avctx, AV_LOG_ERROR, "Processed FLI chunk where chunk size = %d " \
"and final chunk ptr = %d\n", buf_size,
buf_size - bytestream2_get_bytes_left(&g2));


memcpy(s->frame->data[1], s->palette, AVPALETTE_SIZE);
if (s->new_palette) {
s->frame->palette_has_changed = 1;
s->new_palette = 0;
}

if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;

*got_frame = 1;

return buf_size;
}

static int flic_decode_frame_15_16BPP(AVCodecContext *avctx,
void *data, int *got_frame,
const uint8_t *buf, int buf_size)
{


FlicDecodeContext *s = avctx->priv_data;

GetByteContext g2;
int pixel_ptr;
unsigned char palette_idx1;

unsigned int frame_size;
int num_chunks;

unsigned int chunk_size;
int chunk_type;

int i, j, ret;

int lines;
int compressed_lines;
int line_packets;
int y_ptr;
int byte_run;
int pixel_skip;
int pixel_countdown;
unsigned char *pixels;
int pixel;
unsigned int pixel_limit;

bytestream2_init(&g2, buf, buf_size);

if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;

pixels = s->frame->data[0];
pixel_limit = s->avctx->height * s->frame->linesize[0];

frame_size = bytestream2_get_le32(&g2);
bytestream2_skip(&g2, 2); 
num_chunks = bytestream2_get_le16(&g2);
bytestream2_skip(&g2, 8); 
if (frame_size > buf_size)
frame_size = buf_size;

if (frame_size < 16)
return AVERROR_INVALIDDATA;
frame_size -= 16;


while ((frame_size > 0) && (num_chunks > 0) &&
bytestream2_get_bytes_left(&g2) >= 4) {
int stream_ptr_after_chunk;
chunk_size = bytestream2_get_le32(&g2);
if (chunk_size > frame_size) {
av_log(avctx, AV_LOG_WARNING,
"Invalid chunk_size = %u > frame_size = %u\n", chunk_size, frame_size);
chunk_size = frame_size;
}
stream_ptr_after_chunk = bytestream2_tell(&g2) - 4 + chunk_size;

chunk_type = bytestream2_get_le16(&g2);


switch (chunk_type) {
case FLI_256_COLOR:
case FLI_COLOR:



ff_dlog(avctx,
"Unexpected Palette chunk %d in non-palettized FLC\n",
chunk_type);
bytestream2_skip(&g2, chunk_size - 6);
break;

case FLI_DELTA:
case FLI_DTA_LC:
y_ptr = 0;
compressed_lines = bytestream2_get_le16(&g2);
while (compressed_lines > 0) {
if (bytestream2_tell(&g2) + 2 > stream_ptr_after_chunk)
break;
if (y_ptr > pixel_limit)
return AVERROR_INVALIDDATA;
line_packets = sign_extend(bytestream2_get_le16(&g2), 16);
if (line_packets < 0) {
line_packets = -line_packets;
if (line_packets > s->avctx->height)
return AVERROR_INVALIDDATA;
y_ptr += line_packets * s->frame->linesize[0];
} else {
compressed_lines--;
pixel_ptr = y_ptr;
CHECK_PIXEL_PTR(0);
pixel_countdown = s->avctx->width;
for (i = 0; i < line_packets; i++) {

if (bytestream2_tell(&g2) + 2 > stream_ptr_after_chunk)
break;
pixel_skip = bytestream2_get_byte(&g2);
pixel_ptr += (pixel_skip*2); 
pixel_countdown -= pixel_skip;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (byte_run < 0) {
byte_run = -byte_run;
pixel = bytestream2_get_le16(&g2);
CHECK_PIXEL_PTR(2 * byte_run);
for (j = 0; j < byte_run; j++, pixel_countdown -= 2) {
*((signed short*)(&pixels[pixel_ptr])) = pixel;
pixel_ptr += 2;
}
} else {
if (bytestream2_tell(&g2) + 2*byte_run > stream_ptr_after_chunk)
break;
CHECK_PIXEL_PTR(2 * byte_run);
for (j = 0; j < byte_run; j++, pixel_countdown--) {
*((signed short*)(&pixels[pixel_ptr])) = bytestream2_get_le16(&g2);
pixel_ptr += 2;
}
}
}

y_ptr += s->frame->linesize[0];
}
}
break;

case FLI_LC:
av_log(avctx, AV_LOG_ERROR, "Unexpected FLI_LC chunk in non-palettized FLC\n");
bytestream2_skip(&g2, chunk_size - 6);
break;

case FLI_BLACK:

memset(pixels, 0x0000,
s->frame->linesize[0] * s->avctx->height);
break;

case FLI_BRUN:
y_ptr = 0;
for (lines = 0; lines < s->avctx->height; lines++) {
pixel_ptr = y_ptr;


bytestream2_skip(&g2, 1);
pixel_countdown = (s->avctx->width * 2);

while (pixel_countdown > 0) {
if (bytestream2_tell(&g2) + 1 > stream_ptr_after_chunk)
break;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (byte_run > 0) {
palette_idx1 = bytestream2_get_byte(&g2);
CHECK_PIXEL_PTR(byte_run);
for (j = 0; j < byte_run; j++) {
pixels[pixel_ptr++] = palette_idx1;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) (linea%d)\n",
pixel_countdown, lines);
}
} else { 
byte_run = -byte_run;
if (bytestream2_tell(&g2) + byte_run > stream_ptr_after_chunk)
break;
CHECK_PIXEL_PTR(byte_run);
for (j = 0; j < byte_run; j++) {
palette_idx1 = bytestream2_get_byte(&g2);
pixels[pixel_ptr++] = palette_idx1;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) at line %d\n",
pixel_countdown, lines);
}
}
}






#if HAVE_BIGENDIAN
pixel_ptr = y_ptr;
pixel_countdown = s->avctx->width;
while (pixel_countdown > 0) {
*((signed short*)(&pixels[pixel_ptr])) = AV_RL16(&buf[pixel_ptr]);
pixel_ptr += 2;
}
#endif
y_ptr += s->frame->linesize[0];
}
break;

case FLI_DTA_BRUN:
y_ptr = 0;
for (lines = 0; lines < s->avctx->height; lines++) {
pixel_ptr = y_ptr;


bytestream2_skip(&g2, 1);
pixel_countdown = s->avctx->width; 

while (pixel_countdown > 0) {
if (bytestream2_tell(&g2) + 1 > stream_ptr_after_chunk)
break;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (byte_run > 0) {
pixel = bytestream2_get_le16(&g2);
CHECK_PIXEL_PTR(2 * byte_run);
for (j = 0; j < byte_run; j++) {
*((signed short*)(&pixels[pixel_ptr])) = pixel;
pixel_ptr += 2;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d)\n",
pixel_countdown);
}
} else { 
byte_run = -byte_run;
if (bytestream2_tell(&g2) + 2 * byte_run > stream_ptr_after_chunk)
break;
CHECK_PIXEL_PTR(2 * byte_run);
for (j = 0; j < byte_run; j++) {
*((signed short*)(&pixels[pixel_ptr])) = bytestream2_get_le16(&g2);
pixel_ptr += 2;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d)\n",
pixel_countdown);
}
}
}

y_ptr += s->frame->linesize[0];
}
break;

case FLI_COPY:
case FLI_DTA_COPY:

if (chunk_size - 6 > (unsigned int)(FFALIGN(s->avctx->width, 2) * s->avctx->height)*2) {
av_log(avctx, AV_LOG_ERROR, "In chunk FLI_COPY : source data (%d bytes) " \
"bigger than image, skipping chunk\n", chunk_size - 6);
bytestream2_skip(&g2, chunk_size - 6);
} else {

for (y_ptr = 0; y_ptr < s->frame->linesize[0] * s->avctx->height;
y_ptr += s->frame->linesize[0]) {

pixel_countdown = s->avctx->width;
pixel_ptr = 0;
while (pixel_countdown > 0) {
*((signed short*)(&pixels[y_ptr + pixel_ptr])) = bytestream2_get_le16(&g2);
pixel_ptr += 2;
pixel_countdown--;
}
if (s->avctx->width & 1)
bytestream2_skip(&g2, 2);
}
}
break;

case FLI_MINI:

bytestream2_skip(&g2, chunk_size - 6);
break;

default:
av_log(avctx, AV_LOG_ERROR, "Unrecognized chunk type: %d\n", chunk_type);
break;
}

if (stream_ptr_after_chunk - bytestream2_tell(&g2) >= 0) {
bytestream2_skip(&g2, stream_ptr_after_chunk - bytestream2_tell(&g2));
} else {
av_log(avctx, AV_LOG_ERROR, "Chunk overread\n");
break;
}

frame_size -= chunk_size;
num_chunks--;
}



if ((bytestream2_get_bytes_left(&g2) != 0) && (bytestream2_get_bytes_left(&g2) != 1))
av_log(avctx, AV_LOG_ERROR, "Processed FLI chunk where chunk size = %d " \
"and final chunk ptr = %d\n", buf_size, bytestream2_tell(&g2));

if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;

*got_frame = 1;

return buf_size;
}

static int flic_decode_frame_24BPP(AVCodecContext *avctx,
void *data, int *got_frame,
const uint8_t *buf, int buf_size)
{
FlicDecodeContext *s = avctx->priv_data;

GetByteContext g2;
int pixel_ptr;
unsigned char palette_idx1;

unsigned int frame_size;
int num_chunks;

unsigned int chunk_size;
int chunk_type;

int i, j, ret;

int lines;
int compressed_lines;
int line_packets;
int y_ptr;
int byte_run;
int pixel_skip;
int pixel_countdown;
unsigned char *pixels;
int pixel;
unsigned int pixel_limit;

bytestream2_init(&g2, buf, buf_size);

if ((ret = ff_reget_buffer(avctx, s->frame, 0)) < 0)
return ret;

pixels = s->frame->data[0];
pixel_limit = s->avctx->height * s->frame->linesize[0];

frame_size = bytestream2_get_le32(&g2);
bytestream2_skip(&g2, 2); 
num_chunks = bytestream2_get_le16(&g2);
bytestream2_skip(&g2, 8); 
if (frame_size > buf_size)
frame_size = buf_size;

if (frame_size < 16)
return AVERROR_INVALIDDATA;
frame_size -= 16;


while ((frame_size > 0) && (num_chunks > 0) &&
bytestream2_get_bytes_left(&g2) >= 4) {
int stream_ptr_after_chunk;
chunk_size = bytestream2_get_le32(&g2);
if (chunk_size > frame_size) {
av_log(avctx, AV_LOG_WARNING,
"Invalid chunk_size = %u > frame_size = %u\n", chunk_size, frame_size);
chunk_size = frame_size;
}
stream_ptr_after_chunk = bytestream2_tell(&g2) - 4 + chunk_size;

chunk_type = bytestream2_get_le16(&g2);


switch (chunk_type) {
case FLI_256_COLOR:
case FLI_COLOR:



ff_dlog(avctx,
"Unexpected Palette chunk %d in non-palettized FLC\n",
chunk_type);
bytestream2_skip(&g2, chunk_size - 6);
break;

case FLI_DELTA:
case FLI_DTA_LC:
y_ptr = 0;
compressed_lines = bytestream2_get_le16(&g2);
while (compressed_lines > 0) {
if (bytestream2_tell(&g2) + 2 > stream_ptr_after_chunk)
break;
if (y_ptr > pixel_limit)
return AVERROR_INVALIDDATA;
line_packets = sign_extend(bytestream2_get_le16(&g2), 16);
if (line_packets < 0) {
line_packets = -line_packets;
if (line_packets > s->avctx->height)
return AVERROR_INVALIDDATA;
y_ptr += line_packets * s->frame->linesize[0];
} else {
compressed_lines--;
pixel_ptr = y_ptr;
CHECK_PIXEL_PTR(0);
pixel_countdown = s->avctx->width;
for (i = 0; i < line_packets; i++) {

if (bytestream2_tell(&g2) + 2 > stream_ptr_after_chunk)
break;
pixel_skip = bytestream2_get_byte(&g2);
pixel_ptr += (pixel_skip*3); 
pixel_countdown -= pixel_skip;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (byte_run < 0) {
byte_run = -byte_run;
pixel = bytestream2_get_le24(&g2);
CHECK_PIXEL_PTR(3 * byte_run);
for (j = 0; j < byte_run; j++, pixel_countdown -= 1) {
AV_WL24(&pixels[pixel_ptr], pixel);
pixel_ptr += 3;
}
} else {
if (bytestream2_tell(&g2) + 2*byte_run > stream_ptr_after_chunk)
break;
CHECK_PIXEL_PTR(3 * byte_run);
for (j = 0; j < byte_run; j++, pixel_countdown--) {
pixel = bytestream2_get_le24(&g2);
AV_WL24(&pixels[pixel_ptr], pixel);
pixel_ptr += 3;
}
}
}

y_ptr += s->frame->linesize[0];
}
}
break;

case FLI_LC:
av_log(avctx, AV_LOG_ERROR, "Unexpected FLI_LC chunk in non-palettized FLC\n");
bytestream2_skip(&g2, chunk_size - 6);
break;

case FLI_BLACK:

memset(pixels, 0x00,
s->frame->linesize[0] * s->avctx->height);
break;

case FLI_BRUN:
y_ptr = 0;
for (lines = 0; lines < s->avctx->height; lines++) {
pixel_ptr = y_ptr;


bytestream2_skip(&g2, 1);
pixel_countdown = (s->avctx->width * 3);

while (pixel_countdown > 0) {
if (bytestream2_tell(&g2) + 1 > stream_ptr_after_chunk)
break;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (byte_run > 0) {
palette_idx1 = bytestream2_get_byte(&g2);
CHECK_PIXEL_PTR(byte_run);
for (j = 0; j < byte_run; j++) {
pixels[pixel_ptr++] = palette_idx1;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) (linea%d)\n",
pixel_countdown, lines);
}
} else { 
byte_run = -byte_run;
if (bytestream2_tell(&g2) + byte_run > stream_ptr_after_chunk)
break;
CHECK_PIXEL_PTR(byte_run);
for (j = 0; j < byte_run; j++) {
palette_idx1 = bytestream2_get_byte(&g2);
pixels[pixel_ptr++] = palette_idx1;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d) at line %d\n",
pixel_countdown, lines);
}
}
}

y_ptr += s->frame->linesize[0];
}
break;

case FLI_DTA_BRUN:
y_ptr = 0;
for (lines = 0; lines < s->avctx->height; lines++) {
pixel_ptr = y_ptr;


bytestream2_skip(&g2, 1);
pixel_countdown = s->avctx->width; 

while (pixel_countdown > 0) {
if (bytestream2_tell(&g2) + 1 > stream_ptr_after_chunk)
break;
byte_run = sign_extend(bytestream2_get_byte(&g2), 8);
if (byte_run > 0) {
pixel = bytestream2_get_le24(&g2);
CHECK_PIXEL_PTR(3 * byte_run);
for (j = 0; j < byte_run; j++) {
AV_WL24(pixels + pixel_ptr, pixel);
pixel_ptr += 3;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d)\n",
pixel_countdown);
}
} else { 
byte_run = -byte_run;
if (bytestream2_tell(&g2) + 3 * byte_run > stream_ptr_after_chunk)
break;
CHECK_PIXEL_PTR(3 * byte_run);
for (j = 0; j < byte_run; j++) {
pixel = bytestream2_get_le24(&g2);
AV_WL24(pixels + pixel_ptr, pixel);
pixel_ptr += 3;
pixel_countdown--;
if (pixel_countdown < 0)
av_log(avctx, AV_LOG_ERROR, "pixel_countdown < 0 (%d)\n",
pixel_countdown);
}
}
}

y_ptr += s->frame->linesize[0];
}
break;

case FLI_COPY:
case FLI_DTA_COPY:

if (chunk_size - 6 > (unsigned int)(FFALIGN(s->avctx->width, 2) * s->avctx->height)*3) {
av_log(avctx, AV_LOG_ERROR, "In chunk FLI_COPY : source data (%d bytes) " \
"bigger than image, skipping chunk\n", chunk_size - 6);
bytestream2_skip(&g2, chunk_size - 6);
} else {
for (y_ptr = 0; y_ptr < s->frame->linesize[0] * s->avctx->height;
y_ptr += s->frame->linesize[0]) {

bytestream2_get_buffer(&g2, pixels + y_ptr, 3*s->avctx->width);
if (s->avctx->width & 1)
bytestream2_skip(&g2, 3);
}
}
break;

case FLI_MINI:

bytestream2_skip(&g2, chunk_size - 6);
break;

default:
av_log(avctx, AV_LOG_ERROR, "Unrecognized chunk type: %d\n", chunk_type);
break;
}

if (stream_ptr_after_chunk - bytestream2_tell(&g2) >= 0) {
bytestream2_skip(&g2, stream_ptr_after_chunk - bytestream2_tell(&g2));
} else {
av_log(avctx, AV_LOG_ERROR, "Chunk overread\n");
break;
}

frame_size -= chunk_size;
num_chunks--;
}



if ((bytestream2_get_bytes_left(&g2) != 0) && (bytestream2_get_bytes_left(&g2) != 1))
av_log(avctx, AV_LOG_ERROR, "Processed FLI chunk where chunk size = %d " \
"and final chunk ptr = %d\n", buf_size, bytestream2_tell(&g2));

if ((ret = av_frame_ref(data, s->frame)) < 0)
return ret;

*got_frame = 1;

return buf_size;
}

static int flic_decode_frame(AVCodecContext *avctx,
void *data, int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
if (avctx->pix_fmt == AV_PIX_FMT_PAL8) {
return flic_decode_frame_8BPP(avctx, data, got_frame,
buf, buf_size);
} else if ((avctx->pix_fmt == AV_PIX_FMT_RGB555) ||
(avctx->pix_fmt == AV_PIX_FMT_RGB565)) {
return flic_decode_frame_15_16BPP(avctx, data, got_frame,
buf, buf_size);
} else if (avctx->pix_fmt == AV_PIX_FMT_BGR24) {
return flic_decode_frame_24BPP(avctx, data, got_frame,
buf, buf_size);
}





av_log(avctx, AV_LOG_ERROR, "Unknown FLC format, my science cannot explain how this happened.\n");
return AVERROR_BUG;
}


static av_cold int flic_decode_end(AVCodecContext *avctx)
{
FlicDecodeContext *s = avctx->priv_data;

av_frame_free(&s->frame);

return 0;
}

AVCodec ff_flic_decoder = {
.name = "flic",
.long_name = NULL_IF_CONFIG_SMALL("Autodesk Animator Flic video"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_FLIC,
.priv_data_size = sizeof(FlicDecodeContext),
.init = flic_decode_init,
.close = flic_decode_end,
.decode = flic_decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
