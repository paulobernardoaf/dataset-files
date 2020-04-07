#include "exif.h"
static const char *exif_get_tag_name(uint16_t id)
{
int i;
for (i = 0; i < FF_ARRAY_ELEMS(tag_list); i++) {
if (tag_list[i].id == id)
return tag_list[i].name;
}
return NULL;
}
static int exif_add_metadata(void *logctx, int count, int type,
const char *name, const char *sep,
GetByteContext *gb, int le,
AVDictionary **metadata)
{
switch(type) {
case 0:
av_log(logctx, AV_LOG_WARNING,
"Invalid TIFF tag type 0 found for %s with size %d\n",
name, count);
return 0;
case TIFF_DOUBLE : return ff_tadd_doubles_metadata(count, name, sep, gb, le, metadata);
case TIFF_SSHORT : return ff_tadd_shorts_metadata(count, name, sep, gb, le, 1, metadata);
case TIFF_SHORT : return ff_tadd_shorts_metadata(count, name, sep, gb, le, 0, metadata);
case TIFF_SBYTE : return ff_tadd_bytes_metadata(count, name, sep, gb, le, 1, metadata);
case TIFF_BYTE :
case TIFF_UNDEFINED: return ff_tadd_bytes_metadata(count, name, sep, gb, le, 0, metadata);
case TIFF_STRING : return ff_tadd_string_metadata(count, name, gb, le, metadata);
case TIFF_SRATIONAL:
case TIFF_RATIONAL : return ff_tadd_rational_metadata(count, name, sep, gb, le, metadata);
case TIFF_SLONG :
case TIFF_LONG : return ff_tadd_long_metadata(count, name, sep, gb, le, metadata);
default:
avpriv_request_sample(logctx, "TIFF tag type (%u)", type);
return 0;
};
}
static int exif_decode_tag(void *logctx, GetByteContext *gbytes, int le,
int depth, AVDictionary **metadata)
{
int ret, cur_pos;
unsigned id, count;
enum TiffTypes type;
if (depth > 2) {
return 0;
}
ff_tread_tag(gbytes, le, &id, &type, &count, &cur_pos);
if (!bytestream2_tell(gbytes)) {
bytestream2_seek(gbytes, cur_pos, SEEK_SET);
return 0;
}
ret = ff_tis_ifd(id);
if (ret) {
ret = ff_exif_decode_ifd(logctx, gbytes, le, depth + 1, metadata);
} else {
const char *name = exif_get_tag_name(id);
char *use_name = (char*) name;
if (!use_name) {
use_name = av_malloc(7);
if (!use_name) {
return AVERROR(ENOMEM);
}
snprintf(use_name, 7, "0x%04X", id);
}
ret = exif_add_metadata(logctx, count, type, use_name, NULL,
gbytes, le, metadata);
if (!name) {
av_freep(&use_name);
}
}
bytestream2_seek(gbytes, cur_pos, SEEK_SET);
return ret;
}
int ff_exif_decode_ifd(void *logctx, GetByteContext *gbytes,
int le, int depth, AVDictionary **metadata)
{
int i, ret;
int entries;
entries = ff_tget_short(gbytes, le);
if (bytestream2_get_bytes_left(gbytes) < entries * 12) {
return AVERROR_INVALIDDATA;
}
for (i = 0; i < entries; i++) {
if ((ret = exif_decode_tag(logctx, gbytes, le, depth, metadata)) < 0) {
return ret;
}
}
return ff_tget_long(gbytes, le);
}
int avpriv_exif_decode_ifd(void *logctx, const uint8_t *buf, int size,
int le, int depth, AVDictionary **metadata)
{
GetByteContext gb;
bytestream2_init(&gb, buf, size);
return ff_exif_decode_ifd(logctx, &gb, le, depth, metadata);
}
