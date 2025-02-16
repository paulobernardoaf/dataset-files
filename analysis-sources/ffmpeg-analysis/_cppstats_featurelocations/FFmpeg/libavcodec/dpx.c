




















#include "libavutil/avstring.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/intfloat.h"
#include "libavutil/imgutils.h"
#include "bytestream.h"
#include "avcodec.h"
#include "internal.h"

static unsigned int read16(const uint8_t **ptr, int is_big)
{
unsigned int temp;
if (is_big) {
temp = AV_RB16(*ptr);
} else {
temp = AV_RL16(*ptr);
}
*ptr += 2;
return temp;
}

static unsigned int read32(const uint8_t **ptr, int is_big)
{
unsigned int temp;
if (is_big) {
temp = AV_RB32(*ptr);
} else {
temp = AV_RL32(*ptr);
}
*ptr += 4;
return temp;
}

static uint16_t read10in32_gray(const uint8_t **ptr, uint32_t *lbuf,
int *n_datum, int is_big, int shift)
{
uint16_t temp;

if (*n_datum)
(*n_datum)--;
else {
*lbuf = read32(ptr, is_big);
*n_datum = 2;
}

temp = *lbuf >> shift & 0x3FF;
*lbuf = *lbuf >> 10;

return temp;
}

static uint16_t read10in32(const uint8_t **ptr, uint32_t *lbuf,
int *n_datum, int is_big, int shift)
{
if (*n_datum)
(*n_datum)--;
else {
*lbuf = read32(ptr, is_big);
*n_datum = 2;
}

*lbuf = *lbuf << 10 | *lbuf >> shift & 0x3FFFFF;

return *lbuf & 0x3FF;
}

static uint16_t read12in32(const uint8_t **ptr, uint32_t *lbuf,
int *n_datum, int is_big)
{
if (*n_datum)
(*n_datum)--;
else {
*lbuf = read32(ptr, is_big);
*n_datum = 7;
}

switch (*n_datum){
case 7: return *lbuf & 0xFFF;
case 6: return (*lbuf >> 12) & 0xFFF;
case 5: {
uint32_t c = *lbuf >> 24;
*lbuf = read32(ptr, is_big);
c |= *lbuf << 8;
return c & 0xFFF;
}
case 4: return (*lbuf >> 4) & 0xFFF;
case 3: return (*lbuf >> 16) & 0xFFF;
case 2: {
uint32_t c = *lbuf >> 28;
*lbuf = read32(ptr, is_big);
c |= *lbuf << 4;
return c & 0xFFF;
}
case 1: return (*lbuf >> 8) & 0xFFF;
default: return *lbuf >> 20;
}
}

