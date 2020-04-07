#include <inttypes.h>
#include "attributes.h"
#include "pixfmt.h"
#include "version.h"
typedef struct AVComponentDescriptor {
int plane;
int step;
int offset;
int shift;
int depth;
#if FF_API_PLUS1_MINUS1
attribute_deprecated int step_minus1;
attribute_deprecated int depth_minus1;
attribute_deprecated int offset_plus1;
#endif
} AVComponentDescriptor;
typedef struct AVPixFmtDescriptor {
const char *name;
uint8_t nb_components; 
uint8_t log2_chroma_w;
uint8_t log2_chroma_h;
uint64_t flags;
AVComponentDescriptor comp[4];
const char *alias;
} AVPixFmtDescriptor;
#define AV_PIX_FMT_FLAG_BE (1 << 0)
#define AV_PIX_FMT_FLAG_PAL (1 << 1)
#define AV_PIX_FMT_FLAG_BITSTREAM (1 << 2)
#define AV_PIX_FMT_FLAG_HWACCEL (1 << 3)
#define AV_PIX_FMT_FLAG_PLANAR (1 << 4)
#define AV_PIX_FMT_FLAG_RGB (1 << 5)
#define AV_PIX_FMT_FLAG_PSEUDOPAL (1 << 6)
#define AV_PIX_FMT_FLAG_ALPHA (1 << 7)
#define AV_PIX_FMT_FLAG_BAYER (1 << 8)
#define AV_PIX_FMT_FLAG_FLOAT (1 << 9)
int av_get_bits_per_pixel(const AVPixFmtDescriptor *pixdesc);
int av_get_padded_bits_per_pixel(const AVPixFmtDescriptor *pixdesc);
const AVPixFmtDescriptor *av_pix_fmt_desc_get(enum AVPixelFormat pix_fmt);
const AVPixFmtDescriptor *av_pix_fmt_desc_next(const AVPixFmtDescriptor *prev);
enum AVPixelFormat av_pix_fmt_desc_get_id(const AVPixFmtDescriptor *desc);
int av_pix_fmt_get_chroma_sub_sample(enum AVPixelFormat pix_fmt,
int *h_shift, int *v_shift);
int av_pix_fmt_count_planes(enum AVPixelFormat pix_fmt);
const char *av_color_range_name(enum AVColorRange range);
int av_color_range_from_name(const char *name);
const char *av_color_primaries_name(enum AVColorPrimaries primaries);
int av_color_primaries_from_name(const char *name);
const char *av_color_transfer_name(enum AVColorTransferCharacteristic transfer);
int av_color_transfer_from_name(const char *name);
const char *av_color_space_name(enum AVColorSpace space);
int av_color_space_from_name(const char *name);
const char *av_chroma_location_name(enum AVChromaLocation location);
int av_chroma_location_from_name(const char *name);
enum AVPixelFormat av_get_pix_fmt(const char *name);
const char *av_get_pix_fmt_name(enum AVPixelFormat pix_fmt);
char *av_get_pix_fmt_string(char *buf, int buf_size,
enum AVPixelFormat pix_fmt);
void av_read_image_line2(void *dst, const uint8_t *data[4],
const int linesize[4], const AVPixFmtDescriptor *desc,
int x, int y, int c, int w, int read_pal_component,
int dst_element_size);
void av_read_image_line(uint16_t *dst, const uint8_t *data[4],
const int linesize[4], const AVPixFmtDescriptor *desc,
int x, int y, int c, int w, int read_pal_component);
void av_write_image_line2(const void *src, uint8_t *data[4],
const int linesize[4], const AVPixFmtDescriptor *desc,
int x, int y, int c, int w, int src_element_size);
void av_write_image_line(const uint16_t *src, uint8_t *data[4],
const int linesize[4], const AVPixFmtDescriptor *desc,
int x, int y, int c, int w);
enum AVPixelFormat av_pix_fmt_swap_endianness(enum AVPixelFormat pix_fmt);
#define FF_LOSS_RESOLUTION 0x0001 
#define FF_LOSS_DEPTH 0x0002 
#define FF_LOSS_COLORSPACE 0x0004 
#define FF_LOSS_ALPHA 0x0008 
#define FF_LOSS_COLORQUANT 0x0010 
#define FF_LOSS_CHROMA 0x0020 
int av_get_pix_fmt_loss(enum AVPixelFormat dst_pix_fmt,
enum AVPixelFormat src_pix_fmt,
int has_alpha);
enum AVPixelFormat av_find_best_pix_fmt_of_2(enum AVPixelFormat dst_pix_fmt1, enum AVPixelFormat dst_pix_fmt2,
enum AVPixelFormat src_pix_fmt, int has_alpha, int *loss_ptr);
