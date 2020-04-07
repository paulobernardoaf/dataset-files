

























#include "internal.h"

typedef struct FITSContext {
int first_image;
} FITSContext;

static int fits_write_header(AVFormatContext *s)
{
FITSContext *fitsctx = s->priv_data;
fitsctx->first_image = 1;
return 0;
}









static int write_keyword_value(AVFormatContext *s, const char *keyword, int value, int *lines_written)
{
int len, ret;
uint8_t header[80];

len = strlen(keyword);
memset(header, ' ', sizeof(header));
memcpy(header, keyword, len);

header[8] = '=';
header[9] = ' ';

ret = snprintf(header + 10, 70, "%d", value);
memset(&header[ret + 10], ' ', sizeof(header) - (ret + 10));

avio_write(s->pb, header, sizeof(header));
*lines_written += 1;
return 0;
}

static int write_image_header(AVFormatContext *s)
{
AVStream *st = s->streams[0];
AVCodecParameters *encctx = st->codecpar;
FITSContext *fitsctx = s->priv_data;
uint8_t buffer[80];
int bitpix, naxis, naxis3 = 1, bzero = 0, rgb = 0, lines_written = 0, lines_left;

switch (encctx->format) {
case AV_PIX_FMT_GRAY8:
bitpix = 8;
naxis = 2;
break;
case AV_PIX_FMT_GRAY16BE:
bitpix = 16;
naxis = 2;
bzero = 32768;
break;
case AV_PIX_FMT_GBRP:
case AV_PIX_FMT_GBRAP:
bitpix = 8;
naxis = 3;
rgb = 1;
if (encctx->format == AV_PIX_FMT_GBRP) {
naxis3 = 3;
} else {
naxis3 = 4;
}
break;
case AV_PIX_FMT_GBRP16BE:
case AV_PIX_FMT_GBRAP16BE:
bitpix = 16;
naxis = 3;
rgb = 1;
if (encctx->format == AV_PIX_FMT_GBRP16BE) {
naxis3 = 3;
} else {
naxis3 = 4;
}
bzero = 32768;
break;
default:
return AVERROR(EINVAL);
}

if (fitsctx->first_image) {
memcpy(buffer, "SIMPLE = ", 10);
memset(buffer + 10, ' ', 70);
buffer[29] = 'T';
avio_write(s->pb, buffer, sizeof(buffer));
} else {
memcpy(buffer, "XTENSION= 'IMAGE '", 20);
memset(buffer + 20, ' ', 60);
avio_write(s->pb, buffer, sizeof(buffer));
}
lines_written++;

write_keyword_value(s, "BITPIX", bitpix, &lines_written); 
write_keyword_value(s, "NAXIS", naxis, &lines_written); 
write_keyword_value(s, "NAXIS1", encctx->width, &lines_written); 
write_keyword_value(s, "NAXIS2", encctx->height, &lines_written); 

if (rgb)
write_keyword_value(s, "NAXIS3", naxis3, &lines_written); 

if (!fitsctx->first_image) {
write_keyword_value(s, "PCOUNT", 0, &lines_written);
write_keyword_value(s, "GCOUNT", 1, &lines_written);
} else {
fitsctx->first_image = 0;
}






if (bitpix == 16)
write_keyword_value(s, "BZERO", bzero, &lines_written);

if (rgb) {
memcpy(buffer, "CTYPE3 = 'RGB '", 20);
memset(buffer + 20, ' ', 60);
avio_write(s->pb, buffer, sizeof(buffer));
lines_written++;
}

memcpy(buffer, "END", 3);
memset(buffer + 3, ' ', 77);
avio_write(s->pb, buffer, sizeof(buffer));
lines_written++;

lines_left = ((lines_written + 35) / 36) * 36 - lines_written;
memset(buffer, ' ', 80);
while (lines_left > 0) {
avio_write(s->pb, buffer, sizeof(buffer));
lines_left--;
}
return 0;
}

static int fits_write_packet(AVFormatContext *s, AVPacket *pkt)
{
int ret = write_image_header(s);
if (ret < 0)
return ret;
avio_write(s->pb, pkt->data, pkt->size);
return 0;
}

AVOutputFormat ff_fits_muxer = {
.name = "fits",
.long_name = NULL_IF_CONFIG_SMALL("Flexible Image Transport System"),
.extensions = "fits",
.priv_data_size = sizeof(FITSContext),
.audio_codec = AV_CODEC_ID_NONE,
.video_codec = AV_CODEC_ID_FITS,
.write_header = fits_write_header,
.write_packet = fits_write_packet,
};