static int decode_frame(AVCodecContext *avctx,
void *data,
int *got_frame,
AVPacket *avpkt)
{
const uint8_t *buf = avpkt->data;
int buf_size = avpkt->size;
AVFrame *const p = data;
uint8_t *ptr[AV_NUM_DATA_POINTERS];
uint32_t header_version, version = 0;
char creator[101];
char input_device[33];

unsigned int offset;
int magic_num, endian;
int x, y, stride, i, ret;
int w, h, bits_per_color, descriptor, elements, packing;
int encoding, need_align = 0;

unsigned int rgbBuffer = 0;
int n_datum = 0;

if (avpkt->size <= 1634) {
av_log(avctx, AV_LOG_ERROR, "Packet too small for DPX header\n");
return AVERROR_INVALIDDATA;
}

magic_num = AV_RB32(buf);
buf += 4;



if (magic_num == AV_RL32("SDPX")) {
endian = 0;
} else if (magic_num == AV_RB32("SDPX")) {
endian = 1;
} else {
av_log(avctx, AV_LOG_ERROR, "DPX marker not found\n");
return AVERROR_INVALIDDATA;
}

offset = read32(&buf, endian);
if (avpkt->size <= offset) {
av_log(avctx, AV_LOG_ERROR, "Invalid data start offset\n");
return AVERROR_INVALIDDATA;
}

header_version = read32(&buf, 0);
if (header_version == MKTAG('V','1','.','0'))
version = 1;
if (header_version == MKTAG('V','2','.','0'))
version = 2;
if (!version)
av_log(avctx, AV_LOG_WARNING, "Unknown header format version %s.\n",
av_fourcc2str(header_version));


buf = avpkt->data + 660;
ret = read32(&buf, endian);
if (ret != 0xFFFFFFFF) {
avpriv_report_missing_feature(avctx, "Encryption");
av_log(avctx, AV_LOG_WARNING, "The image is encrypted and may "
"not properly decode.\n");
}


buf = avpkt->data + 0x304;
w = read32(&buf, endian);
h = read32(&buf, endian);

if ((ret = ff_set_dimensions(avctx, w, h)) < 0)
return ret;


buf += 20;
descriptor = buf[0];


buf += 3;
avctx->bits_per_raw_sample =
bits_per_color = buf[0];
buf++;
packing = read16(&buf, endian);
encoding = read16(&buf, endian);

if (encoding) {
avpriv_report_missing_feature(avctx, "Encoding %d", encoding);
return AVERROR_PATCHWELCOME;
}

buf += 820;
avctx->sample_aspect_ratio.num = read32(&buf, endian);
avctx->sample_aspect_ratio.den = read32(&buf, endian);
if (avctx->sample_aspect_ratio.num > 0 && avctx->sample_aspect_ratio.den > 0)
av_reduce(&avctx->sample_aspect_ratio.num, &avctx->sample_aspect_ratio.den,
avctx->sample_aspect_ratio.num, avctx->sample_aspect_ratio.den,
0x10000);
else
avctx->sample_aspect_ratio = (AVRational){ 0, 1 };

if (offset >= 1724 + 4) {
buf = avpkt->data + 1724;
i = read32(&buf, endian);
if(i) {
AVRational q = av_d2q(av_int2float(i), 4096);
if (q.num > 0 && q.den > 0)
avctx->framerate = q;
}
}

switch (descriptor) {
case 6: 
elements = 1;
break;
case 52: 
case 51: 
case 103: 
elements = 4;
break;
case 50: 
case 102: 
elements = 3;
break;
case 100: 
elements = 2;
break;
default:
avpriv_report_missing_feature(avctx, "Descriptor %d", descriptor);
return AVERROR_PATCHWELCOME;
}

switch (bits_per_color) {
case 8:
stride = avctx->width * elements;
break;
case 10:
if (!packing) {
av_log(avctx, AV_LOG_ERROR, "Packing to 32bit required\n");
return -1;
}
stride = (avctx->width * elements + 2) / 3 * 4;
break;
case 12:
stride = avctx->width * elements;
if (packing) {
stride *= 2;
} else {
stride *= 3;
if (stride % 8) {
stride /= 8;
stride++;
stride *= 8;
}
stride /= 2;
}
break;
case 16:
stride = 2 * avctx->width * elements;
break;
case 1:
case 32:
case 64:
avpriv_report_missing_feature(avctx, "Depth %d", bits_per_color);
return AVERROR_PATCHWELCOME;
default:
return AVERROR_INVALIDDATA;
}





need_align = FFALIGN(stride, 4);
if (need_align*avctx->height + (int64_t)offset > avpkt->size) {

if (stride*avctx->height + (int64_t)offset > avpkt->size) {
av_log(avctx, AV_LOG_ERROR, "Overread buffer. Invalid header?\n");
return AVERROR_INVALIDDATA;
} else {
av_log(avctx, AV_LOG_INFO, "Decoding DPX without scanline "
"alignment.\n");
need_align = 0;
}
} else {
need_align -= stride;
stride = FFALIGN(stride, 4);
}

switch (1000 * descriptor + 10 * bits_per_color + endian) {
case 6081:
case 6080:
avctx->pix_fmt = AV_PIX_FMT_GRAY8;
break;
case 6121:
case 6120:
avctx->pix_fmt = AV_PIX_FMT_GRAY12;
break;
case 50081:
case 50080:
avctx->pix_fmt = AV_PIX_FMT_RGB24;
break;
case 52081:
case 52080:
avctx->pix_fmt = AV_PIX_FMT_ABGR;
break;
case 51081:
case 51080:
avctx->pix_fmt = AV_PIX_FMT_RGBA;
break;
case 50100:
case 50101:
avctx->pix_fmt = AV_PIX_FMT_GBRP10;
break;
case 51100:
case 51101:
avctx->pix_fmt = AV_PIX_FMT_GBRAP10;
break;
case 50120:
case 50121:
avctx->pix_fmt = AV_PIX_FMT_GBRP12;
break;
case 51120:
case 51121:
avctx->pix_fmt = AV_PIX_FMT_GBRAP12;
break;
case 6100:
case 6101:
avctx->pix_fmt = AV_PIX_FMT_GRAY10;
break;
case 6161:
avctx->pix_fmt = AV_PIX_FMT_GRAY16BE;
break;
case 6160:
avctx->pix_fmt = AV_PIX_FMT_GRAY16LE;
break;
case 50161:
avctx->pix_fmt = AV_PIX_FMT_RGB48BE;
break;
case 50160:
avctx->pix_fmt = AV_PIX_FMT_RGB48LE;
break;
case 51161:
avctx->pix_fmt = AV_PIX_FMT_RGBA64BE;
break;
case 51160:
avctx->pix_fmt = AV_PIX_FMT_RGBA64LE;
break;
case 100081:
avctx->pix_fmt = AV_PIX_FMT_UYVY422;
break;
case 102081:
avctx->pix_fmt = AV_PIX_FMT_YUV444P;
break;
case 103081:
avctx->pix_fmt = AV_PIX_FMT_YUVA444P;
break;
default:
av_log(avctx, AV_LOG_ERROR, "Unsupported format\n");
return AVERROR_PATCHWELCOME;
}

ff_set_sar(avctx, avctx->sample_aspect_ratio);

if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
return ret;

av_strlcpy(creator, avpkt->data + 160, 100);
creator[100] = '\0';
av_dict_set(&p->metadata, "Creator", creator, 0);

av_strlcpy(input_device, avpkt->data + 1556, 32);
input_device[32] = '\0';
av_dict_set(&p->metadata, "Input Device", input_device, 0);


buf = avpkt->data + offset;

for (i=0; i<AV_NUM_DATA_POINTERS; i++)
ptr[i] = p->data[i];

switch (bits_per_color) {
case 10:
for (x = 0; x < avctx->height; x++) {
uint16_t *dst[4] = {(uint16_t*)ptr[0],
(uint16_t*)ptr[1],
(uint16_t*)ptr[2],
(uint16_t*)ptr[3]};
int shift = elements > 1 ? packing == 1 ? 22 : 20 : packing == 1 ? 2 : 0;
for (y = 0; y < avctx->width; y++) {
if (elements >= 3)
*dst[2]++ = read10in32(&buf, &rgbBuffer,
&n_datum, endian, shift);
if (elements == 1)
*dst[0]++ = read10in32_gray(&buf, &rgbBuffer,
&n_datum, endian, shift);
else
*dst[0]++ = read10in32(&buf, &rgbBuffer,
&n_datum, endian, shift);
if (elements >= 2)
*dst[1]++ = read10in32(&buf, &rgbBuffer,
&n_datum, endian, shift);
if (elements == 4)
*dst[3]++ =
read10in32(&buf, &rgbBuffer,
&n_datum, endian, shift);
}
if (memcmp(input_device, "Scanity", 7))
n_datum = 0;
for (i = 0; i < elements; i++)
ptr[i] += p->linesize[i];
}
break;
case 12:
for (x = 0; x < avctx->height; x++) {
uint16_t *dst[4] = {(uint16_t*)ptr[0],
(uint16_t*)ptr[1],
(uint16_t*)ptr[2],
(uint16_t*)ptr[3]};
int shift = packing == 1 ? 4 : 0;
for (y = 0; y < avctx->width; y++) {
if (packing) {
if (elements >= 3)
*dst[2]++ = read16(&buf, endian) >> shift & 0xFFF;
*dst[0]++ = read16(&buf, endian) >> shift & 0xFFF;
if (elements >= 2)
*dst[1]++ = read16(&buf, endian) >> shift & 0xFFF;
if (elements == 4)
*dst[3]++ = read16(&buf, endian) >> shift & 0xFFF;
} else {
if (elements >= 3)
*dst[2]++ = read12in32(&buf, &rgbBuffer,
&n_datum, endian);
*dst[0]++ = read12in32(&buf, &rgbBuffer,
&n_datum, endian);
if (elements >= 2)
*dst[1]++ = read12in32(&buf, &rgbBuffer,
&n_datum, endian);
if (elements == 4)
*dst[3]++ = read12in32(&buf, &rgbBuffer,
&n_datum, endian);
}
}
n_datum = 0;
for (i = 0; i < elements; i++)
ptr[i] += p->linesize[i];

buf += need_align;
}
break;
case 16:
elements *= 2;
case 8:
if ( avctx->pix_fmt == AV_PIX_FMT_YUVA444P
|| avctx->pix_fmt == AV_PIX_FMT_YUV444P) {
for (x = 0; x < avctx->height; x++) {
ptr[0] = p->data[0] + x * p->linesize[0];
ptr[1] = p->data[1] + x * p->linesize[1];
ptr[2] = p->data[2] + x * p->linesize[2];
ptr[3] = p->data[3] + x * p->linesize[3];
for (y = 0; y < avctx->width; y++) {
*ptr[1]++ = *buf++;
*ptr[0]++ = *buf++;
*ptr[2]++ = *buf++;
if (avctx->pix_fmt == AV_PIX_FMT_YUVA444P)
*ptr[3]++ = *buf++;
}
}
} else {
av_image_copy_plane(ptr[0], p->linesize[0],
buf, stride,
elements * avctx->width, avctx->height);
}
break;
}

*got_frame = 1;

return buf_size;
}

AVCodec ff_dpx_decoder = {
.name = "dpx",
.long_name = NULL_IF_CONFIG_SMALL("DPX (Digital Picture Exchange) image"),
.type = AVMEDIA_TYPE_VIDEO,
.id = AV_CODEC_ID_DPX,
.decode = decode_frame,
.capabilities = AV_CODEC_CAP_DR1,
};
