#include "avcodec.h"
#include "png.h"
const uint8_t ff_png_pass_ymask[NB_PASSES] = {
0x80, 0x80, 0x08, 0x88, 0x22, 0xaa, 0x55,
};
static const uint8_t ff_png_pass_xmin[NB_PASSES] = {
0, 4, 0, 2, 0, 1, 0
};
static const uint8_t ff_png_pass_xshift[NB_PASSES] = {
3, 3, 2, 2, 1, 1, 0
};
void *ff_png_zalloc(void *opaque, unsigned int items, unsigned int size)
{
return av_mallocz_array(items, size);
}
void ff_png_zfree(void *opaque, void *ptr)
{
av_free(ptr);
}
int ff_png_get_nb_channels(int color_type)
{
int channels;
channels = 1;
if ((color_type & (PNG_COLOR_MASK_COLOR | PNG_COLOR_MASK_PALETTE)) ==
PNG_COLOR_MASK_COLOR)
channels = 3;
if (color_type & PNG_COLOR_MASK_ALPHA)
channels++;
return channels;
}
int ff_png_pass_row_size(int pass, int bits_per_pixel, int width)
{
int shift, xmin, pass_width;
xmin = ff_png_pass_xmin[pass];
if (width <= xmin)
return 0;
shift = ff_png_pass_xshift[pass];
pass_width = (width - xmin + (1 << shift) - 1) >> shift;
return (pass_width * bits_per_pixel + 7) >> 3;
}
